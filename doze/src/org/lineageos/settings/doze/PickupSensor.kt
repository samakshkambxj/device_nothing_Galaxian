/*
 * Copyright (C) 2021-2025 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.settings.doze

import android.content.Context
import android.hardware.Sensor
import android.hardware.SensorEvent
import android.hardware.SensorEventListener
import android.hardware.SensorManager
import android.os.PowerManager
import android.os.SystemClock
import android.util.Log
import android.view.Display
import java.util.concurrent.Executors

class PickupSensor(
    private val context: Context,
    sensorType: String,
    private val sensorValue: Float,
) : SensorEventListener {
    private val powerManager = context.getSystemService(PowerManager::class.java)!!
    private val wakeLock = powerManager.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, TAG)

    private val sensorManager = context.getSystemService(SensorManager::class.java)!!
    private val sensor = Utils.getSensor(sensorManager, sensorType)

    private val executorService = Executors.newSingleThreadExecutor()
    private var entryTimestamp = 0L
    private var wasTilted = false

    override fun onSensorChanged(event: SensorEvent) {
        if (DEBUG) Log.d(TAG, "Got sensor event: ${event.values[0]}")
        val value = event.values[0]
        // android.sensor.screen_upward reports 1 while the device is tilted/resting
        // and 0 once the screen ends up facing upward, so a pickup is the 1 -> 0
        // transition; a lone 0 event (e.g. the initial value on registering) must
        // not wake the device.
        if (value == TILTED_VALUE) {
            wasTilted = true
        } else if (value == sensorValue) {
            if (wasTilted) {
                wasTilted = false
                val delta = SystemClock.elapsedRealtime() - entryTimestamp
                if (delta >= MIN_PULSE_INTERVAL_MS) {
                    entryTimestamp = SystemClock.elapsedRealtime()
                    if (Utils.isPickUpSetToWake(context)) {
                        wakeLock.acquire(WAKELOCK_TIMEOUT_MS)
                        // wakeUpWithProximityCheck() only exists in trees carrying the
                        // LineageOS proximity-check patch, which this framework does not.
                        // The check is not needed here anyway: screen_upward only reports
                        // the wake value once the screen faces upward, i.e. not pocketed.
                        powerManager.wakeUp(
                            SystemClock.uptimeMillis(),
                            PowerManager.WAKE_REASON_GESTURE,
                            TAG,
                            Display.DEFAULT_DISPLAY,
                        )
                    } else {
                        Utils.launchDozePulse(context)
                    }
                }
            }
        }
    }

    override fun onAccuracyChanged(sensor: Sensor, accuracy: Int) {}

    fun enable() {
        if (sensor != null) {
            Log.d(TAG, "Enabling")
            executorService.submit {
                entryTimestamp = SystemClock.elapsedRealtime()
                sensorManager.registerListener(this, sensor, SensorManager.SENSOR_DELAY_NORMAL)
            }
        }
    }

    fun disable() {
        if (sensor != null) {
            Log.d(TAG, "Disabling")
            executorService.submit {
                sensorManager.unregisterListener(this, sensor)
                wasTilted = false
            }
        }
    }

    companion object {
        private const val TAG = "PickupSensor"

        // Galaxian: left on so `adb logcat -s PickupSensor` shows the
        // android.sensor.screen_upward values while this port is verified.
        private const val DEBUG = true

        // android.sensor.screen_upward value while the device is tilted/resting.
        private const val TILTED_VALUE = 1.0f

        private const val MIN_PULSE_INTERVAL_MS = 2500L
        private const val WAKELOCK_TIMEOUT_MS = 300L
    }
}
