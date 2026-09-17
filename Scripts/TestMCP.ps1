param(
	[string]$McpHost = "espredcanary.local",
	[int]$McpPort = 80,
	[switch]$UseHttps,
	[switch]$SkipCertificateCheck
)

$ErrorActionPreference = "Stop"

function Invoke-McpHttpRequest {
	param(
		[ValidateSet("GET", "POST")]
		[string]$Method,
		[string]$Uri,
		[string]$Body
	)

	$Parameters = @{
		Method = $Method
		Uri = $Uri
		UseBasicParsing = $true
		ErrorAction = "Stop"
	}
	if ($Method -eq "POST") {
		$Parameters.ContentType = "application/json"
		$Parameters.Body = $Body
	}
	if ($SkipCertificateCheck -and ((Get-Command Invoke-WebRequest).Parameters.ContainsKey("SkipCertificateCheck"))) {
		$Parameters.SkipCertificateCheck = $true
	}

	try {
		return Invoke-WebRequest @Parameters
	}
	catch {
		$Response = $_.Exception.Response
		if ($null -eq $Response) {
			throw
		}

		$Reader = New-Object System.IO.StreamReader($Response.GetResponseStream())
		try {
			$ResponseBody = $Reader.ReadToEnd()
		}
		finally {
			$Reader.Dispose()
		}

		return [pscustomobject]@{
			StatusCode = [int]$Response.StatusCode
			Headers = $Response.Headers
			Content = $ResponseBody
		}
	}
}

function Assert-Mcp {
	param(
		[bool]$Condition,
		[string]$Message
	)

	if (-not $Condition) {
		throw $Message
	}
}

function Invoke-McpJsonRpc {
	param(
		[string]$Uri,
		[hashtable]$Request
	)

	$Body = $Request | ConvertTo-Json -Depth 10 -Compress
	$Response = Invoke-McpHttpRequest -Method POST -Uri $Uri -Body $Body
	$Parsed = $null
	if (-not [string]::IsNullOrWhiteSpace($Response.Content)) {
		$Parsed = $Response.Content | ConvertFrom-Json
	}

	return [pscustomobject]@{
		HttpResponse = $Response
		Json = $Parsed
	}
}

function Assert-McpError {
	param(
		[pscustomobject]$Result,
		[int]$Code,
		[string]$Message
	)

	Assert-Mcp ($Result.HttpResponse.StatusCode -eq 200) "$Message returned HTTP $($Result.HttpResponse.StatusCode)."
	Assert-Mcp ($null -ne $Result.Json.error) "$Message did not return a JSON-RPC error."
	Assert-Mcp ($Result.Json.error.code -eq $Code) "$Message returned error code $($Result.Json.error.code), expected $Code."
}

