/*
 * VGMTrans (c) 2002-2019
 * Licensed under the zlib license,
 * refer to the included LICENSE.txt file
 */
#ifndef AUDIO_SOUNDFONT_VGMCOLL_H
#define AUDIO_SOUNDFONT_VGMCOLL_H

#include "common.h"
#include "common/array.h"

class VGMInstrSet;
class VGMSampColl;
class VGMSamp;
class SF2File;
class SynthFile;

class VGMColl {
public:
	SF2File *CreateSF2File(VGMInstrSet *theInstrSet);

private:
	SynthFile *CreateSynthFile(VGMInstrSet *theInstrSet);
	void UnpackSampColl(SynthFile &synthfile, VGMSampColl *sampColl,
						Common::Array<VGMSamp *> &finalSamps);
};

#endif // AUDIO_SOUNDFONT_VGMCOLL_H
