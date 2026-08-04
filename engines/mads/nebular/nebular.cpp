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
#include "common/debug.h"
#include "common/system.h"
#include "graphics/font.h"
#include "graphics/managed_surface.h"
#include "mads/core/mps_installer.h"
#include "mads/core/attr.h"
#include "mads/core/config.h"
#include "mads/core/conv.h"
#include "mads/core/env.h"
#include "mads/core/game.h"
#include "mads/core/imath.h"
#include "mads/core/inter.h"
#include "mads/core/kernel.h"
#include "mads/core/matte.h"
#include "mads/core/mcga.h"
#include "mads/core/object.h"
#include "mads/core/pal.h"
#include "mads/core/rail.h"
#include "mads/core/screen.h"
#include "mads/core/sound.h"
#include "mads/core/text.h"
#include "mads/nebular/nebular.h"
#include "mads/nebular/console.h"
#include "mads/nebular/copy.h"
#include "mads/nebular/global.h"
#include "mads/nebular/mac_resources.h"
#include "mads/nebular/main.h"
#include "mads/nebular/popup.h"
#include "mads/nebular/mads/inventory.h"
#include "mads/nebular/mads/words.h"
#include "mads/nebular/sound/sound.h"
#include "mads/nebular/sound/mac_sound.h"
#include "mads/nebular/rooms/section1.h"
#include "mads/nebular/rooms/section2.h"
#include "mads/nebular/rooms/section3.h"
#include "mads/nebular/rooms/section4.h"
#include "mads/nebular/rooms/section5.h"
#include "mads/nebular/rooms/section6.h"
#include "mads/nebular/rooms/section7.h"
#include "mads/nebular/rooms/section8.h"

namespace MADS {
namespace RexNebular {

enum {
	kMacScreenWidth = 640,
	kMacSceneHeight = 312,
	kMacInterfaceWidth = 512,
	kMacInterfaceHeight = 88,
	kMacInterfaceX = (kMacScreenWidth - kMacInterfaceWidth) / 2,
	kMacScreenHeight = kMacSceneHeight + kMacInterfaceHeight,
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
	// without the checkerboard pattern used by the earlier approximation.
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

