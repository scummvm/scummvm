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

#ifndef MACS2_VIEW1_H
#define MACS2_VIEW1_H


#include "macs2/events.h"
#include "macs2/macs2.h"

namespace Macs2 {

	class GameObject;

	class Character {
private:

	Common::Point StartPosition;
	Common::Point EndPosition;

	uint32 StartTime;
	uint32 Duration;

	bool IsLerping = false;


	public:
	Common::Point Position;
		Macs2::GameObject *GameObject;
		// TODO: Will need time handling
	Macs2::AnimFrame *GetCurrentAnimationFrame();
		void StartLerpTo(const Common::Point &target, uint32 duration);
		void Update();
	};

class View1 : public UIElement {
private:

	Common::Array<Character *> characters;
	byte _pal[256 * 3] = { 0 };
	int _offset = 0;

	uint32 _frameDelayFlag = 100;
	int32 _nextFrameFlag = _frameDelayFlag;
	uint32 _lastMillis = 0;
	bool _hasTicked = false;
	uint32 _flagFrameIndex = 0;

	uint32 _guyFrameIndex = 0;

	// TODO: Probably the start of a mode enum
	bool _isShowingStringBox = false;
	Common::StringArray _drawnStringBox;

	bool _isShowingInventory = false;


	Graphics::ManagedSurface _backgroundSurface;

	void drawDarkRectangle(uint16 x, uint16 y, uint16 width, uint16 height);

	void drawStringBackground(uint16 x, uint16 y, uint16 width, uint16 height);

	void drawBackgroundAnimations(Graphics::ManagedSurface &s);
	void drawBackgroundAnimationNumbers(Graphics::ManagedSurface &s);

	void renderString(uint16 x, uint16 y, Common::String s);

	void showStringBox(const Common::StringArray &sa);

	void drawGlyphs(Macs2::GlyphData *data, int count, uint16 x, uint16 y, Graphics::ManagedSurface &s);

	void handleFading();

	void drawPathfindingPoints(Graphics::ManagedSurface &s);

	void drawPath(Graphics::ManagedSurface &s);

	int currentFadeValue = -1;
	int fadeDelta = 4;



public:
	View1();
	virtual ~View1() {}

	bool msgFocus(const FocusMessage &msg) override;
	bool msgKeypress(const KeypressMessage &msg) override;
	bool msgMouseDown(const MouseDownMessage& msg) override;
	void draw() override;
	bool tick() override;

	void drawInventory(Graphics::ManagedSurface &s);

	void setStringBox(const Common::StringArray& sa);
	void clearStringBox();

	void startFading();

	void DrawSprite(uint16 x, uint16 y, uint16 width, uint16 height, byte* data, Graphics::ManagedSurface& s);
	void DrawSprite(const Common::Point &pos, uint16 width, uint16 height, byte *data, Graphics::ManagedSurface &s);
	void DrawSpriteClipped(uint16 x, uint16 y, Common::Rect &clippingRect, uint16 width, uint16 height, byte *data, Graphics::ManagedSurface &s);
	void DrawSpriteAdvanced(uint16 x, uint16 y, uint16 width, uint16 height, uint16 scaling, byte *data, Graphics::ManagedSurface &s);

	void DrawCharacters(Graphics::ManagedSurface &s);
};

} // namespace Macs2

#endif
