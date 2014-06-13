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

#include "mohawk/console.h"
#include "mohawk/livingbooks.h"
#include "mohawk/sound.h"
#include "mohawk/video.h"

#include "common/system.h"
#include "common/textconsole.h"

#ifdef ENABLE_CSTIME
#include "mohawk/cstime.h"
#endif

#ifdef ENABLE_MYST
#include "mohawk/myst.h"
#include "mohawk/myst_areas.h"
#include "mohawk/myst_graphics.h"
#include "mohawk/myst_scripts.h"
#endif

#ifdef ENABLE_RIVEN
#include "mohawk/riven.h"
#include "mohawk/riven_external.h"
#endif

namespace Mohawk {

#ifdef ENABLE_MYST

MystConsole::MystConsole(MohawkEngine_Myst *vm) : GUI::Debugger(), _vm(vm) {
	registerCmd("changeCard",			WRAP_METHOD(MystConsole, Cmd_ChangeCard));
	registerCmd("curCard",			WRAP_METHOD(MystConsole, Cmd_CurCard));
	registerCmd("var",				WRAP_METHOD(MystConsole, Cmd_Var));
	registerCmd("curStack",			WRAP_METHOD(MystConsole, Cmd_CurStack));
	registerCmd("changeStack",		WRAP_METHOD(MystConsole, Cmd_ChangeStack));
	registerCmd("drawImage",			WRAP_METHOD(MystConsole, Cmd_DrawImage));
	registerCmd("drawRect",			WRAP_METHOD(MystConsole, Cmd_DrawRect));
	registerCmd("setResourceEnable",	WRAP_METHOD(MystConsole, Cmd_SetResourceEnable));
	registerCmd("playSound",			WRAP_METHOD(MystConsole, Cmd_PlaySound));
	registerCmd("stopSound",			WRAP_METHOD(MystConsole, Cmd_StopSound));
	registerCmd("playMovie",			WRAP_METHOD(MystConsole, Cmd_PlayMovie));
	registerCmd("disableInitOpcodes",	WRAP_METHOD(MystConsole, Cmd_DisableInitOpcodes));
	registerCmd("cache",				WRAP_METHOD(MystConsole, Cmd_Cache));
	registerCmd("resources",			WRAP_METHOD(MystConsole, Cmd_Resources));
}

MystConsole::~MystConsole() {
}

bool MystConsole::Cmd_ChangeCard(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Usage: changeCard <card>\n");
		return true;
	}

	_vm->_sound->stopSound();
	_vm->changeToCard((uint16)atoi(argv[1]), kTransitionCopy);

	return false;
}

bool MystConsole::Cmd_CurCard(int argc, const char **argv) {
	debugPrintf("Current Card: %d\n", _vm->getCurCard());
	return true;
}

bool MystConsole::Cmd_Var(int argc, const char **argv) {
	if (argc == 1) {
		debugPrintf("Usage: var <var> (<value>)\n");
		return true;
	}

	if (argc > 2)
		_vm->_scriptParser->setVarValue((uint16)atoi(argv[1]), (uint16)atoi(argv[2]));

	debugPrintf("%d = %d\n", (uint16)atoi(argv[1]), _vm->_scriptParser->getVar((uint16)atoi(argv[1])));

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
	2000,
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
	debugPrintf("Current Stack: %s\n", mystStackNames[_vm->getCurStack()]);
	return true;
}

bool MystConsole::Cmd_ChangeStack(int argc, const char **argv) {
	if (argc != 2 && argc != 3) {
		debugPrintf("Usage: changeStack <stack> [<card>]\n\n");
		debugPrintf("Stacks:\n=======\n");

		for (byte i = 0; i < ARRAYSIZE(mystStackNames); i++)
			debugPrintf(" %s\n", mystStackNames[i]);

		debugPrintf("\n");

		return true;
	}

	byte stackNum = 0;

	for (byte i = 1; i <= ARRAYSIZE(mystStackNames); i++)
		if (!scumm_stricmp(argv[1], mystStackNames[i - 1])) {
			stackNum = i;
			break;
		}

	if (!stackNum) {
		debugPrintf("\'%s\' is not a stack name!\n", argv[1]);
		return true;
	}

	// We need to stop any playing sound when we change the stack
	// as the next card could continue playing it if it.
	_vm->_sound->stopSound();

	uint16 card = 0;
	if (argc == 3)
		card = (uint16)atoi(argv[2]);
	else
		card = default_start_card[stackNum - 1];

	_vm->changeToStack(stackNum - 1, card, 0, 0);

	return false;
}

