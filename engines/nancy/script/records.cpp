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

#include "common/config-manager.h"
#include "common/memstream.h"
#include "common/random.h"
#include "common/textconsole.h"

#include "engines/metaengine.h"

#include "image/png.h"

#include "engines/nancy/cursor.h"
#include "engines/nancy/enginedata.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/nancy.h"
#include "engines/nancy/puzzledata.h"
#include "engines/nancy/resource/resource.h"
#include "engines/nancy/sound.h"

#include "engines/nancy/script/api.h"
#include "engines/nancy/script/convo.h"
#include "engines/nancy/script/graphnode.h"
#include "engines/nancy/script/luabind.h"
#include "engines/nancy/script/particles.h"
#include "engines/nancy/script/physics.h"
#include "engines/nancy/script/records.h"
#include "engines/nancy/script/ref.h"
#include "engines/nancy/script/scenecontext.h"
#include "engines/nancy/script/timer.h"
#include "engines/nancy/script/transformer.h"
#include "engines/nancy/script/types.h"
#include "engines/nancy/script/xsheet.h"

#ifdef ENABLE_NANCY_V5
#include "engines/nancy/script/v5/cliptransformer.h"
#include "engines/nancy/script/v5/messaging.h"
#include "engines/nancy/script/v5/streamtree.h"
#include "engines/nancy/script/v5/textrecord.h"
#include "engines/nancy/script/v5/widgets.h"
#endif

#include "engines/nancy/state/scene.h"

#include "engines/nancy/ui/viewport.h"
#include "engines/nancy/ui/ndui/manager.h"

