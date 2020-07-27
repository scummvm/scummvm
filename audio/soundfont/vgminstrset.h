/*
 * VGMTrans (c) 2002-2019
 * Licensed under the zlib license,
 * refer to the included LICENSE.txt file
 */
#ifndef AUDIO_SOUNDFONT_VGMINSTRSET_H
#define AUDIO_SOUNDFONT_VGMINSTRSET_H

#include "common/scummsys.h"
#include "common/str.h"
#include "common/array.h"
#include "vgmitem.h"
#include "sf2file.h"

class VGMSampColl;
class VGMInstr;
class VGMRgn;
class VGMSamp;
class VGMRgnItem;

// ***********
// VGMInstrSet
// ***********

class VGMInstrSet : public VGMFile {
public:

	VGMInstrSet(RawFile *file, uint32 offset, uint32 length = 0,
				Common::String name = "VGMInstrSet", VGMSampColl *theSampColl = NULL);
	virtual ~VGMInstrSet(void);

	virtual bool Load();
	virtual bool GetHeaderInfo();
	virtual bool GetInstrPointers();
	virtual bool LoadInstrs();

public:
	Common::Array<VGMInstr *> _aInstrs;
	VGMSampColl *_sampColl;
};

// ********
// VGMInstr
// ********

class VGMInstr : public VGMContainerItem {
public:
	VGMInstr(VGMInstrSet *parInstrSet, uint32 offset, uint32 length, uint32 bank,
			 uint32 instrNum, const Common::String &name = "Instrument");
	virtual ~VGMInstr(void);

	virtual bool LoadInstr();

public:
	uint32 _bank;
	uint32 _instrNum;

	VGMInstrSet *_parInstrSet;
	Common::Array<VGMRgn *> _aRgns;
};

#endif // AUDIO_SOUNDFONT_VGMINSTRSET_H
