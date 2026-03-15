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

#include "common/config-manager.h"
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
#include "harvester/runtime_entity.h"
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
static const int kCursorSequence7 = 7;
static const char *const kPlayerActorEntityName = "PLAYER";
static const char *const kPlayerActorResourcePath = "1:/GRAPHIC/MONSTERS/PC/PC0.ABM";
static const uint32 kPaletteFadeTickMs = 4;
static const float kPaletteFadeStep = 0.1f;
static const float kPaletteBrightnessBlack = 0.0f;

static const byte kTextColorNormal = 255;
static const byte kTextColorHover = 251;
static const byte kTextColorDim = 248;
static const byte kShadowColor = 0;
static const byte kPanelFillColor = 1;

struct StartupRoomSceneResources {
	StartupRoomSetupState state;
	byte palette[256 * 3] = { 0 };
	Common::Array<StartupObjectRecord> sceneObjects;
	Common::Array<StartupAnimRecord> sceneAnimations;
	float targetPaletteBrightness = 1.0f;
};

static int resolvePlayerFacingFrame(int facing) {
	switch (facing) {
	case 0:
		return 0x3b;
	case 1:
		return 0x0e;
	case 2:
		return 0x2c;
	case 3:
		return 0x28;
	default:
		return 0x0e;
	}
}

static void getPaletteByteRange(const byte *palette, byte &minValue, byte &maxValue) {
	minValue = 255;
	maxValue = 0;

	for (uint i = 0; i < 256 * 3; ++i) {
		minValue = MIN(minValue, palette[i]);
		maxValue = MAX(maxValue, palette[i]);
	}
}

static uint32 hashPalette(const byte *palette) {
	uint32 hash = 2166136261U;
	for (uint i = 0; i < 256 * 3; ++i) {
		hash ^= palette[i];
		hash *= 16777619U;
	}

	return hash;
}

static void logScenePaletteSummary(const char *label, const StartupRoomSceneResources &scene, float brightness) {
	const byte *palette = scene.palette;
	byte minValue = 0;
	byte maxValue = 0;
	getPaletteByteRange(palette, minValue, maxValue);

	debugC(1, kDebugGeneral,
		"Harvester: %s room='%s' palette='%s' bytes=%u range=[%u,%u] brightness=%.2f hash=%08x idx0=(%u,%u,%u) idx1=(%u,%u,%u) idx127=(%u,%u,%u) idx255=(%u,%u,%u)",
		label, scene.state.roomName.c_str(), scene.state.palettePath.c_str(), 256U * 3U, minValue, maxValue,
		(double)brightness, hashPalette(palette),
		palette[0], palette[1], palette[2],
		palette[3], palette[4], palette[5],
		palette[127 * 3], palette[127 * 3 + 1], palette[127 * 3 + 2],
		palette[255 * 3], palette[255 * 3 + 1], palette[255 * 3 + 2]);
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

	memcpy(dest, data.data(), 256 * 3);

	byte minValue = 0;
	byte maxValue = 0;
	getPaletteByteRange(dest, minValue, maxValue);
	debugC(1, kDebugGeneral,
		"Harvester: decoded room palette '%s' bytes=%u range=[%u,%u] idx0=(%u,%u,%u) idx255=(%u,%u,%u)",
		path.c_str(), (uint)data.size(), minValue, maxValue,
		dest[0], dest[1], dest[2],
		dest[255 * 3], dest[255 * 3 + 1], dest[255 * 3 + 2]);

	return true;
}

static Common::Rect getHotspotBounds(const StartupObjectRecord &object) {
	if (object.boundsX2 > object.currentX && object.boundsY2 > object.currentY)
		return Common::Rect(object.currentX, object.currentY, object.boundsX2 + 1, object.boundsY2 + 1);

	return Common::Rect();
}

static Common::String resolveSceneObjectSpritePath(const StartupObjectRecord &object) {
	const bool atInitialPlacement = object.currentX == object.initialX &&
		object.currentY == object.initialY &&
		object.currentOwnerOrRoom.equalsIgnoreCase(object.initialOwnerOrRoom);
	if (!object.altSpritePath.empty() &&
		(!atInitialPlacement || object.currentOwnerOrRoom.equalsIgnoreCase("INVENTORY")))
		return object.altSpritePath;

	return object.spritePath;
}

