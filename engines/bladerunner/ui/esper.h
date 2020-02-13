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

#ifndef BLADERUNNER_ESPER_H
#define BLADERUNNER_ESPER_H

#include "common/array.h"
#include "common/rect.h"

#include "graphics/surface.h"

namespace BladeRunner {

class BladeRunnerEngine;
class Font;
class Shape;
class Shapes;
class VQAPlayer;
class UIImagePicker;
class ESPERScript;

// CD-changing logic has been removed

enum EsperMainStates {
	kEsperMainStateOpening      = 0,
	kEsperMainStateList         = 1,
	kEsperMainStatePhotoOpening = 2,
	kEsperMainStateClear        = 3,
	kEsperMainStatePhoto        = 5
};

enum EsperPhotoStates {
	kEsperPhotoStateShow              = 0,
	kEsperPhotoStateOpening           = 1,
	kEsperPhotoStateScrolling         = 2,
	kEsperPhotoStateSelectionZooming  = 3,
	kEsperPhotoStateSelectionBlinking = 4,
	kEsperPhotoStatePhotoZooming      = 5,
	kEsperPhotoStatePhotoSharpening   = 6,
	kEsperPhotoStatePhotoZoomOut      = 7,
	kEsperPhotoStateVideoZooming      = 8,
	kEsperPhotoStateVideoShow         = 9,
	kEsperPhotoStateVideoZoomOut      = 10
};

class ESPER {
	static const int kPhotoCount         = 12;
	static const int kRegionCount        = 6;
	static const int kPhotoWidth         = 1280;
	static const int kPhotoHeight        = 960;
	static const int kSelectionZoomSteps = 6;

	static const Common::Rect kScreen;

	struct Photo {
		bool           isPresent;
		int            photoId;
		int            shapeId;
		Common::String name;
	};

	struct Region {
		bool           isPresent;
		int            regionId;
		Common::Rect   rectInner;
		Common::Rect   rectOuter;
		Common::Rect   rectSelection;
		Common::String name;
	};

	BladeRunnerEngine     *_vm;
	ESPERScript           *_script;

	bool _isWaiting;
	bool _isOpen;

	UIImagePicker     *_buttons;

	Graphics::Surface _surfacePhoto;
	Graphics::Surface _surfaceViewport;

	VQAPlayer *_vqaPlayerMain;
	VQAPlayer *_vqaPlayerPhoto;
	int        _vqaLastFrame;

	Shapes *_shapesButtons;
	Shapes *_shapesPhotos;
	const Shape *_shapeThumbnail;

	Photo _photos[kPhotoCount];
	int   _photoIdSelected;

	Region _regions[kRegionCount];
	int    _regionSelected;
	bool   _regionSelectedAck;

	EsperMainStates  _stateMain;
	EsperPhotoStates _statePhoto;

	bool _isDrawingSelection;
	bool _isMouseDown;
	int  _mouseOverScroll;

	float  _zoomHorizontal;
	float  _zoomVertical;
	float  _zoom;
	float  _zoomMin;
	float  _zoomTarget;
	float  _zoomDelta;
	float  _blur;
	int    _zoomSteps;
	int    _zoomStep;
	uint32 _timeZoomNextDiff;
	uint32 _timeZoomNextStart;

	bool   _isZoomingOut;
	uint32 _timeZoomOutNextStart;

	Common::Rect _screen;

	Common::Rect _viewport;
	Common::Rect _viewportNext;
	int          _viewportPositionX;
	int          _viewportPositionY;
	float        _viewportPositionXCurrent;
	float        _viewportPositionYCurrent;
	int          _viewportPositionXTarget;
	int          _viewportPositionYTarget;
	float        _viewportPositionXDelta;
	float        _viewportPositionYDelta;
	int          _viewportWidth;
	int          _viewportHeight;

	int _screenHalfWidth;
	int _screenHalfHeight;

	int _flash;

