void setup_telnet();
void loop_telnet();
void handleHoneypotClient(WiFiClient client);
bool readLine(WiFiClient &client, bool echo = false);