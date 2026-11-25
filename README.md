# Note - This is my personal tree and it might not be buildable at all times, please use trees from the [Org](https://github.com/Nothing-Phone-3a-Lite) if you face issues

## Device Tree for the Nothing Phone (3a) Lite [Galaxian]

The Nothing Phone (3a) Lite (codenamed _"Galaxian"_) is a mid-range smartphone from Nothing released in late 2025.

## Device specifications

Basic   | Spec Sheet
-------:|:-------------------------
CPU     | 4x ARM Cortex-A78 @ 2.50 GHz | 4x ARM Cortex-A55 @ 2.00 GHz
Chipset | Mediatek Dimensity 7300 Pro
GPU     | Mali-G615 MP2
Memory  | 8 GB RAM
Shipped Android Version | 15 (Currently tree based on NOS 4.1 v1508)
Storage | 128 / 256 GB
Battery | 5000 mAh
Display | 1084 x 2392 pixels, 6.77 inches, 60/120 hz

![Nothing Phone (3a) Lite](https://fdn2.gsmarena.com/vv/pics/nothing/nothing-phone-3a-lite-2.jpg)

## Required Repos
- `Vendor blobs`: <https://github.com/samakshkambxj/proprietary_vendor_nothing_Galaxian>
- `Kernel`: <https://github.com/samakshkambxj/device_nothing_Galaxian-kernel>
- `hardware/mediatek`: <https://github.com/samakshkambxj/hardware_mediatek>
- `Vendor sepolicy`: <https://github.com/Andreyka445/device_mediatek_sepolicy_vndr>
- `Dolby Atmos` : <https://github.com/samakshkambxj/hardware_dolby>

## Building

```bash
repo init -u https://github.com/LineageOS/android.git -b lineage-23.2
repo sync -c -j"$(nproc --all)"

source build/envsetup.sh
breakfast Galaxian
mka bacon
```

## Extracting blobs

With the device tree at `device/nothing/Galaxian` and a matching stock dump
(`system/`, `system_ext/`, `product/`, `vendor/`, `odm/`):

```bash
./extract-files.py /path/to/dump
./setup-makefiles.py
```
## Credits

- [LineageOS](https://github.com/LineageOS)
- [sm455](https://github.com/sm455) — bring-up for Galaga (Galaxian tree is based upon it)
- [Sujan Bimali](https://github.com/sujanbimali9) — Maintaining Galaga tree 
- [Andreyka445](https://github.com/Andreyka445) — original bring-up
- [Paranoid Android](https://github.com/AOSPA) — Glyph app
- [Nothing-2A](https://github.com/Nothing-2A) — reference device trees

Required repos💫:

Vendor tree: https://github.com/Andreyka445/vendor_nothing_galaxian

kernel tree: https://github.com/Andreyka445/android_device_nothing_Galaxian-kernel

hw/mtk: https://github.com/Andreyka445/hardware_mediatek

sepolicy: https://github.com/Andreyka445/device_mediatek_sepolicy_vndr

Happy Building🌜
```
Copyright (C) The LineageOS Project

 SPDX-License-Identifier: Apache-2.0
