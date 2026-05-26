# D3D9 RHW Fixup Layer

## Symptom

RTX Remix loads into `popTBM.exe`, but gameplay still looks like the original raster renderer.

Observed log lines:

```text
[RTX-Compatibility-Info] Skipped drawcall, using pre-transformed vertices which isn't currently supported.
[RTX-Compatibility-Info] Trying to raytrace but not detecting a valid camera.
[RTX] CameraManager: rejected an invalid camera
```

## Trigger

The Multiverse `direct3d9` renderer reaches RTX Remix, but its draw stream uses already-transformed screen-space vertices instead of a normal fixed-function world/view/projection path.

In D3D9 terms this is the `D3DFVF_XYZRHW` path: vertex data contains `x`, `y`, `z`, and reciprocal homogeneous `w` after transform. RTX Remix cannot reconstruct a world-space camera from that state.

## Root cause

The earlier selector solved only process routing:

```text
MultiverseLauncher.exe -> system d3d9.dll
popTBM.exe             -> d3d9-remix.dll -> .trex runtime
```

That was necessary but not sufficient. The remaining issue is the shape of the D3D9 render stream inside `popTBM.exe`.

## Fix attempt in this package

The root `d3d9.dll` is still the launcher/game selector, but it now also wraps the D3D9 object returned to game processes.

For `popTBM.exe`, `D3DPopTB.exe`, and `popTB.exe`, the shim now intercepts:

- `Direct3DCreate9`
- `Direct3DCreate9Ex`
- `IDirect3D9::CreateDevice`
- `IDirect3D9Ex::CreateDeviceEx`
- `IDirect3DDevice9::SetFVF`
- `IDirect3DDevice9::DrawPrimitive*`
- `IDirect3DDevice9::DrawIndexedPrimitive*`
- viewport/reset/transform state relevant to the fixup

When the game sets an RHW FVF, the shim does not pass `D3DFVF_XYZRHW` through as-is. Instead it builds a vertex declaration that:

- exposes `x,y,z` as untransformed `POSITION`,
- ignores the RHW float at offset 12,
- keeps diffuse/specular/texture-coordinate offsets at their original positions,
- seeds world/view/projection with a synthetic orthographic camera matching the current viewport/backbuffer.

This is intentionally narrow. Non-RHW FVF draw calls are forwarded unchanged.

## Required runtime options

`rtx.conf` includes:

```text
rtx.orthographicIsUI = False
rtx.useVertexCapture = True
rtx.enableAlwaysCalculateAABB = True
```

`rtx.orthographicIsUI = False` is required because the synthetic camera is orthographic. If Remix treats orthographic draw calls as UI, the fixup can still be ignored by ray tracing.

## Expected log change

A successful first-stage result is not necessarily correct lighting yet. The first success criterion is that these messages are reduced or disappear during gameplay:

```text
Skipped drawcall, using pre-transformed vertices which isn't currently supported.
Trying to raytrace but not detecting a valid camera.
```

If they remain unchanged, the renderer is probably not using FVF `D3DFVF_XYZRHW`; it may be using vertex declarations with `D3DDECLUSAGE_POSITIONT`, shaders, or a final framebuffer quad. That needs the next interception point.

## Regression risk

This is a compatibility shim, not a general D3D9 wrapper. It is scoped to the Populous game executables only. The launcher remains routed to system D3D9.

Possible failure modes:

- crash on device creation if the game depends on unwrapped QueryInterface behavior,
- distorted geometry if the renderer uses RHW data for more than screen-space placement,
- no visible change if the renderer submits only a final full-screen composited texture,
- Remix still ignoring content if the runtime classifies the resulting orthographic content as UI despite config.
