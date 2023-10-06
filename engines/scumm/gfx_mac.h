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

#ifndef SCUMM_GFX_MAC_H
#define SCUMM_GFX_MAC_H

class OSystem;

namespace Graphics {
class Surface;
class Font;
}

namespace Scumm {

class ScummEngine;

class MacIndy3Gui {
public:
	enum Color {
		kBlack = 0,
		kBlue = 1,
		kGreen = 2,
		kCyan = 3,
		kRed = 4,
		kMagenta = 5,
		kBrown = 6,
		kLightGray = 7,
		kDarkGray = 8,
		kBrightBlue = 9,
		kBrightGreen = 10,
		kBrightCyan = 11,
		kBrightRed = 12,
		kBrightMagenta = 13,
		kBrightYellow = 14,
		kWhite = 15,
		kBackground = 254,	// Gray or checkerboard
		kTransparency = 255
	};

	enum FontId {
		kRegular = 0,
		kBold = 1,
		kOutline = 2
	};

	enum ScrollDirection {
		kScrollUp,
		kScrollDown
	};

	MacIndy3Gui(OSystem *system, ScummEngine *vm);
	~MacIndy3Gui();

	// There is a distinction between the GUI being active and being
	// visible. Active means that it's allowed to draw verbs. Visible
	// means that it's actually drawn verbs. From the outside, only the
	// visibility is relevant.
	//
	// One case where this makes a difference is when boxing with the
	// coach. During the "10 minutes later" sign, the GUI is active but
	// it's not drawing verbs, so the SCUMM engine is allowed to draw in
	// the verb area to clear the power meters and text.

	bool isVisible();
	const Graphics::Font *getFont(int n) { return _fonts[n]; }

	void resetAfterLoad();
	void update(int delta);
	void handleEvent(Common::Event &event);

	int getInventoryScrollOffset();
	void setInventoryScrollOffset(int n);

private:
	OSystem *_system = nullptr;
	ScummEngine *_vm = nullptr;
	Graphics::Surface *_surface = nullptr;
	const Graphics::Font *_fonts[3];

	bool _visible = false;

	bool _leftButtonIsPressed = false;
	Common::Point _leftButtonPressed;
	Common::Point _leftButtonHeld;

	int _timer = 0;

	class Widget {
	private:
		int _timer = 0;

	protected:
		bool _redraw = false;
		bool _enabled = false;

	public:
		static ScummEngine *_vm;
		static MacIndy3Gui *_gui;
		static Graphics::Surface *_surface;

		Common::Rect _bounds;

		Widget(int x, int y, int width, int height);
		virtual ~Widget() {}

		void setEnabled(bool b) { _enabled = b; }

		void setTimer(int t) { _timer = t; }
		void clearTimer() { _timer = 0; }
		bool hasTimer() { return _timer > 0; }

		virtual void setRedraw(bool redraw) { _redraw = redraw; }

		virtual void reset();

		virtual bool handleEvent(Common::Event &event) = 0;
		virtual bool handleMouseHeld(Common::Point &pressed, Common::Point &held) { return false; }
		virtual bool updateTimer(int delta);

		virtual void draw() = 0;
		virtual void undraw() {}

		// Primitives
		void copyRectToScreen(Common::Rect r);
		void fill(Common::Rect r);
		void drawBitmap(Common::Rect r, const uint16 *bitmap, Color color);
		void drawShadowBox(Common::Rect r);
		void drawShadowFrame(Common::Rect r, Color shadowColor, Color fillColor);
	};

	class VerbWidget : public Widget {
	protected:
		bool _visible = false;
		int _verbid = 0;
		int _verbslot = -1;
		bool _kill = false;

	public:

		VerbWidget(int x, int y, int width, int height) : Widget(x, y, width, height) {}

		void setVerbid(int n) { _verbid = n; }
		bool hasVerb() { return _verbslot != -1; }
		bool isVisible() { return _visible; }
		void threaten() { _kill = true; }
		bool isDying() { return _kill; }

		void reset();

		virtual void updateVerb(int verbslot);

		void draw();
		void undraw();
	};

	class Button : public VerbWidget {
	private:
		Common::String _text;

	public:

		Button(int x, int y, int width, int height) : VerbWidget(x, y, width, height) {}

		bool handleEvent(Common::Event &event);

		void reset();
		bool updateTimer(int delta);
		void updateVerb(int verbslot);

		void draw();
	};

	class Inventory : public VerbWidget {
	private:
		class ScrollBar : public Widget {
		private:
			int _invCount = 0;
			int _invOffset = 0;

		public:
			ScrollBar(int x, int y, int width, int height);

			void setCounters(int invCount, int invOffset);
			void moveInvOffset(int offset);
			int getHandlePosition();

			void reset();

			bool handleEvent(Common::Event &event);

			void draw();
		};

		class ScrollButton : public Widget {
		private:
			static const uint16 _upArrow[16];
			static const uint16 _downArrow[16];

		public:
			ScrollDirection _direction;

			ScrollButton(int x, int y, int width, int height, ScrollDirection direction);

			bool handleEvent(Common::Event &event);
			bool handleMouseHeld(Common::Point &pressed, Common::Point &held);

			void draw();
		};

		class Slot : public Widget {
		private:
			Common::String _name;
			int _slot = -1;
			int _obj = -1;

		public:
			Slot(int slot, int x, int y, int width, int height);

			void clearName() { _name.clear(); }
			bool hasName() { return !_name.empty(); }

			void clearObject() { _obj = -1; }

			void setObject(int n);
			int getObject() { return _obj; }

			void reset();

			bool handleEvent(Common::Event &event);
			bool updateTimer(int delta);

			void draw();
		};

		Slot *_slots[6];
		ScrollBar *_scrollBar;
		ScrollButton *_scrollButtons[2];

		static const uint16 _upArrow[16];
		static const uint16 _downArrow[16];

	public:
		Inventory(int x, int y, int width, int height);
		~Inventory();

		void setRedraw(bool redraw);

		void reset();

		bool handleEvent(Common::Event &event);
		bool handleMouseHeld(Common::Point &pressed, Common::Point &held);
		bool updateTimer(int delta);
		void updateVerb(int verbslot);

		void draw();
	};

	Common::HashMap<int, VerbWidget *> _widgets;

	const uint16 _ulCorner[4] = { 0xF000, 0xC000, 0x8000, 0x8000 };
	const uint16 _urCorner[4] = { 0xF000, 0x3000, 0x1000, 0x1000 };
	const uint16 _llCorner[4] = { 0x8000, 0x8000, 0xC000, 0xF000 };
	const uint16 _lrCorner[5] = { 0x1000, 0x1000, 0x3000, 0xF000 };

	bool isActive();

	void clear();
	void show();
	void hide();

	void copyRectToScreen(Common::Rect r);
	void fill(Common::Rect r);
	void drawBitmap(Common::Rect r, const uint16 *bitmap, Color color);
};

} // End of namespace Scumm

#endif