bool MystConsole::Cmd_DrawImage(int argc, const char **argv) {
	if (argc != 2 && argc != 6) {
		debugPrintf("Usage: drawImage <image> [<left> <top> <right> <bottom>]\n");
		return true;
	}

	Common::Rect rect;

	if (argc == 2)
		rect = Common::Rect(0, 0, 544, 333);
	else
		rect = Common::Rect((uint16)atoi(argv[2]), (uint16)atoi(argv[3]), (uint16)atoi(argv[4]), (uint16)atoi(argv[5]));

	_vm->_gfx->copyImageToScreen((uint16)atoi(argv[1]), rect);
	_vm->_system->updateScreen();
	return false;
}

bool MystConsole::Cmd_DrawRect(int argc, const char **argv) {
	if (argc != 5 && argc != 2) {
		debugPrintf("Usage: drawRect <left> <top> <right> <bottom>\n");
		debugPrintf("Usage: drawRect <resource id>\n");
		return true;
	}

	if (argc == 5) {
		_vm->_gfx->drawRect(Common::Rect((uint16)atoi(argv[1]), (uint16)atoi(argv[2]), (uint16)atoi(argv[3]), (uint16)atoi(argv[4])), kRectEnabled);
	} else if (argc == 2) {
		uint16 resourceId = (uint16)atoi(argv[1]);
		if (resourceId < _vm->_resources.size())
			_vm->_resources[resourceId]->drawBoundingRect();
	}

	return false;
}

bool MystConsole::Cmd_SetResourceEnable(int argc, const char **argv) {
	if (argc < 3) {
		debugPrintf("Usage: setResourceEnable <resource id> <bool>\n");
		return true;
	}

	_vm->setResourceEnabled((uint16)atoi(argv[1]), atoi(argv[2]) == 1);
	return true;
}

bool MystConsole::Cmd_PlaySound(int argc, const char **argv) {
	if (argc == 1) {
		debugPrintf("Usage: playSound <value>\n");

		return true;
	}

	_vm->_sound->replaceSoundMyst((uint16)atoi(argv[1]));

	return false;
}

bool MystConsole::Cmd_StopSound(int argc, const char **argv) {
	debugPrintf("Stopping Sound\n");

	_vm->_sound->stopSound();

	return true;
}

bool MystConsole::Cmd_PlayMovie(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Usage: playMovie <name> [<stack>] [<left> <top>]\n");
		debugPrintf("NOTE: The movie will play *once* in the background.\n");
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
			debugPrintf("\'%s\' is not a stack name!\n", argv[2]);
			return true;
		}
	}

	if (argc == 2)
		_vm->_video->playMovie(argv[1], 0, 0);
	else if (argc == 3)
		_vm->_video->playMovie(_vm->wrapMovieFilename(argv[1], stackNum - 1), 0, 0);
	else if (argc == 4)
		_vm->_video->playMovie(argv[1], atoi(argv[2]), atoi(argv[3]));
	else
		_vm->_video->playMovie(_vm->wrapMovieFilename(argv[1], stackNum - 1), atoi(argv[3]), atoi(argv[4]));

	return false;
}

bool MystConsole::Cmd_DisableInitOpcodes(int argc, const char **argv) {
	if (argc != 1) {
		debugPrintf("Usage: disableInitOpcodes\n");

		return true;
	}

	_vm->_scriptParser->disablePersistentScripts();

	return true;
}

