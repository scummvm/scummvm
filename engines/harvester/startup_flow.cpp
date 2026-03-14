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

#include "harvester/startup_flow.h"

#include "common/debug.h"
#include "common/endian.h"
#include "common/events.h"
#include "common/formats/ini-file.h"
#include "common/memstream.h"
#include "common/system.h"
#include "graphics/font.h"
#include "graphics/fontman.h"
#include "graphics/framelimiter.h"
#include "harvester/detection.h"
#include "harvester/harvester.h"
#include "harvester/resources.h"
#include "harvester/startup_art.h"
#include "harvester/startup_script.h"

namespace Harvester {

namespace {

static const char *const kQuickTipsPath = "ADJHEAD.RCS";
static const char *const kMenuPath = "MENU.INI";
static const char *const kMenuSectionName = "menu";

static const int kInventoryX = 64;
static const int kInventoryY = 48;
static const int kLogoX = 160;
static const int kLogoY = 0;
static const int kQuickTipsOverlayX = 167;
static const int kQuickTipsOverlayY = 200;

static const int kQuickTipTextX = 180;
static const int kQuickTipTextY = 228;
static const int kQuickTipTextWidth = 280;

static const int kMenuStartY = 100;
static const int kMenuLineSpacing = 28;
static const uint kCursorSequence7FirstFrame = 70;
static const uint kCursorSequence7LastFrame = 79;
static const uint kCursorAnimationRate = 10;
static const uint32 kCursorAnimationTickMs = 100 / kCursorAnimationRate;
static const uint32 kPaletteFadeTickMs = 4;
static const float kPaletteFadeStep = 0.1f;
static const float kPaletteBrightnessBlack = 0.0f;

static const byte kTextColorNormal = 255;
static const byte kTextColorHover = 251;
static const byte kTextColorDim = 248;
static const byte kShadowColor = 0;
static const byte kPanelFillColor = 1;

struct StartupSceneOverlay {
	StartupObjectRecord object;
	IndexedBitmap bitmap;
};

struct StartupRoomSceneResources {
	StartupRoomSetupState state;
	byte palette[256 * 3] = { 0 };
	IndexedBitmap background;
	Common::Array<StartupSceneOverlay> overlays;
	float targetPaletteBrightness = 1.0f;
};

static byte expand6BitColor(byte value) {
	return (value * 255 + 31) / 63;
}

static Common::Rect quickTipsExitRect() {
	return Common::Rect(180, 280, 238, 291);
}

static Common::Rect quickTipsNextRect() {
	return Common::Rect(420, 280, 492, 291);
}

static Common::Rect quickTipsToggleRect() {
	return Common::Rect(258, 280, 366, 291);
}

static void blitBitmap(Graphics::Screen &screen, const IndexedBitmap &bitmap, int x, int y) {
	if (!bitmap.isValid())
		return;

	screen.copyRectToSurface(bitmap.pixels.data(), bitmap.width, x, y, bitmap.width, bitmap.height);
}

static void blitAnimationFrame(Graphics::Screen &screen, const Common::Array<AbmFrame> &frames, uint frameIndex, int x, int y) {
	if (frameIndex >= frames.size() || !frames[frameIndex].isValid())
		return;

	const AbmFrame &frame = frames[frameIndex];
	blitBitmap(screen, frame, x + frame.xOffset, y + frame.yOffset);
}

static void drawShadowedString(Graphics::Screen &screen, const Graphics::Font &font, const Common::String &text,
		int x, int y, int width, byte color, Graphics::TextAlign align = Graphics::kTextAlignLeft) {
	font.drawString(&screen, text, x + 1, y + 1, width, kShadowColor, align);
	font.drawString(&screen, text, x, y, width, color, align);
}

static void drawWrappedShadowedText(Graphics::Screen &screen, const Graphics::Font &font, const Common::String &text,
		int x, int y, int width, byte color) {
	Common::Array<Common::String> lines;
	font.wordWrapText(text, width, lines);

	const int lineHeight = font.getFontHeight() + 2;
	for (uint i = 0; i < lines.size(); ++i)
		drawShadowedString(screen, font, lines[i], x, y + i * lineHeight, width, color);
}

static void buildScaledPalette(const byte *source, float brightness, byte *dest) {
	memset(dest, 0, 256 * 3);
	if (!source)
		return;

	for (uint color = 1; color < 256; ++color) {
		for (uint channel = 0; channel < 3; ++channel) {
			const uint index = color * 3 + channel;
			const int scaled = (int)(source[index] * brightness + 0.5f);
			dest[index] = (byte)MIN<int>(scaled, 255);
		}
	}
}

static void setScaledPalette(Graphics::Screen &screen, const byte *palette, float brightness) {
	byte scaledPalette[256 * 3];
	buildScaledPalette(palette, brightness, scaledPalette);
	screen.setPalette(scaledPalette);
}

static bool loadPaletteResource(ResourceManager &resources, const Common::String &path, byte *dest) {
	Common::Array<byte> data;
	if (!resources.loadFile(path, data) || data.size() < 256 * 3)
		return false;

	for (uint i = 0; i < 256 * 3; ++i)
		dest[i] = expand6BitColor(data[i]);

	return true;
}

static bool loadBitmapResource(ResourceManager &resources, const Common::String &path, IndexedBitmap &bitmap) {
	Common::Array<byte> data;
	if (!resources.loadFile(path, data) || data.size() < 12)
		return false;

	bitmap.width = READ_LE_UINT32(data.data());
	bitmap.height = READ_LE_UINT32(data.data() + 4);
	const uint32 pixelCount = bitmap.width * bitmap.height;
	if (bitmap.width == 0 || bitmap.height == 0 || data.size() < 12 + pixelCount)
		return false;

	bitmap.pixels.resize(pixelCount);
	memcpy(bitmap.pixels.data(), data.data() + 12, pixelCount);
	return true;
}

static Common::Rect getObjectBounds(const StartupObjectRecord &object) {
	if (object.right > object.left && object.bottom > object.top)
		return Common::Rect(object.left, object.top, object.right, object.bottom);

	return Common::Rect();
}

static void queueDrawableRoomObject(const StartupObjectRecord &object, Common::Array<StartupObjectRecord> &drawableObjects) {
	if (object.resourcePath.empty() || !object.resourcePath.hasSuffixIgnoreCase(".BM"))
		return;

	for (const StartupObjectRecord &drawable : drawableObjects) {
		if (drawable.ownerOrRoom.equalsIgnoreCase(object.ownerOrRoom) &&
			drawable.objectName.equalsIgnoreCase(object.objectName)) {
			return;
		}
	}

	drawableObjects.push_back(object);
}

static bool loadRoomSceneResources(const StartupRoomSetupState &state, ResourceManager &resources, StartupRoomSceneResources &scene) {
	scene = StartupRoomSceneResources();
	scene.state = state;
	scene.targetPaletteBrightness = state.paletteBrightness;
	if (!loadPaletteResource(resources, state.palettePath, scene.palette) ||
		!loadBitmapResource(resources, state.backgroundPath, scene.background)) {
		return false;
	}

	Common::Array<StartupObjectRecord> drawableObjects;
	for (const StartupObjectRecord &object : state.roomObjects) {
		if (object.visible)
			queueDrawableRoomObject(object, drawableObjects);
	}
	for (const StartupObjectRecord &object : state.activeObjects)
		queueDrawableRoomObject(object, drawableObjects);

	for (const StartupObjectRecord &object : drawableObjects) {
		StartupSceneOverlay overlay;
		overlay.object = object;
		if (!loadBitmapResource(resources, object.resourcePath, overlay.bitmap))
			continue;
		scene.overlays.push_back(overlay);
	}

	return true;
}

static void drawRoomScene(Graphics::Screen &screen, const StartupRoomSceneResources &scene, float brightness) {
	setScaledPalette(screen, scene.palette, brightness);
	screen.fillRect(screen.getBounds(), 0);
	blitBitmap(screen, scene.background, 0, 0);
	for (const StartupSceneOverlay &overlay : scene.overlays)
		blitBitmap(screen, overlay.bitmap, overlay.object.left, overlay.object.top);
}

static const StartupObjectRecord *findObjectAtPoint(const Common::Array<StartupObjectRecord> &objects, const Common::Point &point) {
	for (int i = (int)objects.size() - 1; i >= 0; --i) {
		const StartupObjectRecord &object = objects[(uint)i];
		if (!object.active)
			continue;

		const Common::Rect bounds = getObjectBounds(object);
		if (!bounds.isEmpty() && bounds.contains(point))
			return &object;
	}

	return nullptr;
}

static const StartupObjectRecord *findRoomObjectAtPoint(const StartupRoomSetupState &state, const Common::Point &point) {
	const StartupObjectRecord *object = findObjectAtPoint(state.activeObjects, point);
	if (object)
		return object;

	return findObjectAtPoint(state.roomObjects, point);
}

static Common::String trimAsciiLine(const Common::String &value) {
	uint start = 0;
	uint end = value.size();

	while (start < end && (value[start] == ' ' || value[start] == '\t'))
		++start;
	while (end > start && (value[end - 1] == ' ' || value[end - 1] == '\t' || value[end - 1] == '\r'))
		--end;

	return value.substr(start, end - start);
}

static uint getStartupCursorSequenceFirstFrame(const StartupArt &art) {
	const Common::Array<AbmFrame> &frames = art.getPointerFrames();
	if (frames.empty())
		return 0;

	return MIN<uint>(kCursorSequence7FirstFrame, frames.size() - 1);
}

static uint getStartupCursorSequenceLastFrame(const StartupArt &art) {
	const Common::Array<AbmFrame> &frames = art.getPointerFrames();
	if (frames.empty())
		return 0;

	return MIN<uint>(kCursorSequence7LastFrame, frames.size() - 1);
}

static void drawStartupCursor(Graphics::Screen &screen, const StartupArt &art, const Common::Point &mousePos,
		uint frameIndex, bool visible) {
	if (!visible)
		return;

	blitAnimationFrame(screen, art.getPointerFrames(), frameIndex, mousePos.x, mousePos.y);
}

static bool loadQuickTipsScene(HarvesterEngine &engine, StartupRoomSceneResources &scene) {
	StartupRoomSetupState state;
	if (!engine.getStartupScript()->resolveRoomSetupState("QUICK_TIPS", state, *engine.getResources()))
		return false;

	return loadRoomSceneResources(state, *engine.getResources(), scene);
}

static void renderQuickTipsScreen(HarvesterEngine &engine, const StartupRoomSceneResources &scene,
		const Common::Point &mousePos, const Common::String &tipText, uint cursorFrameIndex, bool cursorVisible) {
	Graphics::Screen *screen = engine.getScreen();
	const StartupArt *art = engine.getStartupArt();
	const Graphics::Font *font = FontMan.getFontByUsage(Graphics::FontManager::kGUIFont);
	if (!screen || !art || !font)
		return;

	drawRoomScene(*screen, scene, scene.targetPaletteBrightness);
	blitBitmap(*screen, art->getTipsBitmap(), kQuickTipsOverlayX, kQuickTipsOverlayY);

	drawWrappedShadowedText(*screen, *font, tipText, kQuickTipTextX, kQuickTipTextY, kQuickTipTextWidth, kTextColorNormal);
	const Common::Rect exitRect = quickTipsExitRect();
	const Common::Rect nextRect = quickTipsNextRect();
	const Common::Rect toggleRect = quickTipsToggleRect();
	drawShadowedString(*screen, *font, "Exit", exitRect.left, exitRect.top, exitRect.width(),
		exitRect.contains(mousePos) ? kTextColorHover : kTextColorNormal);
	drawShadowedString(*screen, *font, "Next", nextRect.left, nextRect.top, nextRect.width(),
		nextRect.contains(mousePos) ? kTextColorHover : kTextColorNormal);
	drawShadowedString(*screen, *font,
		engine.getStartupScript()->isQuickTipsEnabled() ? "Show_Tips_ON" : "Show_Tips_OFF",
		toggleRect.left, toggleRect.top, toggleRect.width(),
		toggleRect.contains(mousePos) ? kTextColorHover : kTextColorNormal);

	drawStartupCursor(*screen, *art, mousePos, cursorFrameIndex, cursorVisible);
	screen->makeAllDirty();
	screen->update();
}

} // End of anonymous namespace

StartupFlow::StartupFlow(HarvesterEngine &engine) : _engine(engine), _mousePos(320, 200) {
}

bool StartupFlow::load() {
	return loadQuickTips() && loadMenuItems();
}

Common::Error StartupFlow::run() {
	resetCursorAnimationSequence();

	Common::Error error = runQuickTips();
	if (error.getCode() != Common::kNoError)
		return error;

	return runMainMenuStub();
}

bool StartupFlow::loadQuickTips() {
	_quickTips.clear();

	Common::Array<byte> data;
	if (!_engine.getResources()->loadFile(kQuickTipsPath, data)) {
		warning("Harvester: unable to load quick tips '%s'", kQuickTipsPath);
		return true;
	}

	Common::String currentLine;
	for (uint i = 0; i < data.size(); ++i) {
		const char ch = (char)data[i];
		if (ch == '\r')
			continue;
		if (ch == '\n') {
			const Common::String trimmed = trimAsciiLine(currentLine);
			if (!trimmed.empty())
				_quickTips.push_back(trimmed);
			currentLine.clear();
			continue;
		}

		currentLine += ch;
	}

	const Common::String trimmed = trimAsciiLine(currentLine);
	if (!trimmed.empty())
		_quickTips.push_back(trimmed);

	debugC(1, kDebugGeneral, "Harvester: loaded %u quick tips from '%s'", (uint)_quickTips.size(), kQuickTipsPath);
	return true;
}

bool StartupFlow::loadMenuItems() {
	_menuItems.clear();

	Common::Array<byte> data;
	if (!_engine.getResources()->loadFile(kMenuPath, data)) {
		warning("Harvester: unable to load startup menu '%s'", kMenuPath);
		return true;
	}

	Common::MemoryReadStream stream(data.data(), data.size());
	Common::INIFile menu;
	menu.setDefaultSectionName(kMenuSectionName);
	menu.requireKeyValueDelimiter();
	menu.suppressValuelessLineWarning();
	if (!menu.loadFromStream(stream)) {
		warning("Harvester: unable to parse startup menu '%s'", kMenuPath);
		return true;
	}

	for (uint i = 1; i <= 6; ++i) {
		Common::String key = Common::String::format("main_menu_%u", i);
		Common::String value;
		if (menu.getKey(key, kMenuSectionName, value) && !value.empty())
			_menuItems.push_back(value);
	}

	debugC(1, kDebugGeneral, "Harvester: loaded %u startup menu items from '%s'", (uint)_menuItems.size(), kMenuPath);
	return true;
}

Common::Error StartupFlow::runQuickTips() {
	if (!_engine.getStartupScript()->isQuickTipsEnabled() || _quickTips.empty())
		return Common::kNoError;

	Common::Error transitionError = beginRoomSetupTransition();
	if (transitionError.getCode() != Common::kNoError)
		return transitionError;

	StartupRoomSceneResources scene;
	if (!loadQuickTipsScene(_engine, scene))
		return Common::kReadingFailed;

	Graphics::Screen *screen = _engine.getScreen();
	if (screen) {
		drawRoomScene(*screen, scene, kPaletteBrightnessBlack);
		screen->makeAllDirty();
		screen->update();

		transitionError = fadeInRoomScene(scene.palette, scene.targetPaletteBrightness);
		if (transitionError.getCode() != Common::kNoError)
			return transitionError;
	}

	resetCursorAnimationSequence();

	uint tipIndex = _engine.getRandomNumber(_quickTips.size() - 1);
	bool needsRedraw = true;
	Graphics::FrameLimiter limiter(g_system, 60);

	while (!_engine.shouldQuit()) {
		if (needsRedraw) {
			renderQuickTipsScreen(_engine, scene, _mousePos, _quickTips[tipIndex], _cursorFrameIndex, _cursorVisible);
			needsRedraw = false;
		}

		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			Common::Error result = Common::kNoError;
			if (handleSystemEvent(event, result))
				return result;

			switch (event.type) {
			case Common::EVENT_MOUSEMOVE:
				needsRedraw = true;
				break;
			case Common::EVENT_LBUTTONDOWN:
				if (quickTipsExitRect().contains(_mousePos))
					return Common::kNoError;

				if (quickTipsNextRect().contains(_mousePos)) {
					tipIndex = (tipIndex + 1) % _quickTips.size();
					needsRedraw = true;
				} else if (quickTipsToggleRect().contains(_mousePos)) {
					_engine.getStartupScript()->setQuickTipsEnabled(!_engine.getStartupScript()->isQuickTipsEnabled());
					needsRedraw = true;
				}
				break;
			default:
				break;
			}
		}

		if (tickCursorAnimation())
			needsRedraw = true;

		limiter.delayBeforeSwap();
		limiter.startFrame();
	}

