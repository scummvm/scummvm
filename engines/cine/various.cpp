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
 * $URL$
 * $Id$
 *
 */


#include "common/endian.h"
#include "common/events.h"
#include "common/savefile.h"

#include "cine/cine.h"
#include "cine/main_loop.h"
#include "cine/object.h"
#include "cine/sound.h"
#include "cine/bg_list.h"
#include "cine/various.h"

namespace Cine {

bool disableSystemMenu = false;
bool inMenu;

int16 commandVar3[4];
int16 commandVar1;
int16 commandVar2;

//Message messageTable[NUM_MAX_MESSAGE];

uint16 var2;
uint16 var3;
uint16 var4;
uint16 var5;

int16 buildObjectListCommand(int16 param);
int16 canUseOnObject = 0;

void drawString(const char *string, byte param) {
}

void waitPlayerInput(void) {
}

void setTextWindow(uint16 param1, uint16 param2, uint16 param3, uint16 param4) {
}

uint16 errorVar;
byte menuVar;

bool fadeRequired;
uint16 allowPlayerInput;
uint16 checkForPendingDataLoadSwitch;
uint16 isDrawCommandEnabled;
uint16 waitForPlayerClick;
uint16 menuCommandLen;
bool _paletteNeedUpdate;
uint16 _messageLen;
byte _danKeysPressed;

int16 playerCommand;

char commandBuffer[80];
char currentPrcName[20];
char currentRelName[20];
char currentObjectName[20];
char currentMsgName[20];
char newPrcName[20];
char newRelName[20];
char newObjectName[20];
char newMsgName[20];
char currentCtName[15];
char currentPartName[15];
char currentDatName[30];

int16 saveVar2;

byte isInPause = 0;

// TODO: Implement inputVar0's changes in the program
// Currently inputVar0 isn't updated anywhere even though it's used at least in processSeqListElement.
uint16 inputVar0 = 0;
byte inputVar1 = 0;
uint16 inputVar2 = 0, inputVar3 = 0;

SelectedObjStruct currentSelectedObject;

static CommandeType currentSaveName[10];
int16 currentDisk;

static const int16 choiceResultTable[] = { 1, 1, 1, 2, 1, 1, 1 };
static const int16 subObjectUseTable[] = { 3, 3, 3, 3, 3, 0, 0 };
static const int16 canUseOnItemTable[] = { 1, 0, 0, 1, 1, 0, 0 };

CommandeType objectListCommand[20];
int16 objListTab[20];

uint16 exitEngine;
uint16 zoneData[NUM_MAX_ZONE];
uint16 zoneQuery[NUM_MAX_ZONE]; //!< Only exists in Operation Stealth


void stopMusicAfterFadeOut(void) {
//	if (g_sfxPlayer->_fadeOutCounter != 0 && g_sfxPlayer->_fadeOutCounter < 100) {
//		g_sfxPlayer->stop();
//	}
}

void runObjectScript(int16 entryIdx) {
	ScriptPtr tmp(scriptInfo->create(*relTable[entryIdx], entryIdx));
	assert(tmp);
	objectScripts.push_back(tmp);
}

/*! \brief Add action result message to overlay list
 * \param cmd Message description
 * \todo Why are x, y, width and color left uninitialized?
 */
void addPlayerCommandMessage(int16 cmd) {
	overlay tmp;
	memset(&tmp, 0, sizeof(tmp));
	tmp.objIdx = cmd;
	tmp.type = 3;

	overlayList.push_back(tmp);
	waitForPlayerClick = 1;
}

int16 getRelEntryForObject(uint16 param1, uint16 param2, SelectedObjStruct *pSelectedObject) {
	int16 i;
	int16 found = -1;

	for (i = 0; i < (int16)relTable.size(); i++) {
		if (relTable[i]->_param1 == param1 && relTable[i]->_param2 == pSelectedObject->idx) {
			if (param2 == 1) {
				found = i;
			} else if (param2 == 2) {
				if (relTable[i]->_param3 == pSelectedObject->param) {
					found = i;
				}
			}
		}

		if (found != -1)
			break;
	}

	return found;
}

/*! \brief Find index of the object under cursor
 * \param x Mouse cursor coordinate
 * \param y Mouse cursor coordinate
 * \todo Fix displaced type 1 objects
 */
int16 getObjectUnderCursor(uint16 x, uint16 y) {
	Common::List<overlay>::iterator it;

	int16 objX, objY, frame, part, threshold, height, xdif, ydif;
	int width;

	// reverse_iterator would be nice
	for (it = overlayList.reverse_begin(); it != overlayList.end(); --it) {
		if (it->type >= 2 || !objectTable[it->objIdx].name[0]) {
			continue;
		}

		objX = objectTable[it->objIdx].x;
		objY = objectTable[it->objIdx].y;

		frame = ABS((int16)(objectTable[it->objIdx].frame));
		part = objectTable[it->objIdx].part;

		if (it->type == 0) {
			threshold = animDataTable[frame]._var1;
		} else {
			threshold = animDataTable[frame]._width / 2;
		}

		height = animDataTable[frame]._height;
		width = animDataTable[frame]._realWidth;

		xdif = x - objX;
		ydif = y - objY;

		if ((xdif < 0) || ((threshold << 4) <= xdif) || (ydif < 0) || (ydif >= height) || !animDataTable[frame].data()) {
			continue;
		}

		if (g_cine->getGameType() == Cine::GType_OS) {
			if (xdif >= width) {
				continue;
			}

			if (it->type == 0 && animDataTable[frame].getColor(xdif, ydif) != part) {
				return it->objIdx;
			} else if (it->type == 1 && gfxGetBit(xdif, ydif, animDataTable[frame].data(), animDataTable[frame]._width * 4)) {
				return it->objIdx;
			}
		} else if (it->type == 0)	{ // use generated mask
			if (gfxGetBit(xdif, ydif, animDataTable[frame].mask(), animDataTable[frame]._width)) {
				return it->objIdx;
			}
		} else if (it->type == 1) { // is mask
			if (gfxGetBit(xdif, ydif, animDataTable[frame].data(), animDataTable[frame]._width * 4)) {
				return it->objIdx;
			}
		}
	}

	return -1;
}

bool writeChunkHeader(Common::OutSaveFile &out, const ChunkHeader &header) {
	out.writeUint32BE(header.id);
	out.writeUint32BE(header.version);
	out.writeUint32BE(header.size);
	return !out.ioFailed();
}

bool loadChunkHeader(Common::SeekableReadStream &in, ChunkHeader &header) {
	header.id      = in.readUint32BE();
	header.version = in.readUint32BE();
	header.size    = in.readUint32BE();
	return !in.ioFailed();
}

void saveObjectTable(Common::OutSaveFile &out) {
	out.writeUint16BE(NUM_MAX_OBJECT); // Entry count
	out.writeUint16BE(0x20); // Entry size

	for (int i = 0; i < NUM_MAX_OBJECT; i++) {
		out.writeUint16BE(objectTable[i].x);
		out.writeUint16BE(objectTable[i].y);
		out.writeUint16BE(objectTable[i].mask);
		out.writeUint16BE(objectTable[i].frame);
		out.writeUint16BE(objectTable[i].costume);
		out.write(objectTable[i].name, 20);
		out.writeUint16BE(objectTable[i].part);
	}
}

void saveZoneData(Common::OutSaveFile &out) {
	for (int i = 0; i < 16; i++) {
		out.writeUint16BE(zoneData[i]);
	}
}

void saveCommandVariables(Common::OutSaveFile &out) {
	for (int i = 0; i < 4; i++) {
		out.writeUint16BE(commandVar3[i]);
	}
}

void saveAnimDataTable(Common::OutSaveFile &out) {
	out.writeUint16BE(NUM_MAX_ANIMDATA); // Entry count
	out.writeUint16BE(0x1E); // Entry size

	for (int i = 0; i < NUM_MAX_ANIMDATA; i++) {
		animDataTable[i].save(out);
	}
}

void saveScreenParams(Common::OutSaveFile &out) {
	// Screen parameters, unhandled
	out.writeUint16BE(0);
	out.writeUint16BE(0);
	out.writeUint16BE(0);
	out.writeUint16BE(0);
	out.writeUint16BE(0);
	out.writeUint16BE(0);
}

void saveGlobalScripts(Common::OutSaveFile &out) {
	ScriptList::const_iterator it;
	out.writeUint16BE(globalScripts.size());
	for (it = globalScripts.begin(); it != globalScripts.end(); ++it) {
		(*it)->save(out);
	}
}

void saveObjectScripts(Common::OutSaveFile &out) {
	ScriptList::const_iterator it;
	out.writeUint16BE(objectScripts.size());
	for (it = objectScripts.begin(); it != objectScripts.end(); ++it) {
		(*it)->save(out);
	}
}

void saveOverlayList(Common::OutSaveFile &out) {
	Common::List<overlay>::const_iterator it;

	out.writeUint16BE(overlayList.size());

	for (it = overlayList.begin(); it != overlayList.end(); ++it) {
		out.writeUint32BE(0); // next
		out.writeUint32BE(0); // previous?
		out.writeUint16BE(it->objIdx);
		out.writeUint16BE(it->type);
		out.writeSint16BE(it->x);
		out.writeSint16BE(it->y);
		out.writeSint16BE(it->width);
		out.writeSint16BE(it->color);
	}
}

void saveBgIncrustList(Common::OutSaveFile &out) {
	Common::List<BGIncrust>::const_iterator it;
	out.writeUint16BE(bgIncrustList.size());

	for (it = bgIncrustList.begin(); it != bgIncrustList.end(); ++it) {
		out.writeUint32BE(0); // next
		out.writeUint32BE(0); // previous?
		out.writeUint16BE(it->objIdx);
		out.writeUint16BE(it->param);
		out.writeUint16BE(it->x);
		out.writeUint16BE(it->y);
		out.writeUint16BE(it->frame);
		out.writeUint16BE(it->part);
	}
}

void saveZoneQuery(Common::OutSaveFile &out) {
	for (int i = 0; i < 16; i++) {
		out.writeUint16BE(zoneQuery[i]);
	}
}

void saveSeqList(Common::OutSaveFile &out) {
	Common::List<SeqListElement>::const_iterator it;
	out.writeUint16BE(seqList.size());

	for (it = seqList.begin(); it != seqList.end(); ++it) {
		out.writeSint16BE(it->var4);
		out.writeUint16BE(it->objIdx);
		out.writeSint16BE(it->var8);
		out.writeSint16BE(it->frame);
		out.writeSint16BE(it->varC);
		out.writeSint16BE(it->varE);
		out.writeSint16BE(it->var10);
		out.writeSint16BE(it->var12);
		out.writeSint16BE(it->var14);
		out.writeSint16BE(it->var16);
		out.writeSint16BE(it->var18);
		out.writeSint16BE(it->var1A);
		out.writeSint16BE(it->var1C);
		out.writeSint16BE(it->var1E);
	}
}

bool CineEngine::loadSaveDirectory(void) {
	Common::InSaveFile *fHandle;
	char tmp[80];

	snprintf(tmp, 80, "%s.dir", _targetName.c_str());
	fHandle = g_saveFileMan->openForLoading(tmp);

	if (!fHandle) {
		return false;
	}

	fHandle->read(currentSaveName, 10 * 20);
	delete fHandle;

	return true;
}

/*! \brief Savegame format detector
 * \param fHandle Savefile to check
 * \return Savegame format on success, ANIMSIZE_UNKNOWN on failure
 *
 * This function seeks through the savefile and tries to determine the
 * savegame format it uses. There's a miniscule chance that the detection
 * algorithm could get confused and think that the file uses both the older
 * and the newer format but that is such a remote possibility that I wouldn't
 * worry about it at all.
 *
 * Also detects the temporary Operation Stealth savegame format now.
 */
enum CineSaveGameFormat detectSaveGameFormat(Common::SeekableReadStream &fHandle) {
	const uint32 prevStreamPos = fHandle.pos();

