# WarpCut Lite — source project (prototype v0.1)

An independent, **Warp Cut-inspired** real-time beat slicer for Ableton Live.

**Not a port of the original LiveCut algorithm.** Its groove and behaviour will differ from the classic. This first version provides live captured slices, random cut-length changes, repeats and phrase-end fills, with host BPM following. It deliberately contains **no bitcrusher and no comb filter**.

## Controls
- **Cut Chance**: likelihood of interrupting the live feed with a captured slice
- **Repeat Amount**: number of successive repeats before a new cut
- **Warp**: variability of lengths and rapid fills
- **Minimum Slice**: rhythmic grid, from 1/8 to 1/64
- **Cut Fade ms**: brief envelope at boundaries to reduce clicks
- **Mix**: wet/dry blend

## macOS build
Requires macOS, CMake 3.22+, Git, Xcode Command Line Tools, and internet access on the **first build** (CMake fetches JUCE 8.0.10). The build system makes **AU and VST3** formats for the Mac's current architecture.

Open Terminal and enter:

```sh
xcode-select --install
brew install cmake
cd /path/to/WarpCutLite
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j 4
```

`brew install cmake` assumes Homebrew is already installed; use cmake.org if not. `COPY_PLUGIN_AFTER_BUILD` attempts to install into your user plugin directories; if permissions prevent it, find the bundles under `build/WarpCutLite_artefacts/Release/` (or `build/WarpCutLite_artefacts/`) and manually copy them into `~/Library/Audio/Plug-Ins/VST3/` or `~/Library/Audio/Plug-Ins/Components/`. Rescan plug-ins in Ableton. Enable Audio Units if using AU.

## Limitations / next steps
- This package contains **source**, not a precompiled, tested macOS binary. It has not been built or auditioned on a Mac in this environment.
- Uses a generic JUCE controls editor; a custom visual interface can come later.
- The phrase generation, reverse, acceleration/ritard and transport phase-locking are not yet accurate recreations of classic Warp Cut.
- Sync is to **host BPM**, not currently to exact timeline bar boundaries.
- For a truly faithful LiveCut WarpCut mode, a better second step is to isolate and retain its GPL-licensed WarpCut algorithm from the existing open-source port: https://github.com/eventual-recluse/LiveCut
