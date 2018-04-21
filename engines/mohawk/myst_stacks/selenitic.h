/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
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

#ifndef MYST_SCRIPTS_SELENITIC_H
#define MYST_SCRIPTS_SELENITIC_H

#include "common/scummsys.h"
#include "common/util.h"
#include "mohawk/myst_scripts.h"

namespace Mohawk {

class MystAreaImageSwitch;
struct MystScriptEntry;

namespace MystStacks {

#define DECLARE_OPCODE(x) void x(uint16 var, const ArgumentsArray &args)

class Selenitic : public MystScriptParser {
public:
	explicit Selenitic(MohawkEngine_Myst *vm);
	~Selenitic() override;

	void disablePersistentScripts() override;
	void runPersistentScripts() override;

private:
	void setupOpcodes();
	uint16 getVar(uint16 var) override;
	void toggleVar(uint16 var) override;
	bool setVarValue(uint16 var, uint16 value) override;

	uint16 getMap() override { return 9930; }

	DECLARE_OPCODE(o_mazeRunnerMove);
	DECLARE_OPCODE(o_mazeRunnerSoundRepeat);
	DECLARE_OPCODE(o_soundReceiverSigma);
	DECLARE_OPCODE(o_soundReceiverRight);
	DECLARE_OPCODE(o_soundReceiverLeft);
	DECLARE_OPCODE(o_soundReceiverSource);
	DECLARE_OPCODE(o_mazeRunnerDoorButton);
	DECLARE_OPCODE(o_soundReceiverUpdateSound);
	DECLARE_OPCODE(o_soundLockMove);
	DECLARE_OPCODE(o_soundLockStartMove);
	DECLARE_OPCODE(o_soundLockEndMove);
	DECLARE_OPCODE(o_soundLockButton);
	DECLARE_OPCODE(o_soundReceiverEndMove);

	DECLARE_OPCODE(o_mazeRunnerCompass_init);
	DECLARE_OPCODE(o_mazeRunnerWindow_init);
	DECLARE_OPCODE(o_mazeRunnerLight_init);
	DECLARE_OPCODE(o_soundReceiver_init);
	DECLARE_OPCODE(o_soundLock_init);
	DECLARE_OPCODE(o_mazeRunnerRight_init);
	DECLARE_OPCODE(o_mazeRunnerLeft_init);

	enum SoundReceiverSpeed {
		kSoundReceiverSpeedStill  = 0,
		kSoundReceiverSpeedSlow   = 1,
		kSoundReceiverSpeedNormal = 5,   // The original has this at 10
		kSoundReceiverSpeedFast   = 10,  // The original has this at 50 too fast!
		kSoundReceiverSpeedFaster = 13   // The original has this at 100, way too fast!
	};

	void soundReceiver_run();

	MystGameState::Selenitic &_state;

	bool _soundReceiverRunning;
	bool _soundReceiverSigmaPressed; // 6
	MystAreaImageSwitch *_soundReceiverSources[5]; // 92 -> 108
	MystAreaImageSwitch *_soundReceiverCurrentSource; // 112
	uint16 *_soundReceiverPosition; // 116
	uint16 _soundReceiverDirection; // 120
	SoundReceiverSpeed _soundReceiverSpeed; // 122
	uint32 _soundReceiverStartTime; //124
	uint _soundReceiverNearBlinkCounter;
	MystAreaImageSwitch *_soundReceiverViewer; // 128
	MystAreaImageSwitch *_soundReceiverRightButton; // 132
	MystAreaImageSwitch *_soundReceiverLeftButton; // 136
	MystAreaImageSwitch *_soundReceiverAngle1; // 140
	MystAreaImageSwitch *_soundReceiverAngle2; // 144
	MystAreaImageSwitch *_soundReceiverAngle3; // 148
	MystAreaImageSwitch *_soundReceiverAngle4; // 152
	MystAreaImageSwitch *_soundReceiverSigmaButton; // 156

	static const uint16 _mazeRunnerMap[300][4];
	static const uint8 _mazeRunnerVideos[300][4];

	uint16 _mazeRunnerPosition; // 56
	uint16 _mazeRunnerDirection; // 58
	MystAreaImageSwitch *_mazeRunnerWindow; // 68
	MystAreaImageSwitch *_mazeRunnerCompass; // 72
	MystAreaImageSwitch *_mazeRunnerLight; // 76
	MystAreaImageSwitch *_mazeRunnerRightButton; // 80
	MystAreaImageSwitch *_mazeRunnerLeftButton; // 84
	bool _mazeRunnerDoorOpened; // 160

	uint16 _soundLockSoundId;
	MystAreaSlider *_soundLockSlider1; // 164
	MystAreaSlider *_soundLockSlider2; // 168
	MystAreaSlider *_soundLockSlider3; // 172
	MystAreaSlider *_soundLockSlider4; // 176
	MystAreaSlider *_soundLockSlider5; // 180
	MystAreaImageSwitch *_soundLockButton; // 184

	void soundReceiverLeftRight(uint direction);
	void soundReceiverUpdate();
	void soundReceiverSetSubimageRect() const;
	void soundReceiverDrawView();
	void soundReceiverDrawAngle();
	void soundReceiverIncreaseSpeed();
	void soundReceiverUpdateSound();
	uint16 soundReceiverCurrentSound(uint16 source, uint16 position);
	void soundReceiverSolution(uint16 source, uint16 &solution, bool &enabled);

	uint16 soundLockCurrentSound(uint16 position, bool pixels);
	MystAreaSlider *soundLockSliderFromVar(uint16 var);
	void soundLockCheckSolution(MystAreaSlider *slider, uint16 value, uint16 solution, bool &solved);

	bool mazeRunnerForwardAllowed(uint16 position);
	void mazeRunnerUpdateCompass();
	void mazeRunnerPlaySoundHelp();
	void mazeRunnerPlayVideo(uint16 video, uint16 pos);
	void mazeRunnerBacktrack(uint16 &oldPosition);
};

} // End of namespace MystStacks
}

#undef DECLARE_OPCODE

#endif
