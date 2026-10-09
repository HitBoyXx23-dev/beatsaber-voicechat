param(
    [string]$BeatSaberDir = ""
)

$ErrorActionPreference = "Stop"
$pcDir = $PSScriptRoot
$repoDir = Split-Path $pcDir -Parent
$project = Join-Path $pcDir "VoiceChat\VoiceChat.csproj"
$gameVersion = "1.40.8"

$pluginDownloads = @(
    "https://github.com/monkeymanboy/BeatSaberMarkupLanguage/releases/download/v1.12.5/BeatSaberMarkupLanguage-v1.12.5+bs.1.40.0-RELEASE.zip",
    "https://github.com/Auros/SiraUtil/releases/download/v3.2.1/SiraUtil-v3.2.1+bs.1.40.0.zip",
    "https://github.com/Goobwabber/MultiplayerCore/releases/download/v1.6.2/MultiplayerCore-1.6.2-bs1.40.0-0f029ed.zip"
)

function Initialize-References([string]$refsDir) {
    if (-not (Test-Path (Join-Path $refsDir "Beat Saber_Data\Managed\Main.dll"))) {
        Write-Host "Downloading stripped Beat Saber $gameVersion references..."
        if (Test-Path $refsDir) { Remove-Item $refsDir -Recurse -Force }
        git clone --depth 1 --branch "version/$gameVersion" https://github.com/beat-forge/beatsaber-stripped.git $refsDir
        if ($LASTEXITCODE -ne 0) { throw "Could not download the Beat Saber references." }
    }

    $pluginsDir = Join-Path $refsDir "Plugins"
    $required = "BSML.dll", "SiraUtil.dll", "MultiplayerCore.dll"
    $missing = $required | Where-Object { -not (Test-Path (Join-Path $pluginsDir $_)) }
    if (-not $missing) { return }

    Write-Host "Downloading BSML, SiraUtil and MultiplayerCore..."
    New-Item -ItemType Directory $pluginsDir -Force | Out-Null
    $temp = Join-Path ([IO.Path]::GetTempPath()) ("voicechat-plugins-" + [guid]::NewGuid())
    New-Item -ItemType Directory $temp | Out-Null
    try {
        $index = 0
        foreach ($url in $pluginDownloads) {
            $index++
            $zip = Join-Path $temp "plugin$index.zip"
            Invoke-WebRequest $url -OutFile $zip -UseBasicParsing
            Expand-Archive $zip -DestinationPath (Join-Path $temp "plugin$index") -Force
        }
        Get-ChildItem $temp -Recurse -File -Filter *.dll | Where-Object { $_.Directory.Name -eq "Plugins" } |
            Copy-Item -Destination $pluginsDir -Force
        Get-ChildItem $temp -Recurse -File -Filter *.dll | Where-Object { $_.Directory.Name -eq "Libs" } |
            Copy-Item -Destination (Join-Path $refsDir "Libs") -Force
    }
    finally {
        Remove-Item $temp -Recurse -Force -ErrorAction SilentlyContinue
    }
}

if (-not $BeatSaberDir) {
    $BeatSaberDir = Join-Path $pcDir "refs"
    Initialize-References $BeatSaberDir
}

dotnet build $project -c Release "-p:BeatSaberDir=$BeatSaberDir"
if ($LASTEXITCODE -ne 0) { throw "Build failed." }

$dll = Join-Path $pcDir "VoiceChat\bin\Release\net472\VoiceChat.dll"
$stage = Join-Path ([IO.Path]::GetTempPath()) ("voicechat-pc-" + [guid]::NewGuid())
$zipPath = Join-Path $repoDir "site\downloads\VoiceChat-PC-$gameVersion.zip"
try {
    New-Item -ItemType Directory (Join-Path $stage "Plugins") -Force | Out-Null
    Copy-Item $dll (Join-Path $stage "Plugins")
    if (Test-Path $zipPath) { Remove-Item $zipPath -Force }
    Compress-Archive -Path (Join-Path $stage "Plugins") -DestinationPath $zipPath
}
finally {
    Remove-Item $stage -Recurse -Force -ErrorAction SilentlyContinue
}

Write-Host "Built $dll"
Write-Host "Packaged $zipPath"
