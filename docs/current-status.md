# Current Status

The package now includes source for a D3D9 selector shim so RTX Remix is not loaded by `MultiverseLauncher.exe`.

Expected process routing:

```text
MultiverseLauncher.exe -> system d3d9.dll
popTBM.exe             -> d3d9-remix.dll -> .trex runtime
D3DPopTB.exe           -> d3d9-remix.dll -> .trex runtime
popTB.exe              -> d3d9-remix.dll -> .trex runtime
```

Install requirement:

```text
<game>\d3d9.dll          selector shim from this repo
<game>\d3d9-remix.dll    NVIDIA RTX Remix root bridge d3d9.dll, renamed
<game>\.trex\            NVIDIA RTX Remix runtime folder
```

The remaining known blocker is renderer state, not launcher hook selection. This package now includes a first-stage D3D9 RHW fixup for:

```text
[RTX-Compatibility-Info] Skipped drawcall, using pre-transformed vertices which isn't currently supported.
[RTX-Compatibility-Info] Trying to raytrace but not detecting a valid camera.
[RTX] CameraManager: rejected an invalid camera
```

The new D3D9 proxy tries to turn RHW FVF draw calls into fixed-function-position draw calls with a synthetic orthographic camera. This should be tested only with the Multiverse `direct3d9` renderer path first.

The previous source zip was missing the committed root `d3d9.dll`, so renaming NVIDIA's `d3d9.dll` to `d3d9-remix.dll` left no local D3D9 entry point for Windows to load. This version commits the selector binary and keeps NVIDIA runtime binaries ignored.
