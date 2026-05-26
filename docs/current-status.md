# Current status: Multiverse D3D9 path

The selector now routes `MultiverseLauncher.exe` to system D3D9 and delays RTX Remix for `popTBM.exe` until after the unstable intro/menu D3D9 devices.

The latest passive trace showed that the first Remix-routed device was not real world geometry. It submitted:

```text
FVF = 0x104 = D3DFVF_XYZRHW | TEX1
DrawPrimitive primitiveType=TRIANGLESTRIP primitiveCount=2
ZENABLE = 0
LIGHTING = 0
ALPHABLENDENABLE = 1
```

That is a screen-space textured quad / final presentation blit. RTX Remix can load there, but it cannot infer Populous world geometry, camera, materials, or lights from a final 2D frame.

This package therefore changes the default from `deferCreates=3` to `deferCreates=4` so Remix skips that menu/presentation device and starts at the next D3D9 create call. That value is the packaged INI default and the compiled fallback when no INI override is present. It also keeps stream tracing passive and classifies repeated RHW quads as `FINAL_BLIT_RHW` instead of mutating them.

If later gameplay devices still only log `FINAL_BLIT_RHW`, the Multiverse D3D9 renderer is a final blit path, not a Remix-compatible world-geometry path. A D3D9 DLL shim cannot reconstruct the missing world transform/camera from that stream.