bool MystConsole::Cmd_Cache(int argc, const char **argv) {
	if (argc > 2) {
		debugPrintf("Usage: cache on/off - Omit parameter to get current state\n");
		return true;
	}

	bool state = false;

	if (argc == 1) {
		state = _vm->getCacheState();
	} else {
		if (!scumm_stricmp(argv[1], "on"))
			state = true;

		_vm->setCacheState(state);
	}

	debugPrintf("Cache: %s\n", state ? "Enabled" : "Disabled");
	return true;
}

bool MystConsole::Cmd_Resources(int argc, const char **argv) {
	debugPrintf("Resources in card %d:\n", _vm->getCurCard());

	for (uint i = 0; i < _vm->_resources.size(); i++) {
		debugPrintf("#%2d %s\n", i, _vm->_resources[i]->describe().c_str());
	}

	return true;
}

#endif // ENABLE_MYST

#ifdef ENABLE_RIVEN

RivenConsole::RivenConsole(MohawkEngine_Riven *vm) : GUI::Debugger(), _vm(vm) {
	registerCmd("changeCard",		WRAP_METHOD(RivenConsole, Cmd_ChangeCard));
	registerCmd("curCard",		WRAP_METHOD(RivenConsole, Cmd_CurCard));
	registerCmd("var",			WRAP_METHOD(RivenConsole, Cmd_Var));
	registerCmd("playSound",		WRAP_METHOD(RivenConsole, Cmd_PlaySound));
	registerCmd("playSLST",       WRAP_METHOD(RivenConsole, Cmd_PlaySLST));
	registerCmd("stopSound",		WRAP_METHOD(RivenConsole, Cmd_StopSound));
	registerCmd("curStack",		WRAP_METHOD(RivenConsole, Cmd_CurStack));
	registerCmd("changeStack",	WRAP_METHOD(RivenConsole, Cmd_ChangeStack));
	registerCmd("hotspots",		WRAP_METHOD(RivenConsole, Cmd_Hotspots));
	registerCmd("zipMode",		WRAP_METHOD(RivenConsole, Cmd_ZipMode));
	registerCmd("dumpScript",     WRAP_METHOD(RivenConsole, Cmd_DumpScript));
	registerCmd("listZipCards",   WRAP_METHOD(RivenConsole, Cmd_ListZipCards));
	registerCmd("getRMAP",		WRAP_METHOD(RivenConsole, Cmd_GetRMAP));
	registerCmd("combos",         WRAP_METHOD(RivenConsole, Cmd_Combos));
	registerCmd("sliderState",    WRAP_METHOD(RivenConsole, Cmd_SliderState));
}

RivenConsole::~RivenConsole() {
}


bool RivenConsole::Cmd_ChangeCard(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Usage: changeCard <card>\n");
		return true;
	}

	_vm->_sound->stopSound();
	_vm->_sound->stopAllSLST();
	_vm->changeToCard((uint16)atoi(argv[1]));

	return false;
}

bool RivenConsole::Cmd_CurCard(int argc, const char **argv) {
	debugPrintf("Current Card: %d\n", _vm->getCurCard());

	return true;
}

bool RivenConsole::Cmd_Var(int argc, const char **argv) {
	if (argc == 1) {
		debugPrintf("Usage: var <var name> (<value>)\n");
		return true;
	}

	if (!_vm->_vars.contains(argv[1])) {
		debugPrintf("Unknown variable '%s'\n", argv[1]);
		return true;
	}

	uint32 &var = _vm->_vars[argv[1]];

	if (argc > 2)
		var = (uint32)atoi(argv[2]);

	debugPrintf("%s = %d\n", argv[1], var);
	return true;
}

bool RivenConsole::Cmd_PlaySound(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Usage: playSound <value>\n");
		return true;
	}

	_vm->_sound->stopSound();
	_vm->_sound->stopAllSLST();
	_vm->_sound->playSound((uint16)atoi(argv[1]));
	return false;
}

