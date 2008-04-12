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

namespace Kyra {

class SoundDigital;
class Screen_v3;
class MainMenu;

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
	void realInit();

	void setupOpcodeTable() {}

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
	static const char *_mainMenuStrings[];

	// translation stuff
	uint8 *_scoreFile;
	uint8 *_cCodeFile;
	uint8 *_scenesList;

	// interface?
	uint8 *_interfaceCPS1;
	uint8 *_interfaceCPS2;

	// shapes
	uint8 *_gameShapes[50];

	uint8 *_mouseSHPBuf;

	// items
	uint8 *_itemBuffer1;
	uint8 *_itemBuffer2;

	uint8 *_itemList;

	void initItems();

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

