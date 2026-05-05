Write-Host "Building DesktopOrg..." -ForegroundColor Cyan

if (-Not (Test-Path "build")) {
    New-Item -ItemType Directory -Force -Path "build" | Out-Null
}

cd build
cmake ..
if ($LASTEXITCODE -ne 0) {
    Write-Host "CMake configuration failed." -ForegroundColor Red
    exit $LASTEXITCODE
}

cmake --build . --config Release
if ($LASTEXITCODE -ne 0) {
    Write-Host "Build failed." -ForegroundColor Red
    exit $LASTEXITCODE
}

Write-Host "Build completed successfully! Executable is in build/Release/DesktopOrg.exe" -ForegroundColor Green
cd ..
