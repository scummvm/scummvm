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
 */

#include "common/textconsole.h"

#include "m4/m4.h"
#include "m4/console.h"
#include "m4/dialogs.h"
#include "m4/scene.h"
#include "m4/staticres.h"

namespace M4 {

Console::Console(MadsM4Engine *vm) : GUI::Debugger() {
	_vm = vm;

	DCmd_Register("scene",			WRAP_METHOD(Console, cmdLoadScene));
	DCmd_Register("start",			WRAP_METHOD(Console, cmdStartingScene));
	DCmd_Register("show_hotspots",	WRAP_METHOD(Console, cmdShowHotSpots));
	DCmd_Register("list_hotspots",	WRAP_METHOD(Console, cmdListHotSpots));
	DCmd_Register("play_sound",		WRAP_METHOD(Console, cmdPlaySound));
	DCmd_Register("play_dsr_sound",	WRAP_METHOD(Console, cmdPlayDSRSound));
	DCmd_Register("show_resources",	WRAP_METHOD(Console, cmdShowResources));
	DCmd_Register("show_codes",		WRAP_METHOD(Console, cmdShowCodes));
	DCmd_Register("dump_file",		WRAP_METHOD(Console, cmdDumpFile));
	DCmd_Register("sprite",			WRAP_METHOD(Console, cmdShowSprite));
	DCmd_Register("start_conv",		WRAP_METHOD(Console, cmdStartConversation));
	DCmd_Register("textview",		WRAP_METHOD(Console, cmdShowTextview));
	DCmd_Register("animview",		WRAP_METHOD(Console, cmdShowAnimview));
}

Console::~Console() {
}

static int strToInt(const char *s) {
	if (!*s)
		// No string at all
		return 0;
	else if (toupper(s[strlen(s) - 1]) != 'H')
		// Standard decimal string
		return atoi(s);

	// Hexadecimal string
	uint tmp = 0;
	int read = sscanf(s, "%xh", &tmp);
	if (read < 1)
		error("strToInt failed on string \"%s\"", s);
	return (int)tmp;
}

bool Console::cmdLoadScene(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Usage: %s <scene number>\n", argv[0]);
		return true;
	} else {
		if (_vm->isM4())
			_vm->_kernel->newRoom = atoi(argv[1]);
		else
			_vm->_scene->loadScene(atoi(argv[1]));
		return false;
	}
}

bool Console::cmdStartingScene(int argc, const char **argv) {
	if (_vm->getGameType() != GType_Riddle) {
		if (_vm->isM4())
			_vm->_kernel->newRoom = FIRST_SCENE;
		else
			_vm->_scene->loadScene(FIRST_SCENE);
		return false;
	} else {
		DebugPrintf("%s: Riddle of Master Lu is not supported", argv[0]);
		return true;
	}
}

bool Console::cmdShowHotSpots(int argc, const char **argv) {
	_vm->_scene->showHotSpots();
	return false;
}

bool Console::cmdListHotSpots(int argc, const char **argv) {
	DebugPrintf("Scene hotspots\n");
	_vm->_scene->getSceneResources().hotspots->dump();
	if (_vm->isM4()) {
		DebugPrintf("Scene parallax\n");
		_m4Vm->scene()->getSceneResources().parallax->dump();
		DebugPrintf("Scene dynamic hotspots\n");
		_vm->_scene->getSceneResources().dynamicHotspots->dump();
	}
	return true;
}

bool Console::cmdPlaySound(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Usage: %s <sound file>\n", argv[0]);
	} else {
		_vm->_sound->playSound(argv[1], 255, false);
	}
	return true;
}

bool Console::cmdPlayDSRSound(int argc, const char **argv) {
	if (argc != 2 && argc != 3) {
		DebugPrintf("Usage: %s <sound index> <DSR file>\n", argv[0]);
		DebugPrintf("The DSR file parameter is optional, and specifies which DSR to load\n");
	} else {
		if (argc == 3)
			_vm->_sound->loadDSRFile(argv[2]);
		_vm->_sound->playDSRSound(atoi(argv[1]), 255, false);
	}
	return true;
}

bool Console::cmdShowResources(int argc, const char **argv) {
	_vm->res()->dump();
	return true;
}

bool Console::cmdShowCodes(int argc, const char **argv) {
	_vm->_scene->showCodes();
	return false;
}

bool Console::cmdDumpFile(int argc, const char **argv) {
	if (argc != 2 && argc != 3) {
		DebugPrintf("Usage: %s <file> <uncompress>\n", argv[0]);
		DebugPrintf("If uncompress is 1, the file is uncompressed (for MADS games)\n");
	} else {
		if (argc == 2) {
			_vm->dumpFile(argv[1], false);
		} else {
			if (argc == 3 && atoi(argv[2]) == 1)
				_vm->dumpFile(argv[1], true);
			else
				_vm->dumpFile(argv[1], false);
		}
	}
	return true;
}

