# Populous 3 RTX Remix

RTX Remix setup files for experimenting with **Populous: The Beginning / Populous 3** through the **Multiverse Launcher**.

This repository is meant to be copied directly into the Populous 3 game directory. The repo now includes a small root `d3d9.dll` selector shim so the launcher itself does **not** load RTX Remix. Remix is only routed into the actual game executable.

## What is fixed here

The previous layout required NVIDIA's RTX Remix `d3d9.dll` to sit in the game root. Because `MultiverseLauncher.exe` also runs from that folder, the launcher could load RTX Remix before the game process existed.

The new layout is:

```text
D:\Spiele\Populous 3\d3d9.dll             <-- included selector shim from this repo
D:\Spiele\Populous 3\d3d9-remix.dll       <-- NVIDIA RTX Remix root d3d9.dll, renamed
D:\Spiele\Populous 3\.trex\               <-- NVIDIA RTX Remix runtime folder
D:\Spiele\Populous 3\rtx.conf
D:\Spiele\Populous 3\d3d9-selector.ini     <-- startup-device defer count
D:\Spiele\Populous 3\rtx-remix\captures\
D:\Spiele\Populous 3\rtx-remix\logs\
D:\Spiele\Populous 3\rtx-remix\mods\
```

Runtime behavior:

- `MultiverseLauncher.exe` gets the real system `d3d9.dll`.
- `popTBM.exe` gets system D3D9 for its first four D3D9 create calls by default, then `d3d9-remix.dll` for later create calls. This keeps Remix out of Multiverse startup/movie/final-blit devices that are torn down before the stable gameplay device. The count is configurable in `d3d9-selector.ini`.
- `D3DPopTB.exe` and `popTB.exe` get `d3d9-remix.dll`, which is the renamed NVIDIA RTX Remix bridge.
- If `d3d9-remix.dll` is missing, the selector falls back to the system `d3d9.dll` instead of crashing the launcher.

## Install

1. Copy this repository's contents into the Populous 3 directory. The root `d3d9.dll` from this repo must remain in place; it is the selector shim.
2. Download the official NVIDIA RTX Remix Runtime.
3. Copy the runtime `.trex` folder into the Populous 3 directory.
4. Rename the runtime's root `d3d9.dll` to:

```text
d3d9-remix.dll
```

5. Put that renamed file next to this repo's root `d3d9.dll` selector.
6. Start the game normally through `MultiverseLauncher.exe`.

Do **not** rename this repo's root `d3d9.dll`. Rename only NVIDIA's Runtime root `d3d9.dll` to `d3d9-remix.dll`. The root `d3d9.dll` from this repo is the process selector.

## Expected file check

From PowerShell:

```powershell
$GamePath = "D:\Spiele\Populous 3"
Get-Item "$GamePath\d3d9.dll", "$GamePath\d3d9-remix.dll", "$GamePath\.trex\d3d9.dll"
```

Expected meaning:

- `d3d9.dll`: selector shim from this repo.
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


## Startup-device defer setting

`popTBM.exe` creates short-lived D3D9 objects before the actual menu/game renderer is stable. RTX Remix has been observed to crash when attached to those temporary devices during teardown, and the latest trace showed call 4 is still a screen-space final-blit path. The selector therefore keeps Remix out of the first four `popTBM.exe` `Direct3DCreate9` / `Direct3DCreate9Ex` calls by default.

Config file:

```ini
[popTBM]
deferCreates=4
```

Values below `2` are promoted to `2`; values above `16` are clamped to `16`. Increase this value if the selector log still shows Remix attached before gameplay.

## Current D3D9 fixup

The hook problem is separate from the renderer-state problem.

RTX Remix can load into the game process, but the Multiverse `direct3d9` renderer path has been observed to expose pre-transformed/screen-space draw calls instead of a normal fixed-function world/view/projection path.

Observed runtime messages:

```text
[RTX-Compatibility-Info] Skipped drawcall, using pre-transformed vertices which isn't currently supported.
[RTX-Compatibility-Info] Trying to raytrace but not detecting a valid camera.
[RTX] CameraManager: rejected an invalid camera
```

The root `d3d9.dll` selector now includes a Populous-specific D3D9 proxy. In game processes it wraps the stable Remix-routed D3D9 device, forces an auto depth/stencil attachment with retry fallback, rewrites `D3DFVF_XYZRHW` draw state, and wraps vertex declarations using `D3DDECLUSAGE_POSITIONT` so Remix receives normal `POSITION` data plus a synthetic fixed-function orthographic camera path. See `docs/d3d9-rhw-fixup.md`.

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

It exports the normal D3D9 entry points, forwards them at runtime based on the current process name, and wraps the game D3D9 device to fix RHW/pre-transformed FVF draw state before Remix sees it.

## Files intentionally not included

NVIDIA RTX Remix Runtime files are not redistributed here:

```text
d3d9-remix.dll
.trex\
```

Populous game files are also not redistributed.

## If RTX Remix does not load

Check the root folder first. If the only D3D9 file is `d3d9-remix.dll`, Windows has no local `d3d9.dll` to load and Remix cannot start. The folder must contain both files at the same time:

```text
d3d9.dll          selector shim from this repo
d3d9-remix.dll    renamed NVIDIA RTX Remix bridge
.trex\            NVIDIA RTX Remix runtime folder
```

This repo intentionally commits the selector `d3d9.dll`; NVIDIA runtime binaries remain ignored.

## Current Multiverse D3D9 crash handling

The selector intentionally keeps RTX Remix off the first transient Multiverse D3D9 devices. Those devices are used for intro/video/menu transitions and are destroyed before gameplay; attaching RTX Remix to them can crash the bridge during teardown.

Default `d3d9-selector.ini` now uses:

```ini
[popTBM]
deferCreates=4
forceWindowedForRemix=1
enableRhwFixup=0
forceAutoDepthStencilForRemix=0
promoteSystemDeviceWithAutoDepth=1
traceD3D9Stream=1
traceRenderStates=0
traceDrawLimit=2048
```

The selector writes `d3d9-selector.log` next to `d3d9.dll`. If the game still crashes, check whether the log shows `backend=remix` before the gameplay device. Increase `deferCreates` by one if Remix is still attached to the menu device.

If the game runs but Remix has no visible effect, check for these new stream-repair lines:

```text
selector: streamRepair d3dCreateCall=... kind=FVF_XYZRHW
selector: streamRepair d3dCreateCall=... kind=POSITIONT_DECL
```

See `docs/d3d9-rhw-fixup.md` for the D3D9 stream repair path.

## Current D3D9 status

The Multiverse D3D9 renderer is now reached after the unstable launcher/menu devices. The latest trace showed that Direct3DCreate9 call 4 is a screen-space final-blit path (`D3DFVF_XYZRHW | TEX1`, triangle-strip quads, depth/lighting off), so this package defaults to `deferCreates=4` and starts RTX Remix at the next D3D9 device.

If gameplay still logs only `FINAL_BLIT_RHW`, Remix is attached to a 2D presentation stream, not to Populous world geometry. In that case the remaining work is not another `rtx.conf` tweak; the missing layer is the actual world-space renderer/camera data before Multiverse/cnc-ddraw collapses it into a final frame.
