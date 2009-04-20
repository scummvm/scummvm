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

/*#define SCRIPT_DEBUG */

#define SCI_SCRIPTS_NR 1000

#if 0
// Unreferenced
struct script_opcode {
	unsigned opcode;
	int arg1, arg2, arg3;
	int pos, size;
};
#endif


enum script_object_types {
	sci_obj_terminator,
	sci_obj_object,
	sci_obj_code,
	sci_obj_synonyms,
	sci_obj_said,
	sci_obj_strings,
	sci_obj_class,
	sci_obj_exports,
	sci_obj_pointers,
	sci_obj_preload_text, /* This is really just a flag. */
	sci_obj_localvars
};

void script_dissect(ResourceManager *resmgr, int res_no, const Common::StringList &selectorNames);

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
	op_bnot = 0,
	op_add,
	op_sub,
	op_mul,
	op_div,
	op_mod,
	op_shr,
	op_shl,
	op_xor,
	op_and,
	op_or,
	op_neg,
	op_not,
	op_eq,
	op_ne_,
	op_gt_,
	op_ge_,
	op_lt_,
	op_le_,
	op_ugt_,
	op_uge_,
	op_ult_,
	op_ule_,
	op_bt,
	op_bnt,
	op_jmp,
	op_ldi,
	op_push,
	op_pushi,
	op_toss,
	op_dup,
	op_link,
	op_call = 0x20,
	op_callk,
	op_callb,
	op_calle,
	op_ret,
	op_send,
	op_class = 0x28,
	op_self = 0x2a,
	op_super,
	op_rest,
	op_lea,
	op_selfID,
	op_pprev = 0x30,
	op_pToa,
	op_aTop,
	op_pTos,
	op_sTop,
	op_ipToa,
	op_dpToa,
	op_ipTos,
	op_dpTos,
	op_lofsa,
	op_lofss,
	op_push0,
	op_push1,
	op_push2,
	op_pushSelf,
	op_lag = 0x40,
	op_lal,
	op_lat,
	op_lap,
	op_lagi,
	op_lali,
	op_lati,
	op_lapi,
	op_lsg,
	op_lsl,
	op_lst,
	op_lsp,
	op_lsgi,
	op_lsli,
	op_lsti,
	op_lspi,
	op_sag,
	op_sal,
	op_sat,
	op_sap,
	op_sagi,
	op_sali,
	op_sati,
	op_sapi,
	op_ssg,
	op_ssl,
	op_sst,
	op_ssp,
	op_ssgi,
	op_ssli,
	op_ssti,
	op_sspi,
	op_plusag,
	op_plusal,
	op_plusat,
	op_plusap,
	op_plusagi,
	op_plusali,
	op_plusati,
	op_plusapi,
	op_plussg,
	op_plussl,
	op_plusst,
	op_plussp,
	op_plussgi,
	op_plussli,
	op_plussti,
	op_plusspi,
	op_minusag,
	op_minusal,
	op_minusat,
	op_minusap,
	op_minusagi,
	op_minusali,
	op_minusati,
	op_minusapi,
	op_minussg,
	op_minussl,
	op_minusst,
	op_minussp,
	op_minussgi,
	op_minussli,
	op_minussti,
	op_minusspi
};

extern opcode_format g_opcode_formats[128][4];

void script_adjust_opcode_formats(int res_version);

int script_find_selector(EngineState *s, const char *selector_name);
/* Determines the selector ID of a selector by its name
** Parameters: (state_t *) s: VM state
**             (char *) selector_name: Name of the selector to look up
** Returns   : (int) The appropriate selector ID, or -1 on error
*/

void script_free_breakpoints(EngineState *s);

} // End of namespace Sci

#endif // SCI_ENGINE_SCRIPT_H
