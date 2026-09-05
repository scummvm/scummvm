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

// Allow use of stuff in <time.h> and abort()
#define FORBIDDEN_SYMBOL_EXCEPTION_time_h
#define FORBIDDEN_SYMBOL_EXCEPTION_abort

// Disable printf override in common/forbidden.h to avoid
// clashes with log.h from the Android SDK.
// That header file uses
//   __attribute__ ((format(printf, 3, 4)))
// which gets messed up by our override mechanism; this could
// be avoided by either changing the Android SDK to use the equally
// legal and valid
//   __attribute__ ((format(__printf__, 3, 4)))
// or by refining our printf override to use a varadic macro
// (which then wouldn't be portable, though).
// Anyway, for now we just disable the printf override globally
// for the Android port
#define FORBIDDEN_SYMBOL_EXCEPTION_printf

#include "common/scummsys.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "backends/platform/android/android.h"
#include "backends/platform/android/jni-android.h"

#include "audio/musicplugin.h"
#include "audio/mpu401.h"

#include "common/translation.h"

#include <dlfcn.h>
#include <amidi/AMidi.h>

class MidiDriver_Android : public MidiDriver_MPU401 {
public:
	MidiDriver_Android(jobject device, int32_t portId);
	~MidiDriver_Android();
	int open() override;
	bool isOpen() const override { return _inPort; }
	void close() override;
	void send(uint32 b) override;
	void sysEx(const byte *msg, uint16 length) override;

private:
	AMidiDevice *_device;
	int32_t _portId;
	AMidiInputPort *_inPort;


	// We can't use NDK weak symbols because AMidi depends on libamidi.so to be loaded when our own library is
	// but there is no weak library concept so if we reference it, it must be present.
	static int amidi_loaded;
	static media_status_t (*AMidiDevice_fromJava)(
		JNIEnv *env, jobject midiDeviceObj, AMidiDevice **outDevicePtrPtr);
	static media_status_t (*AMidiDevice_release)(const AMidiDevice *midiDevice);
	static ssize_t (*AMidiDevice_getNumInputPorts)(const AMidiDevice *device);
	static media_status_t (*AMidiInputPort_open)(const AMidiDevice *device, int32_t portNumber,
		AMidiInputPort **outInputPortPtr);
	static ssize_t (*AMidiInputPort_send)(const AMidiInputPort *inputPort, const uint8_t *buffer,
                   size_t numBytes);
	static void (*AMidiInputPort_close)(const AMidiInputPort *inputPort);
};

int MidiDriver_Android::amidi_loaded = 0;
media_status_t (*MidiDriver_Android::AMidiDevice_fromJava)(
	JNIEnv *env, jobject midiDeviceObj, AMidiDevice **outDevicePtrPtr) = nullptr;
media_status_t (*MidiDriver_Android::AMidiDevice_release)(const AMidiDevice *midiDevice) = nullptr;
ssize_t (*MidiDriver_Android::AMidiDevice_getNumInputPorts)(const AMidiDevice *device) = nullptr;
media_status_t (*MidiDriver_Android::AMidiInputPort_open)(const AMidiDevice *device, int32_t portNumber,
	AMidiInputPort **outInputPortPtr) = nullptr;
ssize_t (*MidiDriver_Android::AMidiInputPort_send)(const AMidiInputPort *inputPort, const uint8_t *buffer,
	   size_t numBytes) = nullptr;
void (*MidiDriver_Android::AMidiInputPort_close)(const AMidiInputPort *inputPort) = nullptr;

