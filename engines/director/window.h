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

#ifndef DIRECTOR_STAGE_H
#define DIRECTOR_STAGE_H

#include "graphics/macgui/macwindow.h"
#include "director/lingo/lingo-object.h"

namespace Common {
class Error;
}

namespace Graphics {
class ManagedSurface;
class MacWindow;
class MacWindowManager;
}

namespace Director {

class Channel;
class MacArchive;
struct MacShape;
struct LingoState;

struct TransParams {
	TransitionType type;
	uint frame;
	uint duration;
	uint chunkSize;
	uint area;

	int steps;
	int stepDuration;

	int xStepSize;
	int yStepSize;

	int xpos, ypos;

	int stripSize;

	const byte *sourcePal;
	uint16 sourcePalLength;
	const byte *targetPal;
	uint16 targetPalLength;
	byte tempPal[768];

	TransParams() : tempPal() {
		type = kTransNone;
		frame = 0;
		duration = 250;
		chunkSize = 1;
		area = 0;
		steps = 0;
		stepDuration = 0;
		stripSize = 0;

		xStepSize = yStepSize = 0;
		xpos = ypos = 0;

		sourcePal = nullptr;
		sourcePalLength = 0;
		targetPal = nullptr;
		targetPalLength = 0;
	}

	TransParams(uint16 d, uint16 a, uint16 c, TransitionType t) :
			duration(d), area(a), chunkSize(c), type(t), tempPal() {
		frame = 0;
		steps = 0;
		stepDuration = 0;
		stripSize = 0;

		xStepSize = yStepSize = 0;
		xpos = ypos = 0;

		sourcePal = nullptr;
		sourcePalLength = 0;
		targetPal = nullptr;
		targetPalLength = 0;
	}
};

class Window : public Graphics::MacWindow, public Object<Window> {
public:
	Window(int id, bool scrollable, bool resizable, bool editable, Graphics::MacWindowManager *wm, DirectorEngine *vm, bool isStage);
	~Window();

	bool render(bool forceRedraw = false, Graphics::ManagedSurface *blitTo = nullptr);
	void invertChannel(Channel *channel, const Common::Rect &destRect);

	bool needsAppliedColor(DirectorPlotData *pd);
	void setStageColor(uint32 stageColor, bool forceReset = false);
	uint32 getStageColor() { return _stageColor; }

	void reset();

	// transitions.cpp
	void exitTransition(TransParams &t, int step, Graphics::ManagedSurface *nextFrame, Common::Rect clipRect);
	void stepTransition(TransParams &t, int step);
	void playTransition(uint frame, uint16 transDuration, uint8 transArea, uint8 transChunkSize, TransitionType transType, CastMemberID paletteId);
	void initTransParams(TransParams &t, Common::Rect &clipRect);
	void dissolveTrans(TransParams &t, Common::Rect &clipRect, Graphics::ManagedSurface *tmpSurface);
	void dissolvePatternsTrans(TransParams &t, Common::Rect &clipRect, Graphics::ManagedSurface *tmpSurface);
	void transMultiPass(TransParams &t, Common::Rect &clipRect, Graphics::ManagedSurface *tmpSurface);
	void transZoom(TransParams &t, Common::Rect &clipRect, Graphics::ManagedSurface *currentFrame, Graphics::ManagedSurface *nextFrame);

	// window.cpp
	Common::Point getMousePos();

	DirectorEngine *getVM() const { return _vm; }
	Archive *getMainArchive() const { return _mainArchive; }
	Movie *getCurrentMovie() const { return _currentMovie; }
	Common::String getCurrentPath() const { return _currentPath; }
	DirectorSound *getSoundManager() const { return _soundManager; }

	void setVisible(bool visible, bool silent = false) override;
	bool setNextMovie(Common::String &movieFilenameRaw);

	void setWindowType(int type) { _windowType = type; updateBorderType(); }
	int getWindowType() const { return _windowType; }
	void setTitleVisible(bool titleVisible) override;
	Datum getStageRect();
	bool setStageRect(Datum datum);
	void setModal(bool modal);
	bool getModal() { return _isModal; };
	void setFileName(Common::String filename);
	Common::String getFileName() { return getName(); }

	void updateBorderType();

	bool step();
	bool loadNextMovie();
	void loadNewSharedCast(Cast *previousSharedCast);

	Common::Path getSharedCastPath();

	LingoState *getLingoState() { return _lingoState; };
	uint32 frozenLingoStateCount() { return _frozenLingoStates.size(); };
	void freezeLingoState();
	void thawLingoState();

	// events.cpp
	bool processEvent(Common::Event &event) override;

	// tests.cpp
	Common::HashMap<Common::String, Movie *> *scanMovies(const Common::String &folder);
	void testFontScaling();
	void testFonts();
	void enqueueAllMovies();
	MovieReference getNextMovieFromQueue();
	void runTests();

	// resource.cpp
	Common::Error loadInitialMovie();
	void probeResources(Archive *archive);
	void loadINIStream();
	void loadStartMovieXLibs();

	// lingo/lingo-object.cpp
	Common::String asString() override;
	bool hasProp(const Common::String &propName) override;
	Datum getProp(const Common::String &propName) override;
	bool setProp(const Common::String &propName, const Datum &value) override;
	bool hasField(int field) override;
	Datum getField(int field) override;
	bool setField(int field, const Datum &value) override;

public:
	Common::List<Channel *> _dirtyChannels;
	TransParams *_puppetTransition;

	MovieReference _nextMovie;
	Common::List<MovieReference> _movieStack;
	bool _newMovieStarted;

private:
	uint32 _stageColor;

	DirectorEngine *_vm;
	DirectorSound *_soundManager;
	LingoState *_lingoState;
	Common::Array<LingoState *> _frozenLingoStates;
	bool _isStage;
	Archive *_mainArchive;
	Movie *_currentMovie;
	Common::String _currentPath;
	Common::StringArray _movieQueue;
	int16 _startFrame;

	int _windowType;
	bool _isModal;

private:
	void inkBlitFrom(Channel *channel, Common::Rect destRect, Graphics::ManagedSurface *blitTo = nullptr);
	void drawFrameCounter(Graphics::ManagedSurface *blitTo);


};

} // End of namespace Director

#endif
