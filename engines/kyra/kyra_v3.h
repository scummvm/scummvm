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

#ifndef KYRA_KYRA_V3_H
#define KYRA_KYRA_V3_H

#include "kyra/kyra.h"
#include "kyra/screen_v3.h"
#include "common/hashmap.h"

namespace Kyra {

class SoundDigital;
class Screen_v3;
class MainMenu;
class WSAMovieV2;

class KyraEngine_v3 : public KyraEngine {
public:
	KyraEngine_v3(OSystem *system, const GameFlags &flags);
	~KyraEngine_v3();

	Screen *screen() { return _screen; }
	SoundDigital *soundDigital() { return _soundDigital; }

	int go();

	void playVQA(const char *name);

	virtual Movie *createWSAMovie();
private:
	int init();

	void preinit();
	void startup();

	void runStartupScript(int script, int unk1);

	void setupOpcodeTable();

	Screen_v3 *_screen;
	SoundDigital *_soundDigital;

	// sound specific
private:
	void playMenuAudioFile();

	int _musicSoundChannel;
	const char *_menuAudioFile;

	static const char *_soundList[];
	static const int _soundListSize;

	int _curMusicTrack;

	void playMusicTrack(int track, int force);
	void stopMusicTrack();

	int musicUpdate(int forceRestart);

	void snd_playVoiceFile(int) {}

	// main menu
	void initMainMenu();
	void uninitMainMenu();

	Movie *_menuAnim;
	MainMenu *_menu;

	// game speed
	bool skipFlag() const { return false; }
	void resetSkipFlag(bool) {}

	// timer
	void setupTimers() {}
	void setWalkspeed(uint8) {}

	// pathfinder
	bool lineIsPassable(int, int) { return false; }

private:
	// main menu
	static const char *_mainMenuStrings[];

	// animator
	struct AnimObj {
		uint16 index;
		uint16 type;
		bool enabled;
		bool needRefresh;
		uint16 unk8;
		uint16 flags;
		int16 xPos1, yPos1;
		uint8 *shapePtr;
		uint16 shapeIndex;
		uint16 animNum;
		uint16 shapeIndex3;
		uint16 shapeIndex2;
		int16 xPos2, yPos2;
		int16 xPos3, yPos3;
		int16 width, height;
		int16 width2, height2;
		AnimObj *nextObject;
	};

	AnimObj *_animObjects;
	uint8 *_gamePlayBuffer;

	void clearAnimObjects();

	// interface
	uint8 *_interface;
	uint8 *_interfaceCommandLine;

	void loadInterfaceShapes();
	void loadInterface();

	// translation stuff
	uint8 *_scoreFile;
	uint8 *_cCodeFile;
	uint8 *_scenesFile;
	uint8 *_itemFile;
	uint8 *_actorFile;
	uint32 _actorFileSize;

	// items
	uint8 *_itemBuffer1;
	uint8 *_itemBuffer2;
	
	void initItems();

	// shapes
	typedef Common::HashMap<int, uint8*> ShapeMap;
	ShapeMap _gameShapes;

	void addShapeToPool(const uint8 *data, int realIndex, int shape);

	void initMouseShapes();

	int _malcolmShapes;
	void loadMalcolmShapes(int newShapes);
	void updateMalcolmShapes();

	int _malcolmShapeXOffset, _malcolmShapeYOffset;

	struct ShapeDesc {
		uint8 width, height;
		int8 xOffset, yOffset;
	};
	static const ShapeDesc _shapeDescs[];
	static const int _shapeDescsSize;

	// scene animation
	struct SceneAnim {
		uint16 flags;
		int16 x, y;
		int16 x2, y2;
		int16 width, height;
		uint16 unk10;
		uint16 specialSize;
		uint16 unk14;
		uint16 shapeIndex;
		uint16 wsaFlag;
		char filename[13];
	};

	SceneAnim *_sceneAnims;
	WSAMovieV2 *_sceneAnimMovie[16];
	uint8 *_sceneShapes[20];

	// voice
	int _currentTalkFile;
	void openTalkFile(int file);

	// scene
	struct SceneDesc {
		char filename1[10];
		char filename2[10];
		uint16 exit1, exit2, exit3, exit4;
		uint8 flags, sound;
	};

	SceneDesc *_sceneList;
	uint16 _sceneExit1, _sceneExit2, _sceneExit3, _sceneExit4;

	// items
	struct Item {
		uint16 id;
		uint16 sceneId;
		int16 x, y;
		uint16 unk8;
	};

	Item *_itemList;
	uint16 _hiddenItems[100];

	void resetItem(int index);
	void resetItemList();

	int findFreeItem();

	// character
	struct Character {
		uint16 sceneId;
		uint16 dlgIndex;
		uint8 unk4;
		uint8 facing;
		uint16 animFrame;
		//uint8 unk8, unk9;
		uint32 walkspeed;
		uint16 inventory[10];
		int16 x1, y1;
		int16 x2, y2;
		int16 x3, y3;
	};

	Character _mainCharacter;

	// unk
	uint8 *_unkBuffer1040Bytes;
	uint8 *_costPalBuffer;
	uint8 *_screenBuffer;
	uint8 *_gfxBackUpRect;
	uint8 *_paletteOverlay;

	void loadCostPal();
	void loadShadowShape();
	void loadExtrasShapes();

	// opcodes
	int o3_defineItem(ScriptState *script);
	int o3_queryGameFlag(ScriptState *script);
	int o3_resetGameFlag(ScriptState *script);
	int o3_setGameFlag(ScriptState *script);
	int o3_setSceneFilename(ScriptState *script);
	int o3_getRand(ScriptState *script);
	int o3_defineScene(ScriptState *script);
	int o3_setHiddenItemsEntry(ScriptState *script);
	int o3_getHiddenItemsEntry(ScriptState *script);
	int o3_dummy(ScriptState *script);

	// resource specific
private:
	static const char *_languageExtension[];
	static const int _languageExtensionSize;

	int getMaxFileSize(const char *file);
	char *appendLanguage(char *buf, int lang, int bufSize);

	int loadLanguageFile(const char *file, uint8 *&buffer);
};

} // end of namespace Kyra

#endif