namespace Nancy {
namespace Script {

struct RecordKindNames {
	const char *kindName;
	const char *typeName;
};

static const RecordKindNames kRecordKindNames[kNumRecordTypes] = {
	{ "NavLogic", "ND.NavLogic" },
	{ "Summary", "ND.Summary" },
	{ "Overlay", "ND.Overlay" },
	{ "Movie", "ND.Movie" },
	{ "Sound", "ND.SoundAR" },
	{ "VideoConvo", "ND.ConvoAR" },
	{ "SoundConvo", "ND.ConvoAR" },
	{ "Text", "ND.TextAR" },
	{ "Override", "ND.OverrideAR" },
	{ "Particle", "ND.ParticleAR" },
	{ "AnimOverlay", "ND.AnimOvlAR" },
	{ "Physics", "ND.PhysicsAR" },
	{ "Xsheet", "ND.XsheetAR" },
	{ "Transformer", "ND.TransformerAR" },
	{ "Timer", "ND.TimerAR" },
	{ "Hotspot", "ND.Hotspot" },
	{ "Sink", "ND.SinkAR" },
	{ "Button", "ND.ButtonAR" },
	{ "EditBox", "ND.EditBoxAR" },
	{ "RadioButton", "ND.RadioButton" },
	{ "CheckBox", "ND.CheckBox" },
	{ "Slider", "ND.Slider" }
};

const char *getRecordKindName(RecordType type) {
	return kRecordKindNames[type].kindName;
}

const char *getRecordTypeName(RecordType type) {
	return kRecordKindNames[type].typeName;
}

Record *makeRecord(RecordType type, SceneContext &context) {
	switch (type) {
	case kRecordNavLogic:
#ifdef ENABLE_NANCY_V5
		// Nancy26's NavLogic is another name for a Hotspot
		if (g_nancy->getGameType() >= kGameTypeNancy26) {
			return new HotspotRecord(context);
		}
#endif

		return new NavLogicRecord(context);
#ifdef ENABLE_NANCY_V5
	case kRecordHotspot:
		return new HotspotRecord(context);
	case kRecordSink:
		return new SinkRecord(context);
	case kRecordButton:
		return new WidgetRecord(context, kRecordButton);
	case kRecordRadioButton:
		return new WidgetRecord(context, kRecordRadioButton);
	case kRecordCheckBox:
		return new WidgetRecord(context, kRecordCheckBox);
	case kRecordSlider:
		return new SliderRecord(context);
	case kRecordEditBox:
		return new EditBoxRecord(context);
#endif
	case kRecordSummary:
		return new SummaryRecord(context);
	case kRecordOverlay:
		return new OverlayRecord(context);
	case kRecordMovie:
		return new MovieRecord(context);
	case kRecordSound:
		return new SoundRecord(context);
	case kRecordOverride:
		return new OverrideRecord(context);
	case kRecordAnimOverlay:
		return new AnimOverlayRecord(context);
	case kRecordVideoConvo:
		return new ConvoRecord(context, true);
	case kRecordSoundConvo:
		return new ConvoRecord(context, false);
	case kRecordText:
#ifdef ENABLE_NANCY_V5
		if (g_nancy->getGameType() >= kGameTypeNancy26) {
			return new BitmapTextRecord(context);
		}
#endif

		return new TextRecord(context);
	case kRecordParticle:
		return new ParticleRecord(context);
	case kRecordPhysics:
		return new PhysicsRecord(context);
	case kRecordXsheet:
		return new XsheetRecord(context);
	case kRecordTransformer:
#ifdef ENABLE_NANCY_V5
		if (g_nancy->getGameType() >= kGameTypeNancy26) {
			return new ClipTransformerRecord(context);
		}
#endif

		return new TransformerRecord(context);
	case kRecordTimer:
		return new TimerRecord(context);
	default:
		return nullptr;
	}
}

// The background of a scene is as far back as anything gets, and the records
// default to the middle. The viewport has its place among what the engine draws,
// so the numbers of the scripts are counted from there.
static const double kBackgroundScriptZ = -5.0;
static const int kBackgroundZ = 6;

// Scripts tell apart z values as close as 0.001, and every stream is one step
// closer than the one that began it
static const double kStepsPerScriptZ = 1000.0;

uint16 RecordGraphic::getScriptZOrder(double z, int32 zOffset) {
#ifdef ENABLE_NANCY_V5
	if (g_nancy->getGameType() >= kGameTypeNancy26) {
		return getBandZOrder(z, zOffset);
	}
#endif

	const double steps = (z - kBackgroundScriptZ + zOffset) * kStepsPerScriptZ;
	return (uint16)CLIP<int>(kBackgroundZ + (int)floor(steps + 0.5), 0, kMaxScriptZOrder);
}

uint16 RecordGraphic::getBackgroundZOrder(int32 zOffset) {
#ifdef ENABLE_NANCY_V5
	if (g_nancy->getGameType() >= kGameTypeNancy26) {
		return getBandBackgroundZOrder(zOffset);
	}
#endif

	return getScriptZOrder(kBackgroundScriptZ, zOffset);
}

bool RecordGraphic::isViewportRelative() const {
	return g_nancy->getGameType() < kGameTypeNancy26;
}

Common::Rect RecordGraphic::getDrawArea() {
	if (g_nancy->getGameType() >= kGameTypeNancy26) {
		return g_nancy->_graphics->getScreenBounds();
	}

	const VIEW *viewport = GetEngineData(VIEW);
	assert(viewport);
	return Common::Rect(viewport->screenPosition.width(), viewport->screenPosition.height());
}

void RecordGraphic::setScriptZ(double z, const SceneContext &context) {
	setZOrder(getScriptZOrder(z, context.getZOffset()));
	setViewTransform(context.getViewTransform());
}

Common::Rect getTransformedDest(const ScriptRect &dest, const ScriptMatrix &transform, int width, int height) {
	// Far enough for anything a script moves around, and safe to add a size to
	const int kLimit = 16000;

	double x = dest.left;
	double y = dest.top;
	transform.transformPoint(x, y);

	const int left = CLIP<int>((int)x, -kLimit, kLimit);
	const int top = CLIP<int>((int)y, -kLimit, kLimit);
	return Common::Rect(left, top, left + width, top + height);
}

static const uint32 kNoModColor = 0xFFFFFFFF;

// Shows a part of an image where the transform of its record puts it. A transform
// that turns or resizes the picture has it drawn anew, into the area its corners
// end up around, and so does a color the picture gets multiplied with, which
// can darken it or make it see-through.
static void placePicture(RecordGraphic &graphic, Graphics::ManagedSurface &image, const Common::Rect &source, const ScriptRect &dest, const ScriptMatrix &transform, uint32 modColor) {
	const int kLimit = 16000;

	const bool isMovedOnly = transform.isTranslation();
	if (isMovedOnly && modColor == kNoModColor) {
		graphic._drawSurface.create(image, source);
		graphic.setTransparent(true);
		graphic.moveTo(getTransformedDest(dest, transform, source.width(), source.height()));
		graphic.setNeedsRedraw(true);
		return;
	}

	// Points are rows, so the matrix turns (x, y) into (x * a + y * c, x * b + y * d),
	// and then moves it
	const double a = transform.m[0][0];
	const double b = transform.m[0][1];
	const double c = transform.m[1][0];
	const double d = transform.m[1][1];
	const double offsetX = transform.m[3][0];
	const double offsetY = transform.m[3][1];
	const double determinant = a * d - b * c;

	const Common::Rect placed = getTransformedDest(dest, transform, source.width(), source.height());
	Common::Rect area = placed;
	if (!isMovedOnly) {
		double minX = 0.0;
		double minY = 0.0;
		double maxX = 0.0;
		double maxY = 0.0;
		for (uint corner = 0; corner < 4; ++corner) {
			double x = (corner & 1) ? dest.right : dest.left;
			double y = (corner & 2) ? dest.bottom : dest.top;
			transform.transformPoint(x, y);

			minX = corner == 0 ? x : MIN(minX, x);
			maxX = corner == 0 ? x : MAX(maxX, x);
			minY = corner == 0 ? y : MIN(minY, y);
			maxY = corner == 0 ? y : MAX(maxY, y);
		}

		area.left = CLIP<int>((int)floor(minX), -kLimit, kLimit);
		area.top = CLIP<int>((int)floor(minY), -kLimit, kLimit);
		area.right = CLIP<int>((int)ceil(maxX), area.left, kLimit);
		area.bottom = CLIP<int>((int)ceil(maxY), area.top, kLimit);
	}

	// Nothing shows outside the viewport, so only the part inside it gets drawn.
	// The map of Nancy25 is far larger than the viewport when zoomed in.
	area.clip(RecordGraphic::getDrawArea());
	const bool isOnScreen = !area.isEmpty();
	if (!isOnScreen) {
		area = Common::Rect(1, 1);
	}

	// Drawn with an alpha of its own, in place of the color that stands for
	// see-through in the image
	Graphics::ManagedSurface &out = graphic._drawSurface;
	out.create(area.width(), area.height(), g_nancy->_graphics->getTransparentPixelFormat());
	out.clear(0);
	graphic.setTransparent(false);

	// A picture with an alpha channel uses only that for transparency, like
	// when it is shown unchanged. The color key is black from Nancy16 on, and
	// pictures such as Nancy28's task checkboxes are drawn in black.
	byte transR, transG, transB;
	image.format.colorToRGB(g_nancy->_graphics->getTransColor(), transR, transG, transB);
	const bool hasAlpha = image.format.aBits() != 0;

	const uint modA = modColor >> 24;
	const uint modR = (modColor >> 16) & 0xFF;
	const uint modG = (modColor >> 8) & 0xFF;
	const uint modB = modColor & 0xFF;

	// A picture squashed flat, or one without a size, shows nothing
	const bool canDraw = isOnScreen && (isMovedOnly || (fabs(determinant) > 1e-9 && dest.width() != 0 && dest.height() != 0));
	const double scaleX = dest.width() != 0 ? (double)source.width() / dest.width() : 0.0;
	const double scaleY = dest.height() != 0 ? (double)source.height() / dest.height() : 0.0;

	// Back from the viewport to where a pixel is in the picture, which moves by
	// the same steps from one pixel to the next
	const double stepXAcross = d / determinant * scaleX;
	const double stepYAcross = -b / determinant * scaleY;
	const double stepXDown = -c / determinant * scaleX;
	const double stepYDown = a / determinant * scaleY;
	const double firstX = area.left + 0.5 - offsetX;
	const double firstY = area.top + 0.5 - offsetY;
	const double rowStartX = ((firstX * d - firstY * c) / determinant - dest.left) * scaleX;
	const double rowStartY = ((firstY * a - firstX * b) / determinant - dest.top) * scaleY;

	// A picture in the format drawn into, which keeps its colors, gets copied
	// as it is
	const bool isCopied = image.format == out.format && modColor == kNoModColor;
	const uint32 colorMask = ~(uint32)(image.format.aMax() << image.format.aShift);
	const uint32 transColor = g_nancy->_graphics->getTransColor() & colorMask;

	for (int y = 0; canDraw && y < out.h; ++y) {
		uint32 *outRow = (uint32 *)out.getBasePtr(0, y);
		double pictureX = rowStartX + y * stepXDown;
		double pictureY = rowStartY + y * stepYDown;

		for (int x = 0; x < out.w; ++x, pictureX += stepXAcross, pictureY += stepYAcross) {
			int sourceX = area.left - placed.left + x;
			int sourceY = area.top - placed.top + y;

			if (!isMovedOnly) {
				sourceX = (int)floor(pictureX);
				sourceY = (int)floor(pictureY);
			}

			if (sourceX < 0 || sourceX >= source.width() || sourceY < 0 || sourceY >= source.height()) {
				continue;
			}

			const uint32 pixel = image.getPixel(source.left + sourceX, source.top + sourceY);
			if (isCopied) {
				if (hasAlpha || (pixel & colorMask) != transColor) {
					outRow[x] = pixel;
				}

				continue;
			}

			byte alpha, red, green, blue;
			image.format.colorToARGB(pixel, alpha, red, green, blue);
			if (!hasAlpha && red == transR && green == transG && blue == transB) {
				continue;
			}

			if (!hasAlpha) {
				alpha = 0xFF;
			}

			outRow[x] = out.format.ARGBToColor(alpha * modA / 255, red * modR / 255, green * modG / 255, blue * modB / 255);
		}
	}

	graphic.moveTo(area);
	graphic.setNeedsRedraw(true);
}

// Lays a picture placePicture() made over another surface, as it would show
// on the screen. The origin is where the surface is in the viewport.
static void blendPicture(Graphics::ManagedSurface &target, const RecordGraphic &picture, bool isKeyed, const Common::Point &origin) {
	const Graphics::ManagedSurface &image = picture._drawSurface;
	Common::Rect area = picture.getScreenPositionRaw();
	area.translate(-origin.x, -origin.y);

	byte transR, transG, transB;
	image.format.colorToRGB(g_nancy->_graphics->getTransColor(), transR, transG, transB);
	const bool hasAlpha = image.format.aBits() != 0;

	for (int y = MAX<int>(0, -area.top); y < image.h && area.top + y < target.h; ++y) {
		for (int x = MAX<int>(0, -area.left); x < image.w && area.left + x < target.w; ++x) {
			byte alpha, red, green, blue;
			image.format.colorToARGB(image.getPixel(x, y), alpha, red, green, blue);
			if (isKeyed && !hasAlpha) {
				if (red == transR && green == transG && blue == transB) {
					continue;
				}

				alpha = 0xFF;
			}

			if (alpha == 0) {
				continue;
			}

			const int targetX = area.left + x;
			const int targetY = area.top + y;
			byte oldAlpha, oldRed, oldGreen, oldBlue;
			target.format.colorToARGB(target.getPixel(targetX, targetY), oldAlpha, oldRed, oldGreen, oldBlue);

			const uint rest = 255 - alpha;
			target.setPixel(targetX, targetY, target.format.ARGBToColor(
				alpha + oldAlpha * rest / 255,
				(red * alpha + oldRed * rest) / 255,
				(green * alpha + oldGreen * rest) / 255,
				(blue * alpha + oldBlue * rest) / 255));
		}
	}
}

static Graphics::ManagedSurface *s_drawTarget = nullptr;

Graphics::ManagedSurface *getDrawTarget() {
	return s_drawTarget;
}

// Summary

void SummaryRecord::readTable(int tableIndex) {
	lua_State *L = getLua();

	if (!readField(L, tableIndex, "env", _environment)) {
		luaL_error(L, "No environment specified");
	}

	// Nancy21 can leave the background out, e.g. for a scene that keeps the
	// picture of the one before as its facade
	if (!readField(L, tableIndex, "bg", _backgroundName) && g_nancy->getGameType() < kGameTypeNancy21) {
		luaL_error(L, "No background specified");
	}

	// Either whether themes keep playing, or the pause between them as well
	lua_getfield(L, tableIndex, "loopTheme");
	if (lua_type(L, -1) == LUA_TNUMBER) {
		_loopsTheme = true;
		_themePause = ABS(lua_tonumber(L, -1));
	} else if (!lua_isnil(L, -1)) {
		_loopsTheme = lua_toboolean(L, -1) != 0;
	}

	lua_pop(L, 1);

	// Whether turning goes on past the first and the last frame, and Nancy22:
	// whether the player can turn around
	readField(L, tableIndex, "wrap", _wraps);
	readField(L, tableIndex, "enableUturn", _hasUturn);
	readField(L, tableIndex, "enableRotate", _canRotate);
	readField(L, tableIndex, "facade", _isFacade);

	// The background is behind everything else
	limitZ(kBackgroundScriptZ);
}

// The channel and the volume of the themes
static const uint16 kThemeChannel = 0;
static const uint16 kThemeVolume = 85;

// Starts one of the themes of the area, unless one is playing
void SummaryRecord::playTheme() {
	if (g_nancy->_sound->isSoundPlaying(kThemeChannel)) {
		return;
	}

	const ENVS *environments = GetEngineData(ENVS);
	if (!environments) {
		return;
	}

	for (const ENVS::Environment &environment : environments->environments) {
		if (!environment.code.equalsIgnoreCase(_environment) || environment.musicNames.empty()) {
			continue;
		}

		SoundDescription sound;
		sound.name = environment.musicNames[g_nancy->_randomSource->getRandomNumber(environment.musicNames.size() - 1)];
		sound.channelID = kThemeChannel;
		sound.numLoops = 1;
		sound.volume = kThemeVolume;

		g_nancy->_sound->loadSound(sound);
		g_nancy->_sound->playSound(sound);
		return;
	}
}

void SummaryRecord::updateTheme(double sceneTime) {
	// How often the scene looks whether the theme is over
	static const double kThemeCheckSeconds = 1.0;

	if (!_loopsTheme || sceneTime <= _lastThemeCheck + kThemeCheckSeconds) {
		return;
	}

	_lastThemeCheck = sceneTime;

	if (_themeEndTime == 0.0 && !g_nancy->_sound->isSoundPlaying(kThemeChannel)) {
		_themeEndTime = sceneTime;
	}

	if (_themeEndTime != 0.0 && _themeEndTime + _themePause <= sceneTime) {
		_themeEndTime = 0.0;
		playTheme();
	}
}

void SummaryRecord::runOnce() {
	playTheme();

	if (_isFacade) {
		showFacade();
	}
}

// Nancy22: a scene with a facade keeps what the viewport showed when it started
// as its background, e.g. the character after a conversation is over
void SummaryRecord::showFacade() {
	const Graphics::ManagedSurface &image = g_nancy->_graphics->getGameImage();
	Common::Rect area = NancySceneState.getViewport().getScreenPosition();
	area.clip(Common::Rect(image.w, image.h));
	if (area.isEmpty()) {
		return;
	}

	_facade._drawSurface.copyFrom(image.rawSurface().getSubArea(area));
	_facade.init();
	_facade.moveTo(Common::Rect(area.width(), area.height()));
	_facade.setTransparent(false);
	_facade.setScriptZ(kBackgroundScriptZ, getContext());
	_facade.setVisible(true);
	_facade.registerGraphics();
}

void SummaryRecord::registerGraphics() {
	// The facade is only there once it has been shown
	if (_facade._drawSurface.getPixels()) {
		_facade.registerGraphics();
	}
}

void SummaryRecord::setBackground(UI::Viewport *background) {
	_background = background;
	_numFrames = background ? background->getFrameCount() : 0;
	_frame = getContext().getFrame();
}

void SummaryRecord::setFrame(uint frame) {
	_frame = frame;
	_lastTurnTime = _sceneTime;
	getContext().setFrame(frame);

	if (_background) {
		_background->setFrame(frame);
	}
}

SummaryRecord::Turn SummaryRecord::getTurnAt(const Common::Point32 &point, bool inViewport) const {
	// Only a background with several frames can be turned around in
	if (_numFrames <= 1 || !inViewport) {
		return kNoTurn;
	}

	const ScriptBootData *boot = GetEngineData(ScriptBootData);
	assert(boot);

	const Common::Point place(point.x, point.y);
	if (_canRotate && boot->leftNodeHotspot.contains(place)) {
		return canTurn(kTurnLeft) ? kTurnLeft : kNoTurn;
	} else if (_canRotate && boot->rightNodeHotspot.contains(place)) {
		return canTurn(kTurnRight) ? kTurnRight : kNoTurn;
	} else if (boot->uturnNodeHotspot.contains(place)) {
		return _hasUturn ? kTurnAround : kNoTurn;
	}

	return kNoTurn;
}

// Turning left goes to the next frame, and right to the one before. A node that
// doesn't wrap stops at its first and last frame.
bool SummaryRecord::canTurn(Turn turn) const {
	if (_wraps) {
		return true;
	}

	return turn == kTurnLeft ? _frame + 1 < _numFrames : _frame > 0;
}

void SummaryRecord::onMouseMove(const Common::Point32 &point, bool inViewport) {
	// The view turns by itself while the mouse stays on a side, after a first wait
	Turn turn = getTurnAt(point, inViewport);
	if (turn == kTurnAround) {
		turn = kNoTurn;
	}

	if (turn != kNoTurn && turn != _turn) {
		_lastTurnTime = _sceneTime;
	}

	_turn = turn;
}

bool SummaryRecord::getCursor(const Common::Point32 &point, bool inViewport, uint &cursorType, bool &isHot) {
	const ScriptBootData *boot = GetEngineData(ScriptBootData);
	assert(boot);

	int type = -1;
	switch (getTurnAt(point, inViewport)) {
	case kTurnLeft:
		type = g_nancy->_cursor->getCursorTypeByName(boot->leftNodeCursorName);
		break;
	case kTurnRight:
		type = g_nancy->_cursor->getCursorTypeByName(boot->rightNodeCursorName);
		break;
	case kTurnAround:
		type = g_nancy->_cursor->getCursorTypeByName(boot->uturnNodeCursorName);
		break;
	default:
		break;
	}

	if (type < 0) {
		return false;
	}

	cursorType = type;
	isHot = false;
	return true;
}

bool SummaryRecord::onMouseEvent(RecordEvent event, const Common::Point32 &point, bool inViewport) {
	switch (event) {
	case kEventRightDown:
		_isTurningFast = true;
		return false;
	case kEventRightUp:
		_isTurningFast = false;
		return false;
	case kEventLeftClick:
		break;
	default:
		return false;
	}

	switch (getTurnAt(point, inViewport)) {
	case kTurnLeft:
	case kTurnRight:
		// A click doesn't wait for the turn
		_lastTurnTime = _sceneTime - 0.5;
		return true;
	case kTurnAround:
		setFrame((_frame + _numFrames / 2) % _numFrames);
		return true;
	default:
		return false;
	}
}

#ifdef ENABLE_NANCY_V5
bool SummaryRecord::turnByKey(Common::KeyCode key) {
	if (_numFrames <= 1 || !_canRotate) {
		return false;
	}

	switch (key) {
	case Common::KEYCODE_LEFT:
		if (!canTurn(kTurnLeft)) {
			return false;
		}

		setFrame((_frame + 1) % _numFrames);
		return true;
	case Common::KEYCODE_RIGHT:
		if (!canTurn(kTurnRight)) {
			return false;
		}

		setFrame((_frame + _numFrames - 1) % _numFrames);
		return true;
	case Common::KEYCODE_DOWN:
		if (!_hasUturn) {
			return false;
		}

		setFrame((_frame + _numFrames / 2) % _numFrames);
		return true;
	default:
		return false;
	}
}
#endif

void SummaryRecord::run(double sceneTime, double elapsedTime) {
	_sceneTime = sceneTime;
	updateTheme(sceneTime);

	if (_numFrames <= 1 || _turn == kNoTurn || !_canRotate) {
		return;
	}

	if (!canTurn(_turn)) {
		return;
	}

	if (_lastTurnTime + 0.5 < sceneTime || (_isTurningFast && _lastTurnTime + 0.2 < sceneTime)) {
		if (_turn == kTurnLeft) {
			setFrame((_frame + 1) % _numFrames);
		} else {
			setFrame((_frame + _numFrames - 1) % _numFrames);
		}
	}
}

bool SummaryRecord::getField(const Common::String &key) {
	if (key == "frame") {
		lua_pushinteger(getLua(), _frame);
		return true;
	} else if (key == "enableRotate") {
		lua_pushboolean(getLua(), _canRotate);
		return true;
	} else if (key == "enableUturn") {
		lua_pushboolean(getLua(), _hasUturn);
		return true;
	}

	return false;
}

// Nancy24 scripts stop the player from turning while something is going on
bool SummaryRecord::setField(const Common::String &key) {
	if (key == "enableRotate") {
		_canRotate = lua_toboolean(getLua(), 3) != 0;
		return true;
	} else if (key == "enableUturn") {
		_hasUturn = lua_toboolean(getLua(), 3) != 0;
		return true;
	}

	return false;
}

// NavLogic

void NavLogicRecord::readScene(int valueIndex) {
	lua_State *L = getLua();

	// Scenes that go by number are named after it
	if (lua_type(L, valueIndex) == LUA_TNUMBER) {
		_sceneName = Common::String::format("s%d", (int)lua_tointeger(L, valueIndex));
	} else if (lua_isnil(L, valueIndex)) {
		_sceneName.clear();
	} else {
		_sceneName = luaL_checkstring(L, valueIndex);
	}
}

void NavLogicRecord::readCursor(int valueIndex) {
	_hasCursor = testCursor(getLua(), valueIndex, _cursorType, _cursorIsHot);
}

void NavLogicRecord::readRef(int valueIndex) {
	lua_State *L = getLua();

	_ref = lua_isnil(L, valueIndex) ? nullptr : getContext().loadRef(luaL_checkstring(L, valueIndex));
}

void NavLogicRecord::readTable(int tableIndex) {
	lua_State *L = getLua();

	if (pushField(L, tableIndex, "scene")) {
		readScene(-1);
		lua_pop(L, 1);
	}

	if (pushField(L, tableIndex, "cursor")) {
		readCursor(-1);
		lua_pop(L, 1);
	}

	readField(L, tableIndex, "frame", _frame);
	readField(L, tableIndex, "stream", _streamName);

	if (pushField(L, tableIndex, "onScreen")) {
		_hotspot = checkRect(L, -1);
		lua_pop(L, 1);
	}

	if (pushField(L, tableIndex, "ref")) {
		readRef(-1);
		lua_pop(L, 1);
	}

	if (pushField(L, tableIndex, "refColor")) {
		_refColor = checkColor(L, -1);
		_hasRefColor = true;
		lua_pop(L, 1);
	}

	if (pushField(L, tableIndex, "source")) {
		_source = checkRect(L, -1);
		lua_pop(L, 1);
	}

	readField(L, tableIndex, "rightClick", _isRightClick);

	// Nancy22: a hotspot can act like a button, which goes off when the button
	// of the mouse that went down over it comes up over it again
	Common::String mouseMode;
	if (readField(L, tableIndex, "mouseMode", mouseMode)) {
		if (mouseMode == "OnClick") {
			_isPressedAndReleased = false;
		} else if (mouseMode == "OnDownAndUp") {
			_isPressedAndReleased = true;
		} else {
			luaL_error(L, "Mouse mode set to unrecognized value '%s'", mouseMode.c_str());
		}
	}

	// The flags to set, by name
	if (pushField(L, tableIndex, "flags")) {
		luaL_checktype(L, -1, LUA_TTABLE);
		int flags = lua_gettop(L);

		lua_pushnil(L);
		while (lua_next(L, flags)) {
			if (lua_type(L, -2) == LUA_TSTRING) {
				Flag flag;
				flag.name = lua_tostring(L, -2);
				flag.value = lua_toboolean(L, -1) != 0;
				_flags.push_back(flag);
			}

			lua_pop(L, 1);
		}

		lua_pop(L, 1);
	}
}

void NavLogicRecord::run(double sceneTime, double elapsedTime) {
	// Without a place to click, there is nothing to wait for
	if (_hotspot.isEmpty() && !_ref) {
		trigger();
	}
}

// The mouse is taken back to where the hotspot was before the transform, so a
// turned hotspot is turned with its picture
bool NavLogicRecord::isOver(const Common::Point32 &point) const {
	double x = point.x;
	double y = point.y;
	getTransform().inverse().transformPoint(x, y);

	int refX = (int)x;
	int refY = (int)y;
	if (!_ref) {
		return _hotspot.contains(refX, refY);
	}

	if (!_source.isEmpty() || !_hotspot.isEmpty()) {
		if (!_hotspot.contains(refX, refY)) {
			return false;
		}

		refX += _source.left - _hotspot.left;
		refY += _source.top - _hotspot.top;
	}

	uint32 pixel;
	if (!_ref->getPixel(refX, refY, pixel)) {
		return false;
	}

	return _hasRefColor ? pixel == _refColor : (pixel >> 24) != 0;
}

// A hotspot that is held down tells its scripts when the mouse leaves it and
// comes back, with OnUp and OnDown
void NavLogicRecord::onMouseMove(const Common::Point32 &point, bool inViewport) {
	const bool wasMouseOver = _isMouseOver;
	_isMouseOver = inViewport && isOver(point);

	if (_isDown && _isMouseOver && !wasMouseOver) {
		callFunction("OnDown");
	} else if (_isDown && !_isMouseOver && wasMouseOver) {
		callFunction("OnUp");
	}
}

bool NavLogicRecord::getCursor(const Common::Point32 &point, bool inViewport, uint &cursorType, bool &isHot) {
	if (!_hasCursor || !isOver(point)) {
		return false;
	}

	cursorType = _cursorType;
	isHot = _cursorIsHot;
	return true;
}

bool NavLogicRecord::onMouseEvent(RecordEvent event, const Common::Point32 &point, bool inViewport) {
	if (_isPressedAndReleased) {
		const RecordEvent downEvent = _isRightClick ? kEventRightDown : kEventLeftDown;
		const RecordEvent upEvent = _isRightClick ? kEventRightUp : kEventLeftUp;

		if (event == downEvent && _isMouseOver) {
			callFunction("OnDown");
			_isDown = true;
		} else if (event == upEvent) {
			const bool wasDown = _isDown;
			_isDown = false;
			if (wasDown && _isMouseOver) {
				trigger();
			}
		}

		return false;
	}

	const RecordEvent clickEvent = _isRightClick ? kEventRightClick : kEventLeftClick;
	if (event != clickEvent || !isOver(point)) {
		return false;
	}

	trigger();
	return true;
}

bool NavLogicRecord::prepareReset() {
	_isDown = false;
	return true;
}

void NavLogicRecord::trigger() {
	for (const Flag &flag : _flags) {
		if (!setFlagByName(flag.name, flag.value)) {
			warning("Attempted to set flag \"%s\" but it does not exist.", flag.name.c_str());
		}
	}

	if (!_sceneName.empty()) {
		getContext().changeScene(_streamName, _sceneName, (uint16)_frame);
	}

	finish();
}

bool NavLogicRecord::getField(const Common::String &key) {
	lua_State *L = getLua();

	if (key == "scene") {
		lua_pushstring(L, _sceneName.c_str());
	} else if (key == "stream") {
		lua_pushstring(L, _streamName.c_str());
	} else if (key == "frame") {
		lua_pushinteger(L, _frame);
	} else if (key == "onScreen") {
		pushRect(L, _hotspot);
	} else if (key == "mouseOver") {
		lua_pushboolean(L, _isMouseOver);
	} else if (key == "down") {
		lua_pushboolean(L, _isDown);
	} else if (key == "refColor" && _hasRefColor) {
		pushColor(L, _refColor);
	} else if (key == "source") {
		pushRect(L, _source);
	} else if (key == "rightClick") {
		lua_pushboolean(L, _isRightClick);
	} else {
		return false;
	}

	return true;
}

bool NavLogicRecord::setField(const Common::String &key) {
	lua_State *L = getLua();

	if (key == "scene") {
		readScene(3);
	} else if (key == "stream") {
		_streamName = luaL_checkstring(L, 3);
	} else if (key == "cursor") {
		readCursor(3);
	} else if (key == "frame") {
		_frame = luaL_checkint(L, 3);
	} else if (key == "onScreen") {
		_hotspot = checkRect(L, 3);
	} else if (key == "ref") {
		readRef(3);
	} else if (key == "refColor") {
		_hasRefColor = !lua_isnil(L, 3);
		_refColor = _hasRefColor ? checkColor(L, 3) : 0;
	} else if (key == "source") {
		_source = checkRect(L, 3);
	} else if (key == "rightClick") {
		_isRightClick = lua_toboolean(L, 3) != 0;
	} else {
		return false;
	}

	return true;
}

// Override

// What the engine calls in an override, all of which have to be functions
static const char *const kOverrideFunctionNames[] = {
	"OnReset", "OnActivation", "OnDeactivation", "OnDone", "OnMouseMove", "ChangeCursor",
	"OnLeftClick", "OnRightClick", "OnMouseScroll", "OnLeftDown", "OnLeftUp", "OnRightDown",
	"OnRightUp", "OnKey", "Run", "RunOnce", "Render", "OnSave", "ReceiveMessage"
};

static bool isOverrideFunctionName(const Common::String &name) {
	for (const char *functionName : kOverrideFunctionNames) {
		if (name == functionName) {
			return true;
		}
	}

	return false;
}

OverrideRecord::~OverrideRecord() {
	setMessagingName(Common::String(), true);
	delete _canvas;
}

bool OverrideRecord::setMessagingName(const Common::String &name, bool takeOver) {
	if (name.equalsIgnoreCase(_messagingName) && !name.empty()) {
		return true;
	}

	NancyEngine::MessagingNameMap &names = g_nancy->_messagingNames;

	if (!_messagingName.empty()) {
		NancyEngine::MessagingNameMap::iterator it = names.find(_messagingName);
		if (it != names.end() && it->_value == this) {
			names.erase(it);
		}
	}

	_messagingName.clear();

	if (name.empty()) {
		return true;
	}

	if (!takeOver && names.contains(name)) {
		warning("Name '%s' already registered for messaging.", name.c_str());
		return false;
	}

	names.setVal(name, this);
	_messagingName = name;
	return true;
}

void OverrideRecord::readTable(int tableIndex) {
	lua_State *L = getLua();

	for (const char *functionName : kOverrideFunctionNames) {
		lua_getfield(L, tableIndex, functionName);
		if (!lua_isnil(L, -1) && !lua_isfunction(L, -1)) {
			luaL_error(L, "%s must be set to a function", functionName);
		}

		lua_pop(L, 1);
	}
}

bool OverrideRecord::hasMessagingName(const Common::String &name) {
	const NancyEngine::MessagingNameMap &names = g_nancy->_messagingNames;
	NancyEngine::MessagingNameMap::const_iterator it = names.find(name);
	if (it != names.end()) {
		return it->_value == this;
	}

	lua_State *L = getLua();

	pushStoredValue("messagingName");
	const bool isNamed = lua_type(L, -1) == LUA_TSTRING && name.equalsIgnoreCase(lua_tostring(L, -1));
	lua_pop(L, 1);
	return isNamed;
}

void OverrideRecord::registerGraphics() {
	if (_canvas) {
		_canvas->registerGraphics();
	}
}

void OverrideRecord::onZChanged() {
	if (_canvas) {
		_canvas->setScriptZ(getZ(), getContext());
	}
}

// An override draws by having its "Render" function called every frame, which
// draws through the "Draw" object. Here that goes onto a picture as large as
// the viewport, which is started over every frame. The picture is only shown
// again when it changed: the menus darken all the screen behind them, and
// drawing all of it every frame made them slow.
void OverrideRecord::updateGraphics() {
	lua_State *L = getLua();

	pushStoredValue("Render");
	const bool hasRender = lua_isfunction(L, -1);
	lua_pop(L, 1);

	if (!hasRender || !isShown()) {
		if (_canvas) {
			_canvas->setVisible(false, false);
		}

		return;
	}

	if (!_canvas) {
		const Common::Rect bounds = RecordGraphic::getDrawArea();

		_canvas = new RecordGraphic();
		_canvas->init();
		_canvas->_drawSurface.create(bounds.width(), bounds.height(), g_nancy->_graphics->getTransparentPixelFormat());
		_canvas->_drawSurface.clear(0);
		_canvas->moveTo(bounds);
		_canvas->setScriptZ(getZ(), getContext());
		_canvas->registerGraphics();
		_renderSurface.create(bounds.width(), bounds.height(), _canvas->_drawSurface.format);
	}

	_renderSurface.clear(0);

	// From Nancy26 on, a picture the function takes shows what is below it
	const bool isDrawnInPlace = g_nancy->getGameType() >= kGameTypeNancy26;
	if (isDrawnInPlace) {
		g_nancy->_graphics->beginScriptDrawing(_canvas->getZOrder());
	}

	s_drawTarget = &_renderSurface;
	callFunction("Render");
	s_drawTarget = nullptr;

	if (isDrawnInPlace) {
		g_nancy->_graphics->endScriptDrawing();
	}

	Graphics::ManagedSurface &shown = _canvas->_drawSurface;
	const uint32 size = shown.pitch * shown.h;
	if (memcmp(shown.getPixels(), _renderSurface.getPixels(), size) != 0) {
		memcpy(shown.getPixels(), _renderSurface.getPixels(), size);
		_canvas->setNeedsRedraw(true);
	}

	_canvas->setVisible(true, false);
}

// this:Done()
static int overrideDone(lua_State *L) {
	checkRecord(L, 1)->finish();
	return 0;
}

// this:SetMessagingName(name[, takeOver]), Nancy22: the name the record gets
// messages under from now on; an empty one for none. A name that another record
// has stays with it unless the record takes it over. Gives whether it got the name.
static int overrideSetMessagingName(lua_State *L) {
	OverrideRecord *record = dynamic_cast<OverrideRecord *>(checkRecord(L, 1));
	const char *name = luaL_checkstring(L, 2);
	const bool takeOver = lua_gettop(L) > 2 && lua_toboolean(L, 3);

	lua_pushboolean(L, record && record->setMessagingName(name, takeOver));
	return 1;
}

bool OverrideRecord::getField(const Common::String &key) {
	if (key == "Done") {
		lua_pushcfunction(getLua(), overrideDone);
		return true;
	}

	// Nancy22
	if (key == "SetMessagingName") {
		lua_pushcfunction(getLua(), overrideSetMessagingName);
		return true;
	}

#ifdef ENABLE_NANCY_V5
	if (g_nancy->getGameType() >= kGameTypeNancy26) {
		return pushRecordDrawFunction(getLua(), key);
	}
#endif

	return false;
}

bool OverrideRecord::setField(const Common::String &key) {
	lua_State *L = getLua();

	if (isOverrideFunctionName(key) && !lua_isnil(L, 3) && !lua_isfunction(L, 3)) {
		luaL_error(L, "%s must be set to a function or to nil", key.c_str());
	}

	// Kept with everything else scripts store in the record
	return false;
}

// Overlay

// The "LOADSAVE" tree holds the pictures the camera took, which are kept with
// the saved game
static const char *const kPictureTreeName = "LOADSAVE";

// Nancy26: the "SaveFile" tree holds the pictures of the saved games, by their
// names. They are the thumbnails of ScummVM's saved games, at the size the save
// and load screens show them at.
static const char *const kSaveFileTreeName = "SaveFile";
static const int kSavePictureWidth = 184;
static const int kSavePictureHeight = 124;

static bool loadSavePicture(const Common::String &name, Graphics::ManagedSurface &image) {
	const MetaEngine *metaEngine = g_nancy->getMetaEngine();
	const Common::String target = ConfMan.getActiveDomainName();
	const SaveStateList saves = metaEngine->listSaves(target.c_str());
	for (const SaveStateDescriptor &save : saves) {
		if (save.getSaveSlot() == metaEngine->getMaximumSaveSlot() || save.getSaveSlot() == g_nancy->getAutosaveSlot() ||
				!save.getDescription().equalsIgnoreCase(name)) {
			continue;
		}

		const SaveStateDescriptor info = metaEngine->querySaveMetaInfos(target.c_str(), save.getSaveSlot());
		const Graphics::Surface *thumbnail = info.getThumbnail();
		if (!thumbnail) {
			return false;
		}

		Graphics::Surface *scaled = thumbnail->scale(kSavePictureWidth, kSavePictureHeight, true);
		Graphics::Surface *converted = scaled->convertTo(g_nancy->_graphics->getInputPixelFormat(32));
		GraphicsManager::copyToManaged(*converted, image);
		converted->free();
		delete converted;
		scaled->free();
		delete scaled;
		return true;
	}

	warning("There is no saved game named %s", name.c_str());
	return false;
}

bool loadTreeImage(const Common::String &name, const Common::String &treeName, Graphics::ManagedSurface &image) {
	if (treeName.equalsIgnoreCase(kSaveFileTreeName)) {
		return loadSavePicture(name, image);
	}

	if (!treeName.equalsIgnoreCase(kPictureTreeName)) {
		return g_nancy->_resource->loadImage(Common::Path(name), image, treeName);
	}

#ifdef USE_PNG
	const ScriptPictureData *pictures = (const ScriptPictureData *)NancySceneState.getPuzzleData(ScriptPictureData::getTag());
	if (!pictures->pictures.contains(name)) {
		warning("Could not find picture %s amongst the saved ones", name.c_str());
		return false;
	}

	const Common::Array<byte> &data = pictures->pictures[name];
	Common::MemoryReadStream stream(data.data(), data.size());
	Image::PNGDecoder decoder;
	if (!decoder.loadStream(stream)) {
		return false;
	}

	Graphics::Surface *converted = decoder.getSurface()->convertTo(g_nancy->_graphics->getInputPixelFormat(32));
	GraphicsManager::copyToManaged(*converted, image);
	converted->free();
	delete converted;
	return true;
#else
	return false;
#endif
}

// Overlays get their pictures from the image cache of the resource manager,
// and hold on to the cached picture while they use it. Saved pictures aren't
// cached: every record has its own.
static bool loadCachedImage(const Common::String &name, const Common::String &treeName, Common::SharedPtr<Graphics::ManagedSurface> &cached, Graphics::ManagedSurface &image) {
	if (treeName.equalsIgnoreCase(kPictureTreeName) || treeName.equalsIgnoreCase(kSaveFileTreeName)) {
		image.free();
		cached.reset();
		return loadTreeImage(name, treeName, image);
	}

	cached = g_nancy->_resource->loadCachedImage(name, treeName);
	if (!cached) {
		return false;
	}

	// The record draws from the cached picture, without a copy of its own
	image.create(*cached, Common::Rect(cached->w, cached->h));
	return true;
}

void OverlayRecord::readTable(int tableIndex) {
	lua_State *L = getLua();

	Common::String imageName;
	if (!readField(L, tableIndex, "ovl", imageName)) {
		luaL_error(L, "OverlayAR must be created with a ovl parameter");
	}

	// Nancy21 can name the tree the image is in
	Common::String treeName;
	readField(L, tableIndex, "tree", treeName);

	if (!loadCachedImage(imageName, treeName, _cachedImage, _image)) {
		luaL_error(L, "Could not load overlay %s", imageName.c_str());
	}

	_imageName = imageName;
	_treeName = treeName;

	// Both default to the whole image
	_source = ScriptRect(0, 0, _image.w, _image.h);
	if (pushField(L, tableIndex, "source")) {
		_source = checkRect(L, -1);
		lua_pop(L, 1);
	}

	_dest = _source;
	if (pushField(L, tableIndex, "onScreen")) {
		readDest(-1);
		lua_pop(L, 1);
	}

	if (pushField(L, tableIndex, "modColor")) {
		_modColor = checkColor(L, -1);
		lua_pop(L, 1);
	}

	_graphic.init();
	_graphic.setVisible(false);
	applyRects();
	onZChanged();
	_graphic.registerGraphics();
}

// The onScreen of an overlay is a rect, or in Nancy24 also a table of rects;
// the picture is then shown at each of them. The scripts change the rects in
// the table as they go, so the table is kept as it is.
void OverlayRecord::readDest(int index) {
	lua_State *L = getLua();

	_hasDestTable = lua_istable(L, index);
	if (_hasDestTable) {
		lua_pushvalue(L, index);
		storeValue("onScreen");
		readDestTable(_tableDests);
	} else {
		_dest = checkRect(L, index);
	}
}

void OverlayRecord::readDestTable(Common::Array<ScriptRect> &dests) {
	lua_State *L = getLua();

	dests.clear();
	pushStoredValue("onScreen");
	if (lua_istable(L, -1)) {
		lua_pushnil(L);
		while (lua_next(L, -2)) {
			const ScriptRect *dest = testRect(L, -1);
			if (dest) {
				dests.push_back(*dest);
			}

			lua_pop(L, 1);
		}
	}

	lua_pop(L, 1);
}

// A hidden picture gets placed once it shows again. The map of Nancy25 moves
// many hidden ones with its camera.
void OverlayRecord::onTransformChanged() {
	if (isActive()) {
		applyRects();
	} else {
		_isPlacementStale = true;
	}
}

void OverlayRecord::applyRects() {
	_isPlacementStale = false;

	Common::Rect source = _source.toRect();
	source.clip(Common::Rect(_image.w, _image.h));

	// A script can leave the picture out until it knows what to show
	_hasPicture = !source.isEmpty() && (!_hasDestTable || !_tableDests.empty());
	if (!_hasPicture) {
		return;
	}

	const uint32 modColor = takeModColor(_modColor);
	if (!_hasDestTable) {
		placePicture(_graphic, _image, source, _dest, getTransform(), modColor);
		return;
	}

	// The copies of the picture are drawn into one surface that covers them all
	const ScriptMatrix transform = getTransform();
	const bool isKeyed = transform.isTranslation() && modColor == kNoModColor;
	Common::Array<RecordGraphic> pictures(_tableDests.size());
	Common::Rect area;
	for (uint i = 0; i < _tableDests.size(); ++i) {
		placePicture(pictures[i], _image, source, _tableDests[i], transform, modColor);
		if (i == 0) {
			area = pictures[i].getScreenPositionRaw();
		} else {
			area.extend(pictures[i].getScreenPositionRaw());
		}
	}

	Graphics::ManagedSurface &out = _graphic._drawSurface;
	out.create(MAX<int>(area.width(), 1), MAX<int>(area.height(), 1), g_nancy->_graphics->getTransparentPixelFormat());
	out.clear(0);
	_graphic.setTransparent(false);
	for (const RecordGraphic &picture : pictures) {
		blendPicture(out, picture, isKeyed, Common::Point(area.left, area.top));
	}

	_graphic.moveTo(area);
	_graphic.setNeedsRedraw(true);
}

void OverlayRecord::onZChanged() {
	_graphic.setScriptZ(getZ(), getContext());
}

void OverlayRecord::registerGraphics() {
	_graphic.registerGraphics();
}

void OverlayRecord::updateGraphics() {
	if (_hasDestTable && isShown()) {
		Common::Array<ScriptRect> dests;
		readDestTable(dests);
		if (dests != _tableDests) {
			_tableDests = dests;
			applyRects();
		}
	}

	if ((_isPlacementStale || hasAlphaChanged()) && isShown()) {
		applyRects();
	}

	_graphic.setClipRect(getScreenClip());
	_graphic.setVisible(isShown() && _hasPicture, false);

	if (_hasAttachment) {
		_attachment.setVisible(_isAttachmentDrawn, false);
		_isAttachmentDrawn = false;
	}
}

// What a script draws by itself goes over everything else the scene shows
static const uint16 kAttachmentZ = 59999;

// TODO: The picture is cut off at the edges of the viewport
void OverlayRecord::renderNow() {
	Common::Rect source = _source.toRect();
	source.clip(Common::Rect(_image.w, _image.h));
	if (source.isEmpty()) {
		return;
	}

	if (!_hasAttachment) {
		_hasAttachment = true;
		_attachment.init();
		_attachment.setVisible(false);
		_attachment.setZOrder(kAttachmentZ);
		_attachment.registerGraphics();
	}

	// The transform places the picture on the screen, and the records draw
	// in the viewport
	const VIEW *viewport = GetEngineData(VIEW);
	assert(viewport);

	ScriptMatrix transform = getTransform();
	transform.m[3][0] -= viewport->screenPosition.left;
	transform.m[3][1] -= viewport->screenPosition.top;

	placePicture(_attachment, _image, source, _dest, transform, _modColor);
	_isAttachmentDrawn = true;
}

// Draws the picture of the overlay once more. The source, onScreen, transform
// and modColor in the table of Draw:OVL replace the overlay's own.
void OverlayRecord::drawCopy(int tableIndex) {
	lua_State *L = getLua();

	ScriptRect source = _source;
	ScriptRect dest = _dest;
	ScriptMatrix transform = getTransform();
	uint32 modColor = _modColor;

	if (pushField(L, tableIndex, "source")) {
		source = checkRect(L, -1);
		lua_pop(L, 1);
	}

	if (pushField(L, tableIndex, "onScreen")) {
		dest = checkRect(L, -1);
		lua_pop(L, 1);
	}

	// A transform from the table is still combined with those of the nodes
	// the overlay is attached to
	if (pushField(L, tableIndex, "transform")) {
		transform = checkMatrix(L, -1);
		lua_pop(L, 1);

		if (getParentNode()) {
			transform = transform * getParentNode()->getTransform();
		}
	}

	if (pushField(L, tableIndex, "modColor")) {
		modColor = checkColor(L, -1);
		lua_pop(L, 1);
	}

	Graphics::ManagedSurface *target = getDrawTarget();
	if (!target) {
		luaL_error(L, "Draw can only be used while a record is rendering");
	}

	Common::Rect from = source.toRect();
	from.clip(Common::Rect(_image.w, _image.h));
	if (from.isEmpty()) {
		return;
	}

	// The target canvas covers the viewport, and record positions are
	// already relative to the viewport
	RecordGraphic picture;
	placePicture(picture, _image, from, dest, transform, modColor);
	blendPicture(*target, picture, transform.isTranslation() && modColor == kNoModColor, Common::Point());
}

// Draw:OVL(table), Nancy24: table.ar is the Overlay record to draw
int drawOverlay(lua_State *L) {
	luaL_checktype(L, 2, LUA_TTABLE);

	lua_getfield(L, 2, "ar");
	if (lua_isnil(L, -1)) {
		return luaL_argerror(L, 2, "Reference to existing overlay AR must be provided (ar field is nil)");
	}

	OverlayRecord *overlay = dynamic_cast<OverlayRecord *>(checkRecord(L, -1));
	if (!overlay) {
		return luaL_typerror(L, -1, "ND.Overlay");
	}

	lua_pop(L, 1);
	overlay->drawCopy(2);
	return 0;
}

bool OverlayRecord::getField(const Common::String &key) {
	lua_State *L = getLua();

	if (key == "onScreen") {
		if (_hasDestTable) {
			pushStoredValue("onScreen");
		} else {
			pushRect(L, _dest);
		}
	} else if (key == "source") {
		pushRect(L, _source);
	} else if (key == "modColor") {
		pushColor(L, _modColor);
	} else if (key == "ovl") {
		lua_pushstring(L, _imageName.c_str());
	} else if (key == "tree") {
		lua_pushstring(L, _treeName.c_str());
	} else {
		return false;
	}

	return true;
}

// The scripts change the picture by its name, after naming the tree it is in:
// the save and load screens show the pictures of the saved games this way. The
// part of it that shows stays as it was.
bool OverlayRecord::setPicture(const Common::String &imageName) {
	Common::SharedPtr<Graphics::ManagedSurface> cached;
	Graphics::ManagedSurface image;
	if (!loadCachedImage(imageName, _treeName, cached, image)) {
		warning("Could not load overlay %s", imageName.c_str());
		return false;
	}

	_image = Common::move(image);
	_cachedImage = cached;
	_imageName = imageName;
	return true;
}

bool OverlayRecord::setField(const Common::String &key) {
	lua_State *L = getLua();

	if (key == "tree") {
		_treeName = lua_isnil(L, 3) ? Common::String() : Common::String(luaL_checkstring(L, 3));
		return true;
	}

	// Only a change gets the picture drawn anew
	if (key == "ovl") {
		const Common::String imageName = luaL_checkstring(L, 3);
		if (!setPicture(imageName)) {
			return true;
		}
	} else if (key == "onScreen") {
		if (lua_istable(L, 3) || _hasDestTable) {
			readDest(3);
		} else {
			const ScriptRect dest = checkRect(L, 3);
			if (dest == _dest) {
				return true;
			}

			_dest = dest;
		}
	} else if (key == "source") {
		const ScriptRect source = checkRect(L, 3);
		if (source == _source) {
			return true;
		}

		_source = source;
	} else if (key == "modColor") {
		const uint32 modColor = checkColor(L, 3);
		if (modColor == _modColor) {
			return true;
		}

		_modColor = modColor;
	} else {
		return false;
	}

	applyRects();
	return true;
}

// AnimOverlay

void AnimOverlayRecord::readTable(int tableIndex) {
	lua_State *L = getLua();

	readField(L, tableIndex, "firstFrame", _firstFrame);
	readField(L, tableIndex, "lastFrame", _lastFrame);
	readField(L, tableIndex, "fps", _framesPerSecond);
	readField(L, tableIndex, "pauseOnLastFrame", _pausesOnLastFrame);
	readField(L, tableIndex, "visible", _isVisible);

	if (pushField(L, tableIndex, "modColor")) {
		_modColor = checkColor(L, -1);
		lua_pop(L, 1);
	}

	if (!pushField(L, tableIndex, "onScreen")) {
		luaL_error(L, "AnimOverlays must have a onScreen field");
	}

	_dest = checkRect(L, -1);
	lua_pop(L, 1);

	// Either whether to loop forever, or how many times to play
	if (pushField(L, tableIndex, "loop")) {
		if (lua_isboolean(L, -1)) {
			_numLoops = lua_toboolean(L, -1) ? 0 : 1;
		} else if (lua_isnumber(L, -1)) {
			_numLoops = (int)lua_tointeger(L, -1);
		} else {
			luaL_error(L, "loop must be either a boolean or a number");
		}

		lua_pop(L, 1);
	}

	if (!pushField(L, tableIndex, "sources")) {
		luaL_error(L, "AnimOverlays must have a sources field");
	}

	luaL_checktype(L, -1, LUA_TTABLE);
	for (int i = 1; ; ++i) {
		lua_rawgeti(L, -1, i);
		if (lua_isnil(L, -1)) {
			lua_pop(L, 1);
			break;
		}

		_sources.push_back(checkRect(L, -1));
		lua_pop(L, 1);
	}

	lua_pop(L, 1);

	if (_sources.empty()) {
		luaL_error(L, "Field sources must be a table with Rects in it");
	}

	Common::String imageName;
	if (!readField(L, tableIndex, "ovl", imageName)) {
		luaL_error(L, "AnimOverlays must have a ovl field");
	}

	// Nancy21 can name the tree the image is in
	Common::String treeName;
	readField(L, tableIndex, "tree", treeName);

	if (!loadCachedImage(imageName, treeName, _cachedImage, _image)) {
		luaL_error(L, "Could not load overlay %s", imageName.c_str());
	}

	_graphic.init();
	_graphic.setVisible(false);
	setFrame(getFirstFrame());
	onZChanged();
	_graphic.registerGraphics();
}

void AnimOverlayRecord::setFrame(int frame) {
	_frame = CLIP<int>(frame, 1, _sources.size());
	applyFrame();
}

void AnimOverlayRecord::applyFrame() {
	Common::Rect source = _sources[_frame - 1].toRect();
	source.clip(Common::Rect(_image.w, _image.h));

	_hasPicture = !source.isEmpty();
	if (!_hasPicture) {
		return;
	}

	placePicture(_graphic, _image, source, _dest, getTransform(), takeModColor(_modColor));
}

// Starts over from the first frame
void AnimOverlayRecord::beginLoop() {
	_progress = 0.0;
	setFrame(getFirstFrame());

	const int numSteps = getLastFrame() - getFirstFrame();
	_endTime = getContext().getTime() + (_framesPerSecond > 0.0 ? numSteps / _framesPerSecond : 0.0);
}

void AnimOverlayRecord::runOnce() {
	_numLoopsDone = 0;
	beginLoop();
}

void AnimOverlayRecord::run(double sceneTime, double elapsedTime) {
	if (_isPaused) {
		return;
	}

	const int numSteps = getLastFrame() - getFirstFrame();
	if (numSteps > 0) {
		_progress = CLIP(_progress + _framesPerSecond * elapsedTime / numSteps, 0.0, 1.0);
		setFrame(getFirstFrame() + (int)floor(_progress * numSteps));
	}

	if (_endTime >= sceneTime) {
		return;
	}

	++_numLoopsDone;
	if (_numLoops != 0 && _numLoopsDone >= _numLoops) {
		if (_pausesOnLastFrame) {
			_isPaused = true;
			callFunction("OnPause");
		} else {
			finish();
		}
	} else {
		beginLoop();
	}
}

bool AnimOverlayRecord::prepareReset() {
	setFrame(getFirstFrame());
	_isPaused = false;
	return true;
}

void AnimOverlayRecord::onZChanged() {
	_graphic.setScriptZ(getZ(), getContext());
}

void AnimOverlayRecord::registerGraphics() {
	_graphic.registerGraphics();
}

void AnimOverlayRecord::updateGraphics() {
	if (hasAlphaChanged() && isShown() && _hasPicture) {
		applyFrame();
	}

	_graphic.setClipRect(getScreenClip());
	_graphic.setVisible(isShown() && _isVisible && _hasPicture, false);
}

// anim:Play() and anim:Pause(), for the records that play something
static int animOverlayPlay(lua_State *L) {
	AnimOverlayRecord *record = dynamic_cast<AnimOverlayRecord *>(checkRecord(L, 1));
	if (record) {
		record->setPaused(false);
	}

	return 0;
}

static int animOverlayPause(lua_State *L) {
	AnimOverlayRecord *record = dynamic_cast<AnimOverlayRecord *>(checkRecord(L, 1));
	if (record) {
		record->setPaused(true);
	}

	return 0;
}

bool AnimOverlayRecord::getField(const Common::String &key) {
	lua_State *L = getLua();

	if (key == "onScreen") {
		pushRect(L, _dest);
	} else if (key == "Play") {
		lua_pushcfunction(L, animOverlayPlay);
	} else if (key == "Pause") {
		lua_pushcfunction(L, animOverlayPause);
	} else if (key == "paused") {
		lua_pushboolean(L, _isPaused);
	} else if (key == "pauseOnLastFrame") {
		lua_pushboolean(L, _pausesOnLastFrame);
	} else if (key == "visible") {
		lua_pushboolean(L, _isVisible);
	} else if (key == "frame") {
		lua_pushinteger(L, _frame);
	} else if (key == "endTime") {
		lua_pushnumber(L, _endTime);
	} else if (key == "firstFrame") {
		lua_pushinteger(L, getFirstFrame());
	} else if (key == "lastFrame") {
		lua_pushinteger(L, getLastFrame());
	} else if (key == "frames") {
		lua_pushinteger(L, _sources.size());
	} else if (key == "fps") {
		lua_pushnumber(L, _framesPerSecond);
	} else if (key == "source") {
		// Nancy22: the part of the image that shows now
		pushRect(L, _frame >= 1 && _frame <= (int)_sources.size() ? _sources[_frame - 1] : ScriptRect());
	} else {
		return false;
	}

	return true;
}

bool AnimOverlayRecord::setField(const Common::String &key) {
	lua_State *L = getLua();

	if (key == "onScreen") {
		const ScriptRect dest = checkRect(L, 3);
		if (dest != _dest) {
			_dest = dest;
			applyFrame();
		}
	} else if (key == "modColor") {
		const uint32 modColor = checkColor(L, 3);
		if (modColor != _modColor) {
			_modColor = modColor;
			applyFrame();
		}
	} else if (key == "pauseOnLastFrame") {
		_pausesOnLastFrame = lua_toboolean(L, 3) != 0;
	} else if (key == "visible") {
		_isVisible = lua_toboolean(L, 3) != 0;
	} else if (key == "frame") {
		setFrame(luaL_checkint(L, 3));
	} else if (key == "fps") {
		_framesPerSecond = luaL_checknumber(L, 3);
	} else {
		return false;
	}

	return true;
}

// Sound

void SoundRecord::readTable(int tableIndex) {
	lua_State *L = getLua();

	if (pushField(L, tableIndex, "channel")) {
		_channel = checkSoundChannel(L, -1);
		lua_pop(L, 1);
	}

	readField(L, tableIndex, "volume", _volume);
	readField(L, tableIndex, "loop", _isLooping);
	readField(L, tableIndex, "useCC", _usesCaption);

	lua_getfield(L, tableIndex, "channelType");
	readChannelType(-1);
	lua_pop(L, 1);

	if (pushField(L, tableIndex, "sounds")) {
		readStringList(L, -1, _names);
		lua_pop(L, 1);
	}

	if (_names.empty()) {
		luaL_error(L, "sounds must have one or more fields in its table");
	}
}

SoundRecord::SoundRecord(SceneContext &context) : Record(context, "ND.SoundAR") {
	// FX1, which Nancy22 and Nancy23 have further up
	const GameType gameType = g_nancy->getGameType();
	_channel = (gameType >= kGameTypeNancy22 && gameType <= kGameTypeNancy23) ? 16 : 14;
}

void SoundRecord::readChannelType(int index) {
	lua_State *L = getLua();

	if (lua_isnoneornil(L, index)) {
		_channelType.clear();
		return;
	}

	_channelType = luaL_checkstring(L, index);
	if (_channelType != "VOICE" && _channelType != "EFFECTS" && _channelType != "MUSIC") {
		luaL_error(L, "%s was passed an invalid value", "channelType");
	}
}

// A sound without a channelType set by the script uses the type of its channel
const char *SoundRecord::getChannelType() const {
	if (!_channelType.empty()) {
		return _channelType.c_str();
	} else if (_channel >= 26 && _channel <= 29) {
		return "VOICE";
	} else if (_channel >= 3 && _channel <= 25) {
		return "EFFECTS";
	} else if (_channel <= 2) {
		return "MUSIC";
	}

	return "";
}

// A sound that is playing while a movie gets skipped is cut short
void SoundRecord::onSkip() {
	if (isActive() && hasStarted() && !_isLooping) {
		g_nancy->_sound->stopSound(_sound);
		_endTime = 0.0;
	}
}

bool SoundRecord::skipSpeech() {
	if (!isActive() || !hasStarted() || _isLooping || !_isPlaying ||
			!isVoiceChannel(_channel) ||
			!g_nancy->_sound->isSoundPlaying(_sound)) {
		return false;
	}

	// The record ends with its next run, and takes the caption with it
	g_nancy->_sound->stopSound(_sound);
	_endTime = 0.0;
	return true;
}

void SoundRecord::runOnce() {
	// A sound that would start in a movie that is being skipped is over at once,
	// unless it is one that keeps playing afterwards
	if (getContext().isSkipping() && !_isLooping) {
		_sound = SoundDescription();
		_isPlaying = false;
		_wasPreempted = false;
		_caption.clear();
		_endTime = getContext().getTime();
		return;
	}

	_sound = SoundDescription();
	_sound.name = _names[g_nancy->_randomSource->getRandomNumber(_names.size() - 1)];
	_sound.channelID = _channel;
	_sound.numLoops = _isLooping ? 0 : 1;
	_sound.volume = (uint16)CLIP<int>((int)(_volume * 100.0), 0, 100);

	g_nancy->_sound->loadSound(_sound);
	if (_channelType == "VOICE") {
		g_nancy->_sound->playSound(_channel, Audio::Mixer::kSpeechSoundType);
	} else if (_channelType == "MUSIC") {
		g_nancy->_sound->playSound(_channel, Audio::Mixer::kMusicSoundType);
	} else if (_channelType == "EFFECTS") {
		g_nancy->_sound->playSound(_channel, Audio::Mixer::kSFXSoundType);
	} else {
		g_nancy->_sound->playSound(_sound);
	}

	_isPlaying = g_nancy->_sound->isSoundPlaying(_sound);
	_wasPreempted = false;
	_channelSerial = g_nancy->_sound->getChannelSerial(_channel);

	// The record is done once the sound has played through, whether it loops or not
	_duration = _isPlaying ? g_nancy->_sound->getLength(_sound).msecs() / 1000.0 : 0.0;
	_endTime = getContext().getTime() + _duration;

#ifdef ENABLE_NANCY_V5
	if (g_nancy->getGameType() >= kGameTypeNancy26) {
		if (_isPlaying && _usesCaption) {
			sendCaption(true);
		}

		return;
	}
#endif

	// What the sound says is kept under its name
	UI::NDUI::Manager *ui = NancySceneState.getNDUI();
	_caption.clear();
	if (_isPlaying && _usesCaption && ui && findText(_sound.name, _caption)) {
		ui->setCaption(_caption);
	}
}

#ifdef ENABLE_NANCY_V5
void SoundRecord::sendCaption(bool isShown) {
	lua_State *L = getLua();
	const int top = lua_gettop(L);

	if (isShown) {
		lua_pushstring(L, _sound.name.c_str());
		sendMessage(L, getName(), "CC", "SetIfExists", top + 1, 1);
	} else {
		sendMessage(L, getName(), "CC", "Clear", top + 1, 0);
	}

	lua_settop(L, top);
	_hasSentCaption = isShown;
}
#endif

void SoundRecord::run(double sceneTime, double elapsedTime) {
	// The sound is cut off when another sound starts on its channel, or the
	// channel is stopped. Ending a little before its length doesn't count.
	if (_isPlaying && g_nancy->_sound->getChannelSerial(_channel) != _channelSerial && (sceneTime < _endTime || _isLooping)) {
		_wasPreempted = true;
	}

	// From Nancy21 on, a sound that loops keeps the record going
	const bool hasPlayedThrough = _endTime <= sceneTime && !(_isLooping && g_nancy->getGameType() >= kGameTypeNancy21);

	if (hasPlayedThrough || _wasPreempted) {
		// The caption goes with the sound, unless another one has replaced it
		UI::NDUI::Manager *ui = NancySceneState.getNDUI();
		if (!_caption.empty() && ui && ui->getCaption() == _caption) {
			ui->setCaption(Common::String());
		}

		_caption.clear();
#ifdef ENABLE_NANCY_V5
		if (_hasSentCaption) {
			sendCaption(false);
		}
#endif

		finish();
	}
}

bool SoundRecord::prepareReset() {
	_endTime = 0.0;
	_wasPreempted = false;
	_isPlaying = false;
	return true;
}

// Scripts hand a record other sounds, or move it to another channel, and then
// restart it. A record that plays stops when it moves.
bool SoundRecord::setField(const Common::String &key) {
	lua_State *L = getLua();

	if (key == "channel") {
		if (_isPlaying && hasStarted() && !isDone()) {
			g_nancy->_sound->stopSound(_sound);
		}

		_channel = checkSoundChannel(L, 3);
	} else if (key == "channelType") {
		readChannelType(3);
	} else if (key == "volume") {
		_volume = luaL_checknumber(L, 3);
	} else if (key == "useCC") {
		_usesCaption = lua_toboolean(L, 3) != 0;
	} else if (key == "loop") {
		_isLooping = lua_toboolean(L, 3) != 0;
	} else if (key == "sounds") {
		_names.clear();
		readStringList(L, 3, _names);
		if (_names.empty()) {
			luaL_error(L, "sounds must have one or more fields in its table");
		}
	} else {
		return false;
	}

	return true;
}

// sound:StopIfPlaying(), Nancy23: stops the channel, unless another sound has
// started on it since this one did
static int soundStopIfPlaying(lua_State *L) {
	((SoundRecord *)checkRecord(L, 1))->stopIfPlaying();
	return 0;
}

bool SoundRecord::getPlayingChannel(uint16 &channel) const {
	if (!_isPlaying || g_nancy->_sound->getChannelSerial(_channel) != _channelSerial ||
			!g_nancy->_sound->isSoundPlaying(_channel)) {
		return false;
	}

	channel = _channel;
	return true;
}

void SoundRecord::stopIfPlaying() {
	if (_isPlaying && g_nancy->_sound->getChannelSerial(_channel) == _channelSerial) {
		g_nancy->_sound->stopSound(_sound);
	}
}

bool SoundRecord::getField(const Common::String &key) {
	lua_State *L = getLua();

	if (key == "StopIfPlaying") {
		lua_pushcfunction(L, soundStopIfPlaying);
	} else if (key == "channel") {
		pushSoundChannel(L, _channel);
	} else if (key == "channelType") {
		lua_pushstring(L, getChannelType());
	} else if (key == "volume") {
		lua_pushnumber(L, _volume);
	} else if (key == "loop") {
		lua_pushboolean(L, _isLooping);
	} else if (key == "endTime") {
		lua_pushnumber(L, _endTime);
	} else if (key == "preempted") {
		lua_pushboolean(L, _wasPreempted);
	} else if (key == "duration") {
		lua_pushnumber(L, _duration);
	} else if (key == "sounds") {
		// A single sound comes back as its name
		if (_names.size() == 1) {
			lua_pushstring(L, _names[0].c_str());
		} else {
			lua_createtable(L, _names.size(), 0);
			for (uint i = 0; i < _names.size(); ++i) {
				lua_pushstring(L, _names[i].c_str());
				lua_rawseti(L, -2, i + 1);
			}
		}
	} else {
		return false;
	}

	return true;
}

// Text

// How quickly a text fades in; a lower rate is faster
static const float kTextBlendRate = 0.7f;

void TextRecord::readTable(int tableIndex) {
	lua_State *L = getLua();

	if (pushField(L, tableIndex, "onScreen")) {
		_dest = checkRect(L, -1);
		lua_pop(L, 1);
	}

	readField(L, tableIndex, "yOffset", _yOffset);
	readField(L, tableIndex, "yVelocity", _yVelocity);
	readField(L, tableIndex, "useAutotext", _usesAutotext);
	readField(L, tableIndex, "revealAll", _revealsAll);

	_initialDest = _dest;
	_initialYOffset = _yOffset;
	_initialYVelocity = _yVelocity;

	int fontNumber = 0;
	int colorNumber = 0;
	bool isBold = false;
	readField(L, tableIndex, "fontNum", fontNumber);
	readField(L, tableIndex, "fontColor", colorNumber);
	readField(L, tableIndex, "bold", isBold);

	// Fonts and their colors go by number
	Common::String fontName;
	uint32 color = 0xFFFFFFFF;
	const ScriptBootData *boot = (const ScriptBootData *)g_nancy->getEngineData("ScriptBootData");
	if (boot) {
		for (const ScriptBootData::SystemFont &font : boot->fonts) {
			if (font.id == fontNumber) {
				fontName = font.name;
			}
		}

		for (const ScriptBootData::FontColor &fontColor : boot->fontColors) {
			if (fontColor.id == colorNumber) {
				color = fontColor.normal;
			}
		}
	}

	Common::MemoryWriteStreamDynamic data(DisposeAfterUse::YES);
	writeTextElement(L, tableIndex, fontName, isBold, color, data);
	Common::MemoryReadStream stream(data.getData(), data.size());
	_element.read(stream);

	Common::String text;
	if (!readField(L, tableIndex, "text", text)) {
		luaL_error(L, "Text AR requires field %s", "text");
	}

	setText(text);

	_graphic.init();
	_graphic.setVisible(false);
	applyDest();
	onZChanged();
	_graphic.registerGraphics();
}

// The transform moves the text, like the scrolling lists of the UI do with theirs
void TextRecord::applyDest() {
	const Common::Rect dest = _dest.toRect();
	const int width = MAX<int>(dest.width(), 1);
	const int height = MAX<int>(dest.height(), 1);
	if (_graphic._drawSurface.w != width || _graphic._drawSurface.h != height) {
		_graphic._drawSurface.create(width, height, g_nancy->_graphics->getTransparentPixelFormat());
		_isDirty = true;
	}

	_graphic.moveTo(getTransformedDest(_dest, getTransform(), width, height));
	_graphic.setNeedsRedraw(true);
}

void TextRecord::setText(const Common::String &text) {
	_text = text;
	if (_usesAutotext && !findText(text, _text)) {
		_text = "Missing text: " + text;
	}

	_numRevealed = 0;
	_shownText = _revealsAll ? _text : Common::String();
	_isDirty = true;
}

bool TextRecord::revealNext() {
	if (_revealsAll || _numRevealed >= _text.size()) {
		return false;
	}

	// Markup is never shown in part
	uint end = _numRevealed;
	bool isInTag = false;
	while (end < _text.size()) {
		if (isInTag) {
			isInTag = _text[end] != '>';
		} else if (_text[end] == '<') {
			isInTag = true;
		} else {
			break;
		}

		++end;
	}

	end = MIN<uint>(end + 1, _text.size());
	_shownText += _text.substr(_numRevealed, end - _numRevealed);
	_numRevealed = end;
	_isDirty = true;
	return true;
}

void TextRecord::run(double sceneTime, double elapsedTime) {
	if (_element.blend(UI::NDUI::kStateNormal, (float)elapsedTime, kTextBlendRate)) {
		_isDirty = true;
	}

	if (_yVelocity == 0.0) {
		return;
	}

	_yOffset += _yVelocity * elapsedTime;
	_isDirty = true;

	// A text that moves is done once it has left its place
	const Common::Rect dest = _dest.toRect();
	const int textHeight = _element.measureTextHeight(_shownText, dest.width());
	if ((_yOffset > textHeight && _yVelocity > 0.0) || (_yOffset < -dest.height() && _yVelocity < 0.0)) {
		finish();
	}
}

void TextRecord::onZChanged() {
	_graphic.setScriptZ(getZ(), getContext());
}

void TextRecord::registerGraphics() {
	_graphic.registerGraphics();
}

void TextRecord::updateGraphics() {
	_graphic.setVisible(isActive(), false);

	if (_isDirty) {
		_isDirty = false;

		Common::Rect area(_graphic._drawSurface.w, _graphic._drawSurface.h);
		area.translate(0, -(int)_yOffset);

		_graphic._drawSurface.clear(0);
		_element.drawText(_graphic._drawSurface, _shownText, area, false);
		_graphic.setNeedsRedraw(true);
	}
}

bool TextRecord::prepareReset() {
	_dest = _initialDest;
	_yOffset = _initialYOffset;
	_yVelocity = _initialYVelocity;
	applyDest();
	return true;
}

// text:RevealNext()
static int textRevealNext(lua_State *L) {
	TextRecord *text = dynamic_cast<TextRecord *>(checkRecord(L, 1));
	if (!text) {
		return luaL_typerror(L, 1, "ND.TextAR");
	}

	lua_pushboolean(L, text->revealNext());
	return 1;
}

bool TextRecord::getField(const Common::String &key) {
	lua_State *L = getLua();

	if (key == "onScreen") {
		pushRect(L, _dest);
	} else if (key == "yOffset") {
		lua_pushnumber(L, _yOffset);
	} else if (key == "yVelocity") {
		lua_pushnumber(L, _yVelocity);
	} else if (key == "RevealNext") {
		lua_pushcfunction(L, textRevealNext);
	} else {
		return false;
	}

	return true;
}

bool TextRecord::setField(const Common::String &key) {
	lua_State *L = getLua();

	if (key == "onScreen") {
		_dest = checkRect(L, 3);
		applyDest();
	} else if (key == "text") {
		setText(luaL_checkstring(L, 3));
	} else if (key == "yOffset") {
		_yOffset = luaL_checknumber(L, 3);
		_isDirty = true;
	} else if (key == "yVelocity") {
		_yVelocity = luaL_checknumber(L, 3);
	} else {
		return false;
	}

	return true;
}

// Movie

void MovieRecord::readTable(int tableIndex) {
	lua_State *L = getLua();

	if (!readField(L, tableIndex, "movie", _name)) {
		luaL_error(L, "MovieAR must be created with a movie parameter");
	}

	if (!pushField(L, tableIndex, "onScreen")) {
		luaL_error(L, "MovieAR must be created with an 'onScreen' parameter");
	}

	_dest = checkRect(L, -1);
	lua_pop(L, 1);

	if (!_player.loadFile(Common::Path(_name))) {
		luaL_error(L, "Could not load movie %s", _name.c_str());
	}

	_firstFrame = 0;
	_lastFrame = _player.getFrameCount() - 1;
	readField(L, tableIndex, "firstFrame", _firstFrame);
	readField(L, tableIndex, "lastFrame", _lastFrame);
	readField(L, tableIndex, "pauseOnLastFrame", _pausesOnLastFrame);
	readField(L, tableIndex, "visible", _isVisible);

	// The whole of every frame, unless a part of it is asked for
	// An empty source, like Nancy26's (-1, -1, -1, -1), shows the whole movie
	_source = ScriptRect(0, 0, _player.getWidth(), _player.getHeight());
	if (pushField(L, tableIndex, "source")) {
		if (!checkRect(L, -1).isEmpty()) {
			_source = checkRect(L, -1);
		}

		lua_pop(L, 1);
	}

	double volume = 0.85;
	readField(L, tableIndex, "volume", volume);
	_player.setVolume((byte)CLIP<int>((int)(volume * 100.0 + 0.5), 0, 100));

	// Nancy28 plays the movies at 15 frames a second, or what the script asks
	// for, whatever the frame rate of the movie
	if (g_nancy->getGameType() >= kGameTypeNancy28) {
		_frameRate = 15.0;
		readField(L, tableIndex, "framerate", _frameRate);
		_player.setFrameRate(_frameRate);
	}

	// true is forever, false once, and a number that many times
	lua_getfield(L, tableIndex, "loop");
	if (lua_isboolean(L, -1)) {
		_numLoops = lua_toboolean(L, -1) ? 0 : 1;
	} else if (lua_isnumber(L, -1)) {
		_numLoops = lua_tointeger(L, -1);
	} else if (!lua_isnil(L, -1)) {
		luaL_error(L, "loop must be either a boolean or a number");
	}

	lua_pop(L, 1);

	if (pushField(L, tableIndex, "modColor")) {
		_modColor = checkColor(L, -1);
		lua_pop(L, 1);
	}

	_graphic.init();
	_graphic.setVisible(false);
	onZChanged();
	_graphic.registerGraphics();

	_player.goToFrame(_firstFrame);
	drawFrame();
}

// Scripts hide a movie that keeps playing with a color without any alpha
bool MovieRecord::isSeeThrough() const {
	return (applyAlpha(_modColor) >> 24) == 0;
}

void MovieRecord::drawFrame() {
	_hasFrame = false;
	applyPosition();
}

void MovieRecord::applyPosition() {
	if (isSeeThrough()) {
		return;
	}

	const ScriptMatrix transform = getTransform();
	const Common::Rect source = _source.toRect();
	const uint32 modColor = takeModColor(_modColor);

	if (transform.isTranslation() && modColor == kNoModColor) {
		Graphics::ManagedSurface &surface = _graphic._drawSurface;
		if (surface.w != source.width() || surface.h != source.height() || !surface.getPixels()) {
			surface.create(MAX<int>(source.width(), 1), MAX<int>(source.height(), 1), g_nancy->_graphics->getScreenPixelFormat());
		}

		// A frame with see-through parts would let the one before it show
		surface.clear(0);
		_player.drawFrame(surface, source, Common::Point(0, 0));
		_graphic.setTransparent(false);
		_graphic.moveTo(getTransformedDest(_dest, transform, source.width(), source.height()));
		_graphic.setNeedsRedraw(true);
		return;
	}

	// Turned, resized or colored frames get drawn anew, the way overlays are
	if (!_hasFrame || _frame.w != _player.getWidth() || _frame.h != _player.getHeight()) {
		if (_frame.w != _player.getWidth() || _frame.h != _player.getHeight()) {
			_frame.create(_player.getWidth(), _player.getHeight(), g_nancy->_graphics->getScreenPixelFormat());
		}

		_frame.clear(0);
		_player.drawFrame(_frame, Common::Point(0, 0));
		_hasFrame = true;
	}

	Common::Rect clipped = source;
	clipped.clip(Common::Rect(_frame.w, _frame.h));
	placePicture(_graphic, _frame, clipped, _dest, transform, modColor);
}

void MovieRecord::runOnce() {
	_player.playRange(_firstFrame, _lastFrame);
	drawFrame();
	_hasJustStarted = true;
}

// A movie that plays through and ends; one that loops forever never does. One
// that stops on its last frame can be skipped until it gets there: Nancy23's
// cinematics are made of those, each starting when the one before is done.
bool MovieRecord::isSkippable() const {
	if (!isActive() || !hasStarted() || _isPaused || _numLoops == 0) {
		return false;
	}

#ifdef ENABLE_NANCY_V5
	// Nancy26: not while the movie is hidden or paused with what it hangs under
	if (g_nancy->getGameType() >= kGameTypeNancy26 && (!isNodeVisible() || isNodePaused())) {
		return false;
	}
#endif

	return !_pausesOnLastFrame || (_player.isRangePlaying() && !_hasCalledOnPause);
}

void MovieRecord::run(double sceneTime, double elapsedTime) {
	// The frame that starts a movie went by before it, and can be a long one
	// when the scene just loaded. The movie would skip ahead of its sound.
	const double playedTime = _hasJustStarted ? 0.0 : elapsedTime;
	_hasJustStarted = false;

	// A movie that is being skipped moves on by one frame every time it runs, so
	// the scripts that wait for one of its frames still see it. The frames on the
	// way aren't decoded, and neither is the last one: a skipped movie ends, which
	// hides it, and getting to its last frame can take decoding all of it, since
	// some have no other keyframe than their first one.
	if (getContext().isSkipping() && canBeSkipped()) {
		if (_skipFrame < 0) {
			_skipFrame = _player.getCurrentFrame();
		}

		if (_skipFrame < _lastFrame) {
			++_skipFrame;
		} else if (_skipFrame > _lastFrame) {
			--_skipFrame;
		}

		// The movie stays at its last frame for the scripts until it starts over.
		// One that stops there shows it, as the next movie takes over from it.
		if (_skipFrame == _lastFrame) {
			if (_pausesOnLastFrame) {
				_player.goToFrame(_lastFrame);
				drawFrame();
				_hasCalledOnPause = true;
				callFunction("OnPause");
			} else {
				_endTime = sceneTime;
				finish();
			}
		}

		return;
	}

	_skipFrame = -1;

	if (_player.advance(playedTime * 1000.0)) {
		drawFrame();
	}

	if (_player.isRangePlaying()) {
		return;
	}

	// Another time through
	++_numPlayed;
	if (_numLoops == 0 || _numPlayed < _numLoops) {
		_player.playRange(_firstFrame, _lastFrame);
		drawFrame();
		return;
	}

	if (!_pausesOnLastFrame) {
		_endTime = sceneTime;
		finish();
	} else if (!_hasCalledOnPause) {
		// Nancy20 scripts can be told once that the movie stopped on its last frame
		_hasCalledOnPause = true;
		callFunction("OnPause");
	}
}

bool MovieRecord::prepareReset() {
	_endTime = 0.0;
	_numPlayed = 0;
	_skipFrame = -1;
	_isPaused = false;
	_hasCalledOnPause = false;
	_player.goToFrame(_firstFrame);
	drawFrame();
	return true;
}

void MovieRecord::registerGraphics() {
	_graphic.registerGraphics();
}

void MovieRecord::onZChanged() {
	_graphic.setScriptZ(getZ(), getContext());
}

void MovieRecord::updateGraphics() {
	if (hasAlphaChanged() && isShown()) {
		applyPosition();
	}

	_graphic.setClipRect(getScreenClip());
	_graphic.setVisible(isShown() && _isVisible && !isSeeThrough(), false);
}

void MovieRecord::describeState(Common::Array<Common::String> &lines) const {
	lines.push_back(Common::String::format("[movie] frame %d of %d..%d, %s%s, played %d of %d",
		_player.getCurrentFrame(), _firstFrame, _lastFrame,
		_player.isRangePlaying() ? "playing" : "stopped", _isPaused ? ", paused" : "",
		_numPlayed, _numLoops));

	const Common::Rect position = _graphic.getScreenPosition();
	lines.push_back(Common::String::format("[graphic] %s, %s, z %u, at (%d, %d, %d, %d), surface %dx%d%s",
		g_nancy->_graphics->hasObject(&_graphic) ? "registered" : "NOT registered",
		_graphic.isVisible() ? "visible" : "hidden", _graphic.getZOrder(),
		position.left, position.top, position.right, position.bottom,
		_graphic._drawSurface.w, _graphic._drawSurface.h, isSeeThrough() ? ", see-through" : ""));
}

// movie:Play() and movie:Pause()
static int moviePlay(lua_State *L) {
	MovieRecord *record = dynamic_cast<MovieRecord *>(checkRecord(L, 1));
	if (record) {
		record->setPaused(false);
	}

	return 0;
}

static int moviePause(lua_State *L) {
	MovieRecord *record = dynamic_cast<MovieRecord *>(checkRecord(L, 1));
	if (record) {
		record->setPaused(true);
	}

	return 0;
}

bool MovieRecord::getField(const Common::String &key) {
	lua_State *L = getLua();

	if (key == "movie") {
		lua_pushstring(L, _name.c_str());
	} else if (key == "onScreen") {
		pushRect(L, _dest);
	} else if (key == "source") {
		pushRect(L, _source);
	} else if (key == "modColor") {
		pushColor(L, _modColor);
	} else if (key == "Play") {
		lua_pushcfunction(L, moviePlay);
	} else if (key == "Pause") {
		lua_pushcfunction(L, moviePause);
	} else if (key == "paused") {
		lua_pushboolean(L, _isPaused || !_player.isRangePlaying());
	} else if (key == "pauseOnLastFrame") {
		lua_pushboolean(L, _pausesOnLastFrame);
	} else if (key == "visible") {
		lua_pushboolean(L, _isVisible);
	} else if (key == "frame") {
		lua_pushinteger(L, _skipFrame >= 0 ? _skipFrame : _player.getCurrentFrame());
	} else if (key == "endTime") {
		lua_pushnumber(L, _endTime);
	} else if (key == "firstFrame") {
		lua_pushinteger(L, _firstFrame);
	} else if (key == "lastFrame") {
		lua_pushinteger(L, _lastFrame);
	} else if (key == "frames") {
		lua_pushinteger(L, _player.getFrameCount());
	} else if (key == "framerate" && _frameRate > 0.0) {
		lua_pushnumber(L, _frameRate);
	} else {
		return false;
	}

	return true;
}

bool MovieRecord::setField(const Common::String &key) {
	lua_State *L = getLua();

	// Scripts set these every frame, mostly to what they already are
	if (key == "onScreen") {
		const ScriptRect dest = checkRect(L, 3);
		if (dest != _dest) {
			_dest = dest;
			applyPosition();
		}
	} else if (key == "source" && g_nancy->getGameType() >= kGameTypeNancy21) {
		// Earlier games only take the part of the frames when the movie is made
		const ScriptRect source = checkRect(L, 3);
		if (source != _source) {
			_source = source;
			drawFrame();
		}
	} else if (key == "modColor") {
		const uint32 modColor = checkColor(L, 3);
		if (modColor != _modColor) {
			_modColor = modColor;
			drawFrame();
		}
	} else if (key == "volume") {
		_player.setVolume((byte)CLIP<int>((int)(luaL_checknumber(L, 3) * 100.0 + 0.5), 0, 100));
	} else if (key == "pauseOnLastFrame") {
		_pausesOnLastFrame = lua_toboolean(L, 3) != 0;
	} else if (key == "framerate" && g_nancy->getGameType() >= kGameTypeNancy28) {
		_frameRate = MAX(luaL_checknumber(L, 3), 0.0);
		_player.setFrameRate(_frameRate);
	} else if (key == "paused") {
		// Nancy24 has no Play and Pause anymore
		_isPaused = lua_toboolean(L, 3) != 0;
	} else if (key == "visible") {
		_isVisible = lua_toboolean(L, 3) != 0;
	} else if (key == "frame") {
		_player.goToFrame(luaL_checkint(L, 3));
		drawFrame();
	} else {
		return false;
	}

	return true;
}

} // End of namespace Script
} // End of namespace Nancy
