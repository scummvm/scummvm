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

#include "engines/myst3/state.h"
#include "engines/myst3/database.h"
#include "engines/myst3/gfx.h"

#include "common/debug-channels.h"
#include "common/ptr.h"
#include "common/savefile.h"

#include "graphics/surface.h"

namespace Myst3 {

GameState::StateData::StateData() {
	version = GameState::kSaveVersion;
	gameRunning = true;
	tickCount = 0;
	nextSecondsUpdate = 0;
	secondsPlayed = 0;
	dword_4C2C44 = 0;
	dword_4C2C48 = 0;
	dword_4C2C4C = 0;
	dword_4C2C50 = 0;
	dword_4C2C54 = 0;
	dword_4C2C58 = 0;
	dword_4C2C5C = 0;
	dword_4C2C60 = 0;
	currentNodeType = 0;
	lookatPitch = 0;
	lookatHeading = 0;
	lookatFOV = 0;
	pitchOffset = 0;
	headingOffset = 0;
	limitCubeCamera = 0;
	minPitch = 0;
	maxPitch = 0;
	minHeading = 0;
	maxHeading = 0;
	dword_4C2C90 = 0;

	for (uint i = 0; i < 2048; i++)
		vars[i] = 0;

	vars[0] = 0;
	vars[1] = 1;

	inventoryCount = 0;

	for (uint i = 0; i < 7; i++)
		inventoryList[i] = 0;

	for (uint i = 0; i < 64; i++)
		zipDestinations[i] = 0;

	saveDay = 0;
	saveMonth = 0;
	saveYear = 0;
	saveHour = 0;
	saveMinute = 0;
	isAutosave = false;
}

GameState::GameState(const Common::Platform platform, Database *database):
		_platform(platform),
		_db(database) {

#define VAR(var, x, unk) _varDescriptions.setVal(#x, VarDescription(var, #x, unk));

	VAR(14, CursorTransparency, false)

	VAR(47, ProjectorAngleX, false)
	VAR(48, ProjectorAngleY, false)
	VAR(49, ProjectorAngleZoom, false)
	VAR(50, ProjectorAngleBlur, false)
	VAR(51, DraggedWeight, false)

	VAR(57, DragEnded, false)
	VAR(58, DragLeverSpeed, false)
	VAR(59, DragPositionFound, false)
	VAR(60, DragLeverPositionChanged, false)

	VAR(61, LocationAge, false)
	VAR(62, LocationRoom, false)
	VAR(63, LocationNode, false)
	VAR(64, BookSavedAge, false)
	VAR(65, BookSavedRoom, false)
	VAR(66, BookSavedNode, false)
	VAR(67, MenuSavedAge, false)
	VAR(68, MenuSavedRoom, false)
	VAR(69, MenuSavedNode, false)

	VAR(70, SecondsCountdown, false)
	VAR(71, TickCountdown, false)

	// Counters, unused by the game scripts
	VAR(76, CounterUnk76, false)
	VAR(77, CounterUnk77, false)
	VAR(78, CounterUnk78, false)

	VAR(79, SweepEnabled, false)
	VAR(80, SweepValue, false)
	VAR(81, SweepStep, false)
	VAR(82, SweepMin, false)
	VAR(83, SweepMax, false)

	VAR(84, InputMousePressed, false)
	VAR(88, InputEscapePressed, false)
	VAR(89, InputTildePressed, false)
	VAR(90, InputSpacePressed, false)

	VAR(92, HotspotActiveRect, false)

	VAR(93, WaterEffectRunning, false)
	VAR(94, WaterEffectActive, false)
	VAR(95, WaterEffectSpeed, false)
	VAR(96, WaterEffectAttenuation, false)
	VAR(97, WaterEffectFrequency, false)
	VAR(98, WaterEffectAmpl, false)
	VAR(99, WaterEffectMaxStep, false)
	VAR(100, WaterEffectAmplOffset, false)

	VAR(101, LavaEffectActive, false)
	VAR(102, LavaEffectSpeed, false)
	VAR(103, LavaEffectAmpl, false)
	VAR(104, LavaEffectStepSize, false)

	VAR(105, MagnetEffectActive, false)
	VAR(106, MagnetEffectSpeed, false)
	VAR(107, MagnetEffectUnk1, false)
	VAR(108, MagnetEffectUnk2, false)
	VAR(109, MagnetEffectSound, false)
	VAR(110, MagnetEffectNode, false)
	VAR(111, MagnetEffectUnk3, false)

	VAR(112, ShakeEffectAmpl, false)
	VAR(113, ShakeEffectTickPeriod, false)
	VAR(114, RotationEffectSpeed, false)
	VAR(115, SunspotIntensity, false)
	VAR(116, SunspotColor, false)
	VAR(117, SunspotRadius, false)

	VAR(119, AmbiantFadeOutDelay, false)
	VAR(120, AmbiantPreviousFadeOutDelay, false)
	VAR(121, AmbientOverrideFadeOutDelay, false)
	VAR(122, SoundScriptsSuspended, false)

	VAR(124, SoundNextMultipleSounds, false)
	VAR(125, SoundNextIsChoosen, false)
	VAR(126, SoundNextId, false)
	VAR(127, SoundNextIsLast, false)
	VAR(128, SoundScriptsTimer, false)
	VAR(129, SoundScriptsPaused, false)
	VAR(130, SoundScriptFadeOutDelay, false)

	VAR(131, CursorLocked, false)
	VAR(132, CursorHidden, false)

	VAR(136, CameraPitch, false)
	VAR(137, CameraHeading, false)
	VAR(140, CameraMinPitch, false)
	VAR(141, CameraMaxPitch, false)

	VAR(142, MovieStartFrame, false)
	VAR(143, MovieEndFrame, false)
	VAR(144, MovieVolume1, false)
	VAR(145, MovieVolume2, false)
	VAR(146, MovieOverrideSubtitles, false)

	VAR(149, MovieConditionBit, false)
	VAR(150, MoviePreloadToMemory, false)
	VAR(151, MovieScriptDriven, false)
	VAR(152, MovieNextFrameSetVar, false)
	VAR(153, MovieNextFrameGetVar, false)
	VAR(154, MovieStartFrameVar, false)
	VAR(155, MovieEndFrameVar, false)
	VAR(156, MovieForce2d, false)
	VAR(157, MovieVolumeVar, false)
	VAR(158, MovieSoundHeading, false)
	VAR(159, MoviePanningStrenght, false)
	VAR(160, MovieSynchronized, false)

	// We ignore this, and never skip frames
	VAR(161, MovieNoFrameSkip, false)

	// Only play the audio track. This is used in TOHO 3 only.
	// Looks like it works fine without any specific implementation
	VAR(162, MovieAudioOnly, false)

	VAR(163, MovieOverrideCondition, false)
	VAR(164, MovieUVar, false)
	VAR(165, MovieVVar, false)
	VAR(166, MovieOverridePosition, false)
	VAR(167, MovieOverridePosU, false)
	VAR(168, MovieOverridePosV, false)
	VAR(169, MovieScale, false)
	VAR(170, MovieAdditiveBlending, false)
	VAR(171, MovieTransparency, false)
	VAR(172, MovieTransparencyVar, false)
	VAR(173, MoviePlayingVar, false)
	VAR(174, MovieStartSoundId, false)
	VAR(175, MovieStartSoundVolume, false)
	VAR(176, MovieStartSoundHeading, false)
	VAR(177, MovieStartSoundAttenuation, false)

	VAR(178, MovieUseBackground, false)
	VAR(179, CameraSkipAnimation, false)
	VAR(180, MovieAmbiantScriptStartFrame, false)
	VAR(181, MovieAmbiantScript, false)
	VAR(182, MovieScriptStartFrame, false)
	VAR(183, MovieScript, false)

	VAR(185, CameraMoveSpeed, false)

	// We always allow missing SpotItem data
	VAR(186, SpotItemAllowMissing, false)

	VAR(187, TransitionSound, false)
	VAR(188, TransitionSoundVolume, false)
	VAR(189, LocationNextNode, false)
	VAR(190, LocationNextRoom, false)
	VAR(191, LocationNextAge, false)

	VAR(195, BallPosition, false)
	VAR(196, BallFrame, false)
	VAR(197, BallLeverLeft, false)
	VAR(198, BallLeverRight, false)

	VAR(228, BallDoorOpen, false)

	VAR(243, ProjectorX, false)
	VAR(244, ProjectorY, false)
	VAR(245, ProjectorZoom, false)
	VAR(246, ProjectorBlur, false)
	VAR(247, ProjectorAngleXOffset, false)
	VAR(248, ProjectorAngleYOffset, false)
	VAR(249, ProjectorAngleZoomOffset, false)
	VAR(250, ProjectorAngleBlurOffset, false)

	VAR(277, JournalAtrusState, false)
	VAR(279, JournalSaavedroState, false)
	VAR(280, JournalSaavedroClosed, false)
	VAR(281, JournalSaavedroOpen, false)
	VAR(282, JournalSaavedroLastPage, false)
	VAR(283, JournalSaavedroChapter, false)
	VAR(284, JournalSaavedroPageInChapter, false)

	VAR(329, TeslaAllAligned, false)
	VAR(330, TeslaTopAligned, false)
	VAR(331, TeslaMiddleAligned, false)
	VAR(332, TeslaBottomAligned, false)
	VAR(333, TeslaMovieStart, false)

	// Amateria ambient sound / movie counters (XXXX 1001 and XXXX 1002)
	VAR(406, AmateriaSecondsCounter, false)
	VAR(407, AmateriaTicksCounter, false)

	VAR(444, ResonanceRingsSolved, false)

	VAR(460, PinballRemainingPegs, false)

	VAR(475, OuterShieldUp, false)
	VAR(476, InnerShieldUp, false)
	VAR(479, SaavedroStatus, false)

	VAR(480, BookStateTomahna, false)
	VAR(481, BookStateReleeshahn, false)

	VAR(489, SymbolCode2Solved, false)
	VAR(495, SymbolCode1AllSolved, false)
	VAR(496, SymbolCode1CurrentSolved, false)
	VAR(497, SymbolCode1TopSolved, false)
	VAR(502, SymbolCode1LeftSolved, false)
	VAR(507, SymbolCode1RightSolved, false)

	VAR(540, SoundVoltaicUnk540, false)
	VAR(587, SoundEdannaUnk587, false)
	VAR(627, SoundAmateriaUnk627, false)
	VAR(930, SoundAmateriaUnk930, false)
	VAR(1031, SoundEdannaUnk1031, false)
	VAR(1146, SoundVoltaicUnk1146, false)

	VAR(1322, ZipModeEnabled, false)
	VAR(1323, SubtitlesEnabled, false)
	VAR(1324, WaterEffects, false)
	VAR(1325, TransitionSpeed, false)
	VAR(1326, MouseSpeed, false)
	VAR(1327, DialogResult, false)

	VAR(1395, HotspotIgnoreClick, false)
	VAR(1396, HotspotHovered, false)
	VAR(1397, SpotSubtitle, false)

	// Override node from which effect masks are loaded
	// This is only used in LEIS x75, but is useless
	// since all the affected nodes have the same effect masks
	VAR(1398, EffectsOverrideMaskNode, false)

	VAR(1399, DragLeverLimited, false)
	VAR(1400, DragLeverLimitMin, false)
	VAR(1401, DragLeverLimitMax, false)

	// Mouse unk
	VAR(6, Unk6, true)

	// Backup var for opcodes 245, 246 => find usage
	VAR(13, Unk13, true)

	// ???
	VAR(147, MovieUnk147, true)
	VAR(148, MovieUnk148, true)

	if (_platform != Common::kPlatformXbox) {
		VAR(1337, MenuEscapePressed, false)
		VAR(1338, MenuNextAction, false)
		VAR(1339, MenuLoadBack, false)
		VAR(1340, MenuSaveBack, false)
		VAR(1341, MenuSaveAction, false)
		VAR(1342, MenuOptionsBack, false)

		VAR(1350, MenuSaveLoadPageLeft, false)
		VAR(1351, MenuSaveLoadPageRight, false)
		VAR(1352, MenuSaveLoadSelectedItem, false)
		VAR(1353, MenuSaveLoadCurrentPage, false)

		// Menu stuff does not look like it's too useful
		VAR(1361, Unk1361, true)
		VAR(1362, Unk1362, true)
		VAR(1363, Unk1363, true)

		VAR(1374, OverallVolume, false)
		VAR(1377, MusicVolume, false)
		VAR(1380, MusicFrequency, false)
		VAR(1393, LanguageAudio, false)
		VAR(1394, LanguageText, false)

		VAR(1406, ShieldEffectActive, false)
	} else {
		shiftVariables(927, 1);
		shiftVariables(1031, 2);
		shiftVariables(1395, -22);

		VAR(1340, MenuSavesAvailable, false)
		VAR(1341, MenuNextAction, false)
		VAR(1342, MenuLoadBack, false)
		VAR(1343, MenuSaveBack, false)
		VAR(1344, MenuSaveAction, false)
		VAR(1345, MenuOptionsBack, false)
		VAR(1346, MenuSelectedSave, false)

		VAR(1384, MovieOptional, false)
		VAR(1386, VibrationEnabled, false)

		VAR(1430, GamePadActionPressed, false)
		VAR(1431, GamePadDownPressed, false)
		VAR(1432, GamePadUpPressed, false)
		VAR(1433, GamePadLeftPressed, false)
		VAR(1434, GamePadRightPressed, false)
		VAR(1435, GamePadCancelPressed, false)

		VAR(1437, DragWithDirectionKeys, false)
		VAR(1438, MenuAttractCountDown, false)
		VAR(1439, ShieldEffectActive, false)

		VAR(1445, StateCanSave, false)
	}

#undef VAR

	newGame();
}

GameState::~GameState() {
}

void GameState::syncFloat(Common::Serializer &s, float &val,
	                  Common::Serializer::Version minVersion, Common::Serializer::Version maxVersion) {
	static const float precision = 10000.0;

	if (s.isLoading()) {
		int32 saved = 0;
		s.syncAsSint32LE(saved, minVersion, maxVersion);
		val = saved / precision;
	} else {
		int32 toSave = static_cast<int32>(val * precision);
		s.syncAsSint32LE(toSave, minVersion, maxVersion);
	}
}

Common::Error GameState::StateData::syncWithSaveGame(Common::Serializer &s) {
	if (!s.syncVersion(kSaveVersion))
		return Common::Error(Common::kUnknownError, Common::String::format("This savegame (v%d) is too recent (max %d) please get a newer version of ScummVM", s.getVersion(), kSaveVersion));

	s.syncAsUint32LE(gameRunning);
	s.syncAsUint32LE(tickCount);
	s.syncAsUint32LE(nextSecondsUpdate);
	s.syncAsUint32LE(secondsPlayed);
	s.syncAsUint32LE(dword_4C2C44);
	s.syncAsUint32LE(dword_4C2C48);
	s.syncAsUint32LE(dword_4C2C4C);
	s.syncAsUint32LE(dword_4C2C50);
	s.syncAsUint32LE(dword_4C2C54);
	s.syncAsUint32LE(dword_4C2C58);
	s.syncAsUint32LE(dword_4C2C5C);
	s.syncAsUint32LE(dword_4C2C60);
	s.syncAsUint32LE(currentNodeType);

	// The original engine (v148) saved the raw IEE754 data,
	// we save decimal data as fixed point instead to be achieve portability
	if (s.getVersion() < 149) {
		s.syncBytes((byte*) &lookatPitch, sizeof(float));
		s.syncBytes((byte*) &lookatHeading, sizeof(float));
		s.syncBytes((byte*) &lookatFOV, sizeof(float));
		s.syncBytes((byte*) &pitchOffset, sizeof(float));
		s.syncBytes((byte*) &headingOffset, sizeof(float));
	} else {
		syncFloat(s, lookatPitch);
		syncFloat(s, lookatHeading);
		syncFloat(s, lookatFOV);
		syncFloat(s, pitchOffset);
		syncFloat(s, headingOffset);
	}

	s.syncAsUint32LE(limitCubeCamera);

	if (s.getVersion() < 149) {
		s.syncBytes((byte*) &minPitch, sizeof(float));
		s.syncBytes((byte*) &maxPitch, sizeof(float));
		s.syncBytes((byte*) &minHeading, sizeof(float));
		s.syncBytes((byte*) &maxHeading, sizeof(float));
	} else {
		syncFloat(s, minPitch);
		syncFloat(s, maxPitch);
		syncFloat(s, minHeading);
		syncFloat(s, maxHeading);
	}

	s.syncAsUint32LE(dword_4C2C90);

	for (uint i = 0; i < 2048; i++)
		s.syncAsSint32LE(vars[i]);

	s.syncAsUint32LE(inventoryCount);

	for (uint i = 0; i < 7; i++)
		s.syncAsUint32LE(inventoryList[i]);

	for (uint i = 0; i < 64; i++)
		s.syncAsUint32LE(zipDestinations[i]);

	s.syncAsByte(saveDay, 149);
	s.syncAsByte(saveMonth, 149);
	s.syncAsUint16LE(saveYear, 149);
	s.syncAsByte(saveHour, 149);
	s.syncAsByte(saveMinute, 149);
	s.syncString(saveDescription, 149);
	s.syncAsUint32LE(isAutosave, 150);

	return Common::kNoError;
}

const Graphics::PixelFormat GameState::getThumbnailSavePixelFormat() {
#ifdef SCUMM_BIG_ENDIAN
	return Graphics::PixelFormat(4, 8, 8, 8, 0, 8, 16, 24, 0);
#else
	return Graphics::PixelFormat(4, 8, 8, 8, 0, 16, 8, 0, 24);
#endif
}

Graphics::Surface *GameState::readThumbnail(Common::ReadStream *inStream) {
	Graphics::Surface *thumbnail = new Graphics::Surface();
	thumbnail->create(kThumbnailWidth, kThumbnailHeight, getThumbnailSavePixelFormat());

	inStream->read((byte *)thumbnail->getPixels(), kThumbnailWidth * kThumbnailHeight * 4);

	thumbnail->convertToInPlace(Texture::getRGBAPixelFormat());

	return thumbnail;
}

void GameState::writeThumbnail(Common::WriteStream *outStream, const Graphics::Surface *thumbnail) {
	assert(thumbnail->format == Texture::getRGBAPixelFormat());
	assert(thumbnail && thumbnail->w == kThumbnailWidth && thumbnail->h == kThumbnailHeight);

	Graphics::Surface *converted = thumbnail->convertTo(getThumbnailSavePixelFormat());

	outStream->write((byte *)converted->getPixels(), kThumbnailWidth * kThumbnailHeight * 4);

	converted->free();
	delete converted;
}

Graphics::Surface *GameState::resizeThumbnail(Graphics::Surface *big, uint width, uint height) {
	assert(big->format.bytesPerPixel == 4);
	Graphics::Surface *small = new Graphics::Surface();
	small->create(width, height, big->format);

	uint32 *dst = (uint32 *)small->getPixels();
	for (int i = 0; i < small->h; i++) {
		for (int j = 0; j < small->w; j++) {
			uint32 srcX = big->w * j / small->w;
			uint32 srcY = big->h * i / small->h;
			uint32 *src = (uint32 *)big->getBasePtr(srcX, srcY);

			// Copy RGBA pixel
			*dst++ = *src;
		}
	}

	return small;
}

void GameState::newGame() {
	_data = StateData();
	_lastTickStartTime = g_system->getMillis();
}

Common::Error GameState::load(Common::InSaveFile *saveFile) {
	Common::Serializer s = Common::Serializer(saveFile, nullptr);
	Common::Error loadError = _data.syncWithSaveGame(s);

	_data.gameRunning = true;

	if (loadError.getCode() != Common::kNoError) {
		return loadError;
	}

	return Common::kNoError;
}

Common::Error GameState::save(Common::OutSaveFile *saveFile, const Common::String &description, const Graphics::Surface *thumbnail, bool isAutosave) {
	Common::Serializer s = Common::Serializer(nullptr, saveFile);

	// Update save creation info
	TimeDate t;
	g_system->getTimeAndDate(t);
	_data.saveYear = t.tm_year + 1900;
	_data.saveMonth = t.tm_mon + 1;
	_data.saveDay = t.tm_mday;
	_data.saveHour = t.tm_hour;
	_data.saveMinute = t.tm_min;
	_data.saveDescription = description;
	_data.isAutosave = isAutosave;

	_data.gameRunning = false;

	Common::Error saveError = _data.syncWithSaveGame(s);
	if (saveError.getCode() != Common::kNoError) {
		return saveError;
	}

	writeThumbnail(saveFile, thumbnail);

	_data.gameRunning = true;

	return Common::kNoError;
}

Common::String GameState::formatSaveTime() {
	if (_data.saveYear == 0)
		return "";

	// TODO: Check the Xbox NTSC version, maybe it uses that strange MM/DD/YYYY format
	return Common::String::format("%02d/%02d/%02d %02d:%02d",
	                              _data.saveDay, _data.saveMonth, _data.saveYear,
	                              _data.saveHour, _data.saveMinute);
}

Common::Array<uint16> GameState::getInventory() {
	Common::Array<uint16> items;

	for (uint i = 0; i < _data.inventoryCount; i++)
		items.push_back(_data.inventoryList[i]);

	return items;
}

void GameState::updateInventory(const Common::Array<uint16> &items) {
	for (uint i = 0; i < 7; i++)
		_data.inventoryList[i] = 0;

	for (uint i = 0; i < items.size(); i++)
		_data.inventoryList[i] = items[i];

	_data.inventoryCount = items.size();
}

void GameState::checkRange(uint16 var) {
	if (var < 1 || var > 2047)
		error("Variable out of range %d", var);
}

const GameState::VarDescription GameState::findDescription(uint16 var) {
	for (VarMap::const_iterator it = _varDescriptions.begin(); it != _varDescriptions.end(); it++) {
		if (it->_value.var == var) {
			return it->_value;
		}
	}

	return VarDescription();
}

void GameState::shiftVariables(uint16 base, int32 value) {
	for (VarMap::iterator it = _varDescriptions.begin(); it != _varDescriptions.end(); it++) {
		if (it->_value.var >= base) {
			it->_value.var += value;
		}
	}
}

int32 GameState::getVar(uint16 var) {
	checkRange(var);

	return _data.vars[var];
}

void GameState::setVar(uint16 var, int32 value) {
	checkRange(var);

	if (DebugMan.isDebugChannelEnabled(kDebugVariable)) {
		const VarDescription &d = findDescription(var);

		if (d.name && d.unknown) {
			warning("A script is writing to the unimplemented engine-mapped var %d (%s)", var, d.name);
		}
	}

	_data.vars[var] = value;
}

bool GameState::evaluate(int16 condition) {
	uint16 unsignedCond = abs(condition);
	uint16 var = unsignedCond & 2047;
	int32 varValue = getVar(var);
	int32 targetValue = (unsignedCond >> 11) - 1;

	if (targetValue >= 0) {
		if (condition >= 0)
			return varValue == targetValue;
		else
			return varValue != targetValue;
	} else {
		if (condition >= 0)
			return varValue != 0;
		else
			return varValue == 0;
	}
}

int32 GameState::valueOrVarValue(int16 value) {
	if (value < 0)
		return getVar(-value);

	return value;
}

int32 GameState::engineGet(const Common::String &varName) {
	if (!_varDescriptions.contains(varName))
		error("The engine is trying to access an undescribed var (%s)", varName.c_str());

	const VarDescription &d = _varDescriptions.getVal(varName);

	return _data.vars[d.var];
}

void GameState::engineSet(const Common::String &varName, int32 value) {
	if (!_varDescriptions.contains(varName))
		error("The engine is trying to access an undescribed var (%s)", varName.c_str());

	const VarDescription &d = _varDescriptions.getVal(varName);

	_data.vars[d.var] = value;
}

const Common::String GameState::describeVar(uint16 var) {
	const VarDescription &d = findDescription(var);

	if (d.name) {
		return Common::String::format("v%s", d.name);
	} else {
		return Common::String::format("v%d", var);
	}
}

const Common::String GameState::describeCondition(int16 condition) {
	uint16 unsignedCond = abs(condition);
	uint16 var = unsignedCond & 2047;
	int16 value = (unsignedCond >> 11) - 1;

	return Common::String::format("c[%s %s %d]",
	                              describeVar(var).c_str(),
	                              (condition >= 0 && value >= 0) || (condition < 0 && value < 0) ? "==" : "!=",
	                              value >= 0 ? value : 0);
}

void GameState::limitCubeCamera(float minPitch, float maxPitch, float minHeading, float maxHeading) {
	_data.limitCubeCamera = true;
	_data.minPitch = minPitch;
	_data.maxPitch = maxPitch;
	_data.minHeading = minHeading;
	_data.maxHeading = maxHeading;
}

void GameState::updateFrameCounters() {
	if (!_data.gameRunning)
		return;

	uint32 currentTime = g_system->getMillis();
	int32 timeToNextTick = _lastTickStartTime + kTickDuration - currentTime;

	if (timeToNextTick <= 0) {
		_data.tickCount++;
		updateTickCounters();
		_lastTickStartTime = currentTime + timeToNextTick;
	}

	if (currentTime > _data.nextSecondsUpdate || ABS<int32>(_data.nextSecondsUpdate - currentTime) > 2000) {
		_data.secondsPlayed++;
		_data.nextSecondsUpdate = currentTime + 1000;

		int32 secondsCountdown = getSecondsCountdown();
		if (secondsCountdown > 0)
			setSecondsCountdown(--secondsCountdown);

		if (getAmateriaSecondsCounter() > 0)
			setAmateriaSecondsCounter(getAmateriaSecondsCounter() - 1);

		if (getSoundScriptsTimer() > 0)
			setSoundScriptsTimer(getSoundScriptsTimer() - 1);

		if (hasVarMenuAttractCountDown() && getMenuAttractCountDown() > 0)
			setMenuAttractCountDown(getMenuAttractCountDown() - 1);
	}
}

void GameState::updateTickCounters() {
	int32 tickCountdown = getTickCountdown();
	if (tickCountdown > 0)
			setTickCountdown(--tickCountdown);

	if (getAmateriaTicksCounter() > 0)
			setAmateriaTicksCounter(getAmateriaTicksCounter() - 1);

	if (getSweepEnabled()) {
			if (getSweepValue() + getSweepStep() > getSweepMax()) {
				setSweepValue(getSweepMax());

				if (getSweepStep() > 0) {
					setSweepStep(-getSweepStep());
				}
			} else if (getSweepValue() + getSweepStep() < getSweepMin()) {
				setSweepValue(getSweepMin());

				if (getSweepStep() < 0) {
					setSweepStep(-getSweepStep());
				}
			} else {
				setSweepValue(getSweepValue() + getSweepStep());
			}
		}
}

uint GameState::getTickCount() const {
	return _data.tickCount;
}

void GameState::pauseEngine(bool pause) {
	if (!pause) {
		_lastTickStartTime = g_system->getMillis();
	}
}

bool GameState::isZipDestinationAvailable(uint16 node, uint16 room, uint32 age) {
	int32 zipBitIndex = _db->getNodeZipBitIndex(node, room, age);

	int32 arrayIndex = zipBitIndex / 32;
	assert(arrayIndex < 64);

	return (_data.zipDestinations[arrayIndex] & (1 << (zipBitIndex % 32))) != 0;
}

void GameState::markNodeAsVisited(uint16 node, uint16 room, uint32 age) {
	int32 zipBitIndex = _db->getNodeZipBitIndex(node, room, age);

	int32 arrayIndex = zipBitIndex / 32;
	assert(arrayIndex < 64);

	_data.zipDestinations[arrayIndex] |= 1 << (zipBitIndex % 32);
}

Common::String Saves::buildName(const char *name, Common::Platform platform) {
	const char *format;

	if (platform == Common::kPlatformXbox) {
		format = "%s.m3x";
	} else {
		format = "%s.m3s";
	}

	return Common::String::format(format, name);
}

struct AutosaveFirstComparator {
	bool operator()(const Common::String &x, const Common::String &y) const {
		if (x.hasPrefixIgnoreCase("autosave.")) {
			return true;
		}

		if (y.hasPrefixIgnoreCase("autosave.")) {
			return false;
		}

		return x < y;
	}
};

Common::StringArray Saves::list(Common::SaveFileManager *saveFileManager, Common::Platform platform) {
	Common::String searchPattern = Saves::buildName("*", platform);
	Common::StringArray filenames = saveFileManager->listSavefiles(searchPattern);

	// The saves are sorted alphabetically
	Common::sort(filenames.begin(), filenames.end(), AutosaveFirstComparator());

	return filenames;
}

} // End of namespace Myst3
