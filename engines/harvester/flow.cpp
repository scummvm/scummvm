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
#include <memory>

#include "harvester/flow.h"

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/endian.h"
#include "common/events.h"
#include "common/formats/ini-file.h"
#include "common/memstream.h"
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
static const float kTownMapNightPaletteBrightness = 0.6f;
// Native keeps WAIT_BM visible while room construction runs; fast local loads need a short floor.
static const uint32 kTransitionWaitFrameMinMs = 120;
static const int kRoomNpcAmbientLastFrame = 0x3b;
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

static const int kCursorSequenceWalk = 0;
static const int kCursorSequenceExamine = 1;
static const int kCursorSequenceTalk = 2;
static const int kCursorSequenceOperate = 4;
static const int kCursorSequencePickup = 5;
static const int kCursorSequenceTransition = 6;
static const int kCursorSequenceNeutral = 7;
} // End of anonymous namespace

static const CftFontResource *findStartupFontByName(const HarvesterEngine &engine, const char *fontName) {
	const Text *startupText = engine.getStartupText();
	if (!startupText || !fontName)
		return nullptr;

	for (const CftFontResource &font : startupText->getFonts()) {
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

static Common::String resolveRoomDebugObjectLabel(HarvesterEngine &engine, const StartupObjectRecord &object) {
	if (Script *startupScript = engine.getStartupScript()) {
		const Common::String resolvedLabel = startupScript->resolveObjectLabel(object);
		if (!resolvedLabel.empty())
			return resolvedLabel;
	}

	return object.objectName;
}

static Common::String resolveRoomDebugNpcLabel(const StartupNpcRecord &npc) {
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
		const StartupRoomSceneResources &scene) {
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

	for (const StartupRegionRecord &region : scene.sceneRegions)
		drawRoomDebugLabel(screen, *font, region.regionName, region.left, region.top, white, darkGray);

	RuntimeEntityManager *runtimeEntities = engine.getRuntimeEntities();
	for (const StartupObjectRecord &object : scene.sceneObjects) {
		const RuntimeEntity *entity = runtimeEntities
			? runtimeEntities->findSceneEntityByName(object.objectName)
			: nullptr;
		if (entity && entity->getClassId() == kRuntimeEntityClassBackground)
			continue;

		drawRoomDebugLabel(screen, *font, resolveRoomDebugObjectLabel(engine, object),
			object.currentX, object.currentY, white, black);
	}

	for (const StartupNpcRecord &npc : scene.state.roomNpcs) {
		const RuntimeEntity *entity = runtimeEntities
			? runtimeEntities->findSceneEntityByName(npc.npcName)
			: nullptr;
		if (!entity || entity->getClassId() != kRuntimeEntityClassNpc || !entity->isVisible())
			continue;

		const Common::Rect npcRect = entity->getScreenRect();
		drawRoomDebugLabel(screen, *font, resolveRoomDebugNpcLabel(npc),
			npcRect.left, npcRect.top, white, red);
	}
}

static void drawCombatDebugOverlay(HarvesterEngine &engine, Graphics::Screen &screen,
		const StartupRoomSceneResources &scene) {
	if (!engine.isCombatDebugEnabled())
		return;

	const Graphics::Font *font = FontMan.getFontByUsage(Graphics::FontManager::kGUIFont);
	RuntimeEntityManager *runtimeEntities = engine.getRuntimeEntities();
	Script *startupScript = engine.getStartupScript();
	if (!font || !runtimeEntities)
		return;

	byte displayPalette[256 * 3];
	screen.getPalette(displayPalette);
	const byte black = findNearestPaletteColor(displayPalette, 0x00, 0x00, 0x00);
	const byte white = findNearestPaletteColor(displayPalette, 0xff, 0xff, 0xff);
	const byte red = findNearestPaletteColor(displayPalette, 0xff, 0x00, 0x00);

	const RuntimeEntity *playerEntity = runtimeEntities->findSceneEntityByName(kPlayerActorEntityName);
	if (startupScript && playerEntity &&
			playerEntity->getClassId() == kRuntimeEntityClassPlayer &&
			playerEntity->isVisible()) {
		const Common::Rect playerRect = playerEntity->getScreenRect();
		drawRoomDebugLabel(screen, *font,
			resolveCombatDebugLabel(playerEntity->getName(),
				startupScript->getPlayerCurrentHitPoints(), Script::kDefaultPlayerHitPoints),
			playerRect.left, playerRect.top, white, red);
	}

	for (const StartupMonsterRecord &monster : scene.state.roomMonsters) {
		const RuntimeEntity *entity = runtimeEntities->findSceneEntityByName(monster.monsterName);
		if (!entity || entity->getClassId() != kRuntimeEntityClassMonster || !entity->isVisible())
			continue;

		const Common::Rect monsterRect = entity->getScreenRect();
		drawRoomDebugLabel(screen, *font,
			resolveCombatDebugLabel(monster.monsterName,
				monster.currentHitPoints, monster.initialHitPoints),
			monsterRect.left, monsterRect.top, white, black);
	}
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

static bool isTownMapLocationHovered(const StartupMapLocationRecord &location, int currentPanel,
		const Common::Point &mousePos) {
	if (location.panelIndex != currentPanel)
		return false;

	const int cursorRight = mousePos.x + kTownMapCursorHitExtent;
	const int cursorBottom = mousePos.y + kTownMapCursorHitExtent;
	return mousePos.x <= location.maxX && cursorRight >= location.minX &&
		mousePos.y <= location.maxY && cursorBottom >= location.minY;
}

static const StartupMapLocationRecord *findTownMapLocationAt(
		const Common::Array<StartupMapLocationRecord> &locations, int currentPanel,
		const Common::Point &mousePos) {
	for (const StartupMapLocationRecord &location : locations) {
		if (isTownMapLocationHovered(location, currentPanel, mousePos))
			return &location;
	}

	return nullptr;
}

static void setRoomActorScreenPosition(RuntimeEntity &entity, int centerX, int bottomY, float z,
		int width, int height, int xOffset, int yOffset) {
	entity.setPosition(centerX - xOffset - width / 2, bottomY - height - yOffset, z);
}

static float computeRoomActorRenderZ(float z, const RuntimeEntity &entity) {
	// Native actor spawns lower the render-list depth anchor by half the z extent
	// before insertion. Without that adjustment, cemetery Karin lands one layer
	// too far back and gets occluded by the grave mask in CEM10.
	return z - floorf(MAX<float>(entity.getZExtent(), 0.0f) * 0.5f);
}

static bool applyRoomActorPlacementInternal(const StartupRoomSetupState &state, RuntimeEntity &entity,
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

bool applyRoomActorPlacement(const StartupRoomSetupState &state, RuntimeEntity &entity,
		int centerX, int bottomY, float z, const Common::String *entranceName) {
	return applyRoomActorPlacementInternal(state, entity, centerX, bottomY, z, entranceName, true);
}

static bool applyStartupActorPlacement(const StartupRoomSetupState &state, RuntimeEntity &entity) {
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

void logScenePaletteSummary(const char *label, const StartupRoomSceneResources &scene, float brightness) {
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
	const float gammaBrightness = g_engine ? g_engine->getStartupGammaBrightnessScale() : 1.0f;
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

Common::Rect getRoomObjectHotspotBounds(const StartupObjectRecord &object) {
	if (object.boundsX2 > object.currentX && object.boundsY2 > object.currentY)
		return Common::Rect(object.currentX, object.currentY, object.boundsX2 + 1, object.boundsY2 + 1);

	return Common::Rect();
}

static Common::Rect getRegionBounds(const StartupRegionRecord &region) {
	if (region.right > region.left && region.bottom > region.top)
		return Common::Rect(region.left, region.top, region.right + 1, region.bottom + 1);

	return Common::Rect();
}

Common::String resolveSceneObjectSpritePath(const StartupObjectRecord &object) {
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

static void logSceneObjectSelection(const char *decision, const char *source, const StartupObjectRecord &object,
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

static bool isBackgroundSceneObject(const StartupObjectRecord &object, const RuntimeEntity &entity) {
	return object.initialX == 0 && object.initialY == 0 &&
		entity.getBoundsWidth() == 640 && entity.getBoundsHeight() == 480;
}

static bool isInteractiveSceneHotspot(const StartupObjectRecord &object, Script *startupScript) {
	if (object.operatable || !object.actionTag.empty())
		return true;
	if (!startupScript)
		return false;
	if (startupScript->isPickupObject(object))
		return true;

	StartupResolvedText inspectText;
	return startupScript->resolveObjectInspectText(object, inspectText);
}

static int resolveSceneObjectClass(const StartupObjectRecord &object, const RuntimeEntity *entity,
		Script *startupScript) {
	if (entity)
		return isBackgroundSceneObject(object, *entity) ? kRuntimeEntityClassBackground : kRuntimeEntityClassObject;

	return isInteractiveSceneHotspot(object, startupScript)
		? kRuntimeEntityClassRectHotspot
		: kRuntimeEntityClassDisabledHotspot;
}

static bool shouldQueueSceneObject(const StartupObjectRecord &object) {
	if (object.visible)
		return true;

	// Hidden no-sprite hotspots still drive room actions in the authored script data.
	return resolveSceneObjectSpritePath(object).empty() &&
		!getRoomObjectHotspotBounds(object).isEmpty() &&
		(object.operatable || !object.actionTag.empty());
}

static void queueSceneObject(const char *source, const StartupObjectRecord &object,
		Common::Array<StartupObjectRecord> &sceneObjects) {
	if (!shouldQueueSceneObject(object)) {
		logSceneObjectSelection("skipped", source, object, object.visible ? "inactive" : "visible=0");
		return;
	}

	for (const StartupObjectRecord &sceneObject : sceneObjects) {
		if (sceneObject.currentOwnerOrRoom.equalsIgnoreCase(object.currentOwnerOrRoom) &&
			sceneObject.objectName.equalsIgnoreCase(object.objectName)) {
			logSceneObjectSelection("skipped", source, object, "duplicate owner/object");
			return;
		}
	}

	sceneObjects.push_back(object);
	logSceneObjectSelection("queued", source, object);
}

bool loadRoomSceneResources(const StartupRoomSetupState &state, ResourceManager &resources, StartupRoomSceneResources &scene) {
	scene = StartupRoomSceneResources();
	scene.state = state;
	scene.targetPaletteBrightness = state.paletteBrightness;
	if (!loadPaletteResource(resources, state.palettePath, scene.palette)) {
		return false;
	}

	Common::Array<StartupObjectRecord> sceneObjects;
	for (const StartupObjectRecord &object : state.roomObjects)
		queueSceneObject("room", object, sceneObjects);
	for (const StartupObjectRecord &object : state.activeObjects)
		queueSceneObject("active", object, sceneObjects);

	scene.sceneObjects = sceneObjects;
	for (const StartupAnimRecord &anim : state.roomAnimations) {
		if (anim.active || anim.visible)
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

void drawRoomScene(HarvesterEngine &engine, Graphics::Screen &screen, const StartupRoomSceneResources &scene,
		float brightness) {
	setScaledPalette(screen, scene.palette, brightness);
	screen.fillRect(screen.getBounds(), 0);
	if (engine.getRuntimeEntities())
		engine.getRuntimeEntities()->drawSceneEntities(screen);
	drawRoomDebugOverlay(engine, screen, scene);
	drawCombatDebugOverlay(engine, screen, scene);
}

const StartupObjectRecord *findSceneObjectByName(const Common::Array<StartupObjectRecord> &objects,
		const Common::String &objectName) {
	for (const StartupObjectRecord &object : objects) {
		if (object.objectName.equalsIgnoreCase(objectName))
			return &object;
	}

	return nullptr;
}

const StartupRegionRecord *findSceneRegionByName(const Common::Array<StartupRegionRecord> &regions,
		const Common::String &regionName) {
	for (const StartupRegionRecord &region : regions) {
		if (region.regionName.equalsIgnoreCase(regionName))
			return &region;
	}

	return nullptr;
}

static const StartupNpcRecord *findSceneNpcByName(const Common::Array<StartupNpcRecord> &npcs,
		const Common::String &npcName) {
	for (const StartupNpcRecord &npc : npcs) {
		if (npc.npcName.equalsIgnoreCase(npcName))
			return &npc;
	}

	return nullptr;
}

StartupObjectRecord *findSceneObjectByName(Common::Array<StartupObjectRecord> &objects,
		const Common::String &objectName) {
	for (StartupObjectRecord &object : objects) {
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

	const RuntimeEntity *topEntity = nullptr;
	int topDrawIndex = -1;
	for (const StartupObjectRecord &object : sceneObjects) {
		if (object.objectName.empty())
			continue;

		const RuntimeEntity *entity = runtimeEntities->findSceneEntityByName(object.objectName);
		if (!entity || !entity->hitTest(point))
			continue;
		if (entity->getClassId() == kRuntimeEntityClassBackground ||
			entity->getClassId() == kRuntimeEntityClassPlayer ||
			entity->getClassId() == kRuntimeEntityClassRectHotspot19) {
			continue;
		}
		const int drawIndex = runtimeEntities->findSceneEntityDrawIndexByName(entity->getName());
		if (drawIndex < 0)
			continue;
		if (!topEntity || drawIndex > topDrawIndex) {
			topEntity = entity;
			topDrawIndex = drawIndex;
		}
	}
	if (!topEntity)
		return nullptr;

	return findSceneObjectByName(sceneObjects, topEntity->getName());
}

static const StartupRegionRecord *findRoomRegionAtPoint(HarvesterEngine &engine,
		const Common::Array<StartupRegionRecord> &sceneRegions, const Common::Point &point) {
	RuntimeEntityManager *runtimeEntities = engine.getRuntimeEntities();
	if (!runtimeEntities)
		return nullptr;

	const RuntimeEntity *topEntity = nullptr;
	int topDrawIndex = -1;
	for (const StartupRegionRecord &region : sceneRegions) {
		const RuntimeEntity *entity = runtimeEntities->findSceneEntityByName(region.regionName);
		if (!entity || !entity->hitTest(point))
			continue;
		if (entity->getClassId() != kRuntimeEntityClassRectHotspot19)
			continue;
		const int drawIndex = runtimeEntities->findSceneEntityDrawIndexByName(entity->getName());
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

static const RuntimeEntity *findRoomPlayerAtPoint(HarvesterEngine &engine, const Common::Point &point) {
	RuntimeEntityManager *runtimeEntities = engine.getRuntimeEntities();
	if (!runtimeEntities)
		return nullptr;

	const RuntimeEntity *entity = runtimeEntities->findTopSceneEntityAt(point);
	if (!entity || entity->getClassId() != kRuntimeEntityClassPlayer)
		return nullptr;

	return entity;
}

static const StartupNpcRecord *findRoomNpcAtPoint(HarvesterEngine &engine,
		const Common::Array<StartupNpcRecord> &sceneNpcs, const Common::Point &point) {
	RuntimeEntityManager *runtimeEntities = engine.getRuntimeEntities();
	if (!runtimeEntities)
		return nullptr;

	const RuntimeEntity *entity = runtimeEntities->findTopSceneEntityAt(point);
	if (!entity || entity->getClassId() != kRuntimeEntityClassNpc)
		return nullptr;

	return findSceneNpcByName(sceneNpcs, entity->getName());
}

const IndexedBitmap *resolveInspectTextboxBitmap(const Art &art, const StartupResolvedText &text) {
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
		const StartupResolvedText &inspectText, bool useNativeFont) {
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

bool unlocksRoomObjectInteractionAfterInitialExamine(const StartupObjectRecord &object,
		Script &startupScript) {
	return object.operatable || startupScript.isPickupObject(object);
}

static int resolveRoomObjectCursorSequence(const StartupObjectRecord &object, Script &startupScript) {
	if (object.objectName.equalsIgnoreCase("EXIT_BM") || object.objectName.equalsIgnoreCase("EXIT_HS"))
		return kCursorSequenceTransition;

	StartupResolvedText inspectText;
	const bool pickupObject = startupScript.isPickupObject(object);
	const bool pickupBlocked = pickupObject && startupScript.isPickupBlockedByAction(object);
	if (!object.identShown && unlocksRoomObjectInteractionAfterInitialExamine(object, startupScript))
		return kCursorSequenceExamine;
	if (pickupObject && !pickupBlocked)
		return kCursorSequencePickup;
	if (pickupBlocked)
		return kCursorSequenceExamine;
	if (object.operatable)
		return kCursorSequenceOperate;
	if (startupScript.resolveObjectInspectText(object, inspectText))
		return kCursorSequenceExamine;

	if (startupScript.hasObjectInteraction(object))
		return kCursorSequenceExamine;

	return kCursorSequenceNeutral;
}

static Common::String buildRoomObjectPrompt(const StartupObjectRecord &object, Script &startupScript,
		int cursorSequence) {
	// Native room prompts come from explicit interaction metadata. Neutral scene sprites
	// should not surface synthetic "Examine <object id>" prompts or steal hover/click focus.
	if (cursorSequence == kCursorSequenceNeutral)
		return Common::String();

	const Common::String label = startupScript.resolveObjectLabel(object);
	if (label.empty())
		return Common::String();

	if (cursorSequence == kCursorSequenceOperate)
		return Common::String::format("Operate the %s", label.c_str());
	if (cursorSequence == kCursorSequencePickup)
		return Common::String::format("Pick up the %s", label.c_str());
	if (cursorSequence == kCursorSequenceTalk)
		return Common::String::format("Talk to %s", label.c_str());
	if (cursorSequence == kCursorSequenceTransition)
		return Common::String::format("Go to %s", label.c_str());

	return Common::String::format("Examine %s", label.c_str());
}

static Common::String buildRoomNpcPrompt(const StartupNpcRecord &npc) {
	Common::String label = !npc.entityInitArg.empty() ? npc.entityInitArg : npc.npcName;
	for (uint i = 0; i < label.size(); ++i) {
		if (label[i] == '_')
			label.setChar(' ', i);
	}

	if (label.empty())
		return Common::String();
	return Common::String::format("Talk to %s", label.c_str());
}

bool doesPlayerFacingMatchRegion(int playerFacing, const StartupRegionRecord &region) {
	return region.desiredFacing < 0 || playerFacing == region.desiredFacing;
}

bool doesPlayerOverlapRegion(const RuntimeEntity &playerEntity, const StartupRegionRecord &region) {
	const Common::Rect regionBounds = getRegionBounds(region);
	if (regionBounds.isEmpty())
		return false;
	if (!playerEntity.getScreenRect().intersects(regionBounds))
		return false;

	const float playerMaxZ = playerEntity.getZ() + playerEntity.getZExtent();
	return playerMaxZ >= (float)region.minZ && (float)region.maxZ >= playerEntity.getZ();
}

StartupRoomHoverState resolveRoomHoverState(HarvesterEngine &engine, const StartupRoomSetupState &state,
		const Common::Array<StartupObjectRecord> &sceneObjects,
		const Common::Array<StartupNpcRecord> &sceneNpcs,
		const Common::Array<StartupRegionRecord> &sceneRegions, const Common::Point &mousePos) {
	StartupRoomHoverState hoverState;
	RuntimeEntityManager *runtimeEntities = engine.getRuntimeEntities();
	if (const RuntimeEntity *playerEntity = findRoomPlayerAtPoint(engine, mousePos)) {
		hoverState.playerEntity = playerEntity;
		hoverState.cursorSequence = kCursorSequenceExamine;
		return hoverState;
	}
	if (const StartupNpcRecord *npc = findRoomNpcAtPoint(engine, sceneNpcs, mousePos)) {
		hoverState.npc = npc;
		hoverState.cursorSequence = kCursorSequenceTalk;
		hoverState.promptText = buildRoomNpcPrompt(*npc);
		return hoverState;
	}

	Script *startupScript = engine.getStartupScript();
	if (!startupScript)
		return hoverState;
	hoverState.object = findRoomObjectAtPoint(engine, sceneObjects, mousePos);
	hoverState.region = findRoomRegionAtPoint(engine, sceneRegions, mousePos);
	if (runtimeEntities && hoverState.object && hoverState.region) {
		const int objectDrawIndex =
			runtimeEntities->findSceneEntityDrawIndexByName(hoverState.object->objectName);
		const int regionDrawIndex =
			runtimeEntities->findSceneEntityDrawIndexByName(hoverState.region->regionName);
		if (regionDrawIndex >= 0 && objectDrawIndex >= 0 && regionDrawIndex > objectDrawIndex)
			hoverState.object = nullptr;
	}
	if (hoverState.object) {
		hoverState.cursorSequence = resolveRoomObjectCursorSequence(*hoverState.object, *startupScript);
		hoverState.promptText = buildRoomObjectPrompt(*hoverState.object, *startupScript, hoverState.cursorSequence);
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

static bool findRoomObjectProbePoint(HarvesterEngine &engine, const Common::Array<StartupObjectRecord> &sceneObjects,
		const StartupObjectRecord &object, Common::Point &probePoint) {
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

void logStartupRoomProbe(HarvesterEngine &engine, const StartupRoomSceneResources &scene,
		const Common::String &entranceName, Common::Point &mousePos) {
	RuntimeEntityManager *runtimeEntities = engine.getRuntimeEntities();
	Script *startupScript = engine.getStartupScript();
	if (!runtimeEntities || !startupScript)
		return;

	if (const RuntimeEntity *cursor = runtimeEntities->getCursorEntity()) {
		uint32 framePixels = 0;
		uint32 transparentPixels = 0;
		uint32 preservedPixels = 0;
		if (cursor->measureCurrentFrameTransparency(framePixels, transparentPixels, preservedPixels)) {
			debugC(1, kDebugCursor,
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
		debugC(1, kDebugPlayer,
			"Harvester: startup probe player room='%s' entrance='%s' frame=%d rect=(%d,%d)-(%d,%d) visible=%d intersects_screen=%d opaque_pixels=%u",
			scene.state.roomName.c_str(), entranceName.c_str(), player->getCurrentFrame(),
			playerRect.left, playerRect.top, playerRect.right, playerRect.bottom,
			player->isVisible(), playerRect.intersects(screenRect), framePixels - transparentPixels);
	}

	for (const StartupObjectRecord &object : scene.sceneObjects) {
		if (getRoomObjectHotspotBounds(object).isEmpty())
			continue;

		Common::Point probePoint;
		if (!findRoomObjectProbePoint(engine, scene.sceneObjects, object, probePoint))
			continue;

		const StartupObjectRecord *hoveredObject = findRoomObjectAtPoint(engine, scene.sceneObjects, probePoint);
		if (!hoveredObject)
			continue;

		mousePos = probePoint;
		const Common::String objectLabel = startupScript->resolveObjectLabel(*hoveredObject);
		StartupResolvedText inspectText;
		const StartupRoomHoverState hoverState = resolveRoomHoverState(
			engine, scene.state, scene.sceneObjects, scene.state.roomNpcs, scene.sceneRegions, probePoint);
		const bool hasInteraction = startupScript->hasObjectInteraction(*hoveredObject);
		const bool hasInspectText = startupScript->resolveObjectInspectText(*hoveredObject, inspectText);
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
					const StartupRoomHoverState candidateHover = resolveRoomHoverState(
						engine, scene.state, scene.sceneObjects, scene.state.roomNpcs, scene.sceneRegions,
						Common::Point(x, y));
					if (candidateHover.cursorSequence == kCursorSequenceWalk) {
						floorProbe = Common::Point(x, y);
						foundFloorProbe = true;
						break;
					}
				}
			}

			const StartupRoomHoverState floorHover = foundFloorProbe
				? resolveRoomHoverState(engine, scene.state, scene.sceneObjects, scene.state.roomNpcs,
					scene.sceneRegions, floorProbe)
				: StartupRoomHoverState();
			debugC(1, kDebugRoom,
				"Harvester: startup probe floor room='%s' point=(%d,%d) found=%d cursor_sequence=%d prompt='%s'",
				scene.state.roomName.c_str(), floorProbe.x, floorProbe.y, foundFloorProbe,
				floorHover.cursorSequence, floorHover.promptText.c_str());

			RuntimeEntity *player = runtimeEntities->findSceneEntityByName(kPlayerActorEntityName);
			if (player && foundFloorProbe && floorHover.cursorSequence == kCursorSequenceWalk) {
				StartupRoomPlayerState probePlayer;
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
	const Art *art = engine.getStartupArt();
	const Graphics::Font *font = FontMan.getFontByUsage(Graphics::FontManager::kGUIFont);
	Script *startupScript = engine.getStartupScript();
	if (!screen || !art || !font || !startupScript)
		return;

	drawRoomScene(engine, *screen, scene, scene.targetPaletteBrightness);
	blitBitmap(*screen, art->getTipsBitmap(), kQuickTipsOverlayX, kQuickTipsOverlayY);

	drawWrappedShadowedText(*screen, *font, tipText, kQuickTipTextX, kQuickTipTextY, kQuickTipTextWidth, kTextColorNormal);
	const Common::Rect exitRect = quickTipsExitRect();
	const Common::Rect nextRect = quickTipsNextRect();
	const Common::Rect toggleRect = quickTipsToggleRect();
	const Common::String toggleLabel = startupScript->resolveTextValue(
		startupScript->isQuickTipsEnabled() ? "Show_Tips_ON" : "Show_Tips_OFF");
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
	resetRoomNpcDialogueState();
	Common::Error error = Common::kNoError;
	if (_engine.hasPendingLoadedStartupSaveRoomState()) {
		Common::String initialTarget = _engine.getPendingLoadedStartupSaveRoomState().entranceName;
		if (initialTarget.empty())
			initialTarget = _engine.getPendingLoadedStartupSaveRoomState().roomName;
		error = runRoomLoop(initialTarget);
	} else {
		_engine.clearCurrentStartupSaveRoomState();
		_engine.clearPendingLoadedDialogueStateBlob();
		_engine.getStartupScript()->resetRuntimeState();
		error = runQuickTips();
		if (error.getCode() != Common::kNoError)
			return error;

		clearPendingMainMenuReturn();
		_engine.getStartupScript()->resetRuntimeState();
		error = runRoomLoop("START");
	}
	if (error.getCode() != Common::kNoError)
		return error;
	if (!takePendingMainMenuReturn())
		return Common::kNoError;

	_engine.stopStartupMusic();
	_engine.stopStartupSound();
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

bool Flow::loadMenuItems() {
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

Common::Error Flow::runQuickTips() {
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
		_engine.getStartupScript()->resolveTextValue(
			_engine.getStartupScript()->isQuickTipsEnabled() ? "Show_Tips_ON" : "Show_Tips_OFF").c_str());

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

Common::Error Flow::runMainMenuStub() {
	return _menu.runMainMenuStub(*this);
}

Common::Error Flow::runRoomMenuStub(const IndexedBitmap &backdrop, const byte *palette, float paletteBrightness) {
	return _menu.runRoomMenuStub(backdrop, palette, paletteBrightness, *this);
}

Common::Error Flow::runRoomNpcDialogue(const IndexedBitmap &backdrop, const byte *palette,
		float paletteBrightness, const StartupNpcRecord &npc, const Common::String &usedItemName) {
	_queuedDialogueInteraction = StartupInteractionResult();
	_hasQueuedDialogueInteraction = false;
	return _dialogue.runRoomNpcDialogue(backdrop, palette, paletteBrightness, npc, usedItemName, *this);
}

Common::Error Flow::runTownMapSelector(const Common::String &mapEntryName,
		Common::String &destinationEntranceName) {
	destinationEntranceName.clear();

	Script *startupScript = _engine.getStartupScript();
	ResourceManager *resources = _engine.getResources();
	Graphics::Screen *screen = _engine.getScreen();
	const Graphics::Font *font = FontMan.getFontByUsage(Graphics::FontManager::kGUIFont);
	std::unique_ptr<HarvesterCftFont> townMapFont;
	if (const CftFontResource *townMapFontResource = findStartupFontByName(_engine, "TEXTFONT")) {
		townMapFont.reset(new HarvesterCftFont(*townMapFontResource));
		if (townMapFont->isValid())
			font = townMapFont.get();
		else
			townMapFont.reset();
	}
	if (!startupScript || !resources || !screen || !font)
		return Common::kReadingFailed;

	const StartupMapEntranceRecord *mapEntrance = startupScript->findMapEntranceRecord(mapEntryName);
	if (!mapEntrance) {
		warning("Harvester: unresolved town map entry '%s'", mapEntryName.c_str());
		return Common::kReadingFailed;
	}
	const float paletteBrightness = startupScript->getFlagValue("DAY_FLAG")
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

	const Common::String previousMusicPath = _engine.getStartupMusicPath();
	(void)_engine.playStartupMusic(kTownMapMusicPath);

	if (!ensureCursorEntity())
		return Common::kReadingFailed;
	resetCursorAnimationSequence();

	if (_mousePos.x < 0 || _mousePos.y < 0 || _mousePos.x >= screen->w || _mousePos.y >= screen->h)
		_mousePos = Common::Point(screen->w / 2, screen->h / 2);
	if (RuntimeEntityManager *runtimeEntities = _engine.getRuntimeEntities())
		(void)runtimeEntities->syncCursorEntityPosition(_mousePos);

	int currentPanel = clampTownMapPanelIndex(mapEntrance->initialPanelIndex);
	bool needsRedraw = true;
	Graphics::FrameLimiter limiter(g_system, 60);

	auto centerCursor = [&]() {
		_mousePos = Common::Point(screen->w / 2, screen->h / 2);
		if (RuntimeEntityManager *runtimeEntities = _engine.getRuntimeEntities())
			(void)runtimeEntities->syncCursorEntityPosition(_mousePos);
	};
	auto restorePreviousMusic = [&]() {
		if (!destinationEntranceName.empty())
			return;
		if (!previousMusicPath.empty())
			(void)_engine.playStartupMusic(previousMusicPath);
		else
			_engine.stopStartupMusic();
	};

	while (!_engine.shouldQuit()) {
		const int edgePanel = resolveTownMapEdgePanel(currentPanel, _mousePos, screen->w, screen->h);
		if (edgePanel != currentPanel) {
			currentPanel = edgePanel;
			centerCursor();
			needsRedraw = true;
		}

		const StartupMapLocationRecord *hoveredLocation =
			findTownMapLocationAt(startupScript->getMapLocations(), currentPanel, _mousePos);
		if (needsRedraw) {
			setScaledPalette(*screen, palette, paletteBrightness);
			screen->fillRect(screen->getBounds(), 0);
			blitBitmap(*screen, panels[(uint)currentPanel], 0, 0);
			if (hoveredLocation) {
				drawShadowedString(*screen, *font, hoveredLocation->labelText,
					hoveredLocation->labelX, hoveredLocation->labelY, screen->w, kTownMapLabelColor);
			}
			if (RuntimeEntityManager *runtimeEntities = _engine.getRuntimeEntities())
				runtimeEntities->drawCursor(*screen);
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
				const StartupMapLocationRecord *clickedLocation =
					findTownMapLocationAt(startupScript->getMapLocations(), currentPanel, _mousePos);
				if (!clickedLocation)
					break;

				const StartupEntranceRecord *destinationEntrance =
					startupScript->findEntranceRecord(clickedLocation->destinationEntranceName);
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

	Script *startupScript = _engine.getStartupScript();
	if (!startupScript || targetName.empty())
		return Common::kNoError;

	if (startupScript->findMapEntranceRecord(targetName))
		return runTownMapSelector(targetName, resolvedTargetName);

	return Common::kNoError;
}

void Flow::queueDialogueInteraction(const StartupInteractionResult &interaction) {
	_queuedDialogueInteraction = interaction;
	_hasQueuedDialogueInteraction = true;
}

bool Flow::takeQueuedDialogueInteraction(StartupInteractionResult &interaction) {
	if (!_hasQueuedDialogueInteraction)
		return false;

	interaction = _queuedDialogueInteraction;
	_queuedDialogueInteraction = StartupInteractionResult();
	_hasQueuedDialogueInteraction = false;
	return true;
}

void Flow::prepareForNewGame() {
	clearPendingMainMenuReturn();
	clearPendingGameOverReturn();
	clearPendingNewGameRestart();
	_engine.clearPendingLoadedStartupSaveRoomState();
	_engine.clearPendingLoadedDialogueStateBlob();
	_engine.clearCurrentStartupSaveRoomState();
	if (_engine.getStartupScript())
		_engine.getStartupScript()->resetRuntimeState();
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
	_engine.clearCurrentStartupSaveRoomState();
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

Common::Error Flow::runRoomLoop(const Common::String &entranceName) {
	return _room.runRoomLoop(*this, entranceName);
}

bool Flow::ensureCursorEntity() {
	RuntimeEntityManager *runtimeEntities = _engine.getRuntimeEntities();
	if (!runtimeEntities)
		return false;
	if (runtimeEntities->getCursorEntity())
		return true;

	return runtimeEntities->spawnCursorEntity(_mousePos) != nullptr;
}

bool Flow::populateRoomSceneEntities(const StartupRoomSetupState &state,
		const Common::Array<StartupObjectRecord> &drawableObjects,
		const Common::Array<StartupAnimRecord> &drawableAnimations) {
	RuntimeEntityManager *runtimeEntities = _engine.getRuntimeEntities();
	if (!runtimeEntities)
		return false;

	RuntimeEntity *preservedPlayer = runtimeEntities->detachSceneEntityByName(kPlayerActorEntityName);
	runtimeEntities->clearSceneEntities();
	for (const StartupRegionRecord &region : state.roomRegions) {
		const Common::Rect regionBounds = getRegionBounds(region);
		if (regionBounds.isEmpty())
			continue;

		RuntimeEntity *entity = runtimeEntities->spawnSceneHotspotEntity(
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
	for (const StartupTimerRecord &timer : state.roomTimers) {
		if (!runtimeEntities->spawnSceneTimerEntity(timer.timerName,
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
	for (const StartupObjectRecord &object : drawableObjects) {
		RuntimeEntity *entity = nullptr;
		const Common::String spritePath = resolveSceneObjectSpritePath(object);
		const Common::Rect hotspotBounds = getRoomObjectHotspotBounds(object);
		if (!spritePath.empty() && spritePath.hasSuffixIgnoreCase(".BM")) {
			entity = runtimeEntities->spawnSceneBitmapEntity(object.objectName, spritePath,
				Common::Point(object.currentX, object.currentY), (float)object.currentZ);
		} else {
			if (!hotspotBounds.isEmpty())
				entity = runtimeEntities->spawnSceneHotspotEntity(object.objectName, hotspotBounds, (float)object.currentZ);
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

		entity->setClassId(resolveSceneObjectClass(object, entity->hasFrames() ? entity : nullptr, _engine.getStartupScript()));
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
	for (const StartupAnimRecord &anim : drawableAnimations) {
		if (!runtimeEntities->spawnSceneAnimationEntity(anim.animName, anim.resourcePath,
				Common::Point(anim.x, anim.y), (float)anim.z, anim.frameDelay, anim.active, anim.visible,
				anim.looping, anim.backward, anim.pingPong, anim.runtimeState)) {
			debug(1, "Harvester: unable to spawn room anim entity '%s' from '%s'",
				anim.animName.c_str(), anim.resourcePath.c_str());
		}
	}
	for (const StartupNpcRecord &npc : state.roomNpcs) {
		RuntimeEntity *entity = runtimeEntities->spawnSceneActorEntity(npc.npcName,
			npc.modelPath, Common::Point(npc.posX, npc.posY), (float)npc.posZ, 0);
		if (!entity) {
			debug(1, "Harvester: unable to spawn room npc entity '%s' from '%s'",
				npc.npcName.c_str(), npc.modelPath.c_str());
			continue;
		}

		entity->setClassId(kRuntimeEntityClassNpc);
		entity->setHitTestMode(kRuntimeEntityHitTestOpaquePixels);
		// Native spawn_npc_entity_from_record seeds the passive room-NPC loop as frames 0..0x3b.
		entity->setAnimationFrameRange(0, MIN(entity->getLastFrame(), kRoomNpcAmbientLastFrame), true);
		if (npc.frameDelay > 0)
			entity->setAnimationRate(npc.frameDelay);
		// Native room NPCs come from spawn_abm_entity_base, which leaves the depth-scale flag cleared.
		if (!applyRoomActorPlacementInternal(state, *entity,
				npc.posX, npc.posY, (float)npc.posZ, nullptr, false)) {
			debug(1, "Harvester: unable to apply room npc placement for '%s'",
				npc.npcName.c_str());
		}
		runtimeEntities->reinsertSceneEntity(entity);
		debugC(1, kDebugRoom,
			"Harvester: scene npc spawned room='%s' npc='%s' class=0x%x pos=(%d,%d,z=%.2f) frame_delay=%d model='%s' active=%d visible=%d",
			state.roomName.c_str(), npc.npcName.c_str(), entity->getClassId(),
			entity->getX(), entity->getY(), (double)entity->getZ(),
			npc.frameDelay, npc.modelPath.c_str(), npc.active, npc.visible);
	}
	for (const StartupMonsterRecord &monster : state.roomMonsters) {
		RuntimeEntity *entity = runtimeEntities->spawnSceneActorEntity(monster.monsterName,
			monster.modelPath, Common::Point(monster.posX, monster.posY), (float)monster.posZ,
			Monster::resolveFacingFrame(monster.facing));
		if (!entity) {
			debug(1, "Harvester: unable to spawn room monster entity '%s' from '%s'",
				monster.monsterName.c_str(), monster.modelPath.c_str());
			continue;
		}

		entity->setClassId(kRuntimeEntityClassMonster);
		entity->setHitTestMode(kRuntimeEntityHitTestNone);
		entity->setVisible(monster.visible);
		if (!applyRoomActorPlacement(state, *entity, monster.posX, monster.posY, (float)monster.posZ)) {
			debug(1, "Harvester: unable to apply room monster placement for '%s'",
				monster.monsterName.c_str());
		}
		runtimeEntities->reinsertSceneEntity(entity);
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
		Script *startupScript = _engine.getStartupScript();
		const int playerCombatLoadout = startupScript ? startupScript->getPlayerCombatLoadout() : 0;
		const Common::String playerResourcePath =
			Player::resolveCombatLoadoutResourcePath(playerCombatLoadout);
		const bool reusedPlayer = preservedPlayer != nullptr;
		RuntimeEntity *player = preservedPlayer;
		if (!reusedPlayer) {
			player = runtimeEntities->spawnSceneActorEntity(kPlayerActorEntityName,
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
				StartupRoomPlayerState playerState;
				playerState.entity = player;
				playerState.centerX = state.playerSpawnX;
				playerState.bottomY = state.playerSpawnY;
				playerState.z = (float)state.playerSpawnZ;
				playerState.facing = state.playerFacing;
				playerState.combatLoadout = -1;
				(void)Player::syncCombatLoadoutVisual(_engine, state, playerState, playerCombatLoadout);
			}
			if (reusedPlayer)
				runtimeEntities->adoptSceneEntity(player);
			else
				runtimeEntities->reinsertSceneEntity(player);
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

	const Art *art = _engine.getStartupArt();
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

void Flow::executeStartupAudioCommands(const Common::Array<StartupAudioCommand> &commands) {
	for (const StartupAudioCommand &command : commands)
		(void)_engine.executeStartupAudioCommand(command);
}

void Flow::resetRoomNpcDialogueState() {
	_dialogue.resetRoomNpcDialogueState();
}

void Flow::resetCursorAnimationSequence() {
	if (!ensureCursorEntity())
		return;

	RuntimeEntity *cursor = _engine.getRuntimeEntities()->getCursorEntity();
	if (!cursor)
		return;

	cursor->setVisible(true);
	cursor->setAnimationSequence(kCursorSequence7);
	(void)_engine.getRuntimeEntities()->syncCursorEntityPosition(_mousePos);
}

bool Flow::tickRuntimeEntities() {
	RuntimeEntityManager *runtimeEntities = _engine.getRuntimeEntities();
	if (!runtimeEntities)
		return false;
	return runtimeEntities->tickSceneEntities() || runtimeEntities->syncCursorEntityPosition(_mousePos);
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
