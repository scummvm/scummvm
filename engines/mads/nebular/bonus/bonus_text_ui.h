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
 */

#ifndef MADS_NEBULAR_BONUS_TEXT_UI_H
#define MADS_NEBULAR_BONUS_TEXT_UI_H

#include "common/array.h"
#include "common/events.h"
#include "common/func.h"
#include "common/rect.h"
#include "common/str.h"
#include "graphics/surface.h"
#include "mads/nebular/bonus/dos_text_screen.h"
#include "mads/nebular/bonus/bonus_exe_data.h"

namespace MADS {
namespace RexNebular {

class BonusTextUI {
public:
	enum MainChoice {
		kDeathScenes = 0,
		kEvolution,
		kSets,
		kMusic,
		kCoolStuff,
		kExit,
		kAbort = -1
	};

	explicit BonusTextUI(const BonusExeData &text);
	~BonusTextUI();

	bool init(Common::String &errorMessage);
	MainChoice runMainMenu(int &selected);
	int runMusicMenu(int &selected, const bool *enabled);
	bool showBonusText(const Common::Path &filename);
	void prepareNowPlaying(const Common::String &trackTitle);
	void waitForNowPlaying(const Common::String &trackTitle,
			Common::Functor0<bool> &isPlaying);
	void showGoodbye();

private:
	const BonusExeData &_text;
	DOSTextScreen _cells;
	Graphics::Surface _surface;
	Common::Point _mouseCell;
	bool _mouseCellValid;
	bool _restoreSystemCursor;

	const Common::Rect _titleRect;
	const Common::Rect _mainRect;
	const Common::Rect _musicRect;
	const Common::Rect _viewerRect;
	const Common::Rect _nowPlayingRect;

	void drawDesktop();
	void drawTitlePanel();
	void drawMenu(const Common::Rect &rect, const Common::String &title,
			const Common::String *items, int itemCount, int selected,
			const bool *enabled);
	void drawMouseCursor();
	void present(bool forcePalette = false);
	void restorePresentation();
	void updateMouseCell(const Common::Point &position);

	int runMenu(const Common::Rect &rect, const Common::String &title,
			const Common::String *items, int itemCount, int &selected,
			const bool *enabled = nullptr, bool showPCSpeakerNotice = false);
	static int nextEnabled(const bool *enabled, int itemCount,
			int selected, int direction);
	static int itemRow(const Common::Rect &rect, int itemCount, int index);
	static int rowAtMouse(const Common::Rect &rect, int itemCount,
			int mouseX, int mouseY);
	static int acceleratorChoice(const Common::String *items, int itemCount,
			int ascii);
	static void appendWrappedLine(const Common::String &source, int width,
			Common::Array<Common::String> &lines);
	bool processQuitEvent(const Common::Event &event);
	bool processGameMenuEvent(const Common::Event &event);
};

} // namespace RexNebular
} // namespace MADS

#endif // MADS_NEBULAR_BONUS_TEXT_UI_H
