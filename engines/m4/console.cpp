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

#include "m4/m4.h"
#include "m4/console.h"
#include "m4/scene.h"
#include "m4/staticres.h"

namespace M4 {

Console::Console(M4Engine *vm) : GUI::Debugger() {
	_vm = vm;

	DCmd_Register("scene",			WRAP_METHOD(Console, cmdLoadScene));
	DCmd_Register("start",			WRAP_METHOD(Console, cmdStartingScene));
	DCmd_Register("scene_info",		WRAP_METHOD(Console, cmdSceneInfo));
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
	DCmd_Register("anim",			WRAP_METHOD(Console, cmdPlayAnimation));
	DCmd_Register("object",			WRAP_METHOD(Console, cmdObject));
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
	uint tmp;
	sscanf(s, "%xh", &tmp);
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

bool Console::cmdSceneInfo(int argc, const char **argv) {
	DebugPrintf("Current scene is: %i\n", _vm->_scene->getCurrentScene());
	if (_vm->isM4()) {
		DebugPrintf("Scene resources:\n");
		DebugPrintf("artBase: %s\n", _vm->_scene->getSceneResources().artBase);
		DebugPrintf("pictureBase: %s\n", _vm->_scene->getSceneResources().pictureBase);
		DebugPrintf("hotspotCount: %i\n", _vm->_scene->getSceneResources().hotspotCount);
		DebugPrintf("parallaxCount: %i\n", _vm->_scene->getSceneResources().parallaxCount);
		DebugPrintf("propsCount: %i\n", _vm->_scene->getSceneResources().propsCount);
		DebugPrintf("frontY: %i\n", _vm->_scene->getSceneResources().frontY);
		DebugPrintf("backY: %i\n", _vm->_scene->getSceneResources().backY);
		DebugPrintf("frontScale: %i\n", _vm->_scene->getSceneResources().frontScale);
		DebugPrintf("backScale: %i\n", _vm->_scene->getSceneResources().backScale);
		DebugPrintf("depthTable: ");
		for (uint i = 0; i < 16; i++)
			DebugPrintf("%i ", _vm->_scene->getSceneResources().depthTable[i]);
		DebugPrintf("\n");
		DebugPrintf("railNodeCount: %i\n", _vm->_scene->getSceneResources().railNodeCount);
	}
	return true;
}

bool Console::cmdListHotSpots(int argc, const char **argv) {
	DebugPrintf("Scene hotspots\n");
	_vm->_scene->getSceneResources().hotspots->dump();
	if (_vm->isM4()) {
		DebugPrintf("Scene parallax\n");
		_vm->_scene->getSceneResources().parallax->dump();
		DebugPrintf("Scene props\n");
		_vm->_scene->getSceneResources().props->dump();
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
		DebugPrintf("The DSR file parameter is optional, and specifies which DSR to load\n", argv[0]);
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
	if (_vm->getGameType() != GType_RexNebular)
		_vm->_scene->showCodes();
	else
		DebugPrintf("Pathfinding codes not done yet for Rex Nebular");
	return false;
}

bool Console::cmdDumpFile(int argc, const char **argv) {
	if (argc != 2 && argc != 3) {
		DebugPrintf("Usage: %s <file> <uncompress>\n", argv[0]);
		DebugPrintf("If uncompress is 1, the file is uncompressed (for MADS games)\n");
	} else {
		if (argc == 2) {
			_vm->dumpFile(strdup(argv[1]));
		} else {
			if (argc == 3 && atoi(argv[2]) == 1)
				_vm->dumpFile(strdup(argv[1]), true);
			else
				_vm->dumpFile(strdup(argv[1]));
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

			spr->copyTo(bg, x, y, (int)spr->getTransparentColor());

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
	} else {
		_vm->_converse->startConversation(argv[1]);
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

bool Console::cmdPlayAnimation(int argc, const char **argv) {
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
			_vm->_animation->loadFullScreen(resourceName);
		else
			_vm->_animation->load(resourceName);
		_vm->_animation->start();
		view->restore(0, 0, view->width(), view->height());
		return false;
	}

	return true;
}

bool Console::cmdObject(int argc, const char **argv) {
	if (_vm->isM4()) {
		DebugPrintf("Command not implemented for M4 games\n");
	} else if (argc == 1) {
		DebugPrintf("Usage: object ['list' | '#objnum' | 'add #objnum']\n");
	} else if (!strcmp(argv[1], "list")) {
		// List of objects
		for (uint objStart = 0; objStart < _vm->_globals->getObjectsSize(); objStart += 5) {
			DebugPrintf("%2d - ", objStart);
			for (uint objId = objStart; objId < MIN<uint>(_vm->_globals->getObjectsSize(), objStart + 5); ++objId) {
				if (objId != objStart) DebugPrintf(", ");
				uint16 descId = _vm->_globals->getObject(objId)->descId;
				DebugPrintf("%s", _vm->_globals->getVocab(descId));
			}

			DebugPrintf("\n");
		}

		DebugPrintf("\n");
	} else if (!strcmp(argv[1], "add") && (argc == 3)) {
		// Add the specified object to the player's inventory
		int objNum = strToInt(argv[2]);

		if ((objNum < 0) || (objNum >= (int)_vm->_globals->getObjectsSize()))
			DebugPrintf("Invalid object specified\n");
		else if (_vm->isM4())
			DebugPrintf("Not implemented for M4 games\n");
		else {
			_vm->_scene->getMadsInterface()->addObjectToInventory(objNum);
			return false;
		}

	} else {
		// Print the details of a specific object
		int objNum = strToInt(argv[1]);

		if ((objNum < 0) || (objNum >= (int)_vm->_globals->getObjectsSize()))
			DebugPrintf("Invalid object specified\n");
		else {
			const MadsObject *obj = _vm->_globals->getObject(objNum);

			DebugPrintf("Object #%d (%s) room=%d article=%d/%s vocabs=%d", objNum, _vm->_globals->getVocab(obj->descId),
				obj->roomNumber, (int)obj->article, englishMADSArticleList[obj->article], obj->vocabCount);

			if (obj->vocabCount > 0) {
				DebugPrintf(" - ");
				for (int i = 0; i < obj->vocabCount; ++i) {
					if (i != 0) DebugPrintf(", ");
					DebugPrintf("%s (%d)/%d", _vm->_globals->getVocab(obj->vocabList[i].vocabId),
						obj->vocabList[i].vocabId, obj->vocabList[i].unk);
				}
			}
			DebugPrintf("\n");
		}
	}

	return true;
}

} // End of namespace M4
