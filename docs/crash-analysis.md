# Crash analysis: popTBM.exe menu transition

Observed logs from `bridge32.log`, `bridge64.log`, and `remix-dxvk.log` show that the selector hook is working: RTX Remix loads into `popTBM.exe`, the x64 bridge server starts, `.trex\d3d9.dll` is loaded, and DXVK Remix initializes.

The crash occurs later during the first D3D9 device/module teardown:

```text
[bridge32] Creating a thread-safe D3D9 device.
[bridge64] Server side D3D9 Device created successfully!
[bridge32] Restoring display mode...
[bridge32] Creating a thread-safe D3D9 device.
[bridge64] D3D9 Module destroyed.
Exception 0xc0000005 at ... inside .trex\d3d9.dll
```

The minidump places the access violation inside NVIDIA's 64-bit runtime DLL:

```text
module: D:\Spiele\Populous 3\.trex\d3d9.dll
fault offset: 0x3469d7
exception: 0xc0000005, write to 0x1
```

This is not the launcher accidentally loading Remix anymore. The bridge reaches the game process and then dies when `popTBM.exe` destroys the first startup D3D9 object during the transition into the menu.

## Current workaround in the selector

For `popTBM.exe` only, the selector now sends the first `Direct3DCreate9` / `Direct3DCreate9Ex` call to the system D3D9 runtime. The second and later D3D9 create calls are routed to `d3d9-remix.dll`.

This keeps Remix out of the unstable startup/menu-transition D3D9 object lifetime and lets Remix attach to the later game/menu renderer instead.

Other executables are unchanged:

```text
MultiverseLauncher.exe -> system d3d9.dll
popTBM.exe             -> first D3D9 create: system d3d9.dll; later creates: d3d9-remix.dll
D3DPopTB.exe           -> d3d9-remix.dll
popTB.exe              -> d3d9-remix.dll
```

## Remaining uncertainty

This package was validated structurally, but not by running the Windows game. If `popTBM.exe` creates only one D3D9 object on a different install or launch path, this workaround will skip Remix for that run. In that case the defer count should be changed to zero or the selector should be made config-driven.


## 2026-05-26 startup/movie-device crash after RHW fixup

Symptom: black screen during the first video while audio continues briefly, followed by an RTX Remix bridge crash.

Observed bridge sequence: Remix initializes in `popTBM.exe`, creates a D3D9 device, then the game tears down the D3D9 module/device during startup/movie transition. The server-side bridge logs the D3D9 module destruction immediately before the access violation.

Root cause handled here: the RHW proxy was being allowed to attach to the same temporary startup/movie D3D9 devices that are destroyed before the real game/menu renderer is stable. Those devices should not be modified by the RHW fixup at all.

Fix: `popTBM.exe` now defers Remix for the first configurable number of `Direct3DCreate9` / `Direct3DCreate9Ex` calls. Default: `deferCreates=3` in `d3d9-selector.ini`. The selector-owned RHW vertex declaration is also released before the underlying device reaches refcount zero to avoid late child-resource destruction during bridge module teardown.
