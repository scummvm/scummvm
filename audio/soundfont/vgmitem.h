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
 */
/*
 * VGMTrans (c) 2002-2019
 * Licensed under the zlib license,
 * refer to the included VGMTrans_LICENSE.txt file
 */
#ifndef AUDIO_SOUNDFONT_VGMITEM_H
#define AUDIO_SOUNDFONT_VGMITEM_H

#include "common/scummsys.h"
#include "common/str.h"
#include "common/array.h"
#include "rawfile.h"
#include "synthfile.h"

class RawFile;

//template <class T>
class VGMFile;
class VGMItem;
class VGMHeader;

class VGMItem {
public:
	VGMItem();
	VGMItem(VGMFile *thevgmfile, uint32 theOffset, uint32 theLength = 0,
			const Common::String theName = "");
	virtual ~VGMItem(void);

public:
	RawFile *GetRawFile();

protected:
	// TODO make inline
	uint32 GetBytes(uint32 nIndex, uint32 nCount, void *pBuffer);
	uint8 GetByte(uint32 offset);
	uint16 GetShort(uint32 offset);

public:
	VGMFile *_vgmfile;
	Common::String _name;
	uint32 _dwOffset;  // offset in the pDoc data buffer
	uint32 _unLength;  // num of bytes the event engulfs
};

class VGMContainerItem : public VGMItem {
public:
	VGMContainerItem();
	VGMContainerItem(VGMFile *thevgmfile, uint32 theOffset, uint32 theLength = 0,
					 const Common::String theName = "");
	virtual ~VGMContainerItem(void);

	VGMHeader *AddHeader(uint32 offset, uint32 length, const Common::String &name = "Header");

	void AddSimpleItem(uint32 offset, uint32 length, const Common::String &theName);

	template<class T>
	void AddContainer(Common::Array<T *> &container) {
		_containers.push_back(reinterpret_cast<Common::Array<VGMItem *> *>(&container));
	}

public:
	Common::Array<VGMHeader *> _headers;
	Common::Array<Common::Array<VGMItem *> *> _containers;
	Common::Array<VGMItem *> _localitems;
};

class VGMColl;

class VGMFile : public VGMContainerItem {
public:

public:
	VGMFile(RawFile *theRawFile, uint32 offset, uint32 length = 0,
			Common::String theName = "VGM File");
	virtual ~VGMFile();

	bool LoadVGMFile();
	virtual bool Load() = 0;

	RawFile *GetRawFile();

	size_t size() const { return _unLength; }
	Common::String name() const { return _name; }

	uint32 GetBytes(uint32 nIndex, uint32 nCount, void *pBuffer);

	inline uint8 GetByte(uint32 offset) const { return _rawfile->GetByte(offset); }
	inline uint16 GetShort(uint32 offset) const { return _rawfile->GetShort(offset); }
	inline uint32 GetWord(uint32 offset) const { return _rawfile->GetWord(offset); }
	/*
	 * For whatever reason, you can create null-length VGMItems.
	 * The only safe way for now is to
	 * assume maximum length
	 */
	size_t GetEndOffset() { return _rawfile->size(); }

	const char *data() const { return _rawfile->data() + _dwOffset; }

	RawFile *_rawfile;
};

// *********
// VGMHeader
// *********

class VGMHeader : public VGMContainerItem {
public:
	VGMHeader(VGMItem *parItem, uint32 offset = 0, uint32 length = 0,
			  const Common::String &name = "Header");
	virtual ~VGMHeader();
};

class VGMInstr;
class VGMRgnItem;
class VGMSampColl;

// ******
// VGMRgn
// ******

class VGMRgn : public VGMContainerItem {
public:
	VGMRgn(VGMInstr *instr, uint32 offset, uint32 length = 0, Common::String name = "Region");
	~VGMRgn();

	virtual bool LoadRgn() { return true; }

	void AddGeneralItem(uint32 offset, uint32 length, const Common::String &name);
	void SetFineTune(int16 relativePitchCents) { _fineTune = relativePitchCents; }
	void SetPan(uint8 pan);
	void AddPan(uint8 pan, uint32 offset, uint32 length = 1);
	void AddVolume(double volume, uint32 offset, uint32 length = 1);
	void AddUnityKey(int8 unityKey, uint32 offset, uint32 length = 1);
	void AddKeyLow(uint8 keyLow, uint32 offset, uint32 length = 1);
	void AddKeyHigh(uint8 keyHigh, uint32 offset, uint32 length = 1);
	void AddSampNum(int sampNum, uint32 offset, uint32 length = 1);

	VGMInstr *_parInstr;
	uint8 _keyLow;
	uint8 _keyHigh;
	uint8 _velLow;
	uint8 _velHigh;

	int8 _unityKey;
	short _fineTune;

	Loop _loop;

	int _sampNum;
	VGMSampColl *_sampCollPtr;

	double _volume;        /* Percentage of full volume */
	double _pan;           /* Left 0 <- 0.5 Center -> 1 Right */
	double _attack_time;   /* In seconds */
	double _decay_time;    /* In seconds */
	double _release_time;  /* In seconds */
	double _sustain_level; /* Percentage */
	double _sustain_time;  /* In seconds (no positive rate!) */

	uint16 _attack_transform;
	uint16 _release_transform;

	Common::Array<VGMRgnItem *> _items;
};

// **********
// VGMRgnItem
// **********

class VGMRgnItem : public VGMItem {
public:
	enum RgnItemType {
		RIT_GENERIC,
		RIT_UNKNOWN,
		RIT_UNITYKEY,
		RIT_FINETUNE,
		RIT_KEYLOW,
		RIT_KEYHIGH,
		RIT_VELLOW,
		RIT_VELHIGH,
		RIT_PAN,
		RIT_VOL,
		RIT_SAMPNUM
	};

	VGMRgnItem(VGMRgn *rgn, RgnItemType theType, uint32 offset, uint32 length,
			   const Common::String &name);

public:
	RgnItemType _type;
};

#endif // AUDIO_SOUNDFONT_VGMITEM_H
