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
$(call inherit-product, vendor/infinity/config/common_full_phone.mk)

TARGET_BOOT_ANIMATION_RES := 1080

PRODUCT_NAME := infinity_Galaxian
PRODUCT_DEVICE := Galaxian
PRODUCT_MANUFACTURER := Nothing
PRODUCT_BRAND := Nothing
PRODUCT_MODEL := A001T

PRODUCT_GMS_CLIENTID_BASE := android-nothing

PRODUCT_BUILD_PROP_OVERRIDES += \
    DeviceName=Galaxian \
    BuildDesc="sys_mssi_64_64only_ww_armv82-user 15 AP3A.240905.015.A2 2607021815 release-keys" \
    BuildFingerprint=Nothing/Galaxian/Galaxian:16/BP2A.250605.031.A3/2607021815:user/release-keys

# Maintainer Name
INFINITY_MAINTAINER := "Samakshhhh"

# Whether the device supports Fingerprint On Display
TARGET_HAS_UDFPS := true

# Whether Including Google Apps
WITH_GAPPS := true

ro.product.marketname=Nothing Phone (3a) Lite
ro.infinity.soc=Mediatek Dimensity 7300 Pro
ro.infinity.camera=50MP + 8MP + 8MP + 2MP
