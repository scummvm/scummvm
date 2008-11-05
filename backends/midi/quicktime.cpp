/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 */

#if defined(MACOSX) || defined(macintosh)

// HACK to disable deprecated warnings under Mac OS X 10.5.
// Apple depracted the complete QuickTime Music/MIDI API.
// Apps are supposed to use CoreAudio & CoreMIDI. We do support
// those, but while QT Midi support is still around, there is no
// reason to disable this driver. If they really ditch the API in 10.6,
// we can still release binaries with this driver disabled/removed.
#include <AvailabilityMacros.h>
#undef DEPRECATED_ATTRIBUTE
#define DEPRECATED_ATTRIBUTE



#include "common/endian.h"
#include "common/util.h"
#include "sound/musicplugin.h"
#include "sound/mpu401.h"

#if defined(MACOSX)
#include <QuickTime/QuickTimeComponents.h>
#include <QuickTime/QuickTimeMusic.h>
#else
#include <QuickTimeComponents.h>
#include <QuickTimeMusic.h>
#endif


// FIXME: the following disables reverb support in the QuickTime / CoreAudio
// midi backends. For some reasons, reverb will suck away a *lot* of CPU time.
// Until we know for sure what is causing this and if there is a better way to
// fix the problem, we just disable all reverb for these backends.
#define COREAUDIO_REVERB_HACK


/* QuickTime MIDI driver
 * Original QuickTime support by Florent Boudet <flobo@ifrance.com>
 * Modified by Max Horn
 */
class MidiDriver_QT : public MidiDriver_MPU401 {
public:
	MidiDriver_QT();

	int open();
	void close();
	void send(uint32 b);
	void setPitchBendRange (byte channel, uint range);

private:
	NoteAllocator qtNoteAllocator;
	NoteChannel qtNoteChannel[16];
	NoteRequest simpleNoteRequest;

	// Pitch bend tracking. Necessary since QTMA handles
	// pitch bending so differently from MPU401.
	uint16 _pitchbend [16];
	byte _pitchbend_range [16];

	void dispose();
};

MidiDriver_QT::MidiDriver_QT() {
	qtNoteAllocator = 0;
	for (int i = 0; i < 16; i++)
		qtNoteChannel[i] = 0;
}

int MidiDriver_QT::open() {
	ComponentResult qtErr = noErr;

	if (qtNoteAllocator != 0)
		return MERR_ALREADY_OPEN;

	qtNoteAllocator = OpenDefaultComponent(kNoteAllocatorComponentType, 0);
	if (qtNoteAllocator == 0)
		goto bail;

	simpleNoteRequest.info.flags = 0;
	WRITE_BE_UINT16(& simpleNoteRequest.info.polyphony, 11);        // simultaneous tones
	WRITE_BE_UINT16(& simpleNoteRequest.info.typicalPolyphony, 0x00010000);

	qtErr = NAStuffToneDescription(qtNoteAllocator, 1, &simpleNoteRequest.tone);
	if (qtErr != noErr)
		goto bail;

	for (int i = 0; i < 16; i++) {
		qtErr = NANewNoteChannel(qtNoteAllocator, &simpleNoteRequest, &(qtNoteChannel[i]));
		if ((qtErr != noErr) || (qtNoteChannel[i] == 0))
			goto bail;

		qtErr = NAResetNoteChannel(qtNoteAllocator, qtNoteChannel[i]);
		if (qtErr != noErr)
			goto bail;

		// Channel 10 (i.e. index 9) is the drum channel. Set it up as such.
		// All other channels default to piano.
		qtErr = NASetInstrumentNumber(qtNoteAllocator, qtNoteChannel[i], (i == 9 ? kFirstDrumkit : kFirstGMInstrument) + 1);
		if (qtErr != noErr)
			goto bail;
	}
	return 0;

bail:
	error("Init QT failed %x %x %d\n", (int)qtNoteAllocator, (int)qtNoteChannel, (int)qtErr);

	dispose();

	return MERR_DEVICE_NOT_AVAILABLE;
}

void MidiDriver_QT::close() {
	MidiDriver_MPU401::close();
	dispose();
}

