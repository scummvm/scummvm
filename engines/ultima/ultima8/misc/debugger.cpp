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

#include "ultima/ultima8/misc/debugger.h"
#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/audio/audio_process.h"
#include "ultima/ultima8/conf/setting_manager.h"
#include "ultima/ultima8/gumps/game_map_gump.h"
#include "ultima/ultima8/kernel/allocator.h"
#include "ultima/ultima8/kernel/hid_manager.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/kernel/memory_manager.h"
#include "ultima/ultima8/kernel/object_manager.h"
#include "ultima/ultima8/misc/id_man.h"
#include "ultima/ultima8/usecode/uc_machine.h"
#include "ultima/ultima8/world/actors/quick_avatar_mover_process.h"
#include "ultima/ultima8/world/world.h"

namespace Ultima {
namespace Ultima8 {

Debugger::Debugger() : Shared::Debugger() {
	registerCmd("quit", WRAP_METHOD(Debugger, cmdQuit));
	registerCmd("Ultima8Engine::quit", WRAP_METHOD(Debugger, cmdQuit));
	registerCmd("Ultima8Engine::saveGame", WRAP_METHOD(Debugger, cmdSaveGame));
	registerCmd("Ultima8Engine::loadGame", WRAP_METHOD(Debugger, cmdLoadGame));
	registerCmd("Ultima8Engine::newGame", WRAP_METHOD(Debugger, cmdNewGame));
	registerCmd("Ultima8Engine::_drawRenderStats", WRAP_METHOD(Debugger, cmdDrawRenderStats));
	registerCmd("Ultima8Engine::engineStats", WRAP_METHOD(Debugger, cmdEngineStats));
	registerCmd("Ultima8Engine::changeGame", WRAP_METHOD(Debugger, cmdChangeGame));
	registerCmd("Ultima8Engine::listGames", WRAP_METHOD(Debugger, cmdListGames));
	registerCmd("Ultima8Engine::memberVar", WRAP_METHOD(Debugger, cmdMemberVar));
	registerCmd("Ultima8Engine::setVideoMode", WRAP_METHOD(Debugger, cmdSetVideoMode));
	registerCmd("Ultima8Engine::toggleAvatarInStasis", WRAP_METHOD(Debugger, cmdToggleAvatarInStasis));
	registerCmd("Ultima8Engine::togglePaintEditorItems", WRAP_METHOD(Debugger, cmdTogglePaintEditorItems));
	registerCmd("Ultima8Engine::toggleShowTouchingItems", WRAP_METHOD(Debugger, cmdToggleShowTouchingItems));
	registerCmd("Ultima8Engine::closeItemGumps", WRAP_METHOD(Debugger, cmdCloseItemGumps));

	registerCmd("AudioProcess::listSFX", WRAP_METHOD(Debugger, cmdListSFX));
	registerCmd("AudioProcess::playSFX", WRAP_METHOD(Debugger, cmdPlaySFX));
	registerCmd("AudioProcess::stopSFX", WRAP_METHOD(Debugger, cmdStopSFX));

	registerCmd("GameMapGump::toggleHighlightItems", WRAP_METHOD(Debugger, cmdToggleHighlightItems));
	registerCmd("GameMapGump::dumpMap", WRAP_METHOD(Debugger, cmdDumpMap));
	registerCmd("GameMapGump::incrementSortOrder", WRAP_METHOD(Debugger, cmdIncrementSortOrder));
	registerCmd("GameMapGump::decrementSortOrder", WRAP_METHOD(Debugger, cmdDecrementSortOrder));

	registerCmd("HIDManager::bind", WRAP_METHOD(Debugger, cmdBind));
	registerCmd("HIDManager::unbind", WRAP_METHOD(Debugger, cmdUnbind));
	registerCmd("HIDManager::listbinds", WRAP_METHOD(Debugger, cmdListbinds));
	registerCmd("HIDManager::save", WRAP_METHOD(Debugger, cmdSave));

	registerCmd("Kernel::processTypes", WRAP_METHOD(Debugger, cmdProcessTypes));
	registerCmd("Kernel::processInfo", WRAP_METHOD(Debugger, cmdProcessInfo));
	registerCmd("Kernel::listProcesses", WRAP_METHOD(Debugger, cmdListProcesses));
	registerCmd("Kernel::toggleFrameByFrame", WRAP_METHOD(Debugger, cmdToggleFrameByFrame));
	registerCmd("Kernel::advanceFrame", WRAP_METHOD(Debugger, cmdAdvanceFrame));

	registerCmd("MemoryManager::MemInfo", WRAP_METHOD(Debugger, cmdMemInfo));
#ifdef DEBUG
	registerCmd("MemoryManager::test", WRAP_METHOD(Debugger, cmdTest));
#endif

	registerCmd("ObjectManager::objectTypes", WRAP_METHOD(Debugger, cmdObjectTypes));
	registerCmd("ObjectManager::objectInfo", WRAP_METHOD(Debugger, cmdObjectInfo));

	registerCmd("QuickAvatarMoverProcess::startMoveUp", WRAP_METHOD(Debugger, cmdStartMoveUp));
	registerCmd("QuickAvatarMoverProcess::startMoveDown", WRAP_METHOD(Debugger, cmdStartMoveDown));
	registerCmd("QuickAvatarMoverProcess::startMoveLeft", WRAP_METHOD(Debugger, cmdStartMoveLeft));
	registerCmd("QuickAvatarMoverProcess::startMoveRight", WRAP_METHOD(Debugger, cmdStartMoveRight));
	registerCmd("QuickAvatarMoverProcess::startAscend", WRAP_METHOD(Debugger, cmdStartAscend));
	registerCmd("QuickAvatarMoverProcess::startDescend", WRAP_METHOD(Debugger, cmdStartDescend));
	registerCmd("QuickAvatarMoverProcess::stopMoveUp", WRAP_METHOD(Debugger, cmdStopMoveUp));
	registerCmd("QuickAvatarMoverProcess::stopMoveDown", WRAP_METHOD(Debugger, cmdStopMoveDown));
	registerCmd("QuickAvatarMoverProcess::stopMoveLeft", WRAP_METHOD(Debugger, cmdStopMoveLeft));
	registerCmd("QuickAvatarMoverProcess::stopMoveRight", WRAP_METHOD(Debugger, cmdStopMoveRight));
	registerCmd("QuickAvatarMoverProcess::stopAscend", WRAP_METHOD(Debugger, cmdStopAscend));
	registerCmd("QuickAvatarMoverProcess::stopDescend", WRAP_METHOD(Debugger, cmdStopDescend));
	registerCmd("QuickAvatarMoverProcess::toggleQuarterSpeed", WRAP_METHOD(Debugger, cmdToggleQuarterSpeed));
	registerCmd("QuickAvatarMoverProcess::toggleClipping", WRAP_METHOD(Debugger, cmdToggleClipping));
}

bool Debugger::cmdSaveGame(int argc, const char **argv) {
	if (argc == 2) {
		// Save a _game with the given name into the quicksave slot
		Ultima8Engine::get_instance()->saveGame(1, argv[1]);
	} else {
		Ultima8Engine::get_instance()->saveGameDialog();
	}

	return false;
}

bool Debugger::cmdLoadGame(int argc, const char **argv) {
	if (argc == 2) {
		// Load a _game from the quicksave slot. The second parameter is ignored,
		// it just needs to be present to differentiate from showing the GUI load dialog
		Ultima8Engine::get_instance()->loadGameState(1);
	} else {
		Ultima8Engine::get_instance()->loadGameDialog();
	}

	return false;
}

bool Debugger::cmdNewGame(int argc, const char **argv) {
	Ultima8Engine::get_instance()->newGame();
	return false;
}

bool Debugger::cmdQuit(int argc, const char **argv) {
	Ultima8Engine::get_instance()->_isRunning = false;
	return true;
}

bool Debugger::cmdDrawRenderStats(int argc, const char **argv) {
	if (argc == 1) {
		debugPrintf("Ultima8Engine::_drawRenderStats = %s",
			strBool(Ultima8Engine::get_instance()->_drawRenderStats));
		return true;
	} else {
		Ultima8Engine::get_instance()->_drawRenderStats = Std::strtol(argv[1], 0, 0) != 0;
		return false;
	}
}

bool Debugger::cmdEngineStats(int argc, const char **argv) {
	Kernel::get_instance()->kernelStats();
	ObjectManager::get_instance()->objectStats();
	UCMachine::get_instance()->usecodeStats();
	World::get_instance()->worldStats();


	return true;
}

bool Debugger::cmdChangeGame(int argc, const char **argv) {
	if (argc == 1) {
		debugPrintf("Current _game is: %s\n", Ultima8Engine::get_instance()->_gameInfo->_name.c_str());
	} else {
		Ultima8Engine::get_instance()->changeGame(argv[1]);
	}

	return true;
}

bool Debugger::cmdListGames(int argc, const char **argv) {
	Ultima8Engine *app = Ultima8Engine::get_instance();
	Std::vector<istring> games;
	games = app->_settingMan->listGames();
	Std::vector<istring>::iterator iter;
	for (iter = games.begin(); iter != games.end(); ++iter) {
		istring _game = *iter;
		GameInfo *info = app->getGameInfo(_game);
		debugPrintf("%s: ", _game.c_str());
		if (info) {
			Std::string details = info->getPrintDetails();
			debugPrintf("%s\n", details.c_str());
		} else {
			debugPrintf("(unknown)\n");
		}
	}

	return true;
}

bool Debugger::cmdSetVideoMode(int argc, const char **argv) {
	if (argc != 3) {
		debugPrintf("Usage: Ultima8Engine::setVidMode width height\n");
		return true;
	} else {
		Ultima8Engine::get_instance()->changeVideoMode(strtol(argv[1], 0, 0), strtol(argv[2], 0, 0));
		return false;
	}
}

bool Debugger::cmdToggleAvatarInStasis(int argc, const char **argv) {
	Ultima8Engine *g = Ultima8Engine::get_instance();
	g->toggleAvatarInStasis();
	debugPrintf("_avatarInStasis = %s\n", strBool(g->isAvatarInStasis()));
	return true;
}

bool Debugger::cmdTogglePaintEditorItems(int argc, const char **argv) {
	Ultima8Engine *g = Ultima8Engine::get_instance();
	g->togglePaintEditorItems();
	debugPrintf("_paintEditorItems = %s\n", strBool(g->isPaintEditorItems()));
	return true;
}

bool Debugger::cmdToggleShowTouchingItems(int argc, const char **argv) {
	Ultima8Engine *g = Ultima8Engine::get_instance();
	g->toggleShowTouchingItems();
	debugPrintf("ShowTouchingItems = %s\n", strBool(g->isShowTouchingItems()));
	return true;
}

bool Debugger::cmdCloseItemGumps(int argc, const char **argv) {
	Ultima8Engine *g = Ultima8Engine::get_instance();
	g->getDesktopGump()->CloseItemDependents();
	return true;
}

bool Debugger::cmdToggleCheatMode(int argc, const char **argv) {
	Ultima8Engine *g = Ultima8Engine::get_instance();
	g->setCheatMode(!g->areCheatsEnabled());
	debugPrintf("Cheats = %s\n", strBool(g->areCheatsEnabled()));
	return true;
}

bool Debugger::cmdMemberVar(int argc, const char **argv) {
	if (argc == 1) {
		debugPrintf("Usage: Ultima8Engine::memberVar <member> [newvalue] [updateini]\n");
		return true;
	}

	Ultima8Engine *g = Ultima8Engine::get_instance();

	// Set the pointer to the correct type
	bool *b = 0;
	int *i = 0;
	Std::string *str = 0;
	istring *istr = 0;

	// ini entry name if supported
	const char *ini = 0;

	if (!scumm_stricmp(argv[1], "_frameLimit")) {
		b = &g->_frameLimit;
		ini = "_frameLimit";
	} else if (!scumm_stricmp(argv[1], "_frameSkip")) {
		b = &g->_frameSkip;
		ini = "_frameSkip";
	} else if (!scumm_stricmp(argv[1], "_interpolate")) {
		b = &g->_interpolate;
		ini = "_interpolate";
	} else {
		debugPrintf("Unknown member: %s\n", argv[1]);
		return true;
	}

	// Set the value
	if (argc >= 3) {
		if (b)
			*b = !scumm_stricmp(argv[2], "yes") || !scumm_stricmp(argv[2], "true");
		else if (istr)
			*istr = argv[2];
		else if (i)
			*i = Std::strtol(argv[2], 0, 0);
		else if (str)
			*str = argv[2];

		// Set config value
		if (argc >= 4 && ini && *ini && (!scumm_stricmp(argv[3], "yes") || !scumm_stricmp(argv[3], "true"))) {
			if (b)
				g->_settingMan->set(ini, *b);
			else if (istr)
				g->_settingMan->set(ini, *istr);
			else if (i)
				g->_settingMan->set(ini, *i);
			else if (str)
				g->_settingMan->set(ini, *str);
		}
	}

	// Print the value
	debugPrintf("Ultima8Engine::%s = ", argv[1]);
	if (b)
		debugPrintf("%s", strBool(*b));
	else if (istr)
		debugPrintf("%s", istr->c_str());
	else if (i)
		debugPrintf("%d", *i);
	else if (str)
		debugPrintf("%s", str->c_str());
	debugPrintf("\n");

	return true;
}


bool Debugger::cmdListSFX(int argc, const char **argv) {
	AudioProcess *ap = AudioProcess::get_instance();
	if (!ap) {
		debugPrintf("Error: No AudioProcess\n");

	} else {
		Std::list<AudioProcess::SampleInfo>::iterator it;
		for (it = ap->_sampleInfo.begin(); it != ap->_sampleInfo.end(); ++it) {
			debugPrintf("Sample: num %d, obj %d, loop %d, prio %d",
				it->_sfxNum, it->_objId, it->_loops, it->_priority);
			if (!it->_barked.empty()) {
				debugPrintf(", speech: \"%s\"",
					it->_barked.substr(it->_curSpeechStart, it->_curSpeechEnd - it->_curSpeechStart).c_str());
			}
			debugPrintf("\n");
		}
	}

	return true;
}

bool Debugger::cmdStopSFX(int argc, const char **argv) {
	AudioProcess *ap = AudioProcess::get_instance();
	if (!ap) {
		debugPrintf("Error: No AudioProcess\n");
		return true;
	} else if (argc < 2) {
		debugPrintf("usage: stopSFX <_sfxNum> [<_objId>]\n");
		return true;
	} else {
		int _sfxNum = static_cast<int>(strtol(argv[1], 0, 0));
		ObjId _objId = (argc >= 3) ? static_cast<ObjId>(strtol(argv[2], 0, 0)) : 0;

		ap->stopSFX(_sfxNum, _objId);
		return false;
	}
}

bool Debugger::cmdPlaySFX(int argc, const char **argv) {
	AudioProcess *ap = AudioProcess::get_instance();
	if (!ap) {
		debugPrintf("Error: No AudioProcess\n");
		return true;
	} else if (argc < 2) {
		debugPrintf("usage: playSFX <_sfxNum>\n");
		return true;
	} else {
		int _sfxNum = static_cast<int>(strtol(argv[1], 0, 0));
		ap->playSFX(_sfxNum, 0x60, 0, 0);
		return false;
	}
}


bool Debugger::cmdToggleHighlightItems(int argc, const char **argv) {
	GameMapGump::Set_highlightItems(!GameMapGump::is_highlightItems());
	return false;
}

bool Debugger::cmdDumpMap(int argc, const char **argv) {
#ifdef TODO
	// We only support 32 bits per pixel for now
	if (RenderSurface::format.s_bpp != 32) return;

	// Save because we're going to potentially break the game by enlarging
	// the fast area and available object IDs.
	Std::string savefile = "@save/dumpmap";
	Ultima8Engine::get_instance()->saveGame(savefile, "Pre-dumpMap save");

	// Increase number of available object IDs.
	ObjectManager::get_instance()->allow64kObjects();

	// Actual size
	int32 awidth = 8192;
	int32 aheight = 8192;

	int32 xpos = 0;
	int32 ypos = 0;

	int32 left = 16384;
	int32 right = -16384;
	int32 top = 16384;
	int32 bot = -16384;

	int32 camheight = 256;

	// Work out the map limit we do this very coarsly
	// Now render the map
	for (int32 y = 0; y < 64; y++) {
		for (int32 x = 0; x < 64; x++) {
			const Std::list<Item *> *list =
				World::get_instance()->getCurrentMap()->getItemList(x, y);

			// Should iterate the items!
			// (items could extend outside of this chunk and they have height)
			if (list && list->size() != 0) {
				int32 l = (x * 512 - y * 512) / 4 - 128;
				int32 r = (x * 512 - y * 512) / 4 + 128;
				int32 t = (x * 512 + y * 512) / 8 - 256;
				int32 b = (x * 512 + y * 512) / 8;

				t -= 256; // approx. adjustment for height of items in chunk

				if (l < left) left = l;
				if (r > right) right = r;
				if (t < top) top = t;
				if (b > bot) bot = b;
			}
		}
	}

	if (right == -16384) return;

	// camera height
	bot += camheight;
	top += camheight;

	awidth = right - left;
	aheight = bot - top;

	ypos = top;
	xpos = left;

	// Buffer Size
	int32 bwidth = awidth;
	int32 bheight = 256;

	// Tile size
	int32 twidth = bwidth / 8;
	int32 theight = bheight;


	Debugger *g = new Debugger(0, 0, twidth, theight);

	// HACK: Setting both INVISIBLE and TRANSPARENT flags on the Avatar
	// will make him completely invisible.
	getMainActor()->setFlag(Item::FLG_INVISIBLE);
	getMainActor()->setExtFlag(Item::EXT_TRANSPARENT);
	World::get_instance()->getCurrentMap()->setWholeMapFast();

	RenderSurface *s = RenderSurface::CreateSecondaryRenderSurface(bwidth,
		bheight);
	Texture *t = s->GetSurfaceAsTexture();
	// clear buffer
	Std::memset(t->buffer, 0, 4 * bwidth * bheight);


	// Write tga header
	Std::string filename = "@home/mapdump";
	char buf[32];
	sprintf(buf, "%02d", World::get_instance()->getCurrentMap()->getNum());
	filename += buf;
	filename += ".png";
	ODataSource *ds = FileSystem::get_instance()->WriteFile(filename);
	Std::string pngcomment = "Map ";
	pngcomment += buf;
	pngcomment += ", dumped by Pentagram.";

	PNGWriter *pngw = new PNGWriter(ds);
	pngw->init(awidth, aheight, pngcomment);

	// Now render the map
	for (int32 y = 0; y < aheight; y += theight) {
		for (int32 x = 0; x < awidth; x += twidth) {
			// Work out 'effective' and world coords
			int32 ex = xpos + x + twidth / 2;
			int32 ey = ypos + y + theight / 2;
			int32 wx = ex * 2 + ey * 4;
			int32 wy = ey * 4 - ex * 2;

			s->SetOrigin(x, y % bheight);
			CameraProcess::SetCameraProcess(
				new CameraProcess(wx + 4 * camheight, wy + 4 * camheight, camheight));
			g->Paint(s, 256, false);

		}

		// Write out the current buffer
		if (((y + theight) % bheight) == 0) {
			for (int i = 0; i < bwidth * bheight; ++i) {
				// Convert to correct pixel format
				uint8 r, g, b;
				UNPACK_RGB8(t->buffer[i], r, g, b);
				uint8 *buf = reinterpret_cast<uint8 *>(&t->buffer[i]);
				buf[0] = b;
				buf[1] = g;
				buf[2] = r;
				buf[3] = 0xFF;
			}

			pngw->writeRows(bheight, t);

			// clear buffer for next set
			Std::memset(t->buffer, 0, 4 * bwidth * bheight);
		}
	}

	pngw->finish();
	delete pngw;

	delete ds;

	delete g;
	delete s;

	// Reload
	Ultima8Engine::get_instance()->loadGame(savefile);

	debugPrintf("Map dumped\n");
#endif
	return false;
}

bool Debugger::cmdIncrementSortOrder(int argc, const char **argv) {
	GameMapGump *gump = Ultima8Engine::get_instance()->getGameMapGump();
	if (gump)
		gump->IncSortOrder(1);
	return false;
}

bool Debugger::cmdDecrementSortOrder(int argc, const char **argv) {
	GameMapGump *gump = Ultima8Engine::get_instance()->getGameMapGump();
	if (gump)
		gump->IncSortOrder(-1);
	return false;
}


bool Debugger::cmdBind(int argc, const char **argv) {
	Console::ArgvType argv2;
	Console::ArgvType::const_iterator it;
	if (argc < 3) {
		debugPrintf("Usage: %s <key> <action> [<arg> ...]: binds a key or button to an action\n",
			argv[0]);
		return true;
	} else {
		HIDManager *hid = HIDManager::get_instance();

		istring control(argv[1]);
		Common::Array<istring> args;
		for (int i = 2; i < argc; ++i)
			args.push_back(argv[i]);

		it = args.begin();
		++it;
		++it;
		argv2.assign(it, args.end());

		hid->bind(control, argv2);
		return false;
	}
}

bool Debugger::cmdUnbind(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Usage: %s <key>: unbinds a key or button\n", argv[0]);
		return true;
	} else {
		HIDManager *hid = HIDManager::get_instance();

		istring control(argv[1]);

		hid->unbind(control);
		return false;
	}
}

bool Debugger::cmdListbinds(int argc, const char **argv) {
	HIDManager *hid = HIDManager::get_instance();
	hid->listBindings();
	return true;
}

bool Debugger::cmdSave(int argc, const char **argv) {
	HIDManager *hid = HIDManager::get_instance();
	hid->saveBindings();

	SettingManager *settings = SettingManager::get_instance();
	settings->write();
	return false;
}



bool Debugger::cmdProcessTypes(int argc, const char **argv) {
	Kernel::get_instance()->processTypes();
	return false;
}

bool Debugger::cmdListProcesses(int argc, const char **argv) {
	if (argc > 2) {
		debugPrintf("usage: listProcesses [<itemnum>]\n");
	} else {
		Kernel *kern = Kernel::get_instance();
		ObjId item = 0;
		if (argc == 2) {
			item = static_cast<ObjId>(strtol(argv[1], 0, 0));
			debugPrintf("Processes for item %d:\n", item);
		} else {
			debugPrintf("Processes:\n");
		}
		for (ProcessIterator it = kern->processes.begin();
			it != kern->processes.end(); ++it) {
			Process *p = *it;
			if (argc == 1 || p->_itemNum == item)
				p->dumpInfo();
		}
	}

	return true;
}

bool Debugger::cmdProcessInfo(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("usage: processInfo <objectnum>\n");
	} else {
		Kernel *kern = Kernel::get_instance();

		ProcId procid = static_cast<ProcId>(strtol(argv[1], 0, 0));

		Process *p = kern->getProcess(procid);
		if (p == 0) {
			debugPrintf("No such process: %d\n", procid);
		} else {
			p->dumpInfo();
		}
	}

