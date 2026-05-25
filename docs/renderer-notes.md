# Renderer Notes

Observed game configuration:

```ini
[poptbm]
singlecpu=false
accuratetimers=false
fullscreen=True
windowed=False
renderer=direct3d9
handlemouse=false
width=800
```

Observed problem messages in RTX Remix logs:

```text
Skipped drawcall, using pre-transformed vertices which isn't currently supported.
Trying to raytrace but not detecting a valid camera.
CameraManager: rejected an invalid camera.
```

Interpretation:

- RTX Remix is hooked correctly.
- The current D3D9 path is not providing a Remix-useful camera/scene.
- Ray tracing cannot reconstruct the world from already transformed/screen-space draw calls.
- UI/HUD visibility issues with ray tracing enabled are secondary to the missing valid scene/camera.

Potential next tests:

1. Different Multiverse renderer modes.
2. Original hardware renderer path if exposed by launcher.
3. Alternative D3D wrapper chain that preserves fixed-function world transforms.
4. USD capture inspection for `Camera` and meaningful `Mesh` entries.
