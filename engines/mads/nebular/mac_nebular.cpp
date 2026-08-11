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
#include "graphics/managed_surface.h"
#include "mads/core/env.h"
#include "mads/core/inter.h"
#include "mads/core/mcga.h"
#include "mads/core/object.h"
#include "mads/core/pal.h"
#include "mads/core/screen.h"
#include "mads/nebular/mac_menus.h"
#include "mads/nebular/mac_nebular.h"
#include "mads/nebular/mac_resources.h"
#include "mads/nebular/nebular.h"
#include "mads/nebular/popup.h"
#include "mads/nebular/mads/words.h"
#include "mads/nebular/sound/mac_sound.h"

namespace MADS {
namespace RexNebular {

enum {
	kMacScreenWidth = 640,
	kMacSceneHeight = 312,
	kMacInterfaceWidth = 512,
	kMacInterfaceHeight = 88,
	kMacInterfaceX = (kMacScreenWidth - kMacInterfaceWidth) / 2,
	kMacLegacyScreenHeight = kMacSceneHeight + kMacInterfaceHeight,
	kMacDesktopHeight = 480,
	kMacMenuBarHeight = 20,
	kMacDesktopSceneY = kMacMenuBarHeight + 20,
	kMacDesktopSeparatorY = kMacDesktopSceneY + kMacSceneHeight,
	kMacDesktopInterfaceY = kMacDesktopSeparatorY + 1,
	kMacBlackColor = 8,
	kMacNormalTextColor = 15,
	kMacLeftSelectColor = 13,
	kMacRightSelectColor = 14,
	kMacPopupColor = REX_DIALOG_FE_COLOR,
	kMacPanelTintR = 22,
	kMacPanelTintG = 39,
	kMacPanelTintB = 42
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

static int macPanelColorDistance(int r1, int g1, int b1,
		int r2, int g2, int b2) {
	const int dr = r1 - r2;
	const int dg = g1 - g2;
	const int db = b1 - b2;
	return dr * dr + dg * dg + db * db;
}

static void buildMacPanelWashLUT(const byte *nativePalette, byte washLUT[256]) {
	// The panel is still presented as CLUT8. Replace its first eight display
	// colors with a uniform 25-percent source / 75-percent cyan blend, then
	// map the completed panel to those colors. This models a translucent wash
	// without the checkerboard pattern used by an earlier approximation.
	Palette displayPalette;
	for (int color = 0; color < 8; ++color) {
		const int r = (nativePalette[color * 3 + 0] * 63 + 127) / 255;
		const int g = (nativePalette[color * 3 + 1] * 63 + 127) / 255;
		const int b = (nativePalette[color * 3 + 2] * 63 + 127) / 255;
		displayPalette[color].r = (r + 3 * kMacPanelTintR + 2) / 4;
		displayPalette[color].g = (g + 3 * kMacPanelTintG + 2) / 4;
		displayPalette[color].b = (b + 3 * kMacPanelTintB + 2) / 4;
	}

	// Index 8 backs the side gutters, popup text, and caption shadow. The
	// normal MADS interface palette makes it dark gray, not Macintosh black.
	displayPalette[kMacBlackColor].r = 0;
	displayPalette[kMacBlackColor].g = 0;
	displayPalette[kMacBlackColor].b = 0;
	mcga_setpal_range(&displayPalette, 0, kMacBlackColor + 1);

	// CODE 7 draws normal and selected interface words with palette indexes
	// 15, 13, and 14 respectively. Keep those roles independent of room
	// palette changes and outside the panel wash below.
	displayPalette[kMacLeftSelectColor].r =
		(nativePalette[1 * 3 + 0] * 63 + 127) / 255;
	displayPalette[kMacLeftSelectColor].g =
		(nativePalette[1 * 3 + 1] * 63 + 127) / 255;
	displayPalette[kMacLeftSelectColor].b =
		(nativePalette[1 * 3 + 2] * 63 + 127) / 255;
	displayPalette[kMacRightSelectColor].r =
		(nativePalette[2 * 3 + 0] * 63 + 127) / 255;
	displayPalette[kMacRightSelectColor].g =
		(nativePalette[2 * 3 + 1] * 63 + 127) / 255;
	displayPalette[kMacRightSelectColor].b =
		(nativePalette[2 * 3 + 2] * 63 + 127) / 255;
	displayPalette[kMacNormalTextColor].r = 63;
	displayPalette[kMacNormalTextColor].g = 63;
	displayPalette[kMacNormalTextColor].b = 63;
	mcga_setpal_range(&displayPalette, kMacLeftSelectColor, 3);

	for (int sourceColor = 0; sourceColor < 256; ++sourceColor) {
		int sourceR, sourceG, sourceB;
		if (sourceColor < 8 || sourceColor == kMacLeftSelectColor ||
				sourceColor == kMacRightSelectColor) {
			const int nativeColor = sourceColor == kMacLeftSelectColor ? 1 :
				(sourceColor == kMacRightSelectColor ? 2 : sourceColor);
			sourceR = (nativePalette[nativeColor * 3 + 0] * 63 + 127) / 255;
			sourceG = (nativePalette[nativeColor * 3 + 1] * 63 + 127) / 255;
			sourceB = (nativePalette[nativeColor * 3 + 2] * 63 + 127) / 255;
		} else if (sourceColor == kMacNormalTextColor) {
			sourceR = sourceG = sourceB = 63;
		} else {
			sourceR = master_palette[sourceColor].r;
			sourceG = master_palette[sourceColor].g;
			sourceB = master_palette[sourceColor].b;
		}

		const int washedR = (sourceR + 3 * kMacPanelTintR + 2) / 4;
		const int washedG = (sourceG + 3 * kMacPanelTintG + 2) / 4;
		const int washedB = (sourceB + 3 * kMacPanelTintB + 2) / 4;
		int bestColor = 0;
		int bestDistance = macPanelColorDistance(washedR, washedG, washedB,
			displayPalette[0].r, displayPalette[0].g, displayPalette[0].b);
		for (int candidate = 1; candidate < 8; ++candidate) {
			const int distance = macPanelColorDistance(washedR, washedG, washedB,
				displayPalette[candidate].r, displayPalette[candidate].g,
				displayPalette[candidate].b);
			if (distance < bestDistance) {
				bestDistance = distance;
				bestColor = candidate;
			}
		}
		washLUT[sourceColor] = bestColor;
	}
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

static Common::Rect scaleMacInterfaceRect(const Common::Rect &logical) {
	return Common::Rect(
		CLIP<int>(macInterfaceX(logical.left), 0, kMacInterfaceWidth),
		CLIP<int>(macInterfaceY(logical.top), 0, kMacInterfaceHeight),
		CLIP<int>(macInterfaceX(logical.right), 0, kMacInterfaceWidth),
		CLIP<int>(macInterfaceY(logical.bottom), 0, kMacInterfaceHeight));
}

static byte getMacInterfaceTextColor(int class_, int id) {
	if ((class_ == STROKE_COMMAND && id == left_command) ||
			(class_ == STROKE_INVEN && id == left_inven) ||
			(class_ == STROKE_ACTION && id == left_action) ||
			(class_ == STROKE_DIALOG && id == left_command))
		return kMacLeftSelectColor;
	if ((class_ == STROKE_COMMAND && id == right_command) ||
			(class_ == STROKE_INVEN && id == active_inven) ||
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

static void setMacInterfacePixel(Graphics::ManagedSurface &panel,
		int x, int y, byte color) {
	if (x >= 0 && x < panel.w && y >= 0 && y < panel.h)
		*(byte *)panel.getBasePtr(x, y) = color;
}

static void drawMacInterfaceArrow(Graphics::ManagedSurface &panel,
		const Common::Rect &rect, bool up, byte color) {
	if (rect.isEmpty())
		return;

	const int centerX = (rect.left + rect.right - 1) / 2;
	const int centerY = (rect.top + rect.bottom - 1) / 2;
	const int radius = MAX(1, MIN((rect.width() - 1) / 2,
		(rect.height() - 1) / 2));
	for (int row = 0; row <= radius; ++row) {
		const int y = up ? centerY - radius / 2 + row :
			centerY + radius / 2 - row;
		for (int x = centerX - row; x <= centerX + row; ++x)
			setMacInterfacePixel(panel, x, y, color);
	}
}

static Common::Rect getScaledMacInterfaceSpot(int class_, int id) {
	Common::Rect logical;
	return getMacInterfaceSpot(class_, id, logical) ?
		scaleMacInterfaceRect(logical) : Common::Rect();
}

static void drawMacInterfaceScrollbar(Graphics::ManagedSurface &panel) {
	const Common::Rect up = getScaledMacInterfaceSpot(STROKE_SCROLL, SCROLL_UP);
	const Common::Rect down = getScaledMacInterfaceSpot(STROKE_SCROLL, SCROLL_DOWN);
	const Common::Rect elevator =
		getScaledMacInterfaceSpot(STROKE_SCROLL, SCROLL_ELEVATOR);
	Common::Rect thumb = getScaledMacInterfaceSpot(STROKE_SCROLL, SCROLL_THUMB);

	if (!elevator.isEmpty())
		panel.frameRect(elevator, scrollbar_active == SCROLL_ELEVATOR ?
			kMacLeftSelectColor : kMacNormalTextColor);
	drawMacInterfaceArrow(panel, up, true,
		scrollbar_active == SCROLL_UP ? kMacLeftSelectColor : kMacNormalTextColor);
	drawMacInterfaceArrow(panel, down, false,
		scrollbar_active == SCROLL_DOWN ? kMacLeftSelectColor : kMacNormalTextColor);

	if (!thumb.isEmpty() && !elevator.isEmpty()) {
		thumb.left = MAX<int>(thumb.left, elevator.left + 2);
		thumb.right = MIN<int>(thumb.right, elevator.right - 2);
		if (!thumb.isEmpty())
			panel.fillRect(thumb, scrollbar_active == SCROLL_ELEVATOR ?
				kMacLeftSelectColor : kMacNormalTextColor);
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

MacNebular::MacNebular(RexNebularEngine &engine) :
		_engine(engine), _useOriginalMenus(ConfMan.getBool("original_mac_menus")) {
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

	if (_useOriginalMenus) {
		_menus = new MacNebularMenu(_engine, *_resources, _output);
		if (!_menus->initialize()) {
			delete _menus;
			_menus = nullptr;
		}
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

Common::Point MacNebular::screenToGame(const Common::Point &point) const {
	const int sceneY = _useOriginalMenus ? kMacDesktopSceneY : 0;
	const int interfaceY = _useOriginalMenus ?
		kMacDesktopInterfaceY : kMacSceneHeight;
	if (point.y >= sceneY && point.y < sceneY + kMacSceneHeight)
		return Common::Point(CLIP<int>(point.x / 2, 0, 319),
			(point.y - sceneY) / 2);

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
	const int sceneY = _useOriginalMenus ? kMacDesktopSceneY : 0;
	const int interfaceY = _useOriginalMenus ?
		kMacDesktopInterfaceY : kMacSceneHeight;
	if (point.y < 156)
		return Common::Point(point.x * 2, sceneY + point.y * 2);

	return Common::Point(kMacInterfaceX + point.x * kMacInterfaceWidth / 320,
		interfaceY + (point.y - 156) * 2);
}

void MacNebular::presentScreen(int shakeOffset) {
	const int sceneY = _useOriginalMenus ? kMacDesktopSceneY : 0;
	const int interfaceY = _useOriginalMenus ?
		kMacDesktopInterfaceY : kMacSceneHeight;
	_output.fillRect(_output.getBounds(), kMacBlackColor);

	// Native large-window mode doubles the 320x156 scene in both axes.
	for (int y = 0; y < 156; ++y) {
		const byte *source = (const byte *)_engine._screen->getBasePtr(0, y);
		byte *line1 = (byte *)_output.getBasePtr(0, sceneY + y * 2);
		byte *line2 = (byte *)_output.getBasePtr(0, sceneY + y * 2 + 1);
		for (int x = 0; x < 320; ++x) {
			const byte color = source[(x + shakeOffset) % 320];
			line1[x * 2] = color;
			line1[x * 2 + 1] = color;
		}
		memcpy(line2, line1, kMacScreenWidth);
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
							(!interfaceFont ||
							!isMacInterfaceSemanticPixel(logicalX, logicalY)))
						target[x] = current[logicalX];
				}
			}
		}

		// Apply the native blue layer to the panel artwork and non-semantic live
		// state. CODE 7 draws the interface words afterward with distinct
		// normal and selection colors, so they must not be quantized into the
		// eight washed background colors.
		byte washLUT[256];
		buildMacPanelWashLUT(_resources->getNativeInterfacePalette(), washLUT);
		for (int y = 0; y < kMacInterfaceHeight; ++y) {
			byte *target = (byte *)panel.getBasePtr(0, y);
			for (int x = 0; x < kMacInterfaceWidth; ++x)
				target[x] = washLUT[target[x]];
		}

		if (interfaceFont)
			drawMacInterfaceState(panel, *interfaceFont);

		for (int y = 0; y < kMacInterfaceHeight; ++y) {
			memcpy(_output.getBasePtr(kMacInterfaceX, interfaceY + y),
				panel.getBasePtr(0, y), kMacInterfaceWidth);
		}
	} else {
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

	if (_menus)
		_menus->draw();

	g_system->copyRectToScreen(_output.getPixels(), _output.pitch,
		0, 0, kMacScreenWidth, _output.h);
	g_system->updateScreen();
	_engine._screen->clearDirtyRects();
}

bool MacNebular::handleMacEvent(Common::Event &event) {
	return _menus && _menus->processEvent(event);
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
	for (int line = 0; line <= box->text_y; ++line) {
		const word tab = box->tab[line];
		const word plainTab = tab & ~(POPUP_UNDERLINE | POPUP_DOWNPIXEL);
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
	const int popupAreaHeight = _useOriginalMenus ?
		kMacSceneHeight : kMacLegacyScreenHeight;
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
		if (lines[line].tab & POPUP_UNDERLINE) {
			_popup.fillRect(Common::Rect(x, baseline + 1,
				MIN(width - 2, x + textWidth), baseline + 2),
				kMacBlackColor);
		}
	}

	_popupRect = Common::Rect(
		(kMacScreenWidth - width) / 2,
		popupAreaY + (popupAreaHeight - height) / 2,
		(kMacScreenWidth + width) / 2,
		popupAreaY + (popupAreaHeight + height) / 2);
	_popupActive = true;
	presentScreen(0);
}

void MacNebular::hidePopup() {
	if (!_popupActive)
		return;

	_popupActive = false;
	_popup.free();
	presentScreen(0);
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

bool RexNebularEngine::drawPopup() {
	if (!_macNebular)
		return false;

	_macNebular->showPopup();
	return true;
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