	return Common::kNoError;
}

Common::Error StartupFlow::runMainMenuStub() {
	Graphics::FrameLimiter limiter(g_system, 60);
	Common::String statusMessage = "Stub handoff after room_setup(\"QUICK_TIPS\") and before the main menu loop.";
	int selectedItem = _menuItems.empty() ? -1 : 0;
	bool needsRedraw = true;
	const StartupArt *art = _engine.getStartupArt();
	if (art) {
		const uint firstFrame = getStartupCursorSequenceFirstFrame(*art);
		const uint lastFrame = getStartupCursorSequenceLastFrame(*art);
		if (_cursorFrameIndex < firstFrame || _cursorFrameIndex > lastFrame || !_cursorVisible)
			resetCursorAnimationSequence();
	}

	while (!_engine.shouldQuit()) {
		if (needsRedraw) {
			renderMainMenuStub(selectedItem, statusMessage);
			needsRedraw = false;
		}

		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			Common::Error result = Common::kNoError;
			if (handleSystemEvent(event, result))
				return result;

			switch (event.type) {
			case Common::EVENT_MOUSEMOVE: {
				const int hoveredItem = getMenuItemAt(_mousePos);
				if (hoveredItem != -1 && hoveredItem != selectedItem) {
					selectedItem = hoveredItem;
					needsRedraw = true;
				}
				break;
			}
			case Common::EVENT_KEYDOWN:
				if (event.kbd.keycode == Common::KEYCODE_ESCAPE)
					return Common::kNoError;

				if (_menuItems.empty())
					break;

				if (event.kbd.keycode == Common::KEYCODE_UP) {
					selectedItem = (selectedItem + _menuItems.size() - 1) % _menuItems.size();
					needsRedraw = true;
				} else if (event.kbd.keycode == Common::KEYCODE_DOWN) {
					selectedItem = (selectedItem + 1) % _menuItems.size();
					needsRedraw = true;
				} else if (event.kbd.keycode == Common::KEYCODE_RETURN || event.kbd.keycode == Common::KEYCODE_KP_ENTER) {
					if (_menuItems[selectedItem].equalsIgnoreCase("QUIT GAME"))
						return Common::kNoError;

					if (_menuItems[selectedItem].equalsIgnoreCase("NEW GAME")) {
						Common::Error roomError = runRoomSetupStub("START");
						if (roomError.getCode() == Common::kReadingFailed) {
							statusMessage = "Unable to resolve START room setup from HARVEST.SCR.";
							needsRedraw = true;
						} else if (roomError.getCode() != Common::kNoError) {
							return roomError;
						} else {
							statusMessage = "Resolved START room handoff from HARVEST.SCR.";
							needsRedraw = true;
						}
					} else {
						statusMessage = Common::String::format("%s is stubbed; room setup and gameplay loop are next.",
							_menuItems[selectedItem].c_str());
						needsRedraw = true;
					}
				}
				break;
			case Common::EVENT_LBUTTONDOWN:
				if (_menuItems.empty())
					break;

				selectedItem = getMenuItemAt(_mousePos);
				if (selectedItem == -1)
					break;

				if (_menuItems[selectedItem].equalsIgnoreCase("QUIT GAME"))
					return Common::kNoError;

				if (_menuItems[selectedItem].equalsIgnoreCase("NEW GAME")) {
					Common::Error roomError = runRoomSetupStub("START");
					if (roomError.getCode() == Common::kReadingFailed) {
						statusMessage = "Unable to resolve START room setup from HARVEST.SCR.";
						needsRedraw = true;
					} else if (roomError.getCode() != Common::kNoError) {
						return roomError;
					} else {
						statusMessage = "Resolved START room handoff from HARVEST.SCR.";
						needsRedraw = true;
					}
				} else {
					statusMessage = Common::String::format("%s is stubbed; room setup and gameplay loop are next.",
						_menuItems[selectedItem].c_str());
					needsRedraw = true;
				}
				break;
			default:
				break;
			}
		}

		if (tickCursorAnimation())
			needsRedraw = true;

		limiter.delayBeforeSwap();
		limiter.startFrame();
	}

	return Common::kNoError;
}

