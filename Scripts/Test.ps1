param(
	[string]$ESPredCanaryHost = "espredcanary.local"
)

Test-Connection -ComputerName $ESPredCanaryHost -Count 1
write-host  $ESPredCanaryHost
$Response = Invoke-WebRequest -UseBasicParsing -Uri $ESPredCanaryHost
write-host "Port 80: $($Response).StatusCode"

write-host  $ESPredCanaryHost":8080"
$Response = Invoke-WebRequest -UseBasicParsing -Uri $ESPredCanaryHost":8080"
write-host "Port 8080: $($Response).StatusCode"

write-host  $ESPredCanaryHost":443"
$Response = Invoke-WebRequest -UseBasicParsing -Uri $ESPredCanaryHost":443"
write-host "Port 443: $($Response).StatusCode"
Invoke-WebRequest -UseBasicParsing -Uri "espredcanary.local:443"