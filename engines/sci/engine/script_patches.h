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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef SCI_ENGINE_SCRIPT_PATCHES_H
#define SCI_ENGINE_SCRIPT_PATCHES_H

#include "sci/sci.h"
#include "sci/engine/vm.h"	// for SciOpcodes

namespace Sci {

// Please do not use the #defines, that are called SIG_CODE_* / PATCH_CODE_* inside signature/patch-tables
#define SIG_END                      0xFFFF
#define SIG_MISMATCH                 0xFFFE
#define SIG_COMMANDMASK              0xF000
#define SIG_VALUEMASK                0x0FFF
#define SIG_BYTEMASK                 0x00FF
#define SIG_MAGICDWORD               0xF000
#define SIG_CODE_ADDTOOFFSET         0xE000
#define SIG_ADDTOOFFSET(_offset_)    SIG_CODE_ADDTOOFFSET | (_offset_)
#define SIG_CODE_SELECTOR16          0x9000
#define SIG_SELECTOR16(_selectorID_) SIG_CODE_SELECTOR16 | SELECTOR_##_selectorID_
#define SIG_CODE_SELECTOR8           0x8000
#define SIG_SELECTOR8(_selectorID_)  SIG_CODE_SELECTOR8 | SELECTOR_##_selectorID_
#define SIG_CODE_UINT16              0x1000
#define SIG_UINT16(_value_)          SIG_CODE_UINT16 | ((_value_) & 0xFF), ((_value_) >> 8)
#define SIG_CODE_BYTE                0x0000

#define PATCH_END                                              SIG_END
#define PATCH_COMMANDMASK                                      SIG_COMMANDMASK
#define PATCH_VALUEMASK                                        SIG_VALUEMASK
#define PATCH_BYTEMASK                                         SIG_BYTEMASK
#define PATCH_CODE_ADDTOOFFSET                                 SIG_CODE_ADDTOOFFSET
#define PATCH_ADDTOOFFSET(_offset_)                            SIG_CODE_ADDTOOFFSET | (_offset_)
#define PATCH_CODE_GETORIGINALBYTE                             0xC000
#define PATCH_GETORIGINALBYTE(_offset_)                        PATCH_CODE_GETORIGINALBYTE | (_offset_), 0
#define PATCH_GETORIGINALBYTEADJUST(_offset_, _adjustValue_)   PATCH_CODE_GETORIGINALBYTE | (_offset_), (uint16)(_adjustValue_)
#define PATCH_CODE_GETORIGINALUINT16                           0xD000
#define PATCH_GETORIGINALUINT16(_offset_)                      PATCH_CODE_GETORIGINALUINT16 | (_offset_), 0
#define PATCH_GETORIGINALUINT16ADJUST(_offset_, _adjustValue_) PATCH_CODE_GETORIGINALUINT16 | (_offset_), (uint16)(_adjustValue_)
#define PATCH_CODE_SELECTOR16                                  SIG_CODE_SELECTOR16
#define PATCH_SELECTOR16(_selectorID_)                         SIG_CODE_SELECTOR16 | SELECTOR_##_selectorID_
#define PATCH_CODE_SELECTOR8                                   SIG_CODE_SELECTOR8
#define PATCH_SELECTOR8(_selectorID_)                          SIG_CODE_SELECTOR8 | SELECTOR_##_selectorID_
#define PATCH_CODE_UINT16                                      SIG_CODE_UINT16
#define PATCH_UINT16(_value_)                                  SIG_CODE_UINT16 | ((_value_) & 0xFF), ((_value_) >> 8)
#define PATCH_CODE_BYTE                                        SIG_CODE_BYTE

// defines maximum scratch area for getting original bytes from unpatched script data
#define PATCH_VALUELIMIT      4096

struct SciScriptPatcherEntry {
	bool defaultActive;
	uint16 scriptNr;
	const char *description;
	int16 applyCount;
	const uint16 *signatureData;
	const uint16 *patchData;
};

#define SCI_SIGNATUREENTRY_TERMINATOR { false, 0, NULL, 0, NULL, NULL }

struct SciScriptPatcherRuntimeEntry {
	bool active;
	uint32 magicDWord;
	int magicOffset;
};

enum PatchOpcodes {
	OP_0x00_BNOT     = 2 * op_bnot     /* 0x00 */, OP_0x01_BNOT     /* 0x01 */,
	OP_0x02_ADD      = 2 * op_add      /* 0x02 */, OP_0x03_ADD      /* 0x03 */,
	OP_0x04_SUB      = 2 * op_sub      /* 0x04 */, OP_0x05_SUB      /* 0x05 */,
	OP_0x06_MUL      = 2 * op_mul      /* 0x06 */, OP_0x07_MUL      /* 0x07 */,
	OP_0x08_DIV      = 2 * op_div      /* 0x08 */, OP_0x09_DIV      /* 0x09 */,
	OP_0x0a_MOD      = 2 * op_mod      /* 0x0a */, OP_0x0b_MOD      /* 0x0b */,
	OP_0x0c_SHR      = 2 * op_shr      /* 0x0c */, OP_0x0d_SHR      /* 0x0d */,
	OP_0x0e_SHL      = 2 * op_shl      /* 0x0e */, OP_0x0f_SHL      /* 0x0f */,
	OP_0x10_XOR      = 2 * op_xor      /* 0x10 */, OP_0x11_XOR      /* 0x11 */,
	OP_0x12_AND      = 2 * op_and      /* 0x12 */, OP_0x13_AND      /* 0x13 */,
	OP_0x14_OR       = 2 * op_or       /* 0x14 */, OP_0x15_OR       /* 0x15 */,
	OP_0x16_NEG      = 2 * op_neg      /* 0x16 */, OP_0x17_NEG      /* 0x17 */,
	OP_0x18_NOT      = 2 * op_not      /* 0x18 */, OP_0x19_NOT      /* 0x19 */,
	OP_0x1a_EQ_      = 2 * op_eq_      /* 0x1a */, OP_0x1b_EQ_      /* 0x1b */,
	OP_0x1c_NE_      = 2 * op_ne_      /* 0x1c */, OP_0x1d_NE_      /* 0x1d */,
	OP_0x1e_GT_      = 2 * op_gt_      /* 0x1e */, OP_0x1f_GT_      /* 0x1f */,
	OP_0x20_GE_      = 2 * op_ge_      /* 0x20 */, OP_0x21_GE_      /* 0x21 */,
	OP_0x22_LT_      = 2 * op_lt_      /* 0x22 */, OP_0x23_LT_      /* 0x23 */,
	OP_0x24_LE_      = 2 * op_le_      /* 0x24 */, OP_0x25_LE_      /* 0x25 */,
	OP_0x26_UGT_     = 2 * op_ugt_     /* 0x26 */, OP_0x27_UGT_     /* 0x27 */,
	OP_0x28_UGE_     = 2 * op_uge_     /* 0x28 */, OP_0x29_UGE_     /* 0x29 */,
	OP_0x2a_ULT_     = 2 * op_ult_     /* 0x2a */, OP_0x2b_ULT_     /* 0x2b */,
	OP_0x2c_ULE_     = 2 * op_ule_     /* 0x2c */, OP_0x2d_ULE_     /* 0x2d */,
	OP_0x2e_BT       = 2 * op_bt       /* 0x2e */, OP_0x2f_BT       /* 0x2f */,
	OP_0x30_BNT      = 2 * op_bnt      /* 0x30 */, OP_0x31_BNT      /* 0x31 */,
	OP_0x32_JMP      = 2 * op_jmp      /* 0x32 */, OP_0x33_JMP      /* 0x33 */,
	OP_0x34_LDI      = 2 * op_ldi      /* 0x34 */, OP_0x35_LDI      /* 0x35 */,
	OP_0x36_PUSH     = 2 * op_push     /* 0x36 */, OP_0x37_PUSH     /* 0x37 */,
	OP_0x38_PUSHI    = 2 * op_pushi    /* 0x38 */, OP_0x39_PUSHI    /* 0x39 */,
	OP_0x3a_TOSS     = 2 * op_toss     /* 0x3a */, OP_0x3b_TOSS     /* 0x3b */,
	OP_0x3c_DUP      = 2 * op_dup      /* 0x3c */, OP_0x3d_DUP      /* 0x3d */,
	OP_0x3e_LINK     = 2 * op_link     /* 0x3e */, OP_0x3f_LINK     /* 0x3f */,
	OP_0x40_CALL     = 2 * op_call     /* 0x40 */, OP_0x41_CALL     /* 0x41 */,
	OP_0x42_CALLK    = 2 * op_callk    /* 0x42 */, OP_0x43_CALLK    /* 0x43 */,
	OP_0x44_CALLB    = 2 * op_callb    /* 0x44 */, OP_0x45_CALLB    /* 0x45 */,
	OP_0x46_CALLE    = 2 * op_calle    /* 0x46 */, OP_0x47_CALLE    /* 0x47 */,
	OP_0x48_RET      = 2 * op_ret      /* 0x48 */, OP_0x49_RET      /* 0x49 */,
	OP_0x4a_SEND     = 2 * op_send     /* 0x4a */, OP_0x4b_SEND     /* 0x4b */,
	OP_0x4c_INFO     = 2 * op_info     /* 0x4c */, OP_0x4d_INFO     /* 0x4d */,
	OP_0x4e_SUPERP   = 2 * op_superP   /* 0x4e */, OP_0x4f_SUPERP   /* 0x4f */,
	OP_0x50_CLASS    = 2 * op_class    /* 0x50 */, OP_0x51_CLASS    /* 0x51 */,
	// DUMMY                           /* 0x52 */,              /* 0x53 */,
	OP_0x54_SELF     = 2 * op_self     /* 0x54 */, OP_0x55_SELF     /* 0x55 */,
	OP_0x56_SUPER    = 2 * op_super    /* 0x56 */, OP_0x57_SUPER    /* 0x57 */,
	OP_0x58_REST     = 2 * op_rest     /* 0x58 */, OP_0x59_REST     /* 0x59 */,
	OP_0x5a_LEA      = 2 * op_lea      /* 0x5a */, OP_0x5b_LEA      /* 0x5b */,
	OP_0x5c_SELFID   = 2 * op_selfID   /* 0x5c */, OP_0x5d_SELFID   /* 0x5d */,
	// DUMMY                           /* 0x5e */,                  /* 0x5f */,
	OP_0x60_PPREV    = 2 * op_pprev    /* 0x60 */, OP_0x61_PPREV    /* 0x61 */,
	OP_0x62_PTOA     = 2 * op_pToa     /* 0x62 */, OP_0x63_PTOA     /* 0x63 */,
	OP_0x64_ATOP     = 2 * op_aTop     /* 0x64 */, OP_0x65_ATOP     /* 0x65 */,
	OP_0x66_PTOS     = 2 * op_pTos     /* 0x66 */, OP_0x67_PTOS     /* 0x67 */,
	OP_0x68_STOP     = 2 * op_sTop     /* 0x68 */, OP_0x69_STOP     /* 0x69 */,
	OP_0x6a_IPTOA    = 2 * op_ipToa    /* 0x6a */, OP_0x6b_IPTOA    /* 0x6b */,
	OP_0x6c_DPTOA    = 2 * op_dpToa    /* 0x6c */, OP_0x6d_DPTOA    /* 0x6d */,
	OP_0x6e_IPTOS    = 2 * op_ipTos    /* 0x6e */, OP_0x6f_IPTOS    /* 0x6f */,
	OP_0x70_DPTOS    = 2 * op_dpTos    /* 0x70 */, OP_0x71_DPTOS    /* 0x71 */,
	OP_0x72_LOFSA    = 2 * op_lofsa    /* 0x72 */, OP_0x73_LOFSA    /* 0x73 */,
	OP_0x74_LOFSS    = 2 * op_lofss    /* 0x74 */, OP_0x75_LOFSS    /* 0x75 */,
	OP_0x76_PUSH0    = 2 * op_push0    /* 0x76 */, OP_0x77_PUSH0    /* 0x77 */,
	OP_0x78_PUSH1    = 2 * op_push1    /* 0x78 */, OP_0x79_PUSH1    /* 0x79 */,
	OP_0x7a_PUSH2    = 2 * op_push2    /* 0x7a */, OP_0x7b_PUSH2    /* 0x7b */,
	OP_0x7c_PUSHSELF = 2 * op_pushSelf /* 0x7c */, OP_0x7d_PUSHSELF /* 0x7d */,
	OP_0x7e_LINE     = 2 * op_line     /* 0x7e */, OP_0x7f_LINE     /* 0x7f */,
	OP_0x80_LAG      = 2 * op_lag      /* 0x80 */, OP_0x81_LAG      /* 0x81 */,
	OP_0x82_LAL      = 2 * op_lal      /* 0x82 */, OP_0x83_LAL      /* 0x83 */,
	OP_0x84_LAT      = 2 * op_lat      /* 0x84 */, OP_0x85_LAT      /* 0x85 */,
	OP_0x86_LAP      = 2 * op_lap      /* 0x86 */, OP_0x87_LAP      /* 0x87 */,
	OP_0x88_LSG      = 2 * op_lsg      /* 0x88 */, OP_0x89_LSG      /* 0x89 */,
	OP_0x8a_LSL      = 2 * op_lsl      /* 0x8a */, OP_0x8b_LSL      /* 0x8b */,
	OP_0x8c_LST      = 2 * op_lst      /* 0x8c */, OP_0x8d_LST      /* 0x8d */,
	OP_0x8e_LSP      = 2 * op_lsp      /* 0x8e */, OP_0x8f_LSP      /* 0x8f */,
	OP_0x90_LAGI     = 2 * op_lagi     /* 0x90 */, OP_0x91_LAGI     /* 0x91 */,
	OP_0x92_LALI     = 2 * op_lali     /* 0x92 */, OP_0x93_LALI     /* 0x93 */,
	OP_0x94_LATI     = 2 * op_lati     /* 0x94 */, OP_0x95_LATI     /* 0x95 */,
	OP_0x96_LAPI     = 2 * op_lapi     /* 0x96 */, OP_0x97_LAPI     /* 0x97 */,
	OP_0x98_LSGI     = 2 * op_lsgi     /* 0x98 */, OP_0x99_LSGI     /* 0x99 */,
	OP_0x9a_LSLI     = 2 * op_lsli     /* 0x9a */, OP_0x9b_LSLI     /* 0x9b */,
	OP_0x9c_LSTI     = 2 * op_lsti     /* 0x9c */, OP_0x9d_LSTI     /* 0x9d */,
	OP_0x9e_LSPI     = 2 * op_lspi     /* 0x9e */, OP_0x9f_LSPI     /* 0x9f */,
	OP_0xa0_SAG      = 2 * op_sag      /* 0xa0 */, OP_0xa1_SAG      /* 0xa1 */,
	OP_0xa2_SAL      = 2 * op_sal      /* 0xa2 */, OP_0xa3_SAL      /* 0xa3 */,
	OP_0xa4_SAT      = 2 * op_sat      /* 0xa4 */, OP_0xa5_SAT      /* 0xa5 */,
	OP_0xa6_SAP      = 2 * op_sap      /* 0xa6 */, OP_0xa7_SAP      /* 0xa7 */,
	OP_0xa8_SSG      = 2 * op_ssg      /* 0xa8 */, OP_0xa9_SSG      /* 0xa9 */,
	OP_0xaa_SSL      = 2 * op_ssl      /* 0xaa */, OP_0xab_SSL      /* 0xab */,
	OP_0xac_SST      = 2 * op_sst      /* 0xac */, OP_0xad_SST      /* 0xad */,
	OP_0xae_SSP      = 2 * op_ssp      /* 0xae */, OP_0xaf_SSP      /* 0xaf */,
	OP_0xb0_SAGI     = 2 * op_sagi     /* 0xb0 */, OP_0xb1_SAGI     /* 0xb1 */,
	OP_0xb2_SALI     = 2 * op_sali     /* 0xb2 */, OP_0xb3_SALI     /* 0xb3 */,
	OP_0xb4_SATI     = 2 * op_sati     /* 0xb4 */, OP_0xb5_SATI     /* 0xb5 */,
	OP_0xb6_SAPI     = 2 * op_sapi     /* 0xb6 */, OP_0xb7_SAPI     /* 0xb7 */,
	OP_0xb8_SSGI     = 2 * op_ssgi     /* 0xb8 */, OP_0xb9_SSGI     /* 0xb9 */,
	OP_0xba_SSLI     = 2 * op_ssli     /* 0xba */, OP_0xbb_SSLI     /* 0xbb */,
	OP_0xbc_SSTI     = 2 * op_ssti     /* 0xbc */, OP_0xbd_SSTI     /* 0xbd */,
	OP_0xbe_SSPI     = 2 * op_sspi     /* 0xbe */, OP_0xbf_SSPI     /* 0xbf */,
	OP_0xc0_PLUSAG   = 2 * op_plusag   /* 0xc0 */, OP_0xc1_PLUSAG   /* 0xc1 */,
	OP_0xc2_PLUSAL   = 2 * op_plusal   /* 0xc2 */, OP_0xc3_PLUSAL   /* 0xc3 */,
	OP_0xc4_PLUSAT   = 2 * op_plusat   /* 0xc4 */, OP_0xc5_PLUSAT   /* 0xc5 */,
	OP_0xc6_PLUSAP   = 2 * op_plusap   /* 0xc6 */, OP_0xc7_PLUSAP   /* 0xc7 */,
	OP_0xc8_PLUSSG   = 2 * op_plussg   /* 0xc8 */, OP_0xc9_PLUSSG   /* 0xc9 */,
	OP_0xca_PLUSSL   = 2 * op_plussl   /* 0xca */, OP_0xcb_PLUSSL   /* 0xcb */,
	OP_0xcc_PLUSST   = 2 * op_plusst   /* 0xcc */, OP_0xcd_PLUSST   /* 0xcd */,
	OP_0xce_PLUSSP   = 2 * op_plussp   /* 0xce */, OP_0xcf_PLUSSP   /* 0xcf */,
	OP_0xd0_PLUSAGI  = 2 * op_plusagi  /* 0xd0 */, OP_0xd1_PLUSAGI  /* 0xd1 */,
	OP_0xd2_PLUSALI  = 2 * op_plusali  /* 0xd2 */, OP_0xd3_PLUSALI  /* 0xd3 */,
	OP_0xd4_PLUSATI  = 2 * op_plusati  /* 0xd4 */, OP_0xd5_PLUSATI  /* 0xd5 */,
	OP_0xd6_PLUSAPI  = 2 * op_plusapi  /* 0xd6 */, OP_0xd7_PLUSAPI  /* 0xd7 */,
	OP_0xd8_PLUSSGI  = 2 * op_plussgi  /* 0xd8 */, OP_0xd9_PLUSSGI  /* 0xd9 */,
	OP_0xda_PLUSSLI  = 2 * op_plussli  /* 0xda */, OP_0xdb_PLUSSLI  /* 0xdb */,
	OP_0xdc_PLUSSTI  = 2 * op_plussti  /* 0xdc */, OP_0xdd_PLUSSTI  /* 0xdd */,
	OP_0xde_PLUSSPI  = 2 * op_plusspi  /* 0xde */, OP_0xdf_PLUSSPI  /* 0xdf */,
	OP_0xe0_MINUSAG  = 2 * op_minusag  /* 0xe0 */, OP_0xe1_MINUSAG  /* 0xe1 */,
	OP_0xe2_MINUSAL  = 2 * op_minusal  /* 0xe2 */, OP_0xe3_MINUSAL  /* 0xe3 */,
	OP_0xe4_MINUSAT  = 2 * op_minusat  /* 0xe4 */, OP_0xe5_MINUSAT  /* 0xe5 */,
	OP_0xe6_MINUSAP  = 2 * op_minusap  /* 0xe6 */, OP_0xe7_MINUSAP  /* 0xe7 */,
	OP_0xe8_MINUSSG  = 2 * op_minussg  /* 0xe8 */, OP_0xe9_MINUSSG  /* 0xe9 */,
	OP_0xea_MINUSSL  = 2 * op_minussl  /* 0xea */, OP_0xeb_MINUSSL  /* 0xeb */,
	OP_0xec_MINUSST  = 2 * op_minusst  /* 0xec */, OP_0xed_MINUSST  /* 0xed */,
	OP_0xee_MINUSSP  = 2 * op_minussp  /* 0xee */, OP_0xef_MINUSSP  /* 0xef */,
	OP_0xf0_MINUSAGI = 2 * op_minusagi /* 0xf0 */, OP_0xf1_MINUSAGI /* 0xf1 */,
	OP_0xf2_MINUSALI = 2 * op_minusali /* 0xf2 */, OP_0xf3_MINUSALI /* 0xf3 */,
	OP_0xf4_MINUSATI = 2 * op_minusati /* 0xf4 */, OP_0xf5_MINUSATI /* 0xf5 */,
	OP_0xf6_MINUSAPI = 2 * op_minusapi /* 0xf6 */, OP_0xf7_MINUSAPI /* 0xf7 */,
	OP_0xf8_MINUSSGI = 2 * op_minussgi /* 0xf8 */, OP_0xf9_MINUSSGI /* 0xf9 */,
	OP_0xfa_MINUSSLI = 2 * op_minussli /* 0xfa */, OP_0xfb_MINUSSLI /* 0xfb */,
	OP_0xfc_MINUSSTI = 2 * op_minussti /* 0xfc */, OP_0xfd_MINUSSTI /* 0xfd */,
	OP_0xfe_MINUSSPI = 2 * op_minusspi /* 0xfe */, OP_0xff_MINUSSPI /* 0xff */,
};

/**
 * ScriptPatcher class, handles on-the-fly patching of script data
 */
class ScriptPatcher {
public:
	ScriptPatcher();
	~ScriptPatcher();