Common::Error StartupFlow::runRoomSetupStub(const Common::String &entranceName) {
	StartupRoomSetupState state;
	if (!_engine.getStartupScript()->resolveRoomSetupState(entranceName, state, *_engine.getResources()))
		return Common::kReadingFailed;

	Common::Error transitionError = beginRoomSetupTransition();
	if (transitionError.getCode() != Common::kNoError)
		return transitionError;

	StartupRoomSceneResources scene;
	if (!loadRoomSceneResources(state, *_engine.getResources(), scene))
		return Common::kReadingFailed;

	Graphics::Screen *screen = _engine.getScreen();
	const StartupArt *art = _engine.getStartupArt();
	const Graphics::Font *titleFont = FontMan.getFontByUsage(Graphics::FontManager::kBigGUIFont);
	const Graphics::Font *bodyFont = FontMan.getFontByUsage(Graphics::FontManager::kGUIFont);
	if (!screen || !art || !titleFont || !bodyFont)
		return Common::kNoError;

	drawRoomScene(*screen, scene, kPaletteBrightnessBlack);
	screen->makeAllDirty();
	screen->update();

	transitionError = fadeInRoomScene(scene.palette, scene.targetPaletteBrightness);
	if (transitionError.getCode() != Common::kNoError)
		return transitionError;

	resetCursorAnimationSequence();

	Common::String statusMessage = Common::String::format(
		"Resolved %s -> %s using %s and %s.",
		entranceName.c_str(), state.roomName.c_str(), state.palettePath.c_str(), state.backgroundPath.c_str());
	if (!state.musicPath.empty())
		statusMessage += Common::String::format(" Music: %s.", state.musicPath.c_str());
	if (!scene.overlays.empty())
		statusMessage += Common::String::format(" Visible scene objects: %u.", (uint)scene.overlays.size());
	statusMessage += " Click an active hotspot to follow its startup command chain.";
	const Common::String baseStatusMessage = statusMessage;
	StartupResolvedText inspectText;
	Common::String inspectObjectLabel;
	bool showingInspectText = false;
	bool inspectCanDismiss = false;
	bool needsRedraw = true;
	Graphics::FrameLimiter limiter(g_system, 60);

	while (!_engine.shouldQuit()) {
		if (needsRedraw) {
			drawRoomScene(*screen, scene, scene.targetPaletteBrightness);

			const Common::Rect panel(72, 336, 568, 468);
			screen->fillRect(panel, kPanelFillColor);
			drawShadowedString(*screen, *titleFont, Common::String::format("Room Setup Stub: %s", state.roomName.c_str()),
				panel.left, 348, panel.width(), kTextColorNormal, Graphics::kTextAlignCenter);
			Common::String hoverMessage;
			Common::String footerMessage = "Press Enter or Escape to return to the menu stub.";
			if (showingInspectText) {
				hoverMessage = inspectText.value;
				footerMessage = inspectCanDismiss ? "Click, Enter, or Escape to dismiss this inspect text."
					: "Release the mouse button, then click again to dismiss this inspect text.";
				drawShadowedString(*screen, *bodyFont, Common::String::format("Inspect: %s", inspectObjectLabel.c_str()),
					panel.left, 372, panel.width(), kTextColorHover, Graphics::kTextAlignCenter);
			} else {
				const StartupObjectRecord *hoveredObject = findRoomObjectAtPoint(state, _mousePos);
				if (hoveredObject) {
					hoverMessage = _engine.getStartupScript()->resolveObjectLabel(*hoveredObject);
					if (!hoveredObject->interactionCommandTag.empty())
						hoverMessage += " Click to follow the scripted startup interaction.";
					else if (!_engine.getStartupScript()->resolveObjectInspectText(*hoveredObject, inspectText))
						inspectText = StartupResolvedText();
					else
						hoverMessage += " Click to inspect.";
				}
			}
			if (hoverMessage.empty())
				hoverMessage = statusMessage;

			drawWrappedShadowedText(*screen, *bodyFont, hoverMessage, panel.left + 24, 386, panel.width() - 48,
				kTextColorNormal);
			drawShadowedString(*screen, *bodyFont, footerMessage,
				panel.left, 430, panel.width(), kTextColorDim, Graphics::kTextAlignCenter);

			drawStartupCursor(*screen, *art, _mousePos, _cursorFrameIndex, _cursorVisible);
			screen->makeAllDirty();
			screen->update();
			needsRedraw = false;
		}

		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			Common::Error result = Common::kNoError;
			if (handleSystemEvent(event, result))
				return result;

			switch (event.type) {
			case Common::EVENT_MOUSEMOVE:
				needsRedraw = true;
				break;
			case Common::EVENT_LBUTTONUP:
				if (showingInspectText)
					inspectCanDismiss = true;
				break;
			case Common::EVENT_LBUTTONDOWN: {
				if (showingInspectText) {
					if (inspectCanDismiss) {
						showingInspectText = false;
						inspectCanDismiss = false;
						inspectText = StartupResolvedText();
						inspectObjectLabel.clear();
						statusMessage = baseStatusMessage;
						needsRedraw = true;
					}
					break;
				}

				const StartupObjectRecord *clickedObject = findRoomObjectAtPoint(state, _mousePos);
				if (!clickedObject) {
					statusMessage = "No active startup hotspot under the cursor.";
					needsRedraw = true;
					break;
				}

				const Common::String objectLabel = _engine.getStartupScript()->resolveObjectLabel(*clickedObject);
				StartupInteractionResult interaction;
				if (!_engine.getStartupScript()->resolveObjectInteraction(*clickedObject, interaction)) {
					if (_engine.getStartupScript()->resolveObjectInspectText(*clickedObject, inspectText)) {
						inspectObjectLabel = objectLabel;
						showingInspectText = true;
						inspectCanDismiss = false;
						statusMessage = Common::String::format("Showing inspect text for %s.", objectLabel.c_str());
					} else {
						statusMessage = Common::String::format("%s has no supported startup interaction yet.",
							objectLabel.c_str());
					}
					needsRedraw = true;
					break;
				}

				statusMessage = Common::String::format("Following %s.", objectLabel.c_str());
				if (!interaction.soundPath.empty()) {
					const bool soundStarted = _engine.playStartupSound(interaction.soundPath);
					statusMessage += Common::String::format(" Sound: %s (%s).", interaction.soundPath.c_str(),
						soundStarted ? "playing" : "load failed");
				}
				needsRedraw = true;

				if (!interaction.nextRoomName.empty()) {
					Common::Error roomError = runRoomSetupStub(interaction.nextRoomName);
					if (roomError.getCode() == Common::kReadingFailed) {
						statusMessage = Common::String::format("Unable to resolve closeup room '%s'.",
							interaction.nextRoomName.c_str());
						needsRedraw = true;
					} else if (roomError.getCode() != Common::kNoError) {
						return roomError;
					} else {
						statusMessage = Common::String::format("Returned from %s.", interaction.nextRoomName.c_str());
						needsRedraw = true;
					}
				}
				break;
			}
			case Common::EVENT_KEYDOWN:
				if (showingInspectText) {
					if (event.kbd.keycode == Common::KEYCODE_RETURN ||
						event.kbd.keycode == Common::KEYCODE_KP_ENTER ||
						event.kbd.keycode == Common::KEYCODE_ESCAPE ||
						event.kbd.keycode == Common::KEYCODE_SPACE) {
						showingInspectText = false;
						inspectCanDismiss = false;
						inspectText = StartupResolvedText();
						inspectObjectLabel.clear();
						statusMessage = baseStatusMessage;
						needsRedraw = true;
					}
					break;
				}

				if (event.kbd.keycode == Common::KEYCODE_RETURN ||
					event.kbd.keycode == Common::KEYCODE_KP_ENTER ||
					event.kbd.keycode == Common::KEYCODE_ESCAPE) {
					return Common::kNoError;
				}
				break;
			default:
				break;
			}
		}

		if (tickCursorAnimation())
			needsRedraw = true;

		limiter.delayBeforeSwap();
		limiter.startFrame();
	}

	return Common::kNoError;
}

