/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef VARIABLES_H_
#define VARIABLES_H_

#include "common/hashmap.h"
#include "common/hash-str.h"
#include "common/savefile.h"
#include "common/serializer.h"

#include "engines/myst3/myst3.h"

namespace Myst3 {

class Database;

// View type
enum ViewType {
	kCube = 1,
	kFrame = 2,
	kMenu = 3
};

#define DECLARE_VAR(name) \
	void set##name(int32 value) { engineSet(#name, value); } \
	int32 get##name() { return engineGet(#name); } \
	bool hasVar##name() { return _varDescriptions.contains(#name); }

class GameState {
public:
	GameState(const Common::Platform platform, Database *database);
	virtual ~GameState();

	void newGame();
	Common::Error load(Common::InSaveFile *saveFile);
	Common::Error save(Common::OutSaveFile *saveFile, const Common::String &description, const Graphics::Surface *thumbnail, bool isAutosave);

	int32 getVar(uint16 var);
	void setVar(uint16 var, int32 value);
	bool evaluate(int16 condition);
	int32 valueOrVarValue(int16 value);

	const Common::String describeVar(uint16 var);
	const Common::String describeCondition(int16 condition);

	DECLARE_VAR(CursorTransparency)

	DECLARE_VAR(ProjectorAngleX)
	DECLARE_VAR(ProjectorAngleY)
	DECLARE_VAR(ProjectorAngleZoom)
	DECLARE_VAR(ProjectorAngleBlur)
	DECLARE_VAR(DraggedWeight)

	DECLARE_VAR(DragEnded)
	DECLARE_VAR(DragLeverSpeed)
	DECLARE_VAR(DragPositionFound)
	DECLARE_VAR(DragLeverPositionChanged)

	DECLARE_VAR(LocationAge)
	DECLARE_VAR(LocationRoom)
	DECLARE_VAR(LocationNode)
	DECLARE_VAR(BookSavedAge)
	DECLARE_VAR(BookSavedRoom)
	DECLARE_VAR(BookSavedNode)
	DECLARE_VAR(MenuSavedAge)
	DECLARE_VAR(MenuSavedRoom)
	DECLARE_VAR(MenuSavedNode)

	DECLARE_VAR(SecondsCountdown)
	DECLARE_VAR(TickCountdown)

	DECLARE_VAR(SweepEnabled)
	DECLARE_VAR(SweepValue)
	DECLARE_VAR(SweepStep)
	DECLARE_VAR(SweepMin)
	DECLARE_VAR(SweepMax)

	DECLARE_VAR(InputMousePressed)
	DECLARE_VAR(InputEscapePressed)
	DECLARE_VAR(InputTildePressed)
	DECLARE_VAR(InputSpacePressed)

	DECLARE_VAR(HotspotActiveRect)

	DECLARE_VAR(WaterEffectRunning)
	DECLARE_VAR(WaterEffectActive)
	DECLARE_VAR(WaterEffectSpeed)
	DECLARE_VAR(WaterEffectAttenuation)
	DECLARE_VAR(WaterEffectFrequency)
	DECLARE_VAR(WaterEffectAmpl)
	DECLARE_VAR(WaterEffectMaxStep)
	DECLARE_VAR(WaterEffectAmplOffset)

	DECLARE_VAR(LavaEffectActive)
	DECLARE_VAR(LavaEffectSpeed)
	DECLARE_VAR(LavaEffectAmpl)
	DECLARE_VAR(LavaEffectStepSize)

	DECLARE_VAR(MagnetEffectActive)
	DECLARE_VAR(MagnetEffectSpeed)
	DECLARE_VAR(MagnetEffectUnk1)
	DECLARE_VAR(MagnetEffectUnk2)
	DECLARE_VAR(MagnetEffectSound)
	DECLARE_VAR(MagnetEffectNode)
	DECLARE_VAR(MagnetEffectUnk3)

	DECLARE_VAR(ShakeEffectAmpl)
	DECLARE_VAR(ShakeEffectTickPeriod)
	DECLARE_VAR(RotationEffectSpeed)
	DECLARE_VAR(SunspotIntensity)
	DECLARE_VAR(SunspotColor)
	DECLARE_VAR(SunspotRadius)

