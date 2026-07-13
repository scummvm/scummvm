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
#include "common/ptr.h"

namespace Comfy {

ComfyEngine::ScriptDispatchStatus ComfyEngine::scriptDispatch(Actor &actor, byte opcode, uint32 &pc) {
	switch (opcode) {
	case 0x01:
	case 0x02: {
		Actor *target = actorResolve(scriptReadWord(pc), _currentActor);
		pc += 2;
		int16 x = scriptReadStringIndex(pc);
		pc += 2;
		int16 y = scriptReadStringIndex(pc);
		pc += 2;
		byte randomize = scriptReadByte(pc++);
		if (!target)
			error("Script opcode 0x%02X resolved an invalid actor", opcode);

		if ((opcode == 0x01 && x == (int16)0xF830) || (opcode == 0x02 && x == (int16)0x8AD0)) {
			Actor *other = actorResolve((uint16)y, _currentActor);
			if (!other)
				error("Script opcode 0x%02X resolved an invalid reference actor", opcode);

			x = (int16)((int32)actorReadU32(*other, kActorXFixed) >> 12);
			y = (int16)((int32)actorReadU32(*other, kActorYFixed) >> 12);
		} else if (opcode == 0x01 && x == (int16)0xF448) {
			x = (int16)(-(int32)actorReadU32(*target, kActorXFixed) >> 12);
		}

		if (opcode == 0x01 && y == (int16)0xF448)
			y = (int16)(-(int32)actorReadU32(*target, kActorYFixed) >> 12);

		if (randomize) {
			if (opcode == 0x02 || x != (int16)0xFC18)
				x = (int16)(((int32)getRandomNumber(0x7FFF) * x) / 0x8000);

			if (opcode == 0x02 || y != (int16)0xFC18)
				y = (int16)(((int32)getRandomNumber(0x7FFF) * y) / 0x8000);
		}

		if (opcode == 0x01) {
			if (x != (int16)0xFC18)
				actorWriteU32(*target, kActorXFixed, (uint32)((int32)x * 0x1000));

			if (y != (int16)0xFC18)
				actorWriteU32(*target, kActorYFixed, (uint32)((int32)y * 0x1000));
		} else {
			actorWriteU32(*target, kActorXFixed, actorReadU32(*target, kActorXFixed) + (uint32)((int32)x * 0x1000));
			actorWriteU32(*target, kActorYFixed, actorReadU32(*target, kActorYFixed) + (uint32)((int32)y * 0x1000));
		}

		return kScriptContinue;
	}

	case 0x03: {
		Actor *target = actorResolve(scriptReadWord(pc), _currentActor);
		pc += 2;
		uint16 frame = scriptReadStringIndex(pc);
		pc += 2;
		if (target)
			actorWriteU32(*target, kActorSpriteSelector, (uint32)(int32)(int16)frame);

		return kScriptContinue;
	}

	case 0x04: {
		uint16 key = scriptReadWord(pc);
		if (key && !keyBitTest(key)) {
			pc--;
			return kScriptYield;
		}

		pc += 2;
		return kScriptContinue;
	}

	case 0x05:
	case 0x19: {
		uint16 key = scriptReadWord(pc);
		byte state = scriptReadByte(pc + 2);
		uint32 targetPc = scriptReadDword(pc + 3);
		pc += 7;
		if (!key || (byte)keyBitTest(key) == state) {
			if (opcode == 0x19)
				actorSetPc(actor, pc);

			pc = targetPc;
		}

		return kScriptContinue;
	}

	case 0x06: {
		uint16 key = scriptReadWord(pc);
		byte state = scriptReadByte(pc + 2);
		pc += 3;
		if (state == 1)
			keyBitSet(key);
		else
			keyBitClear(key);

		return kScriptContinue;
	}

	case 0x07: {
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

	case 0x08: {
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
			parentSlot = actorReadU16(actor, kActorSceneHandle);

		actorInit(sceneSlot, parentSlot, (flags & 1) != 0, (flags & 2) != 0,
			newPc, x, y, sprite, (flags & 4) != 0);
		return kScriptContinue;
	}

	case 0x09: {
		actorSetFrame((int16)scriptReadWord(pc));
		pc += 2;
		if (!_usesAnimFile && _videoMode == 2)
			videoSetResolution();

		return kScriptContinue;
	}

	case 0x0A:
	case 0x0B:
	case 0x0C:
	case 0x0D: {
		Actor *target = actorResolve(scriptReadWord(pc), _currentActor);
		pc += 2;
		if (!target)
			error("Script opcode 0x%02X resolved an invalid actor", opcode);

		switch (opcode) {
		case 0x0A:
			actorWriteU8(*target, kActorVisible, 1);
			break;
		case 0x0B:
			actorWriteU8(*target, kActorVisible, 0);
			break;
		case 0x0C:
			actorWriteU8(*target, kActorActive, 1);
			break;
		case 0x0D:
			actorWriteU16(*target, kActorMoveTicks, 0);
			actorWriteU8(*target, kActorActive, 0);
			if (target == &actor)
				return kScriptYield;

			break;
		default:
			break;
		}

		return kScriptContinue;
	}

	case 0x0E:
	case 0x0F:
	case 0x10:
	case 0x11: {
		Actor *target = actorResolve(scriptReadWord(pc), _currentActor);
		int16 expected = scriptReadWord(pc + 2);
		uint16 key = scriptReadWord(pc + 4);
		byte mode = scriptReadByte(pc + 6);
		pc += 7;
		if (!target)
			error("Script opcode 0x%02X resolved an invalid actor", opcode);

		int16 actual = (int32)actorReadU32(*target, opcode == 0x0E || opcode == 0x10 ? kActorXFixed : kActorYFixed) >> 12;
		bool matched = ((mode & 2) && expected == actual) || ((mode & 4) && expected < actual) || ((mode & 1) && expected > actual);
		if (matched) {
			switch (opcode) {
			case 0x0E:
			case 0x0F:
				keyBitSet(key);
				break;
			case 0x10:
			case 0x11:
				keyBitClear(key);
				break;
			default:
				break;
			}
		}

		return kScriptContinue;
	}

	case 0x12: {
		uint16 id = scriptReadWord(pc);
		int16 delta = scriptReadStringIndex(pc + 2);
		pc += 4;
		midiAddTrack(id, delta);
		return kScriptContinue;
	}

	case 0x13: {
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

	case 0x14: {
		Actor *target = actorResolve(scriptReadWord(pc), _currentActor);
		byte mode = scriptReadByte(pc + 2);
		pc += 3;
		if (!target)
			error("Script opcode 0x14 resolved an invalid actor");

		uint16 targetIndex = (uint16)(target - &_actors[0]);
		uint16 oldParent = actorReadU16(*target, kActorParent);
		actorUnlink(targetIndex);

		if (mode == 'O')
			actorInsertChild(targetIndex, oldParent);
		else
			actorInsertSibling(targetIndex, oldParent);

		if (_usesAnimFile && (_videoMode == 2 || _videoMode == 4))
			renderInvalidateFullFrame();
		else if (!_usesAnimFile && _videoMode == 2)
			videoSetResolution();

		return kScriptContinue;
	}

	case 0x15:
	case 0x16: {
		byte count = scriptReadByte(pc++);
		for (uint i = 0; i < count; i++) {
			uint16 id = scriptReadWord(pc);
			pc += 2;
			switch (opcode) {
			case 0x15:
				spriteGetConvPtr((int16)id);
				break;
			case 0x16:
				soundLoadEntry(id);
				break;
			default:
				break;
			}
		}

		return kScriptContinue;
	}

	case 0x17: {
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

	case 0x18: {
		Actor *target = actorResolve(scriptReadWord(pc), _currentActor);
		pc += 2;
		if (!target)
			error("Script opcode 0x18 resolved an invalid actor");

		uint32 resetPc = actorReadU32(*target, kActorResetPc);
		actorWriteU32(*target, kActorCurrentPc, resetPc);
		actorWriteU8(*target, kActorActive, 1);
		actorWriteU16(*target, kActorWaitTarget, 0);
		actorWriteU16(*target, kActorWaitAccum, 0);
		if (actorReadU16(*target, kActorMoveTicks)) {
			actorWriteU16(*target, kActorMoveTicks, 0);
			uint16 completionKey = actorReadU16(*target, kActorCompletionKey);
			if (completionKey)
				keyBitSet(completionKey);
		}

		actorFreePcChain(*target);

		if (target == &actor)
			pc = resetPc;

		return kScriptContinue;
	}

	case 0x1A: {
		pc = actorPopPc(actor);
		return kScriptContinue;
	}

	case 0x1B: {
		vocQueuePlayAll();
		return kScriptContinue;
	}

	case 0x1C: {
		midiRemoveTrack(scriptReadWord(pc));
		pc += 2;
		return kScriptContinue;
	}

	case 0x1D: {
		byte packed = scriptReadByte(pc++);
		byte channel = (packed >> 4) - 1;
		byte subop = packed & 0x0F;
		switch (subop) {
		case 1: {
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
		case 2:
		case 3:
		case 4: {
			uint16 value = scriptReadWord(pc);
			uint16 ticks = scriptReadWord(pc + 2);
			pc += 4;
			midiSetChannelParam(channel, subop, value, ticks);
			break;
		}
		case 5:
			if (channel < COMFY_MIDI_CHANNEL_COUNT)
				midiStopAndRemove(channel);
			break;
		case 6:
			if (channel < COMFY_MIDI_CHANNEL_COUNT)
				midiStopAndFireKeys(channel);
			break;
		case 7:
		case 8:
			if (_engineVersion == 3 && channel < COMFY_MIDI_CHANNEL_COUNT && _midiPlyrDriver)
				_midiPlyrDriver->musicSetLoop(subop == 7 ? 1 : 0, channel);

			break;
		default:
			error("Unknown script opcode 0x1D subopcode 0x%02X at script PC 0x%08X", subop, pc - 2);
		}

		return kScriptContinue;
	}

	case 0x1E: {
		Actor *target = actorResolve(scriptReadWord(pc), _currentActor);
		uint16 key = scriptReadWord(pc + 2);
		byte flags = scriptReadByte(pc + 4);
		uint32 triggerPc = scriptReadDword(pc + 5);
		pc += 9;
		if (!target)
			error("Script opcode 0x1E resolved an invalid actor");

		if (key != 0xFC18)
			actorWriteU16(*target, kActorTriggerKey, key);

		if (triggerPc != 0xFFFFFC18) {
			actorWriteU32(*target, kActorTriggerPc, triggerPc);
			actorWriteU8(*target, kActorTriggerFlags, flags);
		} else {
			actorWriteU8(*target, kActorTriggerFlags, (actorReadU8(*target, kActorTriggerFlags) & 1) | flags);
		}

		return kScriptContinue;
	}

	case 0x1F: {
		uint16 bit = scriptReadWord(pc);
		pc += 2;
		if ((int16)bit >= 0x32) {
			bit -= 0x32;
			_musicEventFlag = 1;
		}

		_musicEventMask |= (uint16)(1U << (bit & 0x1F));

		return kScriptYield;
	}

	case 0x20: {
		uint16 scene = scriptReadWord(pc);
		pc += 2;
		if (!_pendingScene || (int16)scene < (int16)_pendingScene)
			_pendingScene = scene;

		if (_videoMode == 2 || (_usesAnimFile && _videoMode == 4))
			renderSetDirty();

		return kScriptYield;
	}

	case 0x21: {
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

	case 0x22: {
		musicSetEnabled(scriptReadByte(pc));
		pc++;
		return kScriptContinue;
	}

	case 0x23: {
		uint16 handle = scriptReadWord(pc);
		int16 delta = scriptReadStringIndex(pc + 2);
		pc += 4;
		midiHandleAddTo(handle, delta);

		return kScriptContinue;
	}

	case 0x24: {
		midiSetTimeScale((int16)scriptReadStringIndex(pc));
		pc += 2;
		return kScriptContinue;
	}

	case 0x25: {
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

	case 0x26:
	case 0x2E: {
		uint16 first = scriptReadWord(pc);
		uint16 second = scriptReadWord(pc + 2);
		uint16 key = scriptReadWord(pc + 4);
		pc += 6;
		int16 lhs = (int16)midiGetHandle(first);
		int16 rhs = opcode == 0x26 ? (int16)second : (int16)midiGetHandle(second);
		if (lhs >= rhs)
			keyBitSet(key);

		return kScriptContinue;
	}

	case 0x28:
	case 0x29: {
		pc += 2;
		return kScriptContinue;
	}

	case 0x2A: {
		Actor *target = actorResolve(scriptReadWord(pc), _currentActor);
		uint32 selector = scriptReadDword(pc + 2);
		pc += 6;
		if (target)
			actorWriteU32(*target, kActorSpriteSelector, (selector + 1) | 0xFF000000);

		return kScriptContinue;
	}

	case 0x2B: {
		pc = (uint32)((int32)pc + (int16)(scriptReadWord(pc) * 2));
		return kScriptContinue;
	}

	case 0x2C: {
		uint16 destination = scriptReadWord(pc);
		uint16 source = scriptReadWord(pc + 2);
		pc += 4;

		if (keyBitTest(source))
			keyBitSet(destination);
		else
			keyBitClear(destination);

		return kScriptContinue;
	}

	case 0x2D: {
		byte subop = scriptReadByte(pc++);
		switch (subop) {
		case 1:
			sceneGoto(scriptReadWord(pc));
			pc += 2;
			break;
		case 2:
			sceneStop();
			break;
		case 3: {
			uint16 descriptor = scriptReadWord(pc);
			uint16 index = scriptReadWord(pc + 2);
			pc += 4;
			sceneEntryLoad(descriptor, index);
			break;
		}
		case 4: {
			uint16 channel = scriptReadWord(pc);
			uint16 frame = scriptReadWord(pc + 2);
			pc += 4;
			midiPlaySongAtFrame(channel, frame);
			break;
		}
		case 5: {
			uint16 channel = scriptReadWord(pc);
			pc += 2;
			midiStopSong(channel);
			break;
		}
		case 6:
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

	case 0x2F: {
		byte command = scriptReadByte(pc);
		uint16 destination = scriptReadWord(pc + 1);
		uint16 first = scriptReadWord(pc + 3);
		uint16 second = scriptReadWord(pc + 5);
		pc += 7;
		if (command == '=')
			midiHandleCopy(destination, first);
		else if (command == 'm')
			midiHandleCopy(destination,
				(int16)midiGetHandle(second) <= (int16)midiGetHandle(first) ? first : second);

		return kScriptContinue;
	}

	case 0x30:
		if (!_usesAnimFile)
			error("Unknown script opcode 0x%02X at script PC 0x%08X", opcode, pc - 1);
	{
		byte subop = scriptReadByte(pc++);
		switch (subop) {
		case 0:
			animFrameShutdown(true);
			break;
		case 1: {
			uint16 animIndex = scriptReadWord(pc);
			uint16 frameKey = scriptReadWord(pc + 2);
			pc += 4;
			animFileLoadFrame(animIndex, frameKey, actorReadU16(actor, kActorSceneHandle));
			break;
		}
		case 2:
			animFrameSetReady(true);
			break;
		case 3:
			animFrameSetReady(false);
			break;
		case 4:
		case 5:
			break;
		case 6:
			if (_engineVersion == 3)
				animFrameInvalidateActorRect();

			break;
		default:
			error("Unknown script opcode 0x30 subopcode 0x%02X at script PC 0x%08X", subop, pc - 2);
		}

		return kScriptContinue;
	}

	case 0x31:
	case 0x32:
	case 0x33:
	case 0x34:
	case 0x35:
	case 0x36:
		if (!(_usesWcomfy99ScriptOps ||
			(opcode == 0x31 && (_game->scriptFeatures & COMFY_SCRIPT_OPCODE_31)) ||
			(opcode == 0x32 && (_game->scriptFeatures & COMFY_SCRIPT_OPCODE_32)) ||
			(opcode == 0x33 && (_game->scriptFeatures & COMFY_SCRIPT_OPCODE_33))))
			error("Unknown script opcode 0x%02X at script PC 0x%08X", opcode, pc - 1);
	{
		switch (opcode) {
		case 0x31: {
			uint16 count = scriptReadStringIndex(pc);
			pc += 2;
			if (_animUsesWaveVocCounter) {
				uint16 slot = _soundEventMaximum ? _soundEventMaximum - 1 : _vocQueueCapacity - 1;
				if (_engineVersion == 3) {
					VocQueueEntry1999 &entry = _vocQueue1999[slot];
					entry.argumentCount = MIN<uint16>(count, COMFY_VOC_ARG_CAPACITY_1999);
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
		case 0x32:
			scriptReadStringIndex(pc);
			scriptReadStringIndex(pc + 2);
			pc += 4;
			break;
		case 0x33: {
			pc += 4;
			byte inlineBytes = scriptReadByte(pc++);
			byte tripleCount = scriptReadByte(pc++);
			pc += inlineBytes + (uint32)tripleCount * 3;
			break;
		}
		case 0x34: {
			byte subop = scriptReadByte(pc++);
			uint16 value = scriptReadWord(pc);
			pc += 2;
			Common::String key;
			for (byte character = scriptReadByte(pc++); character; character = scriptReadByte(pc++))
				key += (char)character;

			key.trim();

			switch (subop) {
			case 0:
				if (value < _stringTable.size())
					_stringTable[value] = ConfMan.hasKey(key) ? ConfMan.getInt(key) : 0;

				break;
			case 1:
				ConfMan.setInt(key, value);
				if (key.equalsIgnoreCase("SENSITIVITY"))
					_wcomfy99Sensitivity = value;

				ConfMan.flushToDisk();
				break;
			case 4:
				if (ConfMan.hasKey(key) && !ConfMan.get(key).equalsIgnoreCase("?????"))
					keyBitSet(value);
				else
					keyBitClear(value);

				break;
			case 5: {
				Common::ScopedPtr<Common::SeekableReadStream> stream(pathFOpen(Common::Path(key), true));
				if (stream)
					keyBitSet(value);
				else
					keyBitClear(value);

				break;
			}
			case 6: {
				key.trim();
				byte last = key.empty() ? 0 : key.lastChar();
				if (value < _stringTable.size())
					_stringTable[value] = Common::isSpace(last) ? 0x81 : Common::isDigit(last) ? 0x12 :
						(Common::isAlpha(last) ? (Common::isUpper(last) ? 0x04 : 0x08) : 0x40);

				break;
			}
			default:
				error("Unknown script opcode 0x34 subopcode 0x%02X near script PC 0x%08X", subop, pc);
			}

			break;
		}
		case 0x35: {
			byte subop = scriptReadByte(pc++);
			switch (subop) {
			case 2:
				_wcomfy99SubsystemWord = 0;
				break;
			default:
				error("Unknown script opcode 0x35 subopcode 0x%02X at script PC 0x%08X", subop, pc - 2);
			}

			break;
		}
		case 0x36: {
			byte subop = scriptReadByte(pc++);
			switch (subop) {
			case 1:
				_wcomfy99RecordHostEnabled = true;
				break;
			case 2:
				_wcomfy99RecordHostEnabled = false;
				break;
			case 3:
			case 4:
			case 5:
				_wcomfy99MixedHostFirstWord = scriptReadWord(pc);
				_wcomfy99MixedHostSecondWord = scriptReadWord(pc + 2);
				pc += 4;
				_wcomfy99RecordHostEnabled = true;

				if (subop != 3) {
					_wcomfy99MixedHostThirdWord = scriptReadWord(pc);
					pc += 2;
				}

				if (subop == 5) {
					_wcomfy99MixedHostFourthWord = scriptReadWord(pc);
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

	case 0x46:
	case 0x47:
		if (!_usesWcomfy99ScriptOps)
			error("Unknown script opcode 0x%02X at script PC 0x%08X", opcode, pc - 1);
	{
		switch (opcode) {
		case 0x46: {
			byte subop = scriptReadByte(pc++);
			if (subop < 1 || subop > 8)
				error("Unknown script opcode 0x46 subopcode 0x%02X at script PC 0x%08X", subop, pc - 2);

			uint16 argument = scriptReadWord(pc);
			pc += 2;
			uint16 percentage = argument == 0x00FF ? 0x00FF : ((int16)argument < 0 ? 0 : MIN<uint16>(argument, 100));

			switch (subop) {
			case 1:
				_wcomfy99HostWordA = argument;
				break;
			case 2:
				if (percentage != 0x00FF)
					_wcomfy99WaveVolumePercent = percentage;
				break;
			case 3:
				if (percentage != 0x00FF)
					_wcomfy99WaveLeftPercent = percentage;
				break;
			case 4:
				if (percentage != 0x00FF)
					_wcomfy99WaveRightPercent = percentage;
				break;
			case 5:
			case 6:
				if (percentage != 0x00FF)
					_wcomfy99MixerVolumePercent = percentage;
				break;
			case 7:
				if (percentage != 0x00FF)
					_wcomfy99MixerAltPercent = percentage;
				break;
			case 8:
				_wcomfy99HostWordB = argument;
				break;
			default:
				break;
			}

			break;
		}
		case 0x47:
			_wcomfy99RangeHostStart = scriptReadWord(pc);
			_wcomfy99RangeHostEnd = scriptReadWord(pc + 2);
			_wcomfy99RangeHostCount = _wcomfy99RangeHostEnd >= _wcomfy99RangeHostStart ?
				_wcomfy99RangeHostEnd - _wcomfy99RangeHostStart + 1 : 0;
			pc += 4;
			break;
		default:
			break;
		}

		return kScriptContinue;
	}

	case 0x50: {
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

	case 0x51:
	case 0x52:
	case 0x70:
	case 0x71: {
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

		if (!duration && (opcode == 0x51 || opcode == 0x52))
			error("Script opcode 0x%02X has a zero movement duration", opcode);

		bool moveTo = opcode == 0x52 || opcode == 0x71;
		if ((moveTo && x == (int16)0xF830) || (!moveTo && x == (int16)0x8AD0)) {
			Actor *other = actorResolve((uint16)y, _currentActor);
			if (!other)
				error("Script opcode 0x%02X resolved an invalid reference actor", opcode);

			if (moveTo) {
				x = (int16)((int32)actorReadU32(*other, kActorXFixed) >> 12);
				y = (int16)((int32)actorReadU32(*other, kActorYFixed) >> 12);
			} else {
				x = (int16)actorReadU16(*other, kActorXFixed) >> 12;
				y = (int16)actorReadU16(*other, kActorYFixed) >> 12;
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
			dx = x == (int16)0xFC18 ? 0 : dx - (int32)actorReadU32(*target, kActorXFixed);
			dy = y == (int16)0xFC18 ? 0 : dy - (int32)actorReadU32(*target, kActorYFixed);
		}

		if (opcode == 0x51 || opcode == 0x52 || (moveTo && duration < 0)) {
			if (duration < 0)
				duration = -duration;

			int16 distanceX = ABS((int16)(dx >> 12));
			int16 distanceY = ABS((int16)(dy >> 12));
			int16 distance = MAX(distanceX, distanceY);
			duration = (int16)(((int32)distance * 100) / duration);
			if (!duration)
				duration = 1;
		}

		actorWriteU32(*target, kActorMoveDx, dx);
		actorWriteU32(*target, kActorMoveDy, dy);
		actorWriteU16(*target, kActorMoveTicks, duration);
		actorWriteU8(*target, kActorBlockingMove, blocking);
		actorWriteU16(*target, kActorCompletionKey, completionKey);
		return blocking && target == &actor ? kScriptYield : kScriptContinue;
	}

	case 0x48:
	case 0x49:
		if (!_usesWcomfy99ScriptOps)
			error("Unknown script opcode 0x%02X at script PC 0x%08X", opcode, pc - 1);
	{
		uint16 key = scriptReadWord(pc);
		uint32 targetPc = scriptReadDword(pc + 2);
		bool jumpWhenSet = opcode == 0x49;
		pc = keyBitTest(key) == jumpWhenSet ? targetPc : pc + 6;
		return kScriptContinue;
	}

	case 0x53: {
		Actor *target = actorResolve(scriptReadWord(pc), _currentActor);
		uint16 frame = scriptReadStringIndex(pc + 2);
		pc += 4;
		if (!target)
			error("Script opcode 0x53 resolved an invalid actor");

		actorWriteU32(*target, kActorSpriteSelector, (uint32)(int32)(int16)frame);
		actorWriteU32(*target, kActorMoveDx, 0);
		actorWriteU32(*target, kActorMoveDy, 0);
		return kScriptContinue;
	}

	case 0x54: {
		if (_engineVersion < 2)
			error("Unknown script opcode 0x%02X at script PC 0x%08X", opcode, pc - 1);

		uint32 endPc = scriptReadDword(pc);
		pc += 4;
		uint32 size = endPc >= pc ? endPc - pc : 0;
		size = MIN<uint32>(size, COMFY_FRAME_LOADER_DATA_BYTES);
		if (pc <= _comfyObjData.size() && size <= _comfyObjData.size() - pc) {
			_frameLoaderData.resize(COMFY_FRAME_LOADER_DATA_BYTES);
			if (size)
				memcpy(&_frameLoaderData[0], &_comfyObjData[pc], size);
		} else {
			error("Script opcode 0x54 references data outside COMFY.OBJ");
		}

		return kScriptContinue;
	}

	case 0x55: {
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

	case 0x6E:
		return kScriptYield;

	case 0x6F: {
		uint16 id = scriptReadWord(pc);
		int16 delta = scriptReadStringIndex(pc + 2);
		pc += 4;
		midiAddEvent(id, delta);
		return kScriptContinue;
	}

	case 0x72: {
		uint16 requested = scriptReadStringIndex(pc);
		pc += 2;
		uint16 balance = actorReadU16(actor, kActorWaitAccum);
		if (!requested)
			balance = 0;

		if (balance) {
			uint16 step = balance < 5 ? balance : 5;
			balance = step < (uint16)(requested - 1) ? step : (uint16)(requested - 1);
			actorWriteU16(actor, kActorWaitAccum, actorReadU16(actor, kActorWaitAccum) - balance);
		}

		actorWriteU16(actor, kActorWaitTarget, requested - balance);
		return kScriptYield;
	}

	case 0x73: {
		_waveOutputActive = false;
		return kScriptContinue;
	}

	case 0x74: {
		uint16 key = scriptReadWord(pc);
		if (keyBitTest(key)) {
			pc--;
			return kScriptYield;
		}

		pc += 2;
		return kScriptContinue;
	}

	case 0x75: {
		byte kind = scriptReadByte(pc++);
		uint16 key = scriptReadWord(pc);
		pc += 2;
		bool condition = false;
		if (_usesWcomfy99ScriptOps) {
			uint16 argument = scriptReadWord(pc);
			pc += 2;
			switch (kind) {
			case 1:
				condition = sceneGetHandle(argument) != 0;
				break;
			case 2:
				condition = _inputDeviceMode == 1;
				break;
			case 14:
				if (argument <= 4) {
					condition = _inputDeviceMode == argument;
				} else if (argument == 20) {
					condition = _wcomfy99RecordHostEnabled;
				} else if (argument == 30) {
					Common::ScopedPtr<Common::SeekableReadStream> stream(pathFOpen(Common::Path("comfy.htm"), true));
					condition = (bool)stream;
				}
				break;
			default:
				error("Unknown script opcode 0x75 subopcode 0x%02X at script PC 0x%08X", kind, pc - 6);
			}
		} else {
			switch (kind) {
			case 1: {
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

	case 0x76:
	case 0x77: {
		uint16 index = scriptReadWord(pc);
		pc += 2;
		if (opcode == 0x77) {
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

	case 0x7F:
		return kScriptContinue;

	default:
		if (opcode)
			error("Unknown script opcode 0x%02X at script PC 0x%08X", opcode, pc - 1);

		Actor *root = actorGetPtr(0);
		if (root)
			actorWriteU8(*root, kActorActive, 0);

		return kScriptContinue;
	}

	error("Script opcode 0x%02X left the dispatch switch without returning", opcode);
	return kScriptContinue;
}

ComfyEngine::ScriptDispatchStatus ComfyEngine::scriptStep(Actor &actor, uint32 &pc) {
	uint32 originalPc = pc;
	byte opcode = scriptReadByte(pc++);
	ScriptDispatchStatus status = scriptDispatch(actor, opcode, pc);
	if (_scriptFault)
		error("Script fault while executing opcode 0x%02X at script PC 0x%08X", opcode, originalPc);

	return status;
}


} // End of namespace Comfy
