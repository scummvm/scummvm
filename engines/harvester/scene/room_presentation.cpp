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

#include "harvester/scene/room_presentation.h"

#include "common/algorithm.h"
#include "common/debug.h"
#include "common/events.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "graphics/font.h"
#include "graphics/fontman.h"
#include "graphics/framelimiter.h"
#include "harvester/art.h"
#include "harvester/cft_font.h"
#include "harvester/detection.h"
#include "harvester/entity.h"
#include "harvester/flow/flow.h"
#include "harvester/harvester.h"
#include "harvester/media/media_manager.h"
#include "harvester/palette_utils.h"
#include "harvester/player/player.h"
#include "harvester/resources.h"
#include "harvester/scene/room_support.h"

namespace Harvester {

static const float kNormalPaletteBrightness = 1.0f;
static const float kDimPaletteBrightness = 0.6f;
static const int kIdentTextboxX = 177;
static const int kIdentTextboxY = 85;
static const int kIdentTextboxTextInsetX = 10;
static const int kIdentTextboxTextInsetY = 5;
static const byte kIdentTextColor = 0xd3;
static const byte kShadowColor = 0;
static const int kNativeIdentTextLineSpacing = 3;
static const char *const kPlayerActorEntityName = "PLAYER";
static const int kRoomDebugLabelPaddingX = 3;
static const int kRoomDebugLabelPaddingY = 2;
static const int kRoomDebugRoomNameX = 4;
static const int kRoomDebugRoomNameY = 4;
static const int kTimerDebugOverlayXDivisor = 8;
static const int kTimerDebugOverlayLineSpacing = 1;

static bool loadPaletteResource(ResourceManager &resources, const Common::String &path, byte *palette) {
	Common::Array<byte> data;
	if (!resources.loadFile(path, data) || data.size() < 256 * 3)
		return false;

	memcpy(palette, data.data(), 256 * 3);
	return true;
}

static void setScaledRoomPalette(Graphics::Screen &screen, const byte *palette, float brightness) {
	byte scaledPalette[256 * 3];
	const float gammaBrightness = g_engine ? g_engine->getGammaBrightnessScale() : 1.0f;
	buildHarvesterDisplayPalette(palette, brightness * gammaBrightness, scaledPalette);
	screen.setPalette(scaledPalette);
}

static byte findNearestPaletteColor(const byte *palette, byte red, byte green, byte blue) {
	if (!palette)
		return 0;

	byte bestIndex = 0;
	uint32 bestDistance = 0xffffffffu;
	for (int i = 0; i < 256; ++i) {
		const int paletteOffset = i * 3;
		const int dr = (int)palette[paletteOffset] - red;
		const int dg = (int)palette[paletteOffset + 1] - green;
		const int db = (int)palette[paletteOffset + 2] - blue;
		const uint32 distance = (uint32)(dr * dr + dg * dg + db * db);
		if (distance < bestDistance) {
			bestDistance = distance;
			bestIndex = (byte)i;
		}
	}

	return bestIndex;
}

static Common::String resolveRoomDebugObjectLabel(HarvesterEngine &engine, const ObjectRecord &object) {
	if (Script *script = engine.getScript()) {
		const Common::String resolvedLabel = script->resolveObjectLabel(object);
		if (!resolvedLabel.empty())
			return resolvedLabel;
	}

	return object.objectName;
}

static Common::String resolveRoomDebugNpcLabel(const NpcRecord &npc) {
	Common::String label = !npc.entityInitArg.empty() ? npc.entityInitArg : npc.npcName;
	for (uint i = 0; i < label.size(); ++i) {
		if (label[i] == '_')
			label.setChar(' ', i);
	}

	return label;
}

static Common::String resolveCombatDebugLabel(const Common::String &name, int currentHitPoints, int maxHitPoints) {
	Common::String label = name;
	for (uint i = 0; i < label.size(); ++i) {
		if (label[i] == '_')
			label.setChar(' ', i);
	}

	return Common::String::format("%s (%d/%d)", label.c_str(), currentHitPoints, maxHitPoints);
}

static void drawRoomDebugLabel(Graphics::Screen &screen, const Graphics::Font &font,
		const Common::String &text, int x, int y, byte textColor, byte backgroundColor) {
	if (text.empty())
		return;

	const int labelWidth = font.getStringWidth(text) + kRoomDebugLabelPaddingX * 2;
	const int labelHeight = font.getFontHeight() + kRoomDebugLabelPaddingY * 2;
	const int drawX = CLIP<int>(x, 0, MAX(0, screen.w - labelWidth));
	const int drawY = CLIP<int>(y, 0, MAX(0, screen.h - labelHeight));
	const Common::Rect background(drawX, drawY,
		MIN<int>(screen.w, drawX + labelWidth), MIN<int>(screen.h, drawY + labelHeight));
	screen.fillRect(background, backgroundColor);
	font.drawString(&screen, text,
		background.left + kRoomDebugLabelPaddingX,
		background.top + kRoomDebugLabelPaddingY,
		MAX(0, background.width() - kRoomDebugLabelPaddingX * 2),
		textColor);
}

static void drawRoomDebugOverlay(HarvesterEngine &engine, Graphics::Screen &screen,
		const SceneState &scene) {
	if (!engine.isRoomDebugEnabled())
		return;

	const Graphics::Font *font = FontMan.getFontByUsage(Graphics::FontManager::kGUIFont);
	if (!font)
		return;

	byte displayPalette[256 * 3];
	screen.getPalette(displayPalette);
	const byte black = findNearestPaletteColor(displayPalette, 0x00, 0x00, 0x00);
	const byte white = findNearestPaletteColor(displayPalette, 0xff, 0xff, 0xff);
	const byte darkGray = findNearestPaletteColor(displayPalette, 0x40, 0x40, 0x40);
	const byte red = findNearestPaletteColor(displayPalette, 0xff, 0x00, 0x00);
	Common::String roomName = scene.state.roomName;
	roomName.toUppercase();

	drawRoomDebugLabel(screen, *font, roomName,
		kRoomDebugRoomNameX, kRoomDebugRoomNameY, black, white);

	for (const RegionRecord &region : scene.sceneRegions)
		drawRoomDebugLabel(screen, *font, region.regionName, region.left, region.top, white, darkGray);

	EntityManager *entityManager = engine.getRuntimeEntities();
	for (const ObjectRecord &object : scene.sceneObjects) {
		const Entity *entity = entityManager
			? entityManager->findSceneEntityByName(object.objectName)
			: nullptr;
		if (entity && entity->getClassId() == kEntityClassBackground)
			continue;

		drawRoomDebugLabel(screen, *font, resolveRoomDebugObjectLabel(engine, object),
			object.currentX, object.currentY, white, black);
	}

	for (const NpcRecord &npc : scene.state.roomNpcs) {
		const Entity *entity = entityManager
			? entityManager->findSceneEntityByName(npc.npcName)
			: nullptr;
		if (!entity || entity->getClassId() != kEntityClassNpc || !entity->isVisible())
			continue;

		const Common::Rect npcRect = entity->getScreenRect();
		drawRoomDebugLabel(screen, *font, resolveRoomDebugNpcLabel(npc),
			npcRect.left, npcRect.top, white, red);
	}
}

static void drawCombatDebugOverlay(HarvesterEngine &engine, Graphics::Screen &screen,
		const SceneState &scene) {
	if (!engine.isCombatDebugEnabled())
		return;

	const Graphics::Font *font = FontMan.getFontByUsage(Graphics::FontManager::kGUIFont);
	EntityManager *entityManager = engine.getRuntimeEntities();
	Script *script = engine.getScript();
	if (!font || !entityManager)
		return;

	byte displayPalette[256 * 3];
	screen.getPalette(displayPalette);
	const byte black = findNearestPaletteColor(displayPalette, 0x00, 0x00, 0x00);
	const byte white = findNearestPaletteColor(displayPalette, 0xff, 0xff, 0xff);
	const byte red = findNearestPaletteColor(displayPalette, 0xff, 0x00, 0x00);

	const Entity *playerEntity = entityManager->findSceneEntityByName(kPlayerActorEntityName);
	if (script && playerEntity &&
			playerEntity->getClassId() == kEntityClassPlayer &&
			playerEntity->isVisible()) {
		const Common::Rect playerRect = playerEntity->getScreenRect();
		drawRoomDebugLabel(screen, *font,
			resolveCombatDebugLabel(playerEntity->getName(),
				script->getPlayerCurrentHitPoints(), Script::kDefaultPlayerHitPoints),
			playerRect.left, playerRect.top, white, red);
	}

	for (const MonsterRecord &monster : scene.state.roomMonsters) {
		const Entity *entity = entityManager->findSceneEntityByName(monster.monsterName);
		if (!entity || entity->getClassId() != kEntityClassMonster || !entity->isVisible())
			continue;

		const Common::Rect monsterRect = entity->getScreenRect();
		drawRoomDebugLabel(screen, *font,
			resolveCombatDebugLabel(monster.monsterName,
				monster.currentHitPoints, monster.initialHitPoints),
			monsterRect.left, monsterRect.top, white, black);
	}
}

static Common::Rect getPathfindingMovementBounds(const Graphics::Screen &screen,
		const RoomSetupState &state) {
	if (!Player::supportsMovementBand(state))
		return Common::Rect();

	return Common::Rect(0, state.roomMaxZScreenY, screen.w, state.roomMinZScreenY + 1);
}

static void drawClippedDebugFrame(Graphics::Screen &screen, Common::Rect rect, byte color) {
	rect.clip(screen.getBounds());
	if (rect.isEmpty())
		return;

	screen.frameRect(rect, color);
}

static bool canEntityAffectPlayerPathfinding(const Entity &entity) {
	if (!entity.isVisible())
		return false;

	switch (entity.getClassId()) {
	case kEntityClassCursor:
	case kEntityClassBackground:
	case kEntityClassPlayer:
	case kEntityClassRectHotspot:
	case kEntityClassRectHotspot19:
	case kEntityClassTimer:
		return false;
	default:
		break;
	}

	return entity.getClassId() == kEntityClassDisabledHotspot || entity.hasFrames();
}

static void drawPathfindingBlockerOutline(Graphics::Screen &screen, EntityManager &entityManager,
		const Common::String &entityName, const Common::Rect &movementBounds, byte color) {
	const Entity *entity = entityManager.findSceneEntityByName(entityName);
	if (!entity || !canEntityAffectPlayerPathfinding(*entity))
		return;

	Common::Rect rect = entity->getScreenRect();
	if (rect.isEmpty())
		return;

	Common::Rect overlap = rect;
	overlap.clip(movementBounds);
	if (overlap.isEmpty())
		return;

	drawClippedDebugFrame(screen, rect, color);
}

static void drawPathfindingDebugOverlay(HarvesterEngine &engine, Graphics::Screen &screen,
		const SceneState &scene) {
	if (!engine.isPathfindingDebugEnabled())
		return;

	byte displayPalette[256 * 3];
	screen.getPalette(displayPalette);
	const byte blue = findNearestPaletteColor(displayPalette, 0x00, 0x40, 0xff);
	const byte green = findNearestPaletteColor(displayPalette, 0x00, 0xff, 0x00);
	const Common::Rect movementBounds = getPathfindingMovementBounds(screen, scene.state);
	if (movementBounds.isEmpty())
		return;

	drawClippedDebugFrame(screen, movementBounds, blue);

	EntityManager *entityManager = engine.getRuntimeEntities();
	if (!entityManager)
		return;

	for (const ObjectRecord &object : scene.sceneObjects)
		drawPathfindingBlockerOutline(screen, *entityManager,
			object.objectName, movementBounds, green);

	for (const AnimRecord &anim : scene.sceneAnimations)
		drawPathfindingBlockerOutline(screen, *entityManager,
			anim.animName, movementBounds, green);
}

static Common::String resolveTimerDebugLabel(const TimerRecord &timer, const Entity &entity) {
	const Common::String actionTag = timer.arg2.empty() ? Common::String("<none>") : timer.arg2;

	return Common::String::format("%s start=%d current=%d action=%s",
		timer.timerName.c_str(), entity.getTimerInitialValue(),
		entity.getTimerCurrentValue(), actionTag.c_str());
}

static void drawTimerDebugOverlay(HarvesterEngine &engine, Graphics::Screen &screen,
		const SceneState &) {
	if (!engine.isTimerDebugEnabled())
		return;

	const Graphics::Font *font = FontMan.getFontByUsage(Graphics::FontManager::kGUIFont);
	EntityManager *entityManager = engine.getRuntimeEntities();
	Script *script = engine.getScript();
	if (!font || !entityManager || !script)
		return;

	Common::Array<Common::String> labels;
	for (const TimerRecord &timer : script->getTimers()) {
		const Entity *entity = entityManager->findSceneEntityByName(timer.timerName);
		if (!entity || entity->getClassId() != kEntityClassTimer || !entity->isTimerEnabled())
			continue;

		const TimerRecord *runtimeTimer = script->findRuntimeTimerRecord(timer.timerName);
		labels.push_back(resolveTimerDebugLabel(runtimeTimer ? *runtimeTimer : timer, *entity));
	}
	if (labels.empty())
		return;

	byte displayPalette[256 * 3];
	screen.getPalette(displayPalette);
	const byte black = findNearestPaletteColor(displayPalette, 0x00, 0x00, 0x00);
	const byte white = findNearestPaletteColor(displayPalette, 0xff, 0xff, 0xff);

	const int labelHeight = font->getFontHeight() + kRoomDebugLabelPaddingY * 2;
	const int lineHeight = labelHeight + kTimerDebugOverlayLineSpacing;
	const int totalHeight = (int)labels.size() * labelHeight +
		(int)(labels.size() - 1) * kTimerDebugOverlayLineSpacing;
	const int x = screen.w / kTimerDebugOverlayXDivisor;
	const int y = CLIP<int>((screen.h - totalHeight) / 2, 0, MAX(0, screen.h - totalHeight));

	for (uint i = 0; i < labels.size(); ++i)
		drawRoomDebugLabel(screen, *font, labels[i], x, y + (int)i * lineHeight, white, black);
}

void RoomPresentation::drawRoomScene(HarvesterEngine &engine, Graphics::Screen &screen, const SceneState &scene,
		float brightness) {
	setScaledRoomPalette(screen, scene.palette, brightness);
	screen.fillRect(screen.getBounds(), 0);
	if (engine.getRuntimeEntities())
		engine.getRuntimeEntities()->drawSceneEntities(screen);
	drawRoomDebugOverlay(engine, screen, scene);
	drawCombatDebugOverlay(engine, screen, scene);
	drawPathfindingDebugOverlay(engine, screen, scene);
	drawTimerDebugOverlay(engine, screen, scene);
}

void RoomPresentation::blitBitmap(Graphics::Screen &screen, const IndexedBitmap &bitmap, int x, int y) {
	if (!bitmap.isValid())
		return;

	screen.copyRectToSurface(bitmap.pixels.data(), bitmap.width, x, y, bitmap.width, bitmap.height);
}

void RoomPresentation::drawShadowedString(Graphics::Screen &screen, const Graphics::Font &font, const Common::String &text,
		int x, int y, int width, byte color, Graphics::TextAlign align) {
	font.drawString(&screen, text, x + 1, y + 1, width, kShadowColor, align);
	font.drawString(&screen, text, x, y, width, color, align);
}

static void drawWrappedShadowedText(Graphics::Screen &screen, const Graphics::Font &font, const Common::String &text,
		int x, int y, int width, byte color) {
	Common::Array<Common::String> lines;
	font.wordWrapText(text, width, lines);

	const int lineHeight = font.getFontHeight() + 2;
	for (uint i = 0; i < lines.size(); ++i)
		RoomPresentation::drawShadowedString(screen, font, lines[i], x, y + i * lineHeight, width, color);
}

void RoomPresentation::drawWrappedText(Graphics::Screen &screen, const Graphics::Font &font, const Common::String &text,
		int x, int y, int width, byte color, int lineSpacing, bool useCftCharacterWrapping) {
	Common::Array<Common::String> lines;
	if (useCftCharacterWrapping)
		wrapCftTextByCharacterCount(font, text, width, lines);
	else
		font.wordWrapText(text, width, lines);

	const int lineHeight = font.getFontHeight() + lineSpacing;
	for (uint i = 0; i < lines.size(); ++i)
		font.drawString(&screen, lines[i], x, y + i * lineHeight, width, color);
}

const IndexedBitmap *RoomPresentation::resolveInspectTextboxBitmap(const Art &art, const ResolvedText &text) {
	if (text.boxName.equalsIgnoreCase("BOX1"))
		return art.getTextboxBitmap(0);
	if (text.boxName.equalsIgnoreCase("BOX2"))
		return art.getTextboxBitmap(1);
	if (text.boxName.equalsIgnoreCase("BOX3"))
		return art.getTextboxBitmap(2);
	if (text.boxName.equalsIgnoreCase("BOX4"))
		return art.getTextboxBitmap(3);

	return nullptr;
}

void RoomPresentation::drawRoomInspectText(Graphics::Screen &screen, const Art &art, const Graphics::Font &font,
		const ResolvedText &inspectText, bool useNativeFont) {
	const IndexedBitmap *textbox = resolveInspectTextboxBitmap(art, inspectText);
	if (!textbox || !textbox->isValid())
		return;

	blitBitmap(screen, *textbox, kIdentTextboxX, kIdentTextboxY);
	if (useNativeFont) {
		drawWrappedText(screen, font, inspectText.value,
			kIdentTextboxX + kIdentTextboxTextInsetX,
			kIdentTextboxY + kIdentTextboxTextInsetY,
			MAX<int>(0, (int)textbox->width - 2),
			0,
			kNativeIdentTextLineSpacing,
			true);
		return;
	}

	drawWrappedShadowedText(screen, font, inspectText.value,
		kIdentTextboxX + kIdentTextboxTextInsetX,
		kIdentTextboxY + kIdentTextboxTextInsetY,
		MAX<int>(0, (int)textbox->width - (kIdentTextboxTextInsetX + 2)),
		kIdentTextColor);
}

RoomPresentation::RoomPresentation(HarvesterEngine &engine, SceneState &scene,
		const Art *art, const Graphics::Font *inspectFont, bool useNativeInspectFont)
	: _engine(engine), _scene(scene), _art(art), _inspectFont(inspectFont),
	  _useNativeInspectFont(useNativeInspectFont) {
}

bool RoomPresentation::captureDialogueBackdrop(IndexedBitmap &dialogueBackdrop) {
	Graphics::Screen *activeScreen = _engine.getScreen();
	if (!activeScreen)
		return false;

	drawRoomScene(_engine, *activeScreen, _scene, _scene.targetPaletteBrightness);
	return captureScreenBackdrop(*activeScreen, dialogueBackdrop);
}

Common::Error RoomPresentation::applyLightingCommand(StartupLightingCommand lightingCommand) {
	Graphics::Screen *activeScreen = _engine.getScreen();
	if (!activeScreen)
		return Common::kNoError;

	switch (lightingCommand) {
	case kStartupLightingCommandNone:
		return Common::kNoError;
	case kStartupLightingCommandDim:
		_engine.getMedia()->clearRoomPaletteFade();
		_scene.targetPaletteBrightness = kDimPaletteBrightness;
		drawRoomScene(_engine, *activeScreen, _scene, _scene.targetPaletteBrightness);
		setScaledRoomPalette(*activeScreen, _scene.palette, _scene.targetPaletteBrightness);
		activeScreen->makeAllDirty();
		activeScreen->update();
		return Common::kNoError;
	case kStartupLightingCommandNormal:
		_engine.getMedia()->clearRoomPaletteFade();
		if (ResourceManager *resources = _engine.getResources()) {
			if (!_scene.state.palettePath.empty() &&
					!loadPaletteResource(*resources, _scene.state.palettePath, _scene.palette)) {
				warning("Harvester: unable to reload room palette '%s' for CHANGE_LIGHTING NORMAL",
					_scene.state.palettePath.c_str());
			}
		}
		_scene.targetPaletteBrightness = kNormalPaletteBrightness;
		drawRoomScene(_engine, *activeScreen, _scene, _scene.targetPaletteBrightness);
		setScaledRoomPalette(*activeScreen, _scene.palette, _scene.targetPaletteBrightness);
		activeScreen->makeAllDirty();
		activeScreen->update();
		return Common::kNoError;
	case kStartupLightingCommandBlack: {
		byte blackPalette[256 * 3] = { 0 };
		activeScreen->fillRect(activeScreen->getBounds(), 0);
		activeScreen->setPalette(blackPalette);
		activeScreen->makeAllDirty();
		activeScreen->update();
		return Common::kNoError;
	}
	case kStartupLightingCommandFadeIn:
		_engine.getMedia()->requestRoomFadeAfterMovie();
		debugC(2, kDebugRoom, "Harvester: room palette fade queued for the next movie");
		return Common::kNoError;
	}

	return Common::kNoError;
}

Common::Error RoomPresentation::runModalShowText(const ResolvedText &modalText, Flow &flow) {
	Graphics::Screen *activeScreen = _engine.getScreen();
	if (!activeScreen || !_art || !_inspectFont)
		return Common::kNoError;
	if (!resolveInspectTextboxBitmap(*_art, modalText)) {
		debug(1, "Harvester: unsupported SHOW_TEXT textbox '%s' text='%s'",
			modalText.boxName.c_str(), modalText.value.c_str());
		return Common::kNoError;
	}

	IndexedBitmap backdrop;
	if (!captureDialogueBackdrop(backdrop))
		return Common::kReadingFailed;

	Graphics::FrameLimiter limiter(g_system, 60);
	bool initialInputReleased = false;
	bool dismissPressed = false;
	uint frameCount = 0;

	while (!_engine.shouldQuit()) {
		Common::Event event;
		Common::Error result = Common::kNoError;
		while (g_system->getEventManager()->pollEvent(event)) {
			if (flow.handleSystemEvent(event, result))
				return result;

			if (!initialInputReleased) {
				if (event.type == Common::EVENT_LBUTTONUP ||
						event.type == Common::EVENT_RBUTTONUP ||
						event.type == Common::EVENT_KEYUP) {
					initialInputReleased = true;
				}
				continue;
			}

			if (!dismissPressed) {
				if (event.type == Common::EVENT_LBUTTONDOWN ||
						event.type == Common::EVENT_RBUTTONDOWN) {
					dismissPressed = true;
				} else if (event.type == Common::EVENT_KEYDOWN) {
					return Common::kNoError;
				}
				continue;
			}

			if (event.type == Common::EVENT_LBUTTONUP ||
					event.type == Common::EVENT_RBUTTONUP) {
				return Common::kNoError;
			}
		}

		if (!initialInputReleased && frameCount++ > 0)
			initialInputReleased = true;

		blitTransparentBitmap(*activeScreen, backdrop, 0, 0);
		setScaledRoomPalette(*activeScreen, _scene.palette, _scene.targetPaletteBrightness);
		drawRoomInspectText(*activeScreen, *_art, *_inspectFont, modalText, _useNativeInspectFont);
		activeScreen->makeAllDirty();
		activeScreen->update();

		limiter.delayBeforeSwap();
		limiter.startFrame();
	}

	return Common::kNoError;
}

} // End of namespace Harvester
