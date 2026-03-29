$Response = Invoke-WebRequest -Uri espredcanary.local:8080
write-host $Response.StatusCode 

$Response = Invoke-WebRequest -Uri espredcanary.local:443
write-host $Response.StatusCode 

