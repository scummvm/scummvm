/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

package org.scummvm.scummvm;

import android.bluetooth.BluetoothDevice;
import android.content.Context;
import android.media.midi.MidiDevice;
import android.media.midi.MidiDeviceInfo;
import android.media.midi.MidiManager;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.annotation.RequiresApi;

import java.util.ArrayList;
import java.util.Set;

// MIDI is available from Marshmallow but not exposed by NDK before Quince Tart
@RequiresApi(Build.VERSION_CODES.Q)
class MidiPort {
    private static class MidiDeviceOpenedListener implements MidiManager.OnDeviceOpenedListener {
        final Object synchronizer = new Object();
        MidiDevice midiDevice = null;

        @Override
        public void onDeviceOpened(MidiDevice midiDevice) {
            synchronized (synchronizer) {
                this.midiDevice = midiDevice;
                synchronizer.notify();
            }
        }
    }

    private final MidiDeviceInfo device;
    private final MidiDeviceInfo.PortInfo port;
    private String name;

    private MidiPort(MidiDeviceInfo device, MidiDeviceInfo.PortInfo port) {
        this.device = device;
        this.port = port;
    }

    @NonNull
    public static ArrayList<MidiPort> getMidiPorts(@NonNull Context context) {
        MidiManager manager = (MidiManager)context.getSystemService(Context.MIDI_SERVICE);
        MidiDeviceInfo[] devices;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
            Set<MidiDeviceInfo> infosSet = manager.getDevicesForTransport(MidiManager.TRANSPORT_MIDI_BYTE_STREAM);
            devices = infosSet.toArray(new MidiDeviceInfo[]{});
        } else {
            // This useless intermediate variable is here to allow attaching the annotation
            @SuppressWarnings({"deprecation", "RedundantSuppression"})
            MidiDeviceInfo[] devices_ = manager.getDevices();
            devices = devices_;
        }
        ArrayList<MidiPort> ports = new ArrayList<>();
        for (MidiDeviceInfo device : devices) {
            for (MidiDeviceInfo.PortInfo port : device.getPorts()) {
                if (port.getType() != MidiDeviceInfo.PortInfo.TYPE_INPUT) {
                    continue;
                }
                ports.add(new MidiPort(device, port));
            }
        }
        ports.sort((MidiPort l, MidiPort r) -> l.getName().compareTo(r.getName()));
        return ports;
    }

    @NonNull
    public String getName() {
        if (name != null) {
            return name;
        }

        Bundle props = device.getProperties();
        String name = props.getString(MidiDeviceInfo.PROPERTY_NAME);
        final String serial = props.getString(MidiDeviceInfo.PROPERTY_SERIAL_NUMBER);
        if (serial != null && !serial.isEmpty()) {
            name += " (" + serial + ")";
        }
        name += " - ";
        String portName = port.getName();
        if (portName != null && !portName.isEmpty()) {
            name += portName;
        } else {
            int portNum = port.getPortNumber();
            name += portNum;
        }
        this.name = name;
        return name;
    }

    public MidiDevice open(@NonNull Context context, @NonNull int[] portId) {
        MidiManager manager = (MidiManager)context.getSystemService(Context.MIDI_SERVICE);

        MidiDeviceOpenedListener listener = new MidiDeviceOpenedListener();
        HandlerThread thread = new HandlerThread("MIDI Device Handler");
        thread.start();
        Handler handler = new Handler(thread.getLooper());

        BluetoothDevice ble;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
            ble = device.getProperties().getParcelable(MidiDeviceInfo.PROPERTY_BLUETOOTH_DEVICE, BluetoothDevice.class);
        } else {
            @SuppressWarnings({"deprecation", "RedundantSuppression"})
            Object ble_ = device.getProperties().getParcelable(MidiDeviceInfo.PROPERTY_BLUETOOTH_DEVICE);
            ble = (BluetoothDevice) ble_;
        }
        synchronized (listener.synchronizer) {
            if (ble != null) {
                manager.openBluetoothDevice(ble, listener, handler);
            } else {
                manager.openDevice(device, listener, handler);
            }

            try {
                listener.synchronizer.wait();
            } catch (InterruptedException e) {
                Log.d(ScummVM.LOG_TAG, "Warning: interrupted while waiting for MIDI device");
                thread.quit();
                return null;
            }
        }

        thread.quit();
        portId[0] = port.getPortNumber();
        return listener.midiDevice;
    }
}
