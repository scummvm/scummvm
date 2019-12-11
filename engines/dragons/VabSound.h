//
// Created by Edu García on 2019-07-25.
//

#ifndef DRAGONS_VABSOUND_H
#define DRAGONS_VABSOUND_H

#include <common/scummsys.h>

namespace Common {
	class SeekableReadStream;
}

namespace Dragons {
class DragonsEngine;

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

	void playSound(uint16 program, uint16 key);

private:
	Common::SeekableReadStream *_vbData;

	VabHeader _header;
	VabProgramAttr _programAttrs[128];
	VabToneAttr *_toneAttrs;

	uint32 _vagSizes[0x100];
	uint32 _vagOffsets[0x100];

	void loadHeader(Common::SeekableReadStream *vhData);
};

} // End of namespace Dragons

#endif //SCUMMVM_VABSOUND_H
