#
# SPDX-FileCopyrightText: LineageOS
#
# SPDX-License-Identifier: Apache-2.0
#

# Inherit from those products. Most specific first.
$(call inherit-product, $(SRC_TARGET_DIR)/product/core_64_bit_only.mk)
$(call inherit-product, $(SRC_TARGET_DIR)/product/full_base_telephony.mk)

# Inherit from device makefile.
$(call inherit-product, device/nothing/Galaxian/device.mk)

# Inherit some common lineageOS stuff.
$(call inherit-product, vendor/lineage/config/common_full_phone.mk)

TARGET_BOOT_ANIMATION_RES := 1080

PRODUCT_NAME := lineage_Galaxian
PRODUCT_DEVICE := Galaxian
PRODUCT_MANUFACTURER := Nothing
PRODUCT_BRAND := Nothing
PRODUCT_MODEL := A001T

PRODUCT_GMS_CLIENTID_BASE := android-nothing

PRODUCT_BUILD_PROP_OVERRIDES += \
    DeviceName=Galaxian \
    BuildDesc="sys_mssi_64_64only_ww_armv82-user 15 AP3A.240905.015.A2 2607021815 release-keys" \
    BuildFingerprint=Nothing/Galaxian/Galaxian:16/BP2A.250605.031.A3/2608191839:user/release-keys

MISTOS_MAINTAINER := Samakshhhh
WITH_GMS := true
TARGET_ENABLE_BLUR := true
TARGET_DEFAULT_PIXEL_LAUNCHER := true

ro.mist.display=1080 x 2400, 120 hz
ro.mist.battery=5000mah
ro.mist.soc=Mediatek® Dimesnity 7300 Pro
ro.mist.camera=50MP + 8MP + 2MP
ro.mist.front=16MP
ro.mist.platform=MT6878
ro.mist.screen=6.67' AMOLED
ro.mist.device.name=Nothing Phone (3a) Lite
