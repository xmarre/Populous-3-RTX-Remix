# Hardware-fix Dd7to9 route

The Multiverse D3D9 backend has been proven to expose only final screen-space
quads to RTX Remix. The selector trace shows:

```text
FVF_XYZRHW a=0x00000104
FINAL_BLIT_RHW a=0x00000005 b=0x00000002
```

That is `D3DFVF_XYZRHW | TEX1` with `D3DPT_TRIANGLESTRIP`, two primitives: one
screen-space textured quad. It is already the rendered frame, not the Populous
3D scene.

The viable non-engine-rewrite route is therefore:

```text
Windows 10/11 Populous hardware fix executable
  -> old DirectDraw/Direct3D renderer
  -> dxwrapper Dd7to9
  -> selector d3d9.dll
  -> RTX Remix d3d9-remix.dll
```

This package does not assume the hardware-fix installer creates a new runtime
executable. The selector continues to recognize the existing Populous runtime
processes (`D3DPopTB.exe` and `popTB.exe`) and routes them to RTX Remix.

The package does not include the fixed game executable, dxwrapper binaries, or
NVIDIA RTX Remix Runtime binaries.
