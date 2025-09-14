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

#include "engines/advancedDetector.h"

#include "teenagent/resources.h"
#include "teenagent/teenagent.h"
#include "common/debug.h"
#include "common/textconsole.h"
#include "common/translation.h"
#include "common/compression/deflate.h"

namespace TeenAgent {

Resources::Resources() {
	_dialogsStartOffset = 0;
	_sceneObjectsStartOffset = 0;
	_sceneObjectsBlockSize = 0;
}

Resources::~Resources() {
	off.close();
	on.close();
	ons.close();
	lan000.close();
	lan500.close();
	mmm.close();
	sam_mmm.close();
	sam_sam.close();
	voices.close();
}

/*
quick note on varia resources:
1: Mark's animations (with head)
2: Mark's idle animation
3: Inventory background
4: Inventory items
5: Metropolis palette
6: TEENAGENT logo (flames)
7: Small font
8: Bigger font
9: Metropolis software house
10: quit registered
11: quit shareware
*/

#define CSEG_SIZE 46000 // 0xb3b0
#define DSEG_SIZE 59280 // 0xe790
#define ESEG_SIZE 35810 // 0x8be2

void Resources::precomputeResourceOffsets(const ResourceInfo &resInfo, Common::Array<uint32> &offsets, uint numTerminators) {
	offsets.push_back(resInfo._offset);
	uint n = 0;
	uint8 current, last = 0xff;
	for (uint32 i = resInfo._offset; i < resInfo._offset + resInfo._size; i++) {
		current = eseg.get_byte(i);

		if (n == numTerminators) {
			offsets.push_back(i);
			n = 0;
		}

		if (current != 0x00 && last == 0x00)
			n = 0;

		if (current == 0x00)
			n++;

		last = current;
	}
}

void Resources::precomputeDialogOffsets(const ResourceInfo &resInfo) {
	precomputeResourceOffsets(resInfo, dialogOffsets, 4);

	debug(1, "Resources::precomputeDialogOffsets() - Found %d dialogs", dialogOffsets.size());
	for (uint i = 0; i < dialogOffsets.size(); i++)
		debug(1, "\tDialog #%d: Offset 0x%04x", i, dialogOffsets[i]);
}

void Resources::precomputeCreditsOffsets(const ResourceInfo &resInfo) {
	precomputeResourceOffsets(resInfo, creditsOffsets);

	debug(1, "Resources::precomputeCreditsOffsets() - Found %d credits", creditsOffsets.size());
	for (uint i = 0; i < creditsOffsets.size(); i++)
		debug(1, "\tCredit #%d: Offset 0x%04x", i, creditsOffsets[i]);
}

void Resources::precomputeItemOffsets(const ResourceInfo &resInfo) {
	precomputeResourceOffsets(resInfo, itemOffsets);

	debug(1, "Resources::precomputeItemOffsets() - Found %d items", itemOffsets.size());
	for (uint i = 0; i < itemOffsets.size(); i++)
		debug(1, "\tItem #%d: Offset 0x%04x", i, itemOffsets[i]);
}

void Resources::precomputeMessageOffsets(const ResourceInfo &resInfo) {
	precomputeResourceOffsets(resInfo, messageOffsets);
}

void Resources::precomputeCombinationOffsets(const ResourceInfo &resInfo) {
	precomputeResourceOffsets(resInfo, combinationOffsets);

	debug(1, "Resources::precomputeCombinationOffsets() - Found %d combination items", combinationOffsets.size());
	for (uint i = 0; i < combinationOffsets.size(); i++)
		debug(1, "\tCombination #%d: Offset 0x%04x", i, combinationOffsets[i]);
}

void Resources::readDialogStacks(byte *src) {
	uint16 base = dsAddr_dialogStackPleadingToMansionGuard;

	byte dialogStackWritten = 0;
	uint i = 0;

	while (dialogStackWritten < kNumDialogStacks) {
		uint16 word = READ_LE_UINT16(src + i * 2);
		dseg.set_word(base + i * 2, word);
		if (word == 0xFFFF)
			dialogStackWritten++;
		i++;
	}
}

void Resources::precomputeAllOffsets(const Common::Array<ResourceInfo> &resourceInfos) {
	for (const auto &resInfo : resourceInfos) {
		switch ((ResourceType)resInfo._id) {
		case kResCombinations:
			precomputeCombinationOffsets(resInfo);
			break;
		case kResCredits:
			precomputeCreditsOffsets(resInfo);
			break;
		case kResDialogs:
			_dialogsStartOffset = resInfo._offset;
			precomputeDialogOffsets(resInfo);
			break;
		case kResItems:
			precomputeItemOffsets(resInfo);
			break;
		case kResMessages:
			precomputeMessageOffsets(resInfo);
			break;
		case kResSceneObjects:
			_sceneObjectsStartOffset = resInfo._offset;
			_sceneObjectsBlockSize = resInfo._size;
			break;
		case kResDialogStacks:
		// fall through
		default:
			break;
		}
	}
}

bool Resources::isVoiceIndexEmpty(uint16 index) {
	uint size = voices.getSize(index);
	if (size == 4 || size == 5)
		return true;
	return false;
}

void Resources::precomputeVoiceIndices(const Common::Array<ResourceInfo>& resourceInfos) {
	byte numTerminators = 0;
	uint16 voiceIndex = 0;

	for (auto &resInfo : resourceInfos) {
		switch ((ResourceType)resInfo._id) {
		case kResMessages:
			voiceIndex = 1;
			numTerminators = 2;
			break;
		case kResCombinations:
			voiceIndex = 567;
			numTerminators = 2;
			break;
		case kResItems:
			voiceIndex = 592;
			numTerminators = 2;
			break;
		case kResDialogs:
			voiceIndex = 902;
			numTerminators = 4;
			break;
		case kResCredits:
		case kResDialogStacks:
		case kResSceneObjects:
			// There are no voiceovers for credits and dialog stacks.
			// For scene objects, voice indices calculated separately
			// in Scene::loadObjectData()
			continue;
		default:
			break;
		}

		_addrToVoiceIndx[resInfo._offset] = voiceIndex++;

		uint16 currentNum = 1;
		uint n = 0; // number of consecutive zero bytes
		byte current, last = 0xff;

		bool setNoIMessage = false;

		for (uint32 i = resInfo._offset; i < resInfo._offset + resInfo._size; i++) {
			current = eseg.get_byte(i);

			if (n == numTerminators) {
				currentNum++;
				n = 0;

				if ((ResourceType)resInfo._id == kResCombinations) {
					uint16 nthCombination = currentNum - 1;
					// For dublicate combination messages don't increment voice index
					if (nthCombination == 3 || nthCombination == 5 ||
						nthCombination == 15 || nthCombination == 16 || nthCombination == 17 ||
						nthCombination == 18 || nthCombination == 22 || nthCombination == 26) {
						_addrToVoiceIndx[i] = voiceIndex - 1;
					} else if (nthCombination == 28) {
						_addrToVoiceIndx[i] = voiceIndex - 2;
					} else {
						_addrToVoiceIndx[i] = voiceIndex++;
					}
				} else if ((ResourceType)resInfo._id == kResDialogs) {
					if (voiceIndex == 1416) {
						// "Dzie= dobry, panie robocie." starts at 1418
						voiceIndex += 2;
						_addrToVoiceIndx[i] = voiceIndex++;
					} else if (voiceIndex == 1864) {
						// "Jak ju< powiedzia%em, nasza organizacja" starts at 1867
						voiceIndex += 3;
						_addrToVoiceIndx[i] = voiceIndex++;
					} else if (isVoiceIndexEmpty(voiceIndex)) {
						voiceIndex += 1;
						if (current != 0x00)
							_addrToVoiceIndx[i] = voiceIndex++;
					} else if (voiceIndex == 1801) {
						_addrToVoiceIndx[i] = 2041; // "]adna pogoda."
					} else if (voiceIndex == 1809) {
						_addrToVoiceIndx[i] = 2042; // "Sir, mamy sygna%y, <e..."
					} else {
						if (current != 0x00)
							_addrToVoiceIndx[i] = voiceIndex++;
					}
				} else if ((ResourceType)resInfo._id == kResMessages) {
					if (currentNum == 334) { // Combination error message
						// HACK: Use most good sounding (sigh) version
						// TODO: Find the correct voice index used in the original
						_addrToVoiceIndx[i] = 1304;
					} else
						_addrToVoiceIndx[i] = voiceIndex++;
				} else {
					_addrToVoiceIndx[i] = voiceIndex++;
				}
			}

			if (current != 0x00 && last == 0x00) {
				if ((ResourceType)resInfo._id == kResDialogs) {
					if (n == 2 || n == 3) {
						// "...to czemu nie u<y^ dziwnych" at 1886
						// "Sze$^ miesi#cy temu z%oto i got*wka" at 1921
						if (voiceIndex == 1885 || voiceIndex == 1920 || isVoiceIndexEmpty(voiceIndex)) {
							voiceIndex += 1;
							_addrToVoiceIndx[i] = voiceIndex++;
						} else if (voiceIndex == 1923 && !setNoIMessage) {
							_addrToVoiceIndx[i] = 1885; // "No i?..."
							setNoIMessage = true;
						} else {
							_addrToVoiceIndx[i] = voiceIndex++;
						}
					} else if (n == 1 && (voiceIndex == 1720 || voiceIndex == 1852)) {
						// Because of the rare case with
						// NEW_LINE at the beginning of dialogs 163, 190
						// we have to assign voiceIndex here
						_addrToVoiceIndx[i] = voiceIndex++;
					}
				}
				n = 0;
			}

			if (current == 0x00)
				n++;

			last = current;
		}
	}
}

bool Resources::loadArchives(const ADGameDescription *gd) {
	Common::File *dat_file = new Common::File();
	Common::String filename = "teenagent.dat";
	if (!dat_file->open(filename.c_str())) {
		delete dat_file;

		const char *msg = _s("Unable to locate the '%s' engine data file.");
		Common::U32String errorMessage = Common::U32String::format(_(msg), filename.c_str());
		warning(msg, filename.c_str());
		GUIErrorMessage(errorMessage);
		return false;
	}

	// teenagent.dat used to be compressed with zlib compression. The usage of
	// zlib here is no longer needed, and it's maintained only for backwards
	// compatibility.
	Common::SeekableReadStream *dat = Common::wrapCompressedReadStream(dat_file);

	byte tempBuffer[256];
	dat->read(tempBuffer, 9);
	tempBuffer[9] = '\0';

	if (strcmp((char *)tempBuffer, "TEENAGENT") != 0) {
		const char *msg = _s("The '%s' engine data file is corrupt.");
		Common::U32String errorMessage = Common::U32String::format(_(msg), filename.c_str());
		GUIErrorMessage(errorMessage);
		warning(msg, filename.c_str());
		return false;
	}

	byte version = dat->readByte();
	if (version != TEENAGENT_DAT_VERSION) {
		const char *msg = _s("Incorrect version of the '%s' engine data file found. Expected %d but got %d.");
		Common::U32String errorMessage = Common::U32String::format(_(msg), filename.c_str(), TEENAGENT_DAT_VERSION, version);
		GUIErrorMessage(errorMessage);
		warning(msg, filename.c_str(), TEENAGENT_DAT_VERSION, version);
		return false;
	}

	dat->skip(CSEG_SIZE);
	dseg.read(dat, DSEG_SIZE);

	// Locate the correct language block
	bool found = false;

	while (!found) {
		dat->read(tempBuffer, 5);
		if (tempBuffer[0] == 0xff) {
			error("Could not locate correct language block");
		}

		if (gd->language == tempBuffer[0]) {
			found = true;
			uint32 dataOffset = READ_LE_UINT32(&tempBuffer[1]);
			dat->seek(dataOffset);
		}
	}

	Common::Array<ResourceInfo> resourceInfos(kNumResources);
	uint32 allResourcesSize = 0;

	for (auto &resInfo : resourceInfos) {
		resInfo._id = dat->readByte();
		resInfo._offset = dat->readUint32LE();
		resInfo._size = dat->readUint32LE();

		// Don't count Dialog stack's size
		// since it will be stored in dseg, not eseg
		if ((ResourceType)resInfo._id != kResDialogStacks)
			allResourcesSize += resInfo._size;
	}

	// Dialog stack data
	dat->read(tempBuffer, resourceInfos[(uint)kResDialogStacks]._size);
	readDialogStacks((byte *)tempBuffer);

	// Store rest of the resources to eseg
	eseg.read(dat, allResourcesSize);

	delete dat;

	precomputeAllOffsets(resourceInfos);

	FilePack varia;
	varia.open("varia.res");
	font7.load(varia, 7, 11, 1);
	font8.load(varia, 8, 31, 0);
	varia.close();

	off.open("off.res");
	on.open("on.res");
	ons.open("ons.res");
	lan000.open("lan_000.res");
	lan500.open("lan_500.res");
	mmm.open("mmm.res");
	sam_mmm.open("sam_mmm.res");
	sam_sam.open("sam_sam.res");
	voices.open("voices.res");

	if (gd->language == Common::PL_POL)
		precomputeVoiceIndices(resourceInfos);

	return true;
}

void Resources::loadOff(Graphics::Surface &surface, byte *palette, int id) {
	uint32 size = off.getSize(id);
	if (size == 0) {
		error("invalid background %d", id);
		return;
	}

	const uint bufferSize = 64768;
	byte *buf = (byte *)malloc(bufferSize);
	if (!buf)
		error("[Resources::loadOff] Cannot allocate buffer");

	off.read(id, buf, bufferSize);

	byte *src = buf;
	byte *dst = (byte *)surface.getPixels();
	memcpy(dst, src, 64000);
	memcpy(palette, buf + 64000, 768);

	free(buf);
}

Common::SeekableReadStream *Resources::loadLan(uint32 id) const {
	return id <= 500 ? loadLan000(id) : lan500.getStream(id - 500);
}

Common::SeekableReadStream *Resources::loadLan000(uint32 id) const {
	switch (id) {
	case 81:
		if (dseg.get_byte(dsAddr_dogHasBoneFlag))
			return lan500.getStream(160);
		break;

	case 137:
		if (dseg.get_byte(dsAddr_mansionTVOnFlag) == 1) {
			if (dseg.get_byte(dsAddr_mansionVCRPlayingTapeFlag) == 1)
				return lan500.getStream(203);
			else
				return lan500.getStream(202);
		}
		break;

	case 25:
		if (dseg.get_byte(dsAddr_FirstActTrialState) == 2) {
			return lan500.getStream(332);
		}
		break;

	case 37:
		if (dseg.get_byte(dsAddr_act1GuardState) == 1) {
			return lan500.getStream(351);
		} else if (dseg.get_byte(dsAddr_act1GuardState) == 2) {
			return lan500.getStream(364);
		}
		break;

	case 29:
		if (dseg.get_byte(dsAddr_birdOnBarRadioAntennaFlag) == 1) {
			return lan500.getStream(380);
		}
		break;

	case 30:
		if (dseg.get_byte(dsAddr_birdOnBarRadioAntennaFlag) == 1) {
			return lan500.getStream(381);
		}
		break;

	case 42:
		if (dseg.get_byte(dsAddr_johnNotyOutsideMansionDoorFlag) == 1) {
			return lan500.getStream(400);
		}
		break;

	default:
		break;
	}
	return lan000.getStream(id);
}

} // End of namespace TeenAgent
