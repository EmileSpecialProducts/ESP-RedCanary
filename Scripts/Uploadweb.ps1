################################################################################################################################
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
    #$r=Invoke-RestMethod -Uri $UploadURL -Method Post -ContentType "multipart/form-data; boundary=`"$boundary`"" -Body $bodyLines 
    $r=Invoke-WebRequest -UseBasicParsing -Uri $UploadURL -Method Post -ContentType "multipart/form-data; boundary=`"$boundary`"" -Body $bodyLines 
    return $r
}
################################################################################################################################

$URI = "ESPRedCanary.local"

$r=Test-Connection -ComputerName $URI -Count 1 -Quiet
    if ($r -eq $false) {
        Write-Host "ESPRedCanary.local is not reachable"
        exit
    }

Write-Host "Host = $URI"
# I neded this scrip to upload the web files to the ESP32.
# if the ESP editor was not functional, 
# you will need to change the location of editor.js in editor.html so that it will not use the github location, and then upload the files using this script. 
# Create the directories on the ESP32 http, https, httpa
Invoke-RestMethod -Uri "$URI/edit?path=/http" -Method put
Write-Host "responce = $($r.StatusCode)"
Invoke-RestMethod -Uri "$URI/edit?path=/https" -Method put
Write-Host "responce = $($r.StatusCode)"
Invoke-RestMethod -Uri "$URI/edit?path=/httpa" -Method put
Write-Host "responce = $($r.StatusCode)"
# Upload the files to the ESP32 directories http, https, httpa
$r=ESPSPIFFSuploadfile "$URI/edit" '../littlefs/http/index.html' 'http/index.html'
Write-Host "responce = $($r.StatusCode)"
$r=ESPSPIFFSuploadfile "$URI/edit" '../littlefs/https/index.html' 'https/index.html'
Write-Host "responce = $($r.StatusCode)"
$r=ESPSPIFFSuploadfile "$URI/edit" '../littlefs/httpa/index.html' 'httpa/index.html'
Write-Host "responce = $($r.StatusCode)"
# Upload the files to the ESP32 root directory
$r=ESPSPIFFSuploadfile "$URI/edit" '../littlefs/index.html' 'index.html'
Write-Host "responce = $($r.StatusCode)"
$r=ESPSPIFFSuploadfile "$URI/edit" '../littlefs/editor.html' 'editor.html'
Write-Host "responce = $($r.StatusCode)"
$r=ESPSPIFFSuploadfile "$URI/edit" '../web/editor.css' 'editor.css'
Write-Host "responce = $($r.StatusCode)"
$r=ESPSPIFFSuploadfile "$URI/edit" '../web/editor.js' 'editor.js'
Write-Host "responce = $($r.StatusCode)"