	DECLARE_VAR(AmbiantFadeOutDelay)
	DECLARE_VAR(AmbiantPreviousFadeOutDelay)
	DECLARE_VAR(AmbientOverrideFadeOutDelay)
	DECLARE_VAR(SoundScriptsSuspended)

	DECLARE_VAR(SoundNextMultipleSounds)
	DECLARE_VAR(SoundNextIsChoosen)
	DECLARE_VAR(SoundNextId)
	DECLARE_VAR(SoundNextIsLast)
	DECLARE_VAR(SoundScriptsTimer)
	DECLARE_VAR(SoundScriptsPaused)
	DECLARE_VAR(SoundScriptFadeOutDelay)

	DECLARE_VAR(CursorLocked)
	DECLARE_VAR(CursorHidden)

	DECLARE_VAR(CameraPitch)
	DECLARE_VAR(CameraHeading)
	DECLARE_VAR(CameraMinPitch)
	DECLARE_VAR(CameraMaxPitch)

	DECLARE_VAR(MovieStartFrame)
	DECLARE_VAR(MovieEndFrame)
	DECLARE_VAR(MovieVolume1)
	DECLARE_VAR(MovieVolume2)
	DECLARE_VAR(MovieOverrideSubtitles)
	DECLARE_VAR(MovieConditionBit)
	DECLARE_VAR(MoviePreloadToMemory)
	DECLARE_VAR(MovieScriptDriven)
	DECLARE_VAR(MovieNextFrameSetVar)
	DECLARE_VAR(MovieNextFrameGetVar)
	DECLARE_VAR(MovieStartFrameVar)
	DECLARE_VAR(MovieEndFrameVar)
	DECLARE_VAR(MovieForce2d)
	DECLARE_VAR(MovieVolumeVar)
	DECLARE_VAR(MovieSoundHeading)
	DECLARE_VAR(MoviePanningStrenght)
	DECLARE_VAR(MovieSynchronized)
	DECLARE_VAR(MovieOverrideCondition)
	DECLARE_VAR(MovieUVar)
	DECLARE_VAR(MovieVVar)
	DECLARE_VAR(MovieOverridePosition)
	DECLARE_VAR(MovieOverridePosU)
	DECLARE_VAR(MovieOverridePosV)
	DECLARE_VAR(MovieAdditiveBlending)
	DECLARE_VAR(MovieTransparency)
	DECLARE_VAR(MovieTransparencyVar)
	DECLARE_VAR(MoviePlayingVar)
	DECLARE_VAR(MovieStartSoundId)
	DECLARE_VAR(MovieStartSoundVolume)
	DECLARE_VAR(MovieStartSoundHeading)
	DECLARE_VAR(MovieStartSoundAttenuation)

	DECLARE_VAR(MovieUseBackground)
	DECLARE_VAR(CameraSkipAnimation)
	DECLARE_VAR(MovieAmbiantScriptStartFrame)
	DECLARE_VAR(MovieAmbiantScript)
	DECLARE_VAR(MovieScriptStartFrame)
	DECLARE_VAR(MovieScript)

	DECLARE_VAR(CameraMoveSpeed)

	DECLARE_VAR(TransitionSound)
	DECLARE_VAR(TransitionSoundVolume)

	DECLARE_VAR(LocationNextNode)
	DECLARE_VAR(LocationNextRoom)
	DECLARE_VAR(LocationNextAge)

	DECLARE_VAR(BallPosition)
	DECLARE_VAR(BallFrame)
	DECLARE_VAR(BallLeverLeft)
	DECLARE_VAR(BallLeverRight)

	DECLARE_VAR(BallDoorOpen)

	DECLARE_VAR(ProjectorX)
	DECLARE_VAR(ProjectorY)
	DECLARE_VAR(ProjectorZoom)
	DECLARE_VAR(ProjectorBlur)
	DECLARE_VAR(ProjectorAngleXOffset)
	DECLARE_VAR(ProjectorAngleYOffset)
	DECLARE_VAR(ProjectorAngleZoomOffset)
	DECLARE_VAR(ProjectorAngleBlurOffset)

	DECLARE_VAR(JournalAtrusState)
	DECLARE_VAR(JournalSaavedroState)
	DECLARE_VAR(JournalSaavedroClosed)
	DECLARE_VAR(JournalSaavedroOpen)
	DECLARE_VAR(JournalSaavedroLastPage)
	DECLARE_VAR(JournalSaavedroChapter)
	DECLARE_VAR(JournalSaavedroPageInChapter)