	// Keep the action caption face independent of room palette changes.
	displayPalette[kMacNormalTextColor].r = 63;
	displayPalette[kMacNormalTextColor].g = 63;
	displayPalette[kMacNormalTextColor].b = 63;
	mcga_setpal_range(&displayPalette, kMacNormalTextColor, 1);

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

RexNebularEngine::RexNebularEngine(OSystem *syst, const MADSGameDescription *gameDesc) :
		MADSEngine(syst, gameDesc) {
	// Initialize globals
	RexNebular::popup_init();
}

RexNebularEngine::~RexNebularEngine() {
	if (_macResources) {
		delete _soundManager;
		_soundManager = nullptr;
		env_set_resource_provider(nullptr);
		delete _macResources;
	}
}

Common::Error RexNebularEngine::run() {
	const bool isMacintosh = getPlatform() == Common::kPlatformMacintosh;
	if (isMacintosh)
		initGraphics(kMacScreenWidth, kMacScreenHeight);
	else
		initGraphics(320, 200);
	applyGameSettings();

	// The shared engine always renders into its original 320x200 work screen.
	// Macintosh presentation expands the scene and supplies its independent
	// native interface panel when the frame is sent to the backend.
	_screen = new Graphics::Screen(320, 200);
	scr_live.data = (byte *)_screen->getPixels();

	// Create a debugger console
	setDebugger(new Console());

	// Set up to read mpslabs installer archive if needed
	if (_gameDescription->desc.flags & GF_INSTALLER) {
		Common::Archive *arch = MpsInstaller::open("MPSLABS");
		if (arch)
			SearchMan.add("mpslabs", arch);
	}

	// Set up the platform resource and sound providers
	if (isMacintosh) {
		_macResources = new MacResourceProvider();
		if (!_macResources->load())
			return Common::Error(Common::kNoGameDataFoundError,
				"Could not open the Macintosh Rex resource files");
		env_set_resource_provider(_macResources);
		_soundManager = new Sound::MacSoundManager(_mixer, _soundFlag, _macResources);
	} else {
		_soundManager = new Sound::RexSoundManager(_mixer, _soundFlag, isDemo());
	}
	_soundManager->validate();

	// Run the game
	RexNebular::nebular_main();

	return Common::kNoError;
}

void RexNebularEngine::applyGameSettings() {
	Engine::applyGameSettings();

	// The Macintosh port's 640x400 large-window mode uses square pixels.
	// DOS-style 320x200 aspect correction would stretch its scene and native
	// interface vertically.
	if (getPlatform() == Common::kPlatformMacintosh &&
			g_system->hasFeature(OSystem::kFeatureAspectRatioCorrection) &&
			g_system->getFeatureState(OSystem::kFeatureAspectRatioCorrection)) {
		g_system->beginGFXTransaction();
		g_system->setFeatureState(OSystem::kFeatureAspectRatioCorrection, false);
		g_system->endGFXTransaction();
	}
}

Common::Point RexNebularEngine::screenToGame(const Common::Point &point) const {
	if (getPlatform() != Common::kPlatformMacintosh)
		return MADSEngine::screenToGame(point);

	if (point.y >= 0 && point.y < kMacSceneHeight)
		return Common::Point(CLIP<int>(point.x / 2, 0, 319), point.y / 2);

	if (point.y >= kMacSceneHeight && point.y < kMacScreenHeight &&
			point.x >= kMacInterfaceX &&
			point.x < kMacInterfaceX + kMacInterfaceWidth) {
		return Common::Point(
			(point.x - kMacInterfaceX) * 320 / kMacInterfaceWidth,
			156 + (point.y - kMacSceneHeight) / 2);
	}

	// The native interface is narrower than the scene. Its side gutters are
	// intentionally inactive rather than being stretched into hotspots.
	return Common::Point(-1, -1);
}

Common::Point RexNebularEngine::gameToScreen(const Common::Point &point) const {
	if (getPlatform() != Common::kPlatformMacintosh)
		return MADSEngine::gameToScreen(point);

	if (point.y < 156)
		return Common::Point(point.x * 2, point.y * 2);

	return Common::Point(kMacInterfaceX + point.x * kMacInterfaceWidth / 320,
		kMacSceneHeight + (point.y - 156) * 2);
}

void RexNebularEngine::presentScreen(int shakeOffset) {
	if (getPlatform() != Common::kPlatformMacintosh) {
		MADSEngine::presentScreen(shakeOffset);
		return;
	}

	_macOutput.resize(kMacScreenWidth * kMacScreenHeight);
	memset(_macOutput.data(), kMacBlackColor, _macOutput.size());

	// Native large-window mode doubles the 320x156 scene in both axes.
	for (int y = 0; y < 156; ++y) {
		const byte *source = (const byte *)_screen->getBasePtr(0, y);
		byte *line1 = _macOutput.data() + (y * 2) * kMacScreenWidth;
		byte *line2 = line1 + kMacScreenWidth;
		for (int x = 0; x < 320; ++x) {
			const byte color = source[(x + shakeOffset) % 320];
			line1[x * 2] = color;
			line1[x * 2 + 1] = color;
		}
		memcpy(line2, line1, kMacScreenWidth);
	}

	const Graphics::Surface *nativeInterface =
		_macResources ? _macResources->getNativeInterface() : nullptr;
	const Graphics::Surface *logicalInterface =
		_macResources ? _macResources->getLogicalInterface() : nullptr;
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
			_macResources ? _macResources->getInterfaceFont() : nullptr;

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
					(const byte *)_screen->getBasePtr(0, 156 + logicalY);
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

		if (interfaceFont)
			drawMacInterfaceState(panel, *interfaceFont);

		// Apply the blue layer after composing artwork, live state, controls,
		// and text so the whole native panel receives one uniform treatment.
		byte washLUT[256];
		buildMacPanelWashLUT(_macResources->getNativeInterfacePalette(), washLUT);
		for (int y = 0; y < kMacInterfaceHeight; ++y) {
			byte *target = (byte *)panel.getBasePtr(0, y);
			for (int x = 0; x < kMacInterfaceWidth; ++x)
				target[x] = washLUT[target[x]];
		}

		for (int y = 0; y < kMacInterfaceHeight; ++y) {
			memcpy(_macOutput.data() +
				(kMacSceneHeight + y) * kMacScreenWidth + kMacInterfaceX,
				panel.getBasePtr(0, y), kMacInterfaceWidth);
		}
	} else {
		// Before the native panel is loaded, retain a structurally equivalent
		// fallback by scaling the shared 320x44 interface into its Mac bounds.
		for (int y = 0; y < kMacInterfaceHeight; ++y) {
			const byte *source = (const byte *)_screen->getBasePtr(0, 156 + y / 2);
			byte *target = _macOutput.data() +
				(kMacSceneHeight + y) * kMacScreenWidth + kMacInterfaceX;
			for (int x = 0; x < kMacInterfaceWidth; ++x)
				target[x] = source[x * 320 / kMacInterfaceWidth];
		}
	}

