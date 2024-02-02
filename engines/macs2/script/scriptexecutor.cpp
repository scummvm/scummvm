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
#include "common/memstream.h"

namespace Macs2 {
namespace Script {

#define ScriptNoEntry debug("Unhandled case in script handling.");
#define ScriptUnimplementedOpcode(opcode) debug("Unimplemented opcode: " "opcode" ".");

inline void ScriptExecutor::FuncA3D2() {
	// TODO: Quality is not at the level of the rest - consider
	// rewriting from the deassembly
	debug("-- Entering A3D2");
	uint16 skipValue = 1; // [bp-4h] - TODO: Better name
	// TODO: Figure out end condition
	for (;;) {
		const byte opcode = ReadByte();
		const byte length = ReadByte();
		if (opcode >= 3) {
			if (opcode <= 6) {
				skipValue++;
			}
		}
		if (opcode == 8) {
			if (skipValue == 1) {
				skipValue--;
			}
		}
		if (opcode == 7) {
			skipValue--;
		}
		// Do the skipping
		_stream->seek(length, SEEK_CUR);
		debug("A3D2 skipping %u bytes for opcode %.2x (%u)", length, opcode, skipValue);

		// TODO: Add a log here
		if (skipValue != 0) {
			// Continue the loop if there is data left in the stream
			// TODO: Check for remaining script data
		} else {
			if (skipValue != 0) {
				// TODO: Implement:
				// mov	word ptr [1028h],1Dh
				// TODO: Add an assert here to see if this ever happens in practice
			}
			break;
		}
		// TODO: Continue here
	}
	debug("-- Leaving A3D2");
}

void ScriptExecutor::Func9F4D(uint16 &out1, uint16 &out2) {
	// Results are [bp-4h] and [bp-2h]
	// TODO: Implement the actual prelude here correctly, documenting which lables we pass as we go
	// debug("-- Entering 9F4D");
	// fn0037_9F4D proc

	byte opcode1 = ReadByte(); // [bp-5h]
	debug("- 9F4D opcode: %.2x", opcode1);
	// TODO: Consider writing this one also 
	uint16 value = ReadWord(); // [bp-7h]

	if (opcode1 == 0x0) {
		ScriptUnimplementedOpcode(0x0)
		/*
		l0037_9F67:
	;; This handles opcode == 0
	mov	ax,[bp-7h]
	xor	dx,dx
	mov	[bp-4h],ax
	mov	[bp-2h],dx
	*/
	}
	// l0037_9F72:
	if (opcode1 > 0) {
		// l0037_9F78:
		if (opcode1 < 0xFF) {
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
				// value between 1 and 0x800
				// TODO: Implement
			/*
				l0037_9F94:
				mov	ax,[bp-7h]
				shl	ax,2h
				les	di,[06C6h]
				add	di,ax
				mov	ax,es:[di-4h]
				mov	dx,es:[di-2h]
				mov	[bp-4h],ax mov[bp - 2h], dx */
			}
		}
	}
	// l0037_9FAE:
	if (opcode1 != 0xFF) {
		// TODO: Do we write out a 0 for the values?
		debug("- 9F4D results: %.4x %.4x", out1, out2);
		return;
	}
	// l0037_9FB7:
	if (value == 0x1) {
		ScriptUnimplementedOpcode(0x1)
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

l0037_A120:
	cmp	ax,0Eh
	jc	0A13Bh

l0037_A125:
	cmp	ax,22h
	ja	0A13Bh

l0037_A12A:
	mov	ax,[bp-7h]
	sub	ax,0Dh
	xor	dx,dx
	mov	[bp-4h],ax
	mov	[bp-2h],dx
	jmp	0A32Ch

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

l0037_A199:
	cmp	ax,26h
	jnz	0A1BCh

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

byte Script::ScriptExecutor::ReadByte() {
	const int64 pos = _stream->pos();
	const byte result = _stream->readByte();
	debug("Script read (byte): %.2x at location %.4x", result, pos);
	return result;
}

uint16 Script::ScriptExecutor::ReadWord() {
	const int64 pos = _stream->pos();
	const uint16 result = _stream->readUint16LE();
	debug("Script read (word): %.4x at location %.4x", result, pos);
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

			Func9F4D(result1, result2);
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
			Func9F4D(v1, v2);
			if ((v1 | v2) == 0) {
				// FuncA3D2();
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
			Func9F4D(v1, v2);
			// [bp-0Bh]
			uint16 v3;
			// [bp-9h]
			uint16 v4;
			Func9F4D(v3, v4);
			// TODO: Not yet implemented:
			// mov	byte ptr [bp-12h],0h

			bool bp12 = false; // [bp-12h] - TODO: Better name

			if (opcode2 == 0x01) {
				// l0037_DC8F:;
				// TODO Cóntinue here
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
				// FuncA3D2(stream);
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


	void Script::ScriptExecutor::ExecuteScript() {
		debug("----- Scripting function entered");
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

		// The loop comprises the first labels in the file
		// l0037_DB73:
		for (;;) {


		// Read an opcode and length
		byte opcode1 = ReadByte(); // [bp - 1h]
		debug("- First block opcode: %.2x", opcode1);
		byte length = ReadByte();  // [bp-2h]

		

		// TODO: Check if a switch would do it
		if (opcode1 == 0x01) {
			ScriptUnimplementedOpcode(0x01)
			/*
			l0037_DBA0:
			call	far 0037h:9F07h
			call	far 0037h:9F23h
			mov	[bp-11h],ax
			call	far 0037h:9F4Dh
			mov	cx,ax
			mov	bx,dx
			mov	ax,[bp-11h]
			shl	ax,2h
			les	di,[06C6h]
			add	di,ax
			mov	es:[di-4h],cx
			mov	es:[di-2h],bx
			jmp	0E3BAh
			*/
		} // l0037_DBCD:
		else if (opcode1 == 0x02) {
			ScriptUnimplementedOpcode(0x02)
			/*
			l0037_DBD1:
			call	far 0037h:9F07h
			call	far 0037h:9F23h
			mov	[bp-11h],ax
			call	far 0037h:9F4Dh
			mov	cx,10h
			xor	bx,bx
			call	far 00CDh:0D93h
			mov	[bp-7h],ax
			mov	[bp-5h],dx
			call	far 0037h:9F4Dh
			or	ax,[bp-7h]
			or	dx,[bp-5h]
			mov	[bp-7h],ax
			mov	[bp-5h],dx
			mov	cx,[bp-7h]
			mov	bx,[bp-5h]
			mov	ax,[bp-11h]
			shl	ax,2h
			les	di,[06C6h]
			add	di,ax
			mov	es:[di-4h],cx
			mov	es:[di-2h],bx
			jmp	0E3BAh*/
		} // l0037_DC21:
		else if (opcode1 == 0x03) {
			ScriptUnimplementedOpcode(0x03)
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
			Func9F4D(result1, result2);
			// TODO: Check if we need the values further below
			/*
			mov	[bp-7h],ax
			mov	[bp-5h],dx
			*/
			// If any bit is set in the result, we skip, otherwise we fall through and continue the loop
			if ((result1 | result2) == 0) {
				FuncA3D2();
			} else {
				// Making it explicit with a continue
				continue;
			}


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
		} else {
			ScriptNoEntry
			// TODO: Breaking for now to be able to see the log until this point
			break;
		}
		}
		// 0E3BDh
		
	

	
	/*
	
	
	
		
	

	l0037_DC5F:
		;; #path We know that AL needs to be 0Ah already
		cmp	al,5h
		jz	0DC66h

	l0037_DC63:
		;; #path This needs to be executed in order to reach the print string
		jmp	0DD3Ch

	l0037_DC66:
		call	far 0037h:9F07h
		mov	[bp-3h],al
		;; TODO: I just now realized this one - why do we call the same function two times? And does
		;; the result change in between?
		call	far 0037h:9F4Dh
		;; This is the result of the function that mapped the action to a hotspot (0037:9F4D)
		mov	[bp-7h],ax
		mov	[bp-5h],dx
		;; Note: I don't think (TODO: Confirm) that the locals here are accessed in this function
		call	far 0037h:9F4Dh
		mov	[bp-0Bh],ax
		mov	[bp-9h],dx
		mov	byte ptr [bp-12h],0h
		mov	al,[bp-3h]
		cmp	al,1h
		jnz	0DCA6h

	l0037_DC8F:
		;; This is called for opcode2 == 1h
		;; These are the results of the function that mapped the action to a hotspot (0037:9F4D)
		;; Note: This AX here will determine in the end the string that is printed - so look up where it is used
		mov	ax,[bp-7h]
		mov	dx,[bp-5h]
		;; Note: For the look on the bow of the ship, both of these are 0
		cmp	dx,[bp-9h]
		;; #script_skip: [bp-9h] == [bp-5h] will lead to the skip, but there is also a way along the non-jump
		jnz	0DCA3h

	l0037_DC9A:
		;; Note: For the look on the bow of the ship, the comparison value is 0409 wheras the hotspot ID is 0806
		;; Note: There is also a pass over this function where we have the right comparison value
		cmp	ax,[bp-0Bh]
		;; Note: If we set [bp-0Bh] to a different hotspot ID at this time, we don't change the behaviour. So it seems like we
		;; have chosen the string earlier than this.
		;; Note: During the next pass over the function, the AL value of 0Ah is set, which goes on to the string printing
		;; So probably something iteresting is happening in the function that sets the [bp-0Bh] result to the right hotspot
		jnz	0DCA3h

	l0037_DC9F:
		;; If [bp-0Bh] is set to the right hotspot, [bp-12h] is set to 1
		mov	byte ptr [bp-12h],1h

	l0037_DCA3:
		jmp	0DD2Eh

	l0037_DCA6:
		cmp	al,2h
		jnz	0DCC0h

	l0037_DCAA:
		mov	ax,[bp-7h]
		mov	dx,[bp-5h]
		cmp	dx,[bp-9h]
		jnz	0DCBAh

	l0037_DCB5:
		cmp	ax,[bp-0Bh]
		jz	0DCBEh

	l0037_DCBA:
		mov	byte ptr [bp-12h],1h

	l0037_DCBE:
		jmp	0DD2Eh

	l0037_DCC0:
		cmp	al,3h
		jnz	0DCDCh

	l0037_DCC4:
		mov	ax,[bp-7h]
		mov	dx,[bp-5h]
		cmp	dx,[bp-9h]
		jl	0DCD6h

	l0037_DCCF:
		jg	0DCDAh

	l0037_DCD1:
		cmp	ax,[bp-0Bh]
		jnc	0DCDAh

	l0037_DCD6:
		mov	byte ptr [bp-12h],1h

	l0037_DCDA:
		jmp	0DD2Eh

	l0037_DCDC:
		cmp	al,4h
		jnz	0DCF8h

	l0037_DCE0:
		mov	ax,[bp-7h]
		mov	dx,[bp-5h]
		cmp	dx,[bp-9h]
		jg	0DCF2h

	l0037_DCEB:
		jl	0DCF6h

	l0037_DCED:
		cmp	ax,[bp-0Bh]
		jbe	0DCF6h

	l0037_DCF2:
		mov	byte ptr [bp-12h],1h

	l0037_DCF6:
		jmp	0DD2Eh

	l0037_DCF8:
		cmp	al,5h
		jnz	0DD14h

	l0037_DCFC:
		mov	ax,[bp-7h]
		mov	dx,[bp-5h]
		cmp	dx,[bp-9h]
		jl	0DD0Eh

	l0037_DD07:
		jg	0DD12h

	l0037_DD09:
		cmp	ax,[bp-0Bh]
		ja	0DD12h

	l0037_DD0E:
		mov	byte ptr [bp-12h],1h

	l0037_DD12:
		jmp	0DD2Eh

	l0037_DD14:
		cmp	al,6h
		jnz	0DD2Eh

	l0037_DD18:
		mov	ax,[bp-7h]
		mov	dx,[bp-5h]
		cmp	dx,[bp-9h]
		;; #script_skip: We don't want to execute this jump, so [bp-5h] needs to be <= [bp-9h]
		jg	0DD2Ah

	l0037_DD23:
		jl	0DD2Eh

	l0037_DD25:
		cmp	ax,[bp-0Bh]
		;; #script_skip: We want to execute the jump, so ax should be < [bp-0Bh]
		jc	0DD2Eh

	l0037_DD2A:
		;; #script_skip: We don't want to end up here
		mov	byte ptr [bp-12h],1h

	l0037_DD2E:
		;; Note: For a look on the bow of the ship, the value here is 1400
		;; For a 0 here, we will go to the skipping function
		cmp	byte ptr [bp-12h],0h
		;; #script_skip: if [bp-12h] == 0, we will skip the script
		jnz	0DD39h

	l0037_DD34:
		;; We skip a section of the script
		;; To document all ways to get there, use this new tag:
		;; #script_skip: We are skipping here.
		call	far 0037h:0A3D2h
		;; After the call, AX is reset

	l0037_DD39:
		jmp	0E3BAh

	l0037_DD3C:
		;; path - we already know that AL needs to be 0Ah to reach the right position, so it can't be 6h here
		cmp	al,6h
		jz	0DD43h

	l0037_DD40:
		;; #path - we need to execute this jump to get there
		jmp	0DDD8h

	l0037_DD43:
		call	far 0037h:9F07h
		mov	[bp-3h],al
		call	far 0037h:9F4Dh
		mov	[bp-7h],ax
		mov	[bp-5h],dx
		call	far 0037h:9F4Dh
		mov	[bp-0Bh],ax
		mov	[bp-9h],dx
		call	far 0037h:9F4Dh
		mov	[bp-0Fh],ax
		mov	[bp-0Dh],dx
		mov	byte ptr [bp-12h],0h
		mov	ax,[bp-0Bh]
		mov	dx,[bp-9h]
		mov	cx,10h
		xor	bx,bx
		call	far 00CDh:0D93h
		or	ax,[bp-0Fh]
		or	dx,[bp-0Dh]
		cmp	dx,[bp-5h]
		jnz	0DD94h

	l0037_DD8B:
		cmp	ax,[bp-7h]
		jnz	0DD94h

	l0037_DD90:
		mov	byte ptr [bp-12h],1h

	l0037_DD94:
		mov	ax,[bp-0Fh]
		mov	dx,[bp-0Dh]
		mov	cx,10h
		xor	bx,bx
		call	far 00CDh:0D93h
		or	ax,[bp-0Bh]
		or	dx,[bp-9h]
		cmp	dx,[bp-5h]
		jnz	0DDB8h

	l0037_DDAF:
		cmp	ax,[bp-7h]
		jnz	0DDB8h

	l0037_DDB4:
		mov	byte ptr [bp-12h],1h

	l0037_DDB8:
		cmp	byte ptr [bp-3h],2h
		jnz	0DDCAh

	l0037_DDBE:
		cmp	byte ptr [bp-12h],0h
		mov	al,0h
		jnz	0DDC7h

	l0037_DDC6:
		inc	ax

	l0037_DDC7:
		mov	[bp-12h],al

	l0037_DDCA:
		cmp	byte ptr [bp-12h],0h
		jnz	0DDD5h

	l0037_DDD0:
		;; Note: Changing [1024h] before this call makes a difference still
		call	far 0037h:0A3D2h

	l0037_DDD5:
		jmp	0E3BAh

	l0037_DDD8:
		;; #path - Compare AL with 8h
		cmp	al,8h
		;; #path - We need to execute the jump, so ZF needs to be 0, so AL needs to be 8h (actually, it should be 0Ah
		;; as we learned below)
		jnz	0DDE4h

	l0037_DDDC:
		call	far 0037h:0A37Ah
		jmp	0E3BAh

	l0037_DDE4:
		;; #path - The ZF will be set if AL == 0Ah
		cmp	al,0Ah
		;; #path - We will jump over the string printing if the zero flag is not set, so if AL != 0Ah
		;; Therefore, AL has to be 0Ah at this time to print the string
		jnz	0DDF5h

	l0037_DDE8:
		push	0h
		;; This is the call that ends up drawing the string
		;; Working my way back by using the hash tag #path
		;; [bp-0Bh] and [bp-9h] have the info about the string
		call	far 0037h:0A903h
		jmp	0E3BDh
	0037:DDF2       E9 C5 05                                    ...

	l0037_DDF5:
		cmp	al,0Bh
		jnz	0DE0Dh

	l0037_DDF9:
		;; This is where we end up picking up the cup in the bar, so 0Bh seems to be the AL for
		;; triggering a pickup (but keeping in mind that the stick in the start scene is picked up with
		;; a different mechanism)
		call	far 0037h:0AA83h
		mov	ax,[0F8Ah]
		cmp	ax,[0F90h]
		jc	0DE0Ah

	l0037_DE07:
		jmp	0E3BDh

	l0037_DE0A:
		jmp	0E3BAh

	l0037_DE0D:
		cmp	al,0Ch
		jnz	0DE23h

	l0037_DE11:
		call	far 0037h:0AD6Eh
		cmp	byte ptr [0F88h],0h
		jz	0DE20h

	l0037_DE1D:
		jmp	0E3BDh

	l0037_DE20:
		jmp	0E3BAh

	l0037_DE23:
		cmp	al,0Dh
		jnz	0DE32h

	l0037_DE27:
		call	far 0037h:0B075h
		jmp	0E3BDh
	0037:DE2F                                              E9                .
	0037:DE30 88 05                                           ..

	l0037_DE32:
		cmp	al,0Eh
		jnz	0DE3Eh

	l0037_DE36:
		call	far 0037h:0B6BEh
		jmp	0E3BAh

	l0037_DE3E:
		cmp	al,0Fh
		jnz	0DE6Eh

	l0037_DE42:
		call	far 0037h:9F4Dh
		mov	[100Ah],ax
		call	far 0037h:0002h
		cmp	word ptr [0774h],1Ah
		jz	0DE5Ch

	l0037_DE56:
		mov	ax,[0774h]
		mov	[0526h],ax

	l0037_DE5C:
		push	1Ah
		call	far 0037h:3EA5h
		call	far 0037h:0056h
		jmp	0E3BDh
	0037:DE6B                                  E9 4C 05                  .L.

	l0037_DE6E:
		cmp	al,10h
		jnz	0DE7Ah

	l0037_DE72:
		call	far 0037h:0B843h
		jmp	0E3BAh

	l0037_DE7A:
		cmp	al,11h
		jz	0DE81h

	l0037_DE7E:
		jmp	0DF30h

	l0037_DE81:
		call	far 0037h:9F4Dh
		sub	ax,400h
		sbb	dx,0h
		mov	[1016h],ax
		cmp	word ptr [1016h],1h
		jl	0DE9Eh

	l0037_DE96:
		cmp	word ptr [1016h],200h
		jle	0DEA4h

	l0037_DE9E:
		mov	word ptr [1028h],2h

	l0037_DEA4:
		mov	di,[1016h]
		shl	di,2h
		mov	ax,[di+77Ch]
		or	ax,[di+77Eh]
		jnz	0DEBBh

	l0037_DEB5:
		mov	word ptr [1028h],19h

	l0037_DEBB:
		cmp	word ptr [1028h],0h
		jz	0DEC5h

	l0037_DEC2:
		jmp	0E3E5h

	l0037_DEC5:
		mov	di,[1016h]
		shl	di,2h
		les	di,[di+77Ch]
		mov	ax,es:[di+0Ah]
		or	ax,es:[di+0Ch]
		jnz	0DEE0h

	l0037_DEDA:
		mov	word ptr [1028h],2h

	l0037_DEE0:
		cmp	word ptr [1028h],0h
		jz	0DEEAh

	l0037_DEE7:
		jmp	0E3E5h

	l0037_DEEA:
		mov	di,[1016h]
		shl	di,2h
		les	di,[di+77Ch]
		les	di,es:[di+0Ah]
		cmp	byte ptr es:[di+231h],0h
		jz	0DF07h

	l0037_DF01:
		mov	word ptr [1028h],1Fh

	l0037_DF07:
		call	far 0037h:0002h
		cmp	word ptr [0774h],1Ah
		jz	0DF19h

	l0037_DF13:
		mov	ax,[0774h]
		mov	[0526h],ax

	l0037_DF19:
		;; This is where we set the cursor mode before starting a pick up animation - for the "cup" variant of picking up.
		;; The AL associated with it is 11h
		push	1Ah
		call	far 0037h:3EA5h
		call	far 0037h:0056h
		xor	ax,ax
		mov	[1018h],ax
		jmp	0E3BDh
	0037:DF2D                                        E9 8A 04              ...

	l0037_DF30:
		cmp	al,12h
		jnz	0DF3Ch

	l0037_DF34:
		call	far 0037h:0C6D7h
		jmp	0E3BAh

	l0037_DF3C:
		cmp	al,13h
		jnz	0DF48h

	l0037_DF40:
		call	far 0037h:0A439h
		jmp	0E3BAh

	l0037_DF48:
		cmp	al,14h
		jnz	0DF54h

	l0037_DF4C:
		call	far 0037h:9F23h
		jmp	0E3BAh

	l0037_DF54:
		cmp	al,15h
		jnz	0DF66h

	l0037_DF58:
		les	di,[0778h]
		xor	ax,ax
		mov	es:[di+5355h],ax
		jmp	0E3BAh

	l0037_DF66:
		cmp	al,16h
		jnz	0DF72h

	l0037_DF6A:
		call	far 0037h:0C75Ah
		jmp	0E3BAh

	l0037_DF72:
		cmp	al,17h
		jnz	0DF81h

	l0037_DF76:
		call	far 0037h:0CEB9h
		jmp	0E3BDh
	0037:DF7E                                           E9 39               .9
	0037:DF80 04                                              .

	l0037_DF81:
		cmp	al,18h
		jnz	0DF8Eh

	l0037_DF85:
		mov	ax,[0F90h]
		mov	[0F8Ah],ax
		jmp	0E3BAh

	l0037_DF8E:
		cmp	al,19h
		jnz	0DF9Dh

	l0037_DF92:
		call	far 0037h:0C475h
		jmp	0E3BDh
	0037:DF9A                               E9 1D 04                    ...

	l0037_DF9D:
		cmp	al,1Ah
		jz	0DFA4h

	l0037_DFA1:
		jmp	0E04Dh

	l0037_DFA4:
		call	far 0037h:9F4Dh
		sub	ax,400h
		sbb	dx,0h
		mov	[bp-7h],ax
		mov	[bp-5h],dx
		cmp	word ptr [bp-5h],0h
		jl	0DFD2h

	l0037_DFBB:
		jg	0DFC3h

	l0037_DFBD:
		cmp	word ptr [bp-7h],1h
		jc	0DFD2h

	l0037_DFC3:
		cmp	word ptr [bp-5h],0h
		jg	0DFD2h

	l0037_DFC9:
		jl	0DFD8h

	l0037_DFCB:
		cmp	word ptr [bp-7h],200h
		jbe	0DFD8h

	l0037_DFD2:
		mov	word ptr [1028h],2h

	l0037_DFD8:
		mov	di,[bp-7h]
		shl	di,2h
		mov	ax,[di+77Ch]
		or	ax,[di+77Eh]
		jnz	0DFEEh

	l0037_DFE8:
		mov	word ptr [1028h],19h

	l0037_DFEE:
		cmp	word ptr [1028h],0h
		jz	0DFF8h

	l0037_DFF5:
		jmp	0E3E5h

	l0037_DFF8:
		mov	di,[bp-7h]
		shl	di,2h
		les	di,[di+77Ch]
		mov	ax,es:[di+0Ah]
		or	ax,es:[di+0Ch]
		jnz	0E012h

	l0037_E00C:
		mov	word ptr [1028h],2h

	l0037_E012:
		cmp	word ptr [1028h],0h
		jz	0E01Ch

	l0037_E019:
		jmp	0E3E5h

	l0037_E01C:
		mov	di,[bp-7h]
		shl	di,2h
		les	di,[di+77Ch]
		les	di,es:[di+0Ah]
		mov	[bp-16h],di
		mov	[bp-14h],es
		call	far 0037h:9F4Dh
		les	di,[bp-16h]
		mov	es:[di+217h],ax
		call	far 0037h:9F4Dh
		les	di,[bp-16h]
		mov	es:[di+219h],ax
		jmp	0E3BAh

	l0037_E04D:
		cmp	al,1Bh
		jnz	0E059h

	l0037_E051:
		call	far 0037h:0BC8Bh
		jmp	0E3BAh

	l0037_E059:
		cmp	al,1Ch
		jnz	0E065h

	l0037_E05D:
		mov	byte ptr [102Ah],1h
		jmp	0E3BAh

	l0037_E065:
		cmp	al,1Dh
		jnz	0E071h

	l0037_E069:
		mov	byte ptr [102Ah],0h
		jmp	0E3BAh

	l0037_E071:
		cmp	al,1Eh
		jnz	0E07Dh

	l0037_E075:
		call	far 0037h:0BD58h
		jmp	0E3BAh

	l0037_E07D:
		cmp	al,1Fh
		jnz	0E089h

	l0037_E081:
		call	far 0037h:0BF9Dh
		jmp	0E3BAh

	l0037_E089:
		cmp	al,20h
		jnz	0E095h

	l0037_E08D:
		call	far 0037h:0C047h
		jmp	0E3BAh

	l0037_E095:
		cmp	al,21h
		jnz	0E0A1h

	l0037_E099:
		call	far 0037h:0C0ECh
		jmp	0E3BAh

	l0037_E0A1:
		cmp	al,22h
		jnz	0E0ADh

	l0037_E0A5:
		call	far 0037h:0C2C4h
		jmp	0E3BAh

	l0037_E0AD:
		cmp	al,23h
		jnz	0E0B9h

	l0037_E0B1:
		call	far 0037h:0BAFCh
		jmp	0E3BAh

	l0037_E0B9:
		cmp	al,24h
		jnz	0E0C5h

	l0037_E0BD:
		call	far 0037h:0C7E6h
		jmp	0E3BAh

	l0037_E0C5:
		cmp	al,25h
		jnz	0E0D1h

	l0037_E0C9:
		call	far 0037h:0C81Fh
		jmp	0E3BAh

	l0037_E0D1:
		cmp	al,26h
		jnz	0E0DDh

	l0037_E0D5:
		call	far 0037h:0C991h
		jmp	0E3BAh

	l0037_E0DD:
		cmp	al,27h
		jnz	0E0E9h

	l0037_E0E1:
		call	far 0037h:0C858h
		jmp	0E3BAh

	l0037_E0E9:
		cmp	al,28h
		jnz	0E0F5h

	l0037_E0ED:
		call	far 0037h:0C8E4h
		jmp	0E3BAh

	l0037_E0F5:
		cmp	al,29h
		jnz	0E110h

	l0037_E0F9:
		call	far 0037h:0C3E6h
		mov	ax,[0F90h]
		mov	[0F8Ah],ax
		mov	word ptr [0F92h],201h
		jmp	0E3BDh
	0037:E10D                                        E9 AA 02              ...

	l0037_E110:
		cmp	al,2Ah
		jnz	0E11Ch

	l0037_E114:
		call	far 0037h:0CB45h
		jmp	0E3BAh

	l0037_E11C:
		cmp	al,2Bh
		jnz	0E128h

	l0037_E120:
		call	far 0037h:0CD1Bh
		jmp	0E3BAh

	l0037_E128:
		cmp	al,2Ch
		jnz	0E134h

	l0037_E12C:
		call	far 0037h:0CD99h
		jmp	0E3BAh

	l0037_E134:
		cmp	al,2Dh
		jnz	0E140h

	l0037_E138:
		call	far 0037h:0C345h
		jmp	0E3BAh

	l0037_E140:
		cmp	al,2Eh
		jnz	0E14Ch

	l0037_E144:
		call	far 0037h:0B78Dh
		jmp	0E3BAh

	l0037_E14C:
		cmp	al,2Fh
		jnz	0E158h

	l0037_E150:
		call	far 0037h:0BE91h
		jmp	0E3BAh

	l0037_E158:
		cmp	al,30h
		jnz	0E169h

	l0037_E15C:
		push	1h
		call	far 0037h:0A903h
		jmp	0E3BDh
	0037:E166                   E9 51 02                            .Q.

	l0037_E169:
		cmp	al,31h
		jnz	0E175h

	l0037_E16D:
		call	far 0037h:0CE0Bh
		jmp	0E3BAh

	l0037_E175:
		cmp	al,32h
		jnz	0E181h

	l0037_E179:
		call	far 0037h:0B9BAh
		jmp	0E3BAh

	l0037_E181:
		cmp	al,33h
		jnz	0E18Dh

	l0037_E185:
		call	far 0037h:0BA5Bh
		jmp	0E3BAh

	l0037_E18D:
		cmp	al,34h
		jnz	0E199h

	l0037_E191:
		call	far 0037h:0CE40h
		jmp	0E3BAh

	l0037_E199:
		cmp	al,35h
		jnz	0E1A5h

	l0037_E19D:
		call	far 0037h:0C19Fh
		jmp	0E3BAh

	l0037_E1A5:
		cmp	al,36h
		jnz	0E1B1h

	l0037_E1A9:
		call	far 0037h:0D6DDh
		jmp	0E3BAh

	l0037_E1B1:
		cmp	al,37h
		jnz	0E1BDh

	l0037_E1B5:
		call	far 0037h:0AD3Eh
		jmp	0E3BAh

	l0037_E1BD:
		cmp	al,38h
		jnz	0E1C9h

	l0037_E1C1:
		call	far 0037h:0D749h
		jmp	0E3BAh

	l0037_E1C9:
		cmp	al,39h
		jnz	0E1D5h

	l0037_E1CD:
		call	far 0037h:0D80Fh
		jmp	0E3BAh

	l0037_E1D5:
		cmp	al,3Ah
		jnz	0E1E1h

	l0037_E1D9:
		call	far 0037h:0D82Ch
		jmp	0E3BAh

	l0037_E1E1:
		cmp	al,3Bh
		jnz	0E1EDh

	l0037_E1E5:
		call	far 0037h:0D902h
		jmp	0E3BAh

	l0037_E1ED:
		cmp	al,3Ch
		jnz	0E225h

	l0037_E1F1:
		cmp	byte ptr [23B4h],0h
		jnz	0E21Dh

	l0037_E1F8:
		call	far 0037h:9F4Dh
		mov	[bp-7h],ax
		mov	[bp-5h],dx
		les	di,[0778h]
		les	di,es:[di+1004h]
		push	es
		push	di
		push	0h
		push	100h
		push	word ptr [bp-7h]
		call	far 00B7h:00BAh
		jmp	0E222h

	l0037_E21D:
		call	far 0037h:9F4Dh

	l0037_E222:
		jmp	0E3BAh

	l0037_E225:
		cmp	al,3Dh
		jnz	0E25Dh

	l0037_E229:
		cmp	byte ptr [23B4h],0h
		jnz	0E255h

	l0037_E230:
		call	far 0037h:9F4Dh
		mov	[bp-7h],ax
		mov	[bp-5h],dx
		les	di,[0778h]
		les	di,es:[di+1004h]
		push	es
		push	di
		push	0h
		push	100h
		push	word ptr [bp-7h]
		call	far 00B7h:012Fh
		jmp	0E25Ah

	l0037_E255:
		call	far 0037h:9F4Dh

	l0037_E25A:
		jmp	0E3BAh

	l0037_E25D:
		cmp	al,3Eh
		jnz	0E269h

	l0037_E261:
		call	far 0017h:0B27h
		jmp	0E3BAh

	l0037_E269:
		cmp	al,3Fh
		jnz	0E275h

	l0037_E26D:
		call	far 0017h:0C2Fh
		jmp	0E3BAh

	l0037_E275:
		cmp	al,40h
		jnz	0E281h

	l0037_E279:
		call	far 0017h:0C7Fh
		jmp	0E3BAh

	l0037_E281:
		cmp	al,41h
		jnz	0E2BCh

	l0037_E285:
		cmp	byte ptr [06BEh],0h
		jz	0E2B9h

	l0037_E28C:
		cmp	byte ptr [1F4Ch],0h
		jz	0E2B9h

	l0037_E293:
		mov	byte ptr [100Ch],1h
		call	far 0037h:0002h
		cmp	word ptr [0774h],1Ah
		jz	0E2AAh

	l0037_E2A4:
		mov	ax,[0774h]
		mov	[0526h],ax

	l0037_E2AA:
		push	1Ah
		call	far 0037h:3EA5h
		call	far 0037h:0056h
		jmp	0E3BDh

	l0037_E2B9:
		jmp	0E3BAh

	l0037_E2BC:
		cmp	al,42h
		jnz	0E2C8h

	l0037_E2C0:
		call	far 0017h:0C6Ch
		jmp	0E3BAh

	l0037_E2C8:
		cmp	al,43h
		jnz	0E2D4h

	l0037_E2CC:
		call	far 0017h:0CACh
		jmp	0E3BAh

	l0037_E2D4:
		cmp	al,44h
		jnz	0E2E0h

	l0037_E2D8:
		call	far 0017h:0E1Eh
		jmp	0E3BAh

	l0037_E2E0:
		cmp	al,45h
		jnz	0E2ECh

	l0037_E2E4:
		call	far 0017h:0F09h
		jmp	0E3BAh

	l0037_E2EC:
		cmp	al,46h
		jnz	0E2F8h

	l0037_E2F0:
		call	far 0017h:0FA4h
		jmp	0E3BAh

	l0037_E2F8:
		cmp	al,47h
		jnz	0E333h

	l0037_E2FC:
		cmp	byte ptr [1F4Ch],0h
		jz	0E330h

	l0037_E303:
		cmp	byte ptr [06C0h],0h
		jz	0E330h

	l0037_E30A:
		mov	byte ptr [100Eh],1h
		call	far 0037h:0002h
		cmp	word ptr [0774h],1Ah
		jz	0E321h

	l0037_E31B:
		mov	ax,[0774h]
		mov	[0526h],ax

	l0037_E321:
		push	1Ah
		call	far 0037h:3EA5h
		call	far 0037h:0056h
		jmp	0E3BDh

	l0037_E330:
		jmp	0E3BAh

	l0037_E333:
		cmp	al,48h
		jnz	0E33Eh

	l0037_E337:
		call	far 0037h:0D917h
		jmp	0E3BAh

	l0037_E33E:
		cmp	al,49h
		jnz	0E349h

	l0037_E342:
		call	far 0037h:0D977h
		jmp	0E3BAh

	l0037_E349:
		cmp	al,4Ah
		jnz	0E354h

	l0037_E34D:
		call	far 0037h:0D9D8h
		jmp	0E3BAh

	l0037_E354:
		cmp	al,4Bh
		jnz	0E35Fh

	l0037_E358:
		call	far 0037h:0DA3Ah
		jmp	0E3BAh

	l0037_E35F:
		cmp	al,4Ch
		jnz	0E36Ah

	l0037_E363:
		call	far 0037h:0DA9Ch
		jmp	0E3BAh

	l0037_E36A:
		cmp	al,4Dh
		jnz	0E375h

	l0037_E36E:
		call	far 0037h:0DAFBh
		jmp	0E3BAh

	l0037_E375:
		cmp	al,4Eh
		jnz	0E3AEh

	l0037_E379:
		cmp	byte ptr [1F4Ch],0h
		jz	0E3ACh

	l0037_E380:
		cmp	byte ptr [06C0h],0h
		jz	0E3ACh

	l0037_E387:
		mov	byte ptr [1010h],1h
		call	far 0037h:0002h
		cmp	word ptr [0774h],1Ah
		jz	0E39Eh

	l0037_E398:
		mov	ax,[0774h]
		mov	[0526h],ax

	l0037_E39E:
		push	1Ah
		call	far 0037h:3EA5h
		call	far 0037h:0056h
		jmp	0E3BDh

	l0037_E3AC:
		jmp	0E3BAh

	l0037_E3AE:
		cmp	byte ptr [bp-1h],8h
		jbe	0E3BAh

	l0037_E3B4:
		mov	word ptr [1028h],7h

	l0037_E3BA:
	;; End of the loop starting at DB73
		jmp	0DB73h

	l0037_E3BD:
		;; Note: During a look on the ship's planks, we exit the loop to this label and by this time, the
		;; string has already been printed
		cmp	word ptr [1028h],0h
		jz	0E3E5h

	l0037_E3C4:
		mov	ax,[0F8Ah]
		mov	[06C2h],ax
		mov	ax,[0F92h]
		mov	[06C4h],ax
		cmp	word ptr [06C4h],0h
		jbe	0E3DFh

	l0037_E3D7:
		add	word ptr [06C4h],400h
		jmp	0E3E5h

	l0037_E3DF:
		mov	ax,[077Ch]
		mov	[06C4h],ax

	l0037_E3E5:
		leave
		retf


*/
debug("----- Scripting function left");
	}

	void ScriptExecutor::SetScript(Common::MemoryReadStream *stream) {
		_stream = stream;
	}

} // namespace Script 
} // namespace Macs2