	// Calculates the magic DWord for fast search and verifies signature/patch data
	// Returns the magic DWord in platform-specific byte-order. This is done on purpose for performance.
	void calculateMagicDWordAndVerify(const char *signatureDescription, const uint16 *signatureData, bool magicDWordIncluded, uint32 &calculatedMagicDWord, int &calculatedMagicDWordOffset);

	// Called when a script is loaded to check for signature matches and apply patches in such cases
	void processScript(uint16 scriptNr, SciSpan<byte> scriptData);

	// Verifies, if a given signature matches the given script data (pointed to by additional byte offset)
	bool verifySignature(uint32 byteOffset, const uint16 *signatureData, const char *signatureDescription, const SciSpan<const byte> &scriptData);

	// searches for a given signature inside script data
	// returns -1 in case it was not found or an offset to the matching data
	int32 findSignature(uint32 magicDWord, int magicOffset, const uint16 *signatureData, const char *patchDescription, const SciSpan<const byte> &scriptData);

private:
	// Initializes a patch table and creates run time information for it (for enabling/disabling), also calculates magic DWORD)
	void initSignature(const SciScriptPatcherEntry *patchTable);

	// Enables a patch inside the patch table (used for optional patches like CD+Text support for KQ6 & LB2)
	void enablePatch(const SciScriptPatcherEntry *patchTable, const char *searchDescription);

	// Searches for a given signature entry inside script data
	// returns -1 in case it was not found or an offset to the matching data
	int32 findSignature(const SciScriptPatcherEntry *patchEntry, const SciScriptPatcherRuntimeEntry *runtimeEntry, const SciSpan<const byte> &scriptData);

	// Applies a patch to a given script + offset (overwrites parts)
	void applyPatch(const SciScriptPatcherEntry *patchEntry, SciSpan<byte> scriptData, int32 signatureOffset);

	Selector *_selectorIdTable;
	SciScriptPatcherRuntimeEntry *_runtimeTable;
	bool _isMacSci11;
};

} // End of namespace Sci

#endif // SCI_ENGINE_WORKAROUNDS_H
