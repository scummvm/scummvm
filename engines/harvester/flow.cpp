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

#include "harvester/flow.h"

#include "common/algorithm.h"
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/events.h"
#include "common/formats/ini-file.h"
#include "common/memstream.h"
#include "common/ptr.h"
#include "common/serializer.h"
#include "common/system.h"
#include "graphics/font.h"
#include "graphics/fontman.h"
#include "graphics/framelimiter.h"
#include "harvester/cft_font.h"
#include "harvester/detection.h"
#include "harvester/fst_player.h"
#include "harvester/harvester.h"
#include "harvester/input.h"
#include "harvester/palette_utils.h"
#include "harvester/resources.h"
#include "harvester/entity.h"
#include "harvester/scene_entities.h"
#include "harvester/room_presentation.h"
#include "harvester/art.h"
#include "harvester/monster.h"
#include "harvester/player.h"
#include "harvester/script.h"

namespace Harvester {

namespace {

static const char *const kQuickTipsPath = "ADJHEAD.RCS";
static const char *const kMenuPath = "MENU.INI";
static const char *const kMenuSectionName = "menu";
static const char *const kBuiltInMenuItems[] = {
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

static const int kCursorSequence7 = 7;
static const int kNativeIdentTextLineSpacing = 3;
static const char *const kPlayerActorEntityName = "PLAYER";
// ramp_palette_brightness (0x23a30) waits four DOS centiseconds per step.
static const uint32 kPaletteFadeIntervalTicks = 4;
static const float kPaletteBrightnessBlack = 0.0f;
static const float kPaletteBrightnessFull = 1.0f;
static const uint32 kDemoEndingScreenDurationMs = 10000;
static const int kDemoEndingScreenNumbers[] = { 1, 2, 3, 4, 6 };
static const char *const kDemoEndingMusicPath = "SOUND/MUSIC/ROCKFITE.CMP";
static const uint32 kDemoIntroductionDurationMs = 17000;
static const char *const kDemoIntroductionBitmapPath = "1:/GRAPHIC/OTHER/MAINMENU.BM";
static const char *const kDemoIntroductionPalettePath = "1:/GRAPHIC/PAL/MAINMENU.PAL";
static const char *const kDemoIntroductionMusicPath = "SOUND/MUSIC/LODGERB.CMP";
static const char *const kDemoIntroductionFstPath = "GRAPHIC/FST/PCFALL.FST";
static const char *const kDemoIntroductionText =
	"After solving the enigmas of the Harvest town, you are thrust into the realm of "
	"the Order of the Harvest Moon, a bizarre institution of secretive knowledge.  "
	"However, the only thing you really care about is escaping from this hellish reality...";
static const float kTownMapNightPaletteBrightness = 0.6f;
// Native keeps WAIT_BM visible while room construction runs; fast local loads need a short floor.
static const uint32 kTransitionWaitFrameMinMs = 120;
static const int kTownMapEdgeThreshold = 9;
static const int kTownMapCursorHitExtent = 5;

static const byte kTownMapLabelColor = 0x28;

static const int kCursorSequenceWalk = 0;
} // End of anonymous namespace

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
	entity.setAnchorMode(kEntityAnchorTopLeft);
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

void logScenePaletteSummary(const char *label, const SceneState &scene, float brightness) {
	debugC(2, kDebugRoom, "Harvester: %s room='%s' palette='%s' brightness=%.2f",
		label, scene.state.roomName.c_str(), scene.state.palettePath.c_str(), (double)brightness);
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

static Common::Rect getRegionBounds(const RegionRecord &region) {
	if (region.right > region.left && region.bottom > region.top)
		return Common::Rect(region.left, region.top, region.right, region.bottom);

	return Common::Rect();
}

bool loadBitmapResource(ResourceManager &resources, const Common::String &path, IndexedBitmap &bitmap) {
	Common::Array<byte> data;
	if (!resources.loadFile(path, data) || data.size() < 12)
		return false;

	bitmap = IndexedBitmap();
	return decodeBitmap(data, bitmap);
}

bool shouldRunStartupRoomProbe() {
	return ConfMan.hasKey("harvester_debug_probe_startup_room") &&
		ConfMan.getBool("harvester_debug_probe_startup_room");
}

bool doesPlayerFacingMatchRegion(int playerFacing, const RegionRecord &region) {
	return region.desiredFacing < 0 || playerFacing == region.desiredFacing;
}

bool doesPlayerOverlapRegion(const Entity &playerEntity, const RegionRecord &region) {
	const Common::Rect regionBounds = getRegionBounds(region);
	if (regionBounds.isEmpty())
		return false;

	// Native do_entity_screen_bounds_overlap (0x4b700) compares each entity's
	// origin plus width/height and rejects only when one edge is strictly before
	// the other. Common::Rect uses exclusive right/bottom edges, so intersects()
	// would incorrectly reject the edge contact that activates narrow exits.
	const Common::Rect playerBounds = playerEntity.getScreenRect();
	if (playerBounds.right < regionBounds.left ||
			regionBounds.right < playerBounds.left ||
			playerBounds.bottom < regionBounds.top ||
			regionBounds.bottom < playerBounds.top) {
		return false;
	}

	const int playerMaxZ = (int)(playerEntity.getZ() + playerEntity.getZExtent());
	return playerMaxZ >= region.minZ && region.maxZ >= (int)playerEntity.getZ();
}

RoomHoverState Flow::getRoomHoverState(const SceneState &scene) const {
	return Input::resolveRoomHoverState(_engine, scene.state, scene.sceneObjects,
		scene.state.roomNpcs, scene.sceneRegions, _mousePos, _menuTextConfig, &_dialogue);
}

static bool findRoomObjectProbePoint(HarvesterEngine &engine, const Common::Array<ObjectRecord> &sceneObjects,
		const ObjectRecord &object, Common::Point &probePoint) {
	const Common::Rect bounds = SceneEntities::getSceneObjectBounds(object);
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
		const ObjectRecord *hit = Input::findRoomObjectAtPoint(engine, sceneObjects, candidate);
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
			const ObjectRecord *hit = Input::findRoomObjectAtPoint(engine, sceneObjects, candidate);
			if (hit && hit->objectName.equalsIgnoreCase(object.objectName)) {
				probePoint = candidate;
				return true;
			}
		}
	}

