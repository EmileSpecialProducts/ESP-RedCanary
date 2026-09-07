#include "canary.hpp"
// Modbus/TCP honeypot implementation
// This code implements a simple Modbus/TCP honeypot that listens for incoming Modbus requests on port 502 and responds with simulated data or exceptions.
// The honeypot supports the following Modbus functions:
// - Read Coils (0x01)
// - Read Discrete Inputs (0x02)
// - Read Holding Registers (0x03)
// - Read Input Registers (0x04)
// - Write Single Coil (0x05)
// - Write Single Register (0x06)
// - Read Device Identification (0x11)
// The honeypot logs incoming requests and outgoing responses to the specified webhook URL.
// The honeypot can be enabled or disabled via the web interface, and the webhook URL can be configured in the settings.
// The honeypot uses a fixed Modbus slave ID for the Read Device Identification function, which can be configured in the settings.
// for specification see https://www.modbus.org/file/secure/modbusprotocolspecification.pdf

namespace {
  const uint16_t ModbusPort = 502;
  const size_t ModbusBufferSize = 260;

  WiFiServer modbusServer(ModbusPort);
  WiFiClient modbusClient;
  String modbusHookweb;
  String modbusSlaveID="";
  bool modbusEnabled = false;
  uint8_t modbusBuffer[ModbusBufferSize];
  size_t modbusBufferLength = 0;

  uint16_t readUint16(const uint8_t *data)
  {
    return (static_cast<uint16_t>(data[0]) << 8) | data[1];
  }

  void writeUint16(uint8_t *data, uint16_t value)
  {
    data[0] = value >> 8;
    data[1] = value & 0xff;
  }

  String bytesToHex(const uint8_t *data, size_t length)
  {
    String result;
    for (size_t index = 0; index < length; index++) {
      if (index > 0) result += ' ';
      if (data[index] < 0x10) result += '0';
      result += String(data[index], HEX);
    }
    return result;
  }

  size_t makeExceptionResponse(const uint8_t *request, uint8_t exceptionCode, uint8_t *response)
  {
    memcpy(response, request, 4);
    response[4] = 0;
    response[5] = 3;
    response[6] = request[6];
    response[7] = request[7] | 0x80;
    response[8] = exceptionCode;
    return 9;
  }

  size_t makeResponse(const uint8_t *request, size_t requestLength, uint8_t *response)
  {
    const uint8_t UnitId = request[6];
    const uint8_t function = request[7];

    size_t responseLength = 0;
  logCommand(modbusHookweb, modbusClient.remoteIP().toString(), ModbusPort,
                "Request Modbus/TCP frame: " + bytesToHex(request, requestLength) + " function=" + String(function) + " UnitId=" + String(UnitId));
    if (requestLength < 8) return makeExceptionResponse(request, 3, response);
    memcpy(response, request, 4);
    response[6] = request[6];
    response[7] = function;
    const uint16_t quantity = readUint16(request + 10);

    switch (function) {
      case 1: // Read Coils
      case 2: // Read Discrete Inputs 
      {
        if (requestLength != 12 || quantity == 0 || quantity > 2000) return makeExceptionResponse(request, 3, response);
        const uint8_t byteCount = (quantity + 7) / 8;
        response[4] = 0;
        response[5] = byteCount + 3;
        response[8] = byteCount;
        memset(response + 9, 0, byteCount);
        responseLength = 9 + byteCount;
        break;
      }
      case 3: // Read Holding Registers
      case 4: // Read Input Registers
      {
        if (requestLength != 12 || quantity == 0 || quantity > 125) return makeExceptionResponse(request, 3, response);
        const uint8_t byteCount = quantity * 2;
        response[4] = 0;
        response[5] = byteCount + 3;
        response[8] = byteCount;
        for (uint16_t index = 0; index < quantity; index++) {
            const uint16_t address = readUint16(request + 8);
          writeUint16(response + 9 + (index * 2), address + index);
        }
        responseLength = 9 + byteCount;
        break;
      }
      case 5: // Write Single Coil
      case 6: // Write Single Register
        if (requestLength != 12) return makeExceptionResponse(request, 3, response);
        response[4] = 0;
        response[5] = 6;
        memcpy(response + 8, request + 8, 4);
        responseLength = 12;
        break;
      case 15: // Write Multiple Coils
      case 16: // Write Multiple Registers
        if (requestLength < 8 || quantity == 0 || (function == 15 && quantity > 1968) || (function == 16 && quantity > 123)) {
          return makeExceptionResponse(request, 3, response);
        }
        response[4] = 0;
        response[5] = 6;
        memcpy(response + 8, request + 8, 4);
        responseLength = 12 ;
        break;
      case 17: // Read Device Identification
        //if (requestLength < 8 || quantity == 0 || (function == 15 && quantity > 1968) || (function == 16 && quantity > 123)) {
        //  return makeExceptionResponse(request, 3, response);
        //}
        writeUint16(response + 4, modbusSlaveID.length()+3);
        response[6] = UnitId;
        response[7] = function;
        response[8] = modbusSlaveID.length();
        memcpy(response + 9, modbusSlaveID.c_str(), modbusSlaveID.length());
        responseLength = 9 + modbusSlaveID.length();
        break;
      default:
        return makeExceptionResponse(request, 1, response);
    }

    return responseLength;
  }