Common::Error StartupFlow::beginRoomSetupTransition() {
	_cursorVisible = false;

	const StartupArt *art = _engine.getStartupArt();
	if (art)
		art->drawWaitFrame();

	Common::Error result = Common::kNoError;
	if (pumpTransitionEvents(result))
		return result;

	return Common::kNoError;
}

Common::Error StartupFlow::fadeInRoomScene(const byte *palette, float targetBrightness) {
	Graphics::Screen *screen = _engine.getScreen();
	if (!screen || !palette)
		return Common::kNoError;

	for (float brightness = kPaletteFadeStep; brightness < targetBrightness; brightness += kPaletteFadeStep) {
		setScaledPalette(*screen, palette, brightness);
		screen->makeAllDirty();
		screen->update();

		const uint32 nextTick = g_system->getMillis() + kPaletteFadeTickMs;
		while ((int32)(nextTick - g_system->getMillis()) > 0) {
			Common::Error result = Common::kNoError;
			if (pumpTransitionEvents(result))
				return result;
			g_system->delayMillis(1);
		}
	}

	setScaledPalette(*screen, palette, targetBrightness);
	screen->makeAllDirty();
	screen->update();
	return Common::kNoError;
}

bool StartupFlow::pumpTransitionEvents(Common::Error &result) {
	Common::Event event;
	while (g_system->getEventManager()->pollEvent(event)) {
		if (handleSystemEvent(event, result))
			return true;
	}

	return false;
}