MidiDriver_Android::MidiDriver_Android(jobject device, int32_t portId)
	: _device(nullptr), _portId(portId), _inPort(nullptr) {
	if (!device) {
		// Device couldn't be open in Android side: create a bogus device
		return;
	}

	if (!amidi_loaded) {
		// Lazy load the AMidi functions
		void *amidi = dlopen("libamidi.so", RTLD_NOW | RTLD_LOCAL);
		if (!amidi) {
			warning("MidiDriver_Android can't load AMidi");
			amidi_loaded = -1;
		}

#define LOAD_FUNC(n, T) \
	if (!amidi_loaded && !(MidiDriver_Android::n = (T)dlsym(amidi, #n))) { \
		amidi_loaded = -1; \
		warning("MidiDriver_Android can't find AMidi function %s", #n); \
	}

		LOAD_FUNC(AMidiDevice_fromJava, media_status_t (*)(JNIEnv *, jobject, AMidiDevice **));
		LOAD_FUNC(AMidiDevice_release, media_status_t (*)(const AMidiDevice *));
		LOAD_FUNC(AMidiDevice_getNumInputPorts, ssize_t (*)(const AMidiDevice *));
		LOAD_FUNC(AMidiInputPort_open, media_status_t (*)(const AMidiDevice *, int32_t, AMidiInputPort **));
		LOAD_FUNC(AMidiInputPort_send, ssize_t (*)(const AMidiInputPort *, const uint8_t *, size_t));
		LOAD_FUNC(AMidiInputPort_close, void (*)(const AMidiInputPort *));

		if (!amidi_loaded) {
			amidi_loaded = 1;
		}

	}
	if (amidi_loaded != 1) {
		// AMidi loading failed: create a bogus device
		return;
	}

	JNIEnv *env = JNI::getEnv();

	media_status_t status = MidiDriver_Android::AMidiDevice_fromJava(env, device, &_device);
	env->DeleteLocalRef(device);

	if (status != AMEDIA_OK) {
		warning("Can't open MIDI device: %d", status);
		_device = nullptr;
	}
}

MidiDriver_Android::~MidiDriver_Android() {
	if (_inPort) {
		MidiDriver_Android::AMidiInputPort_close(_inPort);
		_inPort = nullptr;
	}
	if (_device) {
		MidiDriver_Android::AMidiDevice_release(_device);
		_device = nullptr;
	}
}

int MidiDriver_Android::open() {
	if (!_device) {
		return MERR_DEVICE_NOT_AVAILABLE;
	}

	if (isOpen()) {
		return MERR_ALREADY_OPEN;
	}

	ssize_t inPorts = MidiDriver_Android::AMidiDevice_getNumInputPorts(_device);
	if (inPorts < 0) {
		warning("Can't get MIDI device input ports: %d", (int)inPorts);
		return MERR_DEVICE_NOT_AVAILABLE;
	}

	if (_portId >= inPorts) {
		return MERR_DEVICE_NOT_AVAILABLE;
	}

	media_status_t status = MidiDriver_Android::AMidiInputPort_open(_device, _portId, &_inPort);
	if (status != AMEDIA_OK) {
		g_system->displayMessageOnOSD(_("Can't connect MIDI port: no sound will be produced"));
		_inPort = nullptr;
		return MERR_CANNOT_CONNECT;
	}

	return 0;
}

void MidiDriver_Android::close() {
	MidiDriver_MPU401::close();

	if (_inPort) {
		MidiDriver_Android::AMidiInputPort_close(_inPort);
		_inPort = nullptr;
	}
}

void MidiDriver_Android::send(uint32 b) {
	assert(isOpen());

	midiDriverCommonSend(b);

	// Extract the MIDI data
	byte data[3] = {
		static_cast<byte>(b & 0x000000FF), // status byte
		static_cast<byte>((b & 0x0000FF00) >> 8), // first byte
		static_cast<byte>((b & 0x00FF0000) >> 16), // second byte
	};

	size_t data_length;

	// Compute the correct length of the MIDI command. This is important,
	// else things may screw up badly...
	switch (data[0] & 0xF0) {
	case 0x80:	// Note Off
	case 0x90:	// Note On
	case 0xA0:	// Polyphonic Aftertouch
	case 0xB0:	// Controller Change
	case 0xE0:	// Pitch Bending
		data_length = 3;
		break;
	case 0xC0:	// Programm Change
	case 0xD0:	// Monophonic Aftertouch
		data_length = 2;
		break;
	default:
		warning("Android driver encountered unsupported status byte: 0x%02x", data[0]);
		data_length = 3;
		break;
	}

	MidiDriver_Android::AMidiInputPort_send(_inPort, data, data_length);
}

void MidiDriver_Android::sysEx(const byte *msg, uint16 length) {
	assert(isOpen());

	unsigned char buf[270];

	assert(length + 2 <= ARRAYSIZE(buf));

	midiDriverCommonSysEx(msg, length);

	// Add SysEx frame
	buf[0] = 0xF0;
	memcpy(buf + 1, msg, length);
	buf[length + 1] = 0xF7;

	MidiDriver_Android::AMidiInputPort_send(_inPort, buf, length + 2);
}


// Plugin interface

class AndroidMusicPlugin : public MusicPluginObject {
public:
	const char *getName() const {
		return "Android";
	}

	const char *getId() const {
		return "android";
	}

	MusicDevices getDevices() const;
	Common::Error createInstance(MidiDriver **mididriver, MidiDriver::DeviceHandle = 0) const;
};

MusicDevices AndroidMusicPlugin::getDevices() const {
	MusicDevices devices;
	Common::Array<Common::String> names = JNI::getMIDIDevices();
	for (uint i = 0 ; i < names.size(); i++) {
		devices.push_back(MusicDevice(this, names[i], MT_GM)); //Assume GM here
	}
	return devices;
}

Common::Error AndroidMusicPlugin::createInstance(MidiDriver **mididriver, MidiDriver::DeviceHandle device) const {
	Common::Array<Common::String> names = JNI::getMIDIDevices();

	jobject midiDevice = nullptr;
	int32_t port = 0;

	for (uint i = 0 ; i < names.size(); i++) {
		MusicDevice md(this, names[i], MT_GM);
		if (md.getHandle() == device) {
			midiDevice = JNI::openMIDIDevice(i, &port);
			break;
		}
	}
	// Always return a driver even if the device doesn't exist: engines don't handle the error
	// We will fail at opening
	*mididriver = new MidiDriver_Android(midiDevice, port);
	return Common::kNoError;
}

//#if PLUGIN_ENABLED_DYNAMIC(ANDROID)
	//REGISTER_PLUGIN_DYNAMIC(ANDROID, PLUGIN_TYPE_MUSIC, AndroidMusicPlugin);
//#else
	REGISTER_PLUGIN_STATIC(ANDROID, PLUGIN_TYPE_MUSIC, AndroidMusicPlugin);
//#endif
