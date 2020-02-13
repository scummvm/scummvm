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

#ifndef MADS_SCREEN_H
#define MADS_SCREEN_H

#include "common/scummsys.h"
#include "common/array.h"
#include "mads/msurface.h"
#include "mads/action.h"

namespace MADS {

#define MADS_SCREEN_WIDTH 320
#define MADS_SCREEN_HEIGHT 200

enum ScreenMode {
	SCREENMODE_VGA = 19
};

enum ScreenTransition {
	kTransitionNone = 0,
	kTransitionFadeIn, kTransitionFadeOutIn,
	kTransitionBoxInBottomLeft, kTransitionBoxInBottomRight,
	kTransitionBoxInTopLeft, kTransitionBoxInTopRight,
	kTransitionPanLeftToRight, kTransitionPanRightToLeft,
	kTransitionCircleIn1, kTransitionCircleIn2,
	kTransitionCircleIn3, kTransitionCircleIn4,
	kVertTransition1, kVertTransition2, kVertTransition3,
	kVertTransition4, kVertTransition5, kVertTransition6,
	kVertTransition7, kNullPaletteCopy
};

enum InputMode {
	kInputBuildingSentences = 0,		// Normal sentence building
	kInputConversation = 1,			// Conversation mode
	kInputLimitedSentences = 2		// Use only scene hotspots
};

enum ThroughBlack {
	THROUGH_BLACK1 = 1,
	THROUGH_BLACK2 = 2
};

class SpriteSlot;
class TextDisplay;
class UISlot;

class DirtyArea {
private:
	static MADSEngine *_vm;
	friend class DirtyAreas;
public:
	Common::Rect _bounds;
	bool _textActive;
	bool _active;
	DirtyArea *_mergedArea;

	DirtyArea();

	void setArea(int width, int height, int maxWidth, int maxHeight);

	/**
	 * Set up a dirty area for a sprite slot
	 */
	void setSpriteSlot(const SpriteSlot *spriteSlot);

	/**
	* Set up a dirty area for a text display
	*/
	void setTextDisplay(const TextDisplay *textDisplay);

	/**
	 * Set up a dirty area for a UI slot
	 */
	void setUISlot(const UISlot *slot);
};

class DirtyAreas : public Common::Array<DirtyArea> {
//private:
//	MADSEngine *_vm;
public:
	DirtyAreas(MADSEngine *vm);

	/**
	* Merge together any designated dirty areas that overlap
	* @param startIndex	1-based starting dirty area starting index
	* @param count			Number of entries to process
	*/
	void merge(int startIndex, int count);

	bool intersects(int idx1, int idx2);
	void mergeAreas(int idx1, int idx2);

	/**
	* Copy the data specified by the dirty rect list between surfaces
	* @param srcSurface	Source surface
	* @param destSurface	Dest surface
	* @param posAdjust		Position adjustment
	*/
	void copy(BaseSurface *srcSurface, BaseSurface *destSurface, const Common::Point &posAdjust);

	/**
	* Use the lsit of dirty areas to copy areas of the screen surface to
	* the physical screen
	*/
	void copyToScreen();

	void reset();
};

class ScreenObject {
public:
	bool _active;
	Common::Rect _bounds;
	ScrCategory _category;
	int _descId;
	int _mode;

	ScreenObject();
};

class ScreenObjects : public Common::Array<ScreenObject> {
private:
	MADSEngine *_vm;
	int _objectY;

	int scanBackwards(const Common::Point &pt, int layer);
public:
	InputMode _inputMode;
	int _v7FED6;
	int _v8332A;
	int _forceRescan;
	int _selectedObject;
	ScrCategory _category;
	bool _released;
	int _uiCount;
	bool _eventFlag;
	uint32 _baseTime;
	int _spotId;

	/*
	* Constructor
	*/
	ScreenObjects(MADSEngine *vm);

	/**
	* Add a new item to the list
	*/
	ScreenObject *add(const Common::Rect &bounds, ScreenMode mode, ScrCategory category, int descId);

	/**
	 * Check objects on the screen
	 */
	void check(bool scanFlag);

	/**
	 * Scan the registered screen objects
	 */
	int scan(const Common::Point &pt, int layer);

	/**
	 * Handle an element being highlighted on the screen, and make it active.
	 */
	void elementHighlighted();

	/**
	 * Retrieve a ScreenObject from the list
	 * @remarks	This array is 1-based indexed by the game
	 */
	ScreenObject &operator[](int idx) {
		assert(idx > 0);
		return Common::Array<ScreenObject>::operator[](idx - 1);
	}

	/**
	 * Sets an item identified by category and Desc Id as active or not
	 * @param category		Screen category
	 * @param descId		Description for item
	 * @param active		Whether to set item as active or not
	 */
	void setActive(ScrCategory category, int descId, bool active);

	/**
	 * Synchronize the data
	 */
	void synchronize(Common::Serializer &s);
};

class Screen : public BaseSurface {
private:
	uint16 _random;
	MSurface _rawSurface;

	void panTransition(MSurface &newScreen, byte *palData, int entrySide,
		const Common::Point &srcPos, const Common::Point &destPos,
		ThroughBlack throughBlack, bool setPalette, int numTicks);

	void swapForeground(byte newPalette[PALETTE_SIZE], byte *paletteMap);

	void swapPalette(const byte palData[PALETTE_SIZE], byte swapTable[PALETTE_COUNT], bool foreground);
public:
	int _shakeCountdown;
public:
	/**
	 * Constructor
	 */
	Screen();

	/**
	 * Destructor
	 */
	~Screen() override {}

	/**
	 * Updates the physical screen with contents of the internal surface
	 */
	void update() override;

	/**
	 * Transition to a new screen with a given effect
	 */
	void transition(ScreenTransition transitionType, bool surfaceFlag);

	/**
	 * Set the screen drawing area to a sub-section of the real screen
	 */
	void setClipBounds(const Common::Rect &r);

	/**
	 * Reset back to drawing on the entirety of the screen
	 */
	void resetClipBounds();

	/**
	 * Return the current drawing/clip area
	 */
	const Common::Rect getClipBounds() const {
		const Common::Point pt = getOffsetFromOwner();
		return Common::Rect(pt.x, pt.y, pt.x + this->w, pt.y + this->h);
	}
};

} // End of namespace MADS

#endif /* MADS_SCREEN_H */
