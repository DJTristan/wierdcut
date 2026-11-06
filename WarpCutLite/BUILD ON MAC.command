#!/bin/bash
set -u
cd "$(dirname "$0")" || exit 1
printf '\n=== WarpCut Lite: build + install for this Mac ===\n\n'
if ! xcode-select -p >/dev/null 2>&1; then
  echo 'Xcode Command Line Tools are required. Starting Apple installer...'
  xcode-select --install
  echo 'After installation finishes, open this file again.'
  read -r -p 'Press Return to close...'
  exit 1
fi
if ! command -v cmake >/dev/null 2>&1; then
  echo 'CMake is missing. Install it from https://cmake.org/download/ (macOS .dmg), then rerun.'
  read -r -p 'Press Return to close...'
  exit 1
fi
if ! command -v git >/dev/null 2>&1; then
  echo 'Git is missing; it should come with Apple Command Line Tools.'
  read -r -p 'Press Return to close...'
  exit 1
fi
if cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build --config Release --parallel 4; then
  mkdir -p "$HOME/Library/Audio/Plug-Ins/VST3" "$HOME/Library/Audio/Plug-Ins/Components"
  VST3=$(find build -type d -name 'WarpCut Lite.vst3' -print -quit)
  AU=$(find build -type d -name 'WarpCut Lite.component' -print -quit)
  if [ -n "$VST3" ]; then ditto "$VST3" "$HOME/Library/Audio/Plug-Ins/VST3/WarpCut Lite.vst3"; echo 'Installed VST3.'; else echo 'VST3 output not found.'; fi
  if [ -n "$AU" ]; then ditto "$AU" "$HOME/Library/Audio/Plug-Ins/Components/WarpCut Lite.component"; echo 'Installed AU.'; else echo 'AU output not found.'; fi
  echo 'Build completed. Restart Ableton Live and rescan plugins.'
else
  echo 'Build failed. Copy the last 30 lines of Terminal output and share them for troubleshooting.'
fi
read -r -p 'Press Return to close this window...'
