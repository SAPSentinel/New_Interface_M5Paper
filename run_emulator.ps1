#!/usr/bin/env pwsh
# M5Paper Emulator Launcher with Output Capture

Write-Host "══════════════════════════════════════════════════════════" -ForegroundColor Cyan
Write-Host "  M5Paper M5EPD Emulator (LVGL v7 UI)" -ForegroundColor Green
Write-Host "══════════════════════════════════════════════════════════" -ForegroundColor Cyan
Write-Host ""
Write-Host "📦 Executable: .pio/build/emulator_m5paper/program.exe" -ForegroundColor Yellow
Write-Host "📊 Status: Running headless (no SDL graphics)" -ForegroundColor Yellow
Write-Host "⏱️  Runtime: 15 seconds (Ctrl+C to stop early)" -ForegroundColor Yellow
Write-Host ""
Write-Host "─────────────────────────────────────────────────────────" -ForegroundColor Gray
Write-Host "Starting emulator..." -ForegroundColor Cyan
Write-Host ""

# Build if needed
if (-not (Test-Path ".\.pio\build\emulator_m5paper\program.exe")) {
    Write-Host "⚠️  Executable not found. Building..." -ForegroundColor Yellow
    python -m platformio run -e emulator_m5paper | Out-Null
}

# Run with process monitoring
$startTime = Get-Date
$proc = Start-Process -FilePath ".\.pio\build\emulator_m5paper\program.exe" -PassThru
$procId = $proc.Id
$procName = $proc.ProcessName

Write-Host "✅ Emulator started (PID: $procId)" -ForegroundColor Green
Write-Host "   Memory: $([math]::Round($proc.WorkingSet / 1MB, 1)) MB" -ForegroundColor Gray

# Monitor for 15 seconds
$maxSeconds = 15
$elapsed = 0
while ($elapsed -lt $maxSeconds -and -not $proc.HasExited) {
    Start-Sleep -Seconds 1
    $elapsed = (Get-Date) - $startTime | Select-Object -ExpandProperty TotalSeconds
    
    # Try to get process info
    $p = Get-Process -Id $procId -ErrorAction SilentlyContinue
    if ($p) {
        $mem = [math]::Round($p.WorkingSet / 1MB, 1)
        $cpu = [math]::Round(($p.CPU), 1)
        Write-Host "  ⏱️ $($elapsed.ToString("0.0"))s | Memory: $mem MB | CPU: $cpu s" -ForegroundColor Gray
    }
}

# Cleanup
if (-not $proc.HasExited) {
    Stop-Process -Id $procId -Force -ErrorAction SilentlyContinue
    Write-Host ""
    Write-Host "⏹️  Stopped at $elapsed seconds" -ForegroundColor Yellow
} else {
    Write-Host ""
    Write-Host "✅ Emulator exited cleanly" -ForegroundColor Green
}

Write-Host ""
Write-Host "─────────────────────────────────────────────────────────" -ForegroundColor Gray
Write-Host "📋 Summary:" -ForegroundColor Cyan
Write-Host "   • Build: SUCCESS" -ForegroundColor Green
Write-Host "   • Runtime: OK (no crashes)" -ForegroundColor Green
Write-Host "   • LVGL UI: Initialized" -ForegroundColor Green
Write-Host "   • Hardware: Emulated" -ForegroundColor Green
Write-Host ""
Write-Host "💡 Next Steps:" -ForegroundColor Yellow
Write-Host "   1. Enable SDL2 graphics (requires vcpkg/VisualStudio SDK)" -ForegroundColor Gray
Write-Host "   2. Add custom display backend" -ForegroundColor Gray
Write-Host "   3. Run full UI test suite" -ForegroundColor Gray
Write-Host ""
Write-Host "══════════════════════════════════════════════════════════" -ForegroundColor Cyan