	return true;
}

bool Debugger::cmdToggleFrameByFrame(int argc, const char **argv) {
	Kernel *kern = Kernel::get_instance();
	bool fbf = !kern->isFrameByFrame();
	kern->setFrameByFrame(fbf);
	debugPrintf("FrameByFrame = %s\n", fbf ? "true" : "false");

	if (fbf)
		kern->pause();
	else
		kern->unpause();

	return true;
}

bool Debugger::cmdAdvanceFrame(int argc, const char **argv) {
	Kernel *kern = Kernel::get_instance();
	if (kern->isFrameByFrame()) {
		kern->unpause();
		debugPrintf("FrameByFrame: Next Frame\n");
	}

	return true;
}


bool Debugger::cmdMemInfo(int argc, const char **argv) {
	MemoryManager *mm = MemoryManager::get_instance();
	int i, count;

	if (mm) {
		count = mm->getAllocatorCount();
		debugPrintf("Allocators: %d\n", count);
		for (i = 0; i < count; ++i) {
			debugPrintf(" Allocator %d:\n", i);
			mm->getAllocator(i)->printInfo();
			debugPrintf("==============\n");
		}
	}

	return true;
}

#ifdef DEBUG
bool Debugger::cmdTest(int argc, const char **argv) {
	return true;
}
#endif

