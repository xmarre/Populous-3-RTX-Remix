@echo off
setlocal
set "SCRIPT_DIR=%~dp0"
set "ROOT_DIR=%SCRIPT_DIR%..\.."
set "OBJ=%SCRIPT_DIR%d3d9_remix_selector.obj"
REM Build from a Developer Command Prompt or any shell with clang-cl/lld-link available.
REM Output: repository root d3d9.dll
clang -target i686-windows-gnu -c "%SCRIPT_DIR%d3d9_remix_selector.c" -o "%OBJ%" -fno-stack-protector -nostdlib -Wno-ignored-attributes || exit /b 1
lld-link /dll /machine:x86 /nodefaultlib /out:"%ROOT_DIR%\d3d9.dll" /def:"%SCRIPT_DIR%d3d9_remix_selector.def" /entry:DllMain@12 /subsystem:windows "%OBJ%" || exit /b 1
del "%OBJ%" 2>nul
del "%ROOT_DIR%\d3d9.lib" 2>nul
del "%ROOT_DIR%\d3d9.exp" 2>nul
