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

Portable Zig build used for the current committed selector:

```text
ZIG_GLOBAL_CACHE_DIR=/tmp/zig-global-cache \
ZIG_LOCAL_CACHE_DIR=/tmp/zig-local-cache \
/tmp/zig-x86_64-linux-0.15.1/zig cc \
  -target x86-windows-gnu \
  -O2 \
  -shared \
  -nostdlib \
  -fno-stack-protector \
  -fno-sanitize=undefined \
  -Wno-ignored-attributes \
  src/d3d9-remix-selector/d3d9_remix_selector.c \
  src/d3d9-remix-selector/d3d9_remix_selector.def \
  -Wl,-e,DllMain@12 \
  -Wl,--subsystem,windows \
  -o d3d9.dll
```

Expected result:

```text
d3d9.dll: PE32 executable for MS Windows 6.00 (DLL), Intel i386
```

Export table contains:

```text
Legacy ordinal aliases 1-14
Current Windows D3D9/D3D9On12 ordinal range 16-38
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
The selector also includes legacy ordinal-only aliases 1 through 14 for older D3D9 import tables.

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

SHA-256 of the included selector:

```text
688b0e8157a4d5aaaeaac319fd8e1008ae2dc7125f2eccdaa5a03aa0217307bd  d3d9.dll
```

## Selector binary included

The release archive must contain a root-level `d3d9.dll`. If it does not, RTX Remix cannot load after NVIDIA's bridge has been renamed to `d3d9-remix.dll`.

Install checklist:

```text
<game>\d3d9.dll          selector shim from this repo
<game>\d3d9-remix.dll    renamed NVIDIA RTX Remix bridge
<game>\.trex\            NVIDIA RTX Remix runtime folder
```

Not validated here: live Windows runtime behavior with Multiverse Launcher, because the container cannot run Windows executables.

## Current source validation note

This branch changes the selector source and export manifest. The root
`d3d9.dll` has been rebuilt from that source with the portable Zig command
above.

Do not package this branch as a runtime-ready release until the checks below
have been repeated against the rebuilt DLL in the target game environment.

Static routing audit for the current source:

| Process | First `Direct3DCreate9` / `Direct3DCreate9Ex` | Later create calls | Wrapped for RHW fixup |
| --- | --- | --- | --- |
| `MultiverseLauncher.exe` | system D3D9 | system D3D9 | no |
| `popTBM.exe` | system D3D9 | `d3d9-remix.dll` if available, otherwise system D3D9 | only when the selected module is `d3d9-remix.dll` |
| `D3DPopTB.exe` | `d3d9-remix.dll` if available, otherwise system D3D9 | `d3d9-remix.dll` if available, otherwise system D3D9 | only when the selected module is `d3d9-remix.dll` |
| `popTB.exe` | `d3d9-remix.dll` if available, otherwise system D3D9 | `d3d9-remix.dll` if available, otherwise system D3D9 | only when the selected module is `d3d9-remix.dll` |

The create entry points call `select_d3d9_for_create()` directly instead of the
generic export cache. That preserves the `popTBM.exe` first-create deferral and
prevents a first system-D3D9 resolution from pinning later create calls to the
system DLL. If Remix is missing or the selected module cannot provide the create
export, the fallback path resolves system D3D9 and returns it unwrapped.

Required runtime release checks:

1. Start `MultiverseLauncher.exe` and confirm it loads system D3D9, not Remix.
2. Start `popTBM.exe` and confirm the first D3D9 create uses system D3D9.
3. Confirm the next `popTBM.exe` D3D9 create uses `d3d9-remix.dll` and returns a wrapped D3D9 object.
4. Confirm `D3DPopTB.exe` and `popTB.exe` create calls use `d3d9-remix.dll` and return wrapped D3D9 objects.
5. Temporarily remove or rename `d3d9-remix.dll` and confirm all game create calls fall back to unwrapped system D3D9.
6. In the Multiverse `direct3d9` renderer path, confirm RHW/pre-transformed draw warnings are reduced or identify the next interception point.
