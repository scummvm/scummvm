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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef DRAGONS_VABSOUND_H
#define DRAGONS_VABSOUND_H

#include "common/scummsys.h"

namespace Common {
	class SeekableReadStream;
}

namespace Audio {
	class AudioStream;
}

namespace Dragons {
class DragonsEngine;

#define DRAGONS_VAB_NUM_PROG_ATTRS 128

struct VabHeader {
	char magic[4];

	uint32 version;
	uint32 vabId;
	uint32 waveformSize;

	uint16 reserved0;
	uint16 numPrograms;
	uint16 numTones;
	uint16 numVAG;

	uint8 masterVolume;
	uint8 masterPan;
	uint8 bankAttr1;
	uint8 bankAttr2;

	uint32 reserved1;
};

struct VabProgramAttr {
	uint8 tones;
	uint8 mvol;
	uint8 prior;
	uint8 mode;
	uint8 mpan;
	uint8 reserved0;
	uint16 attr;
	uint32 reserved1;
	uint32 reserved2;
};

struct VabToneAttr {
	uint8 prior;
	uint8 mode;
	uint8 vol;
	uint8 pan;
	uint8 center;
	uint8 shift;
	uint8 min;
	uint8 max;
	uint8 vibW;
	uint8 vibT;
	uint8 porW;
	uint8 porT;
	uint8 pbmin;
	uint8 pbmax;
	uint8 reserved1;
	uint8 reserved2;
	uint16 adsr1;
	uint16 adsr2;
	int16 prog;
	int16 vag;
	int16 reserved[4];
};

class VabSound {
public:
	/**
	 * Creates a VAB file with both header and body (*.MSF). VabSound will dispose msfData
	 * @param data
	 */
	VabSound(Common::SeekableReadStream* msfData, const DragonsEngine *_vm);

	/**
	 * Creates a VAB file with separate header and body (*.VH and *.VB). VabSound will dispose vhData & vbData
	 *
	 * @param dataHeader
	 * @param dataBody
	 */
	VabSound(Common::SeekableReadStream* vhData, Common::SeekableReadStream* vbData);

	~VabSound();

	bool hasSound(uint16 program, uint16 key);
	Audio::AudioStream *getAudioStream(uint16 program, uint16 key);

private:
	byte *_vbData;

	VabHeader _header;
	VabProgramAttr _programAttrs[DRAGONS_VAB_NUM_PROG_ATTRS];
	VabToneAttr *_toneAttrs;

	uint32 _vagSizes[0x100];
	uint32 _vagOffsets[0x100];

	void loadHeader(Common::SeekableReadStream *vhData);
	void loadProgramAttributes(Common::SeekableReadStream *vhData);
	void loadToneAttributes(Common::SeekableReadStream *vhData);

	int16 getVagID(uint16 program, uint16 key);
	int16 getBaseToneKey(uint16 program, uint16 key);

	int getAdjustedSampleRate(int16 desiredKey, int16 baseToneKey);
};

} // End of namespace Dragons

#endif //DRAGONS_VABSOUND_H
