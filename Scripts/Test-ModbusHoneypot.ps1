

function ConvertTo-HexString {
	param([byte[]]$Bytes)

	return (($Bytes | ForEach-Object { $_.ToString("X2") }) -join " ")
}

function Read-ExactBytes {
	param(
		[System.IO.Stream]$Stream,
		[int]$Count
	)

	[byte[]]$Buffer = New-Object byte[] $Count
	$Offset = 0
	while ($Offset -lt $Count) {
		$Read = $Stream.Read($Buffer, $Offset, $Count - $Offset)
		if ($Read -le 0) {
			throw "Modbus connection closed while reading the response."
		}
		$Offset += $Read
	}
	return ,$Buffer
}

function Invoke-ModbusRequest {
	param(
		[System.IO.Stream]$Stream,
		[byte[]]$Request
	)

	$Stream.Write($Request, 0, $Request.Length)
	$Header = Read-ExactBytes -Stream $Stream -Count 6
	$BodyLength = ($Header[4] -shl 8) -bor $Header[5]
	$Body = Read-ExactBytes -Stream $Stream -Count $BodyLength
	return [byte[]]($Header + $Body)
}

function Assert-ModbusResponse {
	param(
		[byte[]]$Response,
		[int]$TransactionId,
		[int]$FunctionCode
	)

	if ($Response.Length -lt 8) {
		throw "Modbus response is too short: $(ConvertTo-HexString $Response)"
	}
	if ($Response[0] -ne (($TransactionId -shr 8) -band 0xff) -or $Response[1] -ne ($TransactionId -band 0xff)) {
		throw "Unexpected Modbus transaction ID: $(ConvertTo-HexString $Response)"
	}
	if ($Response[6] -ne 1) {
		throw "Unexpected Modbus unit ID: $($Response[6])"
	}
	if ($Response[7] -ne $FunctionCode) {
		throw "Unexpected Modbus function code: $($Response[7]) must be $FunctionCode ::  $(ConvertTo-HexString $Response)"
	}
}

function Test-ModbusHoneypot {
    param(
	[string]$ModbusHost = "espredcanary.local",
	[int]$ModbusPort = 502
    )
    [byte] $TransactionId = 1
    [byte] $ProtocolIdLow = 0
    [byte] $ProtocolIdHi = 0
    
	$Client = New-Object System.Net.Sockets.TcpClient
	$Client.ReceiveTimeout = 3000
	$Client.SendTimeout = 3000

	try {
		Write-Host "Connecting to Modbus honeypot at $ModbusHost`:$ModbusPort..."
		$Client.Connect($ModbusHost, $ModbusPort)
		$Stream = $Client.GetStream()

		# Read five holding registers. The honeypot returns register values 0, 1, 2, 3, and 4.
		[byte[]]$ReadHoldingRegisters = 0, $TransactionId, $ProtocolIdHi, $ProtocolIdLow, 0, 6, 1, 3, 0, 0, 0, 5
		$Response = Invoke-ModbusRequest -Stream $Stream -Request $ReadHoldingRegisters
		Assert-ModbusResponse -Response $Response -TransactionId $TransactionId -FunctionCode 3
		if ($Response[8] -ne 10 -or $Response[9] -ne 0 -or $Response[10] -ne 0 -or $Response[17] -ne 0 -or $Response[18] -ne 4) {
			throw "Unexpected holding-register response: $(ConvertTo-HexString $Response)"
		}
		Write-Host "PASS: read holding registers"
        $TransactionId++
		# Read ten coils. The honeypot returns two zero bytes.
		[byte[]]$ReadCoils = 0, $TransactionId, $ProtocolIdHi, $ProtocolIdLow, 0, 6, 1, 1, 0, 0, 0, 10
		$Response = Invoke-ModbusRequest -Stream $Stream -Request $ReadCoils
		Assert-ModbusResponse -Response $Response -TransactionId $TransactionId -FunctionCode 1
		if ($Response[8] -ne 2 -or $Response[9] -ne 0 -or $Response[10] -ne 0) {
			throw "Unexpected coil response: $(ConvertTo-HexString $Response)"
		}
		Write-Host "PASS: read coils"
        $TransactionId++
		# Function 0x46 is unsupported and should produce Modbus exception 0x01.
		[byte[]]$UnsupportedFunction = 0, $TransactionId, $ProtocolIdHi, $ProtocolIdLow, 0, 2, 1, 0x46
		$Response = Invoke-ModbusRequest -Stream $Stream -Request $UnsupportedFunction
		Assert-ModbusResponse -Response $Response -TransactionId $TransactionId -FunctionCode 0xC6
		if ($Response[8] -ne 1) {
			throw "Unexpected Modbus exception response: $(ConvertTo-HexString $Response)"
		}
		Write-Host "PASS: unsupported function returns exception 0x01"
        $TransactionId++
        # Read The ID.
		[byte[]]$ReadCoils = 0, $TransactionId, $ProtocolIdHi, $ProtocolIdLow, 0, 2, 1, 17
		$Response = Invoke-ModbusRequest -Stream $Stream -Request $ReadCoils
		Assert-ModbusResponse -Response $Response -TransactionId $TransactionId -FunctionCode 17
		if ($Response.Length -lt 1 ) {
			throw "Unexpected ID response: $(ConvertTo-HexString $Response)"
		}
		Write-Host "PASS: read ID response: $(ConvertTo-HexString $Response)"
        $TransactionId++
        
	}
	catch {
		Write-Error "Modbus honeypot test failed: $($_.Exception.Message)"
		exit 1
	}
	finally {
		if ($Stream) { $Stream.Dispose() }
		$Client.Dispose()
	}
}

Test-ModbusHoneypot