void StartupFlow::resetCursorAnimationSequence() {
	const StartupArt *art = _engine.getStartupArt();
	if (!art) {
		_cursorFrameIndex = 0;
		_cursorNextFrameTick = 0;
		_cursorVisible = false;
		return;
	}

	_cursorFrameIndex = getStartupCursorSequenceFirstFrame(*art);
	_cursorNextFrameTick = g_system->getMillis() + kCursorAnimationTickMs;
	_cursorVisible = true;
}

bool StartupFlow::tickCursorAnimation() {
	if (!_cursorVisible)
		return false;

	const StartupArt *art = _engine.getStartupArt();
	if (!art || art->getPointerFrames().empty())
		return false;

	const uint firstFrame = getStartupCursorSequenceFirstFrame(*art);
	const uint lastFrame = getStartupCursorSequenceLastFrame(*art);
	if (_cursorFrameIndex < firstFrame || _cursorFrameIndex > lastFrame)
		_cursorFrameIndex = firstFrame;

	if (firstFrame == lastFrame)
		return false;

	const uint32 now = g_system->getMillis();
	if ((int32)(now - _cursorNextFrameTick) < 0)
		return false;

	do {
		_cursorFrameIndex = (_cursorFrameIndex >= lastFrame) ? firstFrame : _cursorFrameIndex + 1;
		_cursorNextFrameTick += kCursorAnimationTickMs;
	} while ((int32)(now - _cursorNextFrameTick) >= 0);

	return true;
}

