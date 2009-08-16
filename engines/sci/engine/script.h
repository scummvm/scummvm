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

#ifndef SCI_ENGINE_SCRIPT_H
#define SCI_ENGINE_SCRIPT_H

#include "common/str.h"

namespace Sci {

struct EngineState;
class ResourceManager;

#define SCI_SCRIPTS_NR 1000

enum ScriptObjectTypes {
	SCI_OBJ_TERMINATOR,
	SCI_OBJ_OBJECT,
	SCI_OBJ_CODE,
	SCI_OBJ_SYNONYMS,
	SCI_OBJ_SAID,
	SCI_OBJ_STRINGS,
	SCI_OBJ_CLASS,
	SCI_OBJ_EXPORTS,
	SCI_OBJ_POINTERS,
	SCI_OBJ_PRELOAD_TEXT, /* This is really just a flag. */
	SCI_OBJ_LOCALVARS
};

/* Opcode formats as used by script.c */
enum opcode_format {
	Script_Invalid = -1,
	Script_None = 0,
	Script_Byte,
	Script_SByte,
	Script_Word,
	Script_SWord,
	Script_Variable,
	Script_SVariable,
	Script_SRelative,
	Script_Property,
	Script_Global,
	Script_Local,
	Script_Temp,
	Script_Param,
	Script_Offset,
	Script_End
};

enum sci_opcodes { /* FIXME */
	op_bnot     = 0x00,	// 000
	op_add      = 0x01,	// 001
	op_sub      = 0x02,	// 002
	op_mul      = 0x03,	// 003
	op_div      = 0x04,	// 004
	op_mod      = 0x05,	// 005
	op_shr      = 0x06,	// 006
	op_shl      = 0x07,	// 007
	op_xor      = 0x08,	// 008
	op_and      = 0x09,	// 009
	op_or       = 0x0a,	// 010
	op_neg      = 0x0b,	// 011
	op_not      = 0x0c,	// 012
	op_eq       = 0x0d,	// 013
	op_ne_      = 0x0e,	// 014
	op_gt_      = 0x0f,	// 015
	op_ge_      = 0x10,	// 016
	op_lt_      = 0x11,	// 017
	op_le_      = 0x12,	// 018
	op_ugt_     = 0x13,	// 019
	op_uge_     = 0x14,	// 020
	op_ult_     = 0x15,	// 021
	op_ule_     = 0x16,	// 022
	op_bt       = 0x17,	// 023
	op_bnt      = 0x18,	// 024
	op_jmp      = 0x19,	// 025
	op_ldi      = 0x1a,	// 026
	op_push     = 0x1b,	// 027
	op_pushi    = 0x1c,	// 028
	op_toss     = 0x1d,	// 029
	op_dup      = 0x1e,	// 030
	op_link     = 0x1f,	// 031
	op_call     = 0x20,	// 032
	op_callk    = 0x21,	// 033
	op_callb    = 0x22,	// 034
	op_calle    = 0x23,	// 035
	op_ret      = 0x24,	// 036
	op_send     = 0x25,	// 037
	// dummy      0x26,	// 038
	// dummy      0x27,	// 039
	op_class    = 0x28,	// 040
	// dummy      0x29,	// 041
	op_self     = 0x2a,	// 042
	op_super    = 0x2b,	// 043
	op_rest     = 0x2c,	// 044
	op_lea      = 0x2d,	// 045
	op_selfID   = 0x2e,	// 046
	// dummy      0x2f	// 047
	op_pprev    = 0x30,	// 048
	op_pToa     = 0x31,	// 049
	op_aTop     = 0x32,	// 050
	op_pTos     = 0x33,	// 051
	op_sTop     = 0x34,	// 052
	op_ipToa    = 0x35,	// 053
	op_dpToa    = 0x36,	// 054
	op_ipTos    = 0x37,	// 055
	op_dpTos    = 0x38,	// 056
	op_lofsa    = 0x39,	// 057
	op_lofss    = 0x3a,	// 058
	op_push0    = 0x3b,	// 059
	op_push1    = 0x3c,	// 060
	op_push2    = 0x3d,	// 061
	op_pushSelf = 0x3e,	// 062
	// dummy      0x3f,	// 063
	op_lag      = 0x40,	// 064
	op_lal      = 0x41,	// 065
	op_lat      = 0x42,	// 066
	op_lap      = 0x43,	// 067
	op_lagi     = 0x44,	// 068
	op_lali     = 0x45,	// 069
	op_lati     = 0x46,	// 070
	op_lapi     = 0x47,	// 071
	op_lsg      = 0x48,	// 072
	op_lsl      = 0x49,	// 073
	op_lst      = 0x4a,	// 074
	op_lsp      = 0x4b,	// 075
	op_lsgi     = 0x4c,	// 076
	op_lsli     = 0x4d,	// 077
	op_lsti     = 0x4e,	// 078
	op_lspi     = 0x4f,	// 079
	op_sag      = 0x50,	// 080
	op_sal      = 0x51,	// 081
	op_sat      = 0x52,	// 082
	op_sap      = 0x53,	// 083
	op_sagi     = 0x54,	// 084
	op_sali     = 0x55,	// 085
	op_sati     = 0x56,	// 086
	op_sapi     = 0x57,	// 087
	op_ssg      = 0x58,	// 088
	op_ssl      = 0x59,	// 089
	op_sst      = 0x5a,	// 090
	op_ssp      = 0x5b,	// 091
	op_ssgi     = 0x5c,	// 092
	op_ssli     = 0x5d,	// 093
	op_ssti     = 0x5e,	// 094
	op_sspi     = 0x5f,	// 095
	op_plusag   = 0x60,	// 096
	op_plusal   = 0x61,	// 097
	op_plusat   = 0x62,	// 098
	op_plusap   = 0x63,	// 099
	op_plusagi  = 0x64,	// 100
	op_plusali  = 0x65,	// 101
	op_plusati  = 0x66,	// 102
	op_plusapi  = 0x67,	// 103
	op_plussg   = 0x68,	// 104
	op_plussl   = 0x69,	// 105
	op_plusst   = 0x6a,	// 106
	op_plussp   = 0x6b,	// 107
	op_plussgi  = 0x6c,	// 108
	op_plussli  = 0x6d,	// 109
	op_plussti  = 0x6e,	// 110
	op_plusspi  = 0x6f,	// 111
	op_minusag  = 0x70,	// 112
	op_minusal  = 0x71,	// 113
	op_minusat  = 0x72,	// 114
	op_minusap  = 0x73,	// 115
	op_minusagi = 0x74,	// 116
	op_minusali = 0x75,	// 117
	op_minusati = 0x76,	// 118
	op_minusapi = 0x77,	// 119
	op_minussg  = 0x78,	// 120
	op_minussl  = 0x79,	// 121
	op_minusst  = 0x7a,	// 122
	op_minussp  = 0x7b,	// 123
	op_minussgi = 0x7c,	// 124
	op_minussli = 0x7d,	// 125
	op_minussti = 0x7e,	// 126
	op_minusspi = 0x7f	// 127
};

extern opcode_format g_opcode_formats[128][4];

void script_adjust_opcode_formats(SciVersion version);

void script_free_breakpoints(EngineState *s);

} // End of namespace Sci

#endif // SCI_ENGINE_SCRIPT_H