	DECLARE_VAR(TeslaAllAligned)
	DECLARE_VAR(TeslaTopAligned)
	DECLARE_VAR(TeslaMiddleAligned)
	DECLARE_VAR(TeslaBottomAligned)
	DECLARE_VAR(TeslaMovieStart)

	DECLARE_VAR(AmateriaSecondsCounter)
	DECLARE_VAR(AmateriaTicksCounter)

	DECLARE_VAR(ResonanceRingsSolved)

	DECLARE_VAR(PinballRemainingPegs)

	DECLARE_VAR(OuterShieldUp)
	DECLARE_VAR(InnerShieldUp)
	DECLARE_VAR(SaavedroStatus)

	DECLARE_VAR(BookStateTomahna)
	DECLARE_VAR(BookStateReleeshahn)

	DECLARE_VAR(SymbolCode2Solved)
	DECLARE_VAR(SymbolCode1AllSolved)
	DECLARE_VAR(SymbolCode1CurrentSolved)
	DECLARE_VAR(SymbolCode1TopSolved)
	DECLARE_VAR(SymbolCode1LeftSolved)
	DECLARE_VAR(SymbolCode1RightSolved)

	DECLARE_VAR(SoundVoltaicUnk540)
	DECLARE_VAR(SoundEdannaUnk587)
	DECLARE_VAR(SoundAmateriaUnk627)
	DECLARE_VAR(SoundAmateriaUnk930)
	DECLARE_VAR(SoundEdannaUnk1031)
	DECLARE_VAR(SoundVoltaicUnk1146)

	DECLARE_VAR(ZipModeEnabled)
	DECLARE_VAR(SubtitlesEnabled)
	DECLARE_VAR(WaterEffects)
	DECLARE_VAR(TransitionSpeed)
	DECLARE_VAR(MouseSpeed)
	DECLARE_VAR(DialogResult)

	DECLARE_VAR(MenuEscapePressed)
	DECLARE_VAR(MenuNextAction)
	DECLARE_VAR(MenuLoadBack)
	DECLARE_VAR(MenuSaveBack)
	DECLARE_VAR(MenuSaveAction)
	DECLARE_VAR(MenuOptionsBack)

	DECLARE_VAR(MenuSaveLoadPageLeft)
	DECLARE_VAR(MenuSaveLoadPageRight)
	DECLARE_VAR(MenuSaveLoadSelectedItem)
	DECLARE_VAR(MenuSaveLoadCurrentPage)

	DECLARE_VAR(OverallVolume)
	DECLARE_VAR(MusicVolume)
	DECLARE_VAR(MusicFrequency)
	DECLARE_VAR(LanguageAudio)
	DECLARE_VAR(LanguageText)
	DECLARE_VAR(HotspotIgnoreClick)
	DECLARE_VAR(HotspotHovered)
	DECLARE_VAR(SpotSubtitle)

	DECLARE_VAR(DragLeverLimited)
	DECLARE_VAR(DragLeverLimitMin)
	DECLARE_VAR(DragLeverLimitMax)

	DECLARE_VAR(ShieldEffectActive)

	// Xbox specific variables
	DECLARE_VAR(GamePadActionPressed)
	DECLARE_VAR(GamePadDownPressed)
	DECLARE_VAR(GamePadUpPressed)
	DECLARE_VAR(GamePadLeftPressed)
	DECLARE_VAR(GamePadRightPressed)
	DECLARE_VAR(GamePadCancelPressed)

	DECLARE_VAR(DragWithDirectionKeys)
	DECLARE_VAR(MenuSavesAvailable)
	DECLARE_VAR(MenuSelectedSave)
	DECLARE_VAR(MenuAttractCountDown)
	DECLARE_VAR(MovieOptional)
	DECLARE_VAR(VibrationEnabled)
	DECLARE_VAR(StateCanSave)

	void updateFrameCounters();
	uint getTickCount() const;

	/** Ensture the counters are correct when the engine is paused or resumed */
	void pauseEngine(bool pause);

	ViewType getViewType() { return static_cast<ViewType>(_data.currentNodeType); }
	void setViewType(ViewType t) { _data.currentNodeType = t; }

