# Current status

The selector/device-routing part is working:

- `MultiverseLauncher.exe` receives system D3D9.
- the first three `popTBM.exe` D3D9 creates stay on system D3D9.
- later `popTBM.exe` D3D9 creates route to `d3d9-remix.dll` and RTX Remix starts.
- forced windowed mode prevents the menu/startup bridge teardown crash.

The remaining blocker is the draw stream. The RHW rewrite experiment proved that
Multiverse uses `D3DFVF_XYZRHW`, but mutating that stream into a fake normal
`POSITION` declaration makes the menu black. That changed presentation state, not
just Remix reconstruction state, so it is disabled again.

This build is passive tracing only: it logs transformed-position FVF/declaration
usage, selected render states, and RHW draw calls while forwarding the original
game stream unchanged.