bool Console::cmdShowSprite(int argc, const char **argv) {
	View *view = _vm->_viewManager->getView(VIEWID_SCENE);
	if (view == NULL)
		DebugPrintf("The scene view isn't currently active\n");
	else if (argc < 2)
		DebugPrintf("Usage: %s resource_name\n", argv[0]);
	else {
		char resourceName[20];
		strncpy(resourceName, argv[1], 15);
		resourceName[15] = '\0';
		if (!strchr(resourceName, '.'))
			strcat(resourceName, ".SS");

		_vm->_viewManager->moveToFront(view);
		Common::SeekableReadStream *data = _vm->res()->get(resourceName);
		SpriteAsset *asset = new SpriteAsset(_vm, data, data->size(), resourceName);
		_vm->res()->toss(resourceName);

		RGBList *palData = new RGBList(asset->getColorCount(), asset->getPalette(), true);
		_vm->_palette->addRange(palData);

		// Get the scene background surface
		M4Surface *bg = _vm->_scene->getBackgroundSurface();

		// Write the sprite onto the screen
		int x = 0, y = 0, yMax = 0;
		for (int index = 0; index < asset->getCount(); index++) {
			M4Sprite *spr = asset->getFrame(index);
			spr->translate(palData);		// sprite pixel translation

			if ((x + spr->width() >= bg->width()) && (yMax != 0)) {
				x = 0;
				y += yMax;
				yMax = 0;
			}

			if (y >= bg->height())
				break;

			spr->copyTo(bg, x, y, (int)spr->getTransparencyIndex());

			x += spr->width();
			yMax = MAX(yMax, spr->height());
		}

		view->restore(0, 0, view->width(), view->height());
		return false;
	}

	return true;
}

bool Console::cmdStartConversation(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Usage: %s <conversation file name>\n", argv[0]);
		return true;
	} else if (_vm->isM4()) {
		((M4Engine *)_vm)->_converse->startConversation(argv[1]);
		return false;
	} else {
		error("MADS engine does not support conversations yet");
		return false;
	}
}

bool Console::cmdShowTextview(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Usage: %s <txr resource>\n", argv[0]);
		return true;
	}

	_vm->_viewManager->showTextView(argv[1], false);
	return false;
}

bool Console::cmdShowAnimview(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Usage: %s <res resource>\n", argv[0]);
		return true;
	}

	char resName[80];
	strcpy(resName, "@");
	strcat(resName, *argv[1] == '@' ? argv[1] + 1 : argv[1]);

	_vm->_viewManager->showAnimView(resName, false);
	return false;
}

/*--------------------------------------------------------------------------*/

MadsConsole::MadsConsole(MadsEngine *vm): Console(vm) {
	_vm = vm;

	DCmd_Register("object",			WRAP_METHOD(MadsConsole, cmdObject));
	DCmd_Register("message",		WRAP_METHOD(MadsConsole, cmdMessage));
	DCmd_Register("scene_info",		WRAP_METHOD(MadsConsole, cmdSceneInfo));
	DCmd_Register("anim",			WRAP_METHOD(MadsConsole, cmdPlayAnimation));
}

bool MadsConsole::cmdObject(int argc, const char **argv) {
	if (argc == 1) {
		DebugPrintf("Usage: object ['list' | '#objnum' | 'add #objnum']\n");
	} else if (!strcmp(argv[1], "list")) {
		// List of objects
		for (uint objStart = 0; objStart < _vm->globals()->getObjectsSize(); objStart += 5) {
			DebugPrintf("%2d - ", objStart);
			for (uint objId = objStart; objId < MIN<uint>(_vm->globals()->getObjectsSize(), objStart + 5); ++objId) {
				if (objId != objStart) DebugPrintf(", ");
				uint16 descId = _vm->globals()->getObject(objId)->_descId;
				DebugPrintf("%s", _vm->globals()->getVocab(descId));
			}

			DebugPrintf("\n");
		}

		DebugPrintf("\n");
	} else if (!strcmp(argv[1], "add") && (argc == 3)) {
		// Add the specified object to the player's inventory
		int objNum = strToInt(argv[2]);

		if ((objNum < 0) || (objNum >= (int)_vm->globals()->getObjectsSize()))
			DebugPrintf("Invalid object specified\n");
		else if (_vm->isM4())
			DebugPrintf("Not implemented for M4 games\n");
		else {
			_vm->_scene->getInterface()->addObjectToInventory(objNum);
			return false;
		}

	} else {
		// Print the details of a specific object
		int objNum = strToInt(argv[1]);

		if ((objNum < 0) || (objNum >= (int)_vm->globals()->getObjectsSize()))
			DebugPrintf("Invalid object specified\n");
		else {
			const MadsObject *obj = _vm->globals()->getObject(objNum);

			DebugPrintf("Object #%d (%s) room=%d article=%d/%s vocabs=%d", objNum, _vm->globals()->getVocab(obj->_descId),
				obj->_roomNumber, (int)obj->_article, englishMADSArticleList[obj->_article], obj->_vocabCount);

			if (obj->_vocabCount > 0) {
				DebugPrintf(" - ");
				for (int i = 0; i < obj->_vocabCount; ++i) {
					if (i != 0) DebugPrintf(", ");
					DebugPrintf("%s (%d)/%d,%d", _vm->globals()->getVocab(obj->_vocabList[i].vocabId),
						obj->_vocabList[i].vocabId, obj->_vocabList[i].flags1, obj->_vocabList[i].flags2);
				}
			}
			DebugPrintf("\n");
		}
	}

	return true;
}