	Common::Rect _selection;
	Common::Rect _selectionTarget;
	Common::Rect _selectionDelta;
	int          _selectionCrosshairX;
	int          _selectionCrosshairY;

	int    _selectionBlinkingCounter;
	int    _selectionBlinkingStyle;
	uint32 _timeSelectionBlinkingNextStart;

	int    _selectionZoomStep;
	uint32 _timeSelectionZoomNextStart;

	int    _photoOpeningWidth;
	int    _photoOpeningHeight;
	uint32 _timePhotoOpeningNextDiff;
	uint32 _timePhotoOpeningNextStart;

	bool   _isScrolling;
	int    _scrollingDirection;
	uint32 _timeScrollNextStart;

	int _soundId1;
	int _volume1;
	int _soundId2;
	int _volume2;
	int _soundId3;
	int _volume3;
	int _ambientVolume;

public:
	ESPER(BladeRunnerEngine *vm);
	~ESPER();

	void open(Graphics::Surface *surface);
	void close();
	bool isOpen() const;

	void handleMouseUp(int x, int y, bool buttonLeft);
	void handleMouseDown(int x, int y, bool buttonLeft);

	void tick();

	void addPhoto(const char *name, int photoId, int shapeId);
	void defineRegion(int regionId, Common::Rect inner, Common::Rect outer, Common::Rect selection, const char *name);

private:
	static void mouseDownCallback(int, void *);
	static void mouseUpCallback(int, void *);

	void reset();
	void resetData();
	void resetPhotos();
	void resetRegions();
	void resetViewport();
	void resetSelectionRect();
	void resetSelectionBlinking();
	void resetPhotoZooming();
	void resetPhotoOpening();

	void updateViewport();

	void activate(bool withOpening);
	void setStateMain(EsperMainStates state);
	void setStatePhoto(EsperPhotoStates state);

	void wait(uint32 timeout);
	void playSound(int soundId, int volume);

	void draw(Graphics::Surface &surface);

	void drawPhotoOpening(Graphics::Surface &surface);
	bool drawSelectionZooming(Graphics::Surface &surface);
	bool drawSelectionBlinking(Graphics::Surface &surface);
	void drawPhotoZooming(Graphics::Surface &surface);
	void drawPhotoSharpening(Graphics::Surface &surface);
	void drawPhotoZoomOut(Graphics::Surface &surface);
	void drawVideoZooming(Graphics::Surface &surface);
	void drawVideoZoomOut(Graphics::Surface &surface);

	void drawPhoto(Graphics::Surface &surface);
	void drawGrid(Graphics::Surface &surface);
	void drawPhotoWithGrid(Graphics::Surface &surface);
	void drawSelection(Graphics::Surface &surface, bool crosshair, int style);
	void drawVideoFrame(Graphics::Surface &surface);
	void drawTextCoords(Graphics::Surface &surface);
	void drawMouse(Graphics::Surface &surface);

	void flashViewport();

	void copyImageScale(Graphics::Surface &src, Common::Rect srcRect, Graphics::Surface &dst, Common::Rect dstRect);
	void copyImageBlur(Graphics::Surface &src, Common::Rect srcRect, Graphics::Surface &dst, Common::Rect dstRect, float u);
	void copyImageBlit(Graphics::Surface &src, Common::Rect srcRect, Graphics::Surface &dst, Common::Rect dstRect);

	void tickSound();
	void tickScroll();

	int findEmptyPhoto();
	void selectPhoto(int photoId);
	void unloadPhotos();

	int findEmptyRegion();
	int findRegion(Common::Rect where);

	void zoomingStart();
	void zoomOutStart();
	void zoomOutStop();

	void scrollingStart(int direction);
	void scrollingStop();
	void scrollUpdate();
	void scrollLeft();
	void scrollUp();
	void scrollRight();
	void scrollDown();

	void goBack();

	void prepareZoom();
	void updateSelection();

	int viewportXToScreenX(int x);
	int viewportYToScreenY(int y);
};

} // End of namespace BladeRunner

#endif
