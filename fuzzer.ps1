$udp = [System.Net.Sockets.UdpClient]::new()
$ep = [System.Net.IPEndPoint]::new([System.Net.IPAddress]::Parse("127.0.0.1"), 1434)

while ($true) {
    $len = Get-Random -Min 100 -Max 301
    $bytes = [byte[]]::new($len + 1)
    $bytes[0] = Get-Random -Min 0 -Max 17
    1..$len | ForEach-Object { $bytes[$_] = Get-Random -Min 0 -Max 256 }
    $udp.Send($bytes, $bytes.Length, $ep) | Out-Null
    Start-Sleep -Milliseconds 100
}

