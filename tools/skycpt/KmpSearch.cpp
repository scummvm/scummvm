/* ScummVM Tools
 * Copyright (C) 2007 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "stdafx.h"
#include "KmpSearch.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _MSC_VER

__declspec(naked) void __fastcall KmpSearch::init(const char *subStr) {
	__asm {
		// kmp initialization, make jump table for mismatches
		push esi
		push edi
		push ebp
		push ebx
		push ecx

		mov esi, edx // edx: subStr argument
		lea edi, [ecx + KmpSearch::_subStr]   // this + 0x100
		lea ebx, [ecx + KmpSearch::_retarget] // this
		lea ebp, [ecx + 1]

		mov byte ptr [ebx], -1

		xor eax, eax

		loopStart:
			shr ecx, 8
			test al, 3
			jnz short dontLoad
				mov ecx, dword ptr [esi + eax]     // load next 4 bytes of subStr
				mov dword ptr [edi], ecx		   // and copy them to this._subStr while we're at it
				lea edi, [edi + 4]
			dontLoad:

			or cl, cl				     // end of subStr?
			jz short endOfString

			mov edx, eax			     // save counter (i) in edx

			xlat					     // al = retarget[i]
			inc al
			mov byte ptr [ebp + edx], al // retarget[i + 1] = retarget[i] + 1
			jz short decLoopEnd
				decrementLoop:
					dec al
					mov ah, byte ptr [esi + eax]	// ah = sub[retarget[i + 1] - 1]
					cmp ah, cl						// if (ah == sub[i])
					jz short decLoopEnd				//     goto decLoopEnd

					xlat							// al = retarget[retarget[i + 1] - 1]
					xor ah, ah
					inc al							// al = retarget[retarget[i + 1] - 1] + 1
					mov byte ptr [ebp + edx], al	// retarget[i + 1] = al
				jnz short decrementLoop
			decLoopEnd:
			lea eax, [edx + 1]		     // i = i + 1
		jmp short loopStart

		endOfString:

		pop ecx		// this
		mov [ecx + KmpSearch::_strLen], eax // length of substring (without '\0')

		pop ebx
		pop ebp
		pop edi
		pop esi
		ret
	}
}

__declspec(naked) const char * __fastcall KmpSearch::search(const char *str) {
	__asm {
		push esi
		push edi
		push ebx

		mov esi, edx	// edx: str argument
		lea edi, [ecx + KmpSearch::_subStr]
		lea ebx, [ecx + KmpSearch::_retarget]
		mov ch,  byte ptr [ecx + KmpSearch::_strLen]

		or  ch, ch				// if (_strLen == 0)
		jz  short endOfString	//	   goto endOfString

		xor edx, edx	// index

		mov cl, 3
		searchLoop:
			shr eax, 8
			inc cl
			test cl, 4
			jz short skipRead
				lodsd
				xor cl, cl
			skipRead:

			test al, al
			jz short endOfString

			newIndexLoop:
				cmp al, byte ptr [edi + edx]	 // while (c != sub[index]) {
				jz short gotNewIndex
				or edx, edx						 //		if (index == 0)
				jz short searchLoop				 //			 goto searchLoop
				movzx edx, byte ptr [ebx + edx]  //		index = retarget[index]
			jmp short newIndexLoop				 // }

			gotNewIndex:
			inc edx
			cmp dl, ch		  // if (index != _strLen)
		jne short searchLoop  //	goto searchLoop

		movzx ebx, cl   // bytes of eax used
		movzx ecx, ch   // length of subStr

		lea eax, [esi + ebx - 3]
		sub eax, ecx

		pop ebx
		pop edi
		pop esi
		ret

		endOfString:
		xor eax, eax	// return NULL
		pop ebx
		pop edi
		pop esi
		ret
	}
}

#else

void __fastcall KmpSearch::init(const char *subStr) {
	strcpy(_subStr, subStr);
}

const char * __fastcall KmpSearch::search(const char *str) {
	return strstr(str, _subStr);
}

#endif