static bool isBackgroundSceneObject(const StartupObjectRecord &object, const RuntimeEntity &entity) {
	return object.initialX == 0 && object.initialY == 0 &&
		entity.getBoundsWidth() == 640 && entity.getBoundsHeight() == 480;
}

static int resolveSceneObjectClass(const StartupObjectRecord &object, const RuntimeEntity *entity) {
	if (entity)
		return isBackgroundSceneObject(object, *entity) ? kRuntimeEntityClassBackground : kRuntimeEntityClassObject;

	return object.actionTag.empty() ? kRuntimeEntityClassDisabledHotspot : kRuntimeEntityClassRectHotspot;
}

static void queueVisibleSceneObject(const StartupObjectRecord &object, Common::Array<StartupObjectRecord> &sceneObjects) {
	if (!object.visible)
		return;

	for (const StartupObjectRecord &sceneObject : sceneObjects) {
		if (sceneObject.currentOwnerOrRoom.equalsIgnoreCase(object.currentOwnerOrRoom) &&
			sceneObject.objectName.equalsIgnoreCase(object.objectName)) {
			return;
		}
	}

	sceneObjects.push_back(object);
}

static bool loadRoomSceneResources(const StartupRoomSetupState &state, ResourceManager &resources, StartupRoomSceneResources &scene) {
	scene = StartupRoomSceneResources();
	scene.state = state;
	scene.targetPaletteBrightness = state.paletteBrightness;
	if (!loadPaletteResource(resources, state.palettePath, scene.palette)) {
		return false;
	}

	Common::Array<StartupObjectRecord> sceneObjects;
	for (const StartupObjectRecord &object : state.roomObjects)
		queueVisibleSceneObject(object, sceneObjects);
	for (const StartupObjectRecord &object : state.activeObjects)
		queueVisibleSceneObject(object, sceneObjects);

	scene.sceneObjects = sceneObjects;
	for (const StartupAnimRecord &anim : state.roomAnimations) {
		if (anim.active || anim.visible)
			scene.sceneAnimations.push_back(anim);
	}

	debugC(1, kDebugGeneral,
		"Harvester: loadRoomSceneResources room='%s' palette='%s' background='%s' brightness=%.2f objects=%u activeObjects=%u sceneObjects=%u anims=%u visibleAnims=%u",
		state.roomName.c_str(), state.palettePath.c_str(), state.backgroundPath.c_str(), (double)state.paletteBrightness,
		(uint)state.roomObjects.size(), (uint)state.activeObjects.size(), (uint)scene.sceneObjects.size(),
		(uint)state.roomAnimations.size(), (uint)scene.sceneAnimations.size());

	return true;
}

static bool shouldRunStartupRoomProbe() {
	return ConfMan.hasKey("harvester_debug_probe_startup_room") &&
		ConfMan.getBool("harvester_debug_probe_startup_room");
}

static void drawRoomScene(HarvesterEngine &engine, Graphics::Screen &screen, const StartupRoomSceneResources &scene,
		float brightness) {
	setScaledPalette(screen, scene.palette, brightness);
	screen.fillRect(screen.getBounds(), 0);
	if (engine.getRuntimeEntities())
		engine.getRuntimeEntities()->drawSceneEntities(screen);
}

static const StartupObjectRecord *findSceneObjectByName(const Common::Array<StartupObjectRecord> &objects,
		const Common::String &objectName) {
	for (const StartupObjectRecord &object : objects) {
		if (object.objectName.equalsIgnoreCase(objectName))
			return &object;
	}

	return nullptr;
}

static const StartupObjectRecord *findRoomObjectAtPoint(HarvesterEngine &engine,
		const Common::Array<StartupObjectRecord> &sceneObjects, const Common::Point &point) {
	RuntimeEntityManager *runtimeEntities = engine.getRuntimeEntities();
	if (!runtimeEntities)
		return nullptr;

	const RuntimeEntity *entity = runtimeEntities->findTopSceneEntityAt(point);
	if (!entity)
		return nullptr;

	return findSceneObjectByName(sceneObjects, entity->getName());
}