bool RivenConsole::Cmd_PlaySLST(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Usage: playSLST <slst index> <card, default = current>\n");

		return true;
	}

	_vm->_sound->stopSound();
	_vm->_sound->stopAllSLST();

	uint16 card = (argc == 3) ? (uint16)atoi(argv[2]) : _vm->getCurCard();

	_vm->_sound->playSLST((uint16)atoi(argv[1]), card);
	return false;
}

bool RivenConsole::Cmd_StopSound(int argc, const char **argv) {
	debugPrintf("Stopping Sound\n");

	_vm->_sound->stopSound();
	_vm->_sound->stopAllSLST();
	return true;
}

bool RivenConsole::Cmd_CurStack(int argc, const char **argv) {
	debugPrintf("Current Stack: %s\n", _vm->getStackName(_vm->getCurStack()).c_str());

	return true;
}

bool RivenConsole::Cmd_ChangeStack(int argc, const char **argv) {
	if (argc < 3) {
		debugPrintf("Usage: changeStack <stack> <card>\n\n");
		debugPrintf("Stacks:\n=======\n");

		for (uint i = kStackFirst; i <= kStackLast; i++)
			debugPrintf(" %s\n", _vm->getStackName(i).c_str());

		debugPrintf("\n");

		return true;
	}

	uint stack = kStackUnknown;

	for (uint i = kStackFirst; i <= kStackLast; i++) {
		if (!scumm_stricmp(argv[1], _vm->getStackName(i).c_str())) {
			stack = i;
			break;
		}
	}

	if (stack == kStackUnknown) {
		debugPrintf("\'%s\' is not a stack name!\n", argv[1]);
		return true;
	}

	_vm->changeToStack(stack);
	_vm->changeToCard((uint16)atoi(argv[2]));

	return false;
}

bool RivenConsole::Cmd_Hotspots(int argc, const char **argv) {
	debugPrintf("Current card (%d) has %d hotspots:\n", _vm->getCurCard(), _vm->getHotspotCount());

	for (uint16 i = 0; i < _vm->getHotspotCount(); i++) {
		debugPrintf("Hotspot %d, index %d, BLST ID %d (", i, _vm->_hotspots[i].index, _vm->_hotspots[i].blstID);

		if (_vm->_hotspots[i].enabled)
			debugPrintf("enabled");
		else
			debugPrintf("disabled");

		debugPrintf(") - (%d, %d, %d, %d)\n", _vm->_hotspots[i].rect.left, _vm->_hotspots[i].rect.top, _vm->_hotspots[i].rect.right, _vm->_hotspots[i].rect.bottom);
		debugPrintf("    Name = %s\n", _vm->getHotspotName(i).c_str());
	}

	return true;
}

bool RivenConsole::Cmd_ZipMode(int argc, const char **argv) {
	uint32 &zipModeActive = _vm->_vars["azip"];
	zipModeActive = !zipModeActive;

	debugPrintf("Zip Mode is ");
	debugPrintf(zipModeActive ? "Enabled" : "Disabled");
	debugPrintf("\n");
	return true;
}

