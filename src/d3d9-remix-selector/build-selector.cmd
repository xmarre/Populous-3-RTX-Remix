@echo off
setlocal
set "SCRIPT_DIR=%~dp0"
set "ROOT_DIR=%SCRIPT_DIR%..\.."
set "OBJ=%SCRIPT_DIR%d3d9_remix_selector.obj"
REM Build from a shell with Zig available.
REM Output: repository root d3d9.dll
zig cc -target x86-windows-gnu -c "%SCRIPT_DIR%d3d9_remix_selector.c" -o "%OBJ%" -fno-stack-protector -fno-sanitize=undefined -nostdlib -Wno-ignored-attributes || exit /b 1
zig cc -target x86-windows-gnu -shared -nostdlib "%OBJ%" "%SCRIPT_DIR%d3d9_remix_selector.def" -Wl,--entry,DllMain@12 -Wl,--subsystem,windows -o "%ROOT_DIR%\d3d9.dll" || exit /b 1
del "%OBJ%" 2>nul
del "%ROOT_DIR%\d3d9.lib" 2>nul
del "%ROOT_DIR%\d3d9.exp" 2>nul