bool MadsConsole::cmdMessage(int argc, const char **argv) {
	if (argc == 1) {
		DebugPrintf("message 'objnum'\n");
	} else if (!strcmp(argv[1], "list_quotes")) {
		// Dump the quotes list
#if 0
		// FIXME: The following code is not portable and hence has been disabled.
		// Try replacing FILE by Common::DumpFile.
		FILE *destFile = fopen("mads_quotes.txt", "wb");
		for (uint i = 0; i < _vm->globals()->getQuotesSize(); ++i)
			fprintf(destFile, "%.3d - %s\n", i, _vm->globals()->getQuote(i));
		fclose(destFile);
#endif

	} else if (!strcmp(argv[1], "list_vocab")) {
		// Dump the vocab list
#if 0
		// FIXME: The following code is not portable and hence has been disabled.
		// Try replacing FILE by Common::DumpFile.
		FILE *destFile = fopen("mads_vocab.txt", "wb");
		for (uint i = 1; i <= _vm->globals()->getVocabSize(); ++i)
			fprintf(destFile, "%.3d/%.3x - %s\n", i, i, _vm->globals()->getVocab(i));
		fclose(destFile);
#endif

	} else {
		int messageIdx = strToInt(argv[1]);

		if ((argc != 3) || (strcmp(argv[2], "idx") != 0))
			messageIdx = _vm->globals()->messageIndexOf(messageIdx);

		const char *msg = _vm->globals()->loadMessage(messageIdx);
		if (!msg)
			DebugPrintf("Unknown message\n");
		else {
			Dialog *dlg = new Dialog(_vm, msg, "TEST DIALOG");

			_vm->_viewManager->addView(dlg);
			_vm->_viewManager->moveToFront(dlg);

			return false;
		}
	}

	return true;
}

bool MadsConsole::cmdSceneInfo(int argc, const char **argv) {
	DebugPrintf("Current scene is: %i\n", _vm->_scene->getCurrentScene());

	return true;
}

bool MadsConsole::cmdPlayAnimation(int argc, const char **argv) {
	View *view = _vm->_viewManager->getView(VIEWID_SCENE);
	if (view == NULL) {
		DebugPrintf("The scene view isn't currently active\n");
	} else if (argc != 2 && argc != 3) {
		DebugPrintf("Usage: %s <anim resource (*.aa)> <fullscreen>\n", argv[0]);
		DebugPrintf("If fullscreen is 1, the screen palette is replaced with the palette of the animation\n");
	} else {
		char resourceName[20];
		strncpy(resourceName, argv[1], 15);
		resourceName[15] = '\0';
		if (!strchr(resourceName, '.'))
			strcat(resourceName, ".AA");

		_vm->_viewManager->moveToFront(view);
		if (argc == 3 && atoi(argv[2]) == 1)
			_madsVm->_palette->deleteAllRanges();

		_madsVm->scene()->_sceneAnimation->load(resourceName, 0);

		view->restore(0, 0, view->width(), view->height());
		return false;
	}

	return true;
}

/*--------------------------------------------------------------------------*/

M4Console::M4Console(M4Engine *vm): Console(vm) {
	_vm = vm;

	DCmd_Register("scene_info",		WRAP_METHOD(M4Console, cmdSceneInfo));
}

bool M4Console::cmdSceneInfo(int argc, const char **argv) {
	DebugPrintf("Current scene is: %i\n", _m4Vm->scene()->getCurrentScene());

	DebugPrintf("Scene resources:\n");
	DebugPrintf("artBase: %s\n", _m4Vm->scene()->getSceneResources().artBase);
	DebugPrintf("pictureBase: %s\n", _m4Vm->scene()->getSceneResources().pictureBase);
	DebugPrintf("hotspotCount: %i\n", _m4Vm->scene()->getSceneResources().hotspots->size());
	DebugPrintf("parallaxCount: %i\n", _m4Vm->scene()->getSceneResources().parallaxCount);
	DebugPrintf("dynHotspotCount: %i\n", _m4Vm->scene()->getSceneResources().dynamicHotspots->size());
	DebugPrintf("frontY: %i\n", _m4Vm->scene()->getSceneResources().frontY);
	DebugPrintf("backY: %i\n", _m4Vm->scene()->getSceneResources().backY);
	DebugPrintf("frontScale: %i\n", _m4Vm->scene()->getSceneResources().frontScale);
	DebugPrintf("backScale: %i\n", _m4Vm->scene()->getSceneResources().backScale);
	DebugPrintf("depthTable: ");
	for (uint i = 0; i < 16; i++)
		DebugPrintf("%i ", _m4Vm->scene()->getSceneResources().depthTable[i]);
	DebugPrintf("\n");
	DebugPrintf("railNodeCount: %i\n", _m4Vm->scene()->getSceneResources().railNodeCount);

	return true;
}

} // End of namespace M4
