@echo off

setlocal enabledelayedexpansion

@echo [+] Building...

rmdir /s /q build
mkdir build

cmake -G "Ninja" -S . -B build
if %errorlevel% eq 0 (
    @echo [+] CMake done
) else (
    @echo [-] CMake failed
    exit /b 1
)

cmake --build build --config Release
if %errorlevel% eq 0 (
    @echo [+] Build done
) else (
    @echo [-] Build failed
    exit /b 1
)

call build\bin\PtP_Chat.exe
if %errorlevel% eq 0 (
    @echo [+] Running chat
) else (
    @echo [-] Running chat failed
    exit /b 1
)

@echo [+] Done!

endlocal