	return false;
}

void Flow::logStartupRoomProbe(const SceneState &scene, const Common::String &entranceName) {
	EntityManager *entityManager = _engine.getRuntimeEntities();
	Script *script = _engine.getScript();
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
		if (SceneEntities::getSceneObjectBounds(object).isEmpty())
			continue;

		Common::Point probePoint;
		if (!findRoomObjectProbePoint(_engine, scene.sceneObjects, object, probePoint))
			continue;

		const ObjectRecord *hoveredObject = Input::findRoomObjectAtPoint(_engine, scene.sceneObjects, probePoint);
		if (!hoveredObject)
			continue;

		_mousePos = probePoint;
		const Common::String objectLabel = script->resolveObjectLabel(*hoveredObject);
		ResolvedText inspectText;
		const RoomHoverState hoverState = Input::resolveRoomHoverState(
			_engine, scene.state, scene.sceneObjects, scene.state.roomNpcs, scene.sceneRegions,
			probePoint, _menuTextConfig);
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
					const RoomHoverState candidateHover = Input::resolveRoomHoverState(
						_engine, scene.state, scene.sceneObjects, scene.state.roomNpcs, scene.sceneRegions,
						Common::Point(x, y), _menuTextConfig);
					if (candidateHover.cursorSequence == kCursorSequenceWalk) {
						floorProbe = Common::Point(x, y);
						foundFloorProbe = true;
						break;
					}
				}
			}

			const RoomHoverState floorHover = foundFloorProbe
				? Input::resolveRoomHoverState(_engine, scene.state, scene.sceneObjects, scene.state.roomNpcs,
					scene.sceneRegions, floorProbe, _menuTextConfig)
				: RoomHoverState();
			debugC(1, kDebugRoom,
				"Harvester: startup probe floor room='%s' point=(%d,%d) found=%d cursor_sequence=%d prompt='%s'",
				scene.state.roomName.c_str(), floorProbe.x, floorProbe.y, foundFloorProbe,
				floorHover.cursorSequence, floorHover.promptText.c_str());

			Entity *player = entityManager->findSceneEntityByName(kPlayerActorEntityName);
			if (player && foundFloorProbe && floorHover.cursorSequence == kCursorSequenceWalk) {
				Player probePlayer;
				probePlayer.initialize(scene.state, player, 0);
				probePlayer.setMoveTargetFromScreenPoint(scene.state, floorProbe.x, floorProbe.y);
				for (int i = 0; i < 32 && probePlayer.getState().hasMoveTarget; ++i)
					(void)probePlayer.stepMoveTarget(_engine, scene.state, scene.sceneObjects, scene.sceneAnimations);

				const Common::Rect movedRect = player->getScreenRect();
				debugC(1, kDebugPlayer,
					"Harvester: startup probe movement room='%s' target=(%d,%d) final_rect=(%d,%d)-(%d,%d) z=%.2f",
					scene.state.roomName.c_str(), floorProbe.x, floorProbe.y,
					movedRect.left, movedRect.top, movedRect.right, movedRect.bottom, (double)probePlayer.getState().z);

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

static bool loadQuickTipsScene(HarvesterEngine &engine, SceneState &scene) {
	RoomSetupState state;
	// The original startup path enters the START room first and overlays quick tips on top of it.
	if (!engine.getScript()->resolveRoomSetupState("START", state, *engine.getResources()))
		return false;

	return SceneEntities::loadRoomSceneResources(state, *engine.getResources(), scene);
}

static void renderQuickTipsScreen(HarvesterEngine &engine, const SceneState &scene,
		const MenuTextConfig &config, const QuickTipsLayout &layout,
		const Common::String &tipText) {
	Graphics::Screen *screen = engine.getScreen();
	if (!screen)
		return;

	RoomPresentation::drawRoomScene(engine, *screen, scene, scene.targetPaletteBrightness);
	drawQuickTipsPanel(engine, config, layout, tipText);

	if (engine.getRuntimeEntities())
		engine.getRuntimeEntities()->drawCursor(*screen);
	screen->makeAllDirty();
	screen->update();
}

Flow::Flow(HarvesterEngine &engine)
	: _engine(engine), _mousePos(320, 200), _dialogue(engine, _mousePos), _inventory(engine),
	  _menu(engine, _mousePos, _menuItems), _cheats(engine),
	  _room(engine, _inventory, _cheats) {
}

bool Flow::load() {
	(void)loadMenuTextConfig(_engine, _menuTextConfig);
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
	Common::Error passwordError = _menu.validateParentalPassword(*this);
	if (passwordError.getCode() != Common::kNoError || _engine.shouldQuit())
		return passwordError;

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
		if (_engine.isDemo()) {
			error = runDemoIntroduction();
			if (error.getCode() != Common::kNoError)
				return error;
		}
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
	return runMainMenu();
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
		for (const char *item : kBuiltInMenuItems)
			_menuItems.push_back(item);
		debugC(2, kDebugGeneral,
			"Harvester: using %u built-in startup menu items because '%s' is unavailable",
			(uint)_menuItems.size(), kMenuPath);
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

	SceneState scene;
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
		RoomPresentation::drawRoomScene(_engine, *screen, scene, kPaletteBrightnessBlack);
		screen->makeAllDirty();
		screen->update();

		logScenePaletteSummary("quick tips fade target", scene, scene.targetPaletteBrightness);
		transitionError = fadeInRoomScene(scene.palette, scene.targetPaletteBrightness);
		if (transitionError.getCode() != Common::kNoError)
			return transitionError;
	}

	resetCursorAnimationSequence();

	QuickTipsLayout quickTipsLayout;
	if (!resolveQuickTipsLayout(_engine, _menuTextConfig, quickTipsLayout))
		return Common::kReadingFailed;
	const Common::String &toggleLabel = _engine.getScript()->isQuickTipsEnabled()
		? _menuTextConfig.quickTipsOnLabel : _menuTextConfig.quickTipsOffLabel;
	debugC(1, kDebugGeneral,
		"Harvester: quick tips panel='%s' header='%s' exit='%s' next='%s' toggle='%s'",
		_menuTextConfig.hasQuickTipsHeader() ? "TEXTBOX6.BM" : "TIPS.BM",
		_menuTextConfig.quickTipsHeader.c_str(), _menuTextConfig.quickTipsExitLabel.c_str(),
		_menuTextConfig.quickTipsNextLabel.c_str(), toggleLabel.c_str());

	uint tipIndex = _engine.getRandomNumber(_quickTips.size() - 1);
	bool needsRedraw = true;
	Graphics::FrameLimiter limiter(g_system, 60);

	while (!_engine.shouldQuit()) {
		if (needsRedraw) {
			renderQuickTipsScreen(_engine, scene, _menuTextConfig, quickTipsLayout, _quickTips[tipIndex]);
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
				if (quickTipsLayout.exitRect.contains(_mousePos))
					return Common::kNoError;

				if (quickTipsLayout.nextRect.contains(_mousePos)) {
					tipIndex = (tipIndex + 1) % _quickTips.size();
					needsRedraw = true;
				} else if (quickTipsLayout.toggleRect.contains(_mousePos)) {
					_engine.getScript()->setQuickTipsEnabled(!_engine.getScript()->isQuickTipsEnabled());
					if (!resolveQuickTipsLayout(_engine, _menuTextConfig, quickTipsLayout))
						return Common::kReadingFailed;
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

	for (uint i = 0; i < ARRAYSIZE(kDemoEndingScreenNumbers) && !_engine.shouldQuit(); ++i) {
		const int screenNumber = kDemoEndingScreenNumbers[i];
		const Common::String bitmapPath = Common::String::format(
			"1:/GRAPHIC/OTHER/SCREEN%d.BM", screenNumber);
		const Common::String palettePath = Common::String::format(
			"1:/GRAPHIC/PAL/SCREEN%d.PAL", screenNumber);
		IndexedBitmap bitmap;
		byte palette[256 * 3];
		if (!loadBitmapResource(*resources, bitmapPath, bitmap) ||
				!Art::loadPaletteResource(*resources, palettePath, palette)) {
			warning("Harvester: unable to load DOS demo ending screen %d", screenNumber);
			return Common::kReadingFailed;
		}

		setScaledPalette(*screen, palette, kPaletteBrightnessBlack);
		screen->fillRect(Common::Rect(0, 0, screen->w, screen->h), 0);
		RoomPresentation::blitBitmap(*screen, bitmap, 0, 0);
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

Common::Error Flow::runDemoIntroduction() {
	Graphics::Screen *screen = _engine.getScreen();
	ResourceManager *resources = _engine.getResources();
	EntityManager *entityManager = _engine.getRuntimeEntities();
	const Art *art = _engine.getArt();
	const Text *text = _engine.getText();
	const CftFontResource *fontResource = text ? text->findFont("TEXTFONT") : nullptr;
	if (!screen || !resources || !art || !fontResource)
		return Common::kReadingFailed;

	IndexedBitmap bitmap;
	byte palette[256 * 3];
	if (!loadBitmapResource(*resources, kDemoIntroductionBitmapPath, bitmap) ||
			!Art::loadPaletteResource(*resources, kDemoIntroductionPalettePath, palette)) {
		return Common::kReadingFailed;
	}

	HarvesterCftFont font(*fontResource);
	if (!font.isValid())
		return Common::kReadingFailed;
	debugC(1, kDebugGeneral,
		"Harvester: showing DOS demo introduction for %u ms",
		kDemoIntroductionDurationMs);

	if (entityManager)
		entityManager->hideCursor();
	setScaledPalette(*screen, palette, kPaletteBrightnessBlack);
	screen->fillRect(Common::Rect(0, 0, screen->w, screen->h), 0);
	RoomPresentation::blitBitmap(*screen, bitmap, 0, 0);
	blitTransparentBitmap(*screen, art->getLogoBitmap(), 160, 0);
	RoomPresentation::drawWrappedText(*screen, font, kDemoIntroductionText, 90, 200, 420, 0xce,
		kNativeIdentTextLineSpacing);
	screen->makeAllDirty();
	screen->update();

	if (!_engine.playMusic(kDemoIntroductionMusicPath))
		return Common::kReadingFailed;
	Common::Error fadeError = fadePalette(
		palette, kPaletteBrightnessBlack, kPaletteBrightnessFull);
	if (fadeError.getCode() != Common::kNoError)
		return fadeError;

	const uint32 deadline = g_system->getMillis() + kDemoIntroductionDurationMs;
	bool skipIntroduction = false;
	while (!skipIntroduction && !_engine.shouldQuit() &&
			(int32)(deadline - g_system->getMillis()) > 0) {
		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			Common::Error eventError = Common::kNoError;
			if (handleSystemEvent(event, eventError))
				return eventError;
			if (event.type == Common::EVENT_LBUTTONDOWN ||
					event.type == Common::EVENT_RBUTTONDOWN ||
					(event.type == Common::EVENT_KEYDOWN &&
					(event.kbd.keycode == Common::KEYCODE_SPACE ||
					event.kbd.keycode == Common::KEYCODE_ESCAPE ||
					event.kbd.keycode == Common::KEYCODE_RETURN))) {
				skipIntroduction = true;
			}
		}
		g_system->delayMillis(1);
	}

	fadeError = fadePalette(palette, kPaletteBrightnessFull, kPaletteBrightnessBlack);
	if (fadeError.getCode() != Common::kNoError)
		return fadeError;
	_engine.stopMusic();

	FstPlayer fstPlayer(_engine);
	if (!fstPlayer.play(kDemoIntroductionFstPath))
		return Common::kReadingFailed;
	return Common::kNoError;
}

Common::Error Flow::runMainMenu() {
	return _menu.runMainMenu(*this);
}

Common::Error Flow::showCdChangePrompt(int discNumber) {
	if (discNumber <= 0)
		return Common::kNoError;

	ResourceManager *resources = _engine.getResources();
	if (!resources)
		return Common::kReadingFailed;
	const int previousDisc = resources->getCurrentDisc();

	if (_engine.shouldShowCdChangePrompts()) {
		bool promptCompleted = false;
		Common::Error promptError = _menu.showCdChangePrompt(discNumber, *resources, *this, promptCompleted);
		// System-event exits can return kNoError without completing the prompt.
		if (promptError.getCode() != Common::kNoError || !promptCompleted)
			return promptError;
	}

	if (!_engine.activateDisc(discNumber)) {
		warning("Harvester: unable to activate disc %d resources", discNumber);
		return Common::kReadingFailed;
	}
	if ((discNumber == 1 || discNumber == 3) &&
			previousDisc > 0 && previousDisc != resources->getCurrentDisc()) {
		Script *script = _engine.getScript();
		if (!script || !script->reloadTownWorld(*resources)) {
			warning("Harvester: unable to reload town script after disc prompt %d -> %d",
				previousDisc, discNumber);
			return Common::kReadingFailed;
		}
	}

	return Common::kNoError;
}

Common::Error Flow::runRoomMenu(const IndexedBitmap &backdrop, const byte *palette,
		float paletteBrightness, bool canSaveGame) {
	return _menu.runRoomMenu(backdrop, palette, paletteBrightness, *this, canSaveGame);
}

Common::Error Flow::runRoomNpcDialogue(const IndexedBitmap &backdrop, const byte *palette,
		float paletteBrightness, const ScriptExecutionContext &executionContext,
		const NpcRecord &npc, const Common::String &usedItemName) {
	_queuedDialogueInteraction = InteractionResult();
	_hasQueuedDialogueInteraction = false;
	return _dialogue.runRoomNpcDialogue(backdrop, palette, paletteBrightness,
		executionContext, npc, usedItemName, *this);
}

Common::Error Flow::runTownMapSelector(const Common::String &mapEntryName,
		Common::String &destinationEntranceName) {
	destinationEntranceName.clear();

	Script *script = _engine.getScript();
	ResourceManager *resources = _engine.getResources();
	Graphics::Screen *screen = _engine.getScreen();
	const Graphics::Font *font = FontMan.getFontByUsage(Graphics::FontManager::kGUIFont);
	Common::ScopedPtr<HarvesterCftFont> townMapFont;
	const Text *text = _engine.getText();
	if (const CftFontResource *townMapFontResource = text ? text->findFont("TEXTFONT") : nullptr) {
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
	if (!Art::loadPaletteResource(*resources, kTownMapPalettePath, palette))
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
			RoomPresentation::blitBitmap(*screen, panels[(uint)currentPanel], 0, 0);
			if (hoveredLocation) {
				RoomPresentation::drawShadowedString(*screen, *font, hoveredLocation->labelText,
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
	_cheats.reset();
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

	SceneEntities sceneRuntime(state, _engine.getScript(), _engine.getResources(), entityManager);
	Entity *preservedPlayer = entityManager->detachSceneEntityByName(kPlayerActorEntityName);
	entityManager->clearSceneEntities(true);
	sceneRuntime.populateRoomRegions();
	for (const TimerRecord &timer : state.roomTimers) {
		Entity *existingTimer = entityManager->findSceneEntityByName(timer.timerName);
		if (existingTimer && existingTimer->getClassId() == kEntityClassTimer) {
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
	sceneRuntime.populateRoomObjects(drawableObjects);
	for (const AnimRecord &anim : drawableAnimations) {
		const bool displayVisible = shouldDisplaySceneAnimation(anim);
		if (!entityManager->spawnSceneAnimationEntity(anim.animName, anim.resourcePath,
				Common::Point(anim.x, anim.y), (float)anim.z, anim.frameDelay, anim.active, displayVisible,
				anim.looping, anim.backward, anim.pingPong, anim.runtimeState)) {
			debug(1, "Harvester: unable to spawn room anim entity '%s' from '%s'",
				anim.animName.c_str(), anim.resourcePath.c_str());
		}
	}
	sceneRuntime.populateRoomNpcs();
	sceneRuntime.populateRoomMonsters();
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
			player->setClassId(kEntityClassPlayer);
			player->setAnchorMode(kEntityAnchorTopLeft);
			player->setHitTestMode(kEntityHitTestOpaquePixels);
			player->setVisible(true);
			player->setAnimationRate(0);
			player->setAnimationFrameRange(playerFrame, playerFrame, false);
			player->setCurrentFrame(playerFrame);
			if (!applyStartupActorPlacement(state, *player)) {
				debug(1, "Harvester: unable to apply startup player placement for entrance '%s'",
					state.entranceName.c_str());
			}
			if (reusedPlayer) {
				Player loadoutPlayer;
				loadoutPlayer.initialize(state, player, -1);
				(void)loadoutPlayer.syncCombatLoadoutVisual(_engine, state, playerCombatLoadout);
			}
			Player spawnPlayer;
			spawnPlayer.initialize(state, player, 0);
			if (spawnPlayer.resolveBlockedStartupSpawn(_engine, state, drawableObjects, drawableAnimations)) {
				state.playerSpawnX = spawnPlayer.getState().centerX;
				state.playerSpawnY = spawnPlayer.getState().bottomY;
				state.playerSpawnZ = (int)spawnPlayer.getState().z;
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
			for (double brightness = 1.0;
					brightness > kPaletteBrightnessBlack; brightness -= 0.1) {
				const uint32 startTick = getRuntimeClockTicks();
				setScaledDisplayPalette(*screen, displayPalette, brightness);
				screen->makeAllDirty();
				screen->update();

				Common::Error fadeError = Common::kNoError;
				if (waitForPaletteFadeTick(startTick, fadeError))
					return fadeError;
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

	for (double brightness = 0.0; brightness < targetBrightness; brightness += 0.1) {
		const uint32 startTick = getRuntimeClockTicks();
		setScaledPalette(*screen, palette, brightness);
		screen->makeAllDirty();
		screen->update();

		Common::Error fadeError = Common::kNoError;
		if (waitForPaletteFadeTick(startTick, fadeError))
			return fadeError;
	}

	setScaledPalette(*screen, palette, targetBrightness);
	screen->makeAllDirty();
	screen->update();
	return Common::kNoError;
}

Common::Error Flow::fadePalette(const byte *palette, float fromBrightness,
		float toBrightness) {
	Graphics::Screen *screen = _engine.getScreen();
	if (!screen || !palette)
		return Common::kNoError;

	const double step = fromBrightness < toBrightness ? 0.1 : -0.1;
	for (double brightness = fromBrightness;
			step > 0.0f ? brightness < toBrightness : brightness > toBrightness;
			brightness += step) {
		const uint32 startTick = getRuntimeClockTicks();
		setScaledPalette(*screen, palette, brightness);
		screen->makeAllDirty();
		screen->update();

		Common::Error fadeError = Common::kNoError;
		if (waitForPaletteFadeTick(startTick, fadeError))
			return fadeError;
	}

	setScaledPalette(*screen, palette, toBrightness);
	screen->makeAllDirty();
	screen->update();
	return Common::kNoError;
}

bool Flow::waitForPaletteFadeTick(uint32 startTick, Common::Error &result) {
	const uint32 deadline = startTick + kPaletteFadeIntervalTicks;
	while ((int32)(deadline - getRuntimeClockTicks()) > 0) {
		if (pumpTransitionEvents(result))
			return true;
		g_system->delayMillis(1);
	}
	debugC(4, kDebugScene,
		"Harvester: palette fade step tick=%u->%u interval=%u",
		startTick, getRuntimeClockTicks(), kPaletteFadeIntervalTicks);
	return false;
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
	const bool sceneChanged = entityManager->tickSceneEntities();
	const bool cursorChanged = entityManager->syncCursorEntityPosition(_mousePos);
	return sceneChanged || cursorChanged;
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
