param([switch]$Release)

$config = if ($Release) { "Release" } else { "Debug" }
Write-Host "Building DesktopOrg [$config]..." -ForegroundColor Cyan

$vcvars = "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat"
$cmake  = "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"

if (-Not (Test-Path $vcvars)) {
    Write-Host "MSVC not found. Please install Visual Studio Build Tools." -ForegroundColor Red
    exit 1
}

if (Test-Path "build") { Remove-Item -Recurse -Force "build" }
New-Item -ItemType Directory -Path "build" | Out-Null

$buildType = if ($Release) { "Release" } else { "Debug" }

$result = cmd /c "`"$vcvars`" x64 && cd build && `"$cmake`" -G `"NMake Makefiles`" -DCMAKE_BUILD_TYPE=$buildType .. && nmake" 2>&1
$result | Write-Host

if ($LASTEXITCODE -ne 0) {
    Write-Host "Build FAILED." -ForegroundColor Red
    exit $LASTEXITCODE
}

$exe = "build\DesktopOrg.exe"
$size = (Get-Item $exe).Length
Write-Host ""
Write-Host "Build SUCCESS [$config]" -ForegroundColor Green
Write-Host "Output : $exe ($([math]::Round($size/1KB, 1)) KB)" -ForegroundColor Green
Write-Host ""
Write-Host "To run: .\build\DesktopOrg.exe" -ForegroundColor Yellow
