/*
 * Copyright (C) 2021-2024 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.settings.doze

import android.content.Context
import android.content.Intent
import android.hardware.Sensor
import android.hardware.SensorManager
import android.hardware.display.AmbientDisplayConfiguration
import android.os.UserHandle
import android.provider.Settings
import android.provider.Settings.Secure.DOZE_ALWAYS_ON
import android.provider.Settings.Secure.DOZE_ENABLED
import android.util.Log
import androidx.preference.PreferenceManager

object Utils {
    private const val TAG = "DozeUtils"

    private const val DOZE_INTENT = "com.android.systemui.doze.pulse"

    const val ALWAYS_ON_DISPLAY = "always_on_display"
    const val DOZE_ENABLE = "doze_enable"

    const val CATEGORY_PICKUP_SENSOR = "pickup_sensor"
    const val CATEGORY_PROXIMITY_SENSOR = "proximity_sensor"

    const val GESTURE_PICK_UP_KEY = "gesture_pick_up_type"
    const val GESTURE_POCKET_KEY = "gesture_pocket"

    // Upstream defaults the pickup gesture to "Do not check" (0). Lift to wake is
    // expected to work out of the box on this device, so default to "Wake device"
    // (2); it can still be changed in the ambient display settings.
    private const val DEFAULT_PICK_UP_TYPE = "2"

    // Upstream defaults the pocket gesture to off. Pocket mode (touch suppression
    // in a pocket, plus a pulse when the device leaves it) is a standard Nothing
    // feature, so enable it out of the box; it can still be turned off in the
    // ambient display settings.
    private const val DEFAULT_POCKET_ENABLED = true

    private fun startService(context: Context) {
        Log.d(TAG, "Starting service")
        context.startServiceAsUser(Intent(context, DozeService::class.java), UserHandle.CURRENT)
    }

    private fun stopService(context: Context) {
        Log.d(TAG, "Stopping service")
        context.stopServiceAsUser(Intent(context, DozeService::class.java), UserHandle.CURRENT)
    }

    fun checkDozeService(context: Context) {
        if (isDozeEnabled(context) && !isAlwaysOnEnabled(context) && areGesturesEnabled(context)) {
            startService(context)
        } else {
            stopService(context)
        }
    }

    fun isDozeEnabled(context: Context): Boolean {
        return Settings.Secure.getInt(context.contentResolver, DOZE_ENABLED, 1) != 0
    }

    fun enableDoze(context: Context, enable: Boolean): Boolean {
        return Settings.Secure.putInt(context.contentResolver, DOZE_ENABLED, if (enable) 1 else 0)
    }

    fun launchDozePulse(context: Context) {
        Log.d(TAG, "Launch doze pulse")
        context.sendBroadcastAsUser(Intent(DOZE_INTENT), UserHandle(UserHandle.USER_CURRENT))
    }

    fun enableAlwaysOn(context: Context, enable: Boolean): Boolean {
        return Settings.Secure.putIntForUser(
            context.contentResolver,
            DOZE_ALWAYS_ON,
            if (enable) 1 else 0,
            UserHandle.USER_CURRENT,
        )
    }

    fun isAlwaysOnEnabled(context: Context): Boolean {
        return Settings.Secure.getIntForUser(
            context.contentResolver,
            DOZE_ALWAYS_ON,
            0,
            UserHandle.USER_CURRENT,
        ) != 0
    }

    fun alwaysOnDisplayAvailable(context: Context): Boolean {
        return AmbientDisplayConfiguration(context).alwaysOnAvailable()
    }

    private fun isGestureEnabled(context: Context, gesture: String?, defValue: Boolean): Boolean {
        return PreferenceManager.getDefaultSharedPreferences(context).getBoolean(gesture, defValue)
    }

    fun isPickUpEnabled(context: Context): Boolean {
        return PreferenceManager.getDefaultSharedPreferences(context)
            .getString(GESTURE_PICK_UP_KEY, DEFAULT_PICK_UP_TYPE) != "0"
    }

    fun isPickUpSetToWake(context: Context): Boolean {
        return PreferenceManager.getDefaultSharedPreferences(context)
            .getString(GESTURE_PICK_UP_KEY, DEFAULT_PICK_UP_TYPE) == "2"
    }

    fun isPocketEnabled(context: Context): Boolean {
        return isGestureEnabled(context, GESTURE_POCKET_KEY, DEFAULT_POCKET_ENABLED)
    }

    private fun areGesturesEnabled(context: Context): Boolean {
        return isPickUpEnabled(context) || isPocketEnabled(context)
    }

    fun getSensor(sm: SensorManager, type: String?): Sensor? {
        return sm.getSensorList(Sensor.TYPE_ALL).find { it.stringType == type }
    }
}