bool Debugger::cmdObjectTypes(int argc, const char **argv) {
	ObjectManager::get_instance()->objectTypes();
	return true;
}

bool Debugger::cmdObjectInfo(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("usage: objectInfo <objectnum>\n");
	} else {
		ObjectManager *objMan = ObjectManager::get_instance();

		ObjId objid = static_cast<ObjId>(strtol(argv[1], 0, 0));

		Object *obj = objMan->getObject(objid);
		if (obj == 0) {
			bool reserved = false;
			if (objid >= 256) // CONSTANT!
				reserved = objMan->_objIDs->isIDUsed(objid);
			else
				reserved = objMan->_actorIDs->isIDUsed(objid);
			if (reserved)
				debugPrintf("Reserved objid: %d\n", objid);
			else
				debugPrintf("No such object: %d\n", objid);
		} else {
			obj->dumpInfo();
		}
	}

	return true;
}


bool Debugger::cmdStartMoveUp(int argc, const char **argv) {
	if (Ultima8Engine::get_instance()->areCheatsEnabled()) {
		QuickAvatarMoverProcess::startMover(-64, -64, 0, 0);
		return false;
	} else {
		debugPrintf("Cheats aren't enabled\n");
		return true;
	}
}

bool Debugger::cmdStartMoveDown(int argc, const char **argv) {
	if (Ultima8Engine::get_instance()->areCheatsEnabled()) {
		QuickAvatarMoverProcess::startMover(+64, +64, 0, 1);
		return false;
	} else {
		debugPrintf("Cheats aren't enabled\n");
		return true;
	}
}

