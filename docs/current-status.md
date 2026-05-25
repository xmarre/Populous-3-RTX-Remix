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
<game>\d3d9.dll          selector shim built from this repo
<game>\d3d9-remix.dll    NVIDIA RTX Remix root bridge d3d9.dll, renamed
<game>\.trex\            NVIDIA RTX Remix runtime folder
```

The remaining known blocker is renderer compatibility, not launcher hook selection:

```text
[RTX-Compatibility-Info] Skipped drawcall, using pre-transformed vertices which isn't currently supported.
[RTX-Compatibility-Info] Trying to raytrace but not detecting a valid camera.
[RTX] CameraManager: rejected an invalid camera
```

That means Remix is not receiving a usable world-space 3D scene/camera from the current renderer path. Visual RTX settings therefore may still not materially change the game scene.
