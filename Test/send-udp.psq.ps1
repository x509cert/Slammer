while (1) {
    $length = Get-Random -Minimum 5 -Maximum 250
    $randomString = -join ((32..255) | Get-Random -Count $length | ForEach-Object {[char]$_})
    $randomStringArray = $randomString.ToCharArray()
    $randomStringArray[0] = Get-Random -Minimum 0 -Maximum 20

    $udpClient = New-Object System.Net.Sockets.UdpClient
    $udpClient.Connect("127.0.0.1", 1434)
    $bytes = [System.Text.Encoding]::UTF8.GetBytes($randomStringArray)
    $udpClient.Send($bytes, $bytes.Length)

    $udpClient.Close()
}