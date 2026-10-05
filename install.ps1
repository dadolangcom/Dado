# Installs Dado on Windows:  irm https://raw.githubusercontent.com/dadolangcom/Dado/main/install.ps1 | iex
#
# Downloads the release archive, checks it against the release's SHA256SUMS,
# unpacks it into %LOCALAPPDATA%\dado (or $env:DADO_DIR), and runs the
# install.bat inside it, which runs `dadoc bootstrap`. $env:DADO_VERSION picks
# a release other than the latest.
$ErrorActionPreference = 'Stop'
$base = if ($env:DADO_RELEASE_BASE) { $env:DADO_RELEASE_BASE } else { 'https://github.com/dadolangcom/Dado/releases' }
$dir = if ($env:DADO_DIR) { $env:DADO_DIR } else { Join-Path $env:LOCALAPPDATA 'dado' }
$asset = 'dado-windows-x86_64.zip'
$url = if ($env:DADO_VERSION) { "$base/download/v$($env:DADO_VERSION.TrimStart('v'))" } else { "$base/latest/download" }
if (Test-Path (Join-Path $dir 'dadoc.exe')) {
    throw "$dir already holds a Dado install: run 'dado upgrade' there, or set DADO_DIR for another folder"
}
$tmp = Join-Path ([IO.Path]::GetTempPath()) ("dado-install-" + [Guid]::NewGuid())
New-Item -ItemType Directory -Path $tmp | Out-Null
try {
    Write-Host "dado install: downloading $asset"
    try {
        Invoke-WebRequest -UseBasicParsing -Uri "$url/$asset" -OutFile (Join-Path $tmp $asset)
    } catch {
        # `latest` skips prereleases, so before the first stable release it answers 404.
        if (-not $env:DADO_VERSION) { throw "cannot download $url/$asset`: there may be no stable release yet; set `$env:DADO_VERSION to a version from $base and run this again" }
        throw
    }
    Invoke-WebRequest -UseBasicParsing -Uri "$url/SHA256SUMS" -OutFile (Join-Path $tmp 'SHA256SUMS')
    $line = Get-Content (Join-Path $tmp 'SHA256SUMS') | Where-Object { ($_ -split '\s+')[1] -in @($asset, "*$asset") } | Select-Object -First 1
    if (-not $line) { throw "SHA256SUMS has no line for $asset" }
    $want = ($line -split '\s+')[0].ToLower()
    $got = (Get-FileHash -Algorithm SHA256 (Join-Path $tmp $asset)).Hash.ToLower()
    if ($got -ne $want) { throw "$asset does not match SHA256SUMS (got $got, want $want); nothing was installed" }
    Expand-Archive -Path (Join-Path $tmp $asset) -DestinationPath (Join-Path $tmp 'unpacked')
    New-Item -ItemType Directory -Force -Path $dir | Out-Null
    Copy-Item -Recurse -Force (Join-Path $tmp 'unpacked\dado\*') $dir
    Write-Host "dado install: unpacked into $dir"
} finally {
    Remove-Item -Recurse -Force $tmp
}
& (Join-Path $dir 'install.bat') @args
exit $LASTEXITCODE
