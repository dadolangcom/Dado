# Installs Dado on Windows:  irm https://raw.githubusercontent.com/dadolangcom/Dado/main/install.ps1 | iex
#
# Downloads the release archive, checks it against the release's SHA256SUMS,
# unpacks it into %LOCALAPPDATA%\dado (or $env:DADO_DIR), and runs the
# install.bat inside it, which runs `dadoc bootstrap`. $env:DADO_VERSION picks
# a release other than the latest. With none, it takes the latest stable
# release, and while there is none the newest prerelease, asked of GitHub's
# releases API ($env:DADO_RELEASE_API overrides where).
#
# `irm | iex` runs this in the reader's own PowerShell session, so the whole
# script is one script block: its settings ($ErrorActionPreference,
# $ProgressPreference) end with it, and it never calls `exit`, which would
# close the window and everything the install said with it. Run as a file
# (`powershell -File install.ps1`), it sets the exit status as well.
# A script block literal knows the file it was written in, and under `iex`
# there is none ($MyInvocation would name the caller's script instead).
$dadoInstallScript = {}.File
& {
    $ErrorActionPreference = 'Stop'
    # Windows PowerShell 5.1 redraws a progress bar for every block it
    # downloads, which makes a download of a few megabytes take minutes.
    $ProgressPreference = 'SilentlyContinue'
    # Windows PowerShell 5.1 on an older .NET offers TLS 1.0 and 1.1 only,
    # which GitHub refuses; ask for 1.2 as well.
    try {
        [Net.ServicePointManager]::SecurityProtocol = [Net.ServicePointManager]::SecurityProtocol -bor [Net.SecurityProtocolType]::Tls12
    } catch { }

    $installed = $false
    $status = 1
    try {
        $base = if ($env:DADO_RELEASE_BASE) { $env:DADO_RELEASE_BASE } else { 'https://github.com/dadolangcom/Dado/releases' }
        $dir = if ($env:DADO_DIR) { $env:DADO_DIR } else { Join-Path $env:LOCALAPPDATA 'dado' }
        $asset = 'dado-windows-x86_64.zip'
        $url = if ($env:DADO_VERSION) { "$base/download/v$($env:DADO_VERSION.TrimStart('v'))" } else { "$base/latest/download" }

        # A finished install has written `done` to its journal; one that
        # stopped part way (no git, no network) is installed over, since
        # `dadoc bootstrap` picks up where it stopped.
        $journal = Join-Path $dir 'install.journal'
        if ((Test-Path (Join-Path $dir 'dadoc.exe')) -and (Test-Path $journal) -and (Select-String -Path $journal -Pattern '^done\t' -Quiet)) {
            throw "$dir already holds a Dado install: run 'dado upgrade' to update it, or set `$env:DADO_DIR to install into another folder"
        }
        if (Test-Path (Join-Path $dir 'dadoc.exe')) {
            Write-Host "dado install: $dir holds an install that did not finish: installing over it"
        }
        # `dadoc bootstrap` clones the collections and tools with git, so say
        # so before downloading anything.
        if (-not (Get-Command git -ErrorAction SilentlyContinue)) {
            throw "Dado's installer needs git, and there is no git on PATH. Install it (https://git-scm.com/download/win, or: winget install --id Git.Git -e), open a new PowerShell window, and run this again"
        }

        $tmp = Join-Path ([IO.Path]::GetTempPath()) ("dado-install-" + [Guid]::NewGuid())
        New-Item -ItemType Directory -Path $tmp | Out-Null
        try {
            Write-Host "dado install: downloading $asset"
            try {
                Invoke-WebRequest -UseBasicParsing -Uri "$url/$asset" -OutFile (Join-Path $tmp $asset)
            } catch {
                if ($env:DADO_VERSION) { throw }
                # `latest` skips prereleases, so before the first stable release it
                # answers 404: take the newest prerelease. GitHub's API lists a
                # repository's releases newest first, and drafts only to its owners.
                $api = $env:DADO_RELEASE_API
                if (-not $api -and $base -match '^https://github\.com/([^/]+/[^/]+)/releases$') { $api = "https://api.github.com/repos/$($Matches[1])/releases" }
                $tag = $null
                if ($api) {
                    # Windows PowerShell 5.1 hands a JSON array on as one
                    # object and PowerShell 7 as its elements; `@(...)` and
                    # the `ForEach-Object` read both the same way.
                    try {
                        $tag = @(Invoke-RestMethod -UseBasicParsing -Uri "$($api)?per_page=1" -Headers @{ Accept = 'application/vnd.github+json' }) |
                            ForEach-Object { $_ } | Select-Object -First 1 | ForEach-Object { $_.tag_name }
                    } catch { }
                }
                if (-not $tag) { throw "cannot download $url/$asset, and found no prerelease to fall back to; set `$env:DADO_VERSION to a version from $base and run this again" }
                $url = "$base/download/$tag"
                Write-Host "dado install: there is no stable release yet: downloading the newest prerelease, $tag"
                Invoke-WebRequest -UseBasicParsing -Uri "$url/$asset" -OutFile (Join-Path $tmp $asset)
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
            # A virus scanner can still hold a file it is reading; what is
            # left in the temporary folder is not worth failing the install.
            Remove-Item -Recurse -Force $tmp -ErrorAction SilentlyContinue
        }

        & (Join-Path $dir 'install.bat') @args
        $status = if ($null -eq $LASTEXITCODE) { 1 } else { $LASTEXITCODE }
        $installed = $status -eq 0
        if (-not $installed) {
            Write-Host ""
            Write-Host "dado install: the installer stopped (status $status); what it said is above. Once it is fixed, run this again: it picks up where it stopped." -ForegroundColor Red
        }
    } catch {
        Write-Host "dado install: $($_.Exception.Message)" -ForegroundColor Red
    }
    $global:LASTEXITCODE = $status
} @args
# Only a script run as a file exits; under `iex` there is no file, and `exit`
# would end the reader's session.
if ($dadoInstallScript) { exit $LASTEXITCODE }
