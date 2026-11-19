param(
    [string]$Engine = 'D:\Engines\UE_5.4',
    [string]$PackageDir = 'G:\Assistant\FlaresToolsetRelease\FlaresToolset',
    [switch]$LowMemory
)

$ErrorActionPreference = 'Stop'
$pluginRoot = $PSScriptRoot
$logFolder = 'G:\Assistant\FlaresToolsetLogs'
$env:uebp_LogFolder = $logFolder
$packageRoot = $PackageDir

New-Item -ItemType Directory -Force $logFolder | Out-Null
New-Item -ItemType Directory -Force (Split-Path $packageRoot -Parent) | Out-Null

$uatArgs = @(
    '-WaitForUATMutex',
    'BuildPlugin',
    "-Plugin=$pluginRoot\FlaresToolset.uplugin",
    "-Package=$packageRoot",
    '-TargetPlatforms=Win64',
    '-Rocket'
)

if ($LowMemory) {
    $uatArgs += @('-NoHostPlatform', '-NoTargetPlatforms', '-NoDeleteHostProject')
}

& (Join-Path $Engine 'Engine\Build\BatchFiles\RunUAT.bat') @uatArgs
if ($LASTEXITCODE -ne 0) {
    throw "Plugin build failed ($LASTEXITCODE). See $env:uebp_LogFolder"
}

if ($LowMemory) {
    # Let UAT create/filter the source package, then compile one action at a time.
    $hostRoot = Join-Path $packageRoot 'HostProject'
    $hostPlugin = Join-Path $hostRoot 'Plugins\FlaresToolset'
    $hostProject = Join-Path $hostRoot 'HostProject.uproject'
    $dotnet = Join-Path $Engine 'Engine\Binaries\ThirdParty\DotNet\6.0.302\windows\dotnet.exe'
    $ubt = Join-Path $Engine 'Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.dll'
    foreach ($target in @(@('UnrealEditor', 'Development'), @('UnrealGame', 'Development'), @('UnrealGame', 'Shipping'))) {
        $manifest = Join-Path $hostRoot "Saved\Manifest-$($target[0])-Win64-$($target[1]).xml"
        & $dotnet $ubt $target[0] Win64 $target[1] "-Project=$hostProject" "-plugin=$hostPlugin\FlaresToolset.uplugin" -noubtmakefiles "-manifest=$manifest" -nohotreload -MaxParallelActions=1 "-log=$env:uebp_LogFolder\LowMemory-$($target[0])-$($target[1]).txt"
        if ($LASTEXITCODE -ne 0) { throw "Low-memory build failed: $($target -join ' ')" }
        [xml]$products = Get-Content -LiteralPath $manifest -Raw
        foreach ($product in $products.BuildManifest.BuildProducts.string) {
            $absolute = [IO.Path]::GetFullPath([string]$product)
            $prefix = [IO.Path]::GetFullPath($hostPlugin).TrimEnd('\') + '\'
            if (!$absolute.StartsWith($prefix, [StringComparison]::OrdinalIgnoreCase)) { continue }
            $destination = Join-Path $packageRoot $absolute.Substring($prefix.Length)
            New-Item -ItemType Directory -Force (Split-Path $destination -Parent) | Out-Null
            Copy-Item -LiteralPath $absolute -Destination $destination -Force
        }
    }
    # Include all generated Inc files
    Get-ChildItem -LiteralPath (Join-Path $hostPlugin 'Intermediate\Build') -Recurse -File | Where-Object { $_.FullName -match '\\Inc\\' } | ForEach-Object {
        $relative = $_.FullName.Substring($hostPlugin.Length + 1)
        $destination = Join-Path $packageRoot $relative
        New-Item -ItemType Directory -Force (Split-Path $destination -Parent) | Out-Null
        Copy-Item -LiteralPath $_.FullName -Destination $destination -Force
    }
    Write-Host 'LOW-MEMORY BUILD SUCCESSFUL.'
}