static bool findRoomObjectProbePoint(HarvesterEngine &engine, const Common::Array<StartupObjectRecord> &sceneObjects,
		const StartupObjectRecord &object, Common::Point &probePoint) {
	const Common::Rect bounds = getHotspotBounds(object);
	if (bounds.isEmpty())
		return false;

	const Common::Point center((bounds.left + bounds.right - 1) / 2, (bounds.top + bounds.bottom - 1) / 2);
	const Common::Point corners[] = {
		center,
		Common::Point(bounds.left, bounds.top),
		Common::Point(bounds.right - 1, bounds.top),
		Common::Point(bounds.left, bounds.bottom - 1),
		Common::Point(bounds.right - 1, bounds.bottom - 1)
	};
	for (const Common::Point &candidate : corners) {
		const StartupObjectRecord *hit = findRoomObjectAtPoint(engine, sceneObjects, candidate);
		if (hit && hit->objectName.equalsIgnoreCase(object.objectName)) {
			probePoint = candidate;
			return true;
		}
	}

	const int xStep = MAX<int>(1, bounds.width() / 6);
	const int yStep = MAX<int>(1, bounds.height() / 6);
	for (int y = bounds.top; y < bounds.bottom; y += yStep) {
		for (int x = bounds.left; x < bounds.right; x += xStep) {
			const Common::Point candidate(x, y);
			const StartupObjectRecord *hit = findRoomObjectAtPoint(engine, sceneObjects, candidate);
			if (hit && hit->objectName.equalsIgnoreCase(object.objectName)) {
				probePoint = candidate;
				return true;
			}
		}
	}

	return false;
}

static void logStartupRoomProbe(HarvesterEngine &engine, const StartupRoomSceneResources &scene,
		const Common::String &entranceName, Common::Point &mousePos) {
	RuntimeEntityManager *runtimeEntities = engine.getRuntimeEntities();
	StartupScript *startupScript = engine.getStartupScript();
	if (!runtimeEntities || !startupScript)
		return;

	if (const RuntimeEntity *cursor = runtimeEntities->getCursorEntity()) {
		uint32 framePixels = 0;
		uint32 transparentPixels = 0;
		uint32 preservedPixels = 0;
		if (cursor->measureCurrentFrameTransparency(framePixels, transparentPixels, preservedPixels)) {
			debugC(1, kDebugGeneral,
				"Harvester: startup probe cursor sequence=%d frame=%d pixels=%u transparent_pixels=%u preserved_pixels=%u",
				cursor->getAnimationSequence(), cursor->getCurrentFrame(), framePixels, transparentPixels, preservedPixels);
		}
	}

	if (const RuntimeEntity *player = runtimeEntities->findSceneEntityByName(kPlayerActorEntityName)) {
		uint32 framePixels = 0;
		uint32 transparentPixels = 0;
		uint32 preservedPixels = 0;
		(void)player->measureCurrentFrameTransparency(framePixels, transparentPixels, preservedPixels);
		const Common::Rect playerRect = player->getScreenRect();
		const Common::Rect screenRect(0, 0, 640, 480);
		debugC(1, kDebugGeneral,
			"Harvester: startup probe player room='%s' entrance='%s' frame=%d rect=(%d,%d)-(%d,%d) visible=%d intersects_screen=%d opaque_pixels=%u",
			scene.state.roomName.c_str(), entranceName.c_str(), player->getCurrentFrame(),
			playerRect.left, playerRect.top, playerRect.right, playerRect.bottom,
			player->isVisible(), playerRect.intersects(screenRect), framePixels - transparentPixels);
	}

	for (const StartupObjectRecord &object : scene.sceneObjects) {
		if (getHotspotBounds(object).isEmpty())
			continue;

		Common::Point probePoint;
		if (!findRoomObjectProbePoint(engine, scene.sceneObjects, object, probePoint))
			continue;

		const StartupObjectRecord *hoveredObject = findRoomObjectAtPoint(engine, scene.sceneObjects, probePoint);
		if (!hoveredObject)
			continue;

		mousePos = probePoint;
		const Common::String objectLabel = startupScript->resolveObjectLabel(*hoveredObject);
		StartupInteractionResult interaction;
		StartupResolvedText inspectText;
		const bool hasInteraction = startupScript->resolveObjectInteraction(*hoveredObject, interaction);
		const bool hasInspectText = startupScript->resolveObjectInspectText(*hoveredObject, inspectText);
		debugC(1, kDebugGeneral,
			"Harvester: startup probe hotspot room='%s' object='%s' point=(%d,%d) label='%s' action_tag='%s' interaction=%d next_room='%s' inspect=%d",
			scene.state.roomName.c_str(), hoveredObject->objectName.c_str(), probePoint.x, probePoint.y,
			objectLabel.c_str(), hoveredObject->actionTag.c_str(), hasInteraction,
			interaction.nextRoomName.c_str(), hasInspectText);
		return;
	}

	debugC(1, kDebugGeneral, "Harvester: startup probe found no interactive hotspot in room '%s'",
		scene.state.roomName.c_str());
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

static bool loadQuickTipsScene(HarvesterEngine &engine, StartupRoomSceneResources &scene) {
	StartupRoomSetupState state;
	// The original startup path enters the START room first and overlays quick tips on top of it.
	if (!engine.getStartupScript()->resolveRoomSetupState("START", state, *engine.getResources()))
		return false;

	return loadRoomSceneResources(state, *engine.getResources(), scene);
}

static void renderQuickTipsScreen(HarvesterEngine &engine, const StartupRoomSceneResources &scene,
		const Common::Point &mousePos, const Common::String &tipText) {
	Graphics::Screen *screen = engine.getScreen();
	const StartupArt *art = engine.getStartupArt();
	const Graphics::Font *font = FontMan.getFontByUsage(Graphics::FontManager::kGUIFont);
	if (!screen || !art || !font)
		return;

	drawRoomScene(engine, *screen, scene, scene.targetPaletteBrightness);
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

	if (engine.getRuntimeEntities())
		engine.getRuntimeEntities()->drawCursor(*screen);
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
	if (!ensureCursorEntity())
		return Common::kReadingFailed;

	Common::Error error = runQuickTips();
	if (error.getCode() != Common::kNoError)
		return error;

	return runRoomLoop("START");
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
		if (!populateRoomSceneEntities(scene.state, scene.sceneObjects, scene.sceneAnimations))
			return Common::kReadingFailed;

		logScenePaletteSummary("quick tips scene palette", scene, kPaletteBrightnessBlack);
		drawRoomScene(_engine, *screen, scene, kPaletteBrightnessBlack);
		screen->makeAllDirty();
		screen->update();

		logScenePaletteSummary("quick tips fade target", scene, scene.targetPaletteBrightness);
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
			renderQuickTipsScreen(_engine, scene, _mousePos, _quickTips[tipIndex]);
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

		if (tickRuntimeEntities())
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
	if (_engine.getRuntimeEntities())
		_engine.getRuntimeEntities()->clearSceneEntities();
	resetCursorAnimationSequence();

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
						Common::Error roomError = runRoomLoop("START");
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
					Common::Error roomError = runRoomLoop("START");
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

		if (tickRuntimeEntities())
			needsRedraw = true;

		limiter.delayBeforeSwap();
		limiter.startFrame();
	}

	return Common::kNoError;
}