  void processModbusFrame()
  {
    const uint16_t TransactionID = readUint16(modbusBuffer);
    const uint16_t protocolId = readUint16(modbusBuffer+2);
    const uint16_t protocolLength = readUint16(modbusBuffer + 4);
    const uint8_t  UnitId = modbusBuffer[6];

    if (protocolId != 0 || protocolLength < 2 || protocolLength > 253) {
      logCommand(modbusHookweb, modbusClient.remoteIP().toString(), ModbusPort,
                "Invalid Modbus/TCP frame: " + bytesToHex(modbusBuffer, modbusBufferLength) + "protocolId=" + String(protocolId) + " protocolLength=" + String(protocolLength) + " UnitId=" + String(UnitId));
      modbusBufferLength = 0;
      return;
    }

    const size_t frameLength = 6 + protocolLength;
    if (frameLength > modbusBufferLength) return;
    logCommand(modbusHookweb, modbusClient.remoteIP().toString(), ModbusPort,
              "Modbus/TCP request: " + bytesToHex(modbusBuffer, frameLength));

    uint8_t response[ModbusBufferSize];
    const size_t responseLength = makeResponse(modbusBuffer, frameLength, response);
    //logCommand(modbusHookweb, modbusClient.remoteIP().toString(), ModbusPort,"Modbus/TCP response:" + bytesToHex(response, responseLength));
    modbusClient.write(response, responseLength);

    const size_t remaining = modbusBufferLength - frameLength;
    memmove(modbusBuffer, modbusBuffer + frameLength, remaining);
    modbusBufferLength = remaining;
  }
}

void setup_Modbus()
{
  modbusEnabled = Settingsdoc["Modbus"]["enable"] | false;
  modbusHookweb = Settingsdoc["Modbus"]["url"] | "";
  modbusSlaveID = Settingsdoc["Modbus"]["slaveId"] | "ESPMODBUS";
  modbusBufferLength = 0;
  if (modbusEnabled) modbusServer.begin();
}

void loop_Modbus()
{
  if (!modbusEnabled) return;

  if (!modbusClient || !modbusClient.connected()) {
    modbusClient = modbusServer.accept();
    modbusBufferLength = 0;
  }
  if (!modbusClient || !modbusClient.connected()) return;

  while (modbusClient.available() && modbusBufferLength < ModbusBufferSize) {
    modbusBuffer[modbusBufferLength++] = modbusClient.read();
  }
  if (modbusBufferLength == ModbusBufferSize && modbusClient.available()) {
    logCommand(modbusHookweb, modbusClient.remoteIP().toString(), ModbusPort,
               "Oversized Modbus/TCP frame");
    modbusBufferLength = 0;
    while (modbusClient.available()) modbusClient.read();
    return;
  }
  if (modbusBufferLength >= 6) processModbusFrame();
}
