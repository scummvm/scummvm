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
#include "scriptexecutor.h"

#include "common/debug.h"
#include "common/debug-channels.h"
#include "common/memstream.h"
#include "macs2/adlib.h"
#include "macs2/macs2.h"
#include "macs2/gameobjects.h"
#include <macs2/view1.h>

namespace Macs2 {
namespace Script {

static Common::String joinDebugStrings(const Common::StringArray &strings) {
	Common::String result;
	for (uint i = 0; i < strings.size(); ++i) {
		if (i != 0)
			result += " | ";
		result += strings[i];
	}
	return result;
}

#define ScriptNoEntry debug("Unhandled case in script handling.");
#define STR_HELPER(x) #x

	


ScriptExecutor::ScriptExecutor() {
	// TODO: Hardcoded values for testing
	constexpr int numVariables = 1000;
	_variables.resize(numVariables);
	for (int i = 0; i < numVariables; i++) {
		_variables[i].a = 0;
		_variables[i].b = 0;
	}
	/* _variables[0xb].a = 0x3;
	// TODO: This needs to be set for opening the box to work
	_variables[0xe].a = 0x1;
	_interactedObjectID = 0x080a; 
	// Hardcoding for the box to start open for testing
	*/
	// Hardcoded values for failed throw at leopard below
	// _interactedObjectID = 0x409;
	// _variables[0xa].a = 0x1;
	// _variables[0x26].a = 0x1;
}

Common::String ScriptExecutor::IdentifyScriptOpcode(uint8 opcode, uint8 opcode2) {
	if (opcode == 0x5)
		return Common::String::format("(%.2x)", opcode);

	return Common::String::format("(%.2x %.2x)", opcode, opcode2);
}

Common::String ScriptExecutor::IdentifyHelperOpcode(uint8 opcode, uint16 value) {
	return Common::String::format("(%.2x %.4x)", opcode, value);
}

inline void ScriptExecutor::scriptSkipBlock() {
	lastOpcodeTriggeredSkip = true;
	if (DebugMan.isDebugChannelEnabled(DebugFlag::DEBUG_SV)) {
		debug("-- Entering A3D2");
	} else {
		debug("-- Skipping using A3D2");
	}
	
	isSkipping = true;
	if (expectedEndLocation != _stream->pos()) {
		warning("Macs2::ScriptExecutor::scriptSkipBlock resyncing stream from %u to %u",
				(uint32)expectedEndLocation, (uint32)_stream->pos());
		expectedEndLocation = _stream->pos();
	}
	int skipDepth = 1;
	while ((skipDepth != 0) && (_stream->pos() < _stream->size())) {
		const byte opcode = ReadByte();
		if (_stream->pos() >= _stream->size()) {
			debugC(DEBUG_SV, "- A3D2 hit end of stream after opcode %.2x [%d]", opcode, skipDepth);
			break;
		}
		const byte length = ReadByte();
		if (opcode >= 3) {
			if (opcode <= 6) {
				skipDepth++;
			}
		}
		if ((opcode == 8) && (skipDepth == 1)) {
			skipDepth = 0;
		}
		if (opcode == 7) {
			skipDepth--;
		}

		const int64 remainingBytes = _stream->size() - _stream->pos();
		if (length > remainingBytes) {
			debugC(DEBUG_SV, "- A3D2 clamping truncated block: opcode %.2x length %u remaining %lld [%d]",
					opcode, length, (long long)remainingBytes, skipDepth);
			_stream->seek(_stream->size(), SEEK_SET);
			break;
		}

		_stream->seek(length, SEEK_CUR);
		debugC(DEBUG_SV, "- A3D2 skipping %u bytes for opcode %.2x [%d]", length, opcode, skipDepth);
	}

	if (skipDepth != 0) {
		debugC(DEBUG_SV, "- A3D2 left skip block early at %lld/%lld [%d]",
				(long long)_stream->pos(), (long long)_stream->size(), skipDepth);
	}

	// Fix up the expected location after skipping
	expectedEndLocation = _stream->pos();
	if (DebugMan.isDebugChannelEnabled(DebugFlag::DEBUG_SV)) {
		debug("-- Leaving A3D2");
	}
	isSkipping = false;
}

void ScriptExecutor::scriptSkipAlternate() {
	if (DebugMan.isDebugChannelEnabled(DebugFlag::DEBUG_SV)) {
		debug("-- Entering A37A");
	} else {
		debug("-- Skipping using A37A");
	}

	isSkipping = true;
	if (expectedEndLocation != _stream->pos()) {
		warning("Macs2::ScriptExecutor::scriptSkipAlternate resyncing stream from %u to %u",
				(uint32)expectedEndLocation, (uint32)_stream->pos());
		expectedEndLocation = _stream->pos();
	}
	int skipDepth = 1;
	while ((skipDepth != 0) && (_stream->pos() < _stream->size())) {
		const byte opcode = ReadByte();
		if (_stream->pos() >= _stream->size()) {
			debugC(DEBUG_SV, "- A37A hit end of stream after opcode %.2x [%d]", opcode, skipDepth);
			break;
		}
		const byte length = ReadByte();
		if (opcode >= 3) {
			if (opcode <= 6) {
				skipDepth++;
			}
		}
		if (opcode == 7) {
			skipDepth--;
		}

		const int64 remainingBytes = _stream->size() - _stream->pos();
		if (length > remainingBytes) {
			debugC(DEBUG_SV, "- A37A clamping truncated block: opcode %.2x length %u remaining %lld [%d]",
					opcode, length, (long long)remainingBytes, skipDepth);
			_stream->seek(_stream->size(), SEEK_SET);
			break;
		}

		_stream->seek(length, SEEK_CUR);
		debugC(DEBUG_SV, "- A37A skipping %u bytes for opcode %.2x [%d]", length, opcode, skipDepth);
	}

	if (skipDepth != 0) {
		debugC(DEBUG_SV, "- A37A left skip block early at %lld/%lld [%d]",
				(long long)_stream->pos(), (long long)_stream->size(), skipDepth);
	}
	
	// Fix up the expected location after skipping
	expectedEndLocation = _stream->pos();
	if (DebugMan.isDebugChannelEnabled(DebugFlag::DEBUG_SV)) {
		debug("-- Leaving A37A");
	}
	isSkipping = false;
}

void ScriptExecutor::SkipUntil14() {
	uint16 tag = ReadWord();
	_stream->seek(0, SEEK_SET);
	while (_stream->pos() < _stream->size()) {
		uint8 opcode = ReadByte();
		uint8 length = ReadByte();
		if (opcode == 0x14) {
			uint16 tag14 = ReadWord();
			if (tag14 == tag) {
				return;
			}
		} else {
			_stream->seek(length, SEEK_CUR);
		}
	}
}

void ScriptExecutor::scriptReadValuePair(uint16 &out1, uint16 &out2) {
	// Results are [bp-4h] and [bp-2h]
	// TODO: Implement the actual prelude here correctly, documenting which lables we pass as we go
	// debug("-- Entering 9F4D");
	// fn0037_9F4D proc
	out1 = 0;
	out2 = 0;

	byte opcode1 = ReadByte(); // [bp-5h]
	
	// TODO: Consider writing this one also 
	uint16 value = ReadWord(); // [bp-7h]

	Common::String opcodeInfo = IdentifyHelperOpcode(opcode1, value);

	debug("- 9F4D opcode: %.2x %.4x %s", opcode1, value, opcodeInfo.c_str());


	if (opcode1 == 0x0) {
		// l0037_9F67:
		out1 = value;
		out2 = 0;
		// TODO: Do we need to do something to exit?
		debug("- 9F4D results: %.4x %.4x", out1, out2);
		return;
	}
	// l0037_9F72:
	if (opcode1 > 0) {
		// l0037_9F78:
		if (opcode1 < 0xFF) {
			// TODO: This still feels off since it should not be possible
			if ((value < 1) || (value > 0x800))  {
				/*
				We reach this by value being less than 1 or more than 800
				l0037_9F8B:
					mov	word ptr [1028h],1Ah
					jmp	0A32Ch
				*/
				// TODO: Implement the jump
				// TODO: Add a return macro
				debug("- 9F4D results: %.4x %.4x", out1, out2);
				return;
			}
			else {
				// l0037_9F94:
				// value between 1 and 0x800
				const ScriptVariable& var = _variables[value];
				out1 = var.a;
				out2 = var.b;
				// TODO: Centralized return handling
				debug("- 9F4D results: %.4x %.4x", out1, out2);
				return;
			}
		}
	}
	// l0037_9FAE:
	if (opcode1 != 0xFF) {
		// TODO: Do we write out a 0 for the values?
		debug("- 9F4D results: %.4x %.4x", out1, out2);
		return;
	}
	// We are starting to execute opcode FFh here
	// l0037_9FB7:
	if (value == 0x1) {
		// l0037_9FBF:
		if (_mouseMode == MouseMode::Use) {
			// l0037_9FC7:
			out1 = _interactedObjectID;
			out2 = _interactedOtherObjectID;
			debug("- 9F4D results: %.4x %.4x", out1, out2);
			return;
		} else if (_mouseMode == MouseMode::UseInventory) {
			// l0037_9FD9:
			// TODO: Not sure why the original code looks so complex
			out1 = _interactedObjectID;
			out2 = _interactedOtherObjectID;
			debug("- 9F4D results: %.4x %.4x", out1, out2);
			return;
		} else {
			out1 = out2 = 0x0000;
		}
	} else if (value == 0x3) {
		if (_mouseMode == MouseMode::Talk) {
			out1 = _interactedObjectID;
			out2 = 0;
		} else {
			out1 = out2 = 0x0000;
		}	
		debug("- 9F4D results: %.4x %.4x", out1, out2);
		return;
	// l0037_A050:
	} else if (value == 0x4) {
		// TODO: What's the difference to 0x27?
		// TODO: Expose the position of the character sprite (or his feet)
		const Common::Point &charPos = GetCharPosition();
		out1 = getAreaAtPoint(charPos.x, charPos.y);
		out2 = 0;
		// TODO: In the logs there is also a value out2 (DX) returned - where
		// does that come from?
		debug("- 9F4D results: %.4x %.4x", out1, out2);
		return;
		/*
		mov	di,[0776h]
	shl	di,2h
	les	di,[di+77Ch]
	push	word ptr es:[di]
	push	word ptr es:[di+2h]
	call	far 0037h:101Dh
	cwd
	mov	[bp-4h],ax
	mov	[bp-2h],dx
	jmp	0A32Ch*/

	} else if (value == 0x7) {
		// l0037_A095:
		out1 = out2 = 0;
		debug("- 9F4D results: %.4x %.4x", out1, out2);
		return;
	} else if (value == 0x8 || value == 0x9) {
		out1 = 0;
		out2 = 0;
		debug("- 9F4D results: %.4x %.4x", out1, out2);
		return;
	}

	// l0037_A008:
/*

;; This handles opcode FFh
	mov	ax,[bp-7h]
	cmp	ax,1h
	jnz	0A008h

l0037_9FBF:
	;; This is the case of [bp-7h] being 1h
	mov	ax,[0774h]
	cmp	ax,15h
	jnz	9FD4h

l0037_9FC7:
	mov	ax,[1024h]
	xor	dx,dx
	mov	[bp-4h],ax
	mov	[bp-2h],dx
	jmp	0A005h

l0037_9FD4:
	cmp	ax,17h
	jnz	9FFDh

l0037_9FD9:
	mov	ax,[1026h]
	xor	dx,dx
	mov	cx,10h
	xor	bx,bx
	call	far 00CDh:0D93h
	mov	cx,ax
	mov	bx,dx
	mov	ax,[1024h]
	xor	dx,dx
	or	ax,cx
	or	dx,bx
	mov	[bp-4h],ax
	mov	[bp-2h],dx
	jmp	0A005h

l0037_9FFD:
	xor	ax,ax
	mov	[bp-4h],ax
	mov	[bp-2h],ax

l0037_A005:
	jmp	0A32Ch

l0037_A008:
	cmp	ax,2h
	jnz	0A02Ch

l0037_A00D:
	cmp	word ptr [0774h],14h
	jnz	0A021h

l0037_A014:
	;; This code is called for issuing a look on an hotspot
	;; Access the active hotspot
	mov	ax,[1024h]
	xor	dx,dx
	;; [bp-4h] gets the active hotspot
	mov	[bp-4h],ax
	;; [bp-2h] gets dx = 0
	mov	[bp-2h],dx
	jmp	0A029h

l0037_A021:
	xor	ax,ax
	mov	[bp-4h],ax
	mov	[bp-2h],ax

l0037_A029:
	;; End handling of cursor mode 14h
	jmp	0A32Ch
*/
	else if (value == 0x2) {
		out1 = _mouseMode == MouseMode::Look ? _interactedObjectID : 0;
		out2 = 0;
		debug("- 9F4D results: %.4x %.4x", out1, out2);
		return;
		} 
/*
l0037_A02C:
	cmp	ax,3h
	jnz	0A050h

l0037_A031:
	cmp	word ptr [0774h],13h
	jnz	0A045h

l0037_A038:
	mov	ax,[1024h]
	xor	dx,dx
	mov	[bp-4h],ax
	mov	[bp-2h],dx
	jmp	0A04Dh

l0037_A045:
	xor	ax,ax
	mov	[bp-4h],ax
	mov	[bp-2h],ax

l0037_A04D:
	jmp	0A32Ch

l0037_A050:
	cmp	ax,4h
	jnz	0A076h

l0037_A055:
	mov	di,[0776h]
	shl	di,2h
	les	di,[di+77Ch]
	push	word ptr es:[di]
	push	word ptr es:[di+2h]
	call	far 0037h:101Dh
	cwd
	mov	[bp-4h],ax
	mov	[bp-2h],dx
	jmp	0A32Ch

l0037_A076:
	cmp	ax,5h
	jnz	0A07Eh

l0037_A07B:
	jmp	0A32Ch

l0037_A07E:
	cmp	ax,6h
	jnz	0A090h

l0037_A083:
	mov	word ptr [bp-4h],1h
	mov	word ptr [bp-2h],0h
	jmp	0A32Ch

	*/

else if (value == 0x6) {
	
	out1 = 1;
	out2 = 0;
	debug("- 9F4D results: %.4x %.4x", out1, out2);
	return;
	} else if (value == 0xb) {

		out1 = (uint16)IsRepeatRun;
		out2 = 0;
		debug("- 9F4D results: %.4x %.4x", out1, out2);
		return;
	} else if (value == 0xc) {
		out1 = 1;
		out2 = 0;
		debug("- 9F4D results: %.4x %.4x", out1, out2);
		return;
	}

/*
l0037_A090:
	cmp	ax,7h
	jnz	0A0A0h

l0037_A095:
	xor	ax,ax
	mov	[bp-4h],ax
	mov	[bp-2h],ax
	jmp	0A32Ch

l0037_A0A0:
	cmp	ax,8h
	jnz	0A0B0h

l0037_A0A5:
	xor	ax,ax
	mov	[bp-4h],ax
	mov	[bp-2h],ax
	jmp	0A32Ch

l0037_A0B0:
	cmp	ax,9h
	jnz	0A0C0h

l0037_A0B5:
	xor	ax,ax
	mov	[bp-4h],ax
	mov	[bp-2h],ax
	jmp	0A32Ch

l0037_A0C0:
	cmp	ax,0Ah
	jnz	0A0D2h
	*/
	else if (value == 0xa) {

		out1 = 1;
		out2 = 0;
		debug("- 9F4D results: %.4x %.4x", out1, out2);
		return;
	}
		/*
	l0037_A0C5:
	mov	word ptr [bp-4h],1h
	mov	word ptr [bp-2h],0h
	jmp	0A32Ch

l0037_A0D2:
	cmp	ax,0Bh
	jnz	0A0F5h

l0037_A0D7:
	cmp	byte ptr [1012h],0h
	jz	0A0EAh

l0037_A0DE:
	mov	word ptr [bp-4h],1h
	mov	word ptr [bp-2h],0h
	jmp	0A0F2h

l0037_A0EA:
	xor	ax,ax
	mov	[bp-4h],ax
	mov	[bp-2h],ax

l0037_A0F2:
	jmp	0A32Ch

l0037_A0F5:
	cmp	ax,0Ch
	jnz	0A107h

l0037_A0FA:
	mov	word ptr [bp-4h],1h
	mov	word ptr [bp-2h],0h
	jmp	0A32Ch

l0037_A107:
	cmp	ax,0Dh
	jnz	0A120h

l0037_A10C:
	les	di,[0778h]
	mov	ax,es:[di+53B7h]
	xor	dx,dx
	mov	[bp-4h],ax
	mov	[bp-2h],dx
	jmp	0A32Ch
	*/
else if (value == 0x0d) {
	// TODO: Confirm this one
	out1 = chosenDialogueOption;
	out2 = 0;
	debug("- 9F4D results: %.4x %.4x", out1, out2);
	return;
// l0037_A120:
} else if (value >= 0xE && value <= 0x22) {
	// l0037_A12A:
	out1 = value - 0x0D;
	out2 = 0;
	return;
} else if (value == 0x23) {
	out1 = pathWalkableResult ? 1 : 0;
	out2 = 0;
	debug("- 9F4D results: %.4x %.4x", out1, out2);
	return;
}
	/*
l0037_A13B:
	cmp	ax,23h
	jnz	0A15Eh

l0037_A140:
	cmp	byte ptr [103Ah],0h
	jz	0A153h

l0037_A147:
	mov	word ptr [bp-4h],1h
	mov	word ptr [bp-2h],0h
	jmp	0A15Bh

l0037_A153:
	xor	ax,ax
	mov	[bp-4h],ax
	mov	[bp-2h],ax

l0037_A15B:
	jmp	0A32Ch

l0037_A15E:
	cmp	ax,24h
	jnz	0A17Bh

l0037_A163:
	mov	di,[0776h]
	shl	di,2h
	les	di,[di+77Ch]
	mov	ax,es:[di]
	cwd
	mov	[bp-4h],ax
	mov	[bp-2h],dx
	jmp	0A32Ch
*/
else if (value == 0x24) {
	const GameObject *actor = GameObjects::instance().GetObjectByIndex(Scenes::instance().CurrentActorIndex);
	if (!actor)
		actor = GameObjects::GetProtagonistObject();
	out1 = actor ? actor->Position.x : 0;
	out2 = 0;
	debug("- 9F4D results: %.4x %.4x", out1, out2);
	return;
}
/*

l0037_A17B:
	cmp	ax,25h
	jnz	0A199h

l0037_A180:
	mov	di,[0776h]
	shl	di,2h
	les	di,[di+77Ch]
	mov	ax,es:[di+2h]
	cwd
	mov	[bp-4h],ax
	mov	[bp-2h],dx
	jmp	0A32Ch
	*/
else if (value == 0x25) {
	const GameObject *actor = GameObjects::instance().GetObjectByIndex(Scenes::instance().CurrentActorIndex);
	if (!actor)
		actor = GameObjects::GetProtagonistObject();
	out1 = actor ? actor->Position.y : 0;
	out2 = 0;
	debug("- 9F4D results: %.4x %.4x", out1, out2);
	return;
}

	// l0037_A199:
	else if (value == 0x26) {
		/*
		l0037_A19E:
	cmp	byte ptr [1014h],0h
	jz	0A1B1h

	l0037_A1A5:
	mov	word ptr [bp-4h],1h
	mov	word ptr [bp-2h],0h
	jmp	0A1B9h

l0037_A1B1:
	xor	ax,ax
	mov	[bp-4h],ax
	mov	[bp-2h],ax
	
l0037_A1B9:
	jmp	0A32Ch
		*/
		out1 = (uint16)IsSceneInitRun;
		out2 = 0;
		debug("- 9F4D results: %.4x %.4x", out1, out2);
		return;
	}
	else if (value == 0x27) {
		// TODO: Handle this part
		/*
		l0037_A1C1:
	cmp	byte ptr [1032h],0h
	jz	0A1E6h
		*/

		// TODO: Expose the position of the character sprite (or his feet)
		const Common::Point &charPos = GetCharPosition();
		if (isRepeatRun) {
			out1 = getAreaAtPoint(charPos.x, charPos.y);
		} else {
			out1 = 0; //  
		}
		
		// TODO: In the logs there is also a value out2 (DX) returned - where
		// does that come from?
		// TODO: Fixing to 0 for now
		out2 = 0;
		debug("- 9F4D results: %.4x %.4x", out1, out2);
		return;
	}
	
	
	/*
l0037_A1BC:
	cmp	ax,27h
	jnz	0A1E9h

l0037_A1C1:
	cmp	byte ptr [1032h],0h
	jz	0A1E6h

l0037_A1C8:
	mov	di,[0776h]
	shl	di,2h
	les	di,[di+77Ch]
	push	word ptr es:[di]
	push	word ptr es:[di+2h]
	call	far 0037h:101Dh
	cwd
	mov	[bp-4h],ax
	mov	[bp-2h],dx

l0037_A1E6:
	jmp	0A32Ch


l0037_A1E9:
	cmp	ax,28h
	jnz	0A20Ch

l0037_A1EE:
	cmp	byte ptr [103Ch],0h
	jz	0A201h

l0037_A1F5:
	mov	word ptr [bp-4h],1h
	mov	word ptr [bp-2h],0h
	jmp	0A209h

l0037_A201:
	xor	ax,ax
	mov	[bp-4h],ax
	mov	[bp-2h],ax

l0037_A209:
	jmp	0A32Ch

*/
	else if (value == 0x28) {
		out1 = inventoryCheckResult ? 1 : 0;
		out2 = 0x0;
		debug("- 9F4D results: %.4x %.4x", out1, out2);
		return;
	} else if (value == 0x29) {
		out1 = animBlobRangeTestResult ? 1 : 0;
		out2 = 0;
		debug("- 9F4D results: %.4x %.4x", out1, out2);
		return;
	}

/*

l0037_A20C:
	cmp	ax,29h
	jnz	0A22Fh

l0037_A211:
	cmp	byte ptr [103Eh],0h
	jz	0A224h

l0037_A218:
	mov	word ptr [bp-4h],1h
	mov	word ptr [bp-2h],0h
	jmp	0A22Ch

l0037_A224:
	xor	ax,ax
	mov	[bp-4h],ax
	mov	[bp-2h],ax

l0037_A22C:
	jmp	0A32Ch

l0037_A22F:
	cmp	ax,2Ah
	jnz	0A259h

l0037_A234:
	cmp	byte ptr [1042h],0h
	jz	0A24Eh

l0037_A23B:
	cmp	word ptr [0FD2h],0h
	jnz	0A24Eh

l0037_A242:
	mov	word ptr [bp-4h],1h
	mov	word ptr [bp-2h],0h
	jmp	0A256h
	*/
	else if (value == 0x2A) {
		View1 *currentView = (View1 *)_engine->findView("View1");
		const bool uiOpen = currentView != nullptr &&
			(currentView->_isShowingInventory || currentView->_isShowingStringBox || currentView->isShowingMainMenu);
		out1 = (inventoryCombineFlag && !uiOpen) ? 1 : 0;
		out2 = 0;
		debug("- 9F4D results: %.4x %.4x", out1, out2);
		return;
	} else if (value == 0x2B) {
		View1 *currentView = (View1 *)_engine->findView("View1");
		const bool uiOpen = currentView != nullptr &&
			(currentView->_isShowingInventory || currentView->_isShowingStringBox || currentView->isShowingMainMenu);
		out1 = (inventoryActionFlag && !uiOpen) ? 1 : 0;
		out2 = 0;
		debug("- 9F4D results: %.4x %.4x", out1, out2);
		return;
	} else if (value == 0x2C) {
		out1 = _mouseMode == MouseMode::PanelUse ? _interactedObjectID : 0;
		out2 = 0;
		debug("- 9F4D results: %.4x %.4x", out1, out2);
		return;
	}

/*


l0037_A24E:
	xor	ax,ax
	mov	[bp-4h],ax
	mov	[bp-2h],ax

l0037_A256:
	jmp	0A32Ch

l0037_A259:
	cmp	ax,2Bh
	jnz	0A283h

l0037_A25E:
	cmp	byte ptr [1040h],0h
	jz	0A278h

l0037_A265:
	cmp	word ptr [0FD2h],0h
	jnz	0A278h

l0037_A26C:
	mov	word ptr [bp-4h],1h
	mov	word ptr [bp-2h],0h
	jmp	0A280h

l0037_A278:
	xor	ax,ax
	mov	[bp-4h],ax
	mov	[bp-2h],ax

l0037_A280:
	jmp	0A32Ch

l0037_A283:
	cmp	ax,2Ch
	jnz	0A2A7h

l0037_A288:
	cmp	word ptr [0774h],18h
	jnz	0A29Ch

l0037_A28F:
	mov	ax,[1024h]
	xor	dx,dx
	mov	[bp-4h],ax
	mov	[bp-2h],dx
	jmp	0A2A4h

l0037_A29C:
	xor	ax,ax
	mov	[bp-4h],ax
	mov	[bp-2h],ax

l0037_A2A4:
	jmp	0A32Ch
*/
	else if (value == 0x2D) {
		out1 = Scenes::instance().CurrentSceneIndex;
		out2 = 0;
	} else if (value == 0x2E) {
		out1 = 2;
		out2 = 0;
		debug("- 9F4D results: %.4x %.4x", out1, out2);
		return;
	}

	/*
	l0037_A2A7:
	cmp	ax,2Dh
	jnz	0A2B9h

l0037_A2AC:
	mov	ax,[077Ch]
	xor	dx,dx
	mov	[bp-4h],ax
	mov	[bp-2h],dx
	jmp	0A32Ch

l0037_A2B9:
	cmp	ax,2Eh
	jnz	0A2CAh

l0037_A2BE:
	mov	word ptr [bp-4h],2h
	mov	word ptr [bp-2h],0h
	jmp	0A32Ch

l0037_A2CA:
	cmp	ax,2Fh
	jnz	0A2DCh

l0037_A2CF:
	mov	ax,[077Eh]
	xor	dx,dx
	mov	[bp-4h],ax
	mov	[bp-2h],dx
	jmp	0A32Ch
*/
	else if (value == 0x2F) {
		out1 = Scenes::instance().LastSceneIndex;
		out2 = 0;
		debug("- 9F4D results: %.4x %.4x", out1, out2);
		return;
	}
/*
l0037_A2DC:
	cmp	ax,30h
	jnz	0A305h

l0037_A2E1:
	cmp	byte ptr [06C0h],0h
	jz	0A2FBh

l0037_A2E8:
	cmp	byte ptr [1F4Ch],0h
	jz	0A2FBh

l0037_A2EF:
	mov	word ptr [bp-4h],1h
	mov	word ptr [bp-2h],0h
	jmp	0A303h

l0037_A2FB:
	xor	ax,ax
	mov	[bp-4h],ax
	mov	[bp-2h],ax

l0037_A303:
	jmp	0A32Ch

l0037_A305:
	cmp	ax,31h
	jnz	0A32Ch

l0037_A30A:
	cmp	byte ptr [06BEh],0h
	jz	0A324h

l0037_A311:
	cmp	byte ptr [1F4Ch],0h
	jz	0A324h

l0037_A318:
	mov	word ptr [bp-4h],1h
	mov	word ptr [bp-2h],0h
	jmp	0A32Ch

l0037_A324:
	xor	ax,ax
	mov	[bp-4h],ax
	mov	[bp-2h],ax

*/
	else if (value == 0x30) {
		out1 = (musicEnabled && mapPanelActive) ? 1 : 0;
		out2 = 0;
		debug("- 9F4D results: %.4x %.4x", out1, out2);
		return;
	} else if (value == 0x31) {
		out1 = (soundEnabled && mapPanelActive) ? 1 : 0;
		out2 = 0;
		debug("- 9F4D results: %.4x %.4x", out1, out2);
		return;
	} else {
		// TODO: Handle others
		ScriptUnimplementedOpcode_Helper(value);
	}

/*

l0037_A32C:
	;; Central end of the function
	;; For the case of mode 14h with a hotspot, we return AX = content of [1024h] and DX = 0
	mov	ax,[bp-4h]
	mov	dx,[bp-2h]
	leave
	retf
	*/

	debug("- 9F4D results: %.4x %.4x", out1, out2);
	// debug("-- Leaving 94FD");
}

void ScriptExecutor::scriptReadValue_Placeholder() {
	scriptReadValue32();
}

uint32 ScriptExecutor::scriptReadValue32() {
	
	uint16 out1;
	uint16 out2;
	scriptReadValuePair(out1, out2);

	// TODO: Probably not portable
	return (static_cast<uint32>(out2) << 16) + static_cast<uint32>(out1);
}

uint16 ScriptExecutor::scriptReadValue16() {
	uint16 out1;
	uint16 out2;
	scriptReadValuePair(out1, out2);
	return out1;
}

void ScriptExecutor::scriptSaveVariable(uint32 value) {
	// TODO: Why is this byte never really used?
	ReadByte(); // unused byte

	uint16 variableID = ReadWord();
	SetVariableValue(variableID, value);
}

void ScriptExecutor::scriptLoadSpecialAnimImpl() {

	uint16 objectID1;
	uint16 objectID2;
	scriptReadValuePair(objectID1, objectID2);
	// [bp-2h]
	// uint16 offset1 = objectID1 - 0x400;
	// TODO: Should handle this as a double word
	objectID1 -= 0x400;

	// {[bp-0Dh]
	uint16 out1;
	uint16 out2;
	scriptReadValuePair(out1, out2);

	
}

void ScriptExecutor::scriptStopAnimationImpl() {
	// scriptStopAnimation (1008:c8e4). Original behavior:
	//   1. Read objectID, validate
	//   2. Set runtime +0x22D = 0x7FFF (remove direction/frame limit)
	//   3. Free overload animation blob if loaded (runtime +0x183 flag)
	//   4. Clear overload flag
	uint32 characterID = scriptReadValue32() - 0x400;
	GameObject *obj = GameObjects::GetObjectByIndex(characterID);
	if (obj == nullptr) {
		warning("Ignoring stop animation for missing object %u", characterID);
		return;
	}
	obj->useOverloadAnimation = false;
	obj->overloadAnimation.clear();
}

void ScriptExecutor::scriptChangeAnimationImpl() {
	// bp-2h
	uint32 id = scriptReadValue32();
	id -= 0x1000;
	// bp-4h
	uint16 bp4 = scriptReadValue16();
	if (id <= 0) {
		// mov	word ptr [1028h],8h
		return;
	}
	// l0037_B6F1:
	// TODO: Check if we try to access an invalid index

	// l0037_B715:
	// TODO: Load the data of the animation

	// l0037_B73C:
	// Use the function to extract some value
	// Subtracting an additional 1 since mine are indexed from 0 and not 1 like the game does
	id -= 1;

	BackgroundAnimationBlob& blob = _engine->_backgroundAnimationsBlobs[id];
	BackgroundAnimationBlob::getAnimFrameCount(blob.Blob);
	// TODO: We should be doing some checking on the result value

	// TODO: Do some comparison with [bp-4h]
	BackgroundAnimationBlob::advanceAnimFrame(blob.Blob, true, bp4 + 0x64);
}

void ScriptExecutor::scriptChangeAnimationWrapper() {
	// TODO: This is a very simplistic implementation, there's a lot
	// to be discovered how this works exaclty
	uint16 id1;
	uint16 id2;
	// TODO: Should handle this as a 32 bit number
	scriptReadValuePair(id1, id2);
	uint16 animFrame;
	uint16 throwaway;
	scriptReadValuePair(animFrame, throwaway);

	// TODO: Access the animation based on the ID
	// Can do it hardcoded for now, but will need to figure out the relationship
	// further down the road

	// Subtracting an additional 1 since mine are indexed from 0 and not 1 like the game does
	uint16 id = id1 - 0x1000 - 1;
	// TODO: This is not accurate, but in practice we should be able to get by with just ping-
	// ponging between animations. In actual practice, we need to read the data correctly
	// and apply the logic from the 1480 function to it
	_engine->_backgroundAnimations[id].FrameIndex++;
	// TODO: Implement proper wrap around based on 1480
	_engine->_backgroundAnimationsBlobs[id].FrameIndex++;
	_engine->_backgroundAnimations[id].FrameIndex = (_engine->_backgroundAnimations[id].FrameIndex + 1) % _engine->_backgroundAnimations[id].numFrames;
	

}

uint16 ScriptExecutor::getAreaAtPoint(uint16 x, uint16 y) {
	uint16 result = _engine->_pathfindingMap.getPixel(x, y);
	// TODO: Need to be careful, there are different functions accessing
	// pathfinding map overrides in different ways!
	// l0037_1078:
	if (result < 0xC8 || result > 0xF9) {
		return result;
	}


	// Get the override value
	// l0037_107F:
	uint8 overrideValue = g_engine->GetPathfindingOverride2(result);
	if (overrideValue >= 0xC8) {
		// l0037_109F:
		result = overrideValue;
	}

	// l0037_10B7:
	// Reminder that this data can be adjusted with a script opcode
	return result;
}

bool ScriptExecutor::IsPathWalkable(const Common::Point &from, const Common::Point &to) {
	int x1 = from.x;
	int y1 = from.y;
	const int x2 = to.x;
	const int y2 = to.y;
	const int dx = abs(x2 - x1);
	const int dy = abs(y2 - y1);
	const int sx = (x1 < x2) ? 1 : -1;
	const int sy = (y1 < y2) ? 1 : -1;
	int err = dx - dy;

	while (true) {
		if (getAreaAtPoint(x1, y1) >= 0xC8)
			return false;
		if (x1 == x2 && y1 == y2)
			return true;

		const int e2 = 2 * err;
		if (e2 > -dy) {
			err -= dy;
			x1 += sx;
		}
		if (e2 < dx) {
			err += dx;
			y1 += sy;
		}
	}
}

bool ScriptExecutor::loadIndexedResource(Common::Array<uint8> &outData, uint8 resourceIndex, uint16 objectTableOffset) {
	if (resourceIndex == 0) {
		warning("Ignoring resource load for zero resource index");
		return false;
	}

	const int64 oldPos = g_engine->_fileStream->pos();
	uint32 address = 0;

	if (_executingScriptObjectID == 0) {
		if (resourceIndex > _engine->array520D.size()) {
			warning("Ignoring resource load for missing scene resource %u", resourceIndex);
			return false;
		}
		address = _engine->array520D[resourceIndex - 1];
	} else {
		GameObject *object = GameObjects::GetObjectByIndex(_executingScriptObjectID);
		if (object == nullptr || object->DataOffset == 0) {
			warning("Ignoring resource load for missing object %u resource %u", _executingScriptObjectID, resourceIndex);
			return false;
		}
		g_engine->_fileStream->seek(object->DataOffset + objectTableOffset + (resourceIndex - 1) * 4, SEEK_SET);
		address = g_engine->_fileStream->readUint32LE();
	}

	if (address == 0) {
		warning("Ignoring resource load for empty resource %u", resourceIndex);
		g_engine->_fileStream->seek(oldPos, SEEK_SET);
		return false;
	}

	g_engine->_fileStream->seek(address, SEEK_SET);
	const uint32 size = g_engine->_fileStream->readUint32LE();
	if (size == 0) {
		warning("Ignoring resource load for zero-sized resource %u", resourceIndex);
		g_engine->_fileStream->seek(oldPos, SEEK_SET);
		return false;
	}
	outData.resize(size);
	g_engine->_fileStream->read(outData.data(), size);
	g_engine->_fileStream->seek(oldPos, SEEK_SET);
	return !outData.empty();
}

bool ScriptExecutor::loadSoundResource(Common::Array<uint8> &outData, uint8 resourceIndex) {
	return loadIndexedResource(outData, resourceIndex);
}

bool ScriptExecutor::loadMusicResource(Common::Array<uint8> &outData, uint8 resourceIndex) {
	return loadIndexedResource(outData, resourceIndex);
}

void ScriptExecutor::ScriptPrintString(bool alignRight) {

	// TODO: Labels above not handled yet
	// TODO: Lots of details not handled
	// l0037_A94E:

	uint16 x = scriptReadValue16();
	uint16 y = scriptReadValue16();
	// TODO: Several globals writes around this code
	uint16 bp2 = ReadWord();
	uint16 bp4 = ReadWord();

	// TODO: Implement naive string printing here, refine later

	Common::StringArray strings;
	if (_executingScriptObjectID == 0) {
		strings = g_engine->DecodeStrings(Scenes::instance().CurrentSceneStrings, bp2, bp4);
	} else {
		Common::MemoryReadStream *s = GameObjects::ReadGameObjectStrings(_executingScriptObjectID, g_engine->_fileStream);
		strings = g_engine->DecodeStrings(s, bp2, bp4);
	}
	
	if (alignRight) {
		x -= g_engine->MeasureStrings(strings) + 0x12;
	}

	// TODO: Look for good pattern for the view, this feels like it is not intended this way
	View1 *currentView = (View1 *)_engine->findView("View1");
	currentView->setStringBoxAt(strings, Common::Point(x, y));
}

void ScriptExecutor::BeginBuffering() {
	lastOpcodeTriggeredSkip = false;
	debugBuffer.clear();
}

void ScriptExecutor::EndBuffering(bool shouldMark) {
	(void)shouldMark;
	lastOpcodeTriggeredSkip = false;
	debugBuffer.clear();
}

void ScriptExecutor::SetVariableValue(uint16 index, uint16 a, uint16 b) {
	_variables[index].a = a;
	_variables[index].b = b;
}

void ScriptExecutor::SetVariableValue(uint16 index, uint32 value) {
	uint16 a = static_cast<uint16>(value >> 16);    // High 16 bits
	uint16 b = static_cast<uint16>(value & 0xFFFF); // Low 16 bits
	SetVariableValue(index, b, a);
}

Common::Point ScriptExecutor::GetCharPosition() {
	const GameObject *actor = GameObjects::instance().GetObjectByIndex(Scenes::instance().CurrentActorIndex);
	if (!actor)
		actor = GameObjects::GetProtagonistObject();
	return actor ? actor->Position : Common::Point();
}

void ScriptExecutor::DumpWholeScript() {

	// TODO: Probably should not hard code this, with this in place, the
	// variable for saving the old position is superfluous
	SetCurrentSceneScriptAt(0);
	_streamDumpPosition = 0;
	expectedEndLocation = _stream->pos();

	// Disable buffering
	lastOpcodeTriggeredSkip = false;

	// Keep track of the depth of the skipping
	uint16 skipValue = 0; 

	// The loop comprises the first labels in the file
	// l0037_DB73:
	for (;;) {
		// TODO: Just for breaking out at the moment when end conditions fail to work
		if (_stream->eos()) {
			break;
		}
		// TODO: Probably only one of these is necessary
		if (_stream->size() == 0 || _stream->pos() >= _stream->size() - 1) {
			break;
		}

		// Make sure we have read all the bytes we should have read
		// TODO: Think about if we should also check this on exiting the function,
		// maybe we miss some cases like this
		if (_stream->pos() != expectedEndLocation) {
			warning("Macs2::ScriptExecutor::DumpWholeScript resyncing stream from %u to %u",
					(uint32)expectedEndLocation, (uint32)_stream->pos());
			expectedEndLocation = _stream->pos();
		}

		// Read an opcode and length
		byte opcode1 = ReadByte(); // [bp - 1h]
		// TODO: For the sake of easier reading the logs, jumping out if we
		// read a 0 opcode.
		if (opcode1 == 0x00) {
			continue;
		}
		Common::String opcodeInfo;
		if (opcode1 != 0x5) {
			opcodeInfo = IdentifyScriptOpcode(opcode1, 0);
		}
		debug("[%u] - First block opcode: %.2x %s", skipValue, opcode1, opcodeInfo.c_str());
		byte length = ReadByte(); // [bp-2h]
		expectedEndLocation += length + 2;

		if (opcode1 == 0x04) {
			uint16 result1;
			uint16 result2;
			scriptReadValuePair(result1, result2);
		}
		else if (opcode1 == 0x5) {
			// l0037_DC66:
			// [bp-3h]
			uint8 opcode2 = ReadByte();
			opcodeInfo = IdentifyScriptOpcode(opcode1, opcode2);
			debug("[%u] - Second block opcode: %.2x %s", skipValue, opcode2, opcodeInfo.c_str());
			// [bp-7h]
			uint16 v1;
			// [bp-5h]
			uint16 v2;
			scriptReadValuePair(v1, v2);
			// [bp-0Bh]
			uint16 v3;
			// [bp-9h]
			uint16 v4;
			scriptReadValuePair(v3, v4);
		}

		
		if (opcode1 >= 3) {
			if (opcode1 <= 6) {
				skipValue++;
			}
		}
		if (opcode1 == 8) {
			if (skipValue == 1) {
				skipValue--;
			}
		}
		if (opcode1 == 7) {
			skipValue--;
		}


		if (opcode1 == 0x0d) {
			// Show a dialogue option
			uint32 objectID = scriptReadValue32() - 0x400;
			debug("Object ID of speaker: %.4x.\n", objectID);
			scriptReadValue16(); // x
			scriptReadValue16(); // y
			scriptReadValue16(); // side
			uint32 offset = ReadWord();
			uint32 numLines = ReadWord();

			// TODO: We are assuming that we are dumping the scene script, if not,
			// we would have to check for the executing object as well
			Common::Array<Common::String> strings;
			strings = g_engine->DecodeStrings(Scenes::instance().CurrentSceneStrings, offset, numLines);

			for (Common::String &currentLine : strings) {
				debug("String: %s", currentLine.c_str());
			}
		}
		_stream->seek(expectedEndLocation);
		EndBuffering(false);
	}
		_stream->seek(_streamDumpPosition, SEEK_SET);
	}

bool ScriptExecutor::IsRelevantObject(const GameObject *obj) {
	// It can be the protagonist
	if (obj->Index == 1) {
		return true;
	}
	// It can be in the scene
	if (obj->SceneIndex == Scenes::instance().CurrentSceneIndex) {
		return true;
	}

	// It can be in the inventory
	// TODO: Don't hardcode the index of the protagonist
	if (obj->SceneIndex == 1) {
		return true;
	}

	// It can be in the inventory
	// TODO: To check if this only applies if the inventory is open
	// TODO: Also keep storage container inventories in mind
	// TODO: Any others?
	// TODO: How exactly does the ordering work?
	// TODO: Where do we decide when to use which scripts and when not?


	return false;
}

void ScriptExecutor::Step() {
	bool shouldContinue = true;

	while (shouldContinue) {
		switch (_state) {
		case ExecutorState::Idle: {
			// TODO: Check if there is a scheduled run
			return;
		};
			break;
		case ExecutorState::Executing: {
			// Continue execution

			// Check if the currently executing script is at the end
			if (_stream->pos() == _stream->size()) {
				// Handle the next one potentially
				shouldContinue = LoadNextScript();
			} else {
				// Let the current script continue
				ExecutionResult result = ExecuteScript();
				if (result == ExecutionResult::WaitingForCallback) {
					// We need to change our state as well now
					_state = ExecutorState::WaitingForCallback;
					return;
				}
			}
		};
			break;
		case ExecutorState::WaitingForCallback: {
			// TODO: Check if this can even occur i.e. if we even schedule something or if
			// we always call the execute directly

		};
			break;
		}
	}
	// Rewind and reset to the scene script after we are done executing
	executingObjectIndex = Scenes::instance().CurrentSceneIndex;
	SetScript(Scenes::instance().CurrentSceneScript);
	if (_stream && _stream->size() > 0) {
		_stream->seek(0, SEEK_SET);
	}
	scriptExecutionState = ScriptExecutionState::ExecutingSceneScript;
	_state = ExecutorState::Idle;
	g_engine->_scriptExecutor->isRepeatRun = false;
}
	
	
bool ScriptExecutor::LoadNextScript() {
	// TODO: Not sure if for example the scripts of the objects are always called, or if
	// there is code to determine that a script run is finished for good
	// The script switching etc. really needs work

	// TODO: Implement
	// TODO: Consider what effect this one can have on the state
	if (scriptExecutionState == ScriptExecutionState::ExecutingSceneScript) {
		// If we are finished with executing the scene, we need to go over all relevant objects
		// The code below will increment to 1 to start at the protagonist
		executingObjectIndex = 0;
		scriptExecutionState = ScriptExecutionState::ExecutingOtherScripts;
	} 

	// We always try to advance to the next object's script until we reach the end
	// of the objects list
	GameObject *candidateObject = nullptr;
	do {
		executingObjectIndex++;
		candidateObject = GameObjects::GetObjectByIndex(executingObjectIndex);

		// TODO: Check if this is a valid option
		if (candidateObject && IsRelevantObject(candidateObject)) {
			if (candidateObject->Script.size() != 0) {
				_stream = candidateObject->GetScriptStream();
				_executingScriptObjectID = candidateObject->Index;
				debug("----- Switching execution to script for object: %.4x", candidateObject->Index);
				return true;
			}
		}
	} while (candidateObject != nullptr);

	_executingScriptObjectID = 0;

	// We are done executing all relevant objects
	if (IsSceneInitRun) {
		// We need to start again at the scene object
		IsSceneInitRun = false;
		IsRepeatRun = true;
		executingObjectIndex = Scenes::instance().CurrentSceneIndex;
		_stream = Scenes::instance().CurrentSceneScript;
		if (!_stream || _stream->size() == 0) {
			return false;
		}
		_stream->seek(0, SEEK_SET);
		scriptExecutionState = ScriptExecutionState::ExecutingSceneScript;
		debug("----- Switching execution to script for scene: %.4x", executingObjectIndex);
		return true;
	}

	if (IsRepeatRun) {
		// We are done
		IsRepeatRun = false;
		return false;
	}

	return false;
};


byte Script::ScriptExecutor::ReadByte() {
	const int64 pos = _stream->pos();
	const byte result = _stream->readByte();
	//if (isSkipping) {
		// TODO: This had the output channel active, to consider if I want to handle this separately
		//debugC(DEBUG_SV,"Script read (byte): %.2x at location %.4x", result, pos);
	//} else {
		debug("Script read (byte): %.2x at location %.4x", result, (uint32)pos);
	//}
	return result;
}

uint16 Script::ScriptExecutor::ReadWord() {
	const int64 pos = _stream->pos();
	const uint16 result = _stream->readUint16LE();
	debug("Script read (word): %.4x at location %.4x", result, (uint32)pos);
	return result;
}
	
/*  void Script::ScriptExecutor::ExecuteScript(Common::MemoryReadStream *stream) {
	_stream = stream;

	// TODO: Add all labels
	// TODO: Add code that is not yet implemented

	// Implements roughly 01E7:DB56 and friends
	// TODO: Change to a proper end condition
	// TODO: Do some bookkeeping on the pointers into the script
	for (;;) {

		// l0037_DB89:

		// Read an opcode (would be 0037:9F07) - [bp-1h]
		byte opcode1 = ReadByte();

		// Read another value - TODO: Not sure yet what this does - [bp-2h]
		byte length = ReadByte();

		// TODO: Handle other opcodes above
		if (opcode1 == 0x01) {
			ReadByte();
			ReadWord();
			uint16 result1;
			uint16 result2;

			scriptReadValuePair(result1, result2);
			// TODO: Implement properly - this looks like some kind of bookkeeping since it doesn't determine if we continue or not?

			call	far 0037h:9F07h
			call	far 0037h : 9F23h
			mov[bp - 11h], ax
			call	far 0037h : 9F4Dh
			mov	cx, ax
			mov	bx, dx
			mov	ax, [bp - 11h]
			shl	ax, 2h
			les	di, [06C6h]
			add	di, ax
			mov	es : [di - 4h] , cx
			mov	es : [di - 2h] , bx
			// This jumps ahead to the end of the loop
			jmp	0E3BAh

		} else if (opcode1 == 0x02 || opcode1 == 0x03) {
			// TODO: Implement
			// ScriptNoEntry
		} else if (opcode1 == 0x04) {
			// l0037_DC44:
			uint16 v1;
			uint16 v2;
			scriptReadValuePair(v1, v2);
			if ((v1 | v2) == 0) {
				// scriptSkipBlock();
			} else {
				// TODO: Implement
				// ScriptNoEntry
			}
		} else if (opcode1 == 0x05) {
			// TODO: Implement this second opcode fetching:
			// [bp-3h]
			byte opcode2 = ReadByte();

			// [bp-7h]
			uint16 v1;
			// [bp-5h]
			uint16 v2;
			scriptReadValuePair(v1, v2);
			// [bp-0Bh]
			uint16 v3;
			// [bp-9h]
			uint16 v4;
			scriptReadValuePair(v3, v4);
			// TODO: Not yet implemented:
			// mov	byte ptr [bp-12h],0h

			bool bp12 = false; // [bp-12h] - TODO: Better name

			if (opcode2 == 0x01) {
				// l0037_DC8F:;
				// TODO C�ntinue here
				if (v2 == v4 && v1 == v3) {
					bp12 = true;
				}
			} else {
				// ScriptNoEntry
			}

			// TODO: Check if we can reach this label also from the other opcodes
			// l0037_DD2E:
			if (!bp12) {
				// Skip ahead
				// scriptSkipBlock(stream);
			}

		}
		// This is where handling of the opcodes > 6 continues
		// TODO: Does it really? To check if I got this right
		// l0037_DD3C
		else if (opcode1 == 0x06) {
			// ScriptNoEntry
		} else if (opcode1 == 0x07) {
			// ScriptNoEntry
		} else if (opcode1 == 0x10) {
			// TODO: Confirm that this code is being hit
			//  l0037_DE6E:
			//	cmp al, 10h jnz 0DE7Ah

			//		l0037_DE72 : call far 0037h : 0B843h jmp 0E3BAh
		} else if (opcode1 == 0x0a) {
			// TODO: Push 0
			// ScriptPrintString(stream);
		} else {
			// ScriptNoEntry break;
		}

		// TODO: Handle other opcodes below
	}
	*/


ExecutionResult Script::ScriptExecutor::ExecuteScript() {
	debug("----- Scripting function entered - scene: %.2x 1014: %.2x 1012: %.2x", Scenes::instance().CurrentSceneIndex, IsSceneInitRun, IsRepeatRun);
	isRunningScript = true;
	// TODO: Check if we can somehow interrupt something that we are waiting on,
	// thereby ending the scripte early
	isAwaitingCallback = false;

	// TODO: Hardcoded to test the box closing
	_engine->_backgroundAnimations[1].FrameIndex = 1;

	requestCallback = false;
	// Not yet implemented - seems to signal that the script is empty?
	/*
	l0037_DB6A:
		mov	word ptr [0F8Ah],1h
		jmp	0E3E5h
	*/

	// Not yet implemented
	/*
	l0037_DB7F:
		;; Check for equality of [1028h] with 0
		;; TODO: Not yet encountered
		;; Note: During a "multi-phase action", like moving to the box and then opening it, [1028h] seems
		;; to remain 0, so not sure when it would be set to something else
		cmp	word ptr [1028h],0h
		jz	0DB89h

	l0037_DB86:
		jmp	0E3BDh
	*/

	// l0037_DB89:

		// We use this to keep track of cases where we did not read all information as we should have
		expectedEndLocation = _stream->pos();
		// The loop comprises the first labels in the file
		// l0037_DB73:
		for (;;) {
		// TODO: Just for breaking out at the moment when end conditions fail to work
		if (_stream->eos()) {
			break;
		}
		// TODO: Probably only one of these is necessary
		if (_stream->size() == 0 || _stream->pos() >= _stream->size() - 1) {
			break;
		}

		// Make sure we have read all the bytes we should have read
		// TODO: Think about if we should also check this on exiting the function,
		// maybe we miss some cases like this
		if (_stream->pos() != expectedEndLocation) {
			warning("Macs2::ScriptExecutor::ExecuteScript resyncing stream from %u to %u",
					(uint32)expectedEndLocation, (uint32)_stream->pos());
			expectedEndLocation = _stream->pos();
		}

		// Read an opcode and length
		byte opcode1 = ReadByte(); // [bp - 1h]
		// TODO: For the sake of easier reading the logs, jumping out if we
		// read a 0 opcode.
		if (opcode1 == 0x00) {
			// Account for the missing byte from reading opcode 0
			expectedEndLocation++;
			continue;
		}
		Common::String opcodeInfo;
		if (opcode1 != 0x5) {
			opcodeInfo = IdentifyScriptOpcode(opcode1, 0);
		}	
		debug("- First block opcode: %.2x %s", opcode1, opcodeInfo.c_str());
		byte length = ReadByte();  // [bp-2h]
		expectedEndLocation += length + 2;

		// TODO: Check if a switch would do it
		if (opcode1 == 0x01) {
			// l0037_DBA0:

			// This writes to a script variable
			ReadByte();
			uint16 variableIndex = ReadWord();
			ScriptVariable var;
			scriptReadValuePair(var.a, var.b);
			_variables[variableIndex] = var;
		} // l0037_DBCD:
		else if (opcode1 == 0x02) {
			// TODO: No idea what this byte achieves
			ReadByte();
			uint16 variableIndex = ReadWord();
			// We skip the left shift and just read the first value directly
			uint16 throwaway;
			uint16 value1;
			scriptReadValuePair(throwaway, value1);
			uint16 value2;
			uint16 value3;
			scriptReadValuePair(value2, value3);
			value2 |= value1;
			value3 |= 0x00;
			SetVariableValue(variableIndex, value2, value3);
		} // l0037_DC21:
		else if (opcode1 == 0x03) {
			uint16 res1;
			uint16 res2;
			scriptReadValuePair(res1, res2);
			expectedEndLocation = _stream->pos();
			if (res1 | res2) {
				scriptSkipBlock();
			}
			expectedEndLocation = _stream->pos();
			/*
			l0037_DC25:
		call	far 0037h:9F4Dh
		mov	[bp-7h],ax
		mov	[bp-5h],dx
		mov	ax,[bp-7h]
		or	ax,[bp-5h]
		jz	0DC3Dh

	l0037_DC38:
		call	far 0037h:0A3D2h

	l0037_DC3D:
		jmp	0E3BAh

			*/
		} // l0037_DC40:
		else if (opcode1 == 0x04) {
			uint16 result1;
			uint16 result2;
			scriptReadValuePair(result1, result2);
			expectedEndLocation = _stream->pos();
			// If any bit is set in the result, we skip, otherwise we fall through and continue the loop
			if ((result1 | result2) == 0) {
				scriptSkipBlock();
			}
			expectedEndLocation = _stream->pos();

			/*
			l0037_DC44:
		;; Handling opcode1 == 04h
		call	far 0037h:9F4Dh
		
		mov	ax,[bp-7h]
		or	ax,[bp-5h]
		;; We compare the two results from 9F4D with an or
		;; If they end up at res1 OR res2 == 0, we skip ahead, otherwise we go into the loop anew
		jnz	0DC5Ch

	l0037_DC57:
		call	far 0037h:0A3D2h

	l0037_DC5C:
		jmp	0E3BAh
			*/
		} else if (opcode1 == 0x5) {
			// Comparison opcode from executeOpcodes (1008:db56).
			// Reads a comparison sub-opcode, two 32-bit values (v1:v2 and v3:v4),
			// and skips the following block if the condition is NOT met.
			// Values are treated as signed 32-bit (v2:v1 = high:low).
			uint8 opcode2 = ReadByte();
			opcodeInfo = IdentifyScriptOpcode(opcode1, opcode2);
			debug("- Second block opcode: %.2x %s", opcode2, opcodeInfo.c_str());
			uint16 v1; // low word of first value
			uint16 v2; // high word of first value
			scriptReadValuePair(v1, v2);
			uint16 v3; // low word of second value
			uint16 v4; // high word of second value
			scriptReadValuePair(v3, v4);

			// conditionMet = true means we execute the block (don't skip)
			bool conditionMet = false;
			int32 val1 = (int32)((uint32)v2 << 16 | (uint32)v1);
			int32 val2 = (int32)((uint32)v4 << 16 | (uint32)v3);

			if (opcode2 == 0x01) {
				// Equal
				conditionMet = (val1 == val2);
			} else if (opcode2 == 0x02) {
				// Not equal
				conditionMet = (val1 != val2);
			} else if (opcode2 == 0x03) {
				// Less than (signed 32-bit)
				conditionMet = (val1 < val2);
			} else if (opcode2 == 0x04) {
				// Greater than (signed 32-bit)
				conditionMet = (val1 > val2);
			} else if (opcode2 == 0x05) {
				// Less than or equal (signed 32-bit)
				conditionMet = (val1 <= val2);
			} else if (opcode2 == 0x06) {
				// Greater than or equal (signed 32-bit)
				conditionMet = (val1 >= val2);
			} else {
				ScriptUnimplementedOpcode_Main(opcode2);
				EndBuffering(lastOpcodeTriggeredSkip);
				break;
			}

			if (!conditionMet) {
				scriptSkipBlock();
			}
		} else if (opcode1 == 0x06) {
			// "Use item on object" comparison from executeOpcodes (1008:db56).
			// Reads sub-opcode (1=match, 2=NOT match), then the interacted pair
			// and two comparison objects. Checks both orderings.
			uint8 subOpcode = ReadByte();
			uint16 interacted1;
			uint16 interacted2;
			scriptReadValuePair(interacted1, interacted2);
			uint16 object1 = scriptReadValue16();
			uint16 object2 = scriptReadValue16();
			// Check both orderings of the pair
			const bool match1 = (interacted1 == object1) && (interacted2 == object2);
			const bool match2 = (interacted1 == object2) && (interacted2 == object1);
			bool matched = match1 || match2;
			// Sub-opcode 2 inverts the result (NOT match)
			if (subOpcode == 0x02) {
				matched = !matched;
			}
			if (!matched) {
				scriptSkipBlock();
			}
		} else if (opcode1 == 0x07) {
			// TODO: Need to figure out what exactly this does
			// It has no specific case handling code in the original
		} else if (opcode1 == 0x08) {
			// This is some kind of skipping as well
			scriptSkipAlternate();
		}
		else if (opcode1 == 0x10) {
			// scriptWalkToPosition (1008:b843). Uses pathfinding like the original:
			// checks direct walkability first, falls back to A* pathfinding.
			uint32 objectID = scriptReadValue32() - 0x400;
			int16 x = (int16)scriptReadValue16();
			int16 y = (int16)scriptReadValue16();

			View1 *currentView = (View1 *)_engine->findView("View1");
			Character *c = currentView ? currentView->GetCharacterByIndex(objectID) : nullptr;
			if (c == nullptr) {
				warning("Ignoring walk-to for missing character %u", objectID);
				continue;
			}

			Common::Point target(x, y);
			Common::Point current = c->GetPosition();

			// Check if direct path is walkable (like isPathWalkable in the original)
			if (c->IsLineSegmentWalkable(current, target)) {
				// Direct path is clear - just lerp straight there
				c->StartLerpTo(target, 1000);
			} else if (c->IsWalkable(target)) {
				// Target is walkable but no direct path - use A* pathfinding
				c->Path.clear();
				c->PathFinalDestination = target;
				if (c->FindPath(target)) {
					c->CurrentPathIndex = -1;
					c->IsFollowingPath = c->TryFollowPath();
				} else {
					// Pathfinding failed - walk directly as fallback
					c->StartLerpTo(target, 1000);
				}
			} else {
				// Target is not walkable - set position directly (no movement)
				// Original: piVar11[0x16]=0, piVar11[0x17]=0, target=current
			}
		} else if (opcode1 == 0x11) {
			// Wait for walk completion from executeOpcodes (1008:db56).
			// Original behavior:
			//   1. Read objectID, validate range and existence
			//   2. Check runtime data exists (field +10/+12 != 0)
			//   3. Check runtime+0x231 (frozen flag) - if set, error 0x1F
			//   4. Set g_wWalkTargetObjectIndex = objectID
			//   5. Hide cursor (save mode, set to Disabled 0x1A)
			//   6. Clear PTR_LOOP_1020_1018
			//   7. Return (gameTick will check walk completion each frame)
			uint32 objectID = scriptReadValue32() - 0x400;
			View1 *currentView = (View1 *)_engine->findView("View1");
			// TODO: Need to be able to address the character objects by ID, now relying
			// on the fact that they were added in a specific order
			Character *c = currentView->GetCharacterByIndex(objectID);
			if (c == nullptr) {
				// TODO: This seems to happen in chapter 3 when leaving from the first
				// interactive screen to the right.
				// For now, just ignoring the command in this case
				// TODO: Not sure if we should be returning or continuing here
				return ExecutionResult::ScriptFinished;
			}
			c->RegisterWaitForMovementFinishedEvent();
			requestCallback = false;
			isAwaitingCallback = true;
			// TODO: Could be special for me with the short timer times, but it can happen
			// that things happen out of order if not ending any timers active
			EndTimer();
			EndBuffering(lastOpcodeTriggeredSkip);
			return ExecutionResult::WaitingForCallback;
		} else if (opcode1 == 0x13) {
			SkipUntil14();
			expectedEndLocation = _stream->pos();
		} else if (opcode1 == 0x14) {
			// If we reach opcode 14 regularly, just discard the payload and continue
			ReadWord();
		}
		else if (opcode1 == 0x0a) {
			// l0037_DDE8:
			ScriptPrintString();
			// TODO: Proper end handling
			EndBuffering(lastOpcodeTriggeredSkip);
			return ExecutionResult::WaitingForCallback;
		} else if (opcode1 == 0x0b) {
			// Move object - scriptMoveObject (1008:aa83).
			// Original behavior:
			//   1. Read objectID, sceneID, X, Y
			//   2. If object was in current scene: remove from render list (sortObjectsByDepth)
			//   3. Set object's SceneIndex, X, Y
			//   4. If new scene is current scene: add to render list (loadSceneObjects)
			//   5. If sceneID > 0x400: object is placed inside another object (parent)
			//      - checks if parent is in current scene for render list update
			//   6. If object has no runtime data (field +10/+12 == 0):
			//      - If it was the UseInventory target: reset cursor to Use (0x15)
			//   7. If object == executing script object: reset script position to 0
			//   8. Call readObjectFromFile() to refresh state
			//   9. Set redraw flag
			uint32 objectID = scriptReadValue32() - 0x400;
			// TODO: Check if these file reads happen every time this is called
			// l0037_AB93:
			uint16 sceneID = scriptReadValue16();
			int16 x = (int16)scriptReadValue16();
			int16 y = (int16)scriptReadValue16();
			// TODO: Now actually place the object
			// TODO: Need to handle 0 scene and moving to non-active scenes
			// TODO: Need to handle negative numbers here

			// TODO: Exception for 0x401 since we assume the protagonist is always in the scene
			// TODO: Even more evidence that we need a refactor!
			if (sceneID > 0x400) {
				// This is the special case of adding a child to an object
				// TODO: This is hardcoded to fit to the special case of the hat,
				// with this addition, the function needs to be refactored to still
				// remain readable
				GameObject* parentObject = GameObjects::instance().Objects[sceneID - 0x400 - 1];
				GameObject *childObject = GameObjects::instance().Objects[objectID - 1];
				childObject->SceneIndex = parentObject->Index;
				// TODO: Clean up
				EndBuffering(lastOpcodeTriggeredSkip);
				continue;
			}

			// TODO: This function really needs refactoring now.
			// Handling the general case of overwriting scene index and position in all cases
			GameObject *childObject = GameObjects::instance().Objects[objectID - 1];
			childObject->SceneIndex = sceneID;
			childObject->Position = Common::Point(x, y);


			View1 *currentView = (View1 *)_engine->findView("View1");
			Character *c = currentView->GetCharacterByIndex(objectID);
			/*  if (c != nullptr) {
				// TODO: Something seems to be wrong with the stick
				// assert(sceneID != Scenes::instance().CurrentSceneIndex);
				int index = currentView->GetCharacterArrayIndex(c);
				currentView->characters.remove_at(index);
				c->GameObject->SceneIndex = sceneID;
				c->GameObject->Position = Common::Point(x, y);
				continue;
			} */
			if (c == nullptr) {
				c = new Character();
				c->GameObject = GameObjects::instance().Objects[objectID - 1];
			}
			// Reset if the character can run the script again
			// TODO: Not sure if this should also be this way
			c->ExecuteScriptOnFinishLerp = false;

			// This doubles as an indication if the character has been created previously
			// and is already in the list
			int index = currentView->GetCharacterArrayIndex(c);

			// TODO: Figure out how to create the list properly
			// TODO: DRY principle
			c->SetPosition(Common::Point(x, y));
			c->GameObject->SceneIndex = sceneID;
			if (sceneID != Scenes::instance().CurrentSceneIndex) {
				// We need to remove the character if it is in the list already
				if (index > -1) {
					currentView->characters.remove_at(index);
				}
			}
			else {
				// We need to add the character unless it's already there
				if (index < 0) {
					currentView->characters.push_back(c);
				}
			}
			// TODO: Not sure if we should handle this earlier
			if (sceneID == 0x401) {
				// This is the character, so put it into his inventory
				currentView->inventoryItems.push_back(GameObjects::instance().Objects[objectID - 1]);
			} else if (sceneID == 0) {
				// Check if it is in the inventory
				int currentIndex = 0;
				int foundIndex = -1;
				for (auto currentItem : currentView->inventoryItems) {
					if (currentItem->Index == objectID) {
						foundIndex = currentIndex;
						break;
					}
					currentIndex++;
				}
				if (foundIndex > 0) {
					currentView->inventoryItems.remove_at(foundIndex);
				}
				assert(!currentView->HasDuplicateCharacters());
			}
		} else if (opcode1 == 0x0c) {
			// Scene change from scriptChangeScene (1008:ad6e).
			// Original behavior:
			//   1. Read sceneID, transitionMode, transitionSpeed
			//   2. Validate: sceneID must be 1..0x200
			//   3. If mode==0 && speed==0 or speed>0x40: error 0x26
			//   4. Save old palette, free scene resources, load new scene
			//   5. Transition:
			//      mode 0: fade from old palette at given speed
			//      mode 1: instant cut (clear screen, set palette)
			//   6. Run init pass (IsSceneInitRun=1) -> runScriptExecutor
			//   7. Draw scene, restore palette
			//   8. Set cursor to Walk (0x16)
			//   9. Set script position to end, executing object to 0x201
			//  10. If script was NOT already executing: run repeat pass
			//      (IsRepeatRun=1) -> runScriptExecutor -> (IsRepeatRun=0)
			//  11. If script WAS executing: error 0x17
			uint32 newSceneID = scriptReadValue32();
			const uint16 transitionMode = scriptReadValue16();
			const uint16 transitionSpeed = scriptReadValue16();
			g_engine->changeScene(newSceneID, false);
			View1 *currentView = (View1 *)_engine->findView("View1");
			if (currentView != nullptr && transitionMode == 0 && transitionSpeed != 0) {
				currentView->startFadingWithSpeed(transitionSpeed);
			}
			// TODO: Confirm that script execution is also stopped always afer this command
			// in the game code
			// TODO: Confirm that these variables are also reset by the game when changing a scene
			// or if there is another mechanism for this
			_interactedObjectID = 0;
			_interactedOtherObjectID = 0;
			requestCallback = false;
			g_engine->ScheduleRun(true);
			isAwaitingCallback = true;
			// TODO: Could be special for me with the short timer times, but it can happen
			// that things happen out of order if not ending any timers active
			EndTimer();
			EndFrameWait();
			EndBuffering(lastOpcodeTriggeredSkip);
			return ExecutionResult::WaitingForCallback;
		} else if (opcode1 == 0x0d) {
			// Show a dialogue option
			uint32 objectID = scriptReadValue32() - 0x400;
			uint16 x = scriptReadValue16();
			uint16 y = scriptReadValue16();
			uint16 side = scriptReadValue16();
			uint32 offset = ReadWord();
			uint32 numLines = ReadWord();

			View1 *currentView = (View1 *)_engine->findView("View1");

			Common::Array<Common::String> strings;
			if (_executingScriptObjectID == 0) {
				strings = g_engine->DecodeStrings(Scenes::instance().CurrentSceneStrings, offset, numLines);
			} else {
				Common::MemoryReadStream *s = GameObjects::ReadGameObjectStrings(_executingScriptObjectID, g_engine->_fileStream);
				strings = g_engine->DecodeStrings(s, offset, numLines);
			}

			debugC(kDebugScript,
				"Opcode 0D dialogue: speaker=%u rawPos=(%u,%u) side=%u textOffset=%u numLines=%u scriptObject=%u text=\"%s\"",
				objectID, x, y, side, offset, numLines, _executingScriptObjectID, joinDebugStrings(strings).c_str());
			
			activeDialogueSpeakerObjectID = objectID;
			currentView->ShowSpeechAct(objectID, strings, Common::Point(x, y), side);
			isAwaitingCallback = true;
			// TODO: Could be special for me with the short timer times, but it can happen
			// that things happen out of order if not ending any timers active
			EndTimer();
			EndFrameWait();
			EndBuffering(lastOpcodeTriggeredSkip);
			return ExecutionResult::WaitingForCallback;
		}
		else
		if (opcode1 == 0x0E) {
			scriptChangeAnimationImpl();
		} else if (opcode1 == 0x0F) {
			// The original interpreter stores a frame countdown that is decremented
			// once per game tick, rather than using a wall-clock timer.
			uint16 duration = scriptReadValue16();
			requestCallback = false;
			StartFrameWait(duration);
			isAwaitingCallback = true;
			EndBuffering(lastOpcodeTriggeredSkip);
			return ExecutionResult::WaitingForCallback;
		}
		else if (opcode1 == 0x12) {
			// scriptSetPathfinding (1008:c6d7). Sets/clears a pathfinding override.
			// Index must be in range 200..0xEF (walkability values).
			// Writes to scene data at index*5 + 0x4EA5 (enable byte) and +0x4EA6 (value).
			uint16 areaID = scriptReadValue16();
			uint16 active = scriptReadValue16();
			uint16 overrideValue = scriptReadValue16();
			if (active) {
				g_engine->SetPathfindingOverride(areaID, overrideValue);
			} else {
				g_engine->RemovePathfindingOverride(areaID);
			}
		} else if (opcode1 == 0x14) {
			// TODO: No idea why we only do this without other side effects or using the
			// read value
			ReadWord();
		} else if (opcode1 == 0x15) {
			// Mark that we are gathering strings for setting up a dialogue choice
			DialogueChoices.clear();

		} else if (opcode1 == 0x16) {
			// Add a dialogue choice
			uint16 index = scriptReadValue16();
			// We don't save the index, instead we make sure that we add them in the right
			// order and use the array to keep track
			// TODO: Removed this assert, during the dialogue in the beginning of chapter
			// 3 (at the fort) an index of 3 came up when only one item had been there before
			// Not sure if the way of handling it still works or reflects the game, needs
			// to be tested
			// assert(index - 1 == DialogueChoices.size());
			uint16 offset = ReadWord();
			uint16 numLines = ReadWord();
			Common::StringArray lines;
			if (_executingScriptObjectID == 0) {
				lines = _engine->DecodeStrings(Scenes::instance().CurrentSceneStrings, offset, numLines);
			} else {
				Common::MemoryReadStream *stringsStream = GameObjects::ReadGameObjectStrings(_executingScriptObjectID, g_engine->_fileStream);
				lines = _engine->DecodeStrings(stringsStream, offset, numLines);
			}
			debugC(kDebugScript,
				"Opcode 16 choice text: index=%u textOffset=%u numLines=%u scriptObject=%u text=\"%s\"",
				index, offset, numLines, _executingScriptObjectID, joinDebugStrings(lines).c_str());
			DialogueChoices.push_back(lines);
		} else if (opcode1 == 0x17) {
			// Finish the dialogue choice

			View1 *currentView = (View1 *)_engine->findView("View1");
			uint32 x = scriptReadValue32();
			uint32 y = scriptReadValue32();
			uint16 side = scriptReadValue16();
			const uint16 speakerObjectID = activeDialogueSpeakerObjectID != 0 ? activeDialogueSpeakerObjectID : _executingScriptObjectID;
			debugC(kDebugScript,
				"Opcode 17 choice box: speaker=%u rawPos=(%u,%u) side=%u choiceCount=%u",
				speakerObjectID, x, y, side, DialogueChoices.size());
			currentView->ShowDialogueChoice(speakerObjectID, DialogueChoices, Common::Point(x, y), side);
			requestCallback = false;
			// TODO: Could be special for me with the short timer times, but it can happen
			// that things happen out of order if not ending any timers active
			EndTimer();
			EndBuffering(lastOpcodeTriggeredSkip);
			return ExecutionResult::WaitingForCallback;
		} else if (opcode1 == 0x18) {
			// Set the stream to the end and let the calling code figure out that we are done
			// for this run
			_stream->seek(_stream->size(), SEEK_SET);
			EndBuffering(lastOpcodeTriggeredSkip);
			return ExecutionResult::ScriptFinished;
		} else if (opcode1 == 0x19) {
			// Walk to and pick up an object
			uint32 actorIndex = scriptReadValue32() - 0x400;
			uint32 objectIndex = scriptReadValue32() -0x400;

			View1 *currentView = (View1 *)_engine->findView("View1");
			Character *actor = currentView->GetCharacterByIndex(actorIndex);
			GameObject *targetObject = GameObjects::GetObjectByIndex(objectIndex);
			if (pickupInProgress) {
				EndTimer();
				EndBuffering(lastOpcodeTriggeredSkip);
				return ExecutionResult::WaitingForCallback;
			}
			if (actor == nullptr || targetObject == nullptr) {
				warning("Invalid pickup request for actor %u target %u", actorIndex, objectIndex);
				continue;
			}
			if (actorIndex == objectIndex || targetObject->SceneIndex == actor->GameObject->Index) {
				warning("Ignoring invalid pickup request for actor %u target %u", actorIndex, objectIndex);
				continue;
			}
			if (targetObject->SceneIndex != actor->GameObject->SceneIndex) {
				warning("Ignoring pickup across scenes for actor %u target %u", actorIndex, objectIndex);
				continue;
			}
			pickupInProgress = true;
			pickupActorObjectID = actorIndex;
			pickupTargetObjectID = objectIndex;
			savedPickupMouseMode = _mouseMode == MouseMode::UseInventory ? MouseMode::Use : _mouseMode;
			currentView->activeInventoryItem = nullptr;
			_engine->SetCursorMode(savedPickupMouseMode);
			currentView->UpdateCursor();
			actor->StartPickup(targetObject);
			requestCallback = false;
			isAwaitingCallback = true;
			// TODO: Could be special for me with the short timer times, but it can happen
			// that things happen out of order if not ending any timers active
			EndTimer();
			EndBuffering(lastOpcodeTriggeredSkip);
			return ExecutionResult::WaitingForCallback;
		} else if (opcode1 == 0x1a) {
			int32 objectID = (int32)scriptReadValue32() - 0x400;
			uint16 value217 = scriptReadValue16();
			uint16 value219 = scriptReadValue16();
			if (objectID < 1 || objectID > 0x200) {
				warning("Ignoring object runtime setup for invalid object %d", objectID);
				continue;
			}

			GameObject *object = GameObjects::GetObjectByIndex((uint16)objectID);
			if (object == nullptr) {
				warning("Ignoring object runtime setup for missing object %d", objectID);
				continue;
			}

			object->RuntimeValue217 = value217;
			object->RuntimeValue219 = value219;
		} else if (opcode1 == 0x1b) {
			int32 objectID = (int32)scriptReadValue32() - 0x400;
			uint16 slotID = scriptReadValue16();
			uint16 value = scriptReadValue16();
			if (objectID < 1 || objectID > 0x200) {
				warning("Ignoring object slot setup for invalid object %d", objectID);
				continue;
			}

			GameObject *object = GameObjects::GetObjectByIndex((uint16)objectID);
			if (object == nullptr) {
				warning("Ignoring object slot setup for missing object %d", objectID);
				continue;
			}

			if (slotID < 1 || slotID > ARRAYSIZE(object->RuntimeSlotValues)) {
				warning("Ignoring object slot setup for invalid slot %u on object %d", slotID, objectID);
				continue;
			}

			object->RuntimeSlotValues[slotID - 1] = value;
		} else if (opcode1 == 0x1c) {
			// Sets g_wScriptSkippable [102Ah] = 1
			scriptSkippable = true;
		} else if (opcode1 == 0x1d) {
			// Sets g_wScriptSkippable [102Ah] = 0
			scriptSkippable = false;
		} else if (opcode1 == 0x1e) {
			// This is playing an animation
			// fn0037_BD58 proc
			// TODO: Skipped for now until the animation system is more in the focus
			uint32 objectID = scriptReadValue32() - 0x400;
			uint32 animationID = scriptReadValue16();
			uint32 offset = scriptReadValue16();
			GameObject *gameObject = GameObjects::GetObjectByIndex(objectID);
			if (animationID == 0x15) {
				gameObject->useOverloadAnimation = true;
				BackgroundAnimationBlob::advanceAnimFrame(gameObject->overloadAnimation,
												  true, offset + 0x64);
				
			} else {
				BackgroundAnimationBlob::advanceAnimFrame(gameObject->Blobs[animationID - 1],
												  true, offset + 0x64);
			}
			
		} else if (opcode1 == 0x1f) {
			uint32 objectID = scriptReadValue32() - 0x400;
			uint32 x = scriptReadValue32();
			uint32 y = scriptReadValue32();
			GameObject *object = GameObjects::GetObjectByIndex(objectID);
			pathWalkableResult = false;
			if (object == nullptr) {
				warning("Ignoring pathfinding test for invalid object %u", objectID);
			} else {
				pathWalkableResult = IsPathWalkable(object->Position, Common::Point(x, y));
			}
		} else if (opcode1 == 0x20) {
			// scriptSetYOffset (1008:c047). Sets object field +8 (vertical offset)
			// AND mirrors it into runtime field +0x21D (motion target).
			int32 objectID = (int32)scriptReadValue32() - 0x400;
			uint16 offset = scriptReadValue16();
			if (objectID < 1 || objectID > 0x200) {
				warning("Ignoring vertical offset set for invalid object %d", objectID);
				continue;
			}

			GameObject *object = GameObjects::GetObjectByIndex((uint16)objectID);
			if (object == nullptr) {
				warning("Ignoring vertical offset set for missing object %d", objectID);
				continue;
			}

			object->Unknown = offset;
			// Original also writes to runtime +0x21D (motion target vertical offset)
			View1 *currentView = (View1 *)_engine->findView("View1");
			if (currentView != nullptr) {
				Character *c = currentView->GetCharacterByIndex((uint16)objectID);
				if (c != nullptr) {
					c->motionTargetVerticalOffset = offset;
				}
			}
		} else if (opcode1 == 0x21) {
			int32 objectID = (int32)scriptReadValue32() - 0x400;
			uint16 targetVerticalOffset = scriptReadValue16();
			uint16 verticalOffsetDelta = scriptReadValue16();
			uint16 motionDistance = scriptReadValue16();
			if (objectID < 1 || objectID > 0x200) {
				warning("Ignoring motion setup for invalid object %d", objectID);
				continue;
			}

			View1 *currentView = (View1 *)_engine->findView("View1");
			Character *character = currentView ? currentView->GetCharacterByIndex((uint16)objectID) : nullptr;
			GameObject *object = GameObjects::GetObjectByIndex((uint16)objectID);
			if (object == nullptr || character == nullptr) {
				warning("Ignoring motion setup for missing character object %d", objectID);
				continue;
			}

			character->motionStartVerticalOffset = object->Unknown;
			character->motionTargetVerticalOffset = targetVerticalOffset;
			character->motionVerticalOffsetDelta = verticalOffsetDelta;
			character->motionDistanceUnits = motionDistance;
			character->motionProgress = 0;
			character->hasMotionVerticalOffset = motionDistance != 0 || targetVerticalOffset != object->Unknown;
		} else if (opcode1 == 0x22) {
			int32 objectID = (int32)scriptReadValue32() - 0x400;
			uint16 animIndex = scriptReadValue16();
			if (objectID < 1 || objectID > 0x200) {
				warning("Ignoring orientation set for invalid object %d", objectID);
				continue;
			}

			GameObject *object = GameObjects::GetObjectByIndex((uint16)objectID);
			if (object == nullptr) {
				warning("Ignoring orientation set for missing object %d", objectID);
				continue;
			}

			if (animIndex < 9 || animIndex > 0x10) {
				warning("Ignoring out-of-range orientation %u for object %d", animIndex, objectID);
				continue;
			}

			object->Orientation = animIndex;
		} else if (opcode1 == 0x23) {
			int32 objectID = (int32)scriptReadValue32() - 0x400;
			uint32 x = scriptReadValue32();
			uint32 y = scriptReadValue32();
			uint16 targetVerticalOffset = scriptReadValue16();
			if (objectID < 1 || objectID > 0x200) {
				warning("Ignoring move-to-position for invalid object %d", objectID);
				continue;
			}

			View1 *currentView = (View1 *)_engine->findView("View1");
			GameObject *object = GameObjects::GetObjectByIndex((uint16)objectID);
			Character *c = currentView ? currentView->GetCharacterByIndex((uint16)objectID) : nullptr;
			if (object == nullptr || c == nullptr) {
				warning("Ignoring move-to-position for missing character object %d", objectID);
				continue;
			}

			const Common::Point target(x, y);
			if (!IsPathWalkable(object->Position, target) && _engine->getWalkabilityAt(target) < 0xC8) {
				warning("Ignoring move-to-position for blocked target (%u,%u) on object %d", x, y, objectID);
				continue;
			}

			c->IsFollowingPath = false;
			c->motionStartVerticalOffset = object->Unknown;
			c->motionTargetVerticalOffset = targetVerticalOffset;
			c->motionVerticalOffsetDelta = ABS<int32>((int32)object->Unknown - (int32)targetVerticalOffset);
			c->motionDistanceUnits = ABS<int32>((int32)x - object->Position.x) + ABS<int32>((int32)y - object->Position.y);
			c->motionProgress = 0;
			c->hasMotionVerticalOffset = true;
			c->StartLerpTo(Common::Point(x, y), 2 * 1000);
			isAwaitingCallback = true;
		} else if (opcode1 == 0x24) {
			// Adds two values read and saves them to a script variable
			// ;; fn0037_C7E6: 0037:C7E6
			uint32 a = scriptReadValue32();
			uint32 b = scriptReadValue32();

			uint32 result = a + b;
			// Go back to the first value being pointed to
			// In this case, 9F4D and A334 can use the same data, since the
			// index of the script variable will be in the word at offset 1
			_stream->seek(-6, SEEK_CUR);
			scriptSaveVariable(result);
			// Skip forward across the second 9F4D read's data
			_stream->seek(3, SEEK_CUR);
		
		} else if (opcode1 == 0x25) {
			// TODO: No visual difference, so only implementing mocked reads here
			// TODO: There is the weird "rewind" in the log here, to be investigated separately
			uint16 throwaway1;
			uint16 throwaway2;
			scriptReadValuePair(throwaway1, throwaway2);
			scriptReadValuePair(throwaway1, throwaway2);
		}
		else if (opcode1 == 0x26) {
			// This one loads a special animation set
			uint32 id = scriptReadValue32() - 0x400;
			// No idea yet what this one does
			scriptReadValue_Placeholder();
			uint8 animationID = ReadByte();
			Common::Array<uint8> blob = Scenes::instance().ReadSpecialAnimBlob(animationID, g_engine->_fileStream);
			GameObject *object = GameObjects::GetObjectByIndex(id);
			object->overloadAnimation = blob;
			object->overloadAnimationMirrored = false;
			object->useOverloadAnimation = false;
			// object->Blobs.push_back(blob);
			// object->Blobs[animationID - 1] = blob;
			//GameObjects::GetObjectByIndex(id)->testOverloadAnimation = object->Blobs.size() - 1;
		} else if (opcode1 == 0x27) {
			// scriptSetDirection (1008:c858). Writes to runtime field +0x22D
			// which is the "max animation frame" limit used by walkAlongPath
			// for animation frame cycling. This is NOT the same as Orientation (+6).
			uint16 characterID = scriptReadValue16() - 0x400;
			uint16 value = scriptReadValue16();
			if (characterID < 1 || characterID > 0x200) {
				warning("Ignoring set direction for invalid object %u", characterID);
				continue;
			}
			GameObject *object = GameObjects::GetObjectByIndex(characterID);
			if (object == nullptr) {
				warning("Ignoring set direction for missing object %u", characterID);
				continue;
			}

			View1 *currentView = (View1 *)_engine->findView("View1");
			Character *c = currentView ? currentView->GetCharacterByIndex(characterID) : nullptr;
			if (c != nullptr) {
				// Runtime field +0x22D controls animation frame limit during walking
				// Value 0x7FFF means "no limit" (default from loadSceneObjects)
				// TODO: Map this to a proper Character field once the animation
				// system is fully understood
			}
			// For now, also update orientation for visual feedback
			object->Orientation = value;
		} else if (opcode1 == 0x28) {
			// TODO: Figure out what this does - it seems to again write data to a
			// hotspot's data
			scriptStopAnimationImpl();
		} else if (opcode1 == 0x29) {
			uint32 objectID = scriptReadValue32();
			objectID -= 0x400;
			View1 *currentView = (View1 *)_engine->findView("View1");
			GameObject *inventorySource = GameObjects::GetObjectByIndex(objectID);
			if (inventorySource == nullptr) {
				warning("Invalid inventory source object %u", objectID);
				continue;
			}
			savedExternalInventoryMouseMode = _mouseMode == MouseMode::UseInventory ? MouseMode::Use : _mouseMode;
			hasPendingExternalInventoryResume = true;
			externalInventorySourceObjectID = objectID;
			secondaryInventoryLocation = _stream->pos();
			currentView->OpenInventory(inventorySource);
			return ExecutionResult::WaitingForCallback;
		} else if (opcode1 == 0x2A) {
			uint32 objectID = scriptReadValue32() - 0x400;
			uint16 slotID = scriptReadValue16();
			const bool decodeBlob = scriptReadValue16() != 0;
			uint8 arrayIndex = ReadByte();

			g_engine->loadAnimationFromSceneData(objectID, slotID, arrayIndex, decodeBlob);
		} else if (opcode1 == 0x2B) {
			const uint16 objectID = scriptReadValue16() - 0x400;
			GameObject *object = GameObjects::GetObjectByIndex(objectID);
			if (object == nullptr) {
				warning("Ignoring object refresh for invalid object %u", objectID);
				continue;
			}
			if (object->Blobs.empty()) {
				warning("Ignoring object refresh for unloaded object %u", objectID);
				continue;
			}
			View1 *currentView = (View1 *)_engine->findView("View1");
			if (currentView == nullptr) {
				continue;
			}

			Character *character = currentView->GetCharacterByIndex(objectID);
			const int currentIndex = currentView->GetCharacterArrayIndex(character);
			if (object->SceneIndex != Scenes::instance().CurrentSceneIndex) {
				if (currentIndex >= 0) {
					currentView->characters.remove_at(currentIndex);
				}
				continue;
			}

			if (character == nullptr) {
				character = new Character();
				character->GameObject = object;
			} else if (currentIndex >= 0) {
				currentView->characters.remove_at(currentIndex);
			}

			currentView->characters.push_back(character);
		} else if (opcode1 == 0x2C) {
			uint16 objectID = scriptReadValue16() - 0x400;
			uint16 parentID = scriptReadValue16();
			const GameObject *object = GameObjects::GetObjectByIndex(objectID);
			if (object == nullptr) {
				warning("Ignoring inventory check for invalid object %u", objectID);
				continue;
			}
			inventoryCheckResult = object->SceneIndex == parentID;
		} else if (opcode1 == 0x2D) {
			const uint16 objectID = scriptReadValue16() - 0x400;
			const bool enabled = scriptReadValue16() != 0;
			GameObject *object = GameObjects::GetObjectByIndex(objectID);
			if (object == nullptr) {
				warning("Ignoring object runtime flag for invalid object %u", objectID);
				continue;
			}
			object->RuntimeFlag22F = enabled;
		} else if (opcode1 == 0x2E) {
			// scriptTestSceneAnimFrame: Tests if a scene's special animation blob's
			// current frame index (via getAnimBlobOffset/source key) falls within
			// [minFrame, maxFrame]. Result stored in animBlobRangeTestResult for helper FF29.
			uint32 sceneAnimIndex = scriptReadValue32();
			uint32 minFrame = scriptReadValue32();
			uint32 maxFrame = scriptReadValue32();
			animBlobRangeTestResult = false;
			if (sceneAnimIndex == 0 || sceneAnimIndex > Scenes::instance().CurrentSceneSpecialAnimOffsets.size()) {
				warning("Ignoring scene animation range test for invalid index %u", sceneAnimIndex);
			} else {
				const uint16 blobSourceKey = static_cast<uint16>(Scenes::instance().CurrentSceneSpecialAnimOffsets[sceneAnimIndex - 1] >> 16);
				animBlobRangeTestResult = blobSourceKey >= minFrame && blobSourceKey <= maxFrame;
			}
		} else if (opcode1 == 0x2F) {
			// scriptTestObjectAnimFrame: Tests if an object's animation blob's
			// current frame index (via getAnimBlobOffset/source key) falls within
			// [minFrame, maxFrame]. Result stored in animBlobRangeTestResult for helper FF29.
			uint32 objectID = scriptReadValue32() - 0x400;
			uint16 slotID = scriptReadValue16();
			uint16 minFrame = scriptReadValue16();
			uint16 maxFrame = scriptReadValue16();
			animBlobRangeTestResult = false;
			GameObject *object = GameObjects::GetObjectByIndex(objectID);
			if (object == nullptr) {
				warning("Ignoring object animation range test for invalid object %u", objectID);
				continue;
			}

			uint16 blobSourceKey = 0;
			bool hasBlob = false;
			if (slotID == 0x15) {
				hasBlob = !object->overloadAnimation.empty();
				blobSourceKey = object->overloadAnimationSourceKey;
			} else if (slotID >= 1 && slotID <= object->Blobs.size()) {
				hasBlob = !object->Blobs[slotID - 1].empty();
				if ((uint)(slotID - 1) < object->BlobSourceKeys.size())
					blobSourceKey = object->BlobSourceKeys[slotID - 1];
			} else {
				warning("Ignoring object animation range test for invalid slot %u on object %u", slotID, objectID);
				continue;
			}

			if (hasBlob) {
				animBlobRangeTestResult = blobSourceKey >= minFrame && blobSourceKey <= maxFrame;
			}
		} else if (opcode1 == 0x30) {
			// TODO: This calls the same function as the print string but adds a param
			// which changes behaviour in the function
			// TODO: Implement the change by the flag
			ScriptPrintString(true);
			// TODO: Proper end handling
			EndBuffering(lastOpcodeTriggeredSkip);
			return ExecutionResult::WaitingForCallback;
		} else if (opcode1 == 0x31) {
			uint16 volume = scriptReadValue16();
			g_engine->getAdlib()->SetVolume(volume);
		} else if (opcode1 == 0x32) {
			uint16 objectID = scriptReadValue16() - 0x0400;
			const uint16 clickable = scriptReadValue16();
			GameObject *object = GameObjects::GetObjectByIndex(objectID);
			if (object == nullptr) {
				warning("Ignoring clickable toggle for invalid object %u", objectID);
				continue;
			}
			object->IsClickable = clickable != 0;
		} else if (opcode1 == 0x33) {
			uint16 objectID = scriptReadValue16() - 0x0400;
			const uint16 visible = scriptReadValue16();
			GameObject *object = GameObjects::GetObjectByIndex(objectID);
			if (object == nullptr) {
				warning("Ignoring visibility toggle for invalid object %u", objectID);
				continue;
			}
			object->IsVisible = visible != 0;
		} else if (opcode1 == 0x34) {
			// Sets an entry in the [5BD1] list for hotspot lookup
			const uint16 v1 = scriptReadValue16() - 0x800;
			const uint16 v2 = scriptReadValue16() - 0x800;

			if (v1 < 0x1 || v1 > 0x10 || v2 < 0x1 || v2 > 0x10) {
				warning("Ignoring hotspot override %.4x -> %.4x outside valid range", v1 + 0x800, v2 + 0x800);
				continue;
			}
			if (v1 == v2) {
				// l0037_CE92:
				g_engine->HotspotOverrides[v1] = 0xFFFF;
			} else {
				// l0037_CEA5:
				g_engine->HotspotOverrides[v1] = v2;
			}
		} else if (opcode1 == 0x35) {
			uint16 objectID = scriptReadValue16() - 0x0400;
			uint16 otherObjectID = scriptReadValue16() - 0x0400;
			const uint16 value1 = scriptReadValue16();
			const uint16 value2 = scriptReadValue16();
			const uint16 value3 = scriptReadValue16();
			GameObject *object = GameObjects::GetObjectByIndex(objectID);
			GameObject *otherObject = GameObjects::GetObjectByIndex(otherObjectID);
			if (object == nullptr || otherObject == nullptr) {
				warning("Ignoring bounds attachment for invalid objects %u -> %u", objectID, otherObjectID);
				continue;
			}

			if (objectID == otherObjectID) {
				object->HasBoundsAttachment = false;
				object->BoundsAttachmentObjectID = 0;
				object->BoundsAttachmentValue1 = 0;
				object->BoundsAttachmentValue2 = 0;
				object->BoundsAttachmentValue3 = 0;
			} else {
				object->HasBoundsAttachment = true;
				object->BoundsAttachmentObjectID = otherObjectID;
				object->BoundsAttachmentValue1 = value1;
				object->BoundsAttachmentValue2 = value2;
				object->BoundsAttachmentValue3 = value3;
			}
		} else if (opcode1 == 0x36) {
			// scriptDismissPanel (1008:d6dd). Restores background if a UI panel
			// was pending, clears panel state, redraws scene, clears timer flag.
			View1 *currentView = (View1 *)_engine->findView("View1");
			if (currentView != nullptr) {
				if (currentView->_isShowingStringBox || currentView->_isShowingDialogueChoice) {
					currentView->_continueScriptAfterUI = false;
					currentView->clearStringBox(false);
				}

				if (currentView->_isShowingInventory) {
					hasPendingExternalInventoryResume = false;
					externalInventorySourceObjectID = 0;
					currentView->CloseInventory();
				}

				if (currentView->isShowingMainMenu) {
					currentView->isShowingMainMenu = false;
					currentView->redraw();
				}
			}
		} else if (opcode1 == 0x37) {
			// scriptResetToSceneScript (1008:ad3e). Resets script execution
			// context back to the current scene script at position 0.
			_executingScriptObjectID = 0;
			executingObjectIndex = Scenes::instance().CurrentSceneIndex;
			scriptExecutionState = ScriptExecutionState::ExecutingSceneScript;
			activeDialogueSpeakerObjectID = 0;
			SetCurrentSceneScriptAt(0);
		} else if (opcode1 == 0x038) {
			// scriptLoadOverlayFont (1008:d749). Loads a font resource for
			// overlay text and sets overlayTextStageActive = true.
			ReadByte();
			overlayTextStageActive = true;
		} else if (opcode1 == 0x039) {
			// scriptEndOverlayText (1008:d80f). Clears the overlay text stage.
			if (overlayTextStageActive) {
				overlayTextStageActive = false;
			}
		} else if (opcode1 == 0x03A) {
			View1 *currentView = (View1 *)_engine->findView("View1");
			if (currentView == nullptr) {
				warning("Ignoring overlay text entry without an active View1");
				scriptReadValue16();
				scriptReadValue16();
				scriptReadValue16();
				ReadWord();
				ReadWord();
				continue;
			}

			const uint16 x = scriptReadValue16();
			const uint16 y = scriptReadValue16();
			const uint8 alignment = scriptReadValue16();
			const uint16 stringOffset = ReadWord();
			const uint16 entryType = ReadWord();
			if (!overlayTextStageActive) {
				warning("Ignoring overlay text entry at %u,%u without active overlay text stage", x, y);
				continue;
			}
			if (currentView->_overlayTextEntries.size() >= 10) {
				warning("Ignoring overlay text entry because the overlay list is full");
				continue;
			}
			if (entryType != 1) {
				warning("Ignoring overlay text entry with unsupported entry type %u", entryType);
				continue;
			}

			Common::StringArray strings;
			if (_executingScriptObjectID == 0) {
				strings = _engine->DecodeStrings(Scenes::instance().CurrentSceneStrings, stringOffset, 1);
			} else {
				Common::MemoryReadStream *stringsStream = GameObjects::ReadGameObjectStrings(_executingScriptObjectID, g_engine->_fileStream);
				strings = _engine->DecodeStrings(stringsStream, stringOffset, 1);
			}
			if (strings.empty()) {
				warning("Ignoring empty overlay text entry at offset %u", stringOffset);
				continue;
			}
			debugC(kDebugScript,
				"Opcode 3A overlay text: rawPos=(%u,%u) align=%u textOffset=%u entryType=%u scriptObject=%u text=\"%s\"",
				x, y, alignment, stringOffset, entryType, _executingScriptObjectID, strings[0].c_str());

			View1::OverlayTextEntry entry;
			entry.position = Common::Point(x, y);
			entry.alignment = alignment;
			entry.text = strings[0];
			if (entry.text.size() > 0x28) {
				entry.text = entry.text.substr(0, 0x28);
			}
			currentView->addOverlayTextEntry(entry);
		} else if (opcode1 == 0x03B) {
			View1 *currentView = (View1 *)_engine->findView("View1");
			if (currentView != nullptr) {
				currentView->clearOverlayTextEntries();
			}
		} else if (opcode1 == 0x03C) {
			const uint16 fadeSpeed = scriptReadValue16();
			View1 *currentView = (View1 *)_engine->findView("View1");
			if (currentView != nullptr && fadeSpeed != 0) {
				currentView->startFadeToBlack();
			}
		} else if (opcode1 == 0x03D) {
			const uint16 fadeSpeed = scriptReadValue16();
			View1 *currentView = (View1 *)_engine->findView("View1");
			if (currentView != nullptr && fadeSpeed != 0) {
				currentView->startFading();
			}
		} else if (opcode1 == 0x3E) {
			const uint8 resourceIndex = ReadByte();
			Common::Array<uint8> soundData;
			if (!loadSoundResource(soundData, resourceIndex))
				continue;

			if (_engine->hasCurrentSound() && soundEnabled)
				_engine->stopCurrentSound();
			_engine->setCurrentSoundData(soundData);
		} else if (opcode1 == 0x3F) {
			if (soundEnabled)
				_engine->stopCurrentSound();
			_engine->clearCurrentSoundData();
		} else if (opcode1 == 0x40) {
			if (soundEnabled) {
				if (!_engine->hasCurrentSound()) {
					warning("Ignoring sound playback without loaded sound data");
					continue;
				}
				_engine->playCurrentSound();
			}
		} else if (opcode1 == 0x41) {
			if (soundEnabled && mapPanelActive) {
				waitForSoundPlayback = true;
				EndTimer();
				EndBuffering(lastOpcodeTriggeredSkip);
				return ExecutionResult::WaitingForCallback;
			}
		} else if (opcode1 == 0x42) {
			if (soundEnabled)
				_engine->stopCurrentSound();
		} else if (opcode1 == 0x43) {
			const uint16 slotID = scriptReadValue16();
			const uint8 resourceIndex = ReadByte();
			if (slotID < 1 || slotID > 2) {
				warning("Ignoring music load for invalid slot %u", slotID);
				continue;
			}

			Common::Array<uint8> slotData;
			if (loadMusicResource(slotData, resourceIndex))
				musicSlots[slotID - 1] = slotData;
		} else if (opcode1 == 0x44) {
			const uint16 slotID = scriptReadValue16();
			const uint16 startMuted = scriptReadValue16();
			const uint16 fadeParam = scriptReadValue16();
			if (slotID < 1 || slotID > 2) {
				warning("Ignoring music start for invalid slot %u", slotID);
				continue;
			}

			if (!musicEnabled || !mapPanelActive) {
				activeMusicSlot = slotID;
				continue;
			}

			if (activeMusicSlot != 0) {
				_engine->getAdlib()->StopMusic();
				activeMusicSlot = 0;
			}

			if (musicSlots[slotID - 1].empty()) {
				warning("Ignoring music start for empty slot %u", slotID);
				continue;
			}

			_engine->getAdlib()->PlaySongData(musicSlots[slotID - 1]);
			if (startMuted == 0) {
				musicControlMode = 1;
				musicControlParam = fadeParam;
				musicControlVolume = 0x3F;
				_engine->getAdlib()->SetVolume(musicControlVolume);
			} else {
				musicControlMode = 0;
				musicControlParam = 0;
				musicControlVolume = 0;
				_engine->getAdlib()->SetVolume(0);
			}

			activeMusicSlot = slotID;
		} else if (opcode1 == 0x45) {
			const uint16 slotID = scriptReadValue16();
			const uint16 stopImmediately = scriptReadValue16();
			const uint16 fadeParam = scriptReadValue16();
			if (slotID < 1 || slotID > 2) {
				warning("Ignoring music stop for invalid slot %u", slotID);
				continue;
			}

			if (!musicEnabled || !mapPanelActive) {
				activeMusicSlot = 0;
				continue;
			}

			if (activeMusicSlot == slotID) {
				if (stopImmediately == 0) {
					musicControlMode = 2;
					musicControlParam = fadeParam;
				} else {
					_engine->getAdlib()->StopMusic();
					musicControlMode = 0;
					musicControlParam = 0;
					activeMusicSlot = 0;
				}
			}
		} else if (opcode1 == 0x47) {
			if (mapPanelActive && musicEnabled) {
				waitForMusicControl = true;
				EndTimer();
				EndBuffering(lastOpcodeTriggeredSkip);
				return ExecutionResult::WaitingForCallback;
			}
		} else if (opcode1 == 0x46) {
			const uint16 slotID = scriptReadValue16();
			if (slotID < 1 || slotID > 2) {
				warning("Ignoring music free for invalid slot %u", slotID);
				continue;
			}

			if (activeMusicSlot == slotID) {
				if (musicEnabled && mapPanelActive)
					_engine->getAdlib()->StopMusic();
				activeMusicSlot = 0;
			}
			musicSlots[slotID - 1].clear();
		} else if (opcode1 == 0x48) {
			// Retrieve object x and use A334 to save it to a script variable
			int32 objectID = (int32)scriptReadValue32() - 0x400;
			if (objectID < 1 || objectID > 0x200) {
				warning("Ignoring object X query for invalid object %d", objectID);
				continue;
			}
			GameObject *object = GameObjects::GetObjectByIndex((uint16)objectID);
			if (object == nullptr) {
				warning("Ignoring object X query for missing object %d", objectID);
				continue;
			}
			scriptSaveVariable(object->Position.x);
		} else if (opcode1 == 0x49) {
			// Retrieve object y and use A334 to save it to a script variable
			int32 objectID = (int32)scriptReadValue32() - 0x400;
			if (objectID < 1 || objectID > 0x200) {
				warning("Ignoring object Y query for invalid object %d", objectID);
				continue;
			}
			GameObject *object = GameObjects::GetObjectByIndex((uint16)objectID);
			if (object == nullptr) {
				warning("Ignoring object Y query for missing object %d", objectID);
				continue;
			}
			scriptSaveVariable(object->Position.y);
		} else if (opcode1 == 0x4A) {
			int32 objectID = (int32)scriptReadValue32() - 0x400;
			if (objectID < 1 || objectID > 0x200) {
				warning("Ignoring object field query for invalid object %d", objectID);
				continue;
			}
			GameObject *object = GameObjects::GetObjectByIndex((uint16)objectID);
			if (object == nullptr) {
				warning("Ignoring object field query for missing object %d", objectID);
				continue;
			}
			scriptSaveVariable(object->Unknown);
		} else if (opcode1 == 0x4B) {
			// Retrieve object orientation and use A334 to save it to a script variable
			int32 objectID = (int32)scriptReadValue32() - 0x400;
			if (objectID < 1 || objectID > 0x200) {
				warning("Ignoring object orientation query for invalid object %d", objectID);
				continue;
			}
			GameObject *object = GameObjects::GetObjectByIndex((uint16)objectID);
			if (object == nullptr) {
				warning("Ignoring object orientation query for missing object %d", objectID);
				continue;
			}
			scriptSaveVariable(object->Orientation);
		} else if (opcode1 == 0x4C) {
			for (GameObject *object : GameObjects::instance().Objects) {
				if (object != nullptr && object->SceneIndex == Scenes::instance().CurrentActorIndex + 0x400) {
					object->SceneIndex = 0;
				}
			}

			View1 *currentView = (View1 *)_engine->findView("View1");
			if (currentView != nullptr && currentView->inventorySource != nullptr) {
				currentView->SetInventorySource(currentView->inventorySource);
				if (currentView->activeInventoryItem != nullptr &&
					currentView->activeInventoryItem->SceneIndex != currentView->inventorySource->Index) {
					currentView->activeInventoryItem = nullptr;
				}
			}
		} else if (opcode1 == 0x4D) {
			const uint16 sourceValue = scriptReadValue16();
			const uint16 targetValue = scriptReadValue16();
			if (sourceValue < 0xC8 || sourceValue > 0xEF || targetValue < 0xC8 || targetValue > 0xEF) {
				warning("Ignoring pathfinding remap %.4x -> %.4x outside valid range", sourceValue, targetValue);
				continue;
			}
			g_engine->pathfindingValueRemaps[sourceValue] = targetValue;
		} else if (opcode1 == 0x4E) {
			if (mapPanelActive && musicEnabled) {
				waitForAdlibReady = true;
				EndTimer();
				EndBuffering(lastOpcodeTriggeredSkip);
				return ExecutionResult::WaitingForCallback;
			}
		}
		else {
			ScriptUnimplementedOpcode_Main(opcode1);
			EndBuffering(lastOpcodeTriggeredSkip);
			break;
		}
		EndBuffering(lastOpcodeTriggeredSkip);
	}
		isRunningScript = false;
		debug("----- Scripting function left");
		return ExecutionResult::ScriptFinished;
	}
	
