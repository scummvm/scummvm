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

#include "engines/engine.h"

#include "common/platform.h"
#include "common/queue.h"
#include "common/rect.h"
#include "common/str.h"

#include "audio/mixer.h"

#include "video/3do_decoder.h"

struct ADGameDescription;

namespace Image {
class ImageDecoder;
}

namespace Graphics {
struct Surface;
}

namespace Plumbers {

class Console;

enum PlumbersDebugChannels {
	kDebugGeneral = 1 << 0
};

static const int kMaxChoice = 3;

struct Choice {
	long _points;
	int  _skipScene;
	Common::Rect _region;
	Common::String _sceneName;
};
	
struct Scene {
	int	 _bitmapNum;
	int	 _startBitmap;
	int	 _decisionChoices;
	Common::String _sceneName;
	Common::String _waveFilename;
	Common::String _decisionBitmap;
	enum {
		STYLE_PC = 0,
		STYLE_DECISION_MIKE = 1,
		STYLE_DECISION_TUN = 2,
		STYLE_VIDEO = 3
	} _style;
	Choice _choices[kMaxChoice];
};

class PlumbersGame : public Engine {
public:
	PlumbersGame(OSystem *syst, const ADGameDescription *gameDesc);
	~PlumbersGame() override;

	Common::Error run() override;

	// Detection related functions
	const ADGameDescription *_gameDescription;
	const char *getGameId() const;
	Common::Platform getPlatform() const;

private:
	static const int kMaxName = 13 + 1;
	static const int kMaxBitmaps = 2000;
	static const int kMaxScene = 100;

	struct {
		int  _duration;
		Common::String _filename;
	} _bitmaps[kMaxBitmaps];

	Scene _scenes[kMaxScene];

	Image::ImageDecoder *_image;
	Image::ImageDecoder *_ctrlHelpImage;
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
	int _screenW, _screenH;
	int _kbdHiLite;
	int _mouseHiLite;
	int _hiLite;
	bool _hiLiteEnabled;
	bool _cheatEnabled;
	int _cheatFSM;
	bool _leftShoulderPressed;

	enum Action {
		Redraw,
		ShowScene,
		UpdateScene,
		ChangeScene,
		PlaySound
	};

	Common::Queue<Action> _actions;
	Graphics::Surface *_compositeSurface;

	void loadImage(const Common::String &name);
	void loadMikeDecision(const Common::String &dirname, const Common::String &baseFilename, uint num);
	void drawScreen();
	void updateHiLite();

	Audio::SoundHandle _soundHandle;
	Video::ThreeDOMovieDecoder *_videoDecoder;

	void playSound(const Common::String &name);
	void stopSound();

	void showScene();
	void updateScene();
	void changeScene();

	void processTimer();
	static void onTimer(void *arg);

	void initTables();
	void readTablesPC(const Common::String &fileName);
    	void readTables3DO(const Common::String &fileName);
	int getSceneNumb(const Common::String &sName);
	int getMouseHiLite();

	void joyUp();
	void joyDown();
	void joyA();
	void skipVideo();
};
} // End of namespace Plumbers

#endif
