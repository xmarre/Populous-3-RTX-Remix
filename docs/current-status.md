# Current status

RTX Remix is reaching `popTBM.exe`; the remaining crash in the latest logs is not a launcher-selection failure.

The bridge log shows Remix creating a D3D9Ex device during the black-screen/menu transition, then immediately sending destroy commands for the device, textures, swapchain, and D3D9 module. The 64-bit bridge then raises an access violation.

The current package therefore fixes the D3D9 presentation path first:

- `MultiverseLauncher.exe` still receives system D3D9.
- The first two `popTBM.exe` D3D9 create calls still receive system D3D9.
- Later `popTBM.exe` D3D9 create calls receive RTX Remix.
- Those Remix create calls are forced to windowed presentation parameters inside the selector before they reach `d3d9-remix.dll`.

The force-windowed rewrite is only on the Remix-selected `popTBM.exe` `CreateDevice`/`CreateDeviceEx` path. Launcher, deferred startup, and other system-D3D9 paths are left unchanged.

This replaces reliance on `.trex\bridge.conf` `client.forceWindowed`, because the submitted bridge log repeatedly reports that option as `Option failed to parse`.

The RHW/POSITIONT fixup remains present in source but is disabled by default through `d3d9-selector.ini`. Re-enable it only after the menu/gameplay device is stable.

The committed root `d3d9.dll` is the patched selector build corresponding to this source update. Future selector binary updates should be rebuilt with `src/d3d9-remix-selector/build-selector.cmd`.
