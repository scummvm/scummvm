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

	bool isVisible() { return _visible; }

	const Graphics::Font *getFont(int n) { return _fonts[n]; }

	void resetAfterLoad();
	void update(int delta);
	void handleEvent(Common::Event &event);

private:
	OSystem *_system = nullptr;
	ScummEngine *_vm = nullptr;
	Graphics::Surface *_macScreen = nullptr;
	const Graphics::Font *_fonts[3];

	bool _visible = false;

	class Widget {
	public:
		static ScummEngine *_vm;
		static MacIndy3Gui *_gui;
		static Graphics::Surface *_surface;

		Common::Rect _bounds;
		int _timer = 0;
		bool _enabled = false;
		bool _redraw = false;

		Widget(int x, int y, int width, int height);
		virtual ~Widget() {}

		void reset();

		virtual void draw() = 0;
		virtual void undraw() {}
		virtual bool updateTimer(int delta);

		// Primitives
		void copyRectToScreen(Common::Rect r);
		void fill(Common::Rect r);
		void drawBitmap(Common::Rect r, const uint16 *bitmap, byte color);
		void drawShadowBox(Common::Rect r);
		void drawShadowFrame(Common::Rect r, byte shadowColor, byte fillColor);
	};

	class VerbWidget : public Widget {
	public:
		int _verbid = 0;
		int _verbslot = -1;
		bool _visible = false;
		bool _kill = false;

		VerbWidget(int x, int y, int width, int height) : Widget(x, y, width, height) {}

		void reset();
		void threaten() { _kill = true; }

		virtual bool handleEvent(Common::Event &event) = 0;
		virtual void updateVerb(int verbslot);

		void draw();
		void undraw();
	};

	class Button : public VerbWidget {
	public:
		Common::String _text;

		Button(int x, int y, int width, int height) : VerbWidget(x, y, width, height) {}

		bool handleEvent(Common::Event &event);

		void reset();
		bool updateTimer(int delta);
		void updateVerb(int verbslot);
		void draw();
	};

	enum ScrollDirection {
		kScrollUp,
		kScrollDown
	};

	class Inventory : public VerbWidget {
	private:
		class ScrollButton : public Widget {
		private:
			static const uint16 _upArrow[16];
			static const uint16 _downArrow[16];

		public:
			ScrollDirection _direction;

			ScrollButton(int x, int y, int width, int height, ScrollDirection direction);

			bool handleEvent(Common::Event &event);
			void draw();
		};

		class Slot : public Widget {
		private:
			int _slot = -1;

		public:
			int _obj = -1;
			Common::String _name;

			Slot(int slot, int x, int y, int width, int height);
			void reset();
			bool handleEvent(Common::Event &event);
			bool updateTimer(int delta);
			void draw();
		};

		Slot *_slots[6];
		ScrollButton *_scrollButtons[2];

		static const uint16 _upArrow[16];
		static const uint16 _downArrow[16];

	public:
		Inventory(int x, int y, int width, int height);
		~Inventory();

		void updateVerb(int verbslot);

		bool handleEvent(Common::Event &event);
		bool updateTimer(int delta);

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
	void drawBitmap(Common::Rect r, const uint16 *bitmap, byte color);
};

} // End of namespace Scumm

#endif
