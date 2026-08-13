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

#include "engines/util.h"
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/events.h"
#include "common/system.h"
#include "graphics/font.h"
#include "graphics/macgamma.h"
#include "graphics/managed_surface.h"
#include "graphics/paletteman.h"
#include "graphics/surface.h"
#include "mads/core/buffer.h"
#include "mads/core/env.h"
#include "mads/core/font.h"
#include "mads/core/inter.h"
#include "mads/core/kernel.h"
#include "mads/core/magic.h"
#include "mads/core/mcga.h"
#include "mads/core/mem.h"
#include "mads/core/object.h"
#include "mads/core/pal.h"
#include "mads/core/room.h"
#include "mads/core/screen.h"
#include "mads/nebular/extra.h"
#include "mads/nebular/mac_menus.h"
#include "mads/nebular/mac_nebular.h"
#include "mads/nebular/mac_resources.h"
#include "mads/nebular/sound/mac_sound.h"
#include "mads/nebular/nebular.h"
#include "mads/nebular/popup.h"
#include "mads/nebular/mads/words.h"

namespace MADS {
namespace RexNebular {

enum {
	kMacScreenWidth = 640,
	kMacLogicalSceneWidth = 320,
	kMacLogicalSceneHeight = 156,
	kMacLargeSceneHeight = 312,
	kMacInterfaceWidth = 512,
	kMacInterfaceHeight = 88,
	kMacInterfaceX = (kMacScreenWidth - kMacInterfaceWidth) / 2,
	kMacLegacyScreenHeight = kMacLargeSceneHeight + kMacInterfaceHeight,
	kMacDesktopHeight = 480,
	kMacMenuBarHeight = 20,
	kMacDesktopSceneY = kMacMenuBarHeight + 20,
	kMacFullFrameHeight = 400,
	kMacDesktopSeparatorY = kMacDesktopSceneY + kMacLargeSceneHeight,
	kMacDesktopInterfaceY = kMacDesktopSeparatorY + 1,
	kMacPanelPaletteColors = 10,
	kMacScenePaletteStart = 16,
	kMacPopupGray = 39,
	kMacPopupColor = 12,
	kMacNormalTextColor = 15,
	kMacLeftSelectColor = 13,
	kMacRightSelectColor = 14,
	kMacBlackColor = kMacRightSelectColor
};

struct MacPopupLine {
	Common::String text;
	word tab;

