# VeynFx native DSP

Vendor DSP effect for the Lunaris Dolby "VeynFx DSP" section, ported from
AxionOS AxionFx (`android_packages_apps_AxionFx`, branch `lineage-23.2`,
commit `b5c19b5730e454ea22439aadac17da9af6dad117`, Apache-2.0).

## What was renamed

`axionfx` -> `veynfx` everywhere in branding: namespace, classes, files,
module (`libveynfxaidl`), effect name and log tags. Untouched on purpose:

- Effect type/implementation UUIDs (the app controller addresses the
  effect by UUID — changing them breaks app compatibility for zero gain)
- Parameter IDs and value scaling (`VeynFxParams.h`, still protocol
  compatible with upstream)
- Upstream copyright headers and the `AxionOS` implementor string

## Build requirements (provided by the ROM tree)

- `aidlaudioeffectservice_defaults`, `:effectCommonFile`
  (`hardware/interfaces/audio/aidl/default`)
- `libpffft` (`external/pffft`)
- Steam Audio is NOT required: `effects/SteamSpatial.h` is a no-op stub
  (the phonon-based `SteamSpatial.cpp` stays on disk, excluded from the
  build). To restore real HRTF, vendor `libsteamaudio` + SDK headers and
  follow the re-enable steps in the stub header.

## Device integration (done in this tree)

- `configs/audio/audio_effects.xml`: `veynfx` library + effect entries
- `device.mk`: `PRODUCT_PACKAGES += libveynfxaidl`
- No sepolicy changes: vendor soundfx libs load under the existing
  audioserver rules (same as the DAP blobs)

## Updating from upstream

1. Fetch the newer upstream `native/` tree
2. Copy over this directory, re-apply the `AxionFx` -> `VeynFx` and
   `axionfx` -> `veynfx` renames (keeping `AxionOS` intact)
3. Re-check `VeynFxParams.h` against the app controller
   (`hardware/dolby/.../audio/VeynFxController.kt`) for new params
