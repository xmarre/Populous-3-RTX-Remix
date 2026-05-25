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

- The selector hook can route Remix into the game executable instead of the launcher.
- The current Direct3D9 renderer path can still be incompatible with RTX Remix scene reconstruction.
- Pre-transformed/screen-space draw calls do not provide the world-space camera and geometry state Remix needs for path tracing.
- UI/HUD visibility issues with ray tracing enabled are secondary to the missing valid scene/camera.

Potential next tests:

1. Try every available Multiverse renderer mode that still produces Direct3D9 calls.
2. Try the original hardware renderer path if exposed by the launcher.
3. Try a DirectDraw-to-D3D9 wrapper path only if it preserves enough fixed-function world/view/projection state for Remix.
4. Capture a USD frame and inspect whether it contains a real `Camera` and meaningful `Mesh` entries.