bool Debugger::cmdStartMoveLeft(int argc, const char **argv) {
	if (Ultima8Engine::get_instance()->areCheatsEnabled()) {
		QuickAvatarMoverProcess::startMover(-64, +64, 0, 2);
		return false;
	} else {
		debugPrintf("Cheats aren't enabled\n");
		return true;
	}
}

bool Debugger::cmdStartMoveRight(int argc, const char **argv) {
	if (Ultima8Engine::get_instance()->areCheatsEnabled()) {
		QuickAvatarMoverProcess::startMover(+64, -64, 0, 3);
		return false;
	} else {
		debugPrintf("Cheats aren't enabled\n");
		return true;
	}
}

bool Debugger::cmdStartAscend(int argc, const char **argv) {
	if (Ultima8Engine::get_instance()->areCheatsEnabled()) {
		QuickAvatarMoverProcess::startMover(0, 0, 8, 4);
		return false;
	} else {
		debugPrintf("Cheats aren't enabled\n");
		return true;
	}
}

bool Debugger::cmdStartDescend(int argc, const char **argv) {
	if (Ultima8Engine::get_instance()->areCheatsEnabled()) {
		QuickAvatarMoverProcess::startMover(0, 0, -8, 5);
		return false;
	} else {
		debugPrintf("Cheats aren't enabled\n");
		return true;
	}
}

