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

#ifndef PLUMBERS_PLUMBERS_H
#define PLUMBERS_PLUMBERS_H

#include "common/scummsys.h"
#include "common/config-manager.h"
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

#include "plumbers/console.h"

struct ADGameDescription;

namespace Plumbers {
enum PlumbersDebugChannels {
	kDebugGeneral = 1 << 0
};

class PlumbersGame : public Engine {
public:
	PlumbersGame(OSystem *syst, const ADGameDescription *gameDesc);
	~PlumbersGame();

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
		int  _duration;
		Common::String _filename;
	} _bitmaps[kMaxBitmaps];

	struct {
		int	 _bitmapNum;
		int	 _startBitmap;
		int	 _decisionChoices;
		Common::String _sceneName;
		Common::String _waveFilename;
		Common::String _decisionBitmap;
		struct {
			long _points;
			int  _sceneIdx;
			int  _skipScene;
			Common::Rect _region;
		} _choices[kMaxChoice];
	} _scenes[kMaxScene];

	Image::ImageDecoder *_image;
	Console *_console;

	bool _showScoreFl;
	bool _setDurationFl;
	bool _leftButtonDownFl;
	bool _endGameFl;
	bool _timerInstalled; 
	int	 _curSceneIdx, _prvSceneIdx;
	int	 _curBitmapIdx;
	int	 _curChoice;
	int	 _totScene;
	long _totScore;

	enum Action {
		Redraw,
		ShowScene,
		UpdateScene,
		ChangeScene,
		PlaySound
	};

	Common::Queue<Action> _actions;

	void loadImage(const Common::String &dirname, const Common::String &filename);
	void drawScreen();

	Audio::SoundHandle _soundHandle;

	void playSound();
	void stopSound();

	void showScene();
	void updateScene();
	void changeScene();

	void processTimer();
	static void onTimer(void *arg);

	void initTables();
	void readTables(const Common::String &fileName);
	int getSceneNumb(int sNo);
};
} // End of namespace Plumbers

#endif