	float getLookAtFOV() { return _data.lookatFOV; }
	void setLookAtFOV(float fov) { _data.lookatFOV = fov; }
	float getLookAtPitch() { return _data.lookatPitch; }
	float getLookAtHeading() { return _data.lookatHeading; }
	void lookAt(float pitch, float heading) { _data.lookatPitch = pitch; _data.lookatHeading = heading; }

	void limitCubeCamera(float minPitch, float maxPitch, float minHeading, float maxHeading);
	void freeCubeCamera() { _data.limitCubeCamera = false; }
	bool isCameraLimited() { return _data.limitCubeCamera != 0; }
	float getMinPitch() { return _data.minPitch; }
	float getMaxPitch() { return _data.maxPitch; }
	float getMinHeading() { return _data.minHeading; }
	float getMaxHeading() { return _data.maxHeading; }

	void markNodeAsVisited(uint16 node, uint16 room, uint32 age);
	bool isZipDestinationAvailable(uint16 node, uint16 room, uint32 age);

	Common::String formatSaveTime();

	Common::Array<uint16> getInventory();
	void updateInventory(const Common::Array<uint16> &items);

	struct StateData {
		uint32 version;
		uint32 gameRunning;
		uint32 tickCount;
		uint32 nextSecondsUpdate;
		uint32 secondsPlayed;
		uint32 dword_4C2C44;
		uint32 dword_4C2C48;
		uint32 dword_4C2C4C;
		uint32 dword_4C2C50;
		uint32 dword_4C2C54;
		uint32 dword_4C2C58;
		uint32 dword_4C2C5C;
		uint32 dword_4C2C60;
		uint32 currentNodeType;
		float lookatPitch;
		float lookatHeading;
		float lookatFOV;
		float pitchOffset;
		float headingOffset;
		uint32 limitCubeCamera;
		float minPitch;
		float maxPitch;
		float minHeading;
		float maxHeading;
		uint32  dword_4C2C90;
		int32 vars[2048];
		uint32 inventoryCount;
		uint32 inventoryList[7];
		uint32 zipDestinations[64];

		uint8 saveDay;
		uint8 saveMonth;
		uint16 saveYear;

		uint8 saveHour;
		uint8 saveMinute;

		Common::String saveDescription;

		bool isAutosave;

		StateData();
		Common::Error syncWithSaveGame(Common::Serializer &s);
	};

	static const Graphics::PixelFormat getThumbnailSavePixelFormat();
	static Graphics::Surface *readThumbnail(Common::ReadStream *inStream);
	static void writeThumbnail(Common::WriteStream *outStream, const Graphics::Surface *thumbnail);
	static Graphics::Surface *resizeThumbnail(Graphics::Surface *big, uint width, uint height);

	static const uint kThumbnailWidth = 240;
	static const uint kThumbnailHeight = 135;

private:
	const Common::Platform _platform;
	Database *_db;

	static const uint32 kSaveVersion = 150;

	StateData _data;

	static const uint32 kTickDuration = 1000 / 30;
	uint32 _lastTickStartTime;

	struct VarDescription {
		VarDescription() : var(0), name(0), unknown(0) {}
		VarDescription(uint16 v, const char *n, bool u) : var(v), name(n), unknown(u) {}

		uint16 var;
		const char *name;
		bool unknown;
	};

	typedef Common::HashMap<Common::String, VarDescription> VarMap;

	VarMap _varDescriptions;

	void checkRange(uint16 var);
	const VarDescription findDescription(uint16 var);
	void shiftVariables(uint16 base, int32 value);

	int32 engineGet(const Common::String &varName);
	void engineSet(const Common::String &varName, int32 value);

	static void syncFloat(Common::Serializer &s, float &val,
			Common::Serializer::Version minVersion = 0,
			Common::Serializer::Version maxVersion = Common::Serializer::kLastVersion);

	void updateTickCounters();
};

/**
 * Save files related utility functions
 */
struct Saves {
	/** Build a save file name according to the game platform */
	static Common::String buildName(const char *name, Common::Platform platform);

	/** List all the save file names for the game platform, sorted alphabetically */
	static Common::StringArray list(Common::SaveFileManager *saveFileManager, Common::Platform platform);
};

} // End of namespace Myst3

#endif // VARIABLES_H_
