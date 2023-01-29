/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef PLUMBERS_PLUMBERS_H
#define PLUMBERS_PLUMBERS_H

#include "engines/engine.h"

#include "common/events.h"
#include "common/platform.h"
#include "common/queue.h"
#include "common/rect.h"
#include "common/str.h"

#include "audio/mixer.h"

#include "video/video_decoder.h"
#include "image/image_decoder.h"

struct ADGameDescription;

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

protected:
	virtual void readTables() = 0;
	virtual void postSceneBitmaps() = 0;
	virtual bool handlePlatformJoyButton(int button) { return false; }
	virtual bool handlePlatformKeyDown(int button) { return false; }
	virtual void loadImage(const Common::String &name);
	virtual void startGraphics() = 0;
	void blitImageSurface(Graphics::Surface *screen, const Graphics::Surface *surface);
	virtual void blitImage(Graphics::Surface *screen);
	virtual void handleEvent(const Common::Event &event);
	virtual int getSceneNumb(const Common::String &sName);
	virtual void preActions() {}

	static const int kMaxName = 13 + 1;
	static const int kMaxBitmaps = 2000;
	static const int kMaxScene = 100;
	void initTables();

	struct {
		int  _duration;
		Common::String _filename;
	} _bitmaps[kMaxBitmaps];

	Scene _scenes[kMaxScene];
	int	 _totScene;
	long _totScore;

	Image::ImageDecoder *_image;
	Console *_console;
	Video::VideoDecoder *_videoDecoder;
	bool _showScoreFl;
	bool _setDurationFl;
	bool _leftButtonDownFl;
	bool _endGameFl;
	bool _timerInstalled;
	int	 _curSceneIdx, _prvSceneIdx;
	int	 _curBitmapIdx;
	int	 _curChoice;
	int _screenW, _screenH;
	bool _quit;

	enum Action {
		Redraw,
		ShowScene,
		UpdateScene,
		ChangeScene,
		PlaySound
	};

	Common::Queue<Action> _actions;
	Graphics::Surface *_compositeSurface;

	void drawScreen();

	Audio::SoundHandle _soundHandle;

	void playSound(const Common::String &name);
	void stopSound();

	void showScene();
	void updateScene();
	void changeScene();

	void processTimer();
	static void onTimer(void *arg);

	int getMouseHiLite();
};

class PlumbersGame3DO : public PlumbersGame {
public:
	PlumbersGame3DO(OSystem *syst, const ADGameDescription *gameDesc);

protected:
	void readTables() override;
	void postSceneBitmaps() override;
	void startGraphics() override;
	void handleEvent(const Common::Event &event) override;
	void blitImage(Graphics::Surface *screen) override;
	int getSceneNumb(const Common::String &sName) override;
	void preActions() override;

private:
	void skipVideo();
	void loadMikeDecision(const Common::String &dirname, const Common::String &baseFilename, uint num);
	void joyUp();
	void joyDown();
	void joyA();
	void updateHiLite();

	bool _cheatEnabled;
	int _cheatFSM;
	bool _leftShoulderPressed;
	int _kbdHiLite;
	int _mouseHiLite;
	int _hiLite;
	Image::ImageDecoder *_ctrlHelpImage;
};

class PlumbersGameWindows : public PlumbersGame {
public:
	PlumbersGameWindows(OSystem *syst, const ADGameDescription *gameDesc);

protected:
	void readTables() override;
	void postSceneBitmaps() override;
	void loadImage(const Common::String &name) override;
	void startGraphics() override;

private:
	bool _halfSize;
};

} // End of namespace Plumbers

#endif
