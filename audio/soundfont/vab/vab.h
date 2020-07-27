/*
 * VGMTrans (c) 2002-2019
 * Licensed under the zlib license,
 * refer to the included LICENSE.txt file
 */
#ifndef AUDIO_SOUNDFONT_VAB_H
#define AUDIO_SOUNDFONT_VAB_H

#include "audio/soundfont/common.h"
#include "common/str.h"
#include "audio/soundfont/vgminstrset.h"
#include "audio/soundfont/vgmsamp.h"

class Vab : public VGMInstrSet {
public:
	Vab(RawFile *file, uint32 offset);
	virtual ~Vab(void);

	virtual bool GetHeaderInfo();
	virtual bool GetInstrPointers();
};

// ********
// VabInstr
// ********

class VabInstr : public VGMInstr {
public:
	VabInstr(VGMInstrSet *instrSet, uint32 offset, uint32 length, uint32 theBank,
			 uint32 theInstrNum, const Common::String &name = "Instrument");
	virtual ~VabInstr(void);

	virtual bool LoadInstr();

public:
	uint8 _tones;
	uint8 _masterVol;
};

// ******
// VabRgn
// ******

class VabRgn : public VGMRgn {
public:
	VabRgn(VabInstr *instr, uint32 offset);

	virtual bool LoadRgn();

public:
	uint16 _ADSR1;  // raw ps2 ADSR1 value (articulation data)
	uint16 _ADSR2;  // raw ps2 ADSR2 value (articulation data)
};

#endif // AUDIO_SOUNDFONT_VAB_H
