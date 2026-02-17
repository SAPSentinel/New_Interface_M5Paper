# PowerShell build wrapper for M5Paper Emulator with MSVC + SDL2
param([switch]$Clean = $false)

Set-Location (Split-Path -Parent $MyInvocation.MyCommand.Path)

Write-Host "============================================================" -ForegroundColor Cyan
Write-Host "M5Paper Emulator Build - MSVC + SDL2" -ForegroundColor Cyan
Write-Host "============================================================" -ForegroundColor Cyan

$vcvars = "D:\VisualStudio2022\VC\Auxiliary\Build\vcvarsall.bat"
$args_str = if($Clean) { "run -e emulator_m5paper --target clean && python -m platformio run -e emulator_m5paper" } else { "run -e emulator_m5paper" }

Write-Host "`nInitializing MSVC and building..." -ForegroundColor Yellow

# Run with vcvarsall
cmd /c "`"$vcvars`" x64 && python -m platformio $args_str"

if ($LASTEXITCODE -eq 0) {
    Write-Host "`n✓ Build successful! Copying SDL2.dll..." -ForegroundColor Green
    Copy-Item -Path "C:\vcpkg\installed\x64-windows\bin\SDL2.dll" -Destination ".pio\build\emulator_m5paper\" -Force -ErrorAction SilentlyContinue
    Write-Host "`n" + ("="*60) -ForegroundColor Green
    Write-Host "✓ BUILD SUCCESSFUL" -ForegroundColor Green  
    Write-Host ("="*60) -ForegroundColor Green
    Write-Host "`nTo run: .\.pio\build\emulator_m5paper\program.exe" -ForegroundColor Yellow
} else {
    Write-Host "`n" + ("="*60) -ForegroundColor Red
    Write-Host "✗ BUILD FAILED" -ForegroundColor Red
    Write-Host ("="*60) -ForegroundColor Red
}

exit $LASTEXITCODE

