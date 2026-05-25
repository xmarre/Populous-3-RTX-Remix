# Current Status

RTX Remix hooks Populous 3 through the Multiverse Launcher Direct3D9 renderer path and the Remix overlay opens.

The active blocker is renderer compatibility:

```text
[RTX-Compatibility-Info] Skipped drawcall, using pre-transformed vertices which isn't currently supported.
[RTX-Compatibility-Info] Trying to raytrace but not detecting a valid camera.
[RTX] CameraManager: rejected an invalid camera
```

This means Remix is not receiving a usable world-space 3D scene/camera. Visual RTX settings therefore do not materially change the game scene.

The next useful work is finding a renderer path that avoids pre-transformed vertices and exposes a valid camera/projection state to RTX Remix.
