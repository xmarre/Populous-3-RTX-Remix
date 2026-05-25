@echo off
setlocal
REM Build from a Developer Command Prompt or any shell with clang-cl/lld-link available.
REM Output: ..\..\d3d9.dll
clang -target i686-windows-gnu -c d3d9_remix_selector.c -o d3d9_remix_selector.obj -fno-stack-protector -nostdlib -Wno-ignored-attributes || exit /b 1
lld-link /dll /machine:x86 /nodefaultlib /out:..\..\d3d9.dll /def:d3d9_remix_selector.def /entry:DllMain@12 /subsystem:windows d3d9_remix_selector.obj || exit /b 1
del d3d9_remix_selector.obj 2>nul
del ..\..\d3d9.lib 2>nul
del ..\..\d3d9.exp 2>nul
