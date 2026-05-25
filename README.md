# Populous 3 RTX Remix

RTX Remix setup files for experimenting with **Populous: The Beginning / Populous 3** through the **Multiverse Launcher**.

This repository is meant to be copied directly into the Populous 3 game directory. The repo now includes source for a small `d3d9.dll` selector shim so the launcher itself does **not** load RTX Remix. Remix is only routed into the actual game executable.

## What is fixed here

The previous layout required NVIDIA's RTX Remix `d3d9.dll` to sit in the game root. Because `MultiverseLauncher.exe` also runs from that folder, the launcher could load RTX Remix before the game process existed.

The new layout is:

```text
D:\Spiele\Populous 3\d3d9.dll             <-- selector shim built from this repo
D:\Spiele\Populous 3\d3d9-remix.dll       <-- NVIDIA RTX Remix root d3d9.dll, renamed
D:\Spiele\Populous 3\.trex\               <-- NVIDIA RTX Remix runtime folder
D:\Spiele\Populous 3\rtx.conf
D:\Spiele\Populous 3\rtx-remix\captures\
D:\Spiele\Populous 3\rtx-remix\logs\
D:\Spiele\Populous 3\rtx-remix\mods\
```

Runtime behavior:

- `MultiverseLauncher.exe` gets the real system `d3d9.dll`.
- `popTBM.exe`, `D3DPopTB.exe`, and `popTB.exe` get `d3d9-remix.dll`, which is the renamed NVIDIA RTX Remix bridge.
- If `d3d9-remix.dll` is missing, the selector falls back to the system `d3d9.dll` instead of crashing the launcher.

## Install

1. Build the selector shim from `src\d3d9-remix-selector\build-selector.cmd`. The output is the repository root `d3d9.dll`.
2. Copy this repository's contents into the Populous 3 directory.
3. Download the official NVIDIA RTX Remix Runtime.
4. Copy the runtime `.trex` folder into the Populous 3 directory.
5. Rename the runtime's root `d3d9.dll` to:

```text
d3d9-remix.dll
```

6. Put that renamed file next to the built `d3d9.dll` selector.
7. Start the game normally through `MultiverseLauncher.exe`.

Do **not** overwrite the built root `d3d9.dll` with NVIDIA's root `d3d9.dll`. The root `d3d9.dll` from this repo is the process selector. NVIDIA's root bridge DLL must be named `d3d9-remix.dll`.

## Expected file check

From PowerShell:

```powershell
$GamePath = "D:\Spiele\Populous 3"
Get-Item "$GamePath\d3d9.dll", "$GamePath\d3d9-remix.dll", "$GamePath\.trex\d3d9.dll"
```

Expected meaning:

- `d3d9.dll`: selector shim built from this repo.
- `d3d9-remix.dll`: renamed NVIDIA RTX Remix bridge DLL.
- `.trex\d3d9.dll`: NVIDIA RTX Remix renderer/runtime DLL.

## Launcher settings used during testing

`ddraw.ini` observed configuration:

```ini
[poptbm]
fullscreen=True
windowed=False
renderer=direct3d9
width=800
```

The Multiverse Launcher FAQ documents DirectX 9 as the relevant renderer dependency and suggests switching renderer mode only as a workaround for DirectX 9 startup failure. For RTX Remix testing, keep the Direct3D9 path enabled.

## Current renderer blocker

The hook problem is separate from the renderer-compatibility problem.

RTX Remix can load into the game process, but the current Multiverse `direct3d9` renderer path appears to expose pre-transformed/screen-space draw calls instead of a Remix-useful world-space 3D scene.

Observed runtime messages:

```text
[RTX-Compatibility-Info] Skipped drawcall, using pre-transformed vertices which isn't currently supported.
[RTX-Compatibility-Info] Trying to raytrace but not detecting a valid camera.
[RTX] CameraManager: rejected an invalid camera
```

Result: the RTX Remix UI can work, but ray tracing may still not materially affect the game scene until a renderer path exposes valid camera/projection/world-space geometry.

## Runtime log checks

After running the game with RTX Remix, search the logs:

```powershell
$GamePath = "D:\Spiele\Populous 3"

Select-String `
  -Path "$GamePath\rtx-remix\logs\*.log", "$GamePath\*.log", "$GamePath\.trex\*.log" `
  -Pattern 'RTX-Compatibility-Info|pre-transformed|valid camera|Skipped drawcall|raytrace|drawcall|camera|projection|view matrix|world matrix|fixed function|shader|geometry|Remix Bridge|Welcome to NVIDIA Remix' `
  -CaseSensitive:$false |
  Select-Object Path, Line |
  Format-List
```

A bad renderer result looks like:

```text
Skipped drawcall, using pre-transformed vertices which isn't currently supported.
Trying to raytrace but not detecting a valid camera.
CameraManager: rejected an invalid camera.
```

## Hook source

The selector source is in:

```text
src/d3d9-remix-selector/
```

It exports the normal D3D9 entry points and forwards them at runtime based on the current process name.

## Files intentionally not included

NVIDIA RTX Remix Runtime files are not redistributed here:

```text
d3d9-remix.dll
.trex\
```

Populous game files are also not redistributed.