	MacPopupLine(const Common::String &lineText, word lineTab) :
			text(lineText), tab(lineTab) {}
};

static void appendWrappedMacPopupText(const Graphics::Font &font,
		const Common::String &text, word tab, int width,
		Common::Array<MacPopupLine> &lines) {
	Common::Array<Common::String> wrapped;
	font.wordWrapText(text, width, wrapped);
	for (uint i = 0; i < wrapped.size(); ++i)
		lines.push_back(MacPopupLine(wrapped[i], tab));
}

static byte macGammaCorrect(byte color) {
	return Graphics::macGammaCorrectionLookUp[color << 2];
}

static byte macPaletteComponentToSixBit(byte color) {
	return (color * 63 + 127) / 255;
}

static bool loadMacAboutRoom(Graphics::ManagedSurface &picture,
		Palette &palette) {
	Palette savedMasterPalette;
	dword savedColorStatus[256];
	int savedFlagUsed[PAL_MAXFLAGS];
	const int savedPaletteLocked = palette_locked;
	const int savedLowSearchLimit = palette_low_search_limit;
	const int savedHighSearchLimit = palette_high_search_limit;
	const int savedManagerActive = pal_manager_active;
	const int savedManagerColors = pal_manager_colors;
	void (*savedManagerUpdate)() = pal_manager_update;
	ShadowListPtr savedMasterShadow = master_shadow;
	const int savedRoomLoadError = room_load_error;
	const byte savedRoomLoadedDepth = room_loaded_depth;

	memcpy(savedMasterPalette, master_palette, sizeof(Palette));
	memcpy(savedColorStatus, color_status, sizeof(color_status));
	memcpy(savedFlagUsed, flag_used, sizeof(flag_used));

	for (int color = 0; color < 256; ++color)
		color_status[color] &= PAL_RESERVED;
	for (int flag = 2; flag < PAL_MAXFLAGS; ++flag)
		flag_used[flag] = false;
	palette_locked = false;
	palette_low_search_limit = 0;
	palette_high_search_limit = 256;
	pal_manager_active = false;
	pal_manager_colors = 0;
	pal_manager_update = nullptr;
	master_shadow = nullptr;

	Buffer roomPicture = {};
	Buffer roomDepth = {};
	RoomPtr aboutRoom = RexNebular::room_load(990, 0, nullptr,
		&roomPicture, &roomDepth, nullptr, nullptr, nullptr, nullptr,
		nullptr, nullptr, -1, -1, 0);
	const bool loaded = aboutRoom && roomPicture.data &&
		roomPicture.x == kMacLogicalSceneWidth &&
		roomPicture.y == kMacLogicalSceneHeight;
	if (loaded) {
		picture.create(roomPicture.x, roomPicture.y,
			Graphics::PixelFormat::createFormatCLUT8());
		for (int y = 0; y < roomPicture.y; ++y)
			memcpy(picture.getBasePtr(0, y),
				roomPicture.data + y * roomPicture.x, roomPicture.x);
		memcpy(palette, master_palette, sizeof(Palette));
	}

	if (aboutRoom) {
		pal_deallocate(aboutRoom->color_handle);
		mem_free(aboutRoom);
	}
	buffer_free(&roomPicture);
	buffer_free(&roomDepth);

	memcpy(master_palette, savedMasterPalette, sizeof(Palette));
	memcpy(color_status, savedColorStatus, sizeof(color_status));
	memcpy(flag_used, savedFlagUsed, sizeof(flag_used));
	palette_locked = savedPaletteLocked;
	palette_low_search_limit = savedLowSearchLimit;
	palette_high_search_limit = savedHighSearchLimit;
	pal_manager_active = savedManagerActive;
	pal_manager_colors = savedManagerColors;
	pal_manager_update = savedManagerUpdate;
	master_shadow = savedMasterShadow;
	room_load_error = savedRoomLoadError;
	room_loaded_depth = savedRoomLoadedDepth;
	return loaded;
}

static void drawMacAboutText(Graphics::ManagedSurface &surface,
		const Graphics::Font &font, const Common::String &text,
		int x, int baseline, byte color) {
	font.drawString(&surface, text, x, baseline - font.getFontAscent(),
		surface.w - x, color);
}

static void drawMacAboutRoom(Graphics::ManagedSurface &output,
		const Graphics::ManagedSurface &picture, int sceneX, int sceneY,
		int sceneWidth, int sceneHeight, byte backgroundColor) {
	output.fillRect(output.getBounds(), backgroundColor);
	for (int y = 0; y < sceneHeight; ++y) {
		const byte *source = (const byte *)picture.getBasePtr(
			0, y * picture.h / sceneHeight);
		byte *target = (byte *)output.getBasePtr(sceneX, sceneY + y);
		for (int x = 0; x < sceneWidth; ++x)
			target[x] = source[x * picture.w / sceneWidth];
	}
}

static void setMacInterfacePalette(const MacResourceProvider *resources) {
	if (resources && resources->getNativeInterface()) {
		const byte *panelPalette = resources->getNativeInterfacePalette();
		// Keep MADS' six-bit palette state synchronized, but install the exact
		// gamma-corrected colors in the backend.
		for (int color = 0; color < kMacPanelPaletteColors; ++color) {
			master_palette[color + 2].r =
				macPaletteComponentToSixBit(panelPalette[color * 3]);
			master_palette[color + 2].g =
				macPaletteComponentToSixBit(panelPalette[color * 3 + 1]);
			master_palette[color + 2].b =
				macPaletteComponentToSixBit(panelPalette[color * 3 + 2]);
		}
		g_system->getPaletteManager()->setPalette(panelPalette, 2,
			kMacPanelPaletteColors);
	}

	const byte popupGray = macGammaCorrect(kMacPopupGray);
	byte popupPalette[3] = { popupGray, popupGray, popupGray };
	master_palette[kMacPopupColor].r =
		macPaletteComponentToSixBit(popupGray);
	master_palette[kMacPopupColor].g =
		macPaletteComponentToSixBit(popupGray);
	master_palette[kMacPopupColor].b =
		macPaletteComponentToSixBit(popupGray);
	g_system->getPaletteManager()->setPalette(popupPalette,
		kMacPopupColor, 1);

	byte normalR = 25;
	byte normalG = 48;
	byte normalB = 51;
	switch (section_id) {
	case 2:
	case 3:
		normalR = 10;
		normalG = 43;
		normalB = 10;
		break;
	case 5:
		normalR = 41;
		normalG = 45;
		normalB = 53;
		break;
	case 6:
	case 7:
		// Section 8 shares section 7's interface behavior in the port.
	case 8:
		normalR = 42;
		normalG = 47;
		normalB = 54;
		break;
	default:
		break;
	}

	byte semanticPalette[3 * 3] = {
		macGammaCorrect(63), macGammaCorrect(63), macGammaCorrect(63),
		macGammaCorrect(0), macGammaCorrect(0), macGammaCorrect(0),
		macGammaCorrect(normalR), macGammaCorrect(normalG),
		macGammaCorrect(normalB)
	};
	for (int color = 0; color < 3; ++color) {
		master_palette[color + kMacLeftSelectColor].r =
			macPaletteComponentToSixBit(semanticPalette[color * 3]);
		master_palette[color + kMacLeftSelectColor].g =
			macPaletteComponentToSixBit(semanticPalette[color * 3 + 1]);
		master_palette[color + kMacLeftSelectColor].b =
			macPaletteComponentToSixBit(semanticPalette[color * 3 + 2]);
	}
	g_system->getPaletteManager()->setPalette(semanticPalette,
		kMacLeftSelectColor, 3);
}

static bool getMacInterfaceSpot(int class_, int id, Common::Rect &rect) {
	int row = 0;
	int col = 0;
	int baseX = 0;
	int baseY = 0;
	int deltaX = 0;

	switch (class_) {
	case STROKE_COMMAND:
		row = id % inter_columns;
		col = id / inter_columns;
		baseX = command_base_x;
		baseY = command_base_y;
		deltaX = command_delta_x;
		break;
	case STROKE_INVEN:
		if (id < first_inven || id >= first_inven + inter_columns)
			return false;
		row = id - first_inven;
		baseX = inven_base_x;
		baseY = inven_base_y;
		deltaX = inven_delta_x;
		break;
	case STROKE_DIALOG:
		row = id;
		baseX = command_base_x;
		baseY = command_base_y;
		deltaX = dialog_delta_x;
		break;
	case STROKE_SCROLL:
		baseX = inter_scroll_x1 + (id == SCROLL_THUMB ? 2 : 0);
		deltaX = inter_scroll_x2 - inter_scroll_x1 + 1;
		switch (id) {
		case SCROLL_UP:
			baseY = inter_up_y1;
			row = 0;
			break;
		case SCROLL_DOWN:
			baseY = inter_down_y1;
			row = 0;
			break;
		case SCROLL_ELEVATOR:
			baseY = inter_elevator_y1;
			row = 0;
			break;
		case SCROLL_THUMB:
			baseY = inter_thumb_y1 + scrollbar_elevator;
			row = 0;
			break;
		default:
			return false;
		}
		break;
	case STROKE_ACTION:
		row = id;
		baseX = action_base_x;
		baseY = action_base_y;
		deltaX = action_delta_x;
		break;
	default:
		return false;
	}

	int height = inter_delta_y;
	if (class_ == STROKE_SCROLL) {
		switch (id) {
		case SCROLL_UP:
			height = inter_up_y2 - inter_up_y1 + 1;
			break;
		case SCROLL_DOWN:
			height = inter_down_y2 - inter_down_y1 + 1;
			break;
		case SCROLL_ELEVATOR:
			height = inter_elevator_y2 - inter_elevator_y1 + 1;
			break;
		case SCROLL_THUMB:
			height = 1;
			break;
		}
	}

	const int x = baseX + col * deltaX;
	const int y = baseY + row * inter_delta_y;
	rect = Common::Rect(x, y, x + deltaX, y + height);
	return true;
}

static int macInterfaceX(int logicalX) {
	return logicalX * kMacInterfaceWidth / 320;
}

static int macInterfaceY(int logicalY) {
	return logicalY * 2;
}

static byte getMacInterfaceTextColor(int class_, int id) {
	if ((class_ == STROKE_COMMAND && id == left_command) ||
			(class_ == STROKE_INVEN && id == left_inven) ||
			(class_ == STROKE_ACTION && id == left_action) ||
			(class_ == STROKE_DIALOG && id == left_command))
		return kMacLeftSelectColor;
	if ((class_ == STROKE_INVEN && id == active_inven) ||
			(class_ == STROKE_ACTION && id == right_action))
		return kMacRightSelectColor;
	return kMacNormalTextColor;
}

static void drawMacInterfaceText(Graphics::ManagedSurface &panel,
		const Graphics::Font &font, int class_, int id,
		const Common::String &text) {
	Common::Rect logical;
	if (text.empty() || !getMacInterfaceSpot(class_, id, logical))
		return;

	// CODE 7 scales the logical x coordinate from 320 to 512 pixels and
	// converts the logical y coordinate into a QuickDraw baseline with
	// y * 2 + 11. Reproduce that transform instead of centering the font in
	// the compatibility rectangle.
	const int left = macInterfaceX(logical.left);
	const int right = macInterfaceX(logical.right);
	const int width = MAX(0, right - left);
	const int y = macInterfaceY(logical.top) + 11 - font.getFontAscent();
	if (width > 0 && y < kMacInterfaceHeight)
		font.drawString(&panel, text, left, y, width,
			getMacInterfaceTextColor(class_, id));
}

static Common::String getMacInterfaceWord(int wordId) {
	char text[80] = { 0 };
	Common::strcpy_s(text, vocab_string(wordId));
	if (text[0])
		text[0] = (char)toupper((byte)text[0]);
	return Common::String(text);
}

static void drawMacScrollbarArrow(Graphics::ManagedSurface &panel,
		const int *x, const int *y, int count, const Common::Rect &bounds,
		byte color) {
	panel.drawPolygonScan(x, y, count, bounds, color);
	for (int i = 0; i < count; ++i)
		panel.drawLine(x[i], y[i], x[(i + 1) % count],
			y[(i + 1) % count], color);
}

static void drawMacInterfaceScrollbar(Graphics::ManagedSurface &panel) {
	const Common::Rect scrollbar(120, 2, 136, 86);
	const Common::Rect up(120, 2, 136, 18);
	const Common::Rect down(120, 70, 136, 86);
	panel.frameRect(scrollbar, kMacNormalTextColor);
	panel.frameRect(up, kMacNormalTextColor);
	panel.frameRect(down, kMacNormalTextColor);

	if (inven_num_objects > 5) {
		// CODE 7 records these QuickDraw line paths as regions, then paints
		// them. The coordinates are native 512x88 panel coordinates.
		const int upX[] = { 122, 127, 128, 133, 128, 128, 127, 127 };
		const int upY[] = { 9, 4, 4, 9, 9, 15, 15, 9 };
		const int downX[] = { 122, 127, 127, 128, 128, 133, 128, 127 };
		const int downY[] = { 79, 79, 73, 73, 79, 79, 84, 84 };
		drawMacScrollbarArrow(panel, upX, upY, ARRAYSIZE(upX),
			Common::Rect(122, 4, 134, 16),
			scrollbar_active == SCROLL_UP ? kMacLeftSelectColor :
			kMacNormalTextColor);
		drawMacScrollbarArrow(panel, downX, downY, ARRAYSIZE(downX),
			Common::Rect(122, 73, 134, 85),
			scrollbar_active == SCROLL_DOWN ? kMacLeftSelectColor :
			kMacNormalTextColor);

		const int lastPage = inven_num_objects - 5;
		const int visibleFirst = MIN(first_inven, lastPage);
		const int thumbTop = 18 +
			(visibleFirst * 48) / lastPage;
		panel.fillRect(Common::Rect(122, thumbTop, 134, thumbTop + 4), 4);
		panel.frameRect(Common::Rect(121, thumbTop, 135, thumbTop + 4),
			scrollbar_active == SCROLL_ELEVATOR ? kMacLeftSelectColor :
			kMacNormalTextColor);
	}
}

static void drawMacInterfaceState(Graphics::ManagedSurface &panel,
		const Graphics::Font &font) {
	if (inter_input_mode == INTER_BUILDING_SENTENCES ||
			inter_input_mode == INTER_LIMITED_SENTENCES) {
		for (int id = 0; id < INTER_COMMANDS; ++id)
			drawMacInterfaceText(panel, font, STROKE_COMMAND, id,
				getMacInterfaceWord(command[id].id));
		for (int id = first_inven; id < first_inven + inter_columns &&
				id < inven_num_objects; ++id) {
			drawMacInterfaceText(panel, font, STROKE_INVEN, id,
				getMacInterfaceWord(object[inven[id]].vocab_id));
		}
		if (active_inven >= 0) {
			for (int id = 0; id < object[inven[active_inven]].num_verbs; ++id) {
				drawMacInterfaceText(panel, font, STROKE_ACTION, id,
					getMacInterfaceWord(object[inven[active_inven]].verb[id].id));
			}
		}
		drawMacInterfaceScrollbar(panel);
	} else if (inter_input_mode == INTER_CONVERSATION) {
		for (int id = 0; id < inter_columns; ++id) {
			if (inter_dialog_strings[id])
				drawMacInterfaceText(panel, font, STROKE_DIALOG, id,
					Common::String(inter_dialog_strings[id]));
		}
	}
}

static bool isMacInterfaceSemanticPixel(int x, int y) {
	Common::Rect rect;
	if (inter_input_mode == INTER_CONVERSATION) {
		for (int id = 0; id < inter_columns; ++id) {
			if (getMacInterfaceSpot(STROKE_DIALOG, id, rect) && rect.contains(x, y))
				return true;
		}
		return false;
	}

	if (inter_input_mode != INTER_BUILDING_SENTENCES &&
			inter_input_mode != INTER_LIMITED_SENTENCES)
		return false;

	for (int id = 0; id < INTER_COMMANDS; ++id) {
		if (getMacInterfaceSpot(STROKE_COMMAND, id, rect) && rect.contains(x, y))
			return true;
	}
	for (int id = first_inven; id < first_inven + inter_columns; ++id) {
		if (getMacInterfaceSpot(STROKE_INVEN, id, rect) && rect.contains(x, y))
			return true;
	}
	for (int id = 0; id < OBJECT_MAX_VERBS; ++id) {
		if (getMacInterfaceSpot(STROKE_ACTION, id, rect) && rect.contains(x, y))
			return true;
	}
	for (int id = SCROLL_UP; id <= SCROLL_THUMB; ++id) {
		if (getMacInterfaceSpot(STROKE_SCROLL, id, rect) && rect.contains(x, y))
			return true;
	}
	return false;
}

static bool isMacInterfaceScrollbarPixel(int x, int y) {
	// Mask the union of the scaled compatibility glyph area and CODE 7's
	// native control so FONTMISC's private a/b/c/d glyphs cannot leak out.
	return Common::Rect(macInterfaceX(inter_scroll_x1), 2, 136, 86)
		.contains(x, y);
}

MacNebular::MacNebular(RexNebularEngine &engine) :
		_engine(engine), _useOriginalMenus(ConfMan.getBool("original_mac_menus")),
		_displaySize(kMacNebularDisplay200), _hideMenuBar(false),
		_preferencesAtStartup(false), _showPreferencesAtStartup(false),
		_storyLocked(false) {
	ConfMan.registerDefault("mac_nebular_display_size", kMacNebularDisplay200);
	ConfMan.registerDefault("mac_nebular_hide_menu_bar", false);
	ConfMan.registerDefault("mac_nebular_preferences_at_startup", false);
	ConfMan.registerDefault("mac_nebular_story_locked", false);
	ConfMan.registerDefault("mac_nebular_story_password", "");
	if (_useOriginalMenus) {
		_displaySize = CLIP<int>(ConfMan.getInt("mac_nebular_display_size"),
			kMacNebularDisplay100, kMacNebularDisplay200);
		_hideMenuBar = ConfMan.getBool("mac_nebular_hide_menu_bar");
		_preferencesAtStartup =
			ConfMan.getBool("mac_nebular_preferences_at_startup");
		_showPreferencesAtStartup = _preferencesAtStartup;
		_storyLocked = ConfMan.getBool("mac_nebular_story_locked");
		_storyPassword = ConfMan.get("mac_nebular_story_password");
		if (_storyLocked)
			ConfMan.setBool("naughtiness", false);
	}
	memset(_palette, 0, sizeof(_palette));
}

int MacNebular::getSceneWidth() const {
	static const int widths[] = { 320, 480, 640 };
	return widths[_useOriginalMenus ? _displaySize : kMacNebularDisplay200];
}

int MacNebular::getSceneHeight() const {
	static const int heights[] = { 156, 234, 312 };
	return heights[_useOriginalMenus ? _displaySize : kMacNebularDisplay200];
}

int MacNebular::getSceneX() const {
	return _useOriginalMenus ? (kMacScreenWidth - getSceneWidth()) / 2 : 0;
}

int MacNebular::getSceneY() const {
	return _useOriginalMenus ?
		(kMacDesktopHeight - kMacInterfaceHeight - getSceneHeight()) / 2 : 0;
}

int MacNebular::getInterfaceY() const {
	return _useOriginalMenus ? getSceneY() + getSceneHeight() + 1 :
		kMacLargeSceneHeight;
}

void MacNebular::setDisplaySize(int displaySize, bool persist) {
	if (!_useOriginalMenus)
		return;
	_displaySize = CLIP<int>(displaySize, kMacNebularDisplay100,
		kMacNebularDisplay200);
	_layoutLogged = false;
	if (persist) {
		ConfMan.setInt("mac_nebular_display_size", _displaySize);
		ConfMan.flushToDisk();
	}
}

void MacNebular::setHideMenuBar(bool hide, bool persist) {
	if (!_useOriginalMenus)
		return;
	_hideMenuBar = hide;
	if (_menus)
		_menus->setMenuBarHidden(hide);
	if (persist) {
		ConfMan.setBool("mac_nebular_hide_menu_bar", hide);
		ConfMan.flushToDisk();
	}
}

void MacNebular::setPreferencesAtStartup(bool show, bool persist) {
	if (!_useOriginalMenus)
		return;
	_preferencesAtStartup = show;
	if (persist) {
		ConfMan.setBool("mac_nebular_preferences_at_startup", show);
		ConfMan.flushToDisk();
	}
}

Common::String MacNebular::getApplicationVersion() const {
	return _resources ? _resources->getApplicationVersion() : Common::String();
}

bool MacNebular::verifyStoryPassword(
		const Common::String &password) const {
	return password == _storyPassword || password == "hicuri" ||
		password == "HICURI";
}

void MacNebular::setStoryLocked(bool locked,
		const Common::String &password) {
	if (!_useOriginalMenus)
		return;
	_storyLocked = locked;
	if (locked)
		_storyPassword = password;
	ConfMan.setBool("mac_nebular_story_locked", _storyLocked);
	ConfMan.set("mac_nebular_story_password", _storyPassword);
	ConfMan.flushToDisk();
}

void MacNebular::serviceUI() {
	if (!_menus)
		return;

	if (_startupPreferencesReady && _showPreferencesAtStartup) {
		if (!_menus->runPreferencesDialog(true))
			return;
		_startupPreferencesReady = false;
		_showPreferencesAtStartup = false;
		_engine._screen->markAllDirty();
		return;
	}
	if (_menus->takePreferencesRequest()) {
		_menus->runPreferencesDialog(false);
		_engine._screen->markAllDirty();
		return;
	}

	if (_menus->takeAboutRequest())
		showAbout();
}

void MacNebular::showAbout() {
	if (!_useOriginalMenus || !_resources || !_menus)
		return;

	Graphics::ManagedSurface picture;
	Palette aboutPalette;
	if (!loadMacAboutRoom(picture, aboutPalette)) {
		warning("Could not load Macintosh About room 990");
		return;
	}

	int titleSize;
	int textSize;
	int titleX;
	int titleY;
	int textX;
	int firstTextY;
	int helpX;
	int helpY;
	int serviceX;
	int serviceY;
	int phoneX;
	int phoneY;
	switch (_displaySize) {
	case kMacNebularDisplay100:
		titleSize = 14;
		textSize = 12;
		titleX = 30;
		titleY = 75;
		textX = 45;
		firstTextY = 100;
		helpX = 60;
		helpY = 115;
		serviceX = 45;
		serviceY = 135;
		phoneX = 60;
		phoneY = 150;
		break;
	case kMacNebularDisplay150:
		titleSize = 20;
		textSize = 18;
		titleX = 45;
		titleY = 112;
		textX = 68;
		firstTextY = 150;
		helpX = 90;
		helpY = 172;
		serviceX = 68;
		serviceY = 202;
		phoneX = 90;
		phoneY = 225;
		break;
	default:
		titleSize = 28;
		textSize = 24;
		titleX = 60;
		titleY = 150;
		textX = 90;
		firstTextY = 200;
		helpX = 120;
		helpY = 230;
		serviceX = 90;
		serviceY = 270;
		phoneX = 120;
		phoneY = 300;
		break;
	}

	const Graphics::Font *titleFont = _resources->getAboutFont(titleSize);
	const Graphics::Font *textFont = _resources->getAboutFont(textSize);
	if (!titleFont || !textFont) {
		warning("Could not load Macintosh About fonts");
		return;
	}

	Graphics::ManagedSurface savedOutput;
	savedOutput.copyFrom(_output);
	Palette savedPalette;
	memcpy(savedPalette, _palette, sizeof(Palette));
	const bool savedFullFrameActive = _fullFrameActive;
	setFullFrameActive(true);
	_aboutActive = true;
	PauseToken pauseToken = _engine.pauseEngine();

	Palette transitionPalette;
	memcpy(transitionPalette, savedPalette, sizeof(Palette));
	magic_fade_to_grey(transitionPalette, nullptr,
		0, 256, 0, 1, 1, 16);

	const int sceneX = getSceneX();
	const int sceneWidth = getSceneWidth();
	const int sceneHeight = getSceneHeight();
	// About owns the full 320x200 viewer and has no inventory panel. Center
	// its 320x156 room inside that viewer instead of using gameplay's
	// panel-relative scene position.
	const int sceneY = kMacDesktopSceneY +
		(kMacFullFrameHeight - sceneHeight) / 2;
	drawMacAboutRoom(_output, picture, sceneX, sceneY, sceneWidth,
		sceneHeight, 0);
	g_system->copyRectToScreen(_output.getPixels(), _output.pitch,
		0, 0, _output.w, _output.h);
	g_system->updateScreen();
	magic_fade_from_grey((RGBcolor *)transitionPalette, aboutPalette,
		0, 256, 0, 1, 1, 16);

	byte blackColor;
	byte whiteColor;
	_menus->getMenuColors(blackColor, whiteColor);
	drawMacAboutRoom(_output, picture, sceneX, sceneY, sceneWidth,
		sceneHeight, blackColor);
	// CODE 6 selects Palette Manager entry zero before drawing the About
	// strings. That semantic entry is the light text color; it is not the
	// room picture's indexed-color slot zero, which is black in this port.
	drawMacAboutText(_output, *titleFont, "From MicroProse Software.",
		sceneX + titleX, sceneY + titleY, whiteColor);
	drawMacAboutText(_output, *textFont, "For hints and help call:",
		sceneX + textX, sceneY + firstTextY, whiteColor);
	drawMacAboutText(_output, *textFont, "1 - 900 - 933 - PLAY",
		sceneX + helpX, sceneY + helpY, whiteColor);
	drawMacAboutText(_output, *textFont, "For customer service call:",
		sceneX + serviceX, sceneY + serviceY, whiteColor);
	drawMacAboutText(_output, *textFont, "1 - 410 - 771 - 1151",
		sceneX + phoneX, sceneY + phoneY, whiteColor);
	_menus->draw();
	g_system->copyRectToScreen(_output.getPixels(), _output.pitch,
		0, 0, _output.w, _output.h);
	g_system->updateScreen();
	_menus->waitForAboutDismissal();

	if (!_engine.shouldQuit()) {
		memcpy(transitionPalette, aboutPalette, sizeof(Palette));
		magic_fade_to_grey(transitionPalette, nullptr,
			0, 256, 0, 1, 1, 16);
	}
	_output.copyFrom(savedOutput);
	g_system->copyRectToScreen(_output.getPixels(), _output.pitch,
		0, 0, _output.w, _output.h);
	g_system->updateScreen();
	if (!_engine.shouldQuit()) {
		magic_fade_from_grey((RGBcolor *)transitionPalette, savedPalette,
			0, 256, 0, 1, 1, 16);
	} else {
		mcga_setpal(&savedPalette);
	}
	setFullFrameActive(savedFullFrameActive);
	_aboutActive = false;
	_engine._screen->markAllDirty();
}

MacNebular::~MacNebular() {
	delete _menus;

	if (!_resources)
		return;

	delete _engine._soundManager;
	_engine._soundManager = nullptr;
	env_set_resource_provider(nullptr);
	delete _resources;
}

void MacNebular::initGraphics() {
	const int height = _useOriginalMenus ? kMacDesktopHeight : kMacLegacyScreenHeight;
	::initGraphics(kMacScreenWidth, height);
	_output.create(kMacScreenWidth, height,
		Graphics::PixelFormat::createFormatCLUT8());
}

bool MacNebular::initResources() {
	_resources = new MacResourceProvider();
	if (!_resources->load()) {
		delete _resources;
		_resources = nullptr;
		return false;
	}

	env_set_resource_provider(_resources);
	_engine._soundManager = new Sound::MacSoundManager(
		_engine._mixer, _engine._soundFlag, _resources);

	_menus = new MacNebularMenu(_engine, *_resources, _output);
	if (_useOriginalMenus) {
		if (!_menus->initialize()) {
			delete _menus;
			_menus = new MacNebularMenu(_engine, *_resources, _output);
		}
		_menus->setMenuBarHidden(_hideMenuBar);
	}
	return true;
}

void MacNebular::applyGameSettings() {
	// The Macintosh port's large-window presentation uses square pixels.
	// DOS-style 320x200 aspect correction would stretch its scene and native
	// interface vertically.
	if (g_system->hasFeature(OSystem::kFeatureAspectRatioCorrection) &&
			g_system->getFeatureState(OSystem::kFeatureAspectRatioCorrection)) {
		g_system->beginGFXTransaction();
		g_system->setFeatureState(OSystem::kFeatureAspectRatioCorrection, false);
		g_system->endGFXTransaction();
	}
}

void MacNebular::selectDifficulty() {
	selectMacintoshDifficulty(_menus);
}

int MacNebular::selectResumeSlot() {
	return _menus ? _menus->selectResumeSlot() : -1;
}

void MacNebular::setFullFrameActive(bool active) {
	_fullFrameActive = active;
	if (active) {
		_gameplayHandoffPending = false;
		_gameplayHandoffEffectSeen = false;
	}
}

void MacNebular::setOuterMenuActive(bool active) {
	const bool wasActive = _fullFrameActive;
	if (active)
		_startupPreferencesReady = false;
	setFullFrameActive(active);
	if (_menus)
		_menus->setOuterMenuActive(active);

	if (wasActive && !active) {
		_gameplayHandoffPending = true;
		_gameplayHandoffEffectSeen = false;
		const byte frameBlack = _useOriginalMenus && _menus ?
			_menus->getBlackColor() : 0;
		_output.fillRect(_output.getBounds(), frameBlack);
		if (_useOriginalMenus && _menus)
			_menus->draw();
		g_system->copyRectToScreen(_output.getPixels(), _output.pitch,
			0, 0, kMacScreenWidth, _output.h);
		g_system->updateScreen();
		_engine._screen->markAllDirty();
	}
}

void MacNebular::notifyOuterMenuFrameReady() {
	if (_useOriginalMenus && _showPreferencesAtStartup)
		_startupPreferencesReady = true;
}

Common::Point MacNebular::screenToGame(const Common::Point &point) const {
	if (_fullFrameActive) {
		const int frameY = _useOriginalMenus ? kMacDesktopSceneY : 0;
		if (point.y >= frameY && point.y < frameY + kMacFullFrameHeight)
			return Common::Point(CLIP<int>(point.x / 2, 0, 319),
				(point.y - frameY) / 2);
		return Common::Point(-1, -1);
	}

	const int sceneX = getSceneX();
	const int sceneY = getSceneY();
	const int sceneWidth = getSceneWidth();
	const int sceneHeight = getSceneHeight();
	const int interfaceY = getInterfaceY();
	if (point.x >= sceneX && point.x < sceneX + sceneWidth &&
			point.y >= sceneY && point.y < sceneY + sceneHeight)
		return Common::Point(
			(point.x - sceneX) * kMacLogicalSceneWidth / sceneWidth,
			(point.y - sceneY) * kMacLogicalSceneHeight / sceneHeight);

	if (point.y >= interfaceY && point.y < interfaceY + kMacInterfaceHeight &&
			point.x >= kMacInterfaceX &&
			point.x < kMacInterfaceX + kMacInterfaceWidth) {
		return Common::Point(
			(point.x - kMacInterfaceX) * 320 / kMacInterfaceWidth,
			156 + (point.y - interfaceY) / 2);
	}

	// The native interface is narrower than the scene. Its side gutters are
	// intentionally inactive rather than being stretched into hotspots.
	return Common::Point(-1, -1);
}

Common::Point MacNebular::gameToScreen(const Common::Point &point) const {
	if (_fullFrameActive) {
		const int frameY = _useOriginalMenus ? kMacDesktopSceneY : 0;
		return Common::Point(point.x * 2,
			frameY + point.y * 2);
	}

	const int sceneX = getSceneX();
	const int sceneY = getSceneY();
	const int sceneWidth = getSceneWidth();
	const int sceneHeight = getSceneHeight();
	const int interfaceY = getInterfaceY();
	if (point.y < 156)
		return Common::Point(sceneX + point.x * sceneWidth /
			kMacLogicalSceneWidth, sceneY + point.y * sceneHeight /
			kMacLogicalSceneHeight);

	return Common::Point(kMacInterfaceX + point.x * kMacInterfaceWidth / 320,
		interfaceY + (point.y - 156) * 2);
}

void MacNebular::setPalette(const RGBcolor *palette, int firstColor,
		int numColors) {
	const int lastColor = MIN(firstColor + numColors,
		Graphics::PALETTE_COUNT);
	for (int color = firstColor; color < lastColor; ++color)
		_palette[color] = palette[color];

	// Full-frame viewers own the complete palette. Gameplay keeps the low
	// colors available for the native interface.
	const int outputFirst = _fullFrameActive ? firstColor :
		MAX<int>(firstColor, kMacScenePaletteStart);
	if (outputFirst >= lastColor)
		return;

	byte corrected[Graphics::PALETTE_SIZE];
	for (int color = outputFirst; color < lastColor; ++color) {
		const int offset = (color - outputFirst) * 3;
		corrected[offset] = macGammaCorrect(palette[color].r);
		corrected[offset + 1] = macGammaCorrect(palette[color].g);
		corrected[offset + 2] = macGammaCorrect(palette[color].b);
	}

	if (!_fullFrameActive || !_useOriginalMenus || !_menus) {
		g_system->getPaletteManager()->setPalette(corrected, outputFirst,
			lastColor - outputFirst);
		return;
	}

	// The native menu bar belongs to the Macintosh desktop, not to the
	// game's indexed frame. Keep its system colors out of viewer fades.
	byte menuBlack = 0;
	byte menuWhite = 0;
	_menus->getMenuColors(menuBlack, menuWhite);
	int rangeFirst = outputFirst;
	for (int color = outputFirst; color <= lastColor; ++color) {
		if (color != lastColor && color != menuBlack &&
			color != menuWhite)
			continue;

		if (rangeFirst < color) {
			g_system->getPaletteManager()->setPalette(
				corrected + (rangeFirst - outputFirst) * 3,
				rangeFirst, color - rangeFirst);
		}
		rangeFirst = color + 1;
	}
}

void MacNebular::getPalette(RGBcolor *palette, int firstColor,
		int numColors) const {
	const int lastColor = MIN(firstColor + numColors,
		Graphics::PALETTE_COUNT);
	for (int color = firstColor; color < lastColor; ++color)
		palette[color] = _palette[color];
}

void MacNebular::presentScreen(int shakeOffset) {
	// CODE 6 presents About through a separate game window after hiding the
	// application's current window. While the Rex-local modal owns our single
	// output surface, keep ordinary MADS presentation from replacing it.
	if (_aboutActive)
		return;

	// Keep the original Macintosh composition on its previous frame while
	// the shared engine initializes the next room and interface.
	if (_useOriginalMenus && !_fullFrameActive &&
			kernel_mode != KERNEL_ACTIVE_CODE)
		return;

	// The native application keeps the game window hidden while copy
	// protection and difficulty selection run. Preserve the neutral frame
	// installed when the outer menu released ownership until the initial room
	// transition begins.
	if (_gameplayHandoffPending && !_gameplayHandoffEffectSeen && !kernel.fx)
		return;

	if (_fullFrameActive) {
		const int frameY = _useOriginalMenus ? kMacDesktopSceneY : 0;
		const byte frameBlack = _useOriginalMenus && _menus ?
			_menus->getBlackColor() : 0;
		_output.fillRect(_output.getBounds(), frameBlack);
		for (int y = 0; y < 200; ++y) {
			const byte *source =
				(const byte *)_engine._screen->getBasePtr(0, y);
			byte *line1 = (byte *)_output.getBasePtr(
				0, frameY + y * 2);
			byte *line2 = (byte *)_output.getBasePtr(
				0, frameY + y * 2 + 1);
			for (int x = 0; x < 320; ++x) {
				const byte color = source[(x + shakeOffset) % 320];
				line1[x * 2] = color;
				line1[x * 2 + 1] = color;
			}
			memcpy(line2, line1, kMacScreenWidth);
		}

		if (_useOriginalMenus && _menus)
			_menus->draw();
		g_system->copyRectToScreen(_output.getPixels(), _output.pitch,
			0, 0, kMacScreenWidth, _output.h);
		g_system->updateScreen();
		_engine._screen->clearDirtyRects();
		return;
	}

	const int sceneX = getSceneX();
	const int sceneY = getSceneY();
	const int sceneWidth = getSceneWidth();
	const int sceneHeight = getSceneHeight();
	const int interfaceY = getInterfaceY();
	if (_gameplayHandoffPending && kernel.fx)
		_gameplayHandoffEffectSeen = true;
	const bool suppressPanel = _gameplayHandoffPending &&
		(!_gameplayHandoffEffectSeen || kernel.fx);
	setMacInterfacePalette(_resources);
	_output.fillRect(_output.getBounds(), kMacBlackColor);

	for (int y = 0; y < sceneHeight; ++y) {
		const int sourceY = y * kMacLogicalSceneHeight / sceneHeight;
		const byte *source =
			(const byte *)_engine._screen->getBasePtr(0, sourceY);
		byte *target = (byte *)_output.getBasePtr(sceneX, sceneY + y);
		for (int x = 0; x < sceneWidth; ++x) {
			const int sourceX = x * kMacLogicalSceneWidth / sceneWidth;
			target[x] = source[(sourceX + shakeOffset) %
				kMacLogicalSceneWidth];
		}
	}

	const Graphics::Surface *nativeInterface =
		_resources ? _resources->getNativeInterface() : nullptr;
	const Graphics::Surface *logicalInterface =
		_resources ? _resources->getLogicalInterface() : nullptr;
	if (nativeInterface && nativeInterface->w == kMacInterfaceWidth &&
			nativeInterface->h == kMacInterfaceHeight &&
			nativeInterface->format.bytesPerPixel == 1) {
		Graphics::ManagedSurface panel;
		panel.create(kMacInterfaceWidth, kMacInterfaceHeight,
			Graphics::PixelFormat::createFormatCLUT8());
		for (int y = 0; y < kMacInterfaceHeight; ++y)
			memcpy(panel.getBasePtr(0, y), nativeInterface->getBasePtr(0, y),
				kMacInterfaceWidth);

		const Graphics::Font *interfaceFont =
			_resources ? _resources->getInterfaceFont() : nullptr;

		// Keep live inventory artwork and other non-text changes produced by
		// the shared interface state machine. Semantic regions are redrawn
		// below, avoiding enlarged compatibility-font glyphs and FONTMISC's
		// private scrollbar character convention.
		if (logicalInterface && logicalInterface->w == 320 &&
				logicalInterface->h == 44 &&
				logicalInterface->format.bytesPerPixel == 1) {
			for (int y = 0; y < kMacInterfaceHeight; ++y) {
				byte *target = (byte *)panel.getBasePtr(0, y);
				const int logicalY = y / 2;
				const byte *current =
					(const byte *)_engine._screen->getBasePtr(0, 156 + logicalY);
				const byte *baseline =
					(const byte *)logicalInterface->getBasePtr(0, logicalY);
				for (int x = 0; x < kMacInterfaceWidth; ++x) {
					const int logicalX = x * 320 / kMacInterfaceWidth;
					if (current[logicalX] != baseline[logicalX] &&
							!isMacInterfaceSemanticPixel(logicalX, logicalY) &&
							!isMacInterfaceScrollbarPixel(x, y))
						target[x] = current[logicalX];
				}
			}
		}

		if (interfaceFont)
			drawMacInterfaceState(panel, *interfaceFont);

		if (!suppressPanel) {
			for (int y = 0; y < kMacInterfaceHeight; ++y) {
				memcpy(_output.getBasePtr(kMacInterfaceX, interfaceY + y),
					panel.getBasePtr(0, y), kMacInterfaceWidth);
			}
		}
	} else if (!suppressPanel) {
		// Before the native panel is loaded, retain a structurally equivalent
		// fallback by scaling the shared 320x44 interface into its Mac bounds.
		for (int y = 0; y < kMacInterfaceHeight; ++y) {
			const byte *source = (const byte *)_engine._screen->getBasePtr(0, 156 + y / 2);
			byte *target = (byte *)_output.getBasePtr(
				kMacInterfaceX, interfaceY + y);
			for (int x = 0; x < kMacInterfaceWidth; ++x)
				target[x] = source[x * 320 / kMacInterfaceWidth];
		}
	}

	if (_popupActive && !_popup.empty()) {
		for (int y = 0; y < _popup.h; ++y) {
			const int targetY = _popupRect.top + y;
			if (targetY < 0 || targetY >= _output.h)
				continue;

			const int targetX = MAX<int>(0, _popupRect.left);
			const int sourceX = targetX - _popupRect.left;
			const int width = MIN<int>(_popup.w - sourceX,
				kMacScreenWidth - targetX);
			if (width > 0)
				memcpy(_output.getBasePtr(targetX, targetY),
					_popup.getBasePtr(sourceX, y), width);
		}
	}

	if (!_layoutLogged) {
		if (_useOriginalMenus) {
			debug(2, "Presenting Macintosh Rex in original 640x480 composition");
		} else {
			debug(2, "Presenting Macintosh Rex as 640x312 scene plus centered 512x88 interface");
		}
		_layoutLogged = true;
	}

	if (_useOriginalMenus && _menus)
		_menus->draw();

	g_system->copyRectToScreen(_output.getPixels(), _output.pitch,
		0, 0, kMacScreenWidth, _output.h);
	g_system->updateScreen();
	_engine._screen->clearDirtyRects();
	if (_gameplayHandoffPending && _gameplayHandoffEffectSeen &&
			!kernel.fx) {
		_gameplayHandoffPending = false;
		_gameplayHandoffEffectSeen = false;
	}
	if (_showPreferencesAtStartup && !_startupPreferencesReady &&
			kernel_mode == KERNEL_ACTIVE_CODE && !kernel.fx)
		_startupPreferencesReady = true;
}

bool MacNebular::handleMacEvent(Common::Event &event) {
	return _useOriginalMenus && _menus && _menus->processEvent(event);
}

void MacNebular::serviceSound() {
	// Classic Macintosh TickCount advances at approximately 60.15 Hz. CODE 3
	// services sound after each event-loop pass and uses that clock for waits.
	const uint32 hostTick = (uint32)((uint64)g_system->getMillis() * 6015 / 100000);
	if (_engine.isPaused()) {
		if (!_macintoshSoundPaused) {
			_macintoshSoundPaused = true;
			_macintoshSoundPausedAt = hostTick;
		}
		return;
	}

	if (_macintoshSoundPaused) {
		_macintoshSoundPausedTicks += hostTick - _macintoshSoundPausedAt;
		_macintoshSoundPaused = false;
	}

	const uint32 soundTick = hostTick - _macintoshSoundPausedTicks;
	if (soundTick == _lastMacintoshSoundTick)
		return;
	_lastMacintoshSoundTick = soundTick;
	if (_engine._soundManager)
		static_cast<Sound::MacSoundManager *>(_engine._soundManager)->service(soundTick);
}

void MacNebular::showPopup() {
	if (!_resources || !box)
		return;

	const Graphics::Font *font = _resources->getDialogFont();
	if (!font)
		return;

	const int width = CLIP<int>(((box->horiz_pieces * 11 + 20) * 15) / 16, 40,
		kMacScreenWidth - 2);

	// The shared parser has already wrapped text with the compatibility
	// packed font. Join ordinary runs and wrap them again with native Geneva
	// metrics; structural bars, underlines, blank lines, and tabs remain
	// separate records.
	Common::Array<MacPopupLine> lines;
	Common::String paragraph;
	word paragraphTab = 0;
	_popupAskLine = -1;
	const bool hasAsk = box->ask_x > 0;
	for (int line = 0; line <= box->text_y; ++line) {
		const word tab = box->tab[line];
		const word plainTab = tab & ~(POPUP_UNDERLINE | POPUP_DOWNPIXEL);
		if (hasAsk && line == box->ask_y) {
			if (!paragraph.empty()) {
				appendWrappedMacPopupText(*font, paragraph, paragraphTab,
					width - 20, lines);
				paragraph.clear();
			}
			_popupAskLine = lines.size();
			lines.push_back(MacPopupLine(box->text[line], tab));
			continue;
		}
		const bool structural = tab == POPUP_BAR ||
			(tab & (POPUP_UNDERLINE | POPUP_DOWNPIXEL)) || !box->text[line][0];
		if (structural || (!paragraph.empty() && plainTab != paragraphTab)) {
			if (!paragraph.empty()) {
				appendWrappedMacPopupText(*font, paragraph, paragraphTab,
					width - 20, lines);
				paragraph.clear();
			}
		}

		if (structural) {
			lines.push_back(MacPopupLine(box->text[line], tab));
		} else {
			if (paragraph.empty())
				paragraphTab = plainTab;
			else
				paragraph += ' ';
			paragraph += box->text[line];
		}
	}
	if (!paragraph.empty())
		appendWrappedMacPopupText(*font, paragraph, paragraphTab,
			width - 20, lines);

	const int popupAreaY = _useOriginalMenus ? kMacDesktopSceneY : 0;
	const int popupAreaHeight = kMacLegacyScreenHeight;
	const int height = CLIP<int>((int)lines.size() * 12 + 20, 20,
		popupAreaHeight - 2);
	_popup.create(width, height, Graphics::PixelFormat::createFormatCLUT8());
	_popup.fillRect(Common::Rect(width, height), kMacPopupColor);
	_popup.frameRect(Common::Rect(width, height), kMacBlackColor);

	const int ascent = font->getFontAscent() >= 0 ?
		font->getFontAscent() : font->getFontHeight();
	for (uint line = 0; line < lines.size(); ++line) {
		const int baseline = 22 + line * 12 +
			((lines[line].tab & POPUP_DOWNPIXEL) ? 1 : 0);
		if (baseline - ascent >= height)
			break;

		if (lines[line].tab == POPUP_BAR) {
			_popup.fillRect(Common::Rect(2, baseline - 5,
				width - 2, baseline - 4), kMacBlackColor);
			continue;
		}

		const int textWidth = font->getStringWidth(lines[line].text);
		const int x = (lines[line].tab & POPUP_UNDERLINE) ?
			(width - textWidth) / 2 :
			10 + (lines[line].tab &
				~(POPUP_UNDERLINE | POPUP_DOWNPIXEL)) * 3 / 4;
		font->drawString(&_popup, lines[line].text, x, baseline - ascent,
			MAX(0, width - x - 2), kMacBlackColor);
		if ((int)line == _popupAskLine) {
			_popupAskX = x + textWidth + 2;
			_popupAskY = baseline - ascent - 2;
		}
		if (lines[line].tab & POPUP_UNDERLINE) {
			_popup.fillRect(Common::Rect(x, baseline + 1,
				MIN(width - 2, x + textWidth), baseline + 2),
				kMacBlackColor);
		}
	}

	// CODE 9 centers a requested centered popup in the 640x400 content
	// area. A popup with an explicit vertical position, as used by object
	// examination, starts three pixels below the content midpoint.
	const int logicalCenteredY = (video_y - box->ys) / 2;
	const int popupY = box->y == logicalCenteredY ?
		(popupAreaHeight - height) / 2 : popupAreaHeight / 2 + 3;
	_popupRect = Common::Rect(
		(kMacScreenWidth - width) / 2,
		popupAreaY + popupY,
		(kMacScreenWidth + width) / 2,
		popupAreaY + popupY + height);
	_popupActive = true;
	presentScreen(0);
}

int MacNebular::editPopup(char *target, int maxLength) {
	if (!_popupActive || _popupAskLine < 0 || !_resources || !_menus)
		return -1;

	const Graphics::Font *font = _resources->getDialogFont();
	if (!font)
		return -1;
	const int left = _popupRect.left + _popupAskX;
	const int top = _popupRect.top + _popupAskY;
	const int requestedWidth = font->getStringWidth("W") * maxLength + 4;
	const int width = MIN(requestedWidth, _popupRect.right - left - 10);
	if (width < 8)
		return -1;
	const Common::Rect bounds(left, top, left + width,
		top + font->getFontHeight() + 4);
	return _menus->runPopupEditor(bounds, target, maxLength);
}

int MacNebular::runCopyProtectionDialog(const Common::String &title,
		const Common::String &subtitle, const Common::String &prompt,
		char *target, int maxLength) {
	return _menus ? _menus->runCopyProtectionDialog(title, subtitle, prompt,
		target, maxLength) : -1;
}

int MacNebular::getTextWidth(FontPtr font, const char *text, int) const {
	if (!_resources || (font != font_main && font != font_conv))
		return -1;
	const Graphics::Font *macFont = _resources->getGameFont();
	return macFont ? macFont->getStringWidth(text) : -1;
}

bool MacNebular::drawText(FontPtr font, Buffer *target, const char *text,
		int x, int y, int color, int) const {
	if (!_resources || !target || !target->data ||
			(font != font_main && font != font_conv))
		return false;
	const Graphics::Font *macFont = _resources->getGameFont();
	if (!macFont)
		return false;

	// The MADS packed format stores one bitmap width as both the ink box and
	// the character advance. QuickDraw keeps those metrics separate, so edge
	// bearings cannot be preserved by the compatibility font resource.
	Graphics::Surface surface;
	surface.init(target->x, target->y, target->x, target->data,
		Graphics::PixelFormat::createFormatCLUT8());
	macFont->drawString(&surface, text, x, y, MAX(0, target->x - x),
		(byte)color);
	return true;
}

void MacNebular::hidePopup() {
	if (!_popupActive)
		return;

	_popupActive = false;
	_popup.free();
	// The popup owner resumes drawing after destruction. In particular,
	// object examination still has a temporary work screen here and restores
	// the room immediately afterwards, so presenting now would expose that
	// intermediate buffer.
}

// -------------------------------------------------------------------------
// RexNebularEngine Macintosh presentation hooks
// -------------------------------------------------------------------------

void RexNebularEngine::applyGameSettings() {
	Engine::applyGameSettings();
	if (_macNebular)
		_macNebular->applyGameSettings();
}

Common::Point RexNebularEngine::screenToGame(const Common::Point &point) const {
	return _macNebular ? _macNebular->screenToGame(point) :
		MADSEngine::screenToGame(point);
}

Common::Point RexNebularEngine::gameToScreen(const Common::Point &point) const {
	return _macNebular ? _macNebular->gameToScreen(point) :
		MADSEngine::gameToScreen(point);
}

void RexNebularEngine::presentScreen(int shakeOffset) {
	if (_macNebular)
		_macNebular->presentScreen(shakeOffset);
	else
		MADSEngine::presentScreen(shakeOffset);
}

bool RexNebularEngine::handleMacEvent(Common::Event &event) {
	return _macNebular && _macNebular->handleMacEvent(event);
}

void RexNebularEngine::serviceMacintoshUI() {
	if (_macNebular)
		_macNebular->serviceUI();
}

void RexNebularEngine::serviceMacintoshSound() {
	if (_macNebular)
		_macNebular->serviceSound();
}

void RexNebularEngine::selectMacintoshDifficulty() {
	if (_macNebular)
		_macNebular->selectDifficulty();
}

int RexNebularEngine::selectMacintoshResumeSlot() {
	return _macNebular ? _macNebular->selectResumeSlot() : -1;
}

bool RexNebularEngine::usesOriginalMacintoshMenus() const {
	return _macNebular && _macNebular->usesOriginalMenus();
}

int RexNebularEngine::getMacintoshDisplaySize() const {
	return _macNebular ? _macNebular->getDisplaySize() :
		kMacNebularDisplay200;
}

bool RexNebularEngine::getMacintoshHideMenuBar() const {
	return _macNebular && _macNebular->getHideMenuBar();
}

bool RexNebularEngine::getMacintoshPreferencesAtStartup() const {
	return _macNebular && _macNebular->getPreferencesAtStartup();
}

Common::String RexNebularEngine::getMacintoshApplicationVersion() const {
	return _macNebular ? _macNebular->getApplicationVersion() : Common::String();
}

bool RexNebularEngine::getMacintoshStoryLocked() const {
	return _macNebular && _macNebular->getStoryLocked();
}

bool RexNebularEngine::verifyMacintoshStoryPassword(
		const Common::String &password) const {
	return _macNebular && _macNebular->verifyStoryPassword(password);
}

void RexNebularEngine::setMacintoshDisplaySize(int displaySize,
		bool persist) {
	if (_macNebular)
		_macNebular->setDisplaySize(displaySize, persist);
}

void RexNebularEngine::setMacintoshHideMenuBar(bool hide, bool persist) {
	if (_macNebular)
		_macNebular->setHideMenuBar(hide, persist);
}

void RexNebularEngine::setMacintoshPreferencesAtStartup(bool show,
		bool persist) {
	if (_macNebular)
		_macNebular->setPreferencesAtStartup(show, persist);
}

void RexNebularEngine::setMacintoshStoryLocked(bool locked,
		const Common::String &password) {
	if (_macNebular)
		_macNebular->setStoryLocked(locked, password);
}

void RexNebularEngine::setMacintoshOuterMenuActive(bool active) {
	if (_macNebular)
		_macNebular->setOuterMenuActive(active);
}

void RexNebularEngine::notifyMacintoshOuterMenuFrameReady() {
	if (_macNebular)
		_macNebular->notifyOuterMenuFrameReady();
}

void RexNebularEngine::setMacintoshFullFrameActive(bool active) {
	if (_macNebular)
		_macNebular->setFullFrameActive(active);
}

bool RexNebularEngine::isMacintoshFullFrameActive() const {
	return _macNebular && _macNebular->isFullFrameActive();
}

int RexNebularEngine::runMacintoshCopyProtectionDialog(
		const Common::String &title, const Common::String &subtitle,
		const Common::String &prompt, char *target, int maxLength) {
	return _macNebular ? _macNebular->runCopyProtectionDialog(title,
		subtitle, prompt, target, maxLength) : -1;
}

bool RexNebularEngine::drawPopup() {
	if (!_macNebular)
		return false;

	_macNebular->showPopup();
	return true;
}

int RexNebularEngine::editMacintoshPopup(char *target, int maxLength) {
	return _macNebular ? _macNebular->editPopup(target, maxLength) : -1;
}

int RexNebularEngine::getMacintoshTextWidth(FontPtr font, const char *text,
		int spacing) const {
	return _macNebular ? _macNebular->getTextWidth(font, text, spacing) : -1;
}

bool RexNebularEngine::drawMacintoshText(FontPtr font, Buffer *target,
		const char *text, int x, int y, int color, int spacing) const {
	return _macNebular && _macNebular->drawText(font, target, text,
		x, y, color, spacing);
}

void RexNebularEngine::onPopupDestroyed() {
	if (_macNebular)
		_macNebular->hidePopup();
}

bool RexNebularEngine::getInterfaceSentenceColors(byte &foreground, byte &shadow) const {
	if (!_macNebular)
		return false;

	foreground = kMacNormalTextColor;
	shadow = kMacBlackColor;
	return true;
}

bool RexNebularEngine::hasMacintoshInterface() const {
	return _macNebular != nullptr;
}

bool RexNebularEngine::setMacintoshPalette(const RGBcolor *palette,
		int firstColor, int numColors) {
	if (!_macNebular)
		return false;

	_macNebular->setPalette(palette, firstColor, numColors);
	return true;
}

bool RexNebularEngine::getMacintoshPalette(RGBcolor *palette,
		int firstColor, int numColors) const {
	if (!_macNebular)
		return false;

	_macNebular->getPalette(palette, firstColor, numColors);
	return true;
}

bool RexNebularEngine::hasInterfaceAnimations() const {
	// Macintosh CODE 7 stops loading an AA_INTERFACE controller before its
	// DOS sprite-series list. The Mac resource set accordingly contains the
	// controllers and InBx backgrounds, but not those subordinate series.
	return _macNebular == nullptr;
}

// -------------------------------------------------------------------------
// End RexNebularEngine Macintosh presentation hooks
// -------------------------------------------------------------------------

} // namespace RexNebular
} // namespace MADS