void StartupFlow::renderMainMenuStub(int selectedItem, const Common::String &statusMessage) const {
	Graphics::Screen *screen = _engine.getScreen();
	const StartupArt *art = _engine.getStartupArt();
	const Graphics::Font *titleFont = FontMan.getFontByUsage(Graphics::FontManager::kBigGUIFont);
	const Graphics::Font *bodyFont = FontMan.getFontByUsage(Graphics::FontManager::kGUIFont);
	if (!screen || !art || !titleFont || !bodyFont)
		return;

	screen->setPalette(art->getWaitPalette());
	screen->fillRect(screen->getBounds(), 0);
	blitBitmap(*screen, art->getInventoryBitmap(), kInventoryX, kInventoryY);
	blitBitmap(*screen, art->getLogoBitmap(), kLogoX, kLogoY);

	const Common::Rect panel(96, 96, 544, 432);
	screen->fillRect(panel, kPanelFillColor);

	drawShadowedString(*screen, *titleFont, "Startup Main Menu Stub", panel.left, 110, panel.width(),
		kTextColorNormal, Graphics::kTextAlignCenter);

	for (uint i = 0; i < _menuItems.size(); ++i) {
		const int itemY = kMenuStartY + (int)i * kMenuLineSpacing;
		const byte color = ((int)i == selectedItem) ? kTextColorHover : kTextColorNormal;
		drawShadowedString(*screen, *bodyFont, _menuItems[i], panel.left, itemY, panel.width(), color,
			Graphics::kTextAlignCenter);
	}

	drawShadowedString(*screen, *bodyFont, "This is the next startup handoff after quick tips.", panel.left, 330,
		panel.width(), kTextColorDim, Graphics::kTextAlignCenter);
	drawWrappedShadowedText(*screen, *bodyFont, statusMessage, panel.left + 24, 360, panel.width() - 48,
		kTextColorNormal);
	drawShadowedString(*screen, *bodyFont, "Use mouse or arrow keys. Enter activates. Esc returns to launcher.",
		panel.left, 404, panel.width(), kTextColorDim, Graphics::kTextAlignCenter);

	drawStartupCursor(*screen, *art, _mousePos, _cursorFrameIndex, _cursorVisible);
	screen->makeAllDirty();
	screen->update();
}

