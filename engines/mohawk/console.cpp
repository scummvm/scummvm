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
#include "mohawk/cursors.h"
#include "mohawk/livingbooks.h"
#include "mohawk/resource.h"
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
#include "mohawk/myst_card.h"
#include "mohawk/myst_graphics.h"
#include "mohawk/myst_scripts.h"
#include "mohawk/myst_sound.h"
#endif

#ifdef ENABLE_RIVEN
#include "mohawk/riven.h"
#include "mohawk/riven_card.h"
#include "mohawk/riven_sound.h"
#include "mohawk/riven_stack.h"
#include "mohawk/riven_stacks/domespit.h"
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
	registerCmd("quickTest",            WRAP_METHOD(MystConsole, Cmd_QuickTest));
	registerVar("show_resource_rects",  &_vm->_showResourceRects);
}

MystConsole::~MystConsole() {
}

bool MystConsole::Cmd_ChangeCard(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Usage: changeCard <card>\n");
		return true;
	}

	_vm->_sound->stopEffect();
	_vm->changeToCard((uint16)atoi(argv[1]), kTransitionCopy);

	return false;
}

bool MystConsole::Cmd_CurCard(int argc, const char **argv) {
	debugPrintf("Current Card: %d\n", _vm->getCard()->getId());
	return true;
}

bool MystConsole::Cmd_Var(int argc, const char **argv) {
	if (argc == 1) {
		debugPrintf("Usage: var <var> (<value>)\n");
		return true;
	}

	if (argc > 2)
		_vm->_stack->setVarValue((uint16)atoi(argv[1]), (uint16)atoi(argv[2]));

	debugPrintf("%d = %d\n", (uint16)atoi(argv[1]), _vm->_stack->getVar((uint16)atoi(argv[1])));

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
	1,
	1,
	6122,
	4134,
	1282,
	1000,
	3000,
	2029
};

bool MystConsole::Cmd_CurStack(int argc, const char **argv) {
	debugPrintf("Current Stack: %s\n", mystStackNames[_vm->_stack->getStackId()]);
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
	_vm->_sound->stopEffect();

	uint16 card = 0;
	if (argc == 3)
		card = (uint16)atoi(argv[2]);
	else
		card = default_start_card[stackNum - 1];

	_vm->changeToStack(static_cast<MystStack>(stackNum - 1), card, 0, 0);

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
		if (resourceId < _vm->getCard()->_resources.size())
			_vm->getCard()->_resources[resourceId]->drawBoundingRect();
	}

	return false;
}

bool MystConsole::Cmd_SetResourceEnable(int argc, const char **argv) {
	if (argc < 3) {
		debugPrintf("Usage: setResourceEnable <resource id> <bool>\n");
		return true;
	}

	_vm->getCard()->setResourceEnabled((uint16)atoi(argv[1]), atoi(argv[2]) == 1);
	return true;
}

bool MystConsole::Cmd_PlaySound(int argc, const char **argv) {
	if (argc == 1) {
		debugPrintf("Usage: playSound <value>\n");

		return true;
	}

	_vm->_sound->playEffect((uint16) atoi(argv[1]));

	return false;
}

bool MystConsole::Cmd_StopSound(int argc, const char **argv) {
	debugPrintf("Stopping Sound\n");

	_vm->_sound->stopEffect();

	return true;
}

bool MystConsole::Cmd_PlayMovie(int argc, const char **argv) {
	if (argc < 3) {
		debugPrintf("Usage: playMovie <name> <stack> [<left> <top>]\n");
		debugPrintf("NOTE: The movie will play *once* in the background.\n");
		return true;
	}

	Common::String fileName = argv[1];
	int8 stackNum = -1;
	for (byte i = 0; i < ARRAYSIZE(mystStackNames); i++)
		if (!scumm_stricmp(argv[2], mystStackNames[i])) {
			stackNum = i;
			break;
		}

	if (stackNum < 0) {
		debugPrintf("\'%s\' is not a stack name!\n", argv[2]);
		return true;
	}

	VideoEntryPtr video = _vm->playMovie(fileName, static_cast<MystStack>(stackNum));

	if (argc == 4) {
		video->setX(atoi(argv[2]));
		video->setY(atoi(argv[3]));
	} else if (argc > 4) {
		video->setX(atoi(argv[3]));
		video->setY(atoi(argv[4]));
	} else {
		video->center();
	}

	return false;
}

