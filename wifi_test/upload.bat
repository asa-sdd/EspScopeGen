@echo off
echo ========================================
echo    ESP32 WROVER Portable Firmware Upload
echo ========================================
echo.
echo IMPORTANT: Hold BOOT button on ESP32 NOW!
echo Then press any key to start upload...
echo.
pause

"C:\Users\exs\AppData\Local\Arduino15\packages\esp32\tools\esptool_py\5.1.0\esptool.exe" --chip esp32 --port COM4 --baud 115200 --before default-reset --after hard-reset write-flash -z --flash-mode dio --flash_freq 80m --flash_size 4MB 0x1000 "%~dp0wifi_test.ino.bootloader.bin" 0x8000 "%~dp0wifi_test.ino.partitions.bin" 0xe000 "%~dp0boot_app0.bin" 0x10000 "%~dp0wifi_test.ino.bin"

echo.
echo ========================================
echo Upload Complete!
echo ========================================
pause