void MidiDriver_QT::send(uint32 b) {
	MusicMIDIPacket midPacket;
	unsigned char *midiCmd = midPacket.data;
	midPacket.length = 3;
	midiCmd[3] = (b & 0xFF000000) >> 24;
	midiCmd[2] = (b & 0x00FF0000) >> 16;
	midiCmd[1] = (b & 0x0000FF00) >> 8;
	midiCmd[0] = (b & 0x000000FF);

	unsigned char chanID = midiCmd[0] & 0x0F;
	switch (midiCmd[0] & 0xF0) {
	case 0x80:										// Note off
		NAPlayNote(qtNoteAllocator, qtNoteChannel[chanID], midiCmd[1], 0);
		break;

	case 0x90:										// Note on
		NAPlayNote(qtNoteAllocator, qtNoteChannel[chanID], midiCmd[1], midiCmd[2]);
		break;

	case 0xB0:										// Effect
		switch (midiCmd[1]) {
		case 0x01:									// Modulation
			NASetController(qtNoteAllocator, qtNoteChannel[chanID], kControllerModulationWheel, midiCmd[2] << 8);
			break;

		case 0x07:									// Volume
			NASetController(qtNoteAllocator, qtNoteChannel[chanID], kControllerVolume, midiCmd[2] << 8);
			break;

		case 0x0A:									// Pan
			NASetController(qtNoteAllocator, qtNoteChannel[chanID], kControllerPan, (midiCmd[2] << 1) + 256);
			break;

		case 0x40:									// Sustain on/off
			NASetController(qtNoteAllocator, qtNoteChannel[chanID], kControllerSustain, midiCmd[2]);
			break;

		case 0x5b:									// ext effect depth
#if !defined(COREAUDIO_REVERB_HACK)
			NASetController(qtNoteAllocator, qtNoteChannel[chanID], kControllerReverb, midiCmd[2] << 8);
#endif
			break;

		case 0x5d:									// chorus depth
			NASetController(qtNoteAllocator, qtNoteChannel[chanID], kControllerChorus, midiCmd[2] << 8);
			break;

		case 0x7b:									// mode message all notes off
			// FIXME: For unknown reasons, the following code causes weird
			// troubles. In particular, in the Sam&Max intro, all channel are
			// sent this event. As a result, not only does the music stop - it
			// also never resumes!!! This is very odd.
/*			for (int i = 0; i < 128; i++)
				NAPlayNote(qtNoteAllocator, qtNoteChannel[chanID], i, 0);
*/
			break;
		case 0x64:
		case 0x65:
		case 0x06:
		case 0x26:
			// pitch bend changes - ignore those for now
			break;

		case 0x12:	// Occurs in Scumm games
		case 0x77:	// Occurs in Simon2
		case 0x79:	// Occurs in Simon1
			// What are these ?!? Ignore it for now
			break;

		default:
			warning("Unknown MIDI effect: %08x", (int)b);
			break;
		}
		break;

	case 0xC0:										// Program change
		NASetInstrumentNumber(qtNoteAllocator, qtNoteChannel[chanID], midiCmd[1] + (chanID == 9 ? kFirstDrumkit : kFirstGMInstrument));
		break;

	case 0xE0:{									// Pitch bend
			// QuickTime specifies pitchbend in semitones, using 8.8 fixed point values;
			// but iMuse sends us the pitch bend data as 0-16383. which has to be mapped
			// to +/- 12 semitones. Based on this, we first center the input data, then
			// multiply it by a factor. If all was right, the factor would be 3/8, but for
			// mysterious reasons the actual factor we have to use is more like 1/32 or 3/64.
			// Maybe the QT docs are right, and
			_pitchbend[chanID] = ((uint16) midiCmd[1] | (uint16) (midiCmd[2] << 7));
			long theBend = ((long) _pitchbend[chanID] - 0x2000) * _pitchbend_range[chanID] / 32;

			NASetController(qtNoteAllocator, qtNoteChannel[chanID], kControllerPitchBend, theBend);
		}
		break;

	default:
		error("Unknown Command: %08x", (int)b);
		NASendMIDI(qtNoteAllocator, qtNoteChannel[chanID], &midPacket);
		break;
	}
}

void MidiDriver_QT::setPitchBendRange (byte channel, uint range) {
	if (_pitchbend_range[channel] == range)
		return;
	_pitchbend_range[channel] = range;

	long theBend = _pitchbend[channel];
	theBend = (theBend - 0x2000) * range / 32;
	NASetController(qtNoteAllocator, qtNoteChannel[channel], kControllerPitchBend, theBend);
}

void MidiDriver_QT::dispose() {
	for (int i = 0; i < 16; i++) {
		if (qtNoteChannel[i] != 0)
			NADisposeNoteChannel(qtNoteAllocator, qtNoteChannel[i]);
		qtNoteChannel[i] = 0;
	}

	if (qtNoteAllocator != 0) {
		CloseComponent(qtNoteAllocator);
		qtNoteAllocator = 0;
	}
}


// Plugin interface

class QuickTimeMusicPlugin : public MusicPluginObject {
public:
	const char *getName() const {
		return "QuickTime";
	}

	const char *getId() const {
		return "qt";
	}

	MusicDevices getDevices() const;
	Common::Error createInstance(Audio::Mixer *mixer, MidiDriver **mididriver) const;
};

MusicDevices QuickTimeMusicPlugin::getDevices() const {
	MusicDevices devices;
	// TODO: Return a different music type depending on the configuration
	// TODO: List the available devices
	devices.push_back(MusicDevice(this, "", MT_GM));
	return devices;
}

Common::Error QuickTimeMusicPlugin::createInstance(Audio::Mixer *mixer, MidiDriver **mididriver) const {
	*mididriver = new MidiDriver_QT();

	return Common::kNoError;
}

MidiDriver *MidiDriver_QT_create(Audio::Mixer *mixer) {
	MidiDriver *mididriver;

	QuickTimeMusicPlugin p;
	p.createInstance(mixer, &mididriver);

	return mididriver;
}

//#if PLUGIN_ENABLED_DYNAMIC(QUICKTIME)
	//REGISTER_PLUGIN_DYNAMIC(QUICKTIME, PLUGIN_TYPE_MUSIC, QuickTimeMusicPlugin);
//#else
	REGISTER_PLUGIN_STATIC(QUICKTIME, PLUGIN_TYPE_MUSIC, QuickTimeMusicPlugin);
//#endif

#endif // MACOSX || macintosh