bool StartupFlow::handleSystemEvent(const Common::Event &event, Common::Error &result) {
	switch (event.type) {
	case Common::EVENT_QUIT:
	case Common::EVENT_RETURN_TO_LAUNCHER:
		result = Common::kNoError;
		return true;
	case Common::EVENT_MOUSEMOVE:
	case Common::EVENT_LBUTTONDOWN:
	case Common::EVENT_LBUTTONUP:
	case Common::EVENT_RBUTTONDOWN:
	case Common::EVENT_RBUTTONUP:
		_mousePos = event.mouse;
		break;
	default:
		break;
	}

	return false;
}

int StartupFlow::getMenuItemAt(const Common::Point &mousePos) const {
	const Graphics::Font *font = FontMan.getFontByUsage(Graphics::FontManager::kGUIFont);
	if (!font)
		return -1;

	for (uint i = 0; i < _menuItems.size(); ++i) {
		const int width = font->getStringWidth(_menuItems[i]);
		const int x = (640 - width) / 2;
		const int y = kMenuStartY + (int)i * kMenuLineSpacing;
		const Common::Rect bounds(x - 12, y - 2, x + width + 12, y + font->getFontHeight() + 2);
		if (bounds.contains(mousePos))
			return (int)i;
	}

	return -1;
}

} // End of namespace Harvester
