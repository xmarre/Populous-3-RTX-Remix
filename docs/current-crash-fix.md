# Current crash fix: keep Remix off transient menu devices

## Observed failure

The run that reached the menu and then crashed when starting a new game means the launcher/process hook is working, but RTX Remix is attached too early.

The bridge crash pattern is consistent across the supplied traces:

- `popTBM.exe` loads the RTX Remix bridge.
- A D3D9Ex device is created successfully.
- Populous/Multiverse destroys that device/module during the transition to the next phase.
- The 64-bit `NvRemixBridge.exe` process crashes during the destroy sequence.

The key invariant is now:

> RTX Remix must not own a D3D9 device that Populous/Multiverse later destroys during intro/video/menu transitions.

The menu is still a transient renderer phase. Attaching Remix there is not useful and makes the menu-to-game transition unsafe.

## Fix strategy

The selector now keeps the first three `Direct3DCreate9` / `Direct3DCreate9Ex` interface creations on the real system D3D9 runtime. This keeps intro/video/menu devices outside RTX Remix.

Default configuration:

```ini
[popTBM]
deferCreates=3
forceWindowedForRemix=1
enableRhwFixup=0
promoteSystemDeviceWithAutoDepth=1
```

`promoteSystemDeviceWithAutoDepth=1` is the important new part. If a deferred system-backed D3D9 interface later creates what looks like the real 3D device (`EnableAutoDepthStencil=TRUE`), the selector promotes that specific create path to RTX Remix on demand. This avoids relying only on raw `Direct3DCreate9` call count.

## Selector log

The selector now writes:

```text
d3d9-selector.log
```

next to `d3d9.dll`.

Useful lines:

```text
selector: Direct3DCreate9Ex call=3 deferCreates=3 backend=system
selector: CreateDeviceEx d3dCreateCall=3 backend=system ... depth=0 ...
selector: promote d3dCreateCall=3 reason=autoDepth result=remix
selector: CreateDeviceEx d3dCreateCall=3 backend=remix ... depth=1 ...
```

If a crash still happens, this log identifies whether Remix was still attached to the menu device or whether the first real gameplay device itself is crashing inside Remix.

## Tuning

If the log still shows Remix attached before gameplay:

```ini
deferCreates=4
```

If gameplay loads but the windowed mode is unwanted, try:

```ini
forceWindowedForRemix=0
```

Only do that after the bridge survives starting a new game.

`enableRhwFixup` stays disabled until device lifetime is stable. The RHW/pre-transformed-vertex problem is a later rendering issue, not the current crash root.