bool RivenConsole::Cmd_DumpScript(int argc, const char **argv) {
	if (argc < 4) {
		debugPrintf("Usage: dumpScript <stack> <CARD or HSPT> <card>\n");
		return true;
	}

	uint16 oldStack = _vm->getCurStack();
	uint newStack = kStackUnknown;

	for (uint i = kStackFirst; i <= kStackLast; i++) {
		if (!scumm_stricmp(argv[1], _vm->getStackName(i).c_str())) {
			newStack = i;
			break;
		}
	}

	if (newStack == kStackUnknown) {
		debugPrintf("\'%s\' is not a stack name!\n", argv[1]);
		return true;
	}

	_vm->changeToStack(newStack);

	// Load in Variable Names
	Common::SeekableReadStream *nameStream = _vm->getResource(ID_NAME, VariableNames);
	Common::StringArray varNames;

	uint16 namesCount = nameStream->readUint16BE();
	uint16 *stringOffsets = new uint16[namesCount];
	for (uint16 i = 0; i < namesCount; i++)
		stringOffsets[i] = nameStream->readUint16BE();
	nameStream->seek(namesCount * 2, SEEK_CUR);
	int32 curNamesPos = nameStream->pos();

	for (uint32 i = 0; i < namesCount; i++) {
		nameStream->seek(curNamesPos + stringOffsets[i]);

		Common::String name;
		for (char c = nameStream->readByte(); c; c = nameStream->readByte())
			name += c;
		varNames.push_back(name);
	}
	delete nameStream;
	delete[] stringOffsets;

	// Load in External Command Names
	nameStream = _vm->getResource(ID_NAME, ExternalCommandNames);
	Common::StringArray xNames;

	namesCount = nameStream->readUint16BE();
	stringOffsets = new uint16[namesCount];
	for (uint16 i = 0; i < namesCount; i++)
		stringOffsets[i] = nameStream->readUint16BE();
	nameStream->seek(namesCount * 2, SEEK_CUR);
	curNamesPos = nameStream->pos();

	for (uint32 i = 0; i < namesCount; i++) {
		nameStream->seek(curNamesPos + stringOffsets[i]);

		Common::String name;
		for (char c = nameStream->readByte(); c; c = nameStream->readByte())
			name += c;
		xNames.push_back(name);
	}
	delete nameStream;
	delete[] stringOffsets;

	// Get CARD/HSPT data and dump their scripts
	if (!scumm_stricmp(argv[2], "CARD")) {
		// Use debugN to print these because the scripts can get very large and would
		// really be useless if the the text console is not used. A DumpFile could also
		// theoretically be used, but I (clone2727) typically use this dynamically and
		// don't want countless files laying around without game context. If one would
		// want a file of a script they could just redirect stdout to a file or use
		// deriven.
		debugN("\n\nDumping scripts for %s\'s card %d!\n", argv[1], (uint16)atoi(argv[3]));
		debugN("==================================\n\n");
		Common::SeekableReadStream *cardStream = _vm->getResource(MKTAG('C','A','R','D'), (uint16)atoi(argv[3]));
		cardStream->seek(4);
		RivenScriptList scriptList = _vm->_scriptMan->readScripts(cardStream, false);
		for (uint32 i = 0; i < scriptList.size(); i++) {
			scriptList[i]->dumpScript(varNames, xNames, 0);
			delete scriptList[i];
		}
		delete cardStream;
	} else if (!scumm_stricmp(argv[2], "HSPT")) {
		// See above for why this is printed via debugN
		debugN("\n\nDumping scripts for %s\'s card %d hotspots!\n", argv[1], (uint16)atoi(argv[3]));
		debugN("===========================================\n\n");

		Common::SeekableReadStream *hsptStream = _vm->getResource(MKTAG('H','S','P','T'), (uint16)atoi(argv[3]));

		uint16 hotspotCount = hsptStream->readUint16BE();

		for (uint16 i = 0; i < hotspotCount; i++) {
			debugN("Hotspot %d:\n", i);
			hsptStream->seek(22, SEEK_CUR);	// Skip non-script related stuff
			RivenScriptList scriptList = _vm->_scriptMan->readScripts(hsptStream, false);
			for (uint32 j = 0; j < scriptList.size(); j++) {
				scriptList[j]->dumpScript(varNames, xNames, 1);
				delete scriptList[j];
			}
		}

		delete hsptStream;
	} else {
		debugPrintf("%s doesn't have any scripts!\n", argv[2]);
	}

	// See above for why this is printed via debugN
	debugN("\n\n");

	_vm->changeToStack(oldStack);

	debugPrintf("Script dump complete.\n");

	return true;
}

bool RivenConsole::Cmd_ListZipCards(int argc, const char **argv) {
	if (_vm->_zipModeData.size() == 0) {
		debugPrintf("No zip card data.\n");
	} else {
		debugPrintf("Listing zip cards:\n");
		for (uint32 i = 0; i < _vm->_zipModeData.size(); i++)
			debugPrintf("ID = %d, Name = %s\n", _vm->_zipModeData[i].id, _vm->_zipModeData[i].name.c_str());
	}

	return true;
}

