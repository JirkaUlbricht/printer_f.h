
# Change this according to your needs
$in  = 'C:\Users\Path\To\Input\Sample.txt'
$out = 'C:\Path\To\Output\Output.txt'


$lines = Get-Content -LiteralPath $in -Encoding UTF8
$escaped = $lines | ForEach-Object {
    $_.Replace('\','\\').Replace('"','\"').Replace('%','%%') + '\n'
}
$text = 'printf("' + ($escaped -join '') + '");'

[System.IO.File]::WriteAllText($out, $text, (New-Object System.Text.UTF8Encoding($false)))