Common::Error StartupFlow::runRoomLoop(const Common::String &entranceName) {
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

	if (!populateRoomSceneEntities(scene.state, scene.sceneObjects, scene.sceneAnimations))
		return Common::kReadingFailed;

	logScenePaletteSummary("room setup stub palette", scene, kPaletteBrightnessBlack);
	drawRoomScene(_engine, *screen, scene, kPaletteBrightnessBlack);
	screen->makeAllDirty();
	screen->update();

	logScenePaletteSummary("room setup fade target", scene, scene.targetPaletteBrightness);
	transitionError = fadeInRoomScene(scene.palette, scene.targetPaletteBrightness);
	if (transitionError.getCode() != Common::kNoError)
		return transitionError;

	resetCursorAnimationSequence();

	Common::String statusMessage = Common::String::format(
		"Entered %s via %s. Move the cursor over active hotspots and click to inspect or follow startup room transitions.",
		state.roomName.c_str(), entranceName.c_str());
	if (!state.musicPath.empty())
		statusMessage += Common::String::format(" Music: %s.", state.musicPath.c_str());
	if (!scene.sceneObjects.empty())
		statusMessage += Common::String::format(" Visible scene objects: %u.", (uint)scene.sceneObjects.size());
	if (!scene.sceneAnimations.empty())
		statusMessage += Common::String::format(" Room anims: %u.", (uint)scene.sceneAnimations.size());
	const Common::String baseStatusMessage = statusMessage;
	StartupResolvedText inspectText;
	Common::String inspectObjectLabel;
	bool showingInspectText = false;
	bool inspectCanDismiss = false;
	bool needsRedraw = true;
	Graphics::FrameLimiter limiter(g_system, 60);

	if (shouldRunStartupRoomProbe())
		logStartupRoomProbe(_engine, scene, entranceName, _mousePos);

	while (!_engine.shouldQuit()) {
		if (needsRedraw) {
			drawRoomScene(_engine, *screen, scene, scene.targetPaletteBrightness);
			const StartupObjectRecord *hoveredObject = showingInspectText ? nullptr :
				findRoomObjectAtPoint(_engine, scene.sceneObjects, _mousePos);
			Common::String hoverMessage;
			if (hoveredObject) {
				hoverMessage = _engine.getStartupScript()->resolveObjectLabel(*hoveredObject);
				if (!hoveredObject->actionTag.empty())
					hoverMessage += " Click to follow the scripted startup interaction.";
				else if (!_engine.getStartupScript()->resolveObjectInspectText(*hoveredObject, inspectText))
					inspectText = StartupResolvedText();
				else
					hoverMessage += " Click to inspect.";
			}
			if (hoverMessage.empty())
				hoverMessage = statusMessage;

			const Common::Rect headerPanel(16, 16, 624, 84);
			screen->fillRect(headerPanel, kPanelFillColor);
			drawShadowedString(*screen, *titleFont, state.roomName,
				headerPanel.left + 16, headerPanel.top + 8, headerPanel.width() - 32,
				kTextColorNormal, Graphics::kTextAlignCenter);
			drawWrappedShadowedText(*screen, *bodyFont, hoverMessage,
				headerPanel.left + 16, headerPanel.top + 34, headerPanel.width() - 32, kTextColorNormal);

			if (showingInspectText) {
				const Common::Rect inspectPanel(56, 338, 584, 468);
				const Common::String footerMessage = inspectCanDismiss
					? "Click, Enter, Space, or Escape to dismiss."
					: "Release the mouse button, then click again to dismiss.";
				screen->fillRect(inspectPanel, kPanelFillColor);
				drawShadowedString(*screen, *titleFont,
					Common::String::format("Inspect: %s", inspectObjectLabel.c_str()),
					inspectPanel.left, inspectPanel.top + 10, inspectPanel.width(),
					kTextColorHover, Graphics::kTextAlignCenter);
				drawWrappedShadowedText(*screen, *bodyFont, inspectText.value,
					inspectPanel.left + 20, inspectPanel.top + 40, inspectPanel.width() - 40, kTextColorNormal);
				drawShadowedString(*screen, *bodyFont, footerMessage,
					inspectPanel.left, inspectPanel.bottom - 20, inspectPanel.width(),
					kTextColorDim, Graphics::kTextAlignCenter);
			} else {
				drawShadowedString(*screen, *bodyFont, "Esc exits this prototype room loop.",
					0, 458, 640, kTextColorDim, Graphics::kTextAlignCenter);
			}

			if (_engine.getRuntimeEntities())
				_engine.getRuntimeEntities()->drawCursor(*screen);
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

					const StartupObjectRecord *clickedObject = findRoomObjectAtPoint(_engine, scene.sceneObjects, _mousePos);
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
					} else if (objectLabel.empty()) {
						statusMessage = "No active startup hotspot under the cursor.";
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
						Common::Error roomError = runRoomLoop(interaction.nextRoomName);
						if (roomError.getCode() == Common::kReadingFailed) {
							(void)populateRoomSceneEntities(scene.state, scene.sceneObjects, scene.sceneAnimations);
							resetCursorAnimationSequence();
							statusMessage = Common::String::format("Unable to resolve closeup room '%s'.",
								interaction.nextRoomName.c_str());
							needsRedraw = true;
						} else if (roomError.getCode() != Common::kNoError) {
							return roomError;
						} else {
							(void)populateRoomSceneEntities(scene.state, scene.sceneObjects, scene.sceneAnimations);
							resetCursorAnimationSequence();
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

		if (tickRuntimeEntities())
			needsRedraw = true;

		limiter.delayBeforeSwap();
		limiter.startFrame();
	}

	return Common::kNoError;
}

bool StartupFlow::ensureCursorEntity() {
	RuntimeEntityManager *runtimeEntities = _engine.getRuntimeEntities();
	if (!runtimeEntities)
		return false;
	if (runtimeEntities->getCursorEntity())
		return true;

	return runtimeEntities->spawnCursorEntity(_mousePos) != nullptr;
}

bool StartupFlow::populateRoomSceneEntities(const StartupRoomSetupState &state,
		const Common::Array<StartupObjectRecord> &drawableObjects,
		const Common::Array<StartupAnimRecord> &drawableAnimations) {
	RuntimeEntityManager *runtimeEntities = _engine.getRuntimeEntities();
	if (!runtimeEntities)
		return false;

	runtimeEntities->clearSceneEntities();
	for (const StartupObjectRecord &object : drawableObjects) {
		RuntimeEntity *entity = nullptr;
		const Common::String spritePath = resolveSceneObjectSpritePath(object);
		if (!spritePath.empty() && spritePath.hasSuffixIgnoreCase(".BM")) {
			entity = runtimeEntities->spawnSceneBitmapEntity(object.objectName, spritePath,
				Common::Point(object.currentX, object.currentY), (float)object.currentZ);
		} else {
			const Common::Rect hotspotBounds = getHotspotBounds(object);
			if (!hotspotBounds.isEmpty())
				entity = runtimeEntities->spawnSceneHotspotEntity(object.objectName, hotspotBounds, (float)object.currentZ);
		}

		if (!entity) {
			debug(1, "Harvester: unable to spawn room object entity '%s' from '%s'",
				object.objectName.c_str(), spritePath.c_str());
			continue;
		}

		entity->setClassId(resolveSceneObjectClass(object, entity->hasFrames() ? entity : nullptr));
		entity->setAnchorMode(kRuntimeEntityAnchorTopLeft);
		entity->setZExtent((float)object.zExtent);
	}
	for (const StartupAnimRecord &anim : drawableAnimations) {
		if (!runtimeEntities->spawnSceneAnimationEntity(anim.animName, anim.resourcePath,
				Common::Point(anim.x, anim.y), (float)anim.z, anim.frameDelay, anim.active, anim.visible,
				anim.looping, anim.backward, anim.pingPong)) {
			debug(1, "Harvester: unable to spawn room anim entity '%s' from '%s'",
				anim.animName.c_str(), anim.resourcePath.c_str());
		}
	}
	if (state.hasEntrance) {
		const int playerFrame = resolvePlayerFacingFrame(state.playerFacing);
		RuntimeEntity *player = runtimeEntities->spawnSceneActorEntity(kPlayerActorEntityName,
			kPlayerActorResourcePath, Common::Point(state.playerSpawnX, state.playerSpawnY),
			(float)state.playerSpawnZ, playerFrame);
		if (!player) {
			debug(1, "Harvester: unable to spawn startup player actor from '%s'", kPlayerActorResourcePath);
		} else {
			debugC(1, kDebugGeneral,
				"Harvester: spawned startup player actor '%s' entrance='%s' pos=(%d,%d,%d) facing=%d frame=%d",
				kPlayerActorResourcePath, state.entranceName.c_str(),
				state.playerSpawnX, state.playerSpawnY, state.playerSpawnZ,
				state.playerFacing, playerFrame);
		}
	}

	return true;
}

Common::Error StartupFlow::beginRoomSetupTransition() {
	if (_engine.getRuntimeEntities())
		_engine.getRuntimeEntities()->hideCursor();

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
	if (!ensureCursorEntity())
		return;

	RuntimeEntity *cursor = _engine.getRuntimeEntities()->getCursorEntity();
	if (!cursor)
		return;

	cursor->setVisible(true);
	cursor->setAnimationSequence(kCursorSequence7);
	(void)_engine.getRuntimeEntities()->syncCursorEntityPosition(_mousePos);
}

bool StartupFlow::tickRuntimeEntities() {
	RuntimeEntityManager *runtimeEntities = _engine.getRuntimeEntities();
	if (!runtimeEntities)
		return false;
	return runtimeEntities->tickSceneEntities() || runtimeEntities->syncCursorEntityPosition(_mousePos);
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

	if (_engine.getRuntimeEntities())
		_engine.getRuntimeEntities()->drawCursor(*screen);
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
