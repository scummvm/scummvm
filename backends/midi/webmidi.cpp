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

// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_EXCEPTION_FILE
#define FORBIDDEN_SYMBOL_EXCEPTION_getenv

#ifdef EMSCRIPTEN
#include <emscripten.h>

#include "audio/mpu401.h"
#include "audio/musicplugin.h"
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/error.h"
#include "common/scummsys.h"
#include "common/textconsole.h"
#include "common/util.h"

/* WebMidi MIDI driver
 */
class MidiDriver_WebMIDI : public MidiDriver_MPU401 {
public:
	MidiDriver_WebMIDI(Common::String name, const char *id);
	~MidiDriver_WebMIDI();
	int open() override;
	bool isOpen() const override;
	void close() override;
	void send(uint32 b) override;
	void sysEx(const byte *msg, uint16 length) override;

private:
	Common::String _name;
	const char *_id;
};

MidiDriver_WebMIDI::MidiDriver_WebMIDI(Common::String name, const char *id)
	: _name(name), _id(id), MidiDriver_MPU401() {
	debug(5, "WebMIDI: Creating device %s %s", _name.c_str(), _id);
}

MidiDriver_WebMIDI::~MidiDriver_WebMIDI() {
}

EM_ASYNC_JS(int, _midiOpen, (const char *id_c), {
	var id = UTF8ToString(id_c);
	result = await midiOutputMap.get(id).open().catch((error) => {
		console.error(error);
		return 1;
	});
	return 0;
});

int MidiDriver_WebMIDI::open() {
	debug(5, "WebMIDI: Opening device %s", _name.c_str());
	if (isOpen())
		return MERR_ALREADY_OPEN;
	return _midiOpen(_id);
}

EM_JS(bool, _midiIsOpen, (const char *id_c), {
	var id = UTF8ToString(id_c);
	return midiOutputMap.get(id) && midiOutputMap.get(id).connection == "open";
});

bool MidiDriver_WebMIDI::isOpen() const {
	return _midiIsOpen(_id);
}

EM_ASYNC_JS(void, _midiClose, (const char *id_c), {
	var id = UTF8ToString(id_c);
	await midiOutputMap.get(id).close(); // TODO: Wait for promise to resolve (should be possible with Asyncify)
});

void MidiDriver_WebMIDI::close() {
	debug(5, "WebMIDI: Closing device %s", _name.c_str());
	MidiDriver_MPU401::close();
	if(_midiIsOpen(_id))
		_midiClose(_id);
	else
		warning("WebMIDI: Device %s is not open", _name.c_str());
}

EM_JS(void, _midiSendMessage, (const char *id_c, int status_byte, int first_byte, int second_byte), {
	var id = UTF8ToString(id_c);
	if (status_byte < 0xc0 || status_byte > 0xdf) {
		midiOutputMap.get(id).send([status_byte, first_byte, second_byte]);
	} else {
		midiOutputMap.get(id).send([status_byte, first_byte]);		
	}
});

void MidiDriver_WebMIDI::send(uint32 b) {
	if(!isOpen()) {
		warning("WebMIDI: Send called on closed device %s", _name.c_str());
		return;
	}
	debug(5, "WebMIDI: Sending message for device %s %x", _name.c_str(), b);
	midiDriverCommonSend(b);

	const byte status_byte = b & 0xff;
	const byte first_byte = (b >> 8) & 0xff;
	const byte second_byte = (b >> 16) & 0xff;
	_midiSendMessage(_id, status_byte, first_byte, second_byte);
}

void MidiDriver_WebMIDI::sysEx(const byte *msg, uint16 length) {
	if(!isOpen()) {
		warning("WebMIDI: SysEx called on closed device %s", _name.c_str());
		return;
	}
	midiDriverCommonSysEx(msg, length);
	warning("WebMIDI: SysEx not implemented yet, skipping %d bytes", length);
}

// Plugin interface

class WebMIDIMusicPlugin : public MusicPluginObject {
public:
	const char *getName() const {
		return "Web Midi";
	}

	const char *getId() const {
		return "WebMidi";
	}

	MusicDevices getDevices() const;
	Common::Error createInstance(MidiDriver **mididriver, MidiDriver::DeviceHandle = 0) const;
};

EM_JS(char **, _midiGetOutputNames, (), {
	deviceNames = Array.from(midiOutputMap || []).map((elem) => elem[1].name);
	deviceNames.push(""); // we need this to find the end of the array on the native side.

	// convert the strings to C strings
	var c_strings = deviceNames.map((s) => {
		var size = lengthBytesUTF8(s) + 1;
		var ret = Module._malloc(size);
		stringToUTF8Array(s, HEAP8, ret, size);
		return ret;
	});

	// populate return array
	var ret_arr = Module._malloc(c_strings.length * 4); // 4-bytes per pointer
	c_strings.forEach((ptr, i) => { Module.setValue(ret_arr + i * 4, ptr, "i32"); });

	return ret_arr;
});

MusicDevices WebMIDIMusicPlugin::getDevices() const {
	MusicDevices devices;
	char **outputNames = _midiGetOutputNames();
	char **iter = outputNames;
	while (strcmp(*iter, "") != 0) {
		char *c_name = *iter++;
		Common::String name = Common::String(c_name);
		devices.push_back(MusicDevice(this, name, MT_GM));
		free(c_name);
	}
	free(outputNames);
	return devices;
}

EM_JS(const char *, _midiGetOutputId, (const char *name_c), {
	var name = UTF8ToString(name_c);
	for (const [key, midiOutput] of midiOutputMap.entries()) {
		if (midiOutput.name == name) {
			return stringToNewUTF8(key)
		}
	}
	return;
});

Common::Error WebMIDIMusicPlugin::createInstance(MidiDriver **mididriver, MidiDriver::DeviceHandle device) const {
	MusicDevices deviceList = getDevices();
	for (MusicDevices::iterator j = deviceList.begin(), jend = deviceList.end(); j != jend; ++j) {
		if (j->getHandle() == device) {
			*mididriver = new MidiDriver_WebMIDI(j->getName(), _midiGetOutputId(j->getName().c_str()));
			return Common::kNoError;
		}
	}
	return Common::kUnknownError;
}

//#if PLUGIN_ENABLED_DYNAMIC(WebMidi)
	//REGISTER_PLUGIN_DYNAMIC(WebMidi, PLUGIN_TYPE_MUSIC, WebMIDIMusicPlugin);
//#else
	REGISTER_PLUGIN_STATIC(WEBMIDI, PLUGIN_TYPE_MUSIC, WebMIDIMusicPlugin);
//#endif

#endif // EMSCRIPTEN
