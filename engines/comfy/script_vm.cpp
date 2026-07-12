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
	if (opcode == 0x01 || opcode == 0x02) {
		Actor *target = actorResolve(scriptReadWord(pc), _currentActor);
		pc += 2;
		int16 x = scriptReadStringIndex(pc);
		pc += 2;
		int16 y = scriptReadStringIndex(pc);
		pc += 2;
		byte randomize = scriptReadByte(pc++);
		if (!target)
			return kScriptUnhandled;

		if ((opcode == 0x01 && x == int16(0xF830)) || (opcode == 0x02 && x == int16(0x8AD0))) {
			Actor *other = actorResolve(uint16(y), _currentActor);
			if (!other)
				return kScriptUnhandled;

			x = int16(int32(actorReadU32(*other, kActorXFixed)) >> 12);
			y = int16(int32(actorReadU32(*other, kActorYFixed)) >> 12);
		} else if (opcode == 0x01 && x == int16(0xF448)) {
			x = int16(-int32(actorReadU32(*target, kActorXFixed)) >> 12);
		}

		if (opcode == 0x01 && y == int16(0xF448))
			y = int16(-int32(actorReadU32(*target, kActorYFixed)) >> 12);

		if (randomize) {
			if (x != int16(0xFC18))
				x = int16((int32(getRandomNumber(0x7FFF)) * x) / 0x8000);

			if (y != int16(0xFC18))
				y = int16((int32(getRandomNumber(0x7FFF)) * y) / 0x8000);
		}

		if (opcode == 0x01) {
			if (x != int16(0xFC18))
				actorWriteU32(*target, kActorXFixed, uint32(int32(x) * 0x1000));

			if (y != int16(0xFC18))
				actorWriteU32(*target, kActorYFixed, uint32(int32(y) * 0x1000));
		} else {
			actorWriteU32(*target, kActorXFixed, actorReadU32(*target, kActorXFixed) + uint32(int32(x) * 0x1000));
			actorWriteU32(*target, kActorYFixed, actorReadU32(*target, kActorYFixed) + uint32(int32(y) * 0x1000));
		}

		return kScriptContinue;
	}

	if (opcode == 0x03) {
		Actor *target = actorResolve(scriptReadWord(pc), _currentActor);
		pc += 2;
		uint16 frame = scriptReadStringIndex(pc);
		pc += 2;
		if (target)
			actorWriteU32(*target, kActorSpriteSelector, uint32(int32(int16(frame))));

		return kScriptContinue;
	}

	if (opcode == 0x04) {
		uint16 key = scriptReadWord(pc);
		if (key && !keyBitTest(key)) {
			pc--;
			return kScriptYield;
		}

		pc += 2;
		return kScriptContinue;
	}

	if (opcode == 0x05 || opcode == 0x19) {
		uint16 key = scriptReadWord(pc);
		byte state = scriptReadByte(pc + 2);
		uint32 targetPc = scriptReadDword(pc + 3);
		pc += 7;
		if (!key || byte(keyBitTest(key)) == state) {
			if (opcode == 0x19)
				actorSetPc(actor, pc);

			pc = targetPc;
		}

		return kScriptContinue;
	}

	if (opcode == 0x06) {
		uint16 key = scriptReadWord(pc);
		byte state = scriptReadByte(pc + 2);
		pc += 3;
		if (state == 1)
			keyBitSet(key);
		else
			keyBitClear(key);

		return kScriptContinue;
	}

	if (opcode == 0x07) {
		Actor *target = actorResolve(scriptReadWord(pc), _currentActor);
		pc += 2;
		if (!target)
			return kScriptUnhandled;

		bool current = target == &actor;
		uint16 actorIndex = uint16(target - &_actors[0]);
		actorUnlink(actorIndex);
		actorFreeTreePc(actorIndex);
		_actorDestroyedCurrent = current;
		return current ? kScriptYield : kScriptContinue;
	}

	if (opcode == 0x08) {
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

		actorInit(sceneSlot, parentSlot, flags & 1, flags & 2, newPc, x, y, sprite, flags & 4);
		return kScriptContinue;
	}

	if (opcode == 0x09) {
		actorSetFrame(int16(scriptReadWord(pc)));
		pc += 2;
		return kScriptContinue;
	}

	if (opcode >= 0x0A && opcode <= 0x0D) {
		Actor *target = actorResolve(scriptReadWord(pc), _currentActor);
		pc += 2;
		if (!target)
			return kScriptUnhandled;

		if (opcode == 0x0A)
			actorWriteU8(*target, kActorVisible, 1);
		else if (opcode == 0x0B)
			actorWriteU8(*target, kActorVisible, 0);
		else if (opcode == 0x0C)
			actorWriteU8(*target, kActorActive, 1);
		else {
			actorWriteU16(*target, kActorMoveTicks, 0);
			actorWriteU8(*target, kActorActive, 0);
			if (target == &actor)
				return kScriptYield;
		}

		return kScriptContinue;
	}

	if (opcode >= 0x0E && opcode <= 0x11) {
		Actor *target = actorResolve(scriptReadWord(pc), _currentActor);
		int16 expected = scriptReadWord(pc + 2);
		uint16 key = scriptReadWord(pc + 4);
		byte mode = scriptReadByte(pc + 6);
		pc += 7;
		if (!target)
			return kScriptUnhandled;

		int16 actual = int32(actorReadU32(*target, opcode == 0x0E || opcode == 0x10 ? kActorXFixed : kActorYFixed)) >> 12;
		bool matched = ((mode & 2) && expected == actual) || ((mode & 4) && expected < actual) || ((mode & 1) && expected > actual);
		if (matched) {
			if (opcode == 0x0E || opcode == 0x0F)
				keyBitSet(key);
			else
				keyBitClear(key);
		}

		return kScriptContinue;
	}

	if (opcode == 0x12) {
		uint16 id = scriptReadWord(pc);
		int16 delta = scriptReadStringIndex(pc + 2);
		pc += 4;
		midiAddTrack(id, delta);
		return kScriptContinue;
	}

	if (opcode == 0x13) {
		byte totalWeight = scriptReadByte(pc++);
		byte installReturnPc = scriptReadByte(pc++);
		uint32 returnPc = pc + installReturnPc - 3;
		byte choice = byte((getRandomNumber(0x7FFF) * totalWeight) / 0x8000);
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

	if (opcode == 0x14) {
		Actor *target = actorResolve(scriptReadWord(pc), _currentActor);
		byte mode = scriptReadByte(pc + 2);
		pc += 3;
		if (!target)
			return kScriptUnhandled;

		uint16 targetIndex = uint16(target - &_actors[0]);
		uint16 oldParent = actorReadU16(*target, kActorParent);
		actorUnlink(targetIndex);
		if (mode == 'O')
			actorInsertChild(targetIndex, oldParent);
		else
			actorInsertSibling(targetIndex, oldParent);

		return kScriptContinue;
	}

	if (opcode == 0x15 || opcode == 0x16) {
		byte count = scriptReadByte(pc++);
		for (uint i = 0; i < count; i++) {
			uint16 id = scriptReadWord(pc);
			pc += 2;
			if (opcode == 0x15) {
				spriteGetPtr(int16(id));
				if (_spriteConversionLoads.count < COMFY_RESOURCE_LIST_CAPACITY)
					_spriteConversionLoads.ids[_spriteConversionLoads.count++] = id;
			} else {
				soundLoadEntry(id);
			}
		}

		return kScriptContinue;
	}

	if (opcode == 0x17) {
		if (_musicEventMask) {
			pc--;
			return kScriptYield;
		}

		uint16 soundId = scriptReadWord(pc);
		pc += 2;
		byte count = scriptReadByte(pc++);
		vocQueuePush(soundId, count, pc);
		pc += uint32(count) * 2;
		return kScriptContinue;
	}

	if (opcode == 0x18) {
		Actor *target = actorResolve(scriptReadWord(pc), _currentActor);
		pc += 2;
		if (!target)
			return kScriptUnhandled;

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

	if (opcode == 0x1B) {
		vocQueuePlayAll();
		return kScriptContinue;
	}

	if (opcode == 0x1C) {
		midiRemoveTrack(scriptReadWord(pc));
		pc += 2;
		return kScriptContinue;
	}

	if (opcode == 0x1D) {
		byte packed = scriptReadByte(pc++);
		byte channel = (packed >> 4) - 1;
		byte subop = packed & 0x0F;
		if (subop == 1) {
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
		} else if (subop >= 2 && subop <= 4) {
			uint16 value = scriptReadWord(pc);
			uint16 ticks = scriptReadWord(pc + 2);
			pc += 4;
			midiSetChannelParam(channel, subop, value, ticks);
		} else if (subop == 5 && channel < COMFY_MIDI_CHANNEL_COUNT) {
			midiStopAndRemove(channel);
		} else if (subop == 6 && channel < COMFY_MIDI_CHANNEL_COUNT) {
			midiStopAndFireKeys(channel);
		} else if ((subop == 7 || subop == 8) && _engineVersion == kEngineVersion3 &&
				channel < COMFY_MIDI_CHANNEL_COUNT && _midiPlyrDriver) {
			_midiPlyrDriver->musicSetLoop(subop == 7 ? 1 : 0, channel);
		}

		return kScriptContinue;
	}

	if (opcode == 0x1E) {
		Actor *target = actorResolve(scriptReadWord(pc), _currentActor);
		uint16 key = scriptReadWord(pc + 2);
		byte flags = scriptReadByte(pc + 4);
		uint32 triggerPc = scriptReadDword(pc + 5);
		pc += 9;
		if (!target)
			return kScriptUnhandled;

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

	if (opcode == 0x1F) {
		uint16 bit = scriptReadWord(pc);
		pc += 2;
		if (int16(bit) >= 0x32) {
			bit -= 0x32;
			_musicEventFlag = 1;
		}

		if (bit < 16)
			_musicEventMask |= 1 << bit;

		return kScriptYield;
	}

	if (opcode == 0x20) {
		uint16 scene = scriptReadWord(pc);
		pc += 2;
		if (!_pendingScene || int16(scene) < int16(_pendingScene))
			_pendingScene = scene;

		renderSetDirty();

		return kScriptYield;
	}

	if (opcode == 0x21) {
		uint16 paletteId = scriptReadStringIndex(pc);
		uint16 fadeTicks = scriptReadStringIndex(pc + 2);
		byte brightness = scriptReadByte(pc + 4);
		pc += 5;
		if (!brightness)
			paletteLoadWithFade(paletteId, fadeTicks);
		else
			paletteApplyBrightness(brightness);

		return kScriptContinue;
	}

	if (opcode == 0x22) {
		musicSetEnabled(scriptReadByte(pc));
		pc++;
		return kScriptContinue;
	}

	if (opcode == 0x23) {
		uint16 handle = scriptReadWord(pc);
		int16 delta = scriptReadStringIndex(pc + 2);
		pc += 4;
		midiHandleAddTo(handle, delta);

		return kScriptContinue;
	}

	if (opcode == 0x24) {
		midiSetTimeScale(int16(scriptReadStringIndex(pc)));
		pc += 2;
		return kScriptContinue;
	}

	if (opcode == 0x1A) {
		pc = actorPopPc(actor);
		return kScriptContinue;
	}

	if (opcode == 0x25) {
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

	if (opcode == 0x26 || opcode == 0x2E) {
		uint16 first = scriptReadWord(pc);
		uint16 second = scriptReadWord(pc + 2);
		uint16 key = scriptReadWord(pc + 4);
		pc += 6;
		int16 lhs = first < _midiHandles.size() ? _midiHandles[first] : 0;
		int16 rhs = opcode == 0x26 ? int16(second) : (second < _midiHandles.size() ? int16(_midiHandles[second]) : 0);
		if (lhs >= rhs)
			keyBitSet(key);

		return kScriptContinue;
	}

	if (opcode == 0x28 || opcode == 0x29) {
		pc += 2;
		return kScriptContinue;
	}

	if (opcode == 0x2A) {
		Actor *target = actorResolve(scriptReadWord(pc), _currentActor);
		uint32 selector = scriptReadDword(pc + 2);
		pc += 6;
		if (target)
			actorWriteU32(*target, kActorSpriteSelector, (selector + 1) | 0xFF000000);

		return kScriptContinue;
	}

	if (opcode == 0x2B) {
		pc = uint32(int32(pc) + int16(scriptReadWord(pc) * 2));
		return kScriptContinue;
	}

	if (opcode == 0x2C) {
		uint16 destination = scriptReadWord(pc);
		uint16 source = scriptReadWord(pc + 2);
		pc += 4;
		if (keyBitTest(source))
			keyBitSet(destination);
		else
			keyBitClear(destination);

		return kScriptContinue;
	}

	if (opcode == 0x2D) {
		byte subop = scriptReadByte(pc++);
		if (subop == 1) {
			sceneGoto(scriptReadWord(pc));
			pc += 2;
		} else if (subop == 2) {
			sceneStop();
		} else if (subop == 3) {
			uint16 descriptor = scriptReadWord(pc);
			uint16 index = scriptReadWord(pc + 2);
			pc += 4;
			sceneEntryLoad(descriptor, index);
		} else if (subop == 4) {
			uint16 channel = scriptReadWord(pc);
			uint16 frame = scriptReadWord(pc + 2);
			pc += 4;
			midiPlaySongAtFrame(channel, frame);
		} else if (subop == 5) {
			uint16 channel = scriptReadWord(pc);
			pc += 2;
			midiStopSong(channel);
		}

		return kScriptContinue;
	}

	if (opcode == 0x2F) {
		byte command = scriptReadByte(pc);
		uint16 destination = scriptReadWord(pc + 1);
		uint16 first = scriptReadWord(pc + 3);
		uint16 second = scriptReadWord(pc + 5);
		pc += 7;
		if (destination < _midiHandles.size() && first < _midiHandles.size()) {
			if (command == '=')
				midiHandleCopy(destination, first);
			else if (command == 'm' && second < _midiHandles.size())
				midiHandleCopy(destination,
					int16(midiGetHandle(second)) <= int16(midiGetHandle(first)) ? first : second);
		}

		return kScriptContinue;
	}

	if (opcode == 0x30 && _usesAnimFile) {
		byte subop = scriptReadByte(pc++);
		if (subop == 0) {
			animFrameShutdown(true);
		} else if (subop == 1) {
			uint16 animIndex = scriptReadWord(pc);
			uint16 frameKey = scriptReadWord(pc + 2);
			pc += 4;
			animFileLoadFrame(animIndex, frameKey, actorReadU16(actor, kActorSceneHandle));
		} else if (subop == 2) {
			animFrameSetReady(true);
		} else if (subop == 3) {
			animFrameSetReady(false);
		} else if (subop == 6) {
			animFrameInvalidateActorRect();
		}

		return kScriptContinue;
	}

	if (opcode >= 0x31 && opcode <= 0x36 && _usesWcomfy99ScriptOps) {
		if (opcode == 0x31) {
			uint16 count = scriptReadStringIndex(pc);
			pc += 2;
			_wcomfy99FeatureWordCount = MIN<uint16>(count, ARRAYSIZE(_wcomfy99FeatureWords));
			for (uint i = 0; i < _wcomfy99FeatureWordCount; i++)
				_wcomfy99FeatureWords[i] = scriptReadWord(pc + i * 2);

			pc += uint32(count) * 2;
		} else if (opcode == 0x32) {
			_wcomfy99Stub32FirstWord = scriptReadWord(pc);
			_wcomfy99Stub32SecondWord = scriptReadWord(pc + 2);
			pc += 4;
		} else if (opcode == 0x33) {
			pc += 4;
			byte inlineBytes = scriptReadByte(pc++);
			byte tripleCount = scriptReadByte(pc++);
			pc += inlineBytes + uint32(tripleCount) * 3;
		} else if (opcode == 0x34) {
			byte subop = scriptReadByte(pc++);
			uint16 value = scriptReadWord(pc);
			pc += 2;
			Common::String key;
			for (byte character = scriptReadByte(pc++); character; character = scriptReadByte(pc++))
				key += char(character);

			key.trim();

			if (subop == 0 && value < _stringTable.size()) {
				_stringTable[value] = ConfMan.hasKey(key) ? ConfMan.getInt(key) : 0;
			} else if (subop == 1) {
				ConfMan.setInt(key, value);
				if (key.equalsIgnoreCase("SENSITIVITY"))
					_wcomfy99Sensitivity = value;

				ConfMan.flushToDisk();
			} else if (subop == 4) {
				if (ConfMan.hasKey(key) && !ConfMan.get(key).equalsIgnoreCase("?????"))
					keyBitSet(value);
				else
					keyBitClear(value);
			} else if (subop == 5) {
				Common::ScopedPtr<Common::SeekableReadStream> stream(pathFOpen(Common::Path(key), true));
				if (stream)
					keyBitSet(value);
				else
					keyBitClear(value);
			} else if (subop == 6) {
				key.trim();
				byte last = key.empty() ? 0 : key.lastChar();
				if (value < _stringTable.size())
					_stringTable[value] = Common::isSpace(last) ? 0x81 : Common::isDigit(last) ? 0x12 :
						(Common::isAlpha(last) ? (Common::isUpper(last) ? 0x04 : 0x08) : 0x40);
			}
		} else if (opcode == 0x35) {
			byte subop = scriptReadByte(pc++);
			if (subop == 2)
				_wcomfy99SubsystemWord = 0;
		} else {
			byte subop = scriptReadByte(pc++);
			if (subop == 1 || subop == 3)
				_wcomfy99RecordHostEnabled = true;
			else if (subop == 2)
				_wcomfy99RecordHostEnabled = false;

			if (subop >= 3 && subop <= 5) {
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
			}
		}

		return kScriptContinue;
	}

	if ((opcode == 0x46 || opcode == 0x47) && _usesWcomfy99ScriptOps) {
		if (opcode == 0x46) {
			byte subop = scriptReadByte(pc++);
			if (subop >= 1 && subop <= 8) {
				uint16 argument = scriptReadWord(pc);
				pc += 2;
				uint16 percentage = argument == 0x00FF ? 0x00FF :
					(int16(argument) < 0 ? 0 : MIN<uint16>(argument, 100));
				if (subop == 1)
					_wcomfy99HostWordA = argument;
				else if (subop == 2 && percentage != 0x00FF)
					_wcomfy99WaveVolumePercent = percentage;
				else if (subop == 3 && percentage != 0x00FF)
					_wcomfy99WaveLeftPercent = percentage;
				else if (subop == 4 && percentage != 0x00FF)
					_wcomfy99WaveRightPercent = percentage;
				else if ((subop == 5 || subop == 6) && percentage != 0x00FF)
					_wcomfy99MixerVolumePercent = percentage;
				else if (subop == 7 && percentage != 0x00FF)
					_wcomfy99MixerAltPercent = percentage;
				else if (subop == 8)
					_wcomfy99HostWordB = argument;
			}
		} else {
			_wcomfy99RangeHostStart = scriptReadWord(pc);
			_wcomfy99RangeHostEnd = scriptReadWord(pc + 2);
			_wcomfy99RangeHostCount = _wcomfy99RangeHostEnd >= _wcomfy99RangeHostStart ?
				_wcomfy99RangeHostEnd - _wcomfy99RangeHostStart + 1 : 0;
			pc += 4;
		}

		return kScriptContinue;
	}

	if (opcode == 0x50) {
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

	if (opcode == 0x51 || opcode == 0x52 || opcode == 0x70 || opcode == 0x71) {
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
		if (!target || !duration)
			return kScriptUnhandled;

		bool moveTo = opcode == 0x52 || opcode == 0x71;
		if ((moveTo && x == int16(0xF830)) || (!moveTo && x == int16(0x8AD0))) {
			Actor *other = actorResolve(uint16(y), _currentActor);
			if (!other)
				return kScriptUnhandled;

			if (moveTo) {
				x = int16(int32(actorReadU32(*other, kActorXFixed)) >> 12);
				y = int16(int32(actorReadU32(*other, kActorYFixed)) >> 12);
			} else {
				x = int16(actorReadU16(*other, kActorXFixed)) >> 12;
				y = int16(actorReadU16(*other, kActorYFixed)) >> 12;
			}
		}

		if (randomize) {
			if (!moveTo || x != int16(0xFC18))
				x = int16((int32(getRandomNumber(0x7FFF)) * x) / 0x8000);

			if (!moveTo || y != int16(0xFC18))
				y = int16((int32(getRandomNumber(0x7FFF)) * y) / 0x8000);
		}

		int32 dx = int32(x) * 0x1000;
		int32 dy = int32(y) * 0x1000;
		if (moveTo) {
			dx = x == int16(0xFC18) ? 0 : dx - int32(actorReadU32(*target, kActorXFixed));
			dy = y == int16(0xFC18) ? 0 : dy - int32(actorReadU32(*target, kActorYFixed));
		}

		if (opcode == 0x51 || opcode == 0x52 || (moveTo && duration < 0)) {
			if (duration < 0)
				duration = -duration;

			int16 distanceX = ABS(int16(dx >> 12));
			int16 distanceY = ABS(int16(dy >> 12));
			int16 distance = MAX(distanceX, distanceY);
			duration = int16((int32(distance) * 100) / duration);
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

	if ((opcode == 0x48 || opcode == 0x49) && _usesWcomfy99ScriptOps) {
		uint16 key = scriptReadWord(pc);
		uint32 targetPc = scriptReadDword(pc + 2);
		bool jumpWhenSet = opcode == 0x49;
		pc = keyBitTest(key) == jumpWhenSet ? targetPc : pc + 6;
		return kScriptContinue;
	}

	if ((opcode == 0x54 || opcode == 0x55) && _usesWcomfy99ScriptOps) {
		pc += 4;
		return kScriptContinue;
	}

	if (opcode == 0x6E)
		return kScriptYield;

	if (opcode == 0x6F) {
		uint16 id = scriptReadWord(pc);
		int16 delta = scriptReadStringIndex(pc + 2);
		pc += 4;
		midiAddEvent(id, delta);
		return kScriptContinue;
	}

	if (opcode == 0x72) {
		uint16 requested = scriptReadStringIndex(pc);
		pc += 2;
		uint16 balance = actorReadU16(actor, kActorWaitAccum);
		if (!requested)
			balance = 0;

		if (balance) {
			uint16 step = balance < 5 ? balance : 5;
			balance = step < uint16(requested - 1) ? step : uint16(requested - 1);
			actorWriteU16(actor, kActorWaitAccum, actorReadU16(actor, kActorWaitAccum) - balance);
		}

		actorWriteU16(actor, kActorWaitTarget, requested - balance);
		return kScriptYield;
	}

	if (opcode == 0x73) {
		_waveOutputActive = false;
		return kScriptContinue;
	}

	if (opcode == 0x74) {
		uint16 key = scriptReadWord(pc);
		if (keyBitTest(key)) {
			pc--;
			return kScriptYield;
		}

		pc += 2;
		return kScriptContinue;
	}

	if (opcode == 0x75) {
		byte kind = scriptReadByte(pc++);
		uint16 key = scriptReadWord(pc);
		pc += 2;
		bool condition = false;
		if (_usesWcomfy99ScriptOps) {
			uint16 argument = scriptReadWord(pc);
			pc += 2;
			if (kind == 1)
				condition = sceneGetHandle(argument) != 0;
			else if (kind == 2)
				condition = _inputDeviceMode == 1;
			else if (kind == 14) {
				if (argument <= 4) {
					condition = _inputDeviceMode == argument;
				} else if (argument == 20) {
					condition = _wcomfy99RecordHostEnabled;
				} else if (argument == 30) {
					Common::ScopedPtr<Common::SeekableReadStream> stream(pathFOpen(Common::Path("comfy.htm"), true));
					condition = bool(stream);
				}
			}
		} else if (kind == 1) {
			uint16 scene = scriptReadWord(pc);
			condition = sceneGetHandle(scene) != 0;
			pc += 2;
		}

		if (condition)
			keyBitSet(key);
		else
			keyBitClear(key);

		return kScriptContinue;
	}

	if (opcode == 0x76 || opcode == 0x77) {
		uint16 index = scriptReadWord(pc);
		pc += 2;
		if (opcode == 0x77) {
			if (_stringTable.size() <= 1) {
				_scriptFault = true;
				return kScriptUnhandled;
			}

			index += _stringTable[1];
		}

		uint16 value = scriptEvalExpr(pc, _currentActor);
		if (index >= _stringTable.size()) {
			_scriptFault = true;
			return kScriptUnhandled;
		}

		_stringTable[index] = value;
		return kScriptContinue;
	}

	if (opcode == 0x7F)
		return kScriptContinue;

	if (!opcode || opcode > 0x7F || opcode == 0x27 || (opcode >= 0x30 && opcode <= 0x4F) ||
	    (opcode >= 0x53 && opcode <= 0x6D) || (opcode >= 0x78 && opcode <= 0x7E)) {
		Actor *root = actorGetPtr(0);
		if (root)
			actorWriteU8(*root, kActorActive, 0);

		return kScriptDeactivatedRoot;
	}

	_scriptFault = true;
	return kScriptUnhandled;
}

ComfyEngine::ScriptDispatchStatus ComfyEngine::scriptStep(Actor &actor, uint32 &pc) {
	uint32 originalPc = pc;
	byte opcode = scriptReadByte(pc++);
	ScriptDispatchStatus status = scriptDispatch(actor, opcode, pc);
	if (status == kScriptUnhandled)
		pc = originalPc;
	else if (status == kScriptYield)
		actorWriteU32(actor, kActorCurrentPc, pc);

	return status;
}


} // End of namespace Comfy
