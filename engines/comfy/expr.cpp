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

namespace Comfy {

#define EO_PUSH_LITERAL_01 0x01
#define EO_PUSH_STRING_TABLE_VALUE 0x02
#define EO_PUSH_LITERAL_03 0x03
#define EO_ADD 0x08
#define EO_SUBTRACT 0x09
#define EO_MULTIPLY 0x0A
#define EO_DIVIDE 0x0B
#define EO_INDEXED_STRING_TABLE_VALUE 0x0C
#define EO_END 0x0D
#define EO_MODULO 0x0E
#define EO_ACTOR_LOCAL_X 0x10
#define EO_ACTOR_LOCAL_Y 0x11
#define EO_ACTOR_WORLD_X 0x12
#define EO_ACTOR_WORLD_Y 0x13
#define EO_ACTOR_SPRITE_SELECTOR 0x14
#define EO_PUSH_LITERAL_15 0x15
#define EO_RANDOMIZE 0x16
#define EO_MOUSE_X 0x17
#define EO_MOUSE_Y 0x18
#define EO_WAVE_BALANCE_PERCENT 0x19
#define EO_HOST_MEDIA_VALUE 0x1A
#define EO_NOOP_1B 0x1B
#define EO_WAVE_LEFT_PERCENT 0x1C
#define EO_WAVE_RIGHT_PERCENT 0x1D
#define EO_LOGICAL_DIMENSION 0x1E
#define EO_NOOP_1F 0x1F
#define EO_HOST_MEDIA_PROGRESS 0x20
#define EO_MIXER_VOLUME_PERCENT 0x21

uint16 ComfyEngine::scriptEvalExpr(uint32 &pc, uint16 fallbackActor) {
	_exprStackTop = 0;

	for (;;) {
		byte opcode = scriptReadByte(pc++);
		if (_scriptFault)
			return 0;

		switch (opcode) {
		case EO_END:
			return _exprStackTop ? _exprStack[0] : 0;

		case EO_PUSH_LITERAL_01:
		case EO_PUSH_LITERAL_03:
		case EO_PUSH_LITERAL_15:
			if (_exprStackTop < COMFY_EXPR_STACK_CAPACITY)
				_exprStack[_exprStackTop++] = scriptReadWord(pc);

			pc += 2;
			continue;

		case EO_PUSH_STRING_TABLE_VALUE: {
			uint16 index = scriptReadWord(pc);
			pc += 2;
			uint16 value = index < _stringTable.size() ? _stringTable[index] : 0;

			if (_exprStackTop < COMFY_EXPR_STACK_CAPACITY)
				_exprStack[_exprStackTop++] = value;

			continue;
		}

		case EO_ACTOR_LOCAL_X:
		case EO_ACTOR_LOCAL_Y:
		case EO_ACTOR_WORLD_X:
		case EO_ACTOR_WORLD_Y:
		case EO_ACTOR_SPRITE_SELECTOR: {
			Actor *actor = actorResolve(scriptReadWord(pc), fallbackActor);
			pc += 2;

			if (!actor) {
				_scriptFault = true;
				return 0;
			}

			uint16 value = 0;
			switch (opcode) {
			case EO_ACTOR_LOCAL_X:
				value = (uint32)actor->xFixed >> 12;
				break;

			case EO_ACTOR_LOCAL_Y:
				value = (uint32)actor->yFixed >> 12;
				break;

			case EO_ACTOR_WORLD_X:
			case EO_ACTOR_WORLD_Y: {
				bool useX = opcode == EO_ACTOR_WORLD_X;
				int32 position = useX ? actor->xFixed : actor->yFixed;

				while (actor->parent) {
					actor = actorGetPtr(actor->parent);
					if (!actor)
						break;

					position = (int32)((uint32)position + (uint32)(useX ? actor->xFixed : actor->yFixed));
				}

				value = (uint16)(position >> 12);
				break;
			}

			case EO_ACTOR_SPRITE_SELECTOR:
				value = (uint16)actor->spriteSelector;
				break;

			default:
				break;
			}

			if (_exprStackTop < COMFY_EXPR_STACK_CAPACITY)
				_exprStack[_exprStackTop++] = value;

			continue;
		}

		case EO_INDEXED_STRING_TABLE_VALUE: {
			if (_exprStackTop < 2) {
				_exprStackTop = 0;
				continue;
			}

			uint16 index = _exprStack[_exprStackTop - 2] + _exprStack[_exprStackTop - 1];
			_exprStackTop--;
			_exprStack[_exprStackTop - 1] = index < _stringTable.size() ? _stringTable[index] : 0;
			continue;
		}

		case EO_RANDOMIZE:
			if (_exprStackTop) {
				int16 maximum = _exprStack[_exprStackTop - 1];
				_exprStack[_exprStackTop - 1] = (uint16)(((int32)getRandomNumber(0x7FFF) * maximum) / 0x8000);
			}

			continue;

		default:
			break;
		}

		if (_engineVersion == 3) {
			uint16 value = 0;
			bool pushValue = true;
			bool handled = true;

			switch (opcode) {
			case EO_MOUSE_X:
				value = _mouseX;
				break;

			case EO_MOUSE_Y:
				value = _mouseY;
				break;

			case EO_WAVE_BALANCE_PERCENT:
				value = _v3SceneWaveBalancePercent;
				break;

			case EO_HOST_MEDIA_VALUE:
				value = _v3MediaValueAvailable ? _v3MediaValue : 0;
				break;

			case EO_NOOP_1B:
				pushValue = false;
				break;

			case EO_WAVE_LEFT_PERCENT:
				value = _v3SceneWaveLeftPercent;
				break;

			case EO_WAVE_RIGHT_PERCENT:
				value = _v3SceneWaveRightPercent;
				break;

			case EO_LOGICAL_DIMENSION:
				if (_exprStackTop) {
					uint16 dimension = _exprStack[_exprStackTop - 1];
					switch (dimension) {
					case 0x6E:
						_exprStack[_exprStackTop - 1] = 320;
						break;

					case 0x6F:
						_exprStack[_exprStackTop - 1] = 200;
						break;

					default:
						_exprStack[_exprStackTop - 1] = 0;
						break;
					}
				}

				pushValue = false;
				break;

			case EO_NOOP_1F:
				pushValue = false;
				break;

			case EO_HOST_MEDIA_PROGRESS:
				value = _v3MediaProgress;
				break;

			case EO_MIXER_VOLUME_PERCENT:
				value = _v3SceneMixerVolumePercent;
				break;

			default:
				handled = false;
				break;
			}

			if (handled) {
				if (pushValue && _exprStackTop < COMFY_EXPR_STACK_CAPACITY)
					_exprStack[_exprStackTop++] = value;

				continue;
			}
		}

		switch (opcode) {
		case EO_ADD:
		case EO_SUBTRACT:
		case EO_MULTIPLY:
		case EO_DIVIDE:
		case EO_MODULO:
			break;

		default:
			error("Unknown expression opcode 0x%02X at script PC 0x%08X", opcode, pc - 1);
		}

		if (_exprStackTop < 2) {
			_exprStackTop = 0;
			continue;
		}

		int16 rhs = _exprStack[--_exprStackTop];
		int16 lhs = _exprStack[_exprStackTop - 1];

		switch (opcode) {
		case EO_ADD:
			_exprStack[_exprStackTop - 1] = (uint16)(lhs + rhs);
			break;

		case EO_SUBTRACT:
			_exprStack[_exprStackTop - 1] = (uint16)(lhs - rhs);
			break;

		case EO_MULTIPLY:
			_exprStack[_exprStackTop - 1] = (uint16)(lhs * rhs);
			break;

		case EO_DIVIDE:
			if (!rhs) {
				_scriptFault = true;
				return 0;
			}

			_exprStack[_exprStackTop - 1] = (uint16)(lhs / rhs);
			break;

		case EO_MODULO:
			if (!rhs) {
				_scriptFault = true;
				return 0;
			}

			_exprStack[_exprStackTop - 1] = (uint16)(lhs % rhs);
			break;

		default:
			break;
		}
	}
}


} // End of namespace Comfy
