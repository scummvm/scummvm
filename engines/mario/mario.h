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

#ifndef MARIO_MARIO_H
#define MARIO_MARIO_H

#include "common/scummsys.h"
#include "common/config-manager.h"
#include "engines/advancedDetector.h"
#include "common/error.h"
#include "engines/engine.h"

#include "audio/audiostream.h"
#include "audio/decoders/wave.h"
#include "audio/mixer.h"
#include "common/events.h"
#include "common/file.h"
#include "common/queue.h"
#include "common/system.h"
#include "common/timer.h"
#include "graphics/palette.h"
#include "graphics/screen.h"
#include "graphics/surface.h"
#include "image/bmp.h"

namespace Mario {

class MarioGame : public Engine {
public:
	MarioGame(OSystem *syst, const ADGameDescription *gameDesc);
	~MarioGame();

	virtual Common::Error run();

	// Detection related functions
	const ADGameDescription *_gameDescription;
	const char *getGameId() const;
	Common::Platform getPlatform() const;

private:

	static const int kMaxName = 13 + 1;
	static const int kMaxBitmaps = 2000;
	static const int kMaxChoice = 3;
	static const int kMaxScene = 100;

	struct {
		int	sBitmapDuration;
		char	cBitmapFileName[kMaxName];
	} aBitmaps[kMaxBitmaps];

	struct {
		int	sNoBitmaps;
		int	sStartBitmap;
		int	sNoDecisionChoices;
		char	cSceneName[kMaxName];
		char	cWaveFileName[kMaxName];
		char	cDecisionBitmap[kMaxName];
		struct {
			long		lPoints;
			int		sGoTo;
			int		bSkipScene;
			Common::Rect	aRegion;
		}	aChoice[kMaxChoice];
	} aScene[kMaxScene];

	struct {
		long	lTotScore;
		int	sPrevScene;
		int	sCurrScene;
		int	sCurrSceneStep;
		int	sLkhdScene;
		int	sLkhdBitmapNo;
		int	sTotScene;
		int	sTotBitmap;
		int	bEightBit;
		int	bIsaDecision;
	} Game;

	Image::ImageDecoder *_image;

	int	xadj;
	int	yadj;

	bool	bShowScore;
	bool	bSetDuration;
	bool	bLButtonDown;
	bool	bEnd;
	int	sCurScene;
	int	sCurBitmap;
	int	sCurChoice;
	int	sPrvScene;
	Common::Point	point;
	Common::Rect	rect;

	enum Action {
		Redraw,
		ShowScene,
		UpdateScene,
		ChangeScene,
		PlaySound
	};

	Common::Queue<Action> _actions;
	
	bool loadImage(char *dirname, char *filename);
	void drawScreen();

	Audio::SoundHandle _soundHandle;

	void playSound();
	void stopSound();

	void showScene();
	void updateScene();
	void changeScene();

	void processTimer();
	static void onTimer(void *arg);

	void sInitTables();
	void sReadTables(char *cFileName);
	int sGetSceneNumb(int sNo);
};

extern MarioGame *g_mario;

} // End of namespace Mario

#endif