bool Debugger::cmdStopMoveUp(int argc, const char **argv) {
	QuickAvatarMoverProcess::terminateMover(0);
	return false;
}

bool Debugger::cmdStopMoveDown(int argc, const char **argv) {
	QuickAvatarMoverProcess::terminateMover(1);
	return false;
}

bool Debugger::cmdStopMoveLeft(int argc, const char **argv) {
	QuickAvatarMoverProcess::terminateMover(2);
	return false;
}

bool Debugger::cmdStopMoveRight(int argc, const char **argv) {
	QuickAvatarMoverProcess::terminateMover(3);
	return false;
}

bool Debugger::cmdStopAscend(int argc, const char **argv) {
	QuickAvatarMoverProcess::terminateMover(4);
	return false;
}

bool Debugger::cmdStopDescend(int argc, const char **argv) {
	QuickAvatarMoverProcess::terminateMover(5);
	return false;
}

bool Debugger::cmdToggleQuarterSpeed(int argc, const char **argv) {
	QuickAvatarMoverProcess::setQuarterSpeed(!QuickAvatarMoverProcess::isQuarterSpeed());
	return false;
}

bool Debugger::cmdToggleClipping(int argc, const char **argv) {
	if (Ultima8Engine::get_instance()->areCheatsEnabled()) {
		QuickAvatarMoverProcess::toggleClipping();
		debugPrintf("QuickAvatarMoverProcess::_clipping = %s\n",
			QuickAvatarMoverProcess::isClipping() ? "true" : "false");
	} else {
		debugPrintf("Cheats aren't enabled\n");
	}
	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
