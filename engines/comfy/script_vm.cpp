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

#include "comfy/comfy.h"

#include "common/config-manager.h"
#include "common/endian.h"

namespace Comfy {

#define O_ACTOR_SET_POSITION 0x01
#define O_ACTOR_ADD_POSITION 0x02
#define O_ACTOR_SET_FRAME 0x03
#define O_WAIT_KEY_SET 0x04
#define O_JUMP_IF_KEY_STATE 0x05
#define O_SET_KEY_STATE 0x06
#define O_ACTOR_DESTROY 0x07
#define O_ACTOR_CREATE 0x08
#define O_SET_CURRENT_ACTOR_FRAME 0x09
#define O_ACTOR_SHOW 0x0A
#define O_ACTOR_HIDE 0x0B
#define O_ACTOR_ACTIVATE 0x0C
#define O_ACTOR_DEACTIVATE 0x0D
#define O_ACTOR_COMPARE_X_SET_KEY 0x0E
#define O_ACTOR_COMPARE_Y_SET_KEY 0x0F
#define O_ACTOR_COMPARE_X_CLEAR_KEY 0x10
#define O_ACTOR_COMPARE_Y_CLEAR_KEY 0x11
#define O_MIDI_ADD_TRACK 0x12
#define O_RANDOM_JUMP 0x13
#define O_ACTOR_REORDER 0x14
#define O_LOAD_SPRITE_CONV_LIST 0x15
#define O_LOAD_SOUND_LIST 0x16
#define O_QUEUE_VOC 0x17
#define O_ACTOR_RESET 0x18
#define O_ACTOR_CALL_IF_KEY_STATE 0x19
#define O_ACTOR_RETURN 0x1A
#define O_PLAY_VOC_QUEUE 0x1B
#define O_MIDI_REMOVE_TRACK 0x1C
#define O_MIDI_CHANNEL_CONTROL 0x1D
#define O_ACTOR_SET_TRIGGER 0x1E
#define O_SET_MUSIC_EVENT 0x1F
#define O_REQUEST_SCENE 0x20
#define O_PALETTE_CONTROL 0x21
#define O_SET_MUSIC_ENABLED 0x22
#define O_SCRIPT_VARIABLE_ADD 0x23
#define O_MIDI_SET_TIME_SCALE 0x24
#define O_CLEAR_KEY_RANGE 0x25
#define O_SCRIPT_VARIABLE_GE_VALUE_SET_KEY 0x26
#define O_SKIP_WORD_28 0x28
#define O_SKIP_WORD_29 0x29
#define O_ACTOR_SET_SPRITE_SELECTOR 0x2A
#define O_JUMP_RELATIVE 0x2B
#define O_COPY_KEY 0x2C
#define O_SCENE_CONTROL 0x2D
#define O_SCRIPT_VARIABLE_GE_VARIABLE_SET_KEY 0x2E
#define O_SCRIPT_VARIABLE_COPY 0x2F
#define O_ANIMATION_CONTROL 0x30
#define O_SET_LAST_VOC_ARGUMENTS 0x31
#define O_UNUSED_32 0x32
#define O_SKIP_TABLE_BLOCK 0x33
#define O_CONFIG_STRING_COMMAND 0x34
#define O_MOUSE_SET_ACTOR 0x35
#define O_HOST_MEDIA_CONTROL 0x36
#define O_ANIMATION_AUDIO_CONTROL 0x46
#define O_PRELOAD_SPRITE_CONV_RANGE 0x47
#define O_JUMP_IF_KEY_CLEAR_V3 0x48
#define O_JUMP_IF_KEY_SET_V3 0x49
#define O_STRING_TABLE_COMPARE_SET_KEY 0x50
#define O_ACTOR_MOVE_BY 0x51
#define O_ACTOR_MOVE_TO 0x52
#define O_ACTOR_SET_FRAME_AND_STOP 0x53
#define O_COPY_INLINE_BLOCK 0x54
#define O_COPY_PALETTE_ENTRY 0x55
#define O_WAIT 0x6E
#define O_MIDI_ADD_EVENT 0x6F
#define O_ACTOR_MOVE_BY_V2 0x70
#define O_ACTOR_MOVE_TO_V2 0x71
#define O_ACTOR_WAIT_TICKS 0x72
#define O_WAVE_OUTPUT_RESET 0x73
#define O_WAIT_KEY_CLEAR 0x74
#define O_CONDITIONAL_SET_KEY 0x75
#define O_STRING_TABLE_SET 0x76
#define O_STRING_TABLE_SET_OFFSET 0x77
#define O_NOP 0x7F

#define SO_MIDI_LOAD_TRACK 1
#define SO_MIDI_SET_RATE 2
#define SO_MIDI_SET_PITCH 3
#define SO_MIDI_SET_VOLUME 4
#define SO_MIDI_STOP_AND_ADVANCE 5
#define SO_MIDI_STOP_AND_FIRE_KEYS 6
#define SO_MIDI_ENABLE_LOOP 7
#define SO_MIDI_DISABLE_LOOP 8
#define SO_SCENE_ENTRY_INIT 1
#define SO_SCENE_ENTRY_STOP 2
#define SO_SCENE_ENTRY_LOAD 3
#define SO_SCENE_MIDI_PLAY_AT_FRAME 4
#define SO_SCENE_MIDI_STOP 5
#define SO_SCENE_MIDI_SET_VOLUME 6
#define SO_ANIM_SHUTDOWN 0
#define SO_ANIM_LOAD_FRAME 1
#define SO_ANIM_SET_READY 2
#define SO_ANIM_CLEAR_READY 3
#define SO_ANIM_NOOP_4 4
#define SO_ANIM_NOOP_5 5
#define SO_ANIM_INVALIDATE_ACTOR_RECT 6
#define SO_CONFIG_READ_INT 0
#define SO_CONFIG_WRITE_INT 1
#define SO_CONFIG_SUSPEND_ACTOR 2
#define SO_CONFIG_RUN_COMMAND 3
#define SO_CONFIG_TEST_VALUE 4
#define SO_CONFIG_TEST_FILE 5
#define SO_CONFIG_SET_LANGUAGE_DATA_SUBDIRECTORY 6
#define SO_MOUSE_SET_ACTOR 1
#define SO_MOUSE_CLEAR_ACTOR 2
#define SO_HOST_MEDIA_START_INPUT 1
#define SO_HOST_MEDIA_STOP_INPUT 2
#define SO_HOST_MEDIA_START_RECORDING_2_ARGS 3
#define SO_HOST_MEDIA_START_RECORDING_3_ARGS 4
#define SO_HOST_MEDIA_START_RECORDING_4_ARGS 5
#define SO_ANIMATION_INDEX_OFFSET 1
#define SO_WAVE_BALANCE 2
#define SO_ANIMATION_ACTOR_SCENE 3
#define SO_WAVE_LEFT 4
#define SO_WAVE_RIGHT 5
#define SO_ANIMATION_AUDIO_NOOP 6
#define SO_HOST_MEDIA_RANGE 7
#define SO_MIXER_VOLUME 8
#define SO_CONDITION_SCENE_EXISTS 1
#define SO_CONDITION_INPUT_MODE 2
#define SO_CONDITION_MOUSE_LEFT_BUTTON 3
#define SO_CONDITION_MOUSE_LEFT_BUTTON_ALT 4
#define SO_CONDITION_MOUSE_RIGHT_BUTTON 5
#define SO_CONDITION_MOUSE_RIGHT_BUTTON_ALT 6
#define SO_CONDITION_ACTOR_HIT_TEST 7
#define SO_CONDITION_ACTOR_BLIT_HIT_TEST 8
#define SO_CONDITION_FALSE 13
#define SO_CONDITION_SYSTEM_FEATURE 14

ComfyEngine::ScriptDispatchStatus ComfyEngine::scriptDispatch(Actor &actor, byte opcode, uint32 &pc) {
	switch (opcode) {
	case O_ACTOR_SET_POSITION:
	case O_ACTOR_ADD_POSITION: {
		Actor *target = actorResolve(scriptReadWord(pc), _currentActor); pc += 2;
		int16 x = scriptReadStringIndex(pc); pc += 2;
		int16 y = scriptReadStringIndex(pc); pc += 2;
		byte randomize = scriptReadByte(pc++);

		if (!target)
			error("Script opcode 0x%02X resolved an invalid actor", opcode);

		if ((opcode == O_ACTOR_SET_POSITION && x == (int16)0xF830) || (opcode == O_ACTOR_ADD_POSITION && x == (int16)0x8AD0)) {
			Actor *other = actorResolve((uint16)y, _currentActor);
			if (!other)
				error("Script opcode 0x%02X resolved an invalid reference actor", opcode);

			x = (int16)(other->xFixed >> 12);
			y = (int16)(other->yFixed >> 12);
		} else if (opcode == O_ACTOR_SET_POSITION && x == (int16)0xF448) {
			x = (int16)(-target->xFixed >> 12);
		}

		if (opcode == O_ACTOR_SET_POSITION && y == (int16)0xF448)
			y = (int16)(-target->yFixed >> 12);

		if (randomize) {
			if (opcode == O_ACTOR_ADD_POSITION || x != (int16)0xFC18)
				x = (int16)(((int32)getRandomNumber(0x7FFF) * x) / 0x8000);

			if (opcode == O_ACTOR_ADD_POSITION || y != (int16)0xFC18)
				y = (int16)(((int32)getRandomNumber(0x7FFF) * y) / 0x8000);
		}


		if (opcode == O_ACTOR_SET_POSITION) {
			if (x != (int16)0xFC18)
				target->xFixed = (int32)x * 0x1000;

			if (y != (int16)0xFC18)
				target->yFixed = (int32)y * 0x1000;
		} else {
			target->xFixed = (int32)((uint32)target->xFixed + (uint32)((int32)x * 0x1000));
			target->yFixed = (int32)((uint32)target->yFixed + (uint32)((int32)y * 0x1000));
		}


		return kScriptContinue;
	}

	case O_ACTOR_SET_FRAME: {
		uint16 actorSlot = scriptReadWord(pc);
		Actor *target = actorResolve(actorSlot, _currentActor);
		pc += 2;
		uint16 frame = scriptReadStringIndex(pc);
		pc += 2;
		if (target) {
			target->spriteSelector = (uint32)(int32)(int16)frame;
		}

		return kScriptContinue;
	}

	case O_WAIT_KEY_SET: {
		uint16 key = scriptReadWord(pc);
		if (key && !keyBitTest(key)) {
			pc--;
			return kScriptYield;
		}

		pc += 2;
		return kScriptContinue;
	}

	case O_JUMP_IF_KEY_STATE:
	case O_ACTOR_CALL_IF_KEY_STATE: {
		uint16 key = scriptReadWord(pc);
		byte state = scriptReadByte(pc + 2);
		uint32 targetPc = scriptReadDword(pc + 3);
		pc += 7;
		if (!key || (byte)keyBitTest(key) == state) {
			if (opcode == O_ACTOR_CALL_IF_KEY_STATE)
				actorSetPc(actor, pc);

			pc = targetPc;
		}

		return kScriptContinue;
	}

	case O_SET_KEY_STATE: {
		uint16 key = scriptReadWord(pc);
		byte state = scriptReadByte(pc + 2);
		pc += 3;
		if (state == 1)
			keyBitSet(key);
		else
			keyBitClear(key);

		return kScriptContinue;
	}

	case O_ACTOR_DESTROY: {
		Actor *target = actorResolve(scriptReadWord(pc), _currentActor);
		pc += 2;
		if (!target)
			error("Script opcode 0x07 resolved an invalid actor");

		bool current = target == &actor;
		uint16 actorIndex = (uint16)(target - &_actors[0]);
		actorUnlink(actorIndex);
		actorFreeTreePc(actorIndex);
		_actorDestroyedCurrent = current;
		return current ? kScriptYield : kScriptContinue;
	}

	case O_ACTOR_CREATE: {
		uint16 sceneSlot = scriptReadWord(pc);
		uint16 parentSlot = scriptReadWord(pc + 2);
		byte flags = scriptReadByte(pc + 4);
		uint32 newPc = scriptReadDword(pc + 5);
		pc += 9;
		int16 x = scriptReadStringIndex(pc);
		pc += 2;
		int16 y = scriptReadStringIndex(pc);
		pc += 2;
		int16 sprite = scriptReadWord(pc);
		pc += 2;
		if (!parentSlot)
			parentSlot = actor.sceneHandle;

		actorInit(sceneSlot, parentSlot, (flags & 1) != 0, (flags & 2) != 0,
			newPc, x, y, sprite, (flags & 4) != 0);
		return kScriptContinue;
	}

	case O_SET_CURRENT_ACTOR_FRAME: {
		int16 frame = scriptReadWord(pc);
		actorSetFrame(frame);
		pc += 2;
		if (!_usesAnimFile && _videoMode == 2)
			renderAddFullFrameDirtyRect();

		return kScriptContinue;
	}

	case O_ACTOR_SHOW:
	case O_ACTOR_HIDE:
	case O_ACTOR_ACTIVATE:
	case O_ACTOR_DEACTIVATE: {
		Actor *target = actorResolve(scriptReadWord(pc), _currentActor);
		pc += 2;
		if (!target)
			error("Script opcode 0x%02X resolved an invalid actor", opcode);

		switch (opcode) {
		case O_ACTOR_SHOW:
			target->visible = 1;
			break;
		case O_ACTOR_HIDE:
			target->visible = 0;
			break;
		case O_ACTOR_ACTIVATE:
			target->active = 1;
			break;
		case O_ACTOR_DEACTIVATE:
			target->moveTicks = 0;
			target->active = 0;
			if (target == &actor)
				return kScriptYield;

			break;
		default:
			break;
		}

		return kScriptContinue;
	}

	case O_ACTOR_COMPARE_X_SET_KEY:
	case O_ACTOR_COMPARE_Y_SET_KEY:
	case O_ACTOR_COMPARE_X_CLEAR_KEY:
	case O_ACTOR_COMPARE_Y_CLEAR_KEY: {
		Actor *target = actorResolve(scriptReadWord(pc), _currentActor);
		int16 expected = scriptReadWord(pc + 2);
		uint16 key = scriptReadWord(pc + 4);
		byte mode = scriptReadByte(pc + 6);
		pc += 7;
		if (!target)
			error("Script opcode 0x%02X resolved an invalid actor", opcode);

		int16 actual = (opcode == O_ACTOR_COMPARE_X_SET_KEY || opcode == O_ACTOR_COMPARE_X_CLEAR_KEY ? target->xFixed : target->yFixed) >> 12;
		bool matched = ((mode & 2) && expected == actual) || ((mode & 4) && expected < actual) || ((mode & 1) && expected > actual);
		if (matched) {
			switch (opcode) {
			case O_ACTOR_COMPARE_X_SET_KEY:
			case O_ACTOR_COMPARE_Y_SET_KEY:
				keyBitSet(key);
				break;
			case O_ACTOR_COMPARE_X_CLEAR_KEY:
			case O_ACTOR_COMPARE_Y_CLEAR_KEY:
				keyBitClear(key);
				break;
			default:
				break;
			}
		}

		return kScriptContinue;
	}

	case O_MIDI_ADD_TRACK: {
		uint16 id = scriptReadWord(pc);
		int16 delta = scriptReadStringIndex(pc + 2);
		pc += 4;
		midiAddTrack(id, delta);
		return kScriptContinue;
	}

	case O_RANDOM_JUMP: {
		byte totalWeight = scriptReadByte(pc++);
		byte installReturnPc = scriptReadByte(pc++);
		uint32 returnPc = pc + installReturnPc - 3;
		byte choice = (byte)((getRandomNumber(0x7FFF) * totalWeight) / 0x8000);
		byte accumulated = 0;
		uint32 targetPc = 0;
		do {
			accumulated += scriptReadByte(pc++);
			targetPc = scriptReadDword(pc);
			pc += 4;
		} while (choice >= accumulated && !_scriptFault);

		if (installReturnPc)
			actorSetPc(actor, returnPc);

		pc = targetPc;
		return kScriptContinue;
	}

	case O_ACTOR_REORDER: {
		Actor *target = actorResolve(scriptReadWord(pc), _currentActor);
		byte mode = scriptReadByte(pc + 2);
		pc += 3;
		if (!target)
			error("Script opcode 0x14 resolved an invalid actor");

		uint16 targetIndex = (uint16)(target - &_actors[0]);
		uint16 oldParent = target->parent;
		actorUnlink(targetIndex);

		if (mode == 'O')
			actorInsertChild(targetIndex, oldParent);
		else
			actorInsertSibling(targetIndex, oldParent);

		if (_usesAnimFile && (_videoMode == 2 || _videoMode == 4))
			renderInvalidateFullFrame();
		else if (!_usesAnimFile && _videoMode == 2)
			renderAddFullFrameDirtyRect();

		return kScriptContinue;
	}

	case O_LOAD_SPRITE_CONV_LIST:
	case O_LOAD_SOUND_LIST: {
		byte count = scriptReadByte(pc++);
		for (uint i = 0; i < count; i++) {
			uint16 id = scriptReadWord(pc);
			pc += 2;
			switch (opcode) {
			case O_LOAD_SPRITE_CONV_LIST:
				spriteGetConvPtr((int16)id);
				break;
			case O_LOAD_SOUND_LIST:
				soundLoadEntry(id);
				break;
			default:
				break;
			}
		}

		return kScriptContinue;
	}

	case O_QUEUE_VOC: {
		if (_musicEventMask) {
			pc--;
			return kScriptYield;
		}

		uint16 soundId = scriptReadWord(pc);
		pc += 2;
		byte count = scriptReadByte(pc++);
		vocQueuePush(soundId, count, pc);
		pc += (uint32)count * 2;
		return kScriptContinue;
	}

	case O_ACTOR_RESET: {
		Actor *target = actorResolve(scriptReadWord(pc), _currentActor);
		pc += 2;
		if (!target)
			error("Script opcode 0x18 resolved an invalid actor");

		uint32 resetPc = target->resetPc;
		target->currentPc = resetPc;
		target->active = 1;
		target->waitTarget = 0;
		target->waitAccum = 0;
		if (target->moveTicks) {
			target->moveTicks = 0;
			uint16 completionKey = target->completionKey;
			if (completionKey)
				keyBitSet(completionKey);
		}

		actorFreePcChain(*target);

		if (target == &actor)
			pc = resetPc;

		return kScriptContinue;
	}

	case O_ACTOR_RETURN: {
		pc = actorPopPc(actor);
		return kScriptContinue;
	}

	case O_PLAY_VOC_QUEUE: {
		vocQueuePlayAll();
		return kScriptContinue;
	}

	case O_MIDI_REMOVE_TRACK: {
		midiRemoveTrack(scriptReadWord(pc));
		pc += 2;
		return kScriptContinue;
	}

	case O_MIDI_CHANNEL_CONTROL: {
		byte packed = scriptReadByte(pc++);
		byte channel = (packed >> 4) - 1;
		byte subop = packed & 0x0F;
		switch (subop) {
		case SO_MIDI_LOAD_TRACK: {
			uint16 songId = scriptReadWord(pc);
			uint16 completionKey = scriptReadWord(pc + 2);
			uint16 count = scriptReadWord(pc + 4);
			pc += 6;
			uint16 frames[COMFY_ANIM_FRAME_CAPACITY];
			memset(frames, 0, sizeof(frames));

			for (uint i = 0; i < count; i++) {
				uint16 frame = scriptReadWord(pc);
				pc += 2;
				if (i < COMFY_ANIM_FRAME_CAPACITY)
					frames[i] = frame;
			}

			midiAddTrackEntry(channel, songId, completionKey, 1, count, frames);
			break;
		}
		case SO_MIDI_SET_RATE:
		case SO_MIDI_SET_PITCH:
		case SO_MIDI_SET_VOLUME: {
			uint16 value = _engineVersion == 1 ? scriptReadWord(pc) : scriptReadStringIndex(pc);
			uint16 ticks = _engineVersion == 1 ? scriptReadWord(pc + 2) : scriptReadStringIndex(pc + 2);
			pc += 4;
			midiSetChannelParam(channel, subop, value, ticks);
			break;
		}
		case SO_MIDI_STOP_AND_ADVANCE:
			if (channel < COMFY_MIDI_CHANNEL_COUNT)
				midiStopAndAdvanceChannel(channel);
			break;
		case SO_MIDI_STOP_AND_FIRE_KEYS:
			if (channel < COMFY_MIDI_CHANNEL_COUNT)
				midiStopAndFireKeys(channel);
			break;
		case SO_MIDI_ENABLE_LOOP:
		case SO_MIDI_DISABLE_LOOP:
			if (_engineVersion == 3 && channel < COMFY_MIDI_CHANNEL_COUNT && _midiPlyrDriver)
				_midiPlyrDriver->musicSetLoop(subop == SO_MIDI_ENABLE_LOOP ? 1 : 0, channel);

			break;
		default:
			error("Unknown script opcode 0x1D subopcode 0x%02X at script PC 0x%08X", subop, pc - 2);
		}

		return kScriptContinue;
	}

	case O_ACTOR_SET_TRIGGER: {
		Actor *target = actorResolve(scriptReadWord(pc), _currentActor);
		uint16 key = scriptReadWord(pc + 2);
		byte flags = scriptReadByte(pc + 4);
		uint32 triggerPc = scriptReadDword(pc + 5);
		pc += 9;
		if (!target)
			error("Script opcode 0x1E resolved an invalid actor");

		if (key != 0xFC18)
			target->triggerKey = key;

		if (triggerPc != 0xFFFFFC18) {
			target->triggerPc = triggerPc;
			target->triggerFlags = flags;
		} else {
			target->triggerFlags = (target->triggerFlags & 1) | flags;
		}

		return kScriptContinue;
	}

	case O_SET_MUSIC_EVENT: {
		uint16 bit = scriptReadWord(pc);
		pc += 2;
		if ((int16)bit >= 0x32) {
			bit -= 0x32;
			_musicEventFlag = 1;
		}

		_musicEventMask |= (uint16)(1U << (bit & 0x1F));

		return kScriptYield;
	}

	case O_REQUEST_SCENE: {
		uint16 scene = scriptReadWord(pc);
		pc += 2;
		if (!_pendingScene || (int16)scene < (int16)_pendingScene)
			_pendingScene = scene;

		if (_videoMode == 2 || (_usesAnimFile && _videoMode == 4))
			renderRequestFullFrameInvalidation();

		return kScriptYield;
	}

	case O_PALETTE_CONTROL: {
		uint16 paletteId = scriptReadStringIndex(pc);
		uint16 fadeTicks = scriptReadStringIndex(pc + 2);
		byte brightness = scriptReadByte(pc + 4);
		pc += 5;
		if (!brightness) {
			actorSetPos(1, (int16)paletteId);
			paletteLoadWithFade(paletteId, fadeTicks);
		} else {
			actorSetPos(2, (int16)brightness);
			paletteApplyBrightness(brightness);
		}

		return kScriptContinue;
	}

	case O_SET_MUSIC_ENABLED: {
		musicSetEnabled(scriptReadByte(pc));
		pc++;
		return kScriptContinue;
	}

	case O_SCRIPT_VARIABLE_ADD: {
		uint16 handle = scriptReadWord(pc);
		int16 delta = scriptReadStringIndex(pc + 2);
		pc += 4;
		scriptVariableAddTo(handle, delta);

		return kScriptContinue;
	}

	case O_MIDI_SET_TIME_SCALE: {
		midiSetTimeScale((int16)scriptReadStringIndex(pc));
		pc += 2;
		return kScriptContinue;
	}

	case O_CLEAR_KEY_RANGE: {
		uint16 first = scriptReadWord(pc);
		uint16 last = scriptReadWord(pc + 2);
		pc += 4;
		for (uint16 key = first; key <= last; key++) {
			keyBitClear(key);
			if (key == 0xFFFF)
				break;
		}

		return kScriptContinue;
	}

	case O_SCRIPT_VARIABLE_GE_VALUE_SET_KEY:
	case O_SCRIPT_VARIABLE_GE_VARIABLE_SET_KEY: {
		uint16 first = scriptReadWord(pc);
		uint16 second = scriptReadWord(pc + 2);
		uint16 key = scriptReadWord(pc + 4);
		pc += 6;
		int16 lhs = (int16)scriptVariableGet(first);
		int16 rhs = opcode == O_SCRIPT_VARIABLE_GE_VALUE_SET_KEY ? (int16)second : (int16)scriptVariableGet(second);
		if (lhs >= rhs)
			keyBitSet(key);

		return kScriptContinue;
	}

	case O_SKIP_WORD_28:
	case O_SKIP_WORD_29: {
		pc += 2;
		return kScriptContinue;
	}

	case O_ACTOR_SET_SPRITE_SELECTOR: {
		Actor *target = actorResolve(scriptReadWord(pc), _currentActor);
		uint32 selector = scriptReadDword(pc + 2);
		pc += 6;
		if (target)
			target->spriteSelector = (selector + 1) | 0xFF000000;

		return kScriptContinue;
	}

	case O_JUMP_RELATIVE: {
		pc = (uint32)((int32)pc + (int16)(scriptReadWord(pc) * 2));
		return kScriptContinue;
	}

	case O_COPY_KEY: {
		uint16 destination = scriptReadWord(pc);
		uint16 source = scriptReadWord(pc + 2);
		pc += 4;

		if (keyBitTest(source))
			keyBitSet(destination);
		else
			keyBitClear(destination);

		return kScriptContinue;
	}

	case O_SCENE_CONTROL: {
		byte subop = scriptReadByte(pc++);
		switch (subop) {
		case SO_SCENE_ENTRY_INIT:
			sceneEntryInit(scriptReadWord(pc));
			pc += 2;
			break;
		case SO_SCENE_ENTRY_STOP:
			sceneEntryStop();
			break;
		case SO_SCENE_ENTRY_LOAD: {
			uint16 descriptor = scriptReadWord(pc);
			uint16 index = _engineVersion == 1 ? scriptReadWord(pc + 2) : scriptReadStringIndex(pc + 2);
			pc += 4;
			sceneEntryLoad(descriptor, index);
			break;
		}
		case SO_SCENE_MIDI_PLAY_AT_FRAME: {
			uint16 channel = _engineVersion == 1 ? scriptReadWord(pc) : scriptReadStringIndex(pc);
			uint16 frame = _engineVersion == 1 ? scriptReadWord(pc + 2) : scriptReadStringIndex(pc + 2);
			pc += 4;
			midiPlaySongAtFrame(channel, frame);
			break;
		}
		case SO_SCENE_MIDI_STOP: {
			uint16 channel = _engineVersion == 1 ? scriptReadWord(pc) : scriptReadStringIndex(pc);
			pc += 2;
			midiStopSong(channel);
			break;
		}
		case SO_SCENE_MIDI_SET_VOLUME:
			if (_engineVersion >= 2) {
				uint16 channel = scriptReadStringIndex(pc);
				uint16 volume = scriptReadStringIndex(pc + 2);
				pc += 6;
				sceneEntrySetVolume(channel, volume);
			}

			break;
		default:
			error("Unknown script opcode 0x2D subopcode 0x%02X at script PC 0x%08X", subop, pc - 2);
		}

		return kScriptContinue;
	}

	case O_SCRIPT_VARIABLE_COPY: {
		byte command = scriptReadByte(pc);
		uint16 destination = scriptReadWord(pc + 1);
		uint16 first = scriptReadWord(pc + 3);
		uint16 second = scriptReadWord(pc + 5);
		pc += 7;
		if (command == '=')
			scriptVariableCopy(destination, first);
		else if (command == 'm')
			scriptVariableCopy(destination,
				(int16)scriptVariableGet(second) <= (int16)scriptVariableGet(first) ? first : second);

		return kScriptContinue;
	}

	case O_ANIMATION_CONTROL:
		if (!_usesAnimFile)
			error("Unknown script opcode 0x%02X at script PC 0x%08X", opcode, pc - 1);
	{
		byte subop = scriptReadByte(pc++);
		switch (subop) {
		case SO_ANIM_SHUTDOWN:
			animFrameShutdown(true);
			break;
		case SO_ANIM_LOAD_FRAME: {
			uint16 animIndex = _engineVersion == 3 ? scriptReadStringIndex(pc) : scriptReadWord(pc);
			uint16 frameKey = scriptReadWord(pc + 2);
			pc += 4;

			if (_engineVersion == 3) {
				animIndex += _v3AnimIndexOffset;
				_v3AnimIndexOffset = 0;

				if (!animFrameIsActive()) {
					if (!_v3AnimActorSceneHandle)
						_v3AnimActorSceneHandle = actor.sceneHandle;

					animFileLoadFrame(animIndex, frameKey, _v3AnimActorSceneHandle);
					_v3AnimActorSceneHandle = 0;
				}
			} else {
				animFileLoadFrame(animIndex, frameKey, actor.sceneHandle);
			}

			break;
		}
		case SO_ANIM_SET_READY:
			animFrameSetReady(true);
			break;
		case SO_ANIM_CLEAR_READY:
			animFrameSetReady(false);
			break;
		case SO_ANIM_NOOP_4:
		case SO_ANIM_NOOP_5:
			break;
		case SO_ANIM_INVALIDATE_ACTOR_RECT:
			if (_engineVersion == 3)
				animFrameInvalidateActorRect();

			break;
		default:
			error("Unknown script opcode 0x30 subopcode 0x%02X at script PC 0x%08X", subop, pc - 2);
		}

		return kScriptContinue;
	}

	case O_SET_LAST_VOC_ARGUMENTS:
	case O_UNUSED_32:
	case O_SKIP_TABLE_BLOCK:
	case O_CONFIG_STRING_COMMAND:
	case O_MOUSE_SET_ACTOR:
	case O_HOST_MEDIA_CONTROL:
		if (!((_engineVersion == 3) ||
			(opcode == O_SET_LAST_VOC_ARGUMENTS && (_game->scriptFeatures & COMFY_SCRIPT_OPCODE_31)) ||
			(opcode == O_UNUSED_32 && (_game->scriptFeatures & COMFY_SCRIPT_OPCODE_32)) ||
			(opcode == O_SKIP_TABLE_BLOCK && (_game->scriptFeatures & COMFY_SCRIPT_OPCODE_33))))
			error("Unknown script opcode 0x%02X at script PC 0x%08X", opcode, pc - 1);
	{
		switch (opcode) {
		case O_SET_LAST_VOC_ARGUMENTS: {
			uint16 count = scriptReadStringIndex(pc);
			pc += 2;
			if (_animUsesWaveVocCounter) {
				uint16 slot = _soundEventMaximum ? _soundEventMaximum - 1 : _vocQueueCapacity - 1;
				if (_engineVersion == 3) {
					VocQueueEntryV3 &entry = _vocQueueV3[slot];
					entry.argumentCount = MIN<uint16>(count, COMFY_VOC_ARG_CAPACITY_V3);
					for (uint i = 0; i < entry.argumentCount; i++)
						entry.arguments[i] = scriptReadWord(pc + i * 2);
				} else {
					VocQueueEntry &entry = _vocQueue[slot];
					entry.argumentCount = MIN<uint16>(count, COMFY_VOC_ARG_CAPACITY);
					for (uint i = 0; i < entry.argumentCount; i++)
						entry.arguments[i] = scriptReadWord(pc + i * 2);
				}
			}

			pc += (uint32)count * 2;

			break;
		}
		case O_UNUSED_32:
			scriptReadStringIndex(pc);
			scriptReadStringIndex(pc + 2);
			pc += 4;
			break;
		case O_SKIP_TABLE_BLOCK: {
			pc += 4;
			byte inlineBytes = scriptReadByte(pc++);
			byte tripleCount = scriptReadByte(pc++);
			pc += inlineBytes + (uint32)tripleCount * 3;
			break;
		}
		case O_CONFIG_STRING_COMMAND: {
			byte subop = scriptReadByte(pc++);
			uint16 value = scriptReadWord(pc);
			pc += 2;
			Common::String key;
			for (byte character = scriptReadByte(pc++); character; character = scriptReadByte(pc++))
				key += (char)character;

			switch (subop) {
			case SO_CONFIG_READ_INT:
				if (value < _stringTable.size())
					_stringTable[value] = ConfMan.hasKey(key) ? ConfMan.getInt(key) : 0;

				break;
			case SO_CONFIG_WRITE_INT:
				ConfMan.setInt(key, value);
				if (key.equalsIgnoreCase("SENSITIVITY"))
					_v3Sensitivity = value;

				ConfMan.flushToDisk();
				break;
			case SO_CONFIG_SUSPEND_ACTOR:
			case SO_CONFIG_RUN_COMMAND:
				_v3ConfigCommandPending = true;
				_v3ConfigCommandSubop = subop;
				_v3ConfigCommandValue = value;
				_v3ConfigCommandText = key;

				if (subop == SO_CONFIG_SUSPEND_ACTOR) {
					Actor *configActor = actorGetPtr(sceneGetHandle(1));
					if (configActor)
						configActor->active = 0;
				} else {
					configRunPendingCommand();
				}

				break;
			case SO_CONFIG_TEST_VALUE:
				if (ConfMan.hasKey(key) && !ConfMan.get(key).equalsIgnoreCase("?????"))
					keyBitSet(value);
				else
					keyBitClear(value);

				break;
			case SO_CONFIG_TEST_FILE: {
				Common::SeekableReadStream *stream = pathFOpen(Common::Path(key), true);
				if (stream)
					keyBitSet(value);
				else
					keyBitClear(value);

				delete stream;
				break;
			}
			case SO_CONFIG_SET_LANGUAGE_DATA_SUBDIRECTORY:
				if (value < _stringTable.size())
					_stringTable[value] = languageSetDataSubdirectory(key);

				break;
			default:
				error("Unknown script opcode 0x34 subopcode 0x%02X near script PC 0x%08X", subop, pc);
			}

			break;
		}
		case O_MOUSE_SET_ACTOR: {
			uint32 opcodePc = pc - 1;
			byte subop = 0;
			Actor *mouseActor = nullptr;
			pc = scriptReadArgs(pc, _currentActor, "CO", &subop, &mouseActor);
			switch (subop) {
			case SO_MOUSE_SET_ACTOR:
				mouseSetActor(mouseActor);
				break;
			case SO_MOUSE_CLEAR_ACTOR:
				mouseSetActor(nullptr);
				break;
			default:
				error("Unknown script opcode 0x35 subopcode 0x%02X at script PC 0x%08X", subop, opcodePc);
			}

			break;
		}
		case O_HOST_MEDIA_CONTROL: {
			byte subop = scriptReadByte(pc++);
			switch (subop) {
			case SO_HOST_MEDIA_START_INPUT:
				setMediaMode(subop);
				break;
			case SO_HOST_MEDIA_STOP_INPUT:
				setMediaMode(subop);
				_v3MediaRecording = false;
				break;
			case SO_HOST_MEDIA_START_RECORDING_2_ARGS:
			case SO_HOST_MEDIA_START_RECORDING_3_ARGS:
			case SO_HOST_MEDIA_START_RECORDING_4_ARGS:
				_v3MediaRecordingArg0 = scriptReadStringIndex(pc);
				_v3MediaRecordingArg1 = scriptReadWord(pc + 2);
				pc += 4;
				_v3MediaRecording = true;
				if (!_v3SceneMediaModeEnabled)
					setMediaMode(SO_HOST_MEDIA_START_INPUT);

				if (subop != SO_HOST_MEDIA_START_RECORDING_2_ARGS) {
					_v3MediaRecordingArg2 = scriptReadStringIndex(pc);
					pc += 2;
				}

				if (subop == SO_HOST_MEDIA_START_RECORDING_4_ARGS) {
					_v3MediaRecordingArg3 = scriptReadStringIndex(pc);
					pc += 2;
				}

				break;
			default:
				error("Unknown script opcode 0x36 subopcode 0x%02X at script PC 0x%08X", subop, pc - 2);
			}

			break;
		}
		default:
			break;
		}

		return kScriptContinue;
	}

	case O_ANIMATION_AUDIO_CONTROL:
	case O_PRELOAD_SPRITE_CONV_RANGE:
		if (_engineVersion != 3)
			error("Unknown script opcode 0x%02X at script PC 0x%08X", opcode, pc - 1);
	{
		switch (opcode) {
		case O_ANIMATION_AUDIO_CONTROL: {
			byte subop = scriptReadByte(pc++);
			if (subop < SO_ANIMATION_INDEX_OFFSET || subop > SO_MIXER_VOLUME)
				error("Unknown script opcode 0x46 subopcode 0x%02X at script PC 0x%08X", subop, pc - 2);

			switch (subop) {
			case SO_ANIMATION_INDEX_OFFSET:
				_v3AnimIndexOffset = scriptReadStringIndex(pc);
				pc += 2;
				break;
			case SO_WAVE_BALANCE:
				setWaveBalancePercent(scriptReadStringIndex(pc));
				pc += 2;
				break;
			case SO_ANIMATION_ACTOR_SCENE:
				_v3AnimActorSceneHandle = scriptReadWord(pc);
				pc += 2;
				break;
			case SO_WAVE_LEFT:
				setWaveLeftPercent(scriptReadStringIndex(pc));
				pc += 2;
				break;
			case SO_WAVE_RIGHT:
				setWaveRightPercent(scriptReadStringIndex(pc));
				pc += 2;
				break;
			case SO_ANIMATION_AUDIO_NOOP:
				break;
			case SO_HOST_MEDIA_RANGE:
				setMediaRangePercent(scriptReadStringIndex(pc));
				pc += 2;
				break;
			case SO_MIXER_VOLUME:
				setMixerVolumePercent(scriptReadStringIndex(pc));
				pc += 2;
				break;
			default:
				break;
			}

			break;
		}
		case O_PRELOAD_SPRITE_CONV_RANGE: {
			uint16 first = scriptReadStringIndex(pc);
			uint16 last = scriptReadStringIndex(pc + 2);
			pc += 4;
			for (uint16 spriteId = first; (int16)spriteId <= (int16)last; spriteId++)
				spriteGetConvPtr((int16)spriteId);

			break;
		}
		default:
			break;
		}

		return kScriptContinue;
	}

	case O_STRING_TABLE_COMPARE_SET_KEY: {
		uint16 lhsIndex = scriptReadWord(pc);
		uint16 rhsIndex = scriptReadWord(pc + 2);
		uint16 key = scriptReadWord(pc + 4);
		byte mode = scriptReadByte(pc + 6);
		pc += 7;
		int16 lhs = lhsIndex < _stringTable.size() ? _stringTable[lhsIndex] : 0;
		int16 rhs = rhsIndex < _stringTable.size() ? _stringTable[rhsIndex] : 0;
		bool matched = ((mode & 4) && lhs > rhs) || ((mode & 2) && lhs == rhs) || ((mode & 1) && lhs < rhs);
		if (matched)
			keyBitSet(key);
		else
			keyBitClear(key);

		return kScriptContinue;
	}

	case O_ACTOR_MOVE_BY:
	case O_ACTOR_MOVE_TO:
	case O_ACTOR_MOVE_BY_V2:
	case O_ACTOR_MOVE_TO_V2: {
		Actor *target = actorResolve(scriptReadWord(pc), _currentActor);
		pc += 2;
		int16 x = scriptReadStringIndex(pc);
		pc += 2;
		int16 y = scriptReadStringIndex(pc);
		pc += 2;
		byte randomize = scriptReadByte(pc++);
		int16 duration = scriptReadStringIndex(pc);
		pc += 2;
		byte blocking = scriptReadByte(pc++);
		uint16 completionKey = scriptReadWord(pc);
		pc += 2;
		if (!target)
			error("Script opcode 0x%02X resolved an invalid actor", opcode);

		if (!duration && (opcode == O_ACTOR_MOVE_BY || opcode == O_ACTOR_MOVE_TO))
			error("Script opcode 0x%02X has a zero movement duration", opcode);

		bool moveTo = opcode == O_ACTOR_MOVE_TO || opcode == O_ACTOR_MOVE_TO_V2;
		if ((moveTo && x == (int16)0xF830) || (!moveTo && x == (int16)0x8AD0)) {
			Actor *other = actorResolve((uint16)y, _currentActor);
			if (!other)
				error("Script opcode 0x%02X resolved an invalid reference actor", opcode);

			if (moveTo) {
				x = (int16)(other->xFixed >> 12);
				y = (int16)(other->yFixed >> 12);
			} else {
				x = (int16)(uint16)other->xFixed >> 12;
				y = (int16)(uint16)other->yFixed >> 12;
			}
		}

		if (randomize) {
			if (!moveTo || x != (int16)0xFC18)
				x = (int16)(((int32)getRandomNumber(0x7FFF) * x) / 0x8000);

			if (!moveTo || y != (int16)0xFC18)
				y = (int16)(((int32)getRandomNumber(0x7FFF) * y) / 0x8000);
		}

		int32 dx = (int32)x * 0x1000;
		int32 dy = (int32)y * 0x1000;
		if (moveTo) {
			dx = x == (int16)0xFC18 ? 0 : dx - target->xFixed;
			dy = y == (int16)0xFC18 ? 0 : dy - target->yFixed;
		}

		if (opcode == O_ACTOR_MOVE_BY || opcode == O_ACTOR_MOVE_TO || (moveTo && duration < 0)) {
			if (duration < 0)
				duration = -duration;

			int16 distanceX = ABS((int16)(dx >> 12));
			int16 distanceY = ABS((int16)(dy >> 12));
			int16 distance = MAX(distanceX, distanceY);
			duration = (int16)(((int32)distance * 100) / duration);
			if (!duration)
				duration = 1;
		}

		target->moveDx = dx;
		target->moveDy = dy;
		target->moveTicks = duration;
		target->blockingMove = blocking;
		target->completionKey = completionKey;
		return blocking && target == &actor ? kScriptYield : kScriptContinue;
	}

	case O_JUMP_IF_KEY_CLEAR_V3:
	case O_JUMP_IF_KEY_SET_V3:
		if (_engineVersion != 3)
			error("Unknown script opcode 0x%02X at script PC 0x%08X", opcode, pc - 1);
	{
		uint16 key = scriptReadWord(pc);
		uint32 targetPc = scriptReadDword(pc + 2);
		bool jumpWhenSet = opcode == O_JUMP_IF_KEY_SET_V3;
		pc = keyBitTest(key) == jumpWhenSet ? targetPc : pc + 6;
		return kScriptContinue;
	}

	case O_ACTOR_SET_FRAME_AND_STOP: {
		uint16 actorSlot = scriptReadWord(pc);
		Actor *target = actorResolve(actorSlot, _currentActor);
		uint16 frame = scriptReadStringIndex(pc + 2);

		pc += 4;

		if (!target)
			error("Script opcode 0x53 resolved an invalid actor");

		target->spriteSelector = (uint32)(int32)(int16)frame;
		target->moveDx = 0;
		target->moveDy = 0;
		return kScriptContinue;
	}

	case O_COPY_INLINE_BLOCK: {
		if (_engineVersion < 2)
			error("Unknown script opcode 0x%02X at script PC 0x%08X", opcode, pc - 1);

		uint32 endPc = scriptReadDword(pc);
		pc += 4;
		uint32 size = endPc >= pc ? endPc - pc : 0;
		size = MIN<uint32>(size, COMFY_FRAME_LOADER_DATA_BYTES);
		if (_comfyObjFile && pc <= _comfyObjFile->fileSize && size <= _comfyObjFile->fileSize - pc) {
			_frameLoaderData.resize(COMFY_FRAME_LOADER_DATA_BYTES);
			if (size)
				objFileReadField(&_frameLoaderData[0], pc, size, _comfyObjFile);
		} else {
			error("Script opcode 0x54 references data outside COMFY.OBJ");
		}

		return kScriptContinue;
	}

	case O_COPY_PALETTE_ENTRY: {
		if (_engineVersion < 2)
			error("Unknown script opcode 0x%02X at script PC 0x%08X", opcode, pc - 1);

		uint16 source = scriptReadStringIndex(pc);
		uint16 destination = scriptReadStringIndex(pc + 2);
		pc += 4;
		if (source < 256 && destination < 256)
			memcpy(&_paletteDisplay[destination * 3], &_paletteDisplay[source * 3], 3);

		_paletteFading = false;
		vsyncSetPalettePtr(_paletteDisplay);
		return kScriptContinue;
	}

	case O_WAIT:
		return kScriptYield;

	case O_MIDI_ADD_EVENT: {
		uint16 id = scriptReadWord(pc);
		int16 delta = scriptReadStringIndex(pc + 2);
		pc += 4;
		midiAddEvent(id, delta);
		return kScriptContinue;
	}

	case O_ACTOR_WAIT_TICKS: {
		uint16 requested = scriptReadStringIndex(pc);
		pc += 2;
		uint16 balance = actor.waitAccum;
		if (!requested)
			balance = 0;

		if (balance) {
			uint16 step = balance < 5 ? balance : 5;
			balance = step < (uint16)(requested - 1) ? step : (uint16)(requested - 1);
			actor.waitAccum -= balance;
		}

		actor.waitTarget = requested - balance;
		return kScriptYield;
	}

	case O_WAVE_OUTPUT_RESET: {
		_waveOutputActive = false;
		return kScriptContinue;
	}

	case O_WAIT_KEY_CLEAR: {
		uint16 key = scriptReadWord(pc);
		if (keyBitTest(key)) {
			pc--;
			return kScriptYield;
		}

		pc += 2;
		return kScriptContinue;
	}

	case O_CONDITIONAL_SET_KEY: {
		byte kind = scriptReadByte(pc++);
		uint16 key = scriptReadWord(pc);
		pc += 2;
		bool condition = false;
		if (_engineVersion == 3) {
			uint16 argument = scriptReadWord(pc);
			pc += 2;
			switch (kind) {
			case SO_CONDITION_SCENE_EXISTS:
				condition = sceneGetHandle(argument) != 0;
				break;
			case SO_CONDITION_INPUT_MODE:
				condition = _inputDeviceMode == 1;
				break;
			case SO_CONDITION_MOUSE_LEFT_BUTTON:
			case SO_CONDITION_MOUSE_LEFT_BUTTON_ALT:
				condition = _mouseLeftButton;
				break;
			case SO_CONDITION_MOUSE_RIGHT_BUTTON:
			case SO_CONDITION_MOUSE_RIGHT_BUTTON_ALT:
				condition = _mouseRightButton;
				break;
			case SO_CONDITION_ACTOR_HIT_TEST:
			case SO_CONDITION_ACTOR_BLIT_HIT_TEST: {
				Actor *target = actorResolve(argument, _currentActor);
				condition = target && actorTestMouseHit(*target, kind == SO_CONDITION_ACTOR_BLIT_HIT_TEST);
				break;
			}
			case SO_CONDITION_FALSE:
				break;
			case SO_CONDITION_SYSTEM_FEATURE:
				if (argument <= 4) {
					condition = _inputDeviceMode == argument;
				} else if (argument == 20) {
					condition = _v3MediaRecording;
				} else if (argument == 30) {
					Common::SeekableReadStream *stream = pathFOpen(Common::Path("comfy.htm"), true);
					condition = stream != nullptr;
					delete stream;
				}
				break;
			default:
				error("Unknown script opcode 0x75 subopcode 0x%02X at script PC 0x%08X", kind, pc - 6);
			}
		} else {
			switch (kind) {
			case SO_CONDITION_SCENE_EXISTS: {
				uint16 scene = scriptReadWord(pc);
				condition = sceneGetHandle(scene) != 0;
				pc += 2;
				break;
			}
			default:
				error("Unknown script opcode 0x75 subopcode 0x%02X at script PC 0x%08X", kind, pc - 4);
			}
		}

		if (condition)
			keyBitSet(key);
		else
			keyBitClear(key);

		return kScriptContinue;
	}

	case O_STRING_TABLE_SET:
	case O_STRING_TABLE_SET_OFFSET: {
		uint16 index = scriptReadWord(pc);
		pc += 2;
		if (opcode == O_STRING_TABLE_SET_OFFSET) {
			if (_stringTable.size() <= 1)
				error("Script opcode 0x77 requires string table entry 1");

			index += _stringTable[1];
		}

		uint16 value = scriptEvalExpr(pc, _currentActor);
		if (index >= _stringTable.size())
			error("Script opcode 0x%02X references invalid string table entry %u", opcode, (uint)index);

		_stringTable[index] = value;
		return kScriptContinue;
	}

	case O_NOP:
		return kScriptContinue;

	default:
		if (opcode)
			error("Unknown script opcode 0x%02X at script PC 0x%08X", opcode, pc - 1);

		Actor *root = actorGetPtr(0);
		if (root)
			root->active = 0;

		return kScriptContinue;
	}

	error("Script opcode 0x%02X left the dispatch switch without returning", opcode);
	return kScriptContinue;
}

ComfyEngine::ScriptDispatchStatus ComfyEngine::scriptStep(Actor &actor, uint32 &pc) {
	uint32 originalPc = pc;
	_scriptFault = false;
	byte opcode = scriptReadByte(pc++);
	if (_scriptFault)
		error("Script fault while fetching opcode at script PC 0x%08X", originalPc);

	ScriptDispatchStatus status = scriptDispatch(actor, opcode, pc);
	if (_scriptFault)
		error("Script fault while executing opcode 0x%02X at script PC 0x%08X", opcode, originalPc);

	return status;
}


} // End of namespace Comfy
