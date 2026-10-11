# GD Physics Toolkit (Android prototype)

This is a **source starter**, not a tested ready-to-install `.geode` binary. It targets Geometry Dash Android 2.2081 and the Geode v4 toolchain configuration. I cannot claim the code compiles on your exact device until you build and test it.

## What this prototype attempts to implement
- FPS target setting through Cocos2d's animation interval (actual rendered FPS remains limited by device/display/game).
- Macro input recording from `PlayLayer::handleButton`, saved as CSV under Geode's mod save directory.
- Same-session macro replay by replaying recorded button events.
- Frame-step pause and a keyboard F6 single-update request (keyboard availability on Android depends on attached keyboard / input dispatch).

## Not implemented yet
- Physics bypass
- Noclip
- Show hitboxes
- Render/export replay as a video

Those features need version-specific hooks and testing. A replay CSV is not a video file. For video rendering, the safe practical workflow is to capture the replay on-device with a screen recorder, or use a desktop capture workflow. Do not treat the listed missing features as working toggles.

## Build on a computer
1. Install the Geode CLI and Android NDK. Follow the official Geode Android build instructions:
   https://docs.geode-sdk.org/getting-started/create-mod/
2. In a terminal, from this project directory, install Android 64-bit SDK binaries:
   `geode sdk install-binaries -p android64`
   For a 32-bit device use `android32` instead.
3. Build:
   `geode build -p android64`
   or `geode build -p android32`
4. If build succeeds, find the generated `.geode` package in `build-android64` or `build-android32`.
5. Copy the package to:
   `/storage/emulated/0/Android/media/com.geode.launcher/game/geode/mods/`
6. Restart Geometry Dash through Geode and check the Geode log for errors.

## Compatibility notes
- Geode's exact installed version must match the `geode` field in `mod.json`. If your installed CLI uses another SDK generation, run `geode sdk` / check its docs and update the metadata accordingly.
- The C++ bindings and input hooks are version-sensitive. If a compile error reports a hook signature mismatch, consult the current Geode bindings for GD 2.2081 and adjust the method declaration.
- Back up levels and data before testing mods. Test in practice/offline levels first.