bool MystConsole::Cmd_DisableInitOpcodes(int argc, const char **argv) {
	if (argc != 1) {
		debugPrintf("Usage: disableInitOpcodes\n");

		return true;
	}

	_vm->_stack->disablePersistentScripts();

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
	debugPrintf("Resources in card %d:\n", _vm->getCard()->getId());

	for (uint i = 0; i < _vm->getCard()->_resources.size(); i++) {
		debugPrintf("#%2d %s\n", i, _vm->getCard()->_resources[i]->describe().c_str());
	}

	return true;
}

bool MystConsole::Cmd_QuickTest(int argc, const char **argv) {
	_debugPauseToken.clear();

	// Go through all the ages, all the views and click random stuff
	for (uint i = 0; i < ARRAYSIZE(mystStackNames); i++) {
		MystStack stackId = static_cast<MystStack>(i);
		if (stackId == kDemoStack || stackId == kMakingOfStack
		    || stackId == kDemoSlidesStack || stackId == kDemoPreviewStack) continue;

		debug("Loading stack %s", mystStackNames[stackId]);
		_vm->changeToStack(stackId, default_start_card[stackId], 0, 0);

		Common::Array<uint16> ids = _vm->getResourceIDList(ID_VIEW);
		for (uint j = 0; j < ids.size(); j++) {
			if (ids[j] == 4632) continue;

			debug("Loading card %d", ids[j]);
			_vm->changeToCard(ids[j], kTransitionCopy);

			_vm->doFrame();

			{
				MystCardPtr card = _vm->getCardPtr();
				int16 resIndex = _vm->_rnd->getRandomNumber(card->_resources.size()) - 1;
				if (resIndex >= 0 && _vm->getCard()->_resources[resIndex]->isEnabled()) {
					card->_resources[resIndex]->handleMouseDown();
					card->_resources[resIndex]->handleMouseUp();
				}
			}

			_vm->doFrame();

			if (_vm->_stack->getStackId() != stackId) {
				// Clicking may have linked us to another age
				_vm->changeToStack(stackId, default_start_card[stackId], 0, 0);
			}
		}
	}

	_debugPauseToken = _vm->pauseEngine();
	return true;
}

#endif // ENABLE_MYST

#ifdef ENABLE_RIVEN

RivenConsole::RivenConsole(MohawkEngine_Riven *vm) : GUI::Debugger(), _vm(vm) {
	registerCmd("changeCard",     WRAP_METHOD(RivenConsole, Cmd_ChangeCard));
	registerCmd("curCard",        WRAP_METHOD(RivenConsole, Cmd_CurCard));
	registerCmd("dumpCard",       WRAP_METHOD(RivenConsole, Cmd_DumpCard));
	registerCmd("var",            WRAP_METHOD(RivenConsole, Cmd_Var));
	registerCmd("playSound",      WRAP_METHOD(RivenConsole, Cmd_PlaySound));
	registerCmd("playSLST",       WRAP_METHOD(RivenConsole, Cmd_PlaySLST));
	registerCmd("stopSound",      WRAP_METHOD(RivenConsole, Cmd_StopSound));
	registerCmd("curStack",       WRAP_METHOD(RivenConsole, Cmd_CurStack));
	registerCmd("dumpStack",      WRAP_METHOD(RivenConsole, Cmd_DumpStack));
	registerCmd("changeStack",    WRAP_METHOD(RivenConsole, Cmd_ChangeStack));
	registerCmd("hotspots",       WRAP_METHOD(RivenConsole, Cmd_Hotspots));
	registerCmd("zipMode",        WRAP_METHOD(RivenConsole, Cmd_ZipMode));
	registerCmd("dumpScript",     WRAP_METHOD(RivenConsole, Cmd_DumpScript));
	registerCmd("listZipCards",   WRAP_METHOD(RivenConsole, Cmd_ListZipCards));
	registerCmd("getRMAP",        WRAP_METHOD(RivenConsole, Cmd_GetRMAP));
	registerCmd("combos",         WRAP_METHOD(RivenConsole, Cmd_Combos));
	registerCmd("sliderState",    WRAP_METHOD(RivenConsole, Cmd_SliderState));
	registerCmd("quickTest",      WRAP_METHOD(RivenConsole, Cmd_QuickTest));
	registerVar("show_hotspots",  &_vm->_showHotspots);
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
	debugPrintf("Current Card: %d\n", _vm->getCard()->getId());

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
		debugPrintf("Usage: playSLST <slst index>\n");

		return true;
	}

	_vm->_sound->stopSound();
	_vm->_sound->stopAllSLST();

	_vm->getCard()->playSound((uint16)atoi(argv[1]));
	return false;
}

