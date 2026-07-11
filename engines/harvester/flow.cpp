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

#include <math.h>

#include "harvester/flow.h"

#include "common/algorithm.h"
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/endian.h"
#include "common/events.h"
#include "common/formats/ini-file.h"
#include "common/memstream.h"
#include "common/ptr.h"
#include "common/serializer.h"
#include "common/system.h"
#include "graphics/blit.h"
#include "graphics/font.h"
#include "graphics/fontman.h"
#include "graphics/framelimiter.h"
#include "harvester/cft_font.h"
#include "harvester/detection.h"
#include "harvester/harvester.h"
#include "harvester/palette_utils.h"
#include "harvester/resources.h"
#include "harvester/runtime_entity.h"
#include "harvester/art.h"
#include "harvester/monster.h"
#include "harvester/player.h"
#include "harvester/script.h"

namespace Harvester {

namespace {

static const char *const kQuickTipsPath = "ADJHEAD.RCS";
static const char *const kMenuPath = "MENU.INI";
static const char *const kMenuSectionName = "menu";
static const char *const kDemoMenuItems[] = {
	"NEW GAME",
	"SAVE GAME",
	"LOAD GAME",
	"OPTIONS",
	"HELP",
	"QUIT GAME"
};
static const char *const kTownMapPalettePath = "1:/GRAPHIC/PAL/HARVMAP.PAL";
static const char *const kTownMapMusicPath = "SOUND/MUSIC/MENACE.CMP";
static const char *const kTownMapBitmapPaths[] = {
	"1:/GRAPHIC/TOWN/HARVMAP1.BM",
	"1:/GRAPHIC/TOWN/HARVMAP2.BM",
	"1:/GRAPHIC/TOWN/HARVMAP3.BM",
	"1:/GRAPHIC/TOWN/HARVMAP4.BM"
};

static const int kQuickTipsOverlayX = 167;
static const int kQuickTipsOverlayY = 200;

static const int kQuickTipTextX = 180;
static const int kQuickTipTextY = 228;
static const int kQuickTipTextWidth = 280;

static const int kCursorSequence7 = 7;
static const int kIdentTextboxX = 177;
static const int kIdentTextboxY = 85;
static const int kIdentTextboxTextInsetX = 10;
static const int kIdentTextboxTextInsetY = 5;
static const int kNativeIdentTextLineSpacing = 3;
static const char *const kPlayerActorEntityName = "PLAYER";
static const uint32 kPaletteFadeTickMs = 4;
static const float kPaletteFadeStep = 0.1f;
static const float kPaletteBrightnessBlack = 0.0f;
static const float kPaletteBrightnessFull = 1.0f;
static const uint32 kDemoEndingScreenDurationMs = 10000;
static const int kDemoEndingScreenNumbers[] = { 1, 2, 3, 4, 6 };
static const char *const kDemoEndingMusicPath = "SOUND/MUSIC/ROCKFITE.CMP";
static const float kTownMapNightPaletteBrightness = 0.6f;
// Native keeps WAIT_BM visible while room construction runs; fast local loads need a short floor.
static const uint32 kTransitionWaitFrameMinMs = 120;
static const int kRoomNpcAmbientLastFrame = 0x3b;
static const float kNativeNpcMonsterZExtent = 5.0f;
static const int kTownMapEdgeThreshold = 9;
static const int kTownMapCursorHitExtent = 5;

static const byte kIdentTextColor = 0xd3;
static const byte kTextColorNormal = 255;
static const byte kShadowColor = 0;
static const byte kQuickTipActionColor = 0xc3;
static const byte kTownMapLabelColor = 0x28;
static const int kRoomDebugLabelPaddingX = 3;
static const int kRoomDebugLabelPaddingY = 2;
static const int kRoomDebugRoomNameX = 4;
static const int kRoomDebugRoomNameY = 4;
static const int kTimerDebugOverlayXDivisor = 8;
static const int kTimerDebugOverlayLineSpacing = 1;

static const int kCursorSequenceWalk = 0;
static const int kCursorSequenceExamine = 1;
static const int kCursorSequenceTalk = 2;
static const int kCursorSequenceOperate = 4;
static const int kCursorSequencePickup = 5;
static const int kCursorSequenceTransition = 6;
static const int kCursorSequenceNeutral = 7;
} // End of anonymous namespace

static const CftFontResource *findStartupFontByName(const HarvesterEngine &engine, const char *fontName) {
	const Text *text = engine.getText();
	if (!text || !fontName)
		return nullptr;

	for (const CftFontResource &font : text->getFonts()) {
		if (font.name.equalsIgnoreCase(fontName))
			return &font;
	}

	return nullptr;
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
		const RoomSceneResources &scene) {
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
		if (entity && entity->getClassId() == kRuntimeEntityClassBackground)
			continue;

		drawRoomDebugLabel(screen, *font, resolveRoomDebugObjectLabel(engine, object),
			object.currentX, object.currentY, white, black);
	}

	for (const NpcRecord &npc : scene.state.roomNpcs) {
		const Entity *entity = entityManager
			? entityManager->findSceneEntityByName(npc.npcName)
			: nullptr;
		if (!entity || entity->getClassId() != kRuntimeEntityClassNpc || !entity->isVisible())
			continue;

		const Common::Rect npcRect = entity->getScreenRect();
		drawRoomDebugLabel(screen, *font, resolveRoomDebugNpcLabel(npc),
			npcRect.left, npcRect.top, white, red);
	}
}

