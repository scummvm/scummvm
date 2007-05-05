/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
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

namespace Kyra {

// maybe subclass KyraEngine_v2 later
class WSAMovieV2;

class KyraEngine_v3 : public KyraEngine {
public:
	KyraEngine_v3(OSystem *system, const GameFlags &flags);
	~KyraEngine_v3();
	
	Movie *createWSAMovie();
	
	SoundDigital *soundDigital() { return _soundDigital; }

	int go();

	void playVQA(const char *name);
private:
	int init();

	void preinit();
	void realInit();

	void setupOpcodeTable() {}

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

	virtual void gui_initMainMenu();
	virtual void gui_updateMainMenuAnimation();

	// unknown
private:
	uint8 *_unkPage1;
	uint8 *_unkPage2;

	uint8 *_unkBuffer5;
	uint8 *_unkBuffer6;
	uint8 *_unkBuffer7;
	uint8 *_unkBuffer9;

	uint8 *_costpalData;

	uint8 *_unkWSAPtr;

	uint8 *_unkShapeTable[20];

	// main menu
	Movie *_mainMenuLogo;
	int _mainMenuFrame;
	int _mainMenuFrameAdd;

	// translation stuff
	uint8 *_scoreFile;
	uint8 *_cCodeFile;
	uint8 *_scenesList;

	// interface?
	uint8 *_interfaceCPS1;
	uint8 *_interfaceCPS2;

	// shapes
	uint8 *_gameShapes[50];
	uint8 *_shapePoolBuffer;

	uint8 *_mouseSHPBuf;

	// items
	uint8 *_itemBuffer1;
	uint8 *_itemBuffer2;

	uint8 *_itemList;

	void initItems();

	// used for CSH loading and some sound stuff (maybe voice files?)
private:
	uint8 *_tableBuffer1;
	uint8 *_tableBuffer2;
	int _unkTableValue;

	// do not think of thouching the code belonging to these functions
	int initTableBuffer(uint8 *buf, int size);
	void updateTableBuffer(uint8 *buf);
	int getTableSize(uint8 *buf);
	uint8 *allocTableSpace(uint8 *buf, int size, int id);
	uint8 *findIdInTable(uint8 *buf, int id);

	int addShapeToTable(const uint8 *buf, int id, int shapeNum);
	uint8 *findShapeInTable(int id);

	// resource specific
private:
	static const char *_languageExtension[];
	static const int _languageExtensionSize;

	int getMaxFileSize(const char *file);
	char *appendLanguage(char *buf, int lang, int bufSize);

	bool loadLanguageFile(const char *file, uint8 *&buffer);
};

} // end of namespace Kyra

#endif