bool RivenConsole::Cmd_StopSound(int argc, const char **argv) {
	debugPrintf("Stopping Sound\n");

	_vm->_sound->stopSound();
	_vm->_sound->stopAllSLST();
	return true;
}

bool RivenConsole::Cmd_CurStack(int argc, const char **argv) {
	debugPrintf("Current Stack: %s\n", RivenStacks::getName(_vm->getStack()->getId()));

	return true;
}

bool RivenConsole::Cmd_ChangeStack(int argc, const char **argv) {
	if (argc < 3) {
		debugPrintf("Usage: changeStack <stack> <card>\n\n");
		debugPrintf("Stacks:\n=======\n");

		for (uint i = kStackFirst; i <= kStackLast; i++)
			debugPrintf(" %s\n", RivenStacks::getName(i));

		debugPrintf("\n");

		return true;
	}

	uint stackId = RivenStacks::getId(argv[1]);
	if (stackId == kStackUnknown) {
		debugPrintf("\'%s\' is not a stack name!\n", argv[1]);
		return true;
	}

	_vm->changeToStack(stackId);
	_vm->changeToCard((uint16)atoi(argv[2]));

	return false;
}

bool RivenConsole::Cmd_Hotspots(int argc, const char **argv) {
	Common::Array<RivenHotspot *> hotspots = _vm->getCard()->getHotspots();

	debugPrintf("Current card (%d) has %d hotspots:\n", _vm->getCard()->getId(), hotspots.size());

	for (uint16 i = 0; i < hotspots.size(); i++) {
		RivenHotspot *hotspot = hotspots[i];
		debugPrintf("Hotspot %d, index %d, BLST ID %d (", i, hotspot->getIndex(), hotspot->getBlstId());

		if (hotspot->isEnabled())
			debugPrintf("enabled");
		else
			debugPrintf("disabled");

		Common::Rect rect = hotspot->getRect();
		debugPrintf(") - (%d, %d, %d, %d)\n", rect.left, rect.top, rect.right, rect.bottom);
		debugPrintf("    Name = %s\n", hotspot->getName().c_str());
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

bool RivenConsole::Cmd_DumpCard(int argc, const char **argv) {
	if (argc != 1) {
		debugPrintf("Usage: dumpCard\n");
		return true;
	}

	_vm->getCard()->dump();

	debugPrintf("Card dump complete.\n");

	return true;
}

bool RivenConsole::Cmd_DumpStack(int argc, const char **argv) {
	if (argc != 1) {
		debugPrintf("Usage: dumpStack\n");
		return true;
	}

	_vm->getStack()->dump();

	debugPrintf("Stack dump complete.\n");

	return true;
}

bool RivenConsole::Cmd_DumpScript(int argc, const char **argv) {
	if (argc < 4) {
		debugPrintf("Usage: dumpScript <stack> <CARD or HSPT> <card>\n");
		return true;
	}

	uint16 oldStack = _vm->getStack()->getId();

	uint newStack = RivenStacks::getId(argv[1]);
	if (newStack == kStackUnknown) {
		debugPrintf("\'%s\' is not a stack name!\n", argv[1]);
		return true;
	}

	_vm->changeToStack(newStack);

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
		RivenScriptList scriptList = _vm->_scriptMan->readScripts(cardStream);
		for (uint32 i = 0; i < scriptList.size(); i++) {
			debugN("Stream Type %d:\n", scriptList[i].type);
			scriptList[i].script->dumpScript(0);
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
			RivenScriptList scriptList = _vm->_scriptMan->readScripts(hsptStream);
			for (uint32 j = 0; j < scriptList.size(); j++) {
				debugN("\tStream Type %d:\n", scriptList[j].type);
				scriptList[j].script->dumpScript(1);
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
	uint32 rmapCode = _vm->getStack()->getCurrentCardGlobalId();
	debugPrintf("RMAP for %s %d = %08x\n", RivenStacks::getName(_vm->getStack()->getId()), _vm->getCard()->getId(), rmapCode);
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
		debugPrintf("%d ", _vm->getStack()->getComboDigit(teleCombo, i));

	debugPrintf("\nPrison Combo:\n  ");
	for (int i = 0; i < 5; i++)
		debugPrintf("%d ", _vm->getStack()->getComboDigit(prisonCombo, i));

	debugPrintf("\nDome Combo:\n  ");
	for (int i = 1; i <= 25; i++)
		if (domeCombo & (1 << (25 - i)))
			debugPrintf("%d ", i);

	debugPrintf("\n");
	return true;
}

bool RivenConsole::Cmd_SliderState(int argc, const char **argv) {
	RivenStacks::DomeSpit *domeSpit = dynamic_cast<RivenStacks::DomeSpit *>(_vm->getStack());
	if (!domeSpit) {
		debugPrintf("No dome in this stack\n");
		return true;
	}

	if (argc > 1)
		domeSpit->setDomeSliderState((uint32)atoi(argv[1]));

	debugPrintf("Dome Slider State = %08x\n", domeSpit->getDomeSliderState());
	return true;
}

bool RivenConsole::Cmd_QuickTest(int argc, const char **argv) {
	_debugPauseToken.clear();

	// Go through all the stacks, all the cards and click random stuff
	for (uint16 stackId = kStackFirst; stackId <= kStackLast; stackId++) {

		debug("Loading stack %s", RivenStacks::getName(stackId));
		_vm->changeToStack(stackId);

		Common::Array<uint16> cardIds = _vm->getResourceIDList(ID_CARD);
		for (uint16 i = 0; i < cardIds.size(); i++) {
			if (_vm->shouldQuit()) break;

			uint16 cardId = cardIds[i];
			if (stackId == kStackTspit && cardId == 366) continue; // Cut card with invalid links
			if (stackId == kStackTspit && cardId == 412) continue; // Cut card with invalid links
			if (stackId == kStackTspit && cardId == 486) continue; // Cut card with invalid links
			if (stackId == kStackBspit && cardId == 465) continue; // Cut card with invalid links
			if (stackId == kStackJspit && cardId == 737) continue; // Cut card with invalid links

			debug("Loading card %d", cardId);
			RivenScriptPtr script = _vm->_scriptMan->createScriptFromData(1,
			                            kRivenCommandChangeCard, 1, cardId);
			_vm->_scriptMan->runScript(script, true);

			_vm->_gfx->setTransitionMode(kRivenTransitionModeDisabled);

			while (_vm->_scriptMan->hasQueuedScripts()) {
				_vm->doFrame();
			}

			// Click on a random hotspot
			Common::Array<RivenHotspot *> hotspots = _vm->getCard()->getHotspots();
			if (!hotspots.empty() && _vm->getStack()->getId() != kStackAspit) {
				uint hotspotIndex = _vm->_rnd->getRandomNumberRng(0, hotspots.size() - 1);
				RivenHotspot *hotspot = hotspots[hotspotIndex];
				if (hotspot->isEnabled()) {
					Common::Rect hotspotRect = hotspot->getRect();
					Common::Point hotspotPoint((hotspotRect.left + hotspotRect.right) / 2, (hotspotRect.top + hotspotRect.bottom) / 2);
					_vm->getStack()->onMouseDown(hotspotPoint);
					_vm->getStack()->onMouseUp(hotspotPoint);
				}

				while (_vm->_scriptMan->hasQueuedScripts()) {
					_vm->doFrame();
				}
			}

			if (_vm->getStack()->getId() != stackId) {
				// Clicking may have linked us to another age
				_vm->changeToStack(stackId);
			}
		}
	}

	_debugPauseToken = _vm->pauseEngine();
	return true;
}

#endif // ENABLE_RIVEN

LivingBooksConsole::LivingBooksConsole(MohawkEngine_LivingBooks *vm) : GUI::Debugger(), _vm(vm) {
	registerCmd("playSound",			WRAP_METHOD(LivingBooksConsole, Cmd_PlaySound));
	registerCmd("stopSound",			WRAP_METHOD(LivingBooksConsole, Cmd_StopSound));
	registerCmd("drawImage",			WRAP_METHOD(LivingBooksConsole, Cmd_DrawImage));
	registerCmd("changePage",			WRAP_METHOD(LivingBooksConsole, Cmd_ChangePage));
	registerCmd("changeCursor",			WRAP_METHOD(LivingBooksConsole, Cmd_ChangeCursor));
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

bool LivingBooksConsole::Cmd_ChangeCursor(int argc, const char **argv) {
	if (argc == 1) {
		debugPrintf("Usage: changeCursor <value>\n");
		return true;
	}

	_vm->_cursor->setCursor((uint16)atoi(argv[1]));
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