static void drawCombatDebugOverlay(HarvesterEngine &engine, Graphics::Screen &screen,
		const RoomSceneResources &scene) {
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
			playerEntity->getClassId() == kRuntimeEntityClassPlayer &&
			playerEntity->isVisible()) {
		const Common::Rect playerRect = playerEntity->getScreenRect();
		drawRoomDebugLabel(screen, *font,
			resolveCombatDebugLabel(playerEntity->getName(),
				script->getPlayerCurrentHitPoints(), Script::kDefaultPlayerHitPoints),
			playerRect.left, playerRect.top, white, red);
	}

	for (const MonsterRecord &monster : scene.state.roomMonsters) {
		const Entity *entity = entityManager->findSceneEntityByName(monster.monsterName);
		if (!entity || entity->getClassId() != kRuntimeEntityClassMonster || !entity->isVisible())
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
	case kRuntimeEntityClassCursor:
	case kRuntimeEntityClassBackground:
	case kRuntimeEntityClassPlayer:
	case kRuntimeEntityClassRectHotspot:
	case kRuntimeEntityClassRectHotspot19:
	case kRuntimeEntityClassTimer:
		return false;
	default:
		break;
	}

	return entity.getClassId() == kRuntimeEntityClassDisabledHotspot || entity.hasFrames();
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
		const RoomSceneResources &scene) {
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
		const RoomSceneResources &) {
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
		if (!entity || entity->getClassId() != kRuntimeEntityClassTimer || !entity->isTimerEnabled())
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

static int clampTownMapPanelIndex(int panelIndex) {
	return CLIP<int>(panelIndex, 0, ARRAYSIZE(kTownMapBitmapPaths) - 1);
}

static int resolveTownMapEdgePanel(int currentPanel, const Common::Point &mousePos, int width, int height) {
	if (mousePos.x <= kTownMapEdgeThreshold) {
		if (currentPanel == 1)
			return 0;
		if (currentPanel == 3)
			return 2;
	} else if (mousePos.x >= width - 1 - kTownMapEdgeThreshold) {
		if (currentPanel == 0)
			return 1;
		if (currentPanel == 2)
			return 3;
	} else if (mousePos.y <= kTownMapEdgeThreshold) {
		if (currentPanel == 2)
			return 0;
		if (currentPanel == 3)
			return 1;
	} else if (mousePos.y >= height - 1 - kTownMapEdgeThreshold) {
		if (currentPanel == 0)
			return 2;
		if (currentPanel == 1)
			return 3;
	}

	return currentPanel;
}

static int resolveTownMapKeyPanel(int currentPanel, Common::KeyCode keycode) {
	switch (keycode) {
	case Common::KEYCODE_LEFT:
		if (currentPanel == 1)
			return 0;
		if (currentPanel == 3)
			return 2;
		break;
	case Common::KEYCODE_RIGHT:
		if (currentPanel == 0)
			return 1;
		if (currentPanel == 2)
			return 3;
		break;
	case Common::KEYCODE_UP:
		if (currentPanel == 2)
			return 0;
		if (currentPanel == 3)
			return 1;
		break;
	case Common::KEYCODE_DOWN:
		if (currentPanel == 0)
			return 2;
		if (currentPanel == 1)
			return 3;
		break;
	default:
		break;
	}

	return currentPanel;
}

static bool isTownMapLocationHovered(const MapLocationRecord &location, int currentPanel,
		const Common::Point &mousePos) {
	if (location.panelIndex != currentPanel)
		return false;

	const int cursorRight = mousePos.x + kTownMapCursorHitExtent;
	const int cursorBottom = mousePos.y + kTownMapCursorHitExtent;
	return mousePos.x <= location.maxX && cursorRight >= location.minX &&
		mousePos.y <= location.maxY && cursorBottom >= location.minY;
}

static const MapLocationRecord *findTownMapLocationAt(
		const Common::Array<MapLocationRecord> &locations, int currentPanel,
		const Common::Point &mousePos) {
	for (const MapLocationRecord &location : locations) {
		if (isTownMapLocationHovered(location, currentPanel, mousePos))
			return &location;
	}

	return nullptr;
}

static void setRoomActorScreenPosition(Entity &entity, int centerX, int bottomY, float z,
		int width, int height, int xOffset, int yOffset) {
	entity.setPosition(centerX - xOffset - width / 2, bottomY - height - yOffset, z);
}

static float computeRoomActorRenderZ(float z, const Entity &entity) {
	// Native actor spawns lower the render-list depth anchor by half the z extent
	// before insertion. Without that adjustment, cemetery Karin lands one layer
	// too far back and gets occluded by the grave mask in CEM10.
	return z - floorf(MAX<float>(entity.getZExtent(), 0.0f) * 0.5f);
}

static bool applyRoomActorPlacementInternal(const RoomSetupState &state, Entity &entity,
		int centerX, int bottomY, float z, const Common::String *entranceName, bool applyDepthScale) {
	int width = 0;
	int height = 0;
	int xOffset = 0;
	int yOffset = 0;
	if (!entity.getCurrentFrameMetrics(width, height, xOffset, yOffset))
		return false;

	const float renderZ = computeRoomActorRenderZ(z, entity);
	entity.setAnchorMode(kRuntimeEntityAnchorTopLeft);
	setRoomActorScreenPosition(entity, centerX, bottomY, renderZ, width, height, xOffset, yOffset);

	float depthScale = 1.0f;
	if (applyDepthScale) {
		depthScale = Player::computeDepthScale(state, renderZ);
		entity.setDepthScale(depthScale);
		if (!entity.getCurrentFrameMetrics(width, height, xOffset, yOffset))
			return false;
		setRoomActorScreenPosition(entity, centerX, bottomY, renderZ, width, height, xOffset, yOffset);
	}

	return true;
}

bool applyRoomActorPlacement(const RoomSetupState &state, Entity &entity,
		int centerX, int bottomY, float z, const Common::String *entranceName) {
	return applyRoomActorPlacementInternal(state, entity, centerX, bottomY, z, entranceName, true);
}

static bool applyStartupActorPlacement(const RoomSetupState &state, Entity &entity) {
	return applyRoomActorPlacement(state, entity,
		state.playerSpawnX, state.playerSpawnY, (float)state.playerSpawnZ, &state.entranceName);
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

void logScenePaletteSummary(const char *label, const RoomSceneResources &scene, float brightness) {
	const byte *palette = scene.palette;
	byte minValue = 0;
	byte maxValue = 0;
	getPaletteByteRange(palette, minValue, maxValue);

	debugC(1, kDebugRoom,
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

bool captureScreenBackdrop(const Graphics::Screen &screen, IndexedBitmap &bitmap) {
	if (screen.w <= 0 || screen.h <= 0 || screen.format.bytesPerPixel != 1)
		return false;

	bitmap.width = (uint32)screen.w;
	bitmap.height = (uint32)screen.h;
	bitmap.pixels.resize(bitmap.width * bitmap.height);
	for (int y = 0; y < screen.h; ++y)
		memcpy(bitmap.pixels.data() + y * bitmap.width, screen.getBasePtr(0, y), bitmap.width);

	return true;
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

static void drawWrappedText(Graphics::Screen &screen, const Graphics::Font &font, const Common::String &text,
		int x, int y, int width, byte color, int lineSpacing) {
	Common::Array<Common::String> lines;
	font.wordWrapText(text, width, lines);

	const int lineHeight = font.getFontHeight() + lineSpacing;
	for (uint i = 0; i < lines.size(); ++i)
		font.drawString(&screen, lines[i], x, y + i * lineHeight, width, color);
}

static void setScaledPalette(Graphics::Screen &screen, const byte *palette, float brightness) {
	byte scaledPalette[256 * 3];
	const float gammaBrightness = g_engine ? g_engine->getGammaBrightnessScale() : 1.0f;
	buildHarvesterDisplayPalette(palette, brightness * gammaBrightness, scaledPalette);
	screen.setPalette(scaledPalette);
}

static void setScaledDisplayPalette(Graphics::Screen &screen, const byte *palette, float brightness) {
	byte scaledPalette[256 * 3];
	const float clampedBrightness = CLIP<float>(brightness, 0.0f, 1.0f);
	for (uint i = 0; i < ARRAYSIZE(scaledPalette); ++i)
		scaledPalette[i] = (byte)CLIP<int>((int)(palette[i] * clampedBrightness + 0.5f), 0, 255);
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
	debugC(1, kDebugRoom,
		"Harvester: decoded room palette '%s' bytes=%u range=[%u,%u] idx0=(%u,%u,%u) idx255=(%u,%u,%u)",
		path.c_str(), (uint)data.size(), minValue, maxValue,
		dest[0], dest[1], dest[2],
		dest[255 * 3], dest[255 * 3 + 1], dest[255 * 3 + 2]);

	return true;
}

Common::Rect getRoomObjectHotspotBounds(const ObjectRecord &object) {
	if (object.boundsX2 > object.currentX && object.boundsY2 > object.currentY)
		return Common::Rect(object.currentX, object.currentY, object.boundsX2 + 1, object.boundsY2 + 1);

	return Common::Rect();
}

static Common::Rect getRegionBounds(const RegionRecord &region) {
	if (region.right > region.left && region.bottom > region.top)
		return Common::Rect(region.left, region.top, region.right + 1, region.bottom + 1);

	return Common::Rect();
}

Common::String resolveSceneObjectSpritePath(const ObjectRecord &object) {
	const bool atInitialPlacement = object.currentX == object.initialX &&
		object.currentY == object.initialY &&
		object.currentOwnerOrRoom.equalsIgnoreCase(object.initialOwnerOrRoom);
	if (!object.altSpritePath.empty() &&
		(!atInitialPlacement || object.currentOwnerOrRoom.equalsIgnoreCase("INVENTORY")))
		return object.altSpritePath;

	return object.spritePath;
}

bool loadBitmapResource(ResourceManager &resources, const Common::String &path, IndexedBitmap &bitmap) {
	Common::Array<byte> data;
	if (!resources.loadFile(path, data) || data.size() < 12)
		return false;

	bitmap = IndexedBitmap();
	bitmap.width = READ_LE_UINT32(data.data());
	bitmap.height = READ_LE_UINT32(data.data() + 4);
	const uint32 pixelCount = bitmap.width * bitmap.height;
	if (bitmap.width == 0 || bitmap.height == 0 || data.size() < 12 + pixelCount)
		return false;

	bitmap.pixels.resize(pixelCount);
	memcpy(bitmap.pixels.data(), data.data() + 12, pixelCount);
	return true;
}

static void logSceneObjectSelection(const char *decision, const char *source, const ObjectRecord &object,
		const Common::String &detail = Common::String()) {
	const Common::Rect hotspotBounds = getRoomObjectHotspotBounds(object);
	const Common::String resolvedSpritePath = resolveSceneObjectSpritePath(object);
	debugC(1, kDebugScene,
		"Harvester: scene object %s source='%s' object='%s' owner='%s' visible=%d runtimeVisible=%d sprite='%s' alt='%s' resolved='%s' pos=(%d,%d,%d) bounds=(%d,%d)-(%d,%d) action='%s' detail='%s'",
		decision, source, object.objectName.c_str(), object.currentOwnerOrRoom.c_str(),
		object.visible, object.runtimeVisible, object.spritePath.c_str(), object.altSpritePath.c_str(),
		resolvedSpritePath.c_str(),
		object.currentX, object.currentY, object.currentZ,
		hotspotBounds.left, hotspotBounds.top, hotspotBounds.right, hotspotBounds.bottom,
		object.actionTag.c_str(), detail.c_str());
}

static bool isBackgroundSceneObject(const RoomSetupState &state,
		const ObjectRecord &object, const Entity &entity) {
	if (!state.backgroundObjectName.empty() &&
			object.objectName.equalsIgnoreCase(state.backgroundObjectName)) {
		return true;
	}

	return object.initialX == 0 && object.initialY == 0 &&
		entity.getBoundsWidth() == 640 && entity.getBoundsHeight() == 480;
}

static bool isInteractiveSceneHotspot(const ObjectRecord &object, Script *script) {
	if (object.operatable || !object.actionTag.empty())
		return true;
	if (!script)
		return false;
	if (script->isPickupObject(object))
		return true;

	ResolvedText inspectText;
	return script->resolveObjectInspectText(object, inspectText);
}

static int resolveSceneObjectClass(const RoomSetupState &state,
		const ObjectRecord &object, const Entity *entity, Script *script) {
	if (entity)
		return isBackgroundSceneObject(state, object, *entity)
			? kRuntimeEntityClassBackground
			: kRuntimeEntityClassObject;

	return isInteractiveSceneHotspot(object, script)
		? kRuntimeEntityClassRectHotspot
		: kRuntimeEntityClassDisabledHotspot;
}

static bool shouldQueueSceneObject(const ObjectRecord &object) {
	return object.visible;
}

bool shouldDisplaySceneAnimation(const AnimRecord &anim) {
	return anim.active || anim.visible;
}

static void queueSceneObject(const char *source, const ObjectRecord &object,
		Common::Array<ObjectRecord> &sceneObjects) {
	if (!shouldQueueSceneObject(object)) {
		logSceneObjectSelection("skipped", source, object, object.visible ? "inactive" : "visible=0");
		return;
	}

	for (const ObjectRecord &sceneObject : sceneObjects) {
		if (sceneObject.currentOwnerOrRoom.equalsIgnoreCase(object.currentOwnerOrRoom) &&
			sceneObject.objectName.equalsIgnoreCase(object.objectName)) {
			logSceneObjectSelection("skipped", source, object, "duplicate owner/object");
			return;
		}
	}

	sceneObjects.push_back(object);
	logSceneObjectSelection("queued", source, object);
}

bool loadRoomSceneResources(const RoomSetupState &state, ResourceManager &resources, RoomSceneResources &scene) {
	scene = RoomSceneResources();
	scene.state = state;
	scene.targetPaletteBrightness = state.paletteBrightness;
	if (!loadPaletteResource(resources, state.palettePath, scene.palette)) {
		return false;
	}

	Common::Array<ObjectRecord> sceneObjects;
	for (const ObjectRecord &object : state.roomObjects)
		queueSceneObject("room", object, sceneObjects);
	for (const ObjectRecord &object : state.activeObjects)
		queueSceneObject("active", object, sceneObjects);

	scene.sceneObjects = Common::move(sceneObjects);
	for (const AnimRecord &anim : state.roomAnimations) {
		if (shouldDisplaySceneAnimation(anim))
			scene.sceneAnimations.push_back(anim);
	}
	scene.sceneRegions = state.roomRegions;

	debugC(1, kDebugRoom,
		"Harvester: loadRoomSceneResources room='%s' palette='%s' background='%s' brightness=%.2f objects=%u activeObjects=%u sceneObjects=%u anims=%u visibleAnims=%u npcs=%u monsters=%u regions=%u",
		state.roomName.c_str(), state.palettePath.c_str(), state.backgroundPath.c_str(), (double)state.paletteBrightness,
		(uint)state.roomObjects.size(), (uint)state.activeObjects.size(), (uint)scene.sceneObjects.size(),
		(uint)state.roomAnimations.size(), (uint)scene.sceneAnimations.size(),
		(uint)state.roomNpcs.size(), (uint)state.roomMonsters.size(), (uint)scene.sceneRegions.size());

	return true;
}

bool shouldRunStartupRoomProbe() {
	return ConfMan.hasKey("harvester_debug_probe_startup_room") &&
		ConfMan.getBool("harvester_debug_probe_startup_room");
}

void drawRoomScene(HarvesterEngine &engine, Graphics::Screen &screen, const RoomSceneResources &scene,
		float brightness) {
	setScaledPalette(screen, scene.palette, brightness);
	screen.fillRect(screen.getBounds(), 0);
	if (engine.getRuntimeEntities())
		engine.getRuntimeEntities()->drawSceneEntities(screen);
	drawRoomDebugOverlay(engine, screen, scene);
	drawCombatDebugOverlay(engine, screen, scene);
	drawPathfindingDebugOverlay(engine, screen, scene);
	drawTimerDebugOverlay(engine, screen, scene);
}

const ObjectRecord *findSceneObjectByName(const Common::Array<ObjectRecord> &objects,
		const Common::String &objectName) {
	for (const ObjectRecord &object : objects) {
		if (object.objectName.equalsIgnoreCase(objectName))
			return &object;
	}

	return nullptr;
}

const RegionRecord *findSceneRegionByName(const Common::Array<RegionRecord> &regions,
		const Common::String &regionName) {
	for (const RegionRecord &region : regions) {
		if (region.regionName.equalsIgnoreCase(regionName))
			return &region;
	}

	return nullptr;
}

static const NpcRecord *findSceneNpcByName(const Common::Array<NpcRecord> &npcs,
		const Common::String &npcName) {
	for (const NpcRecord &npc : npcs) {
		if (npc.npcName.equalsIgnoreCase(npcName))
			return &npc;
	}

	return nullptr;
}

static bool canTalkToRoomNpc(HarvesterEngine &engine, const NpcRecord &npc,
		const DialogueSystem *dialogue) {
	Script *script = engine.getScript();
	if (!script || !script->isNamedNpcDeathTypeClear(npc.npcName))
		return false;

	return dialogue && dialogue->hasRoomNpcHandler(npc.npcName);
}

ObjectRecord *findSceneObjectByName(Common::Array<ObjectRecord> &objects,
		const Common::String &objectName) {
	for (ObjectRecord &object : objects) {
		if (object.objectName.equalsIgnoreCase(objectName))
			return &object;
	}

	return nullptr;
}

static const ObjectRecord *findRoomObjectAtPoint(HarvesterEngine &engine,
		const Common::Array<ObjectRecord> &sceneObjects, const Common::Point &point) {
	EntityManager *entityManager = engine.getRuntimeEntities();
	if (!entityManager)
		return nullptr;

	const Entity *topEntity = nullptr;
	const ObjectRecord *topObject = nullptr;
	int topDrawIndex = -1;
	for (const ObjectRecord &object : sceneObjects) {
		if (object.objectName.empty())
			continue;

		const Entity *entity = entityManager->findSceneEntityByName(object.objectName);
		if (!entity || !entity->hitTest(point))
			continue;
		if (entity->getClassId() == kRuntimeEntityClassBackground ||
			entity->getClassId() == kRuntimeEntityClassPlayer ||
			entity->getClassId() == kRuntimeEntityClassRectHotspot19) {
			continue;
		}
		const int drawIndex = entityManager->findSceneEntityDrawIndexByName(entity->getName());
		if (drawIndex < 0)
			continue;
		const bool objectVisible = object.visible && object.runtimeVisible;
		const bool topObjectVisible = topObject && topObject->visible && topObject->runtimeVisible;
		if (!topEntity ||
			(objectVisible && !topObjectVisible) ||
			(objectVisible == topObjectVisible && drawIndex > topDrawIndex)) {
			topEntity = entity;
			topObject = &object;
			topDrawIndex = drawIndex;
		}
	}
	if (!topEntity)
		return nullptr;

	return topObject;
}

static const RegionRecord *findRoomRegionAtPoint(HarvesterEngine &engine,
		const Common::Array<RegionRecord> &sceneRegions, const Common::Point &point) {
	EntityManager *entityManager = engine.getRuntimeEntities();
	if (!entityManager)
		return nullptr;

	const Entity *topEntity = nullptr;
	int topDrawIndex = -1;
	for (const RegionRecord &region : sceneRegions) {
		const Entity *entity = entityManager->findSceneEntityByName(region.regionName);
		if (!entity || !entity->hitTest(point))
			continue;
		if (entity->getClassId() != kRuntimeEntityClassRectHotspot19)
			continue;
		const int drawIndex = entityManager->findSceneEntityDrawIndexByName(entity->getName());
		if (drawIndex < 0)
			continue;
		if (!topEntity || drawIndex > topDrawIndex) {
			topEntity = entity;
			topDrawIndex = drawIndex;
		}
	}
	if (!topEntity)
		return nullptr;

	return findSceneRegionByName(sceneRegions, topEntity->getName());
}

static const Entity *findRoomPlayerAtPoint(HarvesterEngine &engine, const Common::Point &point) {
	EntityManager *entityManager = engine.getRuntimeEntities();
	if (!entityManager)
		return nullptr;

	const Entity *entity = entityManager->findTopSceneEntityAt(point);
	if (!entity || entity->getClassId() != kRuntimeEntityClassPlayer)
		return nullptr;

	return entity;
}

static const NpcRecord *findRoomNpcAtPoint(HarvesterEngine &engine,
		const Common::Array<NpcRecord> &sceneNpcs, const Common::Point &point,
		const DialogueSystem *dialogue) {
	EntityManager *entityManager = engine.getRuntimeEntities();
	if (!entityManager)
		return nullptr;

	const Entity *entity = entityManager->findTopSceneEntityAt(point);
	if (!entity || entity->getClassId() != kRuntimeEntityClassNpc)
		return nullptr;

	const NpcRecord *npc = findSceneNpcByName(sceneNpcs, entity->getName());
	if (!npc || !canTalkToRoomNpc(engine, *npc, dialogue))
		return nullptr;

	return npc;
}

const IndexedBitmap *resolveInspectTextboxBitmap(const Art &art, const ResolvedText &text) {
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

void drawRoomInspectText(Graphics::Screen &screen, const Art &art, const Graphics::Font &font,
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
			kNativeIdentTextLineSpacing);
		return;
	}

	drawWrappedShadowedText(screen, font, inspectText.value,
		kIdentTextboxX + kIdentTextboxTextInsetX,
		kIdentTextboxY + kIdentTextboxTextInsetY,
		MAX<int>(0, (int)textbox->width - (kIdentTextboxTextInsetX + 2)),
		kIdentTextColor);
}

static bool suppressesInitialObjectInspectGate(const ObjectRecord &object) {
	// Native HARVEST.SCR uses ident key X as a no-text sentinel.
	return object.identTextKey.equalsIgnoreCase("X");
}

static bool usesBareOperatePrompt(const ObjectRecord &object) {
	return object.objectName.equalsIgnoreCase("HAPPLY_HS") ||
		object.objectName.equalsIgnoreCase("KILL_STEPH_HS");
}

bool unlocksRoomObjectInteractionAfterInitialExamine(const ObjectRecord &object,
		Script &script) {
	if (suppressesInitialObjectInspectGate(object))
		return false;

	return object.operatable || script.isPickupObject(object);
}

static int resolveRoomObjectCursorSequence(const ObjectRecord &object, Script &script) {
	if (object.objectName.equalsIgnoreCase("EXIT_BM") || object.objectName.equalsIgnoreCase("EXIT_HS"))
		return kCursorSequenceTransition;

	ResolvedText inspectText;
	const bool pickupObject = script.isPickupObject(object);
	const bool pickupBlocked = pickupObject && script.isPickupBlockedByAction(object);
	if (!object.identShown && unlocksRoomObjectInteractionAfterInitialExamine(object, script))
		return kCursorSequenceExamine;
	if (pickupObject && !pickupBlocked)
		return kCursorSequencePickup;
	if (pickupBlocked)
		return kCursorSequenceExamine;
	if (object.operatable)
		return kCursorSequenceOperate;
	if (script.resolveObjectInspectText(object, inspectText))
		return kCursorSequenceExamine;

	if (script.hasObjectInteraction(object))
		return kCursorSequenceExamine;

	return kCursorSequenceNeutral;
}

static Common::String buildRoomObjectPrompt(const ObjectRecord &object, Script &script,
		int cursorSequence) {
	// Native room prompts come from explicit interaction metadata. Neutral scene sprites
	// should not surface synthetic "Examine <object id>" prompts or steal hover/click focus.
	if (cursorSequence == kCursorSequenceNeutral)
		return Common::String();

	const Common::String label = script.resolveObjectLabel(object);
	if (label.empty())
		return Common::String();

	if (cursorSequence == kCursorSequenceOperate) {
		if (usesBareOperatePrompt(object))
			return label;
		return Common::String::format("Operate the %s", label.c_str());
	}
	if (cursorSequence == kCursorSequencePickup)
		return Common::String::format("Pick up the %s", label.c_str());
	if (cursorSequence == kCursorSequenceTalk)
		return Common::String::format("Talk to %s", label.c_str());
	if (cursorSequence == kCursorSequenceTransition)
		return Common::String::format("Go to %s", label.c_str());

	return Common::String::format("Examine %s", label.c_str());
}

static Common::String buildRoomNpcPrompt(const NpcRecord &npc) {
	Common::String label = !npc.entityInitArg.empty() ? npc.entityInitArg : npc.npcName;
	for (uint i = 0; i < label.size(); ++i) {
		if (label[i] == '_')
			label.setChar(' ', i);
	}

	if (label.empty())
		return Common::String();
	return Common::String::format("Talk to %s", label.c_str());
}

bool doesPlayerFacingMatchRegion(int playerFacing, const RegionRecord &region) {
	return region.desiredFacing < 0 || playerFacing == region.desiredFacing;
}

bool doesPlayerOverlapRegion(const Entity &playerEntity, const RegionRecord &region) {
	const Common::Rect regionBounds = getRegionBounds(region);
	if (regionBounds.isEmpty())
		return false;
	if (!playerEntity.getScreenRect().intersects(regionBounds))
		return false;

	const float playerMaxZ = playerEntity.getZ() + playerEntity.getZExtent();
	return playerMaxZ >= (float)region.minZ && (float)region.maxZ >= playerEntity.getZ();
}

RoomHoverState resolveRoomHoverState(HarvesterEngine &engine, const RoomSetupState &state,
		const Common::Array<ObjectRecord> &sceneObjects,
		const Common::Array<NpcRecord> &sceneNpcs,
		const Common::Array<RegionRecord> &sceneRegions, const Common::Point &mousePos,
		const DialogueSystem *dialogue) {
	RoomHoverState hoverState;
	EntityManager *entityManager = engine.getRuntimeEntities();
	if (const Entity *playerEntity = findRoomPlayerAtPoint(engine, mousePos)) {
		hoverState.playerEntity = playerEntity;
		hoverState.cursorSequence = kCursorSequenceExamine;
		return hoverState;
	}
	if (const NpcRecord *npc = findRoomNpcAtPoint(engine, sceneNpcs, mousePos, dialogue)) {
		hoverState.npc = npc;
		hoverState.cursorSequence = kCursorSequenceTalk;
		hoverState.promptText = buildRoomNpcPrompt(*npc);
		return hoverState;
	}

	Script *script = engine.getScript();
	if (!script)
		return hoverState;
	hoverState.object = findRoomObjectAtPoint(engine, sceneObjects, mousePos);
	hoverState.region = findRoomRegionAtPoint(engine, sceneRegions, mousePos);
	if (entityManager && hoverState.object && hoverState.region) {
		const int objectDrawIndex =
			entityManager->findSceneEntityDrawIndexByName(hoverState.object->objectName);
		const int regionDrawIndex =
			entityManager->findSceneEntityDrawIndexByName(hoverState.region->regionName);
		if (regionDrawIndex >= 0 && objectDrawIndex >= 0 && regionDrawIndex > objectDrawIndex)
			hoverState.object = nullptr;
	}
	if (hoverState.object) {
		hoverState.cursorSequence = resolveRoomObjectCursorSequence(*hoverState.object, *script);
		hoverState.promptText = buildRoomObjectPrompt(*hoverState.object, *script, hoverState.cursorSequence);
		if (hoverState.cursorSequence != kCursorSequenceNeutral || !hoverState.promptText.empty())
			return hoverState;
		hoverState.object = nullptr;
	}
	if (hoverState.region && hoverState.region->cursorEnabled) {
		hoverState.object = nullptr;
		hoverState.cursorSequence = kCursorSequenceTransition;
		hoverState.promptText.clear();
		return hoverState;
	}

	if (Player::supportsMovementBand(state) &&
		mousePos.y >= state.roomMaxZScreenY &&
		mousePos.y <= state.roomMinZScreenY) {
		hoverState.cursorSequence = kCursorSequenceWalk;
	}

	return hoverState;
}

static bool findRoomObjectProbePoint(HarvesterEngine &engine, const Common::Array<ObjectRecord> &sceneObjects,
		const ObjectRecord &object, Common::Point &probePoint) {
	const Common::Rect bounds = getRoomObjectHotspotBounds(object);
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
		const ObjectRecord *hit = findRoomObjectAtPoint(engine, sceneObjects, candidate);
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
			const ObjectRecord *hit = findRoomObjectAtPoint(engine, sceneObjects, candidate);
			if (hit && hit->objectName.equalsIgnoreCase(object.objectName)) {
				probePoint = candidate;
				return true;
			}
		}
	}

	return false;
}

void logStartupRoomProbe(HarvesterEngine &engine, const RoomSceneResources &scene,
		const Common::String &entranceName, Common::Point &mousePos) {
	EntityManager *entityManager = engine.getRuntimeEntities();
	Script *script = engine.getScript();
	if (!entityManager || !script)
		return;

	if (const Entity *cursor = entityManager->getCursorEntity()) {
		uint32 framePixels = 0;
		uint32 transparentPixels = 0;
		uint32 preservedPixels = 0;
		if (cursor->measureCurrentFrameTransparency(framePixels, transparentPixels, preservedPixels)) {
			debugC(1, kDebugCursor,
				"Harvester: startup probe cursor sequence=%d frame=%d pixels=%u transparent_pixels=%u preserved_pixels=%u",
				cursor->getAnimationSequence(), cursor->getCurrentFrame(), framePixels, transparentPixels, preservedPixels);
		}
	}

	if (const Entity *player = entityManager->findSceneEntityByName(kPlayerActorEntityName)) {
		uint32 framePixels = 0;
		uint32 transparentPixels = 0;
		uint32 preservedPixels = 0;
		(void)player->measureCurrentFrameTransparency(framePixels, transparentPixels, preservedPixels);
		const Common::Rect playerRect = player->getScreenRect();
		const Common::Rect screenRect(0, 0, 640, 480);
		debugC(1, kDebugPlayer,
			"Harvester: startup probe player room='%s' entrance='%s' frame=%d rect=(%d,%d)-(%d,%d) visible=%d intersects_screen=%d opaque_pixels=%u",
			scene.state.roomName.c_str(), entranceName.c_str(), player->getCurrentFrame(),
			playerRect.left, playerRect.top, playerRect.right, playerRect.bottom,
			player->isVisible(), playerRect.intersects(screenRect), framePixels - transparentPixels);
	}

	for (const ObjectRecord &object : scene.sceneObjects) {
		if (getRoomObjectHotspotBounds(object).isEmpty())
			continue;

		Common::Point probePoint;
		if (!findRoomObjectProbePoint(engine, scene.sceneObjects, object, probePoint))
			continue;

		const ObjectRecord *hoveredObject = findRoomObjectAtPoint(engine, scene.sceneObjects, probePoint);
		if (!hoveredObject)
			continue;

		mousePos = probePoint;
		const Common::String objectLabel = script->resolveObjectLabel(*hoveredObject);
		ResolvedText inspectText;
		const RoomHoverState hoverState = resolveRoomHoverState(
			engine, scene.state, scene.sceneObjects, scene.state.roomNpcs, scene.sceneRegions, probePoint);
		const bool hasInteraction = script->hasObjectInteraction(*hoveredObject);
		const bool hasInspectText = script->resolveObjectInspectText(*hoveredObject, inspectText);
		debugC(1, kDebugRoom,
			"Harvester: startup probe hotspot room='%s' object='%s' point=(%d,%d) label='%s' prompt='%s' cursor_sequence=%d action_tag='%s' interaction=%d next_room='%s' inspect=%d",
			scene.state.roomName.c_str(), hoveredObject->objectName.c_str(), probePoint.x, probePoint.y,
			objectLabel.c_str(), hoverState.promptText.c_str(), hoverState.cursorSequence,
			hoveredObject->actionTag.c_str(), hasInteraction,
			"", hasInspectText);

		if (Player::supportsMovementBand(scene.state)) {
			Common::Point floorProbe;
			bool foundFloorProbe = false;
			for (int y = scene.state.roomMaxZScreenY; y <= scene.state.roomMinZScreenY && !foundFloorProbe; y += 12) {
				for (int x = 48; x < 592; x += 16) {
					const RoomHoverState candidateHover = resolveRoomHoverState(
						engine, scene.state, scene.sceneObjects, scene.state.roomNpcs, scene.sceneRegions,
						Common::Point(x, y));
					if (candidateHover.cursorSequence == kCursorSequenceWalk) {
						floorProbe = Common::Point(x, y);
						foundFloorProbe = true;
						break;
					}
				}
			}

			const RoomHoverState floorHover = foundFloorProbe
				? resolveRoomHoverState(engine, scene.state, scene.sceneObjects, scene.state.roomNpcs,
					scene.sceneRegions, floorProbe)
				: RoomHoverState();
			debugC(1, kDebugRoom,
				"Harvester: startup probe floor room='%s' point=(%d,%d) found=%d cursor_sequence=%d prompt='%s'",
				scene.state.roomName.c_str(), floorProbe.x, floorProbe.y, foundFloorProbe,
				floorHover.cursorSequence, floorHover.promptText.c_str());

			Entity *player = entityManager->findSceneEntityByName(kPlayerActorEntityName);
			if (player && foundFloorProbe && floorHover.cursorSequence == kCursorSequenceWalk) {
				RoomPlayerState probePlayer;
				probePlayer.entity = player;
				probePlayer.centerX = scene.state.playerSpawnX;
				probePlayer.bottomY = scene.state.playerSpawnY;
				probePlayer.z = (float)scene.state.playerSpawnZ;
				probePlayer.facing = scene.state.playerFacing;
				Player::setMoveTargetFromScreenPoint(scene.state, probePlayer, floorProbe.x, floorProbe.y);
				for (int i = 0; i < 32 && probePlayer.hasMoveTarget; ++i)
					(void)Player::stepMoveTarget(
						engine, scene.state, scene.sceneObjects, scene.sceneAnimations, probePlayer);

				const Common::Rect movedRect = player->getScreenRect();
				debugC(1, kDebugPlayer,
					"Harvester: startup probe movement room='%s' target=(%d,%d) final_rect=(%d,%d)-(%d,%d) z=%.2f",
					scene.state.roomName.c_str(), floorProbe.x, floorProbe.y,
					movedRect.left, movedRect.top, movedRect.right, movedRect.bottom, (double)probePlayer.z);

				player->setCurrentFrame(Player::resolveFacingFrame(scene.state.playerFacing));
				(void)applyRoomActorPlacement(scene.state, *player,
					scene.state.playerSpawnX, scene.state.playerSpawnY, (float)scene.state.playerSpawnZ);
			}
		}
		return;
	}

	debugC(1, kDebugRoom, "Harvester: startup probe found no interactive hotspot in room '%s'",
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

static bool loadQuickTipsScene(HarvesterEngine &engine, RoomSceneResources &scene) {
	RoomSetupState state;
	// The original startup path enters the START room first and overlays quick tips on top of it.
	if (!engine.getScript()->resolveRoomSetupState("START", state, *engine.getResources()))
		return false;

	return loadRoomSceneResources(state, *engine.getResources(), scene);
}

static void renderQuickTipsScreen(HarvesterEngine &engine, const RoomSceneResources &scene,
		const Common::Point &mousePos, const Common::String &tipText) {
	Graphics::Screen *screen = engine.getScreen();
	const Art *art = engine.getArt();
	const Graphics::Font *font = FontMan.getFontByUsage(Graphics::FontManager::kGUIFont);
	Script *script = engine.getScript();
	if (!screen || !art || !font || !script)
		return;

	drawRoomScene(engine, *screen, scene, scene.targetPaletteBrightness);
	blitBitmap(*screen, art->getTipsBitmap(), kQuickTipsOverlayX, kQuickTipsOverlayY);

	drawWrappedShadowedText(*screen, *font, tipText, kQuickTipTextX, kQuickTipTextY, kQuickTipTextWidth, kTextColorNormal);
	const Common::Rect exitRect = quickTipsExitRect();
	const Common::Rect nextRect = quickTipsNextRect();
	const Common::Rect toggleRect = quickTipsToggleRect();
	const Common::String toggleLabel = script->resolveTextValue(
		script->isQuickTipsEnabled() ? "Show_Tips_ON" : "Show_Tips_OFF");
	drawShadowedString(*screen, *font, "Exit", exitRect.left, exitRect.top, exitRect.width(),
		kQuickTipActionColor);
	drawShadowedString(*screen, *font, "Next", nextRect.left, nextRect.top, nextRect.width(),
		kQuickTipActionColor);
	drawShadowedString(*screen, *font, toggleLabel,
		toggleRect.left, toggleRect.top, toggleRect.width(), kQuickTipActionColor);

	if (engine.getRuntimeEntities())
		engine.getRuntimeEntities()->drawCursor(*screen);
	screen->makeAllDirty();
	screen->update();
}

Flow::Flow(HarvesterEngine &engine)
	: _engine(engine), _mousePos(320, 200), _dialogue(engine, _mousePos), _inventory(engine),
	  _menu(engine, _mousePos, _menuItems), _room(engine, _mousePos, _inventory) {
}

bool Flow::load() {
	return loadQuickTips() && loadMenuItems();
}

bool Flow::buildDialogueSaveStateBlob(Common::Array<byte> &blob, uint32 saveVersion) {
	Common::MemoryWriteStreamDynamic stream(DisposeAfterUse::NO);
	Common::Serializer serializer(nullptr, &stream);
	serializer.setVersion(saveVersion);
	_dialogue.syncRuntimeSaveState(serializer);
	if (serializer.err())
		return false;

	blob.resize(stream.size());
	if (!blob.empty())
		memcpy(blob.data(), stream.getData(), blob.size());
	return true;
}

bool Flow::loadDialogueSaveStateBlob(const Common::Array<byte> &blob, uint32 saveVersion) {
	if (blob.empty())
		return false;

	Common::MemoryReadStream stream(blob.data(), blob.size());
	Common::Serializer serializer(&stream, nullptr);
	serializer.setVersion(saveVersion);
	_dialogue.syncRuntimeSaveState(serializer);
	return !serializer.err();
}

Common::Error Flow::run() {
	if (!ensureCursorEntity())
		return Common::kReadingFailed;

	clearPendingMainMenuReturn();
	clearPendingGameOverReturn();
	clearPendingNewGameRestart();
	clearPendingCloseupParentRestart();
	resetRoomNpcDialogueState();
	Common::Error error = Common::kNoError;
	if (_engine.hasPendingLoadedSaveRoomState()) {
		Common::String initialTarget = _engine.getPendingLoadedSaveRoomState().entranceName;
		if (initialTarget.empty())
			initialTarget = _engine.getPendingLoadedSaveRoomState().roomName;
		error = runRoomLoop(initialTarget);
	} else {
		_engine.clearCurrentSaveRoomState();
		_engine.clearPendingLoadedDialogueStateBlob();
		_engine.getScript()->resetRuntimeState();
		error = runQuickTips();
		if (error.getCode() != Common::kNoError)
			return error;

		clearPendingMainMenuReturn();
		_engine.getScript()->resetRuntimeState();
		error = runRoomLoop("START");
	}
	if (error.getCode() != Common::kNoError)
		return error;
	if (!takePendingMainMenuReturn())
		return Common::kNoError;

	_engine.stopMusic();
	_engine.stopSound();
	return runMainMenuStub();
}

bool Flow::loadQuickTips() {
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
			Common::String trimmed = trimAsciiLine(currentLine);
			if (!trimmed.empty())
				_quickTips.push_back(Common::move(trimmed));
			currentLine.clear();
			continue;
		}

		currentLine += ch;
	}

	Common::String trimmed = trimAsciiLine(currentLine);
	if (!trimmed.empty())
		_quickTips.push_back(Common::move(trimmed));

	debugC(1, kDebugGeneral, "Harvester: loaded %u quick tips from '%s'", (uint)_quickTips.size(), kQuickTipsPath);
	return true;
}

bool Flow::loadMenuItems() {
	_menuItems.clear();

	Common::Array<byte> data;
	if (!_engine.getResources()->loadFile(kMenuPath, data)) {
		if (_engine.isDemo()) {
			for (const char *item : kDemoMenuItems)
				_menuItems.push_back(item);
			debugC(1, kDebugGeneral,
				"Harvester: using %u built-in DOS demo menu items",
				(uint)_menuItems.size());
			return true;
		}

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
			_menuItems.push_back(Common::move(value));
	}

	debugC(1, kDebugGeneral, "Harvester: loaded %u startup menu items from '%s'", (uint)_menuItems.size(), kMenuPath);
	return true;
}

Common::Error Flow::runQuickTips() {
	if (!_engine.getScript()->isQuickTipsEnabled() || _quickTips.empty())
		return Common::kNoError;

	Common::Error transitionError = beginRoomSetupTransition();
	if (transitionError.getCode() != Common::kNoError)
		return transitionError;

	RoomSceneResources scene;
	if (!loadQuickTipsScene(_engine, scene))
		return Common::kReadingFailed;

	Graphics::Screen *screen = _engine.getScreen();
	if (screen) {
		if (!populateRoomSceneEntities(scene.state, scene.sceneObjects, scene.sceneAnimations))
			return Common::kReadingFailed;

		transitionError = waitForRoomSetupTransitionHold();
		if (transitionError.getCode() != Common::kNoError)
			return transitionError;

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

	debugC(1, kDebugGeneral,
		"Harvester: quick tips labels exit='Exit' next='Next' toggle='%s'",
		_engine.getScript()->resolveTextValue(
			_engine.getScript()->isQuickTipsEnabled() ? "Show_Tips_ON" : "Show_Tips_OFF").c_str());

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
					_engine.getScript()->setQuickTipsEnabled(!_engine.getScript()->isQuickTipsEnabled());
					needsRedraw = true;
				}
				break;
			case Common::EVENT_KEYDOWN:
				if (event.kbd.keycode == Common::KEYCODE_ESCAPE)
					return Common::kNoError;
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

Common::Error Flow::runDemoEnding() {
	Graphics::Screen *screen = _engine.getScreen();
	ResourceManager *resources = _engine.getResources();
	EntityManager *entityManager = _engine.getRuntimeEntities();
	if (!screen || !resources)
		return Common::kReadingFailed;

	if (entityManager)
		entityManager->hideCursor();
	_engine.stopSound();
	if (!_engine.playMusic(kDemoEndingMusicPath))
		return Common::kReadingFailed;

	auto fadePalette = [&](const byte *palette, float from, float to) -> Common::Error {
		const float step = from < to ? kPaletteFadeStep : -kPaletteFadeStep;
		for (float brightness = from; step > 0.0f ? brightness < to : brightness > to;
				brightness += step) {
			setScaledPalette(*screen, palette, brightness);
			screen->makeAllDirty();
			screen->update();

			const uint32 nextTick = g_system->getMillis() + kPaletteFadeTickMs;
			while ((int32)(nextTick - g_system->getMillis()) > 0) {
				Common::Error eventError = Common::kNoError;
				if (pumpTransitionEvents(eventError))
					return eventError;
				g_system->delayMillis(1);
			}
		}

		setScaledPalette(*screen, palette, to);
		screen->makeAllDirty();
		screen->update();
		return Common::kNoError;
	};

	for (uint i = 0; i < ARRAYSIZE(kDemoEndingScreenNumbers) && !_engine.shouldQuit(); ++i) {
		const int screenNumber = kDemoEndingScreenNumbers[i];
		const Common::String bitmapPath = Common::String::format(
			"1:/GRAPHIC/OTHER/SCREEN%d.BM", screenNumber);
		const Common::String palettePath = Common::String::format(
			"1:/GRAPHIC/PAL/SCREEN%d.PAL", screenNumber);
		IndexedBitmap bitmap;
		byte palette[256 * 3];
		if (!loadBitmapResource(*resources, bitmapPath, bitmap) ||
				!loadPaletteResource(*resources, palettePath, palette)) {
			warning("Harvester: unable to load DOS demo ending screen %d", screenNumber);
			return Common::kReadingFailed;
		}

		setScaledPalette(*screen, palette, kPaletteBrightnessBlack);
		screen->fillRect(Common::Rect(0, 0, screen->w, screen->h), 0);
		blitBitmap(*screen, bitmap, 0, 0);
		screen->makeAllDirty();
		screen->update();

		Common::Error fadeError = fadePalette(
			palette, kPaletteBrightnessBlack, kPaletteBrightnessFull);
		if (fadeError.getCode() != Common::kNoError)
			return fadeError;

		const uint32 deadline = g_system->getMillis() + kDemoEndingScreenDurationMs;
		bool skipScreen = false;
		while (!skipScreen && !_engine.shouldQuit() &&
				(int32)(deadline - g_system->getMillis()) > 0) {
			Common::Event event;
			while (g_system->getEventManager()->pollEvent(event)) {
				Common::Error eventError = Common::kNoError;
				if (handleSystemEvent(event, eventError))
					return eventError;
				if (event.type == Common::EVENT_LBUTTONDOWN ||
						(event.type == Common::EVENT_KEYDOWN &&
						(event.kbd.keycode == Common::KEYCODE_SPACE ||
						event.kbd.keycode == Common::KEYCODE_ESCAPE ||
						event.kbd.keycode == Common::KEYCODE_RETURN))) {
					skipScreen = true;
				}
			}
			g_system->delayMillis(1);
		}

		if (i + 1 == ARRAYSIZE(kDemoEndingScreenNumbers))
			_engine.stopMusic();
		fadeError = fadePalette(
			palette, kPaletteBrightnessFull, kPaletteBrightnessBlack);
		if (fadeError.getCode() != Common::kNoError)
			return fadeError;
	}

	_engine.quitGame();
	return Common::kNoError;
}

Common::Error Flow::runMainMenuStub() {
	return _menu.runMainMenuStub(*this);
}

Common::Error Flow::runRoomMenuStub(const IndexedBitmap &backdrop, const byte *palette,
		float paletteBrightness, bool canSaveGame) {
	return _menu.runRoomMenuStub(backdrop, palette, paletteBrightness, *this, canSaveGame);
}

Common::Error Flow::runRoomNpcDialogue(const IndexedBitmap &backdrop, const byte *palette,
		float paletteBrightness, const NpcRecord &npc, const Common::String &usedItemName) {
	_queuedDialogueInteraction = InteractionResult();
	_hasQueuedDialogueInteraction = false;
	return _dialogue.runRoomNpcDialogue(backdrop, palette, paletteBrightness, npc, usedItemName, *this);
}

Common::Error Flow::runTownMapSelector(const Common::String &mapEntryName,
		Common::String &destinationEntranceName) {
	destinationEntranceName.clear();

	Script *script = _engine.getScript();
	ResourceManager *resources = _engine.getResources();
	Graphics::Screen *screen = _engine.getScreen();
	const Graphics::Font *font = FontMan.getFontByUsage(Graphics::FontManager::kGUIFont);
	Common::ScopedPtr<HarvesterCftFont> townMapFont;
	if (const CftFontResource *townMapFontResource = findStartupFontByName(_engine, "TEXTFONT")) {
		townMapFont.reset(new HarvesterCftFont(*townMapFontResource));
		if (townMapFont->isValid())
			font = townMapFont.get();
		else
			townMapFont.reset();
	}
	if (!script || !resources || !screen || !font)
		return Common::kReadingFailed;

	const MapEntranceRecord *mapEntrance = script->findMapEntranceRecord(mapEntryName);
	if (!mapEntrance) {
		warning("Harvester: unresolved town map entry '%s'", mapEntryName.c_str());
		return Common::kReadingFailed;
	}
	const float paletteBrightness = script->getFlagValue("DAY_FLAG")
		? 1.0f
		: kTownMapNightPaletteBrightness;

	byte palette[256 * 3];
	if (!loadPaletteResource(*resources, kTownMapPalettePath, palette))
		return Common::kReadingFailed;

	Common::Array<IndexedBitmap> panels;
	panels.resize(ARRAYSIZE(kTownMapBitmapPaths));
	for (uint i = 0; i < ARRAYSIZE(kTownMapBitmapPaths); ++i) {
		if (!loadBitmapResource(*resources, kTownMapBitmapPaths[i], panels[i]))
			return Common::kReadingFailed;
	}

	const Common::String previousMusicPath = _engine.getMusicPath();
	(void)_engine.playMusic(kTownMapMusicPath);

	if (!ensureCursorEntity())
		return Common::kReadingFailed;
	resetCursorAnimationSequence();

	if (_mousePos.x < 0 || _mousePos.y < 0 || _mousePos.x >= screen->w || _mousePos.y >= screen->h)
		_mousePos = Common::Point(screen->w / 2, screen->h / 2);
	if (EntityManager *entityManager = _engine.getRuntimeEntities())
		(void)entityManager->syncCursorEntityPosition(_mousePos);

	int currentPanel = clampTownMapPanelIndex(mapEntrance->initialPanelIndex);
	bool needsRedraw = true;
	Graphics::FrameLimiter limiter(g_system, 60);

	auto centerCursor = [&]() {
		_mousePos = Common::Point(screen->w / 2, screen->h / 2);
		if (EntityManager *entityManager = _engine.getRuntimeEntities())
			(void)entityManager->syncCursorEntityPosition(_mousePos);
	};
	auto restorePreviousMusic = [&]() {
		if (!destinationEntranceName.empty())
			return;
		if (!previousMusicPath.empty())
			(void)_engine.playMusic(previousMusicPath);
		else
			_engine.stopMusic();
	};

	while (!_engine.shouldQuit()) {
		const int edgePanel = resolveTownMapEdgePanel(currentPanel, _mousePos, screen->w, screen->h);
		if (edgePanel != currentPanel) {
			currentPanel = edgePanel;
			centerCursor();
			needsRedraw = true;
		}

		const MapLocationRecord *hoveredLocation =
			findTownMapLocationAt(script->getMapLocations(), currentPanel, _mousePos);
		if (needsRedraw) {
			setScaledPalette(*screen, palette, paletteBrightness);
			screen->fillRect(screen->getBounds(), 0);
			blitBitmap(*screen, panels[(uint)currentPanel], 0, 0);
			if (hoveredLocation) {
				drawShadowedString(*screen, *font, hoveredLocation->labelText,
					hoveredLocation->labelX, hoveredLocation->labelY, screen->w, kTownMapLabelColor);
			}
			if (EntityManager *entityManager = _engine.getRuntimeEntities())
				entityManager->drawCursor(*screen);
			screen->makeAllDirty();
			screen->update();
			needsRedraw = false;
		}

		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			Common::Error result = Common::kNoError;
			if (handleSystemEvent(event, result)) {
				restorePreviousMusic();
				return result;
			}

			switch (event.type) {
			case Common::EVENT_MOUSEMOVE:
				needsRedraw = true;
				break;
			case Common::EVENT_LBUTTONDOWN: {
				const MapLocationRecord *clickedLocation =
					findTownMapLocationAt(script->getMapLocations(), currentPanel, _mousePos);
				if (!clickedLocation)
					break;

				const EntranceRecord *destinationEntrance =
					script->findEntranceRecord(clickedLocation->destinationEntranceName);
				if (!destinationEntrance) {
					warning("Harvester: unresolved town map destination '%s' from '%s'",
						clickedLocation->destinationEntranceName.c_str(), mapEntryName.c_str());
					break;
				}

				destinationEntranceName = destinationEntrance->entranceName;
				debugC(1, kDebugGeneral,
					"Harvester: town map selection entry='%s' panel=%d label='%s' destination='%s'",
					mapEntryName.c_str(), currentPanel, clickedLocation->labelText.c_str(),
					destinationEntranceName.c_str());
				return Common::kNoError;
			}
			case Common::EVENT_KEYDOWN: {
				const int nextPanel = resolveTownMapKeyPanel(currentPanel, event.kbd.keycode);
				if (nextPanel != currentPanel) {
					currentPanel = nextPanel;
					centerCursor();
					needsRedraw = true;
				}
				break;
			}
			default:
				break;
			}
		}

		if (tickRuntimeEntities())
			needsRedraw = true;

		limiter.delayBeforeSwap();
		limiter.startFrame();
	}

	restorePreviousMusic();
	return Common::kNoError;
}

Common::Error Flow::resolveRoomTransitionTarget(const Common::String &targetName,
		Common::String &resolvedTargetName) {
	resolvedTargetName = targetName;

	Script *script = _engine.getScript();
	if (!script || targetName.empty())
		return Common::kNoError;

	if (script->findMapEntranceRecord(targetName))
		return runTownMapSelector(targetName, resolvedTargetName);

	return Common::kNoError;
}

void Flow::queueDialogueInteraction(const InteractionResult &interaction) {
	_queuedDialogueInteraction = interaction;
	_hasQueuedDialogueInteraction = true;
}

bool Flow::takeQueuedDialogueInteraction(InteractionResult &interaction) {
	if (!_hasQueuedDialogueInteraction)
		return false;

	interaction = Common::move(_queuedDialogueInteraction);
	_queuedDialogueInteraction = InteractionResult();
	_hasQueuedDialogueInteraction = false;
	return true;
}

bool Flow::queueDebugInteraction(const InteractionResult &interaction) {
	if (_hasQueuedDebugInteraction)
		return false;

	// Console-triggered commands are consumed by the room loop so they follow the same
	// transition, dialogue, and continuation handling as in-world interactions.
	_queuedDebugInteraction = interaction;
	_hasQueuedDebugInteraction = true;
	return true;
}

bool Flow::takeQueuedDebugInteraction(InteractionResult &interaction) {
	if (!_hasQueuedDebugInteraction)
		return false;

	interaction = Common::move(_queuedDebugInteraction);
	_queuedDebugInteraction = InteractionResult();
	_hasQueuedDebugInteraction = false;
	return true;
}

void Flow::prepareForNewGame() {
	clearPendingMainMenuReturn();
	clearPendingGameOverReturn();
	clearPendingNewGameRestart();
	_pendingDebugRoomName.clear();
	_queuedDebugInteraction = InteractionResult();
	_hasQueuedDebugInteraction = false;
	_engine.clearPendingLoadedSaveRoomState();
	_engine.clearPendingLoadedDialogueStateBlob();
	_engine.clearCurrentSaveRoomState();
	if (_engine.getScript())
		_engine.getScript()->resetRuntimeState();
	resetRoomNpcDialogueState();
}

void Flow::requestNewGameRestart() {
	_pendingNewGameRestart = true;
}

bool Flow::hasPendingNewGameRestart() const {
	return _pendingNewGameRestart;
}

bool Flow::takePendingNewGameRestart() {
	const bool requested = _pendingNewGameRestart;
	_pendingNewGameRestart = false;
	return requested;
}

void Flow::clearPendingNewGameRestart() {
	_pendingNewGameRestart = false;
}

void Flow::requestGameOverReturn() {
	requestMainMenuReturn();
	_pendingGameOverReturn = true;
}

bool Flow::hasPendingGameOverReturn() const {
	return _pendingGameOverReturn;
}

bool Flow::takePendingGameOverReturn() {
	const bool requested = _pendingGameOverReturn;
	_pendingGameOverReturn = false;
	return requested;
}

void Flow::clearPendingGameOverReturn() {
	_pendingGameOverReturn = false;
}

void Flow::requestMainMenuReturn() {
	_pendingMainMenuReturn = true;
	_engine.clearCurrentSaveRoomState();
}

bool Flow::hasPendingMainMenuReturn() const {
	return _pendingMainMenuReturn;
}

bool Flow::takePendingMainMenuReturn() {
	const bool requested = _pendingMainMenuReturn;
	_pendingMainMenuReturn = false;
	return requested;
}

void Flow::clearPendingMainMenuReturn() {
	_pendingMainMenuReturn = false;
}

void Flow::requestCloseupParentRestart() {
	_pendingCloseupParentRestart = true;
}

bool Flow::hasPendingCloseupParentRestart() const {
	return _pendingCloseupParentRestart;
}

bool Flow::takePendingCloseupParentRestart() {
	const bool requested = _pendingCloseupParentRestart;
	_pendingCloseupParentRestart = false;
	return requested;
}

void Flow::clearPendingCloseupParentRestart() {
	_pendingCloseupParentRestart = false;
}

bool Flow::requestDebugRoomChange(const Common::String &roomName) {
	if (roomName.empty())
		return false;

	_pendingDebugRoomName = roomName;
	return true;
}

Common::String Flow::takePendingDebugRoomChange() {
	const Common::String roomName = _pendingDebugRoomName;
	_pendingDebugRoomName.clear();
	return roomName;
}

Common::Error Flow::runRoomLoop(const Common::String &entranceName) {
	const bool nestedRoomLoop = _roomLoopDepth != 0;
	Common::String targetName = entranceName;
	bool targetIsRoomName = false;

	++_roomLoopDepth;
	while (true) {
		const Common::Error error = _room.runRoomLoop(*this, targetName, targetIsRoomName);
		if (error.getCode() != Common::kNoError) {
			--_roomLoopDepth;
			return error;
		}
		if (!hasPendingDebugRoomChange()) {
			--_roomLoopDepth;
			return Common::kNoError;
		}
		if (nestedRoomLoop) {
			--_roomLoopDepth;
			return Common::kNoError;
		}

		targetName = takePendingDebugRoomChange();
		targetIsRoomName = true;
	}
}

bool Flow::ensureCursorEntity() {
	EntityManager *entityManager = _engine.getRuntimeEntities();
	if (!entityManager)
		return false;
	if (entityManager->getCursorEntity())
		return true;

	return entityManager->spawnCursorEntity(_mousePos) != nullptr;
}

bool Flow::populateRoomSceneEntities(RoomSetupState &state,
		const Common::Array<ObjectRecord> &drawableObjects,
		const Common::Array<AnimRecord> &drawableAnimations) {
	EntityManager *entityManager = _engine.getRuntimeEntities();
	if (!entityManager)
		return false;

	Entity *preservedPlayer = entityManager->detachSceneEntityByName(kPlayerActorEntityName);
	entityManager->clearSceneEntities(true);
	for (const RegionRecord &region : state.roomRegions) {
		const Common::Rect regionBounds = getRegionBounds(region);
		if (regionBounds.isEmpty())
			continue;

		Entity *entity = entityManager->spawnSceneHotspotEntity(
			region.regionName, regionBounds, (float)region.minZ);
		if (!entity) {
			debugC(1, kDebugRoom,
				"Harvester: scene region skipped room='%s' region='%s' bounds=(%d,%d)-(%d,%d) z=[%d,%d]",
				state.roomName.c_str(), region.regionName.c_str(),
				regionBounds.left, regionBounds.top, regionBounds.right, regionBounds.bottom,
				region.minZ, region.maxZ);
			continue;
		}

		entity->setClassId(kRuntimeEntityClassRectHotspot19);
		entity->setAnchorMode(kRuntimeEntityAnchorTopLeft);
		entity->setZExtent((float)MAX(0, region.maxZ - region.minZ));
		if (!region.cursorEnabled)
			entity->setHitTestMode(kRuntimeEntityHitTestNone);
		debugC(1, kDebugRoom,
			"Harvester: scene region spawned room='%s' region='%s' class=0x%x bounds=(%d,%d)-(%d,%d) z=[%d,%d] facing=%d cursor=%d action='%s'",
			state.roomName.c_str(), region.regionName.c_str(), entity->getClassId(),
			regionBounds.left, regionBounds.top, regionBounds.right, regionBounds.bottom,
			region.minZ, region.maxZ, region.desiredFacing, region.cursorEnabled, region.actionTag.c_str());
	}
	for (const TimerRecord &timer : state.roomTimers) {
		Entity *existingTimer = entityManager->findSceneEntityByName(timer.timerName);
		if (existingTimer && existingTimer->getClassId() == kRuntimeEntityClassTimer) {
			debugC(1, kDebugRoom,
				"Harvester: scene timer reused room='%s' timer='%s' current=%d initial=%d enabled=%d loop=%d global=%d",
				state.roomName.c_str(), timer.timerName.c_str(), existingTimer->getTimerCurrentValue(),
				existingTimer->getTimerInitialValue(), existingTimer->isTimerEnabled(),
				existingTimer->isTimerLooping(), existingTimer->isTimerGlobal());
			continue;
		}

		if (!entityManager->spawnSceneTimerEntity(timer.timerName,
				timer.initialValue, timer.currentValue, timer.enabled, timer.looping, timer.global)) {
			debug(1, "Harvester: unable to spawn room timer entity '%s'",
				timer.timerName.c_str());
			continue;
		}

		debugC(1, kDebugRoom,
			"Harvester: scene timer spawned room='%s' timer='%s' current=%d initial=%d enabled=%d loop=%d global=%d",
			state.roomName.c_str(), timer.timerName.c_str(), timer.currentValue, timer.initialValue,
			timer.enabled, timer.looping, timer.global);
	}
	for (const ObjectRecord &object : drawableObjects) {
		Entity *entity = nullptr;
		const Common::String spritePath = resolveSceneObjectSpritePath(object);
		const Common::Rect hotspotBounds = getRoomObjectHotspotBounds(object);
		if (!spritePath.empty() && spritePath.hasSuffixIgnoreCase(".BM")) {
			entity = entityManager->spawnSceneBitmapEntity(object.objectName, spritePath,
				Common::Point(object.currentX, object.currentY), (float)object.currentZ);
		} else {
			if (!hotspotBounds.isEmpty())
				entity = entityManager->spawnSceneHotspotEntity(object.objectName, hotspotBounds, (float)object.currentZ);
		}

		if (!entity) {
			debugC(1, kDebugRoom,
				"Harvester: scene entity skipped room='%s' object='%s' resolved='%s' bounds=(%d,%d)-(%d,%d) reason='%s'",
				state.roomName.c_str(), object.objectName.c_str(), spritePath.c_str(),
				hotspotBounds.left, hotspotBounds.top, hotspotBounds.right, hotspotBounds.bottom,
				spritePath.empty() || !spritePath.hasSuffixIgnoreCase(".BM")
					? "no_bitmap_and_no_hotspot_bounds"
					: "spawn_failed");
			debug(1, "Harvester: unable to spawn room object entity '%s' from '%s'",
				object.objectName.c_str(), spritePath.c_str());
			continue;
		}

		entity->setClassId(resolveSceneObjectClass(
			state, object, entity->hasFrames() ? entity : nullptr, _engine.getScript()));
		entity->setAnchorMode(kRuntimeEntityAnchorTopLeft);
		entity->setZExtent((float)object.zExtent);
		const Common::Rect entityRect = entity->getScreenRect();
		debugC(1, kDebugRoom,
			"Harvester: scene entity spawned room='%s' object='%s' type='%s' class=0x%x pos=(%d,%d,z=%.2f) rect=(%d,%d)-(%d,%d) sprite='%s' action='%s' operatable=%d ident='%s'",
			state.roomName.c_str(), object.objectName.c_str(),
			entity->hasFrames() ? "bitmap" : "hotspot", entity->getClassId(),
			entity->getX(), entity->getY(), (double)entity->getZ(),
			entityRect.left, entityRect.top, entityRect.right, entityRect.bottom,
			spritePath.c_str(), object.actionTag.c_str(), object.operatable, object.identTextKey.c_str());
	}
	for (const AnimRecord &anim : drawableAnimations) {
		const bool displayVisible = shouldDisplaySceneAnimation(anim);
		if (!entityManager->spawnSceneAnimationEntity(anim.animName, anim.resourcePath,
				Common::Point(anim.x, anim.y), (float)anim.z, anim.frameDelay, anim.active, displayVisible,
				anim.looping, anim.backward, anim.pingPong, anim.runtimeState)) {
			debug(1, "Harvester: unable to spawn room anim entity '%s' from '%s'",
				anim.animName.c_str(), anim.resourcePath.c_str());
		}
	}
	for (const NpcRecord &npc : state.roomNpcs) {
		Entity *entity = entityManager->spawnSceneActorEntity(npc.npcName,
			npc.modelPath, Common::Point(npc.posX, npc.posY), (float)npc.posZ, 0);
		if (!entity) {
			debug(1, "Harvester: unable to spawn room npc entity '%s' from '%s'",
				npc.npcName.c_str(), npc.modelPath.c_str());
			continue;
		}

		entity->setClassId(kRuntimeEntityClassNpc);
		entity->setZExtent(kNativeNpcMonsterZExtent);
		const bool isCorpse = !npc.active &&
			npc.runtimeSpawned &&
			npc.deathDamageType != 0 &&
			npc.runtimeState >= 0;
		entity->setHitTestMode(isCorpse ? kRuntimeEntityHitTestNone : kRuntimeEntityHitTestOpaquePixels);
		if (isCorpse) {
			const int corpseFrame = MIN(entity->getLastFrame(), npc.runtimeState);
			entity->setAnimationFrameRange(corpseFrame, corpseFrame, false);
			entity->setAnimationRate(0);
			entity->setCurrentFrame(corpseFrame);
			entity->setAnimationEnabled(false);
		} else {
			// Native spawn_npc_entity_from_record seeds the passive room-NPC loop as frames 0..0x3b.
			entity->setAnimationFrameRange(0, MIN(entity->getLastFrame(), kRoomNpcAmbientLastFrame), true);
			if (npc.frameDelay > 0)
				entity->setAnimationRate(npc.frameDelay);
		}
		// Native room NPCs come from spawn_abm_entity_base, which leaves the depth-scale flag cleared.
		if (!applyRoomActorPlacementInternal(state, *entity,
				npc.posX, npc.posY, (float)npc.posZ, nullptr, false)) {
			debug(1, "Harvester: unable to apply room npc placement for '%s'",
				npc.npcName.c_str());
		}
		entityManager->reinsertSceneEntity(entity);
		debugC(1, kDebugRoom,
			"Harvester: scene npc spawned room='%s' npc='%s' class=0x%x pos=(%d,%d,z=%.2f) frame_delay=%d model='%s' active=%d visible=%d",
			state.roomName.c_str(), npc.npcName.c_str(), entity->getClassId(),
			entity->getX(), entity->getY(), (double)entity->getZ(),
			npc.frameDelay, npc.modelPath.c_str(), npc.active, npc.visible);
	}
	for (const MonsterRecord &monster : state.roomMonsters) {
		Entity *entity = entityManager->spawnSceneActorEntity(monster.monsterName,
			monster.modelPath, Common::Point(monster.posX, monster.posY), (float)monster.posZ,
			Monster::resolveFacingFrame(monster.facing));
		if (!entity) {
			debug(1, "Harvester: unable to spawn room monster entity '%s' from '%s'",
				monster.monsterName.c_str(), monster.modelPath.c_str());
			continue;
		}

		entity->setClassId(kRuntimeEntityClassMonster);
		entity->setZExtent(kNativeNpcMonsterZExtent);
		entity->setHitTestMode(kRuntimeEntityHitTestNone);
		entity->setVisible(monster.visible);
		// Native restores a saved monster corpse frame before set_entity_screen_position().
		Monster::applyAnimation(*entity, monster);
		if (!applyRoomActorPlacement(state, *entity, monster.posX, monster.posY, (float)monster.posZ)) {
			debug(1, "Harvester: unable to apply room monster placement for '%s'",
				monster.monsterName.c_str());
		}
		entityManager->reinsertSceneEntity(entity);
		debugC(1, kDebugRoom,
			"Harvester: scene monster spawned room='%s' monster='%s' class=0x%x pos=(%d,%d,z=%.2f) facing=%d hp=%d/%d damage=%d engage=%d damage_type='%s' model='%s' active=%d visible=%d",
			state.roomName.c_str(), monster.monsterName.c_str(), entity->getClassId(),
			entity->getX(), entity->getY(), (double)entity->getZ(), monster.facing,
			monster.currentHitPoints, monster.initialHitPoints,
			monster.damageAmount, monster.engageDistance, Player::describeCombatDamageType(monster.damageType),
			monster.modelPath.c_str(), monster.active, monster.visible);
	}
	if (state.hasEntrance) {
		const int playerFrame = Player::resolveFacingFrame(state.playerFacing);
		Script *script = _engine.getScript();
		const int playerCombatLoadout = script ? script->getPlayerCombatLoadout() : 0;
		const Common::String playerResourcePath =
			Player::resolveCombatLoadoutResourcePath(playerCombatLoadout);
		const bool reusedPlayer = preservedPlayer != nullptr;
		Entity *player = preservedPlayer;
		if (!reusedPlayer) {
			player = entityManager->spawnSceneActorEntity(kPlayerActorEntityName,
				playerResourcePath, Common::Point(state.playerSpawnX, state.playerSpawnY),
				(float)state.playerSpawnZ, playerFrame);
		}
		if (!player) {
			debug(1, "Harvester: unable to spawn startup player actor from '%s'", playerResourcePath.c_str());
		} else {
			player->setClassId(kRuntimeEntityClassPlayer);
			player->setAnchorMode(kRuntimeEntityAnchorTopLeft);
			player->setHitTestMode(kRuntimeEntityHitTestOpaquePixels);
			player->setVisible(true);
			player->setAnimationRate(0);
			player->setAnimationFrameRange(playerFrame, playerFrame, false);
			player->setCurrentFrame(playerFrame);
			if (!applyStartupActorPlacement(state, *player)) {
				debug(1, "Harvester: unable to apply startup player placement for entrance '%s'",
					state.entranceName.c_str());
			}
			if (reusedPlayer) {
				RoomPlayerState playerState;
				playerState.entity = player;
				playerState.centerX = state.playerSpawnX;
				playerState.bottomY = state.playerSpawnY;
				playerState.z = (float)state.playerSpawnZ;
				playerState.facing = state.playerFacing;
				playerState.combatLoadout = -1;
				(void)Player::syncCombatLoadoutVisual(_engine, state, playerState, playerCombatLoadout);
			}
			RoomPlayerState playerSpawnState;
			playerSpawnState.entity = player;
			playerSpawnState.centerX = state.playerSpawnX;
			playerSpawnState.bottomY = state.playerSpawnY;
			playerSpawnState.z = (float)state.playerSpawnZ;
			playerSpawnState.facing = state.playerFacing;
			if (Player::resolveBlockedStartupSpawn(
					_engine, state, drawableObjects, drawableAnimations, playerSpawnState)) {
				state.playerSpawnX = playerSpawnState.centerX;
				state.playerSpawnY = playerSpawnState.bottomY;
				state.playerSpawnZ = (int)playerSpawnState.z;
			}
			if (reusedPlayer)
				entityManager->adoptSceneEntity(player);
			else
				entityManager->reinsertSceneEntity(player);
			debugC(1, kDebugPlayer,
				"Harvester: %s startup player actor '%s' entrance='%s' pos=(%d,%d,%d) facing=%d frame=%d",
				reusedPlayer ? "reused" : "spawned", playerResourcePath.c_str(), state.entranceName.c_str(),
				state.playerSpawnX, state.playerSpawnY, state.playerSpawnZ,
				state.playerFacing, playerFrame);
		}
		preservedPlayer = nullptr;
	} else if (preservedPlayer) {
		delete preservedPlayer;
	}

	return true;
}

Common::Error Flow::beginRoomSetupTransition() {
	Graphics::Screen *screen = _engine.getScreen();
	if (screen) {
		byte displayPalette[256 * 3];
		screen->getPalette(displayPalette);
		bool hasVisiblePalette = false;
		for (byte value : displayPalette) {
			if (value != 0) {
				hasVisiblePalette = true;
				break;
			}
		}

		if (hasVisiblePalette) {
			for (float brightness = 1.0f - kPaletteFadeStep;
					brightness > kPaletteBrightnessBlack; brightness -= kPaletteFadeStep) {
				setScaledDisplayPalette(*screen, displayPalette, brightness);
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

			setScaledDisplayPalette(*screen, displayPalette, kPaletteBrightnessBlack);
			screen->makeAllDirty();
			screen->update();
		}
	}

	if (_engine.getRuntimeEntities())
		_engine.getRuntimeEntities()->hideCursor();

	const Art *art = _engine.getArt();
	if (art && screen)
		art->drawWaitFrame(*screen);
	_roomSetupTransitionShownTick = g_system ? g_system->getMillis() : 0;

	Common::Error result = Common::kNoError;
	if (pumpTransitionEvents(result))
		return result;

	return Common::kNoError;
}

Common::Error Flow::waitForRoomSetupTransitionHold() {
	if (_roomSetupTransitionShownTick == 0 || !g_system)
		return Common::kNoError;

	const uint32 deadline = _roomSetupTransitionShownTick + kTransitionWaitFrameMinMs;
	while ((int32)(deadline - g_system->getMillis()) > 0) {
		Common::Error result = Common::kNoError;
		if (pumpTransitionEvents(result))
			return result;
		g_system->delayMillis(1);
	}

	_roomSetupTransitionShownTick = 0;
	return Common::kNoError;
}

Common::Error Flow::fadeInRoomScene(const byte *palette, float targetBrightness) {
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

bool Flow::pumpTransitionEvents(Common::Error &result) {
	Common::Event event;
	while (g_system->getEventManager()->pollEvent(event)) {
		if (handleSystemEvent(event, result))
			return true;
	}

	return false;
}

void Flow::executeStartupAudioCommands(const Common::Array<AudioCommand> &commands) {
	for (const AudioCommand &command : commands)
		(void)_engine.executeAudioCommand(command);
}

void Flow::resetRoomNpcDialogueState() {
	_dialogue.resetRoomNpcDialogueState();
}

void Flow::resetCursorAnimationSequence() {
	if (!ensureCursorEntity())
		return;

	Entity *cursor = _engine.getRuntimeEntities()->getCursorEntity();
	if (!cursor)
		return;

	cursor->setVisible(true);
	cursor->setAnimationSequence(kCursorSequence7);
	(void)_engine.getRuntimeEntities()->syncCursorEntityPosition(_mousePos);
}

bool Flow::tickRuntimeEntities() {
	EntityManager *entityManager = _engine.getRuntimeEntities();
	if (!entityManager)
		return false;
	return entityManager->tickSceneEntities() || entityManager->syncCursorEntityPosition(_mousePos);
}

bool Flow::handleSystemEvent(const Common::Event &event, Common::Error &result) {
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

} // End of namespace Harvester