bool RivenConsole::Cmd_GetRMAP(int argc, const char **argv) {
	uint32 rmapCode = _vm->getCurCardRMAP();
	debugPrintf("RMAP for %s %d = %08x\n", _vm->getStackName(_vm->getCurStack()).c_str(), _vm->getCurCard(), rmapCode);
	return true;
}

bool RivenConsole::Cmd_Combos(int argc, const char **argv) {
	// In the vain of SCUMM's 'drafts' command, this command will list
	// out all combinations needed in Riven, decoded from the variables.
	// You'll need to look up the Rebel Tunnel puzzle on your own; the
	// solution is constant.

	uint32 teleCombo = _vm->_vars["tcorrectorder"];
	uint32 prisonCombo = _vm->_vars["pcorrectorder"];
	uint32 domeCombo = _vm->_vars["adomecombo"];

	debugPrintf("Telescope Combo:\n  ");
	for (int i = 0; i < 5; i++)
		debugPrintf("%d ", _vm->_externalScriptHandler->getComboDigit(teleCombo, i));

	debugPrintf("\nPrison Combo:\n  ");
	for (int i = 0; i < 5; i++)
		debugPrintf("%d ", _vm->_externalScriptHandler->getComboDigit(prisonCombo, i));

	debugPrintf("\nDome Combo:\n  ");
	for (int i = 1; i <= 25; i++)
		if (domeCombo & (1 << (25 - i)))
			debugPrintf("%d ", i);

	debugPrintf("\n");
	return true;
}

bool RivenConsole::Cmd_SliderState(int argc, const char **argv) {
	if (argc > 1)
		_vm->_externalScriptHandler->setDomeSliderState((uint32)atoi(argv[1]));

	debugPrintf("Dome Slider State = %08x\n", _vm->_externalScriptHandler->getDomeSliderState());
	return true;
}

#endif // ENABLE_RIVEN

LivingBooksConsole::LivingBooksConsole(MohawkEngine_LivingBooks *vm) : GUI::Debugger(), _vm(vm) {
	registerCmd("playSound",			WRAP_METHOD(LivingBooksConsole, Cmd_PlaySound));
	registerCmd("stopSound",			WRAP_METHOD(LivingBooksConsole, Cmd_StopSound));
	registerCmd("drawImage",			WRAP_METHOD(LivingBooksConsole, Cmd_DrawImage));
	registerCmd("changePage",			WRAP_METHOD(LivingBooksConsole, Cmd_ChangePage));
}

LivingBooksConsole::~LivingBooksConsole() {
}

bool LivingBooksConsole::Cmd_PlaySound(int argc, const char **argv) {
	if (argc == 1) {
		debugPrintf("Usage: playSound <value>\n");
		return true;
	}

	_vm->_sound->stopSound();
	_vm->_sound->playSound((uint16)atoi(argv[1]));
	return false;
}

bool LivingBooksConsole::Cmd_StopSound(int argc, const char **argv) {
	debugPrintf("Stopping Sound\n");

	_vm->_sound->stopSound();
	return true;
}

bool LivingBooksConsole::Cmd_DrawImage(int argc, const char **argv) {
	if (argc == 1) {
		debugPrintf("Usage: drawImage <value>\n");
		return true;
	}

	_vm->_gfx->copyAnimImageToScreen((uint16)atoi(argv[1]));
	_vm->_system->updateScreen();
	return false;
}

bool LivingBooksConsole::Cmd_ChangePage(int argc, const char **argv) {
	if (argc < 2 || argc > 3) {
		debugPrintf("Usage: changePage <page>[.<subpage>] [<mode>]\n");
		return true;
	}

	int page, subpage = 0;
	if (sscanf(argv[1], "%d.%d", &page, &subpage) == 0) {
		debugPrintf("Usage: changePage <page>[.<subpage>] [<mode>]\n");
		return true;
	}
	LBMode mode = argc == 2 ? _vm->getCurMode() : (LBMode)atoi(argv[2]);
	if (subpage == 0) {
		if (_vm->tryLoadPageStart(mode, page))
			return false;
	} else {
		if (_vm->loadPage(mode, page, subpage))
			return false;
	}
	debugPrintf("no such page %d.%d\n", page, subpage);
	return true;
}

