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

// Taken from mintlib (https://github.com/freemint/mintlib)
// (c) Thorsten Otto

#include <mint/osbind.h>
#include <stdint.h>

#define NATFEAT_ID   0x7300
#define NATFEAT_CALL 0x7301

#define ASM_NATFEAT3(opcode) "\t.word " #opcode "\n"
#define ASM_NATFEAT2(opcode) ASM_NATFEAT3(opcode)
#define ASM_NATFEAT(n) ASM_NATFEAT2(n)

static unsigned short const nf_id_opcodes[] = { NATFEAT_ID, 0x4e75 };
static unsigned short const nf_call_opcodes[] = { NATFEAT_CALL, 0x4e75 };

#define _nf_get_id(feature_name) ((long (__CDECL *)(const char *))nf_id_opcodes)(feature_name)
#define _nf_call(id, ...) ((long (__CDECL *)(long, ...))nf_call_opcodes)(id, __VA_ARGS__)

/*
 * on ColdFire, the NATFEAT_ID opcode is actually
 * "mvs.b d0,d1",
 * which means the following code will NOT detect
 * the presence of an emulator (should there ever
 * be an emulator capable of emulating a ColdFire processor).
 * Luckily, executing the code on a CF processor is still
 * harmless since all it does is clobber D1.
 */
static long _nf_detect_tos(void) {
	register long ret __asm__ ("d0");
	register const char *nf_version __asm__("a1") = "NF_VERSION";

	__asm__ volatile(
	"\tmove.l	%1,-(%%sp)\n"
	"\tmoveq	#0,%%d0\n"			/* assume no NatFeats available */
	"\tmove.l	%%d0,-(%%sp)\n"
	"\tlea		(1f:w,%%pc),%%a1\n"
	"\tmove.l	(0x0010).w,%%a0\n"	/* illegal instruction vector */
	"\tmove.l	%%a1,(0x0010).w\n"
	"\tmove.l	%%sp,%%a1\n"		/* save the ssp */

	"\tnop\n"						/* flush pipelines (for 68040+) */

	ASM_NATFEAT(NATFEAT_ID)			/* Jump to NATFEAT_ID */
	"\ttst.l	%%d0\n"
	"\tbeq.s	1f\n"
	"\tmoveq	#1,%%d0\n"			/* NatFeats detected */
	"\tmove.l	%%d0,(%%sp)\n"

"1:\n"
	"\tmove.l	%%a1,%%sp\n"
	"\tmove.l	%%a0,(0x0010).w\n"
	"\tmove.l	(%%sp)+,%%d0\n"
	"\taddq.l	#4,%%sp\n"			/* pop nf_version argument */

	"\tnop\n"						/* flush pipelines (for 68040+) */
	: "=g"(ret)  /* outputs */
	: "g"(nf_version)		/* inputs  */
	: __CLOBBER_RETURN("d0") "a0", "d1", "cc" AND_MEMORY
	);
	return ret;
}

long nf_stderr_id;

void nf_init(void) {
	long ret = Supexec(_nf_detect_tos);
	if (ret == 1)
		nf_stderr_id = _nf_get_id("NF_STDERR");
}

void nf_print(const char* msg) {
	if (nf_stderr_id)
		_nf_call(nf_stderr_id | 0, (uint32_t)msg);
}
