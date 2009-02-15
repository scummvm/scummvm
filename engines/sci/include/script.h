/***************************************************************************
 script.h Copyright (C) 2000,01 Magnus Reftel

 This program may be modified and copied freely according to the terms of
 the GNU general public license (GPL), as long as the above copyright
 notice and the licensing information contained herein are preserved.

 Please refer to www.gnu.org for licensing details.

 This work is provided AS IS, without warranty of any kind, expressed or
 implied, including but not limited to the warranties of merchantibility,
 noninfringement, and fitness for a specific purpose. The author will not
 be held liable for any damage caused by this work or derivatives of it.

 By using this source code, you agree to the licensing terms as stated
 above.

***************************************************************************/

#ifndef SCRIPT_H
#define SCRIPT_H

#include <sciresource.h>

/*#define SCRIPT_DEBUG */

#define SCI_SCRIPTS_NR 1000

typedef struct script_opcode_
{
  unsigned opcode;
  int arg1, arg2, arg3;
  int pos, size;
} script_opcode;


typedef enum {
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
} script_object_types;

void script_dissect(resource_mgr_t *resmgr, int res_no, char **snames, int snames_nr);

/* Opcode formats as used by script.c */
typedef enum {
  Script_Invalid=-1,
  Script_None=0,
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
} opcode_format;

typedef enum { /* FIXME */
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
} sci_opcodes;

extern DLLEXTERN opcode_format formats[128][4];

void script_adjust_opcode_formats(int res_version);

int
script_find_selector(struct _state *s, const char *selector_name);
/* Determines the selector ID of a selector by its name
** Parameters: (state_t *) s: VM state
**             (char *) selector_name: Name of the selector to look up
** Returns   : (int) The appropriate selector ID, or -1 on error
*/

struct _state;
void script_free_breakpoints(struct _state *s);

#endif
