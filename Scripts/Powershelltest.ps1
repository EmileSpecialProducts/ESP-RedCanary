
Invoke-WebRequest -Uri "pool.ntp.org:23"

$URI = "http://ESPRedCanary.local"
# Dir 
Invoke-RestMethod -Uri "$URI/list?dir=/" -Method get
Invoke-RestMethod -Uri "$URI/test2.txt" -Method get
# Delete file
Invoke-RestMethod -Uri "$URI/test2.txt" -Method delete
Invoke-WebRequest -Uri "$URI/test2.txt" -Method delete
# for more Curl examples see the https://github.com/EmileSpecialProducts/UltraWiFiDuck/blob/master/test/curlTestScript.sh
curl -XDELETE "$URI/test2.txt"
# Dir /test 
Invoke-RestMethod -Uri "$URI/list?dir=/" -Method get
Invoke-WebRequest -Uri "$URI/list?dir=/" -Method get

# create file
Invoke-RestMethod -Uri "$URI/edit?path=/test.txt" -Method put

#delete file /test.txt
Invoke-RestMethod -Uri "$URI/edit?path=/test.txt" -Method delete
# Create dirctory is not supported on the
Invoke-RestMethod -Uri "$URI/edit?path=/test" -Method put
Invoke-RestMethod -Uri "$URI/list?dir=/test" -Method get
# Upload a file 
# create a test file 
"Testing $(get-date) " + [System.Guid]::NewGuid().ToString() > 'test.txt'

##################################################################################################################################
# Upload a file
##################################################################################################################################
function ESPSPIFFSuploadfile() {
    param (
        [Parameter(Mandatory = $true)][String] $UploadURL, 
        [Parameter(Mandatory = $true)][String] $File, 
        [Parameter(Mandatory = $false)][String] $Destinaionfilename 
    )
    if ([string]::IsNullOrWhiteSpace($Destinaionfilename)) { $Destinaionfilename = "/$(Split-Path $File -leaf)" }     
    
    $FilePath = Get-Item -Path $File;
    $fileBytes = [System.IO.File]::ReadAllBytes($FilePath);
    $fileEnc = [System.Text.Encoding]::GetEncoding('iso-8859-1').GetString($fileBytes);
    $boundary = [System.Guid]::NewGuid().ToString(); 
    $EOL = "`r`n";

    $bodyLines = ( 
        "--$boundary",
        "Content-Disposition: form-data; name=`"data`"; filename=`"$Destinaionfilename`"",
        "Content-Type: application/octet-stream",
        "",
        $fileEnc,
        "--$boundary", 
        "",
        "$EOL" 
    ) -join $EOL
    Invoke-RestMethod -Uri $UploadURL -Method Post -ContentType "multipart/form-data; boundary=`"$boundary`"" -Body $bodyLines 
}
################################################################################################################################
ESPSPIFFSuploadfile "$URI/edit" 'test.txt' '/test2.txt'


ESPSPIFFSuploadfile "$URI/edit" 'web\Mars.jpg' '/Mars2.jpg'
Invoke-WebRequest "$URI/Mars2.jpg" -OutFile 'web\Mars3.jpg'
compare-object (get-content 'web\Mars.jpg') (get-content 'web\Mars3.jpg')
Invoke-RestMethod -Uri "$URI/list?dir=/" -Method get
Invoke-RestMethod -Uri "$URI/edit?path=/Mars2.jpg" -Method delete

Invoke-WebRequest -URI "http://ESPRedCanary.local/reboot"
Invoke-WebRequest -URI "http://ESPRedCanary.local/list?dir=/"


For ($i = 0; $i -le 50; $i++) { $a=Invoke-WebRequest -URI "http://ESPRedCanary.local/run?cmd=ls" }

Invoke-WebRequest -URI "http://ESPRedCanary.local/reboot" -UseBasicParsing
Invoke-WebRequest -URI "http://ESPRedCanary.local/reboot"
Invoke-WebRequest -URI "http://ESPRedCanary.local:8080"
For ($i = 0; $i -le 50; $i++) { Invoke-WebRequest -URI "http://ESPRedCanary.local:8080" }

https://canarytokens.com/

Invoke-WebRequest -URI "http://canarytokens.com/static/ __canarytoken__ /payments.js"
Invoke-WebRequest -URI "http://canarytokens.com/static/ __canarytoken__ /payments.js?l=isLparameter&r=Rparameter&f=isFparameter"

$params = @{"content"="📡 **Honeypot**";
 "📌 Port"="22";
 "💻 Command"="SSH";
}
Invoke-WebRequest -URI "http://canarytokens.com/static/73vs6zjuwpk1ywfu6fnf8vfxo/payments.js"
Invoke-WebRequest -Uri "http://canarytokens.com/static/73vs6zjuwpk1ywfu6fnf8vfxo/payments.js" -Method POST -Body ($params|ConvertTo-Json) -ContentType "application/json"

Invoke-WebRequest -Uri "http://canarytokens.com/static/73vs6zjuwpk1ywfu6fnf8vfxo/payments.js" -Method POST -Body $params