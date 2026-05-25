# Populous 3 RTX Remix

RTX Remix setup files and notes for experimenting with NVIDIA RTX Remix in **Populous: The Beginning / Populous 3** using the Multiverse Launcher and the Direct3D9 renderer path.

This repository is meant to be copied into the Populous 3 game directory.

## Current status

RTX Remix can hook the game and open the Remix menu, but the current Multiverse `direct3d9` renderer path appears to expose pre-transformed/screen-space draw calls instead of a Remix-useful world-space 3D scene.

Observed runtime messages:

```text
[RTX-Compatibility-Info] Skipped drawcall, using pre-transformed vertices which isn't currently supported.
[RTX-Compatibility-Info] Trying to raytrace but not detecting a valid camera.
[RTX] CameraManager: rejected an invalid camera
```

Result: the RTX Remix UI works, but ray tracing does not materially affect the game scene yet. This is a renderer compatibility problem, not a DLSS/path-tracing/tonemapping setting problem.

## Install layout

Copy the repository contents into the Populous 3 folder, for example:

```text
D:\Spiele\Populous 3\
```

Expected game-side layout after copying this repository and installing the RTX Remix Runtime:

```text
D:\Spiele\Populous 3\d3d9.dll                  <-- RTX Remix Runtime, not stored in this repo
D:\Spiele\Populous 3\.trex\                    <-- RTX Remix Runtime, not stored in this repo
D:\Spiele\Populous 3\rtx.conf
D:\Spiele\Populous 3\rtx-remix\captures\
D:\Spiele\Populous 3\rtx-remix\logs\
D:\Spiele\Populous 3\rtx-remix\mods\
```

The NVIDIA RTX Remix Runtime files are intentionally not included in this repository:

```text
d3d9.dll
.trex\
```

Download those from the official RTX Remix Runtime release and copy them into the same folder as the Populous executable.

## Launcher settings used during testing

`ddraw.ini` observed configuration:

```ini
[poptbm]
fullscreen=True
windowed=False
renderer=direct3d9
width=800
```

For stability, test lower resolutions first and avoid exclusive fullscreen if the launcher exposes a windowed option.

## Known blocker

The current blocker is not installation. RTX Remix loads and the overlay opens. The blocker is that the renderer path appears to provide transformed vertices and no valid camera to RTX Remix.

Useful next tests are renderer-path tests:

1. Try every available Multiverse renderer mode and check the Remix logs for camera / transformed-vertex messages.
2. Try a native/original hardware renderer path if the launcher can expose one.
3. Try a different D3D wrapper chain only if it preserves world-space fixed-function D3D9 calls.
4. Capture a USD frame and inspect whether it contains a real `Camera` and meaningful `Mesh` entries.

## Runtime log checks

After running the game with RTX Remix, search the logs for compatibility messages:

```powershell
$GamePath = "D:\Spiele\Populous 3"

Select-String `
  -Path "$GamePath\rtx-remix\logs\remix-dxvk.log","$GamePath\rtx-remix\logs\bridge32.log","$GamePath\rtx-remix\logs\bridge64.log" `
  -Pattern 'RTX-Compatibility-Info|pre-transformed|valid camera|Skipped drawcall|raytrace|drawcall|camera|projection|view matrix|world matrix|fixed function|shader|geometry' `
  -CaseSensitive:$false |
  Select-Object Line |
  Format-List
```

A bad result looks like:

```text
Skipped drawcall, using pre-transformed vertices which isn't currently supported.
Trying to raytrace but not detecting a valid camera.
CameraManager: rejected an invalid camera.
```

## Files intentionally ignored

The repository ignores runtime binaries, game files, captures, logs, dumps, and local launcher state. Do not commit NVIDIA runtime binaries or Populous game files.