function Test-Mcp {
	param(
		[string]$DeviceHost,
		[int]$Port,
		[bool]$Https,
		[bool]$AllowInvalidCertificate
	)

	$Scheme = if ($Https) { "https" } else { "http" }
	$Uri = "${Scheme}://${DeviceHost}:${Port}/mcp"
	$Passed = 0

	Write-Host "Testing MCP endpoint $Uri"

	$GetResponse = Invoke-McpHttpRequest -Method GET -Uri $Uri
	Assert-Mcp ($GetResponse.StatusCode -eq 405) "GET /mcp returned HTTP $($GetResponse.StatusCode), expected 405."
	Assert-Mcp ($GetResponse.Headers["Allow"] -eq "POST") "GET /mcp did not advertise Allow: POST."
	Write-Host "PASS: GET /mcp is rejected with Allow: POST"
	$Passed++

	$Initialize = Invoke-McpJsonRpc -Uri $Uri -Request @{ jsonrpc = "2.0"; id = 1; method = "initialize"; params = @{} }
	Assert-Mcp ($Initialize.HttpResponse.StatusCode -eq 200) "initialize returned HTTP $($Initialize.HttpResponse.StatusCode)."
	Assert-Mcp ($Initialize.Json.id -eq 1) "initialize returned the wrong request ID."
	Assert-Mcp ($Initialize.Json.result.protocolVersion -eq "2025-06-18") "initialize returned the wrong protocol version."
	Assert-Mcp ($Initialize.Json.result.serverInfo.name -eq "ESP-RedCanary MCP Simulator") "initialize returned the wrong server name."
	Write-Host "PASS: initialize"
	$Passed++

	$Ping = Invoke-McpJsonRpc -Uri $Uri -Request @{ jsonrpc = "2.0"; id = 2; method = "ping" }
	Assert-Mcp ($Ping.Json.id -eq 2 -and $null -ne $Ping.Json.result) "ping returned an unexpected response."
	Write-Host "PASS: ping"
	$Passed++

	$Tools = Invoke-McpJsonRpc -Uri $Uri -Request @{ jsonrpc = "2.0"; id = 3; method = "tools/list" }
	$ToolNames = @($Tools.Json.result.tools | ForEach-Object { $_.name })
	Assert-Mcp ($ToolNames -contains "get_device_status") "tools/list did not include get_device_status."
	Assert-Mcp ($ToolNames -contains "scan_networks") "tools/list did not include scan_networks."
	Write-Host "PASS: tools/list"
	$Passed++

	$Status = Invoke-McpJsonRpc -Uri $Uri -Request @{ jsonrpc = "2.0"; id = 4; method = "tools/call"; params = @{ name = "get_device_status" } }
	Assert-Mcp ($Status.Json.result.content[0].type -eq "text") "get_device_status returned a non-text result."
	Assert-Mcp ($Status.Json.result.content[0].text -match "ESP32 RedCanary honeypot online; uptime=\d+ms") "get_device_status returned unexpected text."
	Write-Host "PASS: tools/call get_device_status"
	$Passed++

	$Networks = Invoke-McpJsonRpc -Uri $Uri -Request @{ jsonrpc = "2.0"; id = 5; method = "tools/call"; params = @{ name = "scan_networks" } }
	Assert-Mcp ($Networks.Json.result.content[0].text -eq "CafeNet (-54 dBm), OfficeGuest (-67 dBm), PrinterSetup (-78 dBm)") "scan_networks returned unexpected text."
	Write-Host "PASS: tools/call scan_networks"
	$Passed++

	$UnknownTool = Invoke-McpJsonRpc -Uri $Uri -Request @{ jsonrpc = "2.0"; id = 6; method = "tools/call"; params = @{ name = "unknown_tool" } }
	Assert-McpError -Result $UnknownTool -Code -32602 -Message "unknown tool"
	Write-Host "PASS: unknown tool returns -32602"
	$Passed++

	$UnknownMethod = Invoke-McpJsonRpc -Uri $Uri -Request @{ jsonrpc = "2.0"; id = 7; method = "unknown/method" }
	Assert-McpError -Result $UnknownMethod -Code -32601 -Message "unknown method"
	Write-Host "PASS: unknown method returns -32601"
	$Passed++

	$InvalidId = Invoke-McpJsonRpc -Uri $Uri -Request @{ jsonrpc = "2.0"; id = @{ invalid = $true }; method = "ping" }
	Assert-McpError -Result $InvalidId -Code -32600 -Message "invalid request ID"
	Write-Host "PASS: invalid request ID returns -32600"
	$Passed++

	$ParseResponse = Invoke-McpHttpRequest -Method POST -Uri $Uri -Body '{"jsonrpc":"2.0",'
	$ParseJson = $ParseResponse.Content | ConvertFrom-Json
	Assert-Mcp ($ParseResponse.StatusCode -eq 200 -and $ParseJson.error.code -eq -32700) "parse error response was unexpected."
	Write-Host "PASS: malformed JSON returns -32700"
	$Passed++

	$Notification = Invoke-McpJsonRpc -Uri $Uri -Request @{ jsonrpc = "2.0"; method = "ping" }
	Assert-Mcp ($Notification.HttpResponse.StatusCode -eq 202) "notification returned HTTP $($Notification.HttpResponse.StatusCode), expected 202."
	Assert-Mcp ([string]::IsNullOrEmpty($Notification.HttpResponse.Content)) "notification returned a response body."
	Write-Host "PASS: notification returns 202 with an empty body"
	$Passed++

	Write-Host "MCP test completed: $Passed checks passed."
}

try {
	Test-Mcp -DeviceHost $McpHost -Port $McpPort -Https ([bool]$UseHttps) -AllowInvalidCertificate ([bool]$SkipCertificateCheck)
}
catch {
	Write-Error "MCP test failed: $($_.Exception.Message)"
	exit 1
}
