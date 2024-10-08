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

#ifndef SCUMM_MACGUI_MACGUI_INDY3_H
#define SCUMM_MACGUI_MACGUI_INDY3_H

#include "common/events.h"
#include "common/rect.h"
#include "common/str.h"

#include "graphics/surface.h"

namespace Scumm {

class MacGuiImpl;

class MacIndy3Gui : public MacGuiImpl {
public:
	enum ScrollDirection {
		kScrollUp,
		kScrollDown
	};

	MacIndy3Gui(ScummEngine *vm, const Common::Path &resourceFile);
	~MacIndy3Gui();

	const Common::String name() const override { return "Indy"; }

	Graphics::Surface _textArea;

	const Graphics::Font *getFontByScummId(int32 id) override;

	void setupCursor(int &width, int &height, int &hotspotX, int &hotspotY, int &animate) override;

	Graphics::Surface *textArea() override { return &_textArea; }
	void clearTextArea() override { _textArea.fillRect(Common::Rect(_textArea.w, _textArea.h), kBlack); }
	void initTextAreaForActor(Actor *a, byte color) override;
	void printCharToTextArea(int chr, int x, int y, int color) override;

	// There is a distinction between the GUI being allowed and being
	// active. Allowed means that it's allowed to draw verbs, but not that
	// it necessarily is. Active means that there are verbs on screen. From
	// the outside, only the latter is relevant.
	//
	// One case where this makes a difference is when boxing with the
	// coach. During the "10 minutes later" sign, the GUI is active but
	// it's not drawing verbs, so the SCUMM engine is allowed to draw in
	// the verb area to clear the power meters and text.

	bool isVerbGuiActive() const override;

	void reset() override;
	void resetAfterLoad() override;
	void update(int delta) override;
	bool handleEvent(Common::Event event) override;

protected:
	bool getFontParams(FontId fontId, int &id, int &size, int &slant) const override;

	bool handleMenu(int id, Common::String &name) override;

	void runAboutDialog() override;
	bool runOpenDialog(int &saveSlotToHandle) override;
	bool runSaveDialog(int &saveSlotToHandle, Common::String &name) override;
	bool runOptionsDialog() override;
	bool runIqPointsDialog();

	void readStrings() override;

private:
	int _verbGuiTop = 0;
	Graphics::Surface _verbGuiSurface;

	bool _visible = false;

	bool _leftButtonIsPressed = false;
	Common::Point _leftButtonPressed;
	Common::Point _leftButtonHeld;

	int _timer = 0;

	bool updateVerbs(int delta);
	void updateMouseHeldTimer(int delta);
	void drawVerbs();

	void clearAboutDialog(MacDialogWindow *window);

	int getInventoryScrollOffset() const;
	void setInventoryScrollOffset(int n) const;

	class Widget : public MacGuiObject {
	private:
		int _timer = 0;

	public:
		static ScummEngine *_vm;
		static MacIndy3Gui *_gui;
		static Graphics::Surface *_surface;

		Widget(int x, int y, int width, int height);
		virtual ~Widget() {}

		void setEnabled(bool enabled) {
			if (enabled != _enabled)
				setRedraw(true);
			if (!_enabled)
				_timer = 0;
			_enabled = enabled;
		}

		void setTimer(int t) { _timer = t; }
		void clearTimer() { _timer = 0; }
		bool hasTimer() const { return _timer > 0; }

		virtual void setRedraw(bool redraw) { _redraw = redraw; }

		virtual void reset();

		virtual bool handleEvent(Common::Event &event) = 0;
		virtual bool handleMouseHeld(Common::Point &pressed, Common::Point &held) { return false; }
		virtual void updateTimer(int delta);
		virtual void timeOut() {}

		virtual void draw();
		virtual void undraw();

		byte translateChar(byte c) const;

		// Primitives
		void fill(Common::Rect r);
		void drawBitmap(Common::Rect r, const uint16 *bitmap, Color color) const;
		void drawShadowBox(Common::Rect r) const;
		void drawShadowFrame(Common::Rect r, Color shadowColor, Color fillColor);

		void markScreenAsDirty(Common::Rect r) const;
	};

	class VerbWidget : public Widget {
	protected:
		int _verbid = 0;
		int _verbslot = -1;
		bool _kill = false;

	public:
		VerbWidget(int x, int y, int width, int height) : Widget(x, y, width, height) {}

		void setVerbid(int n) { _verbid = n; }
		bool hasVerb() const { return _verbslot != -1; }
		void threaten() { _kill = true; }
		bool isDying() const { return _kill; }

		void reset();

		virtual void updateVerb(int verbslot);

		void draw();
		void undraw();
	};

	class Button : public VerbWidget {
	private:
		Common::String _text;

	public:
		Button(int x, int y, int width, int height);

		bool handleEvent(Common::Event &event);

		void reset();
		void timeOut();
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

			void setInventoryParameters(int invCount, int invOffset);
			void scroll(ScrollDirection dir);
			int getHandlePosition();

			void reset();

			bool handleEvent(Common::Event &event);

			void draw();
		};

		class ScrollButton : public Widget {
		public:
			ScrollDirection _direction;

			ScrollButton(int x, int y, int width, int height, ScrollDirection direction);

			bool handleEvent(Common::Event &event);
			bool handleMouseHeld(Common::Point &pressed, Common::Point &held);
			void timeOut();

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
			bool hasName() const { return !_name.empty(); }

			void clearObject();
			void setObject(int n);
			int getObject() const { return _obj; }

			void reset();

			bool handleEvent(Common::Event &event);
			void timeOut();

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
		void updateTimer(int delta);
		void updateVerb(int verbslot);

		void draw();
	};

	Common::HashMap<int, VerbWidget *> _widgets;
	Common::Array<Common::Rect> _dirtyRects;

	bool isVerbGuiAllowed() const;

	void show();
	void hide();

	void fill(Common::Rect r);

	void markScreenAsDirty(Common::Rect r);
	void copyDirtyRectsToScreen();
};

} // End of namespace Scumm
#endif