	// First check for the temporary Operation Stealth savegame format.
	fHandle.seek(0);
	ChunkHeader hdr;
	loadChunkHeader(fHandle, hdr);
	fHandle.seek(prevStreamPos);
	if (hdr.id == TEMP_OS_FORMAT_ID) {
		return TEMP_OS_FORMAT;
	}

	// Ok, so the savegame isn't using the temporary Operation Stealth savegame format.
	// Let's check for the plain Future Wars savegame format and its different versions then.
	// The animDataTable begins at savefile position 0x2315.
	// Each animDataTable entry takes 23 bytes in older saves (Revisions 21772-31443)
	// and 30 bytes in the save format after that (Revision 31444 and onwards).
	// There are 255 entries in the animDataTable in both of the savefile formats.
	static const uint animDataTableStart = 0x2315;
	static const uint animEntriesCount = 255;
	static const uint oldAnimEntrySize = 23;
	static const uint newAnimEntrySize = 30;
	static const uint animEntrySizeChoices[] = {oldAnimEntrySize, newAnimEntrySize};
	Common::Array<uint> animEntrySizeMatches;

	// Try to walk through the savefile using different animDataTable entry sizes
	// and make a list of all the successful entry sizes.
	for (uint i = 0; i < ARRAYSIZE(animEntrySizeChoices); i++) {
		// 206 = 2 * 50 * 2 + 2 * 3 (Size of global and object script entries)
		// 20 = 4 * 2 + 2 * 6 (Size of overlay and background incrust entries)
		static const uint sizeofScreenParams = 2 * 6;
		static const uint globalScriptEntrySize = 206;
		static const uint objectScriptEntrySize = 206;
		static const uint overlayEntrySize = 20;
		static const uint bgIncrustEntrySize = 20;
		static const uint chainEntrySizes[] = {
			globalScriptEntrySize,
			objectScriptEntrySize,
			overlayEntrySize,
			bgIncrustEntrySize
		};
		
		uint animEntrySize = animEntrySizeChoices[i];
		// Jump over the animDataTable entries and the screen parameters
		uint32 newPos = animDataTableStart + animEntrySize * animEntriesCount + sizeofScreenParams;
		// Check that there's data left after the point we're going to jump to
		if (newPos >= fHandle.size()) {
			continue;
		}
		fHandle.seek(newPos);

		// Jump over the remaining items in the savegame file
		// (i.e. the global scripts, object scripts, overlays and background incrusts).
		bool chainWalkSuccess = true;
		for (uint chainIndex = 0; chainIndex < ARRAYSIZE(chainEntrySizes); chainIndex++) {
			// Read entry count and jump over the entries
			int entryCount = fHandle.readSint16BE();
			newPos = fHandle.pos() + chainEntrySizes[chainIndex] * entryCount;
			// Check that we didn't go past the end of file.
			// Note that getting exactly to the end of file is acceptable.
			if (newPos > fHandle.size()) {
				chainWalkSuccess = false;
				break;
			}
			fHandle.seek(newPos);
		}
		
		// If we could walk the chain successfully and
		// got exactly to the end of file then we've got a match.
		if (chainWalkSuccess && fHandle.pos() == fHandle.size()) {
			// We found a match, let's save it
			animEntrySizeMatches.push_back(animEntrySize);
		}
	}

	// Check that we got only one entry size match.
	// If we didn't, then return an error.
	enum CineSaveGameFormat result = ANIMSIZE_UNKNOWN;
	if (animEntrySizeMatches.size() == 1) {
		const uint animEntrySize = animEntrySizeMatches[0];
		assert(animEntrySize == oldAnimEntrySize || animEntrySize == newAnimEntrySize);
		if (animEntrySize == oldAnimEntrySize) {
			result = ANIMSIZE_23;
		} else { // animEntrySize == newAnimEntrySize		
			// Check data and mask pointers in all of the animDataTable entries
			// to see whether we've got the version with the broken data and mask pointers or not.
			// In the broken format all data and mask pointers were always zero.
			static const uint relativeDataPos = 2 * 4;
			bool pointersIntact = false;
			for (uint i = 0; i < animEntriesCount; i++) {
				fHandle.seek(animDataTableStart + i * animEntrySize + relativeDataPos);
				uint32 data = fHandle.readUint32BE();
				uint32 mask = fHandle.readUint32BE();
				if ((data != 0) || (mask != 0)) {
					pointersIntact = true;
					break;
				}
			}
			result = (pointersIntact ? ANIMSIZE_30_PTRS_INTACT : ANIMSIZE_30_PTRS_BROKEN);
		}
	} else if (animEntrySizeMatches.size() > 1) {
		warning("Savegame format detector got confused by input data. Detecting savegame to be using an unknown format");
	} else { // animEtrySizeMatches.size() == 0
		debug(3, "Savegame format detector was unable to detect savegame's format");
	}

