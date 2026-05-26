# Hardware-fix + dxwrapper Dd7to9 route

The Multiverse `direct3d9` renderer is a presentation backend. The selector log
proved it draws the visible frame as `D3DFVF_XYZRHW | TEX1` triangle-strip quads,
which is a final screen-space blit. RTX Remix can load there, but it cannot see
Populous world geometry, depth, camera, or material state.

This route uses the Populous Windows 10/11 hardware-mode fixed executable instead
of the Multiverse software/cnc-ddraw presentation path, then converts its old
DirectDraw/Direct3D 1-7 renderer to D3D9 through dxwrapper `Dd7to9` before RTX
Remix receives the calls.

Required external files, not included here:

1. The Populous Windows 10 Hardware (D3D) Fix executable from thebeginning.uk.
   Run the installer against the game folder. This route does not assume it
   creates a new runtime executable.
2. dxwrapper release files:
   - `ddraw.dll` from dxwrapper's Stub folder
   - `dxwrapper.dll`
   - `dxwrapper.ini` from this folder copied to the game root
3. NVIDIA RTX Remix Runtime:
   - root NVIDIA `d3d9.dll` renamed to `d3d9-remix.dll`
   - `.trex\` copied beside it
4. This repo's selector `d3d9.dll` in the game root.

Expected chain:

```text
D3DPopTB.exe or popTB.exe
  -> ddraw.dll / dxwrapper
  -> Dd7to9
  -> local d3d9.dll selector
  -> d3d9-remix.dll
  -> .trex\d3d9.dll
```

Do not use the Multiverse `direct3d9` renderer for the actual RTX Remix capture
attempt. Its visible D3D9 stream is already a final 2D frame.

First test:

1. Delete old logs: `d3d9-selector.log`, `dxwrapper.log`, `bridge32.log`,
   `bridge64.log`, `remix-dxvk.log`.
2. Run the hardware-mode Populous runtime directly (`D3DPopTB.exe` or
   `popTB.exe`, depending on the installed game layout).
3. Check `d3d9-selector.log`.

Good sign:

```text
selector: session build=hardware-dx7to9-route ... hardwareExeSupport=1
selector: Direct3DCreate9 call=1 deferCreates=3 backend=remix
```

Bad sign:

```text
FINAL_BLIT_RHW
```

If the hardware-fix route still reports only final blit quads, dxwrapper is also
receiving a collapsed framebuffer and RTX Remix cannot work from this API layer.