	if (_macPopupActive && !_macPopup.empty()) {
		for (int y = 0; y < _macPopup.h; ++y) {
			const int targetY = _macPopupRect.top + y;
			if (targetY < 0 || targetY >= kMacScreenHeight)
				continue;

			const int targetX = MAX<int>(0, _macPopupRect.left);
			const int sourceX = targetX - _macPopupRect.left;
			const int width = MIN<int>(_macPopup.w - sourceX,
				kMacScreenWidth - targetX);
			if (width > 0)
				memcpy(_macOutput.data() + targetY * kMacScreenWidth + targetX,
					_macPopup.getBasePtr(sourceX, y), width);
		}
	}

	if (!_macLayoutLogged) {
		debug(2, "Presenting Macintosh Rex as 640x312 scene plus centered 512x88 interface");
		_macLayoutLogged = true;
	}

	g_system->copyRectToScreen(_macOutput.data(), kMacScreenWidth,
		0, 0, kMacScreenWidth, kMacScreenHeight);
	g_system->updateScreen();
	_screen->clearDirtyRects();
}

void RexNebularEngine::showMacPopup() {
	if (getPlatform() != Common::kPlatformMacintosh || !_macResources || !box)
		return;

	const Graphics::Font *font = _macResources->getDialogFont();
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

	const int height = CLIP<int>((int)lines.size() * 12 + 20, 20,
		kMacScreenHeight - 2);
	_macPopup.create(width, height, Graphics::PixelFormat::createFormatCLUT8());
	_macPopup.fillRect(Common::Rect(width, height), kMacPopupColor);
	_macPopup.frameRect(Common::Rect(width, height), kMacBlackColor);

	const int ascent = font->getFontAscent() >= 0 ?
		font->getFontAscent() : font->getFontHeight();
	for (uint line = 0; line < lines.size(); ++line) {
		const int baseline = 22 + line * 12 +
			((lines[line].tab & POPUP_DOWNPIXEL) ? 1 : 0);
		if (baseline - ascent >= height)
			break;

		if (lines[line].tab == POPUP_BAR) {
			_macPopup.fillRect(Common::Rect(2, baseline - 5,
				width - 2, baseline - 4), kMacBlackColor);
			continue;
		}

		const int textWidth = font->getStringWidth(lines[line].text);
		const int x = (lines[line].tab & POPUP_UNDERLINE) ?
			(width - textWidth) / 2 :
			10 + (lines[line].tab &
				~(POPUP_UNDERLINE | POPUP_DOWNPIXEL)) * 3 / 4;
		font->drawString(&_macPopup, lines[line].text, x, baseline - ascent,
			MAX(0, width - x - 2), kMacBlackColor);
		if (lines[line].tab & POPUP_UNDERLINE) {
			_macPopup.fillRect(Common::Rect(x, baseline + 1,
				MIN(width - 2, x + textWidth), baseline + 2),
				kMacBlackColor);
		}
	}

	_macPopupRect = Common::Rect(
		(kMacScreenWidth - width) / 2,
		(kMacScreenHeight - height) / 2,
		(kMacScreenWidth + width) / 2,
		(kMacScreenHeight + height) / 2);
	_macPopupActive = true;
	presentScreen(0);
}

void RexNebularEngine::hideMacPopup() {
	if (!_macPopupActive)
		return;

	_macPopupActive = false;
	_macPopup.free();
	presentScreen(0);
}

int RexNebularEngine::main_copy_verify() {
	return global_copy_verify();
}

void RexNebularEngine::global_init_code() {
	RexNebular::global_init_code();
}

void RexNebularEngine::section_music(int section_num) {
	switch (section_num) {
	case 1: Rooms::section_1_music(); break;
	case 2: Rooms::section_2_music(); break;
	case 3: Rooms::section_3_music(); break;
	case 4: Rooms::section_4_music(); break;
	case 5: Rooms::section_5_music(); break;
	case 6: Rooms::section_6_music(); break;
	case 7: Rooms::section_7_music(); break;
	case 8: Rooms::section_8_music(); break;
	}
}

void RexNebularEngine::global_section_constructor() {
	RexNebular::global_section_constructor();
}

void RexNebularEngine::syncRoom(Common::Serializer &s) {
	RexNebular::sync_room(s);
}

void RexNebularEngine::global_daemon_code() {
	if (player.walker_visible && player.commands_allowed && !player.walking &&
		(player.facing == player.turn_to_facing)) {


		if (kernel.clock >= READGLOBAL32(kWalkerTiming)) {
			if (player.stop_walker_pointer == 0) {
				int randomVal = getRandomNumber(29999);
				if (global[kSexOfRex] == REX_MALE) {
					switch (player.facing) {
					case FACING_SOUTHWEST:
					case FACING_SOUTHEAST:
					case FACING_NORTHWEST:
					case FACING_NORTHEAST:
						if (randomVal < 200) {
							player_add_stop_walker(-1, 0);
							player_add_stop_walker(1, 0);
						}
						break;

					case FACING_WEST:
					case FACING_EAST:
						if (randomVal < 500) {
							for (int count = 0; count < 10; ++count) {
								player_add_stop_walker(1, 0);
							}
						}
						break;

					case FACING_SOUTH:
						if (randomVal < 500) {
							for (int count = 0; count < 10; ++count) {
								player_add_stop_walker((randomVal < 250) ? 1 : 2, 0);
							}
						} else if (randomVal < 750) {
							for (int count = 0; count < 5; ++count) {
								player_add_stop_walker(1, 0);
							}

							player_add_stop_walker(0, 0);
							player_add_stop_walker(0, 0);

							for (int count = 0; count < 5; ++count) {
								player_add_stop_walker(2, 0);
							}
						}
						break;

					default:
						break;
					}
				}
			}

			WRITEGLOBAL32(kWalkerTiming, READGLOBAL32(kWalkerTiming) + 6);
		}
	}

	// Below is countdown to set the timebomb off in room 604
	if (global[kTimebombStatus] == TIMEBOMB_ACTIVATED) {
		int diff = kernel.clock - READGLOBAL32(kTimebombClock);
		if ((diff >= 0) && (diff <= 60)) {
			WRITEGLOBAL32(kTimebombTimer, READGLOBAL32(kTimebombTimer) + diff);
		} else {
			WRITEGLOBAL32(kTimebombTimer, READGLOBAL32(kTimebombTimer) + 1);
		}

		WRITEGLOBAL32(kTimebombClock, kernel.clock);
	}
}

void RexNebularEngine::global_pre_parser_code() {
	if (player_said_1(look) || player_said_1(throw))
		player.need_to_walk = false;
}

void RexNebularEngine::global_verb_filter() {
	// Switches the 'look at' used with the binoculars to a standard look verb,
	// as expected by the room scripts in at least room 202
	if (player_verb == words_look_at)
		player_verb = words_look;
}

void RexNebularEngine::showRecipe() {
	int count;

	for (count = 0; count < 4; count++) {
		switch (global[kIngredientQuantity + count]) {
		case 0:
			text_index[count] = words_drop;
			break;
		case 1:
			text_index[count] = words_dollop;
			break;
		case 2:
			text_index[count] = words_dash;
			break;
		case 3:
			text_index[count] = words_splash;
			break;
		default:
			break;
		}
	}

	for (count = 0; count < 4; count++) {
		switch (global[kIngredientList + count]) {
		case 0:
			text_index[count + 4] = words_alcohol;
			break;
		case 1:
			text_index[count + 4] = words_lecithin;
			break;
		case 2:
			text_index[count + 4] = words_petrox;
			break;
		case 3:
			text_index[count + 4] = words_formaldehyde;
			break;
		default:
			break;
		}
	}

	text_show(401);
}

void RexNebularEngine::global_parser_code() {
	int id;

	if (player_said_1(smell) && room_id > 103 && room_id < 111) {
		text_show(440);
	} else if (player_said_1(eat) && room_id > 103 && room_id < 111) {
		text_show(441);
	} else if (player_said_2(smell, burger)) {
		text_show(442);
	} else if (player_said_2(eat, burger)) {
		text_show(443);
	} else if (player_said_2(smell, stuffed_fish)) {
		text_show(444);
	} else if (player_said_2(eat, stuffed_fish)) {
		text_show(445);
	} else if (player_said_2(wear, rebreather)) {
		text_show(room_id > 103 && room_id < 111 ? 446 : 447);
	} else if (player_said_2(set, timer_module)) {
		text_show(448);
	} else if (player_said_2(nibble_on, big_leaves)) {
		text_show(449);
	} else if (player_said_2(lick, poison_darts)) {
		text_show(450);
	} else if (player_said_2(eat, twinkifruit)) {
		inter_move_object(OBJ_TWINKIFRUIT, NOWHERE);
		text_show(451);
	} else if (player_said_2(gorge_on, twinkifruit)) {
		inter_move_object(OBJ_TWINKIFRUIT, NOWHERE);
		text_show(452);
	} else if (player_said_1(gnaw_on)) {
		text_show(453);
	} else if (player_said_2(massage, audio_tape)) {
		text_show(454);
	} else if (player_said_2(mangle, credit_chip)) {
		text_show(455);
	} else if (player_said_2(fondle, charge_cases)) {
		text_show(456);
	} else if (player_said_2(rub, bomb)) {
		text_show(457);
	} else if (player_said_2(set, timebomb)) {
		text_show(458);
		// Original also had a second unreachable block for: text_show(427);
	} else if (player_said_2(guzzle, alien_liquor)) {
		text_show(459);
	} else if (player_said_2(smash, target_module)) {
		text_show(460);
	} else if (player_said_1(juggle)) {
		text_show(461);
	} else if (player_said_2(apply, polycement)) {
		text_show(462);
	} else if (player_said_2(sniff, polycement)) {
		text_show(465);
	} else if (player_said_2(tie, fishing_line)) {
		text_show(463);
	} else if (player_said_2(attach, fishing_line)) {
		text_show(463);
	} else if (player_said_1(unlock)) {
		text_show(464);
	} else if (player_said_1(reflect)) {
		text_show(466);
	} else if (player_said_2(gaze_into, rearview_mirror)) {
		text_show(467);
	} else if (player_said_2(eat, chicken_bomb)) {
		text_show(469);
	} else if (player_said_2(break, vase)) {
		text_show(471);
	} else if (player_said_2(shake_hands, guards_arm2)) {
		text_show(472);
	} else if (player_said_2(read, log)) {
		text_show(473);
	} else if (player_said_2(rub, bombs)) {
		text_show(474);
	} else if (player_said_2(drink, formaldehyde)) {
		text_show(475);
	} else if (player_said_2(drink, petrox)) {
		text_show(476);
	} else if (player_said_2(drink, lecithin)) {
		text_show(477);
	} else if (player_said_3(put, poison_darts, plant_stalk) && player_has(OBJ_POISON_DARTS)
		&& player_has(OBJ_PLANT_STALK)) {
		inter_give_to_player(OBJ_BLOWGUN);
		inter_move_object(OBJ_PLANT_STALK, NOWHERE);
		global[kBlowgunStatus] = 0;
		object_examine(OBJ_BLOWGUN, 809);
	} else if (player_said_3(put, poison_darts, blowgun) && player_has(OBJ_POISON_DARTS)
		&& player_has(OBJ_BLOWGUN)) {
		text_show(433);
	} else if (player_said_1(deface) && player_said_1(fold) && player_said_1(mutilate)) {
		text_show(434);
	} else if (player_said_1(spindle)) {
		text_show(479);
	} else if ((player_said_1(read) || player_said_1(look_at) || player_said_1(look)) &&
		player_said_1(note) && player_has(OBJ_NOTE)) {
		inter_move_object(OBJ_NOTE, NOWHERE);
		inter_give_to_player(OBJ_COMBINATION);
		object_examine(OBJ_COMBINATION, 851);
	} else if ((player_said_1(look) || player_said_1(read)) &&
		((id = object_named(inter_main_noun)) > 0 ||
			(inter_second_noun > 0 &&
				(id = object_named(inter_second_noun)))) &&
		player_has(id)) {
		if (id == OBJ_REPAIR_LIST) {
			text_index[0] = global[kTeleporterCode + 7];
			text_index[1] = global[kTeleporterCode + 8];
			text_index[2] = global[kTeleporterCode + 6];
			text_index[3] = global[kTeleporterCode + 9];
			text_index[4] = global[kTeleporterCode + 0];
			text_index[5] = global[kTeleporterCode + 1];
			text_index[6] = global[kTeleporterCode + 4];
			text_index[7] = global[kTeleporterCode + 5];
			text_index[8] = global[kTeleporterCode + 2];

			object_examine(id, 402);
		} else {
			int messageId = 800 + id;
			if ((id == OBJ_CHARGE_CASES) && object_check_quality(OBJ_CHARGE_CASES, 3) != 0) {
				messageId = 860;
			}

			if (id == OBJ_TAPE_PLAYER && object[OBJ_AUDIO_TAPE].location == OBJ_TAPE_PLAYER)
				messageId = 867;

			if (id == 32 && object[OBJ_FISHING_LINE].location == 3)
				messageId = 862;

			if (id == OBJ_BOTTLE && global[kBottleStatus] != 0)
				messageId = 862 + global[kBottleStatus];

			if (id == OBJ_PHONE_HANDSET && global[kHandsetCellStatus])
				messageId = 861;

			object_examine(id, messageId);
		}
	} else if (player_said_3(put, burger, dead_fish)) {
		if (player_has(OBJ_BURGER) || player_has(OBJ_DEAD_FISH)) {
			inter_take_from_player(OBJ_DEAD_FISH, NOWHERE);
			inter_take_from_player(OBJ_BURGER, NOWHERE);
			inter_give_to_player(OBJ_STUFFED_FISH);
			object_examine(OBJ_STUFFED_FISH, 803);
		}
	} else if (player_said_3(put, audio_tape, tape_player) && player_has(OBJ_AUDIO_TAPE) &&
		player_has(OBJ_TAPE_PLAYER)) {
		inter_move_object(OBJ_AUDIO_TAPE, OBJ_TAPE_PLAYER);
	} else if (player_said_2(activate, tape_player) && player_has(OBJ_TAPE_PLAYER)) {
		if (object[OBJ_AUDIO_TAPE].location == OBJ_TAPE_PLAYER) {
			showRecipe();
		} else {
			text_show(406);
		}
	} else if (player_said_2(eject, tape_player) && player_has(OBJ_TAPE_PLAYER)) {
		if (object[OBJ_AUDIO_TAPE].location == OBJ_TAPE_PLAYER) {
			inter_give_to_player(OBJ_AUDIO_TAPE);
		} else {
			text_show(407);
		}
	} else if (player_said_2(disassemble, tape_player)) {
		text_show(408);
	} else if (player_said_2(activate, remote)) {
		text_show(global[kTopButtonPushed] ? 502 : 501);
	} else if ((player_said_3(attach, detonators, charge_cases) || player_said_3(put, detonators, charge_cases)) &&
		player_has(OBJ_DETONATORS) && player_has(OBJ_CHARGE_CASES)) {
		if (object_check_quality(OBJ_CHARGE_CASES, 3)) {
			inter_move_object(OBJ_CHARGE_CASES, NOWHERE);
			inter_move_object(OBJ_DETONATORS, NOWHERE);
			inter_give_to_player(OBJ_BOMBS);
			object_examine(OBJ_BOMBS, 403);
		} else {
			text_show(405);
		}
	} else if (player_said_2(attach, detonators)) {
		text_show(470);
	} else if ((player_said_3(attach, timer_module, bombs) || player_said_3(put, timer_module, bombs) || player_said_3(attach, timer_module, bomb)
		|| player_said_3(put, timer_module, bomb)) && player_has(OBJ_TIMER_MODULE) && (
			player_has(OBJ_BOMBS) || player_has(OBJ_BOMB))) {
		if (player_has(OBJ_BOMBS)) {
			inter_move_object(OBJ_BOMBS, NOWHERE);
			inter_give_to_player(OBJ_BOMB);
		} else {
			inter_move_object(OBJ_BOMB, NOWHERE);
		}

		inter_move_object(OBJ_TIMER_MODULE, NOWHERE);
		inter_give_to_player(OBJ_TIMEBOMB);
		object_examine(OBJ_TIMEBOMB, 404);
	} else if (player_said_2(fondle, plant_stalk)) {
		text_show(410);
	} else if (player_said_2(empty, bottle)) {
		global[kBottleStatus] = 0;
		text_show(432);
	} else if (player_said_2(disassemble, fishing_rod)) {
		if (object[OBJ_FISHING_LINE].location == 3) {
			inter_give_to_player(OBJ_FISHING_LINE);
			object_examine(OBJ_FISHING_LINE, 409);
		} else {
			text_show(428);
		}
	} else if (player_said_2(disassemble, penlight)) {
		switch (global[kPenlightCellStatus]) {
		case 1:
		case 2:
			inter_give_to_player(OBJ_DURAFAIL_CELLS);
			object_examine(OBJ_DURAFAIL_CELLS, 412);
			break;
		case 3:
			inter_give_to_player(OBJ_PHONE_CELLS);
			object_examine(OBJ_DURAFAIL_CELLS, 413);
			break;
		case 5:
			inter_give_to_player(OBJ_DURAFAIL_CELLS);
			object_examine(OBJ_DURAFAIL_CELLS, 411);
			break;
		case 6:
			inter_give_to_player(OBJ_DURAFAIL_CELLS);
			object_examine(OBJ_DURAFAIL_CELLS, 429);
			break;
		default:
			text_show(478);
			break;
		}

		global[kPenlightCellStatus] = 0;

	} else if (player_said_2(disassemble, phone_handset)) {
		switch (global[kHandsetCellStatus]) {
		case 1:
			inter_give_to_player(OBJ_DURAFAIL_CELLS);
			object_examine(OBJ_DURAFAIL_CELLS,
				game.difficulty != DIFFICULTY_HARD || global[kDurafailRecharged] ? 415 : 414);
			global[kDurafailRecharged] = true;
			break;
		case 2:
			inter_give_to_player(OBJ_DURAFAIL_CELLS);
			if (game.difficulty == DIFFICULTY_HARD) {
				object_examine(OBJ_DURAFAIL_CELLS, 416);
			}
			break;
		case 3:
			inter_give_to_player(OBJ_PHONE_CELLS);
			object_examine(OBJ_PHONE_CELLS, 418);
			break;
		case 4:
			inter_give_to_player(OBJ_PHONE_CELLS);
			object_examine(OBJ_PHONE_CELLS, 417);
			break;
		default:
			text_show(478);
			break;
		}

		global[kHandsetCellStatus] = 0;

	} else if (player_said_3(put, phone_cells, penlight)) {
		if (global[kPenlightCellStatus] == 0) {
			global[kPenlightCellStatus] = 3;
			inter_move_object(OBJ_PHONE_CELLS, NOWHERE);
			text_show(419);
		} else {
			text_show(420);
		}
	} else if (player_said_3(put, phone_cells, phone_handset)) {
		if (global[kHandsetCellStatus] == 0) {
			global[kHandsetCellStatus] = 3;
			inter_move_object(OBJ_PHONE_CELLS, NOWHERE);
			text_show(421);
		} else {
			text_show(422);
		}
	} else if (player_said_3(put, durafail_cells, penlight)) {
		if (global[kPenlightCellStatus]) {
			text_show(424);
		} else {
			inter_move_object(OBJ_DURAFAIL_CELLS, NOWHERE);
			global[kPenlightCellStatus] = game.difficulty != DIFFICULTY_HARD || global[kDurafailRecharged] ? 1 : 2;
			text_show(423);
		}
	} else if (player_said_3(put, durafail_cells, phone_handset)) {
		if (global[kHandsetCellStatus]) {
			text_show(426);
		} else {
			inter_move_object(OBJ_DURAFAIL_CELLS, NOWHERE);
			global[kHandsetCellStatus] = game.difficulty != DIFFICULTY_HARD || global[kDurafailRecharged] ? 1 : 2;
			text_show(425);
		}
	} else if (player_said_3(put, bomb, chicken) || player_said_3(put, bombs, chicken)) {
		inter_move_object(OBJ_CHICKEN, NOWHERE);
		if (player_has(OBJ_BOMBS)) {
			inter_move_object(OBJ_BOMBS, NOWHERE);
			inter_give_to_player(OBJ_BOMB);
		} else {
			inter_move_object(OBJ_BOMB, NOWHERE);
		}
		inter_give_to_player(OBJ_CHICKEN_BOMB);
		object_examine(OBJ_CHICKEN_BOMB, 430);
	} else {
		goto done;
	}

	player.command_ready = false;

done:
	;
}

void RexNebularEngine::global_error_code() {
	int randVal = getRandomNumber(1, 1000);

	if (player_said_2(throw, bomb) || player_said_2(throw, bombs)
		|| player_said_2(throw, timebomb) || player_said_2(throw, chicken_bomb))
		text_show(42);
	else if (player_said_1(disassemble))
		text_show(435);
	else if ((player_said_2(eat, dead_fish) || player_said_2(eat, stuffed_fish)) && player_has(object_named(inter_main_noun)))
		text_show(12);
	else if ((player_said_2(smell, dead_fish) || player_said_2(smell, stuffed_fish)) && player_has(object_named(inter_main_noun)))
		text_show(13);
	else if (player_said_2(eat, chicken) && player_has(OBJ_CHICKEN))
		text_show(912);
	else if ((player_said_1(shoot) || player_said_1(hose_down)) && player_said_1(blowgun)) {
		if ((room_id >= 104) && (room_id <= 111))
			text_show(38);
		else if (player_said_1(piranha))
			text_show(41);
		else if (player_said_1(chicken) || player_said_1(vulture) || player_said_1(spider)
			|| player_said_1(yellow_bird) || player_said_1(swooping_creature) || player_said_1(captive_creature)) {
			text_show(40);
		} else
			text_show(39);
	} else if (player_said_1(talkto)) {
		global[kTalkInanimateCount] = (global[kTalkInanimateCount] + 1) % 16;
		if (!global[kTalkInanimateCount]) {
			text_show(2);
		} else {
			static char msg[32];
			Common::sprintf_s(msg, "\"Greetings, %s!\"", vocab_string(inter_main_noun));
			kernel_message_purge();
			kernel_message_add(msg, 0, 0, 0x1110, 120, 0, 34);
		}
	} else if (player_said_3(give, door, ceiling) || player_said_2(close, chair))
		text_show(3);
	else if (player_said_1(throw)) {
		int objId = object_named(inter_main_noun);
		if (objId < 0)
			text_show(4);
		else if (object[objId].location != 2)
			text_show(5);
		else
			text_show(6);
	} else if (player_said_1(look)) {
		if (player_said_1(binoculars) && (inter_second_noun > 0))
			text_show(10);
		else if (randVal < 600)
			text_show(7);
		else
			text_show(21);
	} else if (player_said_1(take)) {
		int objId = object_named(inter_main_noun);
		if (player_has(objId))
			text_show(16);
		else if (randVal <= 333)
			text_show(8);
		else if (randVal <= 666)
			text_show(22);
		else
			text_show(23);
	} else if (player_said_1(close)) {
		if (randVal <= 333)
			text_show(9);
		else
			text_show(33);
	} else if (player_said_1(open)) {
		if (randVal <= 500)
			text_show(30);
		else if (randVal <= 750)
			text_show(31);
		else
			text_show(32);
	} else if (player_said_1(pull))
		text_show(18);
	else if (player_said_1(push)) {
		if (randVal < 750)
			text_show(19);
		else
			text_show(20);
	} else if (player_said_1(put)) {
		int objId = object_named(inter_main_noun);
		if (player_has(objId))
			text_show(25);
		else
			text_show(24);
	} else if (player_said_1(give)) {
		int objId = object_named(inter_main_noun);
		if (!player_has(objId))
			text_show(26);
		else if (randVal <= 500)
			text_show(28);
		else
			text_show(29);
	} else if (!player_said_1(walkto) && !player_said_1(walk_across) && !player_said_1(walk_towards) && !player_said_1(walk_down)
		&& !player_said_1(swim_to) && !player_said_1(swim_across) && !player_said_1(swim_into) && !player_said_1(swim_through)
		&& !player_said_1(swim_under)) {
		if (randVal <= 100)
			text_show(36);
		else if (randVal <= 200)
			text_show(1);
		else if (randVal <= 475)
			text_show(34);
		else if (randVal <= 750)
			text_show(35);
		else
			text_show(37);
	}
}

void RexNebularEngine::global_sound_driver() {
	Common::strcpy_s(kernel.sound_driver, "/");
	env_catint(kernel.sound_driver, new_section, 1);
}

} // namespace RexNebular
} // namespace MADS