#ifdef ENABLE_CSTIME

CSTimeConsole::CSTimeConsole(MohawkEngine_CSTime *vm) : GUI::Debugger(), _vm(vm) {
	registerCmd("playSound",			WRAP_METHOD(CSTimeConsole, Cmd_PlaySound));
	registerCmd("stopSound",			WRAP_METHOD(CSTimeConsole, Cmd_StopSound));
	registerCmd("drawImage",			WRAP_METHOD(CSTimeConsole, Cmd_DrawImage));
	registerCmd("drawSubimage",			WRAP_METHOD(CSTimeConsole, Cmd_DrawSubimage));
	registerCmd("changeCase",			WRAP_METHOD(CSTimeConsole, Cmd_ChangeCase));
	registerCmd("changeScene",			WRAP_METHOD(CSTimeConsole, Cmd_ChangeScene));
	registerCmd("caseVariable",			WRAP_METHOD(CSTimeConsole, Cmd_CaseVariable));
	registerCmd("invItem",			WRAP_METHOD(CSTimeConsole, Cmd_InvItem));
}

CSTimeConsole::~CSTimeConsole() {
}

bool CSTimeConsole::Cmd_PlaySound(int argc, const char **argv) {
	if (argc == 1) {
		debugPrintf("Usage: playSound <value>\n");
		return true;
	}

	_vm->_sound->stopSound();
	_vm->_sound->playSound((uint16)atoi(argv[1]));
	return false;
}

bool CSTimeConsole::Cmd_StopSound(int argc, const char **argv) {
	debugPrintf("Stopping Sound\n");

	_vm->_sound->stopSound();
	return true;
}

bool CSTimeConsole::Cmd_DrawImage(int argc, const char **argv) {
	if (argc == 1) {
		debugPrintf("Usage: drawImage <value>\n");
		return true;
	}

	_vm->_gfx->copyAnimImageToScreen((uint16)atoi(argv[1]));
	_vm->_system->updateScreen();
	return false;
}

bool CSTimeConsole::Cmd_DrawSubimage(int argc, const char **argv) {
	if (argc < 3) {
		debugPrintf("Usage: drawSubimage <value> <subimage>\n");
		return true;
	}

	_vm->_gfx->copyAnimSubImageToScreen((uint16)atoi(argv[1]), (uint16)atoi(argv[2]));
	_vm->_system->updateScreen();
	return false;
}

bool CSTimeConsole::Cmd_ChangeCase(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Usage: changeCase <value>\n");
		return true;
	}

	error("Can't change case yet"); // FIXME
	return false;
}

bool CSTimeConsole::Cmd_ChangeScene(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Usage: changeScene <value>\n");
		return true;
	}

	_vm->addEvent(CSTimeEvent(kCSTimeEventNewScene, 0xffff, atoi(argv[1])));
	return false;
}

bool CSTimeConsole::Cmd_CaseVariable(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Usage: caseVariable <id> [<value>]\n");
		return true;
	}

	if (argc == 2) {
		debugPrintf("case variable %d has value %d\n", atoi(argv[1]), _vm->_caseVariable[atoi(argv[1])]);
	} else {
		_vm->_caseVariable[atoi(argv[1])] = atoi(argv[2]);
	}
	return true;
}

bool CSTimeConsole::Cmd_InvItem(int argc, const char **argv) {
	if (argc < 3) {
		debugPrintf("Usage: invItem <id> <0 or 1>\n");
		return true;
	}

	if (atoi(argv[2])) {
		_vm->addEvent(CSTimeEvent(kCSTimeEventDropItemInInventory, 0xffff, atoi(argv[1])));
	} else {
		_vm->addEvent(CSTimeEvent(kCSTimeEventRemoveItemFromInventory, 0xffff, atoi(argv[1])));
	}
	return false;
}

#endif // ENABLE_CSTIME

} // End of namespace Mohawk
