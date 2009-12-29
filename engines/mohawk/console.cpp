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

#include "mohawk/console.h"
#include "mohawk/myst.h"
#include "mohawk/myst_scripts.h"
#include "mohawk/graphics.h"
#include "mohawk/riven.h"
#include "mohawk/livingbooks.h"

namespace Mohawk {

MystConsole::MystConsole(MohawkEngine_Myst *vm) : GUI::Debugger(), _vm(vm) {
	DCmd_Register("changeCard",			WRAP_METHOD(MystConsole, Cmd_ChangeCard));
	DCmd_Register("curCard",			WRAP_METHOD(MystConsole, Cmd_CurCard));
	DCmd_Register("var",				WRAP_METHOD(MystConsole, Cmd_Var));
	DCmd_Register("curStack",			WRAP_METHOD(MystConsole, Cmd_CurStack));
	DCmd_Register("changeStack",		WRAP_METHOD(MystConsole, Cmd_ChangeStack));
	DCmd_Register("drawImage",			WRAP_METHOD(MystConsole, Cmd_DrawImage));
	DCmd_Register("drawRect",			WRAP_METHOD(MystConsole, Cmd_DrawRect));
	DCmd_Register("setResourceEnable",	WRAP_METHOD(MystConsole, Cmd_SetResourceEnable));
	DCmd_Register("playSound",			WRAP_METHOD(MystConsole, Cmd_PlaySound));
	DCmd_Register("stopSound",			WRAP_METHOD(MystConsole, Cmd_StopSound));
	DCmd_Register("playMovie",			WRAP_METHOD(MystConsole, Cmd_PlayMovie));
	DCmd_Register("disableInitOpcodes",	WRAP_METHOD(MystConsole, Cmd_DisableInitOpcodes));
}

MystConsole::~MystConsole() {
}

void MystConsole::preEnter() {
	_vm->_sound->pauseSound();
}

void MystConsole::postEnter() {
	_vm->_sound->resumeSound();
}

bool MystConsole::Cmd_ChangeCard(int argc, const char **argv) {
	if (argc < 2) {
		DebugPrintf("Usage: changeCard <card>\n");
		return true;
	}

	_vm->_sound->stopSound();
	_vm->changeToCard((uint16)atoi(argv[1]));

	return false;
}
	
bool MystConsole::Cmd_CurCard(int argc, const char **argv) {
	DebugPrintf("Current Card: %d\n", _vm->getCurCard());
	return true;
}

bool MystConsole::Cmd_Var(int argc, const char **argv) {
	if (argc == 1) {
		DebugPrintf("Usage: var <var> (<value>)\n");
		return true;
	}
	
	if (argc > 2)
		_vm->_varStore->setVar((uint16)atoi(argv[1]), (uint32)atoi(argv[2]));

	DebugPrintf("%d = %d\n", (uint16)atoi(argv[1]), _vm->_varStore->getVar((uint16)atoi(argv[1])));
	
	return true;
}

static const char *mystStackNames[12] = {
	"Channelwood",
	"Credits",
	"Demo",
	"D'ni",
	"Intro",
	"MakingOf",
	"Mechanical",
	"Myst",
	"Selenitic",
	"Slideshow",
	"SneakPreview",
	"Stoneship"
};

static const uint16 default_start_card[12] = {
	3137,
	10000,
	2001, // TODO: Should be 2000?
	5038,
	2, // TODO: Should be 1?
	1,
	6122,
	4134,
	1282,
	1000,
	3000,
	2029
};

bool MystConsole::Cmd_CurStack(int argc, const char **argv) {
	DebugPrintf("Current Stack: %s\n", mystStackNames[_vm->getCurStack()]);
	return true;
}

bool MystConsole::Cmd_ChangeStack(int argc, const char **argv) {
	if (argc != 2 && argc != 3) {
		DebugPrintf("Usage: changeStack <stack> [<card>]\n\n");
		DebugPrintf("Stacks:\n=======\n");

		for (byte i = 0; i < ARRAYSIZE(mystStackNames); i++)
			DebugPrintf(" %s\n", mystStackNames[i]);

		DebugPrintf("\n");
		
		return true;
	}
	
	byte stackNum = 0;

	for (byte i = 1; i <= ARRAYSIZE(mystStackNames); i++)
		if (!scumm_stricmp(argv[1], mystStackNames[i - 1])) {
			stackNum = i;
			break;
		}

	if (!stackNum) {
		DebugPrintf("\'%s\' is not a stack name!\n", argv[1]);
		return true;
	}
	
	// We need to stop any playing sound when we change the stack
	// as the next card could continue playing it if it.
	_vm->_sound->stopSound();

	_vm->changeToStack(stackNum - 1);

	if (argc == 3)
		_vm->changeToCard((uint16)atoi(argv[2]));
	else
		_vm->changeToCard(default_start_card[stackNum - 1]);

	return false;
}

bool MystConsole::Cmd_DrawImage(int argc, const char **argv) {
	if (argc != 2 && argc != 6) {
		DebugPrintf("Usage: drawImage <image> [<left> <top> <right> <bottom>]\n");
		return true;
	}

	Common::Rect rect;

	if (argc == 2)
		rect = Common::Rect(0, 0, 544, 333);
	else
		rect = Common::Rect((uint16)atoi(argv[2]), (uint16)atoi(argv[3]), (uint16)atoi(argv[4]), (uint16)atoi(argv[5]));
	
	_vm->_gfx->copyImageToScreen((uint16)atoi(argv[1]), rect);
	return false;
}

bool MystConsole::Cmd_DrawRect(int argc, const char **argv) {
	if (argc < 5) {
		DebugPrintf("Usage: drawRect <left> <top> <right> <bottom>\n");
		return true;
	}

	_vm->_gfx->drawRect(Common::Rect((uint16)atoi(argv[1]), (uint16)atoi(argv[2]), (uint16)atoi(argv[3]), (uint16)atoi(argv[4])), true);
	return false;
}

bool MystConsole::Cmd_SetResourceEnable(int argc, const char **argv) {
	if (argc < 3) {
		DebugPrintf("Usage: setResourceEnable <resource id> <bool>\n");
		return true;
	}

	_vm->setResourceEnabled((uint16)atoi(argv[1]), atoi(argv[2]) == 1);
	return true;
}

bool MystConsole::Cmd_PlaySound(int argc, const char **argv) {
	if (argc == 1) {
		DebugPrintf("Usage: playSound <value>\n");

		return true;
	}

	_vm->_sound->stopSound();
	_vm->_sound->playSound((uint16)atoi(argv[1]));

	return false;
}

bool MystConsole::Cmd_StopSound(int argc, const char **argv) {
	DebugPrintf("Stopping Sound\n");

	_vm->_sound->stopSound();

	return true;
}

bool MystConsole::Cmd_PlayMovie(int argc, const char **argv) {
	if (argc < 2) {
		DebugPrintf("Usage: playMovie <name> [<stack>] [<left> <top>]\n");
		DebugPrintf("NOTE: The movie will play *once* in the background.\n");
		return true;
	}
	
	int8 stackNum = 0;

	if (argc == 3 || argc > 4) {
		for (byte i = 1; i <= ARRAYSIZE(mystStackNames); i++)
			if (!scumm_stricmp(argv[2], mystStackNames[i - 1])) {
				stackNum = i;
				break;
			}

		if (!stackNum) {
			DebugPrintf("\'%s\' is not a stack name!\n", argv[2]);
			return true;
		}
	}

	if (argc == 2)
		_vm->_video->playBackgroundMovie(argv[1], 0, 0);
	else if (argc == 3)
		_vm->_video->playBackgroundMovie(_vm->wrapMovieFilename(argv[1], stackNum - 1), 0, 0);
	else if (argc == 4)
		_vm->_video->playBackgroundMovie(argv[1], atoi(argv[2]), atoi(argv[3]));
	else
		_vm->_video->playBackgroundMovie(_vm->wrapMovieFilename(argv[1], stackNum - 1), atoi(argv[3]), atoi(argv[4]));

	return false;
}

bool MystConsole::Cmd_DisableInitOpcodes(int argc, const char **argv) {
	if (argc != 1) {
		DebugPrintf("Usage: disableInitOpcodes\n");

		return true;
	}

	_vm->_scriptParser->disableInitOpcodes();

	return true;
}

RivenConsole::RivenConsole(MohawkEngine_Riven *vm) : GUI::Debugger(), _vm(vm) {
	DCmd_Register("changeCard",		WRAP_METHOD(RivenConsole, Cmd_ChangeCard));
	DCmd_Register("curCard",		WRAP_METHOD(RivenConsole, Cmd_CurCard));
	DCmd_Register("var",			WRAP_METHOD(RivenConsole, Cmd_Var));
	DCmd_Register("playSound",		WRAP_METHOD(RivenConsole, Cmd_PlaySound));
	DCmd_Register("playSLST",       WRAP_METHOD(RivenConsole, Cmd_PlaySLST));
	DCmd_Register("stopSound",		WRAP_METHOD(RivenConsole, Cmd_StopSound));
	DCmd_Register("curStack",		WRAP_METHOD(RivenConsole, Cmd_CurStack));
	DCmd_Register("changeStack",	WRAP_METHOD(RivenConsole, Cmd_ChangeStack));
	DCmd_Register("restart",		WRAP_METHOD(RivenConsole, Cmd_Restart));
	DCmd_Register("hotspots",		WRAP_METHOD(RivenConsole, Cmd_Hotspots));
	DCmd_Register("zipMode",		WRAP_METHOD(RivenConsole, Cmd_ZipMode));
	DCmd_Register("dumpScript",     WRAP_METHOD(RivenConsole, Cmd_DumpScript));
	DCmd_Register("listZipCards",   WRAP_METHOD(RivenConsole, Cmd_ListZipCards));
	DCmd_Register("getRMAP",		WRAP_METHOD(RivenConsole, Cmd_GetRMAP));
}

RivenConsole::~RivenConsole() {
}

void RivenConsole::preEnter() {
	_vm->_sound->pauseSound();
	_vm->_sound->pauseSLST();
}

void RivenConsole::postEnter() {
	_vm->_sound->resumeSound();
	_vm->_sound->resumeSLST();
}

bool RivenConsole::Cmd_ChangeCard(int argc, const char **argv) {
	if (argc < 2) {
		DebugPrintf("Usage: changeCard <card>\n");
		return true;
	}

	_vm->_sound->stopSound();
	_vm->_sound->stopAllSLST();
	_vm->changeToCard((uint16)atoi(argv[1]));

	return false;
}
	
bool RivenConsole::Cmd_CurCard(int argc, const char **argv) {
	DebugPrintf("Current Card: %d\n", _vm->getCurCard());

	return true;
}
	
bool RivenConsole::Cmd_Var(int argc, const char **argv) {
	if (argc == 1) {
		DebugPrintf("Usage: var <var name> (<value>)\n");
		return true;
	}
	
	uint32 *globalVar = _vm->matchVarToString(argv[1]);
	
	if (!globalVar) {
		DebugPrintf("Unknown variable \'%s\'\n", argv[1]);
		return true;
	}
	
	if (argc > 2)
		*globalVar = (uint32)atoi(argv[2]);

	DebugPrintf("%s = %d\n", argv[1], *globalVar);

	return true;
}

bool RivenConsole::Cmd_PlaySound(int argc, const char **argv) {
	if (argc < 2) {
		DebugPrintf("Usage: playSound <value> (<use main sound file, default = true>)\n");
		DebugPrintf("The main sound file is default, but you can use the word \'false\' to make it use the current stack file.\n");

		return true;
	}

	_vm->_sound->stopSound();
	_vm->_sound->stopAllSLST();
	
	bool mainSoundFile = (argc < 3) || (scumm_stricmp(argv[2], "false") != 0);
	
	_vm->_sound->playSound((uint16)atoi(argv[1]), mainSoundFile);

	return false;
}

bool RivenConsole::Cmd_PlaySLST(int argc, const char **argv) {
	if (argc < 2) {
		DebugPrintf("Usage: playSLST <slst index> <card, default = current>\n");

		return true;
	}

	_vm->_sound->stopSound();
	_vm->_sound->stopAllSLST();
	
	uint16 card = _vm->getCurCard();
	
	if (argc == 3)
		card = (uint16)atoi(argv[2]);
	
	_vm->_sound->playSLST((uint16)atoi(argv[1]), card);

	return false;
}

bool RivenConsole::Cmd_StopSound(int argc, const char **argv) {
	DebugPrintf("Stopping Sound\n");

	_vm->_sound->stopSound();
	_vm->_sound->stopAllSLST();

	return true;
}

bool RivenConsole::Cmd_CurStack(int argc, const char **argv) {
	DebugPrintf("Current Stack: %s\n", _vm->getStackName(_vm->getCurStack()).c_str());

	return true;
}

bool RivenConsole::Cmd_ChangeStack(int argc, const char **argv) {
	byte i;

	if (argc < 3) {
		DebugPrintf("Usage: changeStack <stack> <card>\n\n");
		DebugPrintf("Stacks:\n=======\n");

		for (i = 0; i <= tspit; i++)
			DebugPrintf(" %s\n", _vm->getStackName(i).c_str());

		DebugPrintf("\n");
		
		return true;
	}
	
	byte stackNum = 0;

	for (i = 1; i <= tspit + 1; i++)
		if (!scumm_stricmp(argv[1], _vm->getStackName(i - 1).c_str())) {
			stackNum = i;
			break;
		}

	if (!stackNum) {
		DebugPrintf("\'%s\' is not a stack name!\n", argv[1]);
		return true;
	}

	_vm->changeToStack(stackNum - 1);
	_vm->changeToCard((uint16)atoi(argv[2]));

	return false;
}

bool RivenConsole::Cmd_Restart(int argc, const char **argv) {
	_vm->initVars();
	_vm->changeToStack(aspit);
	_vm->changeToCard(1);

	return false;
}

bool RivenConsole::Cmd_Hotspots(int argc, const char **argv) {
	DebugPrintf("Current card (%d) has %d hotspots:\n", _vm->getCurCard(), _vm->getHotspotCount());

	for (uint16 i = 0; i < _vm->getHotspotCount(); i++) {
		DebugPrintf("Hotspot %d, index %d, BLST ID %d (", i, _vm->_hotspots[i].index, _vm->_hotspots[i].blstID);

		if (_vm->_hotspots[i].enabled)
			DebugPrintf("enabled)\n");
		else
			DebugPrintf("disabled)\n");
			
		DebugPrintf("    Name = %s\n", _vm->getHotspotName(i).c_str());
	}	

	return true;
}

bool RivenConsole::Cmd_ZipMode(int argc, const char **argv) {
	uint32 *zipModeActive = _vm->matchVarToString("azip");
	*zipModeActive = !(*zipModeActive);

	DebugPrintf("Zip Mode is ");
	DebugPrintf((*zipModeActive) ? "Enabled" : "Disabled");
	DebugPrintf("\n");
	return true;
}

bool RivenConsole::Cmd_DumpScript(int argc, const char **argv) {
	if (argc < 4) {
		DebugPrintf("Usage: dumpScript <stack> <CARD or HSPT> <card>\n");
		return true;
	}
	
	uint16 oldStack = _vm->getCurStack();
	
	byte newStack = 0;

	for (byte i = 1; i <= tspit + 1; i++)
		if (!scumm_stricmp(argv[1], _vm->getStackName(i - 1).c_str())) {
			newStack = i;
			break;
		}

	if (!newStack) {
		DebugPrintf("\'%s\' is not a stack name!\n", argv[1]);
		return true;
	}
	
	newStack--;
	_vm->changeToStack(newStack);
	
	// Load in Variable Names
	Common::SeekableReadStream *nameStream = _vm->getRawData(ID_NAME, VariableNames);
	Common::StringList varNames;
	
	uint16 namesCount = nameStream->readUint16BE();
	uint16 *stringOffsets = new uint16[namesCount];
	for (uint16 i = 0; i < namesCount; i++)
		stringOffsets[i] = nameStream->readUint16BE();
	nameStream->seek(namesCount * 2, SEEK_CUR);
	int32 curNamesPos = nameStream->pos();
	
	for (uint32 i = 0; i < namesCount; i++) {
		nameStream->seek(curNamesPos + stringOffsets[i]);
			
		Common::String name = Common::String::emptyString;
		for (char c = nameStream->readByte(); c; c = nameStream->readByte())
			name += c;
		varNames.push_back(name);
	}
	delete nameStream;
	
	// Load in External Command Names
	nameStream = _vm->getRawData(ID_NAME, ExternalCommandNames);
	Common::StringList xNames;
	
	namesCount = nameStream->readUint16BE();
	stringOffsets = new uint16[namesCount];
	for (uint16 i = 0; i < namesCount; i++)
		stringOffsets[i] = nameStream->readUint16BE();
	nameStream->seek(namesCount * 2, SEEK_CUR);
	curNamesPos = nameStream->pos();
	
	for (uint32 i = 0; i < namesCount; i++) {
		nameStream->seek(curNamesPos + stringOffsets[i]);
			
		Common::String name = Common::String::emptyString;
		for (char c = nameStream->readByte(); c; c = nameStream->readByte())
			name += c;
		xNames.push_back(name);
	}
	delete nameStream;
	
	// Get CARD/HSPT data and dump their scripts
	if (!scumm_stricmp(argv[2], "CARD")) {
		printf ("\n\nDumping scripts for %s\'s card %d!\n", argv[1], (uint16)atoi(argv[3]));
		printf ("==================================\n\n");
		Common::SeekableReadStream *cardStream = _vm->getRawData(MKID_BE('CARD'), (uint16)atoi(argv[3]));
		cardStream->seek(4);
		RivenScriptList scriptList = RivenScript::readScripts(_vm, cardStream);
		for (uint32 i = 0; i < scriptList.size(); i++)
			scriptList[i]->dumpScript(varNames, xNames, 0);
		delete cardStream;
	} else if (!scumm_stricmp(argv[2], "HSPT")) {
		printf ("\n\nDumping scripts for %s\'s card %d hotspots!\n", argv[1], (uint16)atoi(argv[3]));
		printf ("===========================================\n\n");
		
		Common::SeekableReadStream *hsptStream = _vm->getRawData(MKID_BE('HSPT'), (uint16)atoi(argv[3]));
		
		uint16 hotspotCount = hsptStream->readUint16BE();
		
		for (uint16 i = 0; i < hotspotCount; i++) {
			printf ("Hotspot %d:\n", i);
			hsptStream->seek(22, SEEK_CUR);	// Skip non-script related stuff
			RivenScriptList scriptList = RivenScript::readScripts(_vm, hsptStream);
			for (uint32 j = 0; j < scriptList.size(); j++)
				scriptList[j]->dumpScript(varNames, xNames, 1);
		}
		
		delete hsptStream;
	} else {
		DebugPrintf("%s doesn't have any scripts!\n", argv[2]);
	}
	
	printf("\n\n");
	
	_vm->changeToStack(oldStack);
	
	DebugPrintf("Script dump complete.\n");
	
	return true;
}

bool RivenConsole::Cmd_ListZipCards(int argc, const char **argv) {
	if (_vm->_zipModeData.size() == 0) {
		DebugPrintf("No zip card data.\n");
	} else {
		DebugPrintf("Listing zip cards:\n");
		for (uint32 i = 0; i < _vm->_zipModeData.size(); i++)
			DebugPrintf("ID = %d, Name = %s\n", _vm->_zipModeData[i].id, _vm->_zipModeData[i].name.c_str());
	}
	
	return true;
}

bool RivenConsole::Cmd_GetRMAP(int argc, const char **argv) {
	Common::SeekableReadStream *rmapStream = _vm->getRawData(ID_RMAP, 1);
	rmapStream->seek(_vm->getCurCard() * 4);
	DebugPrintf("RMAP for %s %d = %08x\n", _vm->getStackName(_vm->getCurStack()).c_str(), _vm->getCurCard(), rmapStream->readUint32BE());
	delete rmapStream;

	return true;
}

LivingBooksConsole::LivingBooksConsole(MohawkEngine_LivingBooks *vm) : GUI::Debugger(), _vm(vm) {
	DCmd_Register("playSound",			WRAP_METHOD(LivingBooksConsole, Cmd_PlaySound));
	DCmd_Register("stopSound",			WRAP_METHOD(LivingBooksConsole, Cmd_StopSound));
	DCmd_Register("drawImage",			WRAP_METHOD(LivingBooksConsole, Cmd_DrawImage));
}

LivingBooksConsole::~LivingBooksConsole() {
}

void LivingBooksConsole::preEnter() {
	_vm->_sound->pauseSound();
}

void LivingBooksConsole::postEnter() {
	_vm->_sound->resumeSound();
}

bool LivingBooksConsole::Cmd_PlaySound(int argc, const char **argv) {
	if (argc == 1) {
		DebugPrintf("Usage: playSound <value>\n");

		return true;
	}

	_vm->_sound->stopSound();
	_vm->_sound->playSound((uint16)atoi(argv[1]));

	return false;
}

bool LivingBooksConsole::Cmd_StopSound(int argc, const char **argv) {
	DebugPrintf("Stopping Sound\n");

	_vm->_sound->stopSound();

	return true;
}

bool LivingBooksConsole::Cmd_DrawImage(int argc, const char **argv) {
	if (argc == 1) {
		DebugPrintf("Usage: drawImage <value>\n");
		return true;
	}

	if (_vm->getGameType() == GType_OLDLIVINGBOOKS)
		DebugPrintf("This isn't supported in the old Living Books games (yet)!\n");

	_vm->_gfx->copyImageToScreen((uint16)atoi(argv[1]));
	return _vm->getGameType() != GType_OLDLIVINGBOOKS;
}

} // End of namespace Mohawk
