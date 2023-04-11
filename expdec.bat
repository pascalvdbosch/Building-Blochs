@echo off

set ENV = featheresp32
rem set PROJECT = firmware
rem set ADDRESS = 0x400d2965
rem set ELFFILE = .pio/build/featheresp32/firmware.elf

rem echo on

echo.

@pushd %~dp0
echo %~dp0
%HOME%\.platformio/packages/toolchain-xtensa32/bin/xtensa-esp32-elf-addr2line -pfiaC -e .pio/build/featheresp32/firmware.elf %*
@popd