	fHandle.seek(prevStreamPos);
	return result;
}

/*! \brief Restore script list item from savefile
 * \param fHandle Savefile handle open for reading
 * \param isGlobal Restore object or global script?
 */
void loadScriptFromSave(Common::SeekableReadStream &fHandle, bool isGlobal) {
	ScriptVars localVars, labels;
	uint16 compare, pos;
	int16 idx;

	labels.load(fHandle);
	localVars.load(fHandle);

	compare = fHandle.readUint16BE();
	pos = fHandle.readUint16BE();
	idx = fHandle.readUint16BE();

	// no way to reinitialize these
	if (idx < 0) {
		return;
	}

	// original code loaded everything into globalScripts, this should be
	// the correct behavior
	if (isGlobal) {
		ScriptPtr tmp(scriptInfo->create(*scriptTable[idx], idx, labels, localVars, compare, pos));
		assert(tmp);
		globalScripts.push_back(tmp);
	} else {
		ScriptPtr tmp(scriptInfo->create(*relTable[idx], idx, labels, localVars, compare, pos));
		assert(tmp);
		objectScripts.push_back(tmp);
	}
}

/*! \brief Restore overlay sprites from savefile
 * \param fHandle Savefile open for reading
 */
void loadOverlayFromSave(Common::SeekableReadStream &fHandle) {
	overlay tmp;

	fHandle.readUint32BE();
	fHandle.readUint32BE();

	tmp.objIdx = fHandle.readUint16BE();
	tmp.type = fHandle.readUint16BE();
	tmp.x = fHandle.readSint16BE();
	tmp.y = fHandle.readSint16BE();
	tmp.width = fHandle.readSint16BE();
	tmp.color = fHandle.readSint16BE();

	overlayList.push_back(tmp);
}

void CineEngine::resetEngine() {
	g_sound->stopMusic();
	freeAnimDataTable();
	overlayList.clear();
	bgIncrustList.clear();
	closePart();

	objectScripts.clear();
	globalScripts.clear();
	relTable.clear();
	scriptTable.clear();
	messageTable.clear();

	for (int i = 0; i < NUM_MAX_OBJECT; i++) {
		objectTable[i].x = 0;
		objectTable[i].y = 0;
		objectTable[i].part = 0;
		objectTable[i].name[0] = 0;
		objectTable[i].frame = 0;
		objectTable[i].mask = 0;
		objectTable[i].costume = 0;
	}

	globalVars.reset();

	var2 = var3 = var4 = var5 = 0;

	strcpy(newPrcName, "");
	strcpy(newRelName, "");
	strcpy(newObjectName, "");
	strcpy(newMsgName, "");
	strcpy(currentCtName, "");

	allowPlayerInput = 0;
	waitForPlayerClick = 0;
	playerCommand = -1;
	isDrawCommandEnabled = 0;

	strcpy(commandBuffer, "");

	globalVars[VAR_MOUSE_X_POS] = 0;
	globalVars[VAR_MOUSE_Y_POS] = 0;

	fadeRequired = false;

	renderer->clear();

	checkForPendingDataLoadSwitch = 0;

	if (g_cine->getGameType() == Cine::GType_OS) {
		seqList.clear();
		currentAdditionalBgIdx = 0;
		currentAdditionalBgIdx2 = 0;
		// TODO: Add resetting of the following variables
		// adBgVar1 = 0;
		// adBgVar0 = 0;		
		// gfxFadeOutCompleted = 0;
	}
}

bool loadObjectTable(Common::SeekableReadStream &in) {
	in.readUint16BE(); // Entry count
	in.readUint16BE(); // Entry size

	for (int i = 0; i < NUM_MAX_OBJECT; i++) {
		objectTable[i].x = in.readSint16BE();
		objectTable[i].y = in.readSint16BE();
		objectTable[i].mask = in.readUint16BE();
		objectTable[i].frame = in.readSint16BE();
		objectTable[i].costume = in.readSint16BE();
		in.read(objectTable[i].name, 20);
		objectTable[i].part = in.readUint16BE();
	}
	return !in.ioFailed();
}

bool loadZoneData(Common::SeekableReadStream &in) {
	for (int i = 0; i < 16; i++) {
		zoneData[i] = in.readUint16BE();
	}
	return !in.ioFailed();
}

bool loadCommandVariables(Common::SeekableReadStream &in) {
	for (int i = 0; i < 4; i++) {
		commandVar3[i] = in.readUint16BE();
	}
	return !in.ioFailed();
}

bool loadScreenParams(Common::SeekableReadStream &in) {
	// TODO: handle screen params (really required ?)
	in.readUint16BE();
	in.readUint16BE();
	in.readUint16BE();
	in.readUint16BE();
	in.readUint16BE();
	in.readUint16BE();
	return !in.ioFailed();
}

bool loadGlobalScripts(Common::SeekableReadStream &in) {
	int size = in.readSint16BE();
	for (int i = 0; i < size; i++) {
		loadScriptFromSave(in, true);
	}
	return !in.ioFailed();
}

bool loadObjectScripts(Common::SeekableReadStream &in) {
	int size = in.readSint16BE();
	for (int i = 0; i < size; i++) {
		loadScriptFromSave(in, false);
	}
	return !in.ioFailed();
}

bool loadOverlayList(Common::SeekableReadStream &in) {
	int size = in.readSint16BE();
	for (int i = 0; i < size; i++) {
		loadOverlayFromSave(in);
	}
	return !in.ioFailed();
}

bool loadSeqList(Common::SeekableReadStream &in) {
	uint size = in.readUint16BE();
	SeqListElement tmp;
	for (uint i = 0; i < size; i++) {
		tmp.var4   = in.readSint16BE();
		tmp.objIdx = in.readUint16BE();
		tmp.var8   = in.readSint16BE();
		tmp.frame  = in.readSint16BE();
		tmp.varC   = in.readSint16BE();
		tmp.varE   = in.readSint16BE();
		tmp.var10  = in.readSint16BE();
		tmp.var12  = in.readSint16BE();
		tmp.var14  = in.readSint16BE();
		tmp.var16  = in.readSint16BE();
		tmp.var18  = in.readSint16BE();
		tmp.var1A  = in.readSint16BE();
		tmp.var1C  = in.readSint16BE();
		tmp.var1E  = in.readSint16BE();
		seqList.push_back(tmp);
	}
	return !in.ioFailed();
}

bool loadZoneQuery(Common::SeekableReadStream &in) {
	for (int i = 0; i < 16; i++) {
		zoneQuery[i] = in.readUint16BE();
	}
	return !in.ioFailed();
}

bool CineEngine::loadTempSaveOS(Common::SeekableReadStream &in) {
	char musicName[13];
	char bgNames[8][13];

	// First check the temporary Operation Stealth savegame format header.
	ChunkHeader hdr;
	loadChunkHeader(in, hdr);
	if (hdr.id != TEMP_OS_FORMAT_ID) {
		warning("loadTempSaveOS: File has incorrect identifier. Not loading savegame");
		return false;
	} else if (hdr.version > CURRENT_OS_SAVE_VER) {
		warning("loadTempSaveOS: Detected newer format version. Not loading savegame");
		return false;		
	} else if ((int)hdr.version < (int)CURRENT_OS_SAVE_VER) {
		warning("loadTempSaveOS: Detected older format version. Trying to load nonetheless. Things may break");
	} else { // hdr.id == TEMP_OS_FORMAT_ID && hdr.version == CURRENT_OS_SAVE_VER
		debug(3, "loadTempSaveOS: Found correct header (Both the identifier and version number match).");
	}

	// There shouldn't be any data in the header's chunk currently so it's an error if there is.
	if (hdr.size > 0) {
		warning("loadTempSaveOS: Format header's chunk seems to contain data so format is incorrect. Not loading savegame");
		return false;
	}

	// Ok, so we've got a correct header for a temporary Operation Stealth savegame.
	// Let's start loading the plain savegame data then.
	currentDisk = in.readUint16BE();	
	in.read(currentPartName, 13);
	in.read(currentPrcName, 13);
	in.read(currentRelName, 13);
	in.read(currentMsgName, 13);

	// Load the 8 background names.
	for (uint i = 0; i < 8; i++) {
		in.read(bgNames[i], 13);
	}
	
	in.read(currentCtName, 13);

	// Moved the loading of current procedure, relation,
	// backgrounds and Ct here because if they were at the
	// end of this function then the global scripts loading
	// made an array out of bounds access. In the original
	// game's disassembly these aren't here but at the end.
	// The difference is probably in how we handle loading
	// the global scripts and some other things (i.e. the
	// loading routines aren't exactly the same and subtle
	// semantic differences result in having to do things
	// in a different order).
	{
		// Not sure if this is needed with Operation Stealth...
		checkDataDisk(currentDisk);

		if (strlen(currentPrcName)) {
			loadPrc(currentPrcName);
		}

		if (strlen(currentRelName)) {
			loadRel(currentRelName);
		}

		// Load first background (Uses loadBg)
		if (strlen(bgNames[0])) {
			loadBg(bgNames[0]);
		}

		// Add backgrounds 1-7 (Uses addBackground)
		for (int i = 1; i < 8; i++) {
			if (strlen(bgNames[i])) {
				addBackground(bgNames[i], i);
			}
		}

		if (strlen(currentCtName)) {
			loadCtOS(currentCtName);
		}
	}

	loadObjectTable(in);
	renderer->restorePalette(in);
	globalVars.load(in, NUM_MAX_VAR);
	loadZoneData(in);
	loadCommandVariables(in);
	in.read(commandBuffer, 0x50);
	loadZoneQuery(in);

	// TODO: Use the loaded string (Current music name (String, 13 bytes)).
	in.read(musicName, 13);

	// TODO: Use the loaded value (Is music loaded? (Uint16BE, Boolean)).
	in.readUint16BE();

	// TODO: Use the loaded value (Is music playing? (Uint16BE, Boolean)).
	in.readUint16BE();

	renderer->_cmdY      = in.readUint16BE();	
	in.readUint16BE(); // Some unknown variable that seems to always be zero
	allowPlayerInput     = in.readUint16BE();
	playerCommand        = in.readUint16BE();
	commandVar1          = in.readUint16BE();
	isDrawCommandEnabled = in.readUint16BE();
	var5                 = in.readUint16BE();
	var4                 = in.readUint16BE();
	var3                 = in.readUint16BE();
	var2                 = in.readUint16BE();
	commandVar2          = in.readUint16BE();
	renderer->_messageBg = in.readUint16BE();
	
	// TODO: Use the loaded value (adBgVar1 (Uint16BE)).
	in.readUint16BE();

	currentAdditionalBgIdx = in.readSint16BE();
	currentAdditionalBgIdx2 = in.readSint16BE();

	// TODO: Check whether the scroll value really gets used correctly after this.
	// Note that the backgrounds are loaded only later than this value is set.
	renderer->setScroll(in.readUint16BE());

	// TODO: Use the loaded value (adBgVar0 (Uint16BE). Maybe this means bgVar0?).
	in.readUint16BE();

	disableSystemMenu = in.readUint16BE();

	// TODO: adBgVar1 = 1 here

	// Load the animDataTable entries
	in.readUint16BE(); // Entry count (255 in the PC version of Operation Stealth).
	in.readUint16BE(); // Entry size (36 in the PC version of Operation Stealth).
	loadResourcesFromSave(in, ANIMSIZE_30_PTRS_INTACT);

	loadScreenParams(in);
	loadGlobalScripts(in);
	loadObjectScripts(in);
	loadSeqList(in);
	loadOverlayList(in);
	loadBgIncrustFromSave(in);

	// Left this here instead of moving it earlier in this function with
	// the other current value loadings (e.g. loading of current procedure,
	// current backgrounds etc). Mostly emulating the way we've handled
	// Future Wars savegames and hoping that things work out.
	if (strlen(currentMsgName)) {
		loadMsg(currentMsgName);
	}

	// TODO: Add current music loading and playing here
	// TODO: Palette handling?

	if (in.pos() == in.size()) {
		debug(3, "loadTempSaveOS: Loaded the whole savefile.");		
	} else {
		warning("loadTempSaveOS: Loaded the savefile but didn't exhaust it completely. Something was left over");
	}

	return !in.ioFailed();
}

bool CineEngine::loadPlainSaveFW(Common::SeekableReadStream &in, CineSaveGameFormat saveGameFormat) {
	char bgName[13];

	// At savefile position 0x0000:
	currentDisk = in.readUint16BE();

	// At 0x0002:
	in.read(currentPartName, 13);
	// At 0x000F:
	in.read(currentDatName, 13);

	// At 0x001C:
	saveVar2 = in.readSint16BE();

	// At 0x001E:
	in.read(currentPrcName, 13);
	// At 0x002B:
	in.read(currentRelName, 13);
	// At 0x0038:
	in.read(currentMsgName, 13);
	// At 0x0045:
	in.read(bgName, 13);
	// At 0x0052:
	in.read(currentCtName, 13);

	checkDataDisk(currentDisk);

	if (strlen(currentPartName)) {
		loadPart(currentPartName);
	}

	if (strlen(currentPrcName)) {
		loadPrc(currentPrcName);
	}

	if (strlen(currentRelName)) {
		loadRel(currentRelName);
	}

	if (strlen(bgName)) {
		loadBg(bgName);
	}

	if (strlen(currentCtName)) {
		loadCtFW(currentCtName);
	}

	// At 0x005F:
	loadObjectTable(in);

	// At 0x2043 (i.e. 0x005F + 2 * 2 + 255 * 32):
	renderer->restorePalette(in);

	// At 0x2083 (i.e. 0x2043 + 16 * 2 * 2):
	globalVars.load(in, NUM_MAX_VAR);

	// At 0x2281 (i.e. 0x2083 + 255 * 2):
	loadZoneData(in);

	// At 0x22A1 (i.e. 0x2281 + 16 * 2):
	loadCommandVariables(in);

	// At 0x22A9 (i.e. 0x22A1 + 4 * 2):
	in.read(commandBuffer, 0x50);
	renderer->setCommand(commandBuffer);

	// At 0x22F9 (i.e. 0x22A9 + 0x50):
	renderer->_cmdY = in.readUint16BE();

	// At 0x22FB:
	bgVar0 = in.readUint16BE();
	// At 0x22FD:
	allowPlayerInput = in.readUint16BE();
	// At 0x22FF:
	playerCommand = in.readSint16BE();
	// At 0x2301:
	commandVar1 = in.readSint16BE();
	// At 0x2303:
	isDrawCommandEnabled = in.readUint16BE();
	// At 0x2305:
	var5 = in.readUint16BE();
	// At 0x2307:
	var4 = in.readUint16BE();
	// At 0x2309:
	var3 = in.readUint16BE();
	// At 0x230B:
	var2 = in.readUint16BE();
	// At 0x230D:
	commandVar2 = in.readSint16BE();

	// At 0x230F:
	renderer->_messageBg = in.readUint16BE();

	// At 0x2311:
	in.readUint16BE();
	// At 0x2313:
	in.readUint16BE();

	// At 0x2315:
	loadResourcesFromSave(in, saveGameFormat);

	loadScreenParams(in);
	loadGlobalScripts(in);
	loadObjectScripts(in);
	loadOverlayList(in);
	loadBgIncrustFromSave(in);

	if (strlen(currentMsgName)) {
		loadMsg(currentMsgName);
	}

	if (strlen(currentDatName)) {
/*		i = saveVar2;
		saveVar2 = 0;
		loadMusic();
		if (i) {
			playMusic();
		}*/
	}

	return !in.ioFailed();
}

bool CineEngine::makeLoad(char *saveName) {
	Common::SharedPtr<Common::InSaveFile> saveFile(g_saveFileMan->openForLoading(saveName));

	if (!saveFile) {
		drawString(otherMessages[0], 0);
		waitPlayerInput();
		// restoreScreen();
		checkDataDisk(-1);
		return false;
	}

	setMouseCursor(MOUSE_CURSOR_DISK);

	uint32 saveSize = saveFile->size();
	// TODO: Evaluate the maximum savegame size for the temporary Operation Stealth savegame format.
	if (saveSize == 0) { // Savefile's compressed using zlib format can't tell their unpacked size, test for it
		// Can't get information about the savefile's size so let's try
		// reading as much as we can from the file up to a predefined upper limit.
		//
		// Some estimates for maximum savefile sizes (All with 255 animDataTable entries of 30 bytes each):
		// With 256 global scripts, object scripts, overlays and background incrusts:
		// 0x2315 + (255 * 30) + (2 * 6) + (206 + 206 + 20 + 20) * 256 = ~129kB
		// With 512 global scripts, object scripts, overlays and background incrusts:
		// 0x2315 + (255 * 30) + (2 * 6) + (206 + 206 + 20 + 20) * 512 = ~242kB
		//
		// I think it extremely unlikely that there would be over 512 global scripts, object scripts,
		// overlays and background incrusts so 256kB seems like quite a safe upper limit.		
		// NOTE: If the savegame format is changed then this value might have to be re-evaluated!
		// Hopefully devices with more limited memory can also cope with this memory allocation.
		saveSize = 256 * 1024;
	}
	Common::SharedPtr<Common::MemoryReadStream> in(saveFile->readStream(saveSize));

	// Try to detect the used savegame format
	enum CineSaveGameFormat saveGameFormat = detectSaveGameFormat(*in);

	// Handle problematic savegame formats
	bool load = true; // Should we try to load the savegame?
	bool result = false;
	if (saveGameFormat == ANIMSIZE_30_PTRS_BROKEN) {
		// One might be able to load the ANIMSIZE_30_PTRS_BROKEN format but
		// that's not implemented here because it was never used in a stable
		// release of ScummVM but only during development (From revision 31453,
		// which introduced the problem, until revision 32073, which fixed it).
		// Therefore be bail out if we detect this particular savegame format.
		warning("Detected a known broken savegame format, not loading savegame");
		load = false; // Don't load the savegame
	} else if (saveGameFormat == ANIMSIZE_UNKNOWN) {
		// If we can't detect the savegame format
		// then let's try the default format and hope for the best.
		warning("Couldn't detect the used savegame format, trying default savegame format. Things may break");
		saveGameFormat = ANIMSIZE_30_PTRS_INTACT;
	}

	if (load) {
		// Reset the engine's state
		resetEngine();
		
		if (saveGameFormat == TEMP_OS_FORMAT) {
			// Load the temporary Operation Stealth savegame format
			result = loadTempSaveOS(*in);
		} else {
			// Load the plain Future Wars savegame format
			result = loadPlainSaveFW(*in, saveGameFormat);
		}
	}

	setMouseCursor(MOUSE_CURSOR_NORMAL);

	return result;
}

void CineEngine::makeSaveFW(Common::OutSaveFile &out) {
	out.writeUint16BE(currentDisk);
	out.write(currentPartName, 13);
	out.write(currentDatName, 13);
	out.writeUint16BE(saveVar2);
	out.write(currentPrcName, 13);
	out.write(currentRelName, 13);
	out.write(currentMsgName, 13);
	renderer->saveBgNames(out);
	out.write(currentCtName, 13);

	saveObjectTable(out);
	renderer->savePalette(out);
	globalVars.save(out, NUM_MAX_VAR);
	saveZoneData(out);
	saveCommandVariables(out);
	out.write(commandBuffer, 0x50);

	out.writeUint16BE(renderer->_cmdY);
	out.writeUint16BE(bgVar0);
	out.writeUint16BE(allowPlayerInput);
	out.writeUint16BE(playerCommand);
	out.writeUint16BE(commandVar1);
	out.writeUint16BE(isDrawCommandEnabled);
	out.writeUint16BE(var5);
	out.writeUint16BE(var4);
	out.writeUint16BE(var3);
	out.writeUint16BE(var2);
	out.writeUint16BE(commandVar2);
	out.writeUint16BE(renderer->_messageBg);

	saveAnimDataTable(out);
	saveScreenParams(out);

	saveGlobalScripts(out);
	saveObjectScripts(out);
	saveOverlayList(out);
	saveBgIncrustList(out);
}

void CineEngine::makeSave(char *saveFileName) {
	Common::SharedPtr<Common::OutSaveFile> fHandle(g_saveFileMan->openForSaving(saveFileName));

	setMouseCursor(MOUSE_CURSOR_DISK);

	if (!fHandle) {
		drawString(otherMessages[1], 0);
		waitPlayerInput();
		// restoreScreen();
		checkDataDisk(-1);
	} else {
		if (g_cine->getGameType() == GType_FW) {
			makeSaveFW(*fHandle);
		} else {
			makeSaveOS(*fHandle);
		}
	}

	setMouseCursor(MOUSE_CURSOR_NORMAL);
}

void CineEngine::makeSystemMenu(void) {
	int16 numEntry, systemCommand;
	int16 mouseX, mouseY, mouseButton;
	int16 selectedSave;

	if (!disableSystemMenu) {
		inMenu = true;

		do {
			manageEvents();
			getMouseData(mouseUpdateStatus, (uint16 *)&mouseButton, (uint16 *)&mouseX, (uint16 *)&mouseY);
		} while (mouseButton);

		numEntry = 6;

		if (!allowPlayerInput) {
			numEntry--;
		}

		systemCommand = makeMenuChoice(systemMenu, numEntry, mouseX, mouseY, 140);

		switch (systemCommand) {
		case 0:
			{
				drawString(otherMessages[2], 0);
				waitPlayerInput();
				break;
			}
		case 1:
			{
				getMouseData(mouseUpdateStatus, (uint16 *)&mouseButton, (uint16 *)&mouseX, (uint16 *)&mouseY);
				if (!makeMenuChoice(confirmMenu, 2, mouseX, mouseY + 8, 100)) {
					//reinitEngine();
				}
				break;
			}
		case 2:
			{
				getMouseData(mouseUpdateStatus, (uint16 *)&mouseButton, (uint16 *)&mouseX, (uint16 *)&mouseY);
				if (!makeMenuChoice(confirmMenu, 2, mouseX, mouseY + 8, 100)) {
					exitEngine = 1;
				}
				break;
			}
		case 3:	// Select save drive... change ?
			{
				break;
			}
		case 4:	// load game
			{
				if (loadSaveDirectory()) {
//					int16 selectedSave;

					getMouseData(mouseUpdateStatus, (uint16 *)&mouseButton, (uint16 *)&mouseX, (uint16 *)&mouseY);
					selectedSave = makeMenuChoice(currentSaveName, 10, mouseX, mouseY + 8, 180);

					if (selectedSave >= 0) {
						char saveNameBuffer[256];
						sprintf(saveNameBuffer, "%s.%1d", _targetName.c_str(), selectedSave);

						getMouseData(mouseUpdateStatus, (uint16 *)&mouseButton, (uint16 *)&mouseX, (uint16 *)&mouseY);
						if (!makeMenuChoice(confirmMenu, 2, mouseX, mouseY + 8, 100)) {
							char loadString[256];

							sprintf(loadString, otherMessages[3], currentSaveName[selectedSave]);
							drawString(loadString, 0);

							makeLoad(saveNameBuffer);
						} else {
							drawString(otherMessages[4], 0);
							waitPlayerInput();
							checkDataDisk(-1);
						}
					} else {
						drawString(otherMessages[4], 0);
						waitPlayerInput();
						checkDataDisk(-1);
					}
				} else {
					drawString(otherMessages[5], 0);
					waitPlayerInput();
					checkDataDisk(-1);
				}
				break;
			}
		case 5:
			{
				loadSaveDirectory();
				selectedSave = makeMenuChoice(currentSaveName, 10, mouseX, mouseY + 8, 180);

				if (selectedSave >= 0) {
					char saveFileName[256];
					char saveName[20];
					saveName[0] = 0;

					if (!makeTextEntryMenu(otherMessages[6], saveName, 20, 120))
						break;

					strncpy(currentSaveName[selectedSave], saveName, 20);

					sprintf(saveFileName, "%s.%1d", _targetName.c_str(), selectedSave);

					getMouseData(mouseUpdateStatus, (uint16 *)&mouseButton, (uint16 *)&mouseX, (uint16 *)&mouseY);
					if (!makeMenuChoice(confirmMenu, 2, mouseX, mouseY + 8, 100)) {
						char saveString[256], tmp[80];

						snprintf(tmp, 80, "%s.dir", _targetName.c_str());

						Common::OutSaveFile *fHandle = g_saveFileMan->openForSaving(tmp);
						if (!fHandle) {
							warning("Unable to open file %s for saving", tmp);
							break;
						}

						fHandle->write(currentSaveName, 200);
						delete fHandle;

						sprintf(saveString, otherMessages[3], currentSaveName[selectedSave]);
						drawString(saveString, 0);

						makeSave(saveFileName);

						checkDataDisk(-1);
					} else {
						drawString(otherMessages[4], 0);
						waitPlayerInput();
						checkDataDisk(-1);
					}
				}
				break;
			}
		}

		inMenu = false;
	}
}

/**
 * Save an Operation Stealth type savegame. WIP!
 *
 * NOTE: This is going to be very much a work in progress so the Operation Stealth's
 *       savegame formats that are going to be tried are extremely probably not going
 *       to be supported at all after Operation Stealth becomes officially supported.
 *       This means that the savegame format will hopefully change to something nicer
 *       when official support for Operation Stealth begins.
 */
void CineEngine::makeSaveOS(Common::OutSaveFile &out) {
	int i;

	// Make a temporary Operation Stealth savegame format chunk header and save it.	
	ChunkHeader header;
	header.id = TEMP_OS_FORMAT_ID;
	header.version = CURRENT_OS_SAVE_VER;
	header.size = 0; // No data is currently put inside the chunk, all the plain data comes right after it.
	writeChunkHeader(out, header);

	// Start outputting the plain savegame data right after the chunk header.
	out.writeUint16BE(currentDisk);
	out.write(currentPartName, 13);
	out.write(currentPrcName, 13);
	out.write(currentRelName, 13);
	out.write(currentMsgName, 13);
	renderer->saveBgNames(out);
	out.write(currentCtName, 13);

	saveObjectTable(out);
	renderer->savePalette(out);
	globalVars.save(out, NUM_MAX_VAR);
	saveZoneData(out);
	saveCommandVariables(out);
	out.write(commandBuffer, 0x50);
	saveZoneQuery(out);

	// FIXME: Save a proper name here, saving an empty string currently.
	// 0x2925: Current music name (String, 13 bytes).
	for (i = 0; i < 13; i++) {
		out.writeByte(0);
	}
	// FIXME: Save proper value for this variable, currently writing zero
	// 0x2932: Is music loaded? (Uint16BE, Boolean).
	out.writeUint16BE(0);
	// FIXME: Save proper value for this variable, currently writing zero
	// 0x2934: Is music playing? (Uint16BE, Boolean).
	out.writeUint16BE(0);

	out.writeUint16BE(renderer->_cmdY);	
	out.writeUint16BE(0); // Some unknown variable that seems to always be zero
	out.writeUint16BE(allowPlayerInput);
	out.writeUint16BE(playerCommand);
	out.writeUint16BE(commandVar1);
	out.writeUint16BE(isDrawCommandEnabled);
	out.writeUint16BE(var5);
	out.writeUint16BE(var4);
	out.writeUint16BE(var3);
	out.writeUint16BE(var2);
	out.writeUint16BE(commandVar2);
	out.writeUint16BE(renderer->_messageBg);
	
	// FIXME: Save proper value for this variable, currently writing zero.
	// An unknown variable at 0x295E: adBgVar1 (Uint16BE).
	out.writeUint16BE(0);
	out.writeSint16BE(currentAdditionalBgIdx);
	out.writeSint16BE(currentAdditionalBgIdx2);
	// FIXME: Save proper value for this variable, currently writing zero.
	// 0x2954: additionalBgVScroll (Uint16BE). This probably means renderer->_bgShift.
	out.writeUint16BE(0);
	// FIXME: Save proper value for this variable, currently writing zero.
	// An unknown variable at 0x2956: adBgVar0 (Uint16BE). Maybe this means bgVar0?
	out.writeUint16BE(0);
	out.writeUint16BE(disableSystemMenu);

	saveAnimDataTable(out);
	saveScreenParams(out);
	saveGlobalScripts(out);
	saveObjectScripts(out);
	saveSeqList(out);
	saveOverlayList(out);
	saveBgIncrustList(out);
}

void drawMessageBox(int16 x, int16 y, int16 width, int16 currentY, int16 offset, int16 color, byte* page) {
	gfxDrawLine(x + offset, y + offset, x + width - offset, y + offset, color, page);	// top
	gfxDrawLine(x + offset, currentY + 4 - offset, x + width - offset, currentY + 4 - offset, color, page);	// bottom
	gfxDrawLine(x + offset, y + offset, x + offset, currentY + 4 - offset, color, page);	// left
	gfxDrawLine(x + width - offset, y + offset, x + width - offset, currentY + 4 - offset, color, page);	// right
}

void drawDoubleMessageBox(int16 x, int16 y, int16 width, int16 currentY, int16 color, byte* page) {
	drawMessageBox(x, y, width, currentY, 1, 0, page);
	drawMessageBox(x, y, width, currentY, 0, color, page);
}

void processInventory(int16 x, int16 y) {
	int16 listSize = buildObjectListCommand(-2);
	uint16 button;

	if (!listSize)
		return;

	renderer->drawMenu(objectListCommand, listSize, x, y, 140, -1);
	renderer->blit();

	do {
		manageEvents();
		getMouseData(mouseUpdateStatus, &button, &dummyU16, &dummyU16);
	} while (!button);
}

int16 buildObjectListCommand(int16 param) {
	int16 i = 0, j = 0;

	for (i = 0; i < 20; i++) {
		objectListCommand[i][0] = 0;
	}

	for (i = 0; i < 255; i++) {
		if (objectTable[i].name[0] && objectTable[i].costume == param) {
			strcpy(objectListCommand[j], objectTable[i].name);
			objListTab[j] = i;
			j++;
		}
	}

	return j;
}

int16 selectSubObject(int16 x, int16 y, int16 param) {
	int16 listSize = buildObjectListCommand(param);
	int16 selectedObject;
	bool osExtras = g_cine->getGameType() == Cine::GType_OS;

	if (!listSize) {
		return -2;
	}

	selectedObject = makeMenuChoice(objectListCommand, listSize, x, y, 140, osExtras);

	if (selectedObject == -1)
		return -1;

	if (osExtras) {
		if (selectedObject >= 8000) {
			return objListTab[selectedObject - 8000] + 8000;
		}
	}

	return objListTab[selectedObject];
}

void makeCommandLine(void) {
	uint16 x, y;

	commandVar1 = 0;
	commandVar2 = -10;

	if (playerCommand != -1) {
		strcpy(commandBuffer, defaultActionCommand[playerCommand]);
	} else {
		strcpy(commandBuffer, "");
	}

	if ((playerCommand != -1) && (choiceResultTable[playerCommand] == 2)) {	// need object selection ?
		int16 si;

		getMouseData(mouseUpdateStatus, &dummyU16, &x, &y);

		if (g_cine->getGameType() == Cine::GType_FW) {
			si = selectSubObject(x, y + 8, -2);
		} else {
			si = selectSubObject(x, y + 8, -subObjectUseTable[playerCommand]);
		}

		if (si < 0) {
			playerCommand = -1;
			strcpy(commandBuffer, "");
		} else {
			if (g_cine->getGameType() == Cine::GType_OS) {
				if (si >= 8000) {
					si -= 8000;
					canUseOnObject = canUseOnItemTable[playerCommand];
				} else {
					canUseOnObject = 0;
				}
			}

			commandVar3[0] = si;
			commandVar1 = 1;

			strcat(commandBuffer, " ");
			strcat(commandBuffer, objectTable[commandVar3[0]].name);
			strcat(commandBuffer, " ");
			strcat(commandBuffer, commandPrepositionOn);
		}
	} else {
		if (playerCommand == 2) {
			getMouseData(mouseUpdateStatus, &dummyU16, &x, &y);
			processInventory(x, y + 8);
			playerCommand = -1;
			commandVar1 = 0;
			strcpy(commandBuffer, "");
		}
	}

	if (g_cine->getGameType() == Cine::GType_OS) {
		if (playerCommand != -1 && canUseOnObject != 0)	{ // call use on sub object
			int16 si;

			getMouseData(mouseUpdateStatus, &dummyU16, &x, &y);

			si = selectSubObject(x, y + 8, -subObjectUseTable[playerCommand]);

			if (si) {
				if (si >= 8000) {
					si -= 8000;
				}

				commandVar3[commandVar1] = si;

				commandVar1++;

				// TODO: add command message draw
			}

			isDrawCommandEnabled = 1;

			if (playerCommand != -1 && choiceResultTable[playerCommand] == commandVar1) {
				SelectedObjStruct obj;
				obj.idx = commandVar3[0];
				obj.param = commandVar3[1];
				int16 di = getRelEntryForObject(playerCommand, commandVar1, &obj);

				if (di != -1) {
					runObjectScript(di);
				}
			}
		}
	}

	if (!disableSystemMenu) {
		renderer->setCommand(commandBuffer);
	}
}

uint16 needMouseSave = 0;

uint16 menuVar4 = 0;
uint16 menuVar5 = 0;

int16 makeMenuChoice(const CommandeType commandList[], uint16 height, uint16 X, uint16 Y, uint16 width, bool recheckValue) {
	int16 paramY;
	uint16 button;
	int16 var_A;
	int16 di;
	uint16 j;
	int16 mouseX, mouseY;
	int16 var_16;
	int16 var_14;
	int16 currentSelection, oldSelection;
	int16 var_4;

	if (disableSystemMenu)
		return -1;

	paramY = (height * 9) + 10;

	if (X + width > 319) {
		X = 319 - width;
	}

	if (Y + paramY > 199) {
		Y = 199 - paramY;
	}

	renderer->drawMenu(commandList, height, X, Y, width, -1);
	renderer->blit();

	do {
		manageEvents();
		getMouseData(mouseUpdateStatus, &button, &dummyU16, &dummyU16);
	} while (button);

	var_A = 0;

	currentSelection = 0;

	di = currentSelection * 9 + Y + 4;

	renderer->drawMenu(commandList, height, X, Y, width, currentSelection);
	renderer->blit();

	manageEvents();
	getMouseData(mouseUpdateStatus, &button, (uint16 *)&mouseX, (uint16 *)&mouseY);

	var_16 = mouseX;
	var_14 = mouseY;

	menuVar = 0;

	do {
		manageEvents();
		getMouseData(mouseUpdateStatus, &button, (uint16 *)&mouseX, (uint16 *)&mouseY);

		if (button) {
			var_A = 1;
		}

		oldSelection = currentSelection;

		if (needMouseSave) {
			for (j = 0; j < 3; j++) {
				mainLoopSub6();
			}

			if (menuVar4 && currentSelection > 0) {	// go up
				currentSelection--;
			}

			if (menuVar5) {	// go down
				if (height - 1 > currentSelection) {
					currentSelection++;
				}
			}
		} else {
			if (mouseX > X && mouseX < X + width && mouseY > Y && mouseY < Y + height * 9) {
				currentSelection = (mouseY - (Y + 4)) / 9;

				if (currentSelection < 0)
					currentSelection = 0;

				if (currentSelection >= height)
					currentSelection = height - 1;
			}
		}

		if (currentSelection != oldSelection) {	// old != new
			if (needMouseSave) {
				hideMouse();
			}

			di = currentSelection * 9 + Y + 4;

			renderer->drawMenu(commandList, height, X, Y, width, currentSelection);
			renderer->blit();

//			if (needMouseSave) {
//				gfxRedrawMouseCursor();
//			}
		}

	} while (!var_A);

	assert(!needMouseSave);

	var_4 = button;

	menuVar = 0;

	do {
		manageEvents();
		getMouseData(mouseUpdateStatus, &button, &dummyU16, &dummyU16);
	} while (button);

	if (var_4 == 2)	{	// recheck
		if (!recheckValue)
			return -1;
		else
			return currentSelection + 8000;
	}

	return currentSelection;
}

void makeActionMenu(void) {
	uint16 mouseButton;
	uint16 mouseX;
	uint16 mouseY;

	inMenu = true;

	getMouseData(mouseUpdateStatus, &mouseButton, &mouseX, &mouseY);

	if (g_cine->getGameType() == Cine::GType_OS) {
		playerCommand = makeMenuChoice(defaultActionCommand, 6, mouseX, mouseY, 70, true);

		if (playerCommand >= 8000) {
			playerCommand -= 8000;
			canUseOnObject = 1;
		}
	} else {
		playerCommand = makeMenuChoice(defaultActionCommand, 6, mouseX, mouseY, 70);
	}

	inMenu = false;
}

uint16 executePlayerInput(void) {
	uint16 var_5E;
	uint16 var_2;
	uint16 mouseX, mouseY, mouseButton;
	uint16 currentEntry = 0;
	uint16 di = 0;

	canUseOnObject = 0;

	if (isInPause) {
		drawString(otherMessages[2], 0);
		waitPlayerInput();
		isInPause = 0;
	}

	if (allowPlayerInput) {
		getMouseData(mouseUpdateStatus, &mouseButton, &mouseX, &mouseY);

		while (mouseButton && currentEntry < 200) {
			if (mouseButton & 1) {
				di |= 1;
			}

			if (mouseButton & 2) {
				di |= 2;
			}

			getMouseData(mouseUpdateStatus, &mouseButton, &mouseX, &mouseY);

			currentEntry++;
		}

		if (di) {
			mouseButton = di;
		}

		if (playerCommand != -1) {
			if (mouseButton & 1) {
				if (mouseButton & 2) {
					g_cine->makeSystemMenu();
				} else {
					int16 si;
					do {
						manageEvents();
						getMouseData(mouseUpdateStatus, &mouseButton, &dummyU16, &dummyU16);
					} while (mouseButton);

					si = getObjectUnderCursor(mouseX,
					    mouseY);

					if (si != -1) {
						commandVar3[commandVar1] = si;
						commandVar1++;

						strcat(commandBuffer, " ");
						strcat(commandBuffer, objectTable[si].name);

						isDrawCommandEnabled = 1;

						if (choiceResultTable[playerCommand] == commandVar1) {
							int16 relEntry;

							SelectedObjStruct obj;
							obj.idx = commandVar3[0];
							obj.param = commandVar3[1];

							relEntry = getRelEntryForObject(playerCommand, commandVar1, &obj);

							if (relEntry != -1) {
								runObjectScript(relEntry);
							} else {
								addPlayerCommandMessage(playerCommand);
							}

							playerCommand = -1;

							commandVar1 = 0;
							strcpy(commandBuffer, "");
							renderer->setCommand("");
						}
					} else {
						globalVars[VAR_MOUSE_X_POS] = mouseX;
						globalVars[VAR_MOUSE_Y_POS] = mouseY;
					}
				}
			} else if (mouseButton & 2) {
				if (mouseButton & 1) {
					g_cine->makeSystemMenu();
				}

				makeActionMenu();
				makeCommandLine();
			} else {
				int16 objIdx;

				objIdx = getObjectUnderCursor(mouseX, mouseY);

				if (commandVar2 != objIdx) {
					if (objIdx != -1) {
						char command[256];

						strcpy(command, commandBuffer);
						strcat(command, " ");
						strcat(command, objectTable[objIdx].name);

						renderer->setCommand(command);
					} else {
						isDrawCommandEnabled = 1;
					}
				}

				commandVar2 = objIdx;
			}
		} else {
			if (mouseButton & 2) {
				if (!(mouseButton & 1)) {
					if (g_cine->getGameType() == Cine::GType_OS) {
						playerCommand = makeMenuChoice(defaultActionCommand, 6, mouseX, mouseY, 70, true);

						if (playerCommand >= 8000) {
							playerCommand -= 8000;
							canUseOnObject = 1;
						}
					} else {
						playerCommand = makeMenuChoice(defaultActionCommand, 6, mouseX, mouseY, 70);
					}

					makeCommandLine();
				} else {
					g_cine->makeSystemMenu();
				}
			} else {
				if (mouseButton & 1) {
					if (!(mouseButton & 2)) {
						int16 objIdx;
						int16 relEntry;

						globalVars[VAR_MOUSE_X_POS] = mouseX;
						if (!mouseX) {
							globalVars[VAR_MOUSE_X_POS]++;
						}

						globalVars[VAR_MOUSE_Y_POS] = mouseY;

						objIdx = getObjectUnderCursor(mouseX, mouseY);

						if (objIdx != -1) {
							currentSelectedObject.idx = objIdx;
							currentSelectedObject.param = -1;

							relEntry = getRelEntryForObject(6, 1, &currentSelectedObject);

							if (relEntry != -1) {
								runObjectScript(relEntry);
							}
						}
					} else {
						g_cine->makeSystemMenu();
					}
				}
			}
		}
	} else {
		di = 0;
		getMouseData(mouseUpdateStatus, &mouseButton, &mouseX, &mouseY);

		while (mouseButton) {
			if (mouseButton & 1) {
				di |= 1;
			}

			if (mouseButton & 2) {
				di |= 2;
			}

			manageEvents();
			getMouseData(mouseUpdateStatus, &mouseButton, &mouseX, &mouseY);
		}

		if (di) {
			mouseButton = di;
		}

		if ((mouseButton & 1) && (mouseButton & 2)) {
			g_cine->makeSystemMenu();
		}
	}

	var_2 = menuVar & 0x7F;
	var_5E = var_2;

	if (menuVar & 0x80) {
		var_5E = 0;
		var_2 = 0;
	}

	if (inputVar1 && allowPlayerInput) {	// use keyboard
		inputVar1 = 0;

		switch (globalVars[VAR_MOUSE_X_MODE]) {
		case 1:
			mouseX = objectTable[1].x + 12;
			break;
		case 2:
			mouseX = objectTable[1].x + 7;
			break;
		default:
			mouseX = globalVars[VAR_MOUSE_X_POS];
			break;
		}

		switch (globalVars[VAR_MOUSE_Y_MODE]) {
		case 1:
			mouseY = objectTable[1].y + 34;
			break;
		case 2:
			mouseY = objectTable[1].y + 28;
			break;
		default:
			mouseY = globalVars[VAR_MOUSE_Y_POS];
			break;
		}

		if (var_5E == bgVar0) {
			var_5E = 0;

			globalVars[VAR_MOUSE_X_POS] = mouseX;
			globalVars[VAR_MOUSE_Y_POS] = mouseY;
		} else {
			if (inputVar2) {
				if (inputVar2 == 2) {
					globalVars[VAR_MOUSE_X_POS] = 1;
				} else {
					globalVars[VAR_MOUSE_X_POS] = 320;
				}
			} else {
				globalVars[VAR_MOUSE_X_POS] = mouseX;
			}

			if (inputVar3) {
				if (inputVar3 == 2) {
					globalVars[VAR_MOUSE_Y_POS] = 1;
				} else {
					globalVars[VAR_MOUSE_Y_POS] = 200;
				}
			} else {
				globalVars[VAR_MOUSE_Y_POS] = mouseY;
			}
		}

		bgVar0 = var_5E;
	} else {		// don't use keyboard for move -> shortcuts to commands
		getMouseData(mouseUpdateStatus, &mouseButton, &mouseX, &mouseY);

		switch (var_2 - 59) {
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
			if (allowPlayerInput) {
				playerCommand = var_2 - 59;
				makeCommandLine();
			}
			break;
		case 6:
		case 7:
		case 8:
		case 23:
			break;
		case 9:
		case 24:
			g_cine->makeSystemMenu();
			break;
		default:
			//  printf("Unhandled case %d in last part of executePlayerInput\n",var2-59);
			break;
		}
	}

	return var_5E;
}

void drawSprite(Common::List<overlay>::iterator it, const byte *spritePtr, const byte *maskPtr, uint16 width, uint16 height, byte *page, int16 x, int16 y) {
	byte *msk = NULL;
	int16 maskX, maskY, maskWidth, maskHeight;
	uint16 maskSpriteIdx;

	msk = (byte *)malloc(width * height);

	if (g_cine->getGameType() == Cine::GType_OS) {
		generateMask(spritePtr, msk, width * height, objectTable[it->objIdx].part);
	} else {
		memcpy(msk, maskPtr, width * height);
	}

	for (++it; it != overlayList.end(); ++it) {
		if (it->type != 5) {
			continue;
		}

		maskX = objectTable[it->objIdx].x;
		maskY = objectTable[it->objIdx].y;

		maskSpriteIdx = ABS((int16)(objectTable[it->objIdx].frame));

		maskWidth = animDataTable[maskSpriteIdx]._realWidth;
		maskHeight = animDataTable[maskSpriteIdx]._height;
		gfxUpdateSpriteMask(msk, x, y, width, height, animDataTable[maskSpriteIdx].data(), maskX, maskY, maskWidth, maskHeight);

#ifdef DEBUG_SPRITE_MASK
		gfxFillSprite(animDataTable[maskSpriteIdx].data(), maskWidth, maskHeight, page, maskX, maskY, 1);
#endif
	}

	gfxDrawMaskedSprite(spritePtr, msk, width, height, page, x, y);
	free(msk);
}

void removeMessages() {
	Common::List<overlay>::iterator it;

	for (it = overlayList.begin(); it != overlayList.end(); ) {
		if (it->type == 2 || it->type == 3) {
			it = overlayList.erase(it);
		} else {
			++it;
		}
	}
}

uint16 processKeyboard(uint16 param) {
	return 0;
}

void mainLoopSub6(void) {
}

void checkForPendingDataLoad(void) {
	if (newPrcName[0] != 0) {
		bool loadPrcOk = loadPrc(newPrcName);

		strcpy(currentPrcName, newPrcName);
		strcpy(newPrcName, "");

		// Check that the loading of the script file was successful before
		// trying to add script 1 from it to the global scripts list. This
		// fixes a crash when failing copy protection in Amiga or Atari ST
		// versions of Future Wars.
		if (loadPrcOk) {
			addScriptToGlobalScripts(1);
		} else if (scumm_stricmp(currentPrcName, COPY_PROT_FAIL_PRC_NAME)) {
			// We only show an error here for other files than the file that
			// is loaded if copy protection fails (i.e. L201.ANI).
			warning("checkForPendingDataLoad: loadPrc(%s) failed", currentPrcName);
		}
	}

	if (newRelName[0] != 0) {
		loadRel(newRelName);

		strcpy(currentRelName, newRelName);
		strcpy(newRelName, "");
	}

	if (newObjectName[0] != 0) {
		overlayList.clear();

		loadObject(newObjectName);

		strcpy(currentObjectName, newObjectName);
		strcpy(newObjectName, "");
	}

	if (newMsgName[0] != 0) {
		loadMsg(newMsgName);

		strcpy(currentMsgName, newMsgName);
		strcpy(newMsgName, "");
	}
}

void hideMouse(void) {
}

void removeExtention(char *dest, const char *source) {
	strcpy(dest, source);

	byte *ptr = (byte *) strchr(dest, '.');

	if (ptr) {
		*ptr = 0;
	}
}

void addMessage(byte param1, int16 param2, int16 param3, int16 param4, int16 param5) {
	overlay tmp;

	tmp.objIdx = param1;
	tmp.type = 2;
	tmp.x = param2;
	tmp.y = param3;
	tmp.width = param4;
	tmp.color = param5;

	overlayList.push_back(tmp);
	waitForPlayerClick = 1;
}

Common::List<SeqListElement> seqList;

void removeSeq(uint16 param1, uint16 param2, uint16 param3) {
	Common::List<SeqListElement>::iterator it;

	for (it = seqList.begin(); it != seqList.end(); ++it) {
		if (it->objIdx == param1 && it->var4 == param2 && it->varE == param3) {
			it->var4 = -1;
			break;
		}
	}
}

bool isSeqRunning(uint16 param1, uint16 param2, uint16 param3) {
	Common::List<SeqListElement>::iterator it;

	for (it = seqList.begin(); it != seqList.end(); ++it) {
		if (it->objIdx == param1 && it->var4 == param2 && it->varE == param3) {
			// Just to be on the safe side there's a restriction of the
			// addition's result to 16-bit arithmetic here like in the
			// original. It's possible that it's not strictly needed.
			return ((it->var14 + it->var16) & 0xFFFF) == 0;
		}
	}

	return true;
}

void addSeqListElement(uint16 objIdx, int16 param1, int16 param2, int16 frame, int16 param4, int16 param5, int16 param6, int16 param7, int16 param8) {
	Common::List<SeqListElement>::iterator it;
	SeqListElement tmp;

	for (it = seqList.begin(); it != seqList.end() && it->varE < param7; ++it) ;

	tmp.objIdx = objIdx;
	tmp.var4 = param1;
	tmp.var8 = param2;
	tmp.frame = frame;
	tmp.varC = param4;
	tmp.var14 = 0;
	tmp.var16 = 0;
	tmp.var18 = param5;
	tmp.var1A = param6;
	tmp.varE = param7;
	tmp.var10 = param8;
	tmp.var12 = param8;
	tmp.var1C = 0;
	tmp.var1E = 0;

	seqList.insert(it, tmp);
}

void modifySeqListElement(uint16 objIdx, int16 var4Test, int16 param1, int16 param2, int16 param3, int16 param4) {
	// Find a suitable list element and modify it
	for (Common::List<SeqListElement>::iterator it = seqList.begin(); it != seqList.end(); ++it) {
		if (it->objIdx == objIdx && it->var4 == var4Test) {
			it->varC  = param1;
			it->var18 = param2;
			it->var1A = param3;
			it->var10 = it->var12 = param4;
			break;
		}
	}
}

void computeMove1(SeqListElement &element, int16 x, int16 y, int16 param1,
    int16 param2, int16 x2, int16 y2) {
	element.var16 = 0;
	element.var14 = 0;

	if (y2) {
		if (y - param2 > y2) {
			element.var16 = 2;
		}

		if (y + param2 < y2) {
			element.var16 = 1;
		}
	}

	if (x2) {
		if (x - param1 > x2) {
			element.var14 = 2;
		}

		if (x + param1 < x2) {
			element.var14 = 1;
		}
	}
}

uint16 computeMove2(SeqListElement &element) {
	int16 returnVar = 0;

	if (element.var16 == 1) {
		returnVar = 4;
	} else if (element.var16 == 2) {
		returnVar = 3;
	}

	if (element.var14 == 1) {
		returnVar = 1;
	} else if (element.var14 == 2) {
		returnVar = 2;
	}

	return returnVar;
}

uint16 addAni(uint16 param1, uint16 objIdx, const int8 *ptr, SeqListElement &element, uint16 param3, int16 *param4) {
	const int8 *ptrData;
	const int8 *ptr2;
	int16 di;

	// In the original an error string is set and 0 is returned if the following doesn't hold
	assert(ptr);

	// We probably could just use a local variable here instead of the dummyU16 but
	// haven't checked if this has any side-effects so keeping it this way still.
	dummyU16 = READ_BE_UINT16(ptr + param1 * 2 + 8);
	ptrData = ptr + dummyU16;

	// In the original an error string is set and 0 is returned if the following doesn't hold
	assert(*ptrData);

	di = (objectTable[objIdx].costume + 1) % (*ptrData);
	++ptrData; // Jump over the just read byte
	// Here ptr2 seems to be indexing a table of structs (8 bytes per struct):
	//	struct {
	//		int8 x;			// 0 (Used with checkCollision)
	//		int8 y;			// 1 (Used with checkCollision)
	//		int8 numZones;	// 2 (Used with checkCollision)
	//		int8 var3;		// 3 (Not used in this function)
	//		int8 xAdd;		// 4 (Used with an object)
	//		int8 yAdd;		// 5 (Used with an object)
	//		int8 maskAdd;	// 6 (Used with an object)
	//		int8 frameAdd;	// 7 (Used with an object)
	//	};
	ptr2 = ptrData + di * 8;

	// We might probably safely discard the AND by 1 here because
	// at least in the original checkCollision returns always 0 or 1.
	if ((checkCollision(objIdx, ptr2[0], ptr2[1], ptr2[2], ptr[0]) & 1)) {
		return 0;
	}

	objectTable[objIdx].x += ptr2[4];
	objectTable[objIdx].y += ptr2[5];
	objectTable[objIdx].mask += ptr2[6];

	if (ptr2[6]) {
		resetGfxEntityEntry(objIdx);
	}

	objectTable[objIdx].frame = ptr2[7] + element.var8;

	if (param3 || !element.var14) {
		objectTable[objIdx].costume = di;
	} else {
		assert(param4);
		*param4 = di;
	}

	return 1;
}

/*! 
 * Permutates the overlay list into a different order according to some logic.
 * \todo Check this function for correctness (Wasn't very easy to reverse engineer so there may be errors)
 */
void resetGfxEntityEntry(uint16 objIdx) {
	Common::List<overlay>::iterator it, bObjsCutPoint;
	Common::List<overlay> aReverseObjs, bObjs;
	bool foundCutPoint = false;	

	// Go through the overlay list and partition the whole list into two categories (Type A and type B objects)
	for (it = overlayList.begin(); it != overlayList.end(); ++it) {
		if (it->objIdx == objIdx && it->type != 2 && it->type != 3) { // Type A object
			aReverseObjs.push_front(*it);
		} else { // Type B object
			bObjs.push_back(*it);
			uint16 objectMask;
			if (it->type == 2 || it->type == 3) {
				objectMask = 10000;
			} else {
				objectMask = objectTable[it->objIdx].mask;
			}
	
			if (objectTable[objIdx].mask > objectMask) { // Check for B objects' cut point
				bObjsCutPoint = bObjs.reverse_begin();
				foundCutPoint = true;
			}
		}
	}
	
	// Recreate the overlay list in a different order.
	overlayList.clear();
	if (foundCutPoint) {
		// If a cut point was found the order is:
		// B objects before the cut point, the cut point, A objects in reverse order, B objects after cut point.
		++bObjsCutPoint; // Include the cut point in the first list insertion
		overlayList.insert(overlayList.end(), bObjs.begin(), bObjsCutPoint);
		overlayList.insert(overlayList.end(), aReverseObjs.begin(), aReverseObjs.end());
		overlayList.insert(overlayList.end(), bObjsCutPoint, bObjs.end());
	} else {
		// If no cut point was found the order is:
		// A objects in reverse order, B objects.
		overlayList.insert(overlayList.end(), aReverseObjs.begin(), aReverseObjs.end());
		overlayList.insert(overlayList.end(), bObjs.begin(), bObjs.end());
	}
}

void processSeqListElement(SeqListElement &element) {
	int16 x = objectTable[element.objIdx].x;
	int16 y = objectTable[element.objIdx].y;
	const int8 *ptr1 = (const int8 *) animDataTable[element.frame].data();
	int16 var_10;
	int16 var_4;
	int16 var_2;

	if (element.var12 < element.var10) {
		element.var12++;
		return;
	}

	element.var12 = 0;

	if (ptr1) {
		int16 param1 = ptr1[1];
		int16 param2 = ptr1[2];

		if (element.varC != 255) {
			int16 x2 = element.var18;
			int16 y2 = element.var1A;
			if (element.varC) {
				x2 += objectTable[element.varC].x;
				y2 += objectTable[element.varC].y;
			}
			computeMove1(element, ptr1[4] + x, ptr1[5] + y, param1, param2, x2, y2);
		} else {
			if (inputVar0 && allowPlayerInput) {
				int16 adder = param1 + 1;
				if (inputVar0 != 1) {
					adder = -adder;
				}
				// FIXME: In Operation Stealth's disassembly global variable 251 is used here
				//        but it's named as VAR_MOUSE_Y_MODE in ScummVM. Is it correct or a
				//        left over from Future Wars's reverse engineering?
				globalVars[VAR_MOUSE_X_POS] = globalVars[251] = ptr1[4] + x + adder;
			}

			if (inputVar1 && allowPlayerInput) {
				int16 adder = param2 + 1;
				if (inputVar1 != 1) {
					adder = -adder;
				}
				// TODO: Name currently unnamed global variable 252
				globalVars[VAR_MOUSE_Y_POS] = globalVars[252] = ptr1[5] + y + adder;
			}

			if (globalVars[VAR_MOUSE_X_POS] || globalVars[VAR_MOUSE_Y_POS]) {
				computeMove1(element, ptr1[4] + x, ptr1[5] + y, param1, param2, globalVars[VAR_MOUSE_X_POS], globalVars[VAR_MOUSE_Y_POS]);
			} else {
				element.var16 = 0;
				element.var14 = 0;
			}
		}

		var_10 = computeMove2(element);

		if (var_10) {
			element.var1C = var_10;
			element.var1E = var_10;
		}

		var_4 = -1;

		if ((element.var16 == 1
			&& !addAni(3, element.objIdx, ptr1, element, 0, &var_4)) || (element.var16 == 2	&& !addAni(2, element.objIdx, ptr1, element, 0,
			    &var_4))) {
			if (element.varC == 255) {
				globalVars[VAR_MOUSE_Y_POS] = 0;
			}
		}

		if ((element.var14 == 1
			&& !addAni(0, element.objIdx, ptr1, element, 1, &var_2))) {
			if (element.varC == 255) {
				globalVars[VAR_MOUSE_X_POS] = 0;

				if (var_4 != -1) {
					objectTable[element.objIdx].costume = var_4;
				}
			}
		}

		if ((element.var14 == 2 && !addAni(1, element.objIdx, ptr1, element, 1, &var_2))) {
			if (element.varC == 255) {
				globalVars[VAR_MOUSE_X_POS] = 0;

				if (var_4 != -1) {
					objectTable[element.objIdx].costume = var_4;
				}
			}
		}

		if (element.var16 + element.var14 == 0) {
			if (element.var1C) {
				if (element.var1E) {
					objectTable[element.objIdx].costume = 0;
					element.var1E = 0;
				}

				addAni(element.var1C + 3, element.objIdx, ptr1, element, 1, &var_2);

			}
		}

	}
}

void processSeqList(void) {
	Common::List<SeqListElement>::iterator it;

	for (it = seqList.begin(); it != seqList.end(); ++it) {
		if (it->var4 == -1) {
			continue;
		}

		processSeqListElement(*it);
	}
}


bool makeTextEntryMenu(const char *messagePtr, char *inputString, int stringMaxLength, int y) {
	int len = strlen(messagePtr);
	int16 width = 6 * len + 20;

	width = CLIP((int)width, 180, 250);

	int16 x = (320 - width) / 2;

	getKeyData(); // clear input key

	int quit = 0;
	bool redraw = true;
	CommandeType tempString;
	int inputLength = strlen(inputString);
	int inputPos = inputLength + 1;

	while (!quit) {
		if (redraw) {
			renderer->drawInputBox(messagePtr, inputString, inputPos, x - 16, y, width + 32);
			renderer->blit();
			redraw = false;
		}

		char ch[2];
		memset(tempString, 0, stringMaxLength);
		ch[1] = 0;

		manageEvents();
		int keycode = getKeyData();
		uint16 mouseButton, mouseX, mouseY;

		getMouseData(0, &mouseButton, &mouseX, &mouseY);

		if (mouseButton & 2)
			quit = 2;
		else if (mouseButton & 1)
			quit = 1;

		switch (keycode) {
		case Common::KEYCODE_BACKSPACE:
			if (inputPos <= 1) {
				break;
			}
			inputPos--;
			redraw = true;
		case Common::KEYCODE_DELETE:
			if (inputPos <= inputLength) {
				if (inputPos != 1) {
					strncpy(tempString, inputString, inputPos - 1);
				}
				if (inputPos != inputLength) {
					strncat(tempString, &inputString[inputPos], inputLength - inputPos);
				}
				strcpy(inputString, tempString);
				inputLength = strlen(inputString);
				redraw = true;
			}
			break;
		case Common::KEYCODE_LEFT:
			if (inputPos > 1) {
				inputPos--;
				redraw = true;
			}
			break;
		case Common::KEYCODE_RIGHT:
			if (inputPos <= inputLength) {
				inputPos++;
				redraw = true;
			}
			break;
		default:
			if (((keycode >= 'a') && (keycode <='z')) ||
				((keycode >= '0') && (keycode <='9')) ||
				((keycode >= 'A') && (keycode <='Z')) ||
				(keycode == ' ')) {
				if (inputLength < stringMaxLength - 1) {
					ch[0] = keycode;
					if (inputPos != 1) {
						strncpy(tempString, inputString, inputPos - 1);
						strcat(tempString, ch);
					}
					if ((inputLength == 0) || (inputPos == 1)) {
						strcpy(tempString, ch);
					}
					if ((inputLength != 0) && (inputPos != inputLength)) {
						strncat(tempString, &inputString[inputPos - 1], inputLength - inputPos + 1);
					}

					strcpy(inputString, tempString);
					inputLength = strlen(inputString);
					inputPos++;
					redraw = true;
				}
			}
			break;
		}
	}

	if (quit == 2)
		return false;

	return true;
}

} // End of namespace Cine
