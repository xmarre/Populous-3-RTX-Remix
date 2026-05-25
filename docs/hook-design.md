# D3D9 Selector Hook Design

## Symptom

RTX Remix can be loaded before the actual Populous game process starts, because `MultiverseLauncher.exe` runs from the same directory as the game and can see the same root `d3d9.dll`.

## Trigger

The standard RTX Remix install layout places NVIDIA's bridge interposer as:

```text
<game folder>\d3d9.dll
<game folder>\.trex\
```

That works for games launched directly, but it is not selective when a launcher process also runs from the same directory.

## Root cause

Windows DLL search order is process-local but directory-based. If a process loads `d3d9.dll`, Windows will prefer the local copy in the process/application directory before the system DirectX DLL. With the standard RTX Remix layout, both the launcher and the game are eligible to load the same Remix bridge DLL.

## Violated invariant

RTX Remix must be injected into the process that renders the game scene, not into the launcher UI process.

For this package the intended routing is:

```text
MultiverseLauncher.exe -> Windows system d3d9.dll
popTBM.exe             -> d3d9-remix.dll -> .trex runtime
D3DPopTB.exe           -> d3d9-remix.dll -> .trex runtime
popTB.exe              -> d3d9-remix.dll -> .trex runtime
```

## Fix

The repository contains selector shim source under `src/d3d9-remix-selector`. Building it writes the selector DLL to the repository root as `d3d9.dll`.

The official NVIDIA RTX Remix bridge DLL must be renamed to:

```text
d3d9-remix.dll
```

The shim exports the normal D3D9 entry points. On first use it checks the current process executable name:

- game executable: load and forward to `d3d9-remix.dll` from the game root.
- anything else, including `MultiverseLauncher.exe`: load and forward to `%SystemRoot%\System32\d3d9.dll`.

If the renamed Remix bridge is missing, or if it does not export a requested D3D9 entry point, the shim falls back to the system D3D9 DLL.

| Process | Selected D3D9 target |
| --- | --- |
| `MultiverseLauncher.exe` | `%SystemRoot%\System32\d3d9.dll` |
| `popTBM.exe` | `d3d9-remix.dll`, then system fallback |
| `D3DPopTB.exe` | `d3d9-remix.dll`, then system fallback |
| `popTB.exe` | `d3d9-remix.dll`, then system fallback |

The shim is 32-bit because RTX Remix Bridge targets 32-bit D3D9 games and Populous/Multiverse's game process is 32-bit.

## Remaining separate issue

This does not magically make the current Multiverse Direct3D9 renderer path Remix-compatible. It only fixes process selection.

If logs still show pre-transformed vertices and no valid camera, the next failure is renderer-state compatibility, not hook placement.
