# Validation

Build the selector with:

```text
clang -target i686-windows-gnu -c src/d3d9-remix-selector/d3d9_remix_selector.c \
  -o d3d9_remix_selector.obj \
  -fno-stack-protector -nostdlib -Wno-ignored-attributes -Wall -Wextra

lld-link /dll /machine:x86 /nodefaultlib \
  /out:d3d9.dll \
  /def:src/d3d9-remix-selector/d3d9_remix_selector.def \
  /entry:DllMain@12 \
  /subsystem:windows \
  d3d9_remix_selector.obj
```

Expected result:

```text
d3d9.dll: PE32 executable for MS Windows 6.00 (DLL), Intel i386
```

Export table contains:

```text
D3DPERF_BeginEvent
D3DPERF_EndEvent
D3DPERF_GetStatus
D3DPERF_QueryRepeatFrame
D3DPERF_SetMarker
D3DPERF_SetOptions
D3DPERF_SetRegion
DebugSetLevel
DebugSetMute
Direct3DCreate9
Direct3DCreate9Ex
Direct3DCreate9On12
Direct3DCreate9On12Ex
Direct3D9EnableMaximizedWindowedModeShim
Direct3DShaderValidatorCreate9
PSGPError
PSGPSampleTexture
```

Static export-surface check performed against the local 32-bit Windows system DLL:

```text
objdump -p C:\Windows\SysWOW64\d3d9.dll
```

The observed system export table uses ordinal base 16 and address-table ordinals 16 through 38. The selector `.def` mirrors those ordinals, including unnamed ordinal-only exports 16, 17, 18, 19, 22, and 23.

Selector export manifest:

| Ordinal | Name |
| --- | --- |
| 16 | unnamed ordinal-only forward |
| 17 | unnamed ordinal-only forward |
| 18 | unnamed ordinal-only forward |
| 19 | unnamed ordinal-only forward |
| 20 | `Direct3DCreate9On12` |
| 21 | `Direct3DCreate9On12Ex` |
| 22 | unnamed ordinal-only forward |
| 23 | unnamed ordinal-only forward |
| 24 | `Direct3DShaderValidatorCreate9` |
| 25 | `PSGPError` |
| 26 | `PSGPSampleTexture` |
| 27 | `D3DPERF_BeginEvent` |
| 28 | `D3DPERF_EndEvent` |
| 29 | `D3DPERF_GetStatus` |
| 30 | `D3DPERF_QueryRepeatFrame` |
| 31 | `D3DPERF_SetMarker` |
| 32 | `D3DPERF_SetOptions` |
| 33 | `D3DPERF_SetRegion` |
| 34 | `DebugSetLevel` |
| 35 | `DebugSetMute` |
| 36 | `Direct3D9EnableMaximizedWindowedModeShim` |
| 37 | `Direct3DCreate9` |
| 38 | `Direct3DCreate9Ex` |

The selector DLL has no import table. It resolves `kernel32`/`kernelbase` exports through the process PEB, then loads either the renamed Remix bridge or the system D3D9 DLL by absolute path.

Not validated here: live Windows runtime behavior with Multiverse Launcher, because the container cannot run Windows executables.