	void ScriptExecutor::Run(bool firstRun) {
		// Scene initialization run
		// TODO: Need to better encapsulate this down the road
		// TODO: Not sure which order is really right, need to check in SIS logs
		const bool resumingAfterCallback = (_state == ExecutorState::WaitingForCallback) && !firstRun;
		if (!resumingAfterCallback) {
			// TODO: Not sure if this is the right place and condition to reset this
			// variable. Context here is that we might have an object that triggers several
			// description strings in a row, and we would disable the executing object
			// if we always reset this object
			// TODO: Watch out for issues caused by this
			_executingScriptObjectID = 0;
			IsRepeatRun = false;
			IsSceneInitRun = firstRun;
		}
		_state = ExecutorState::Executing;
		Step();
	}

	void ScriptExecutor::SetScript(Common::MemoryReadStream *stream) {
		_stream = stream;
	}

	void ScriptExecutor::SetCurrentSceneScriptAt(uint32 offset) {
		SetScript(Scenes::instance().CurrentSceneScript);
		_stream->seek(offset, SEEK_SET);
	}

	void ScriptExecutor::tick() {
		if (musicControlMode != 0 && activeMusicSlot != 0) {
			const uint16 step = MAX<uint16>(musicControlParam, 1);
			if (musicControlMode == 1) {
				musicControlVolume = (musicControlVolume > step) ? musicControlVolume - step : 0;
				_engine->getAdlib()->SetVolume(musicControlVolume);
				if (musicControlVolume == 0) {
					musicControlMode = 0;
				}
			} else {
				const uint16 nextVolume = MIN<uint16>(musicControlVolume + step, 0x3F);
				musicControlVolume = nextVolume;
				if (musicControlVolume < 0x3F) {
					_engine->getAdlib()->SetVolume(musicControlVolume);
				} else {
					musicControlMode = 0;
					activeMusicSlot = 0;
					_engine->getAdlib()->StopMusic();
				}
			}
		}

		if (waitForSoundPlayback) {
			if (!_engine->isCurrentSoundPlaying()) {
				waitForSoundPlayback = false;
				Run();
			}
			return;
		}

		if (waitForMusicControl) {
			if (musicControlMode == 0) {
				waitForMusicControl = false;
				Run();
			}
			return;
		}

		if (waitForAdlibReady) {
			if (_engine->getAdlib()->isPlaybackReady()) {
				waitForAdlibReady = false;
				Run();
			}
			return;
		}

		if (isFrameWaitActive) {
			if (frameWaitTicksRemaining > 0) {
				--frameWaitTicksRemaining;
			}
			if (frameWaitTicksRemaining == 0) {
				isFrameWaitActive = false;
				Run();
			}
		}

		if (isTimerActive) {
			if (g_engine->currentMillis > timerEndMillis) {
				isTimerActive = false;
				// TODO: Think about if this is the right way of executing it, or maybe we rather need
				// to use Execute
				Run();
			}
		}
	}

	void ScriptExecutor::StartTimer(uint32 duration) {
		isTimerActive = true;
		timerEndMillis = g_engine->currentMillis + duration;
	}

	void ScriptExecutor::EndTimer() {
		isTimerActive = false;
	}

	void ScriptExecutor::StartFrameWait(uint16 duration) {
		isFrameWaitActive = true;
		frameWaitTicksRemaining = duration;
	}

	void ScriptExecutor::EndFrameWait() {
		isFrameWaitActive = false;
		frameWaitTicksRemaining = 0;
	}

	void ScriptExecutor::Rewind() {
		_stream->seek(0);
	}

} // namespace Script

} // namespace Macs2
