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

uint16 ComfyEngine::scriptEvalExpr(uint32 &pc, uint16 fallbackActor) {
	_exprStackTop = 0;
	for (;;) {
		byte opcode = scriptReadByte(pc++);
		if (_scriptFault)
			return 0;

		if (opcode == 0x0D)
			return _exprStackTop ? _exprStack[0] : 0;

		if (opcode == 0x01 || opcode == 0x03 || opcode == 0x15) {
			if (_exprStackTop < COMFY_EXPR_STACK_CAPACITY)
				_exprStack[_exprStackTop++] = scriptReadWord(pc);

			pc += 2;
			continue;
		}

		if (opcode == 0x02) {
			uint16 index = scriptReadWord(pc);
			pc += 2;
			uint16 value = index < _stringTable.size() ? _stringTable[index] : 0;
			if (_exprStackTop < COMFY_EXPR_STACK_CAPACITY)
				_exprStack[_exprStackTop++] = value;

			continue;
		}

		if (opcode >= 0x10 && opcode <= 0x14) {
			Actor *actor = actorResolve(scriptReadWord(pc), fallbackActor);
			pc += 2;
			if (!actor) {
				_scriptFault = true;
				return 0;
			}

			uint16 value = 0;
			if (opcode == 0x10)
				value = actorReadU32(*actor, kActorXFixed) >> 12;
			else if (opcode == 0x11)
				value = actorReadU32(*actor, kActorYFixed) >> 12;
			else if (opcode == 0x12 || opcode == 0x13) {
				uint offset = opcode == 0x12 ? kActorXFixed : kActorYFixed;
				int32 position = actorReadU32(*actor, offset);
				while (actorReadU16(*actor, kActorParent)) {
					actor = actorGetPtr(actorReadU16(*actor, kActorParent));
					if (!actor)
						break;

					position += actorReadU32(*actor, offset);
				}

				value = uint16(position >> 12);
			} else if (opcode == 0x14)
				value = actorReadU16(*actor, kActorSpriteSelector);

			if (_exprStackTop < COMFY_EXPR_STACK_CAPACITY)
				_exprStack[_exprStackTop++] = value;

			continue;
		}

		if (opcode == 0x16) {
			if (_exprStackTop) {
				int16 maximum = _exprStack[_exprStackTop - 1];
				_exprStack[_exprStackTop - 1] = uint16((int32(getRandomNumber(0x7FFF)) * maximum) / 0x8000);
			}

			continue;
		}

		if (_engineVersion == kEngineVersion3 && opcode >= 0x17 && opcode <= 0x21) {
			uint16 value = 0;
			bool pushValue = true;
			if (opcode == 0x17)
				value = _mouseX;
			else if (opcode == 0x18)
				value = _mouseY;
			else if (opcode == 0x19)
				value = _wcomfy99VocState0;
			else if (opcode == 0x1A)
				value = _wcomfy99HostMediaValueAvailable ? _wcomfy99HostMediaValue : 0;
			else if (opcode == 0x1B)
				pushValue = false;
			else if (opcode == 0x1C)
				value = _wcomfy99VocState2;
			else if (opcode == 0x1D)
				value = _wcomfy99VocState3;
			else if (opcode == 0x1E) {
				if (_exprStackTop) {
					uint16 dimension = _exprStack[_exprStackTop - 1];
					if (dimension == 0x6E)
						_exprStack[_exprStackTop - 1] = 320;
					else if (dimension == 0x6F)
						_exprStack[_exprStackTop - 1] = 200;
					else
						_exprStack[_exprStackTop - 1] = 0;
				}

				pushValue = false;
			} else if (opcode == 0x1F || opcode == 0x20) {
				value = _wcomfy99HostMediaProgress;
			} else if (opcode == 0x21) {
				value = _wcomfy99VocState6;
			}

			if (pushValue && _exprStackTop < COMFY_EXPR_STACK_CAPACITY)
				_exprStack[_exprStackTop++] = value;

			continue;
		}

		if (opcode == 0x0C) {
			if (_exprStackTop < 2) {
				_exprStackTop = 0;
				continue;
			}

			uint16 index = _exprStack[_exprStackTop - 2] + _exprStack[_exprStackTop - 1];
			_exprStackTop--;
			_exprStack[_exprStackTop - 1] = index < _stringTable.size() ? _stringTable[index] : 0;
			continue;
		}

		if (opcode != 0x08 && opcode != 0x09 && opcode != 0x0A && opcode != 0x0B && opcode != 0x0E)
			continue;

		if (_exprStackTop < 2) {
			_exprStackTop = 0;
			continue;
		}

		int16 rhs = _exprStack[--_exprStackTop];
		int16 lhs = _exprStack[_exprStackTop - 1];
		if (opcode == 0x08)
			_exprStack[_exprStackTop - 1] = uint16(lhs + rhs);
		else if (opcode == 0x09)
			_exprStack[_exprStackTop - 1] = uint16(lhs - rhs);
		else if (opcode == 0x0A)
			_exprStack[_exprStackTop - 1] = uint16(lhs * rhs);
		else if ((opcode == 0x0B || opcode == 0x0E) && !rhs) {
			_scriptFault = true;
			return 0;
		} else if (opcode == 0x0B)
			_exprStack[_exprStackTop - 1] = uint16(lhs / rhs);
		else if (opcode == 0x0E)
			_exprStack[_exprStackTop - 1] = uint16(lhs % rhs);
	}
}


} // End of namespace Comfy
