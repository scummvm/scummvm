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
 */

#include <stdio.h>
#include <string.h>
#include "common/scummsys.h"
#include "trecision/nl/3d/3dinc.h"
#include "trecision/nl/sysdef.h"
#include "trecision/nl/struct.h"
#include "trecision/nl/ll/llinc.h"
#include "trecision/nl/message.h"
#include "trecision/nl/extern.h"
#include "trecision/nl/define.h"
#include "trecision/trecision.h"

namespace Trecision {

// puzzle pannello 35
uint16 Comb35[7], Count35 = 0;
// puzzle meridiana 49
uint16 Comb49[4];
// puzzle meridiana 4CT
uint16 Comb4CT[6];
// puzzle tastiera 58
uint16 Comb58[6], Count58 = 0;
// SlotMachine41
uint16 Try41;
// CloseUp12 e 13
uint16 CloseUpObj;

/*-------------------------------------------------------------------------*/
/*                                 ROOMIN             					   */
/*-------------------------------------------------------------------------*/
void doRoomIn(uint16 curObj) {
	FlagMouseEnabled = false;

	uint16 theAction = g_vm->_obj[curObj]._anim;
	uint16 thePos = g_vm->_obj[curObj]._ninv;

	doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, g_vm->_obj[curObj]._goRoom, theAction, thePos, curObj);

	g_vm->_obj[curObj]._flag |= OBJFLAG_DONE;
}

/*-------------------------------------------------------------------------*/
/*                                 ROOMOUT         						   */
/*-------------------------------------------------------------------------*/
void doRoomOut(uint16 TheObj) {
	uint16 TheAction, ThePos;

	FlagMouseEnabled = false;

	switch (TheObj) {
	case oSCALA32:
		if (g_vm->_obj[oBOTOLAC32]._mode & OBJMODE_OBJSTATUS) {
			CharacterSay(g_vm->_obj[TheObj]._action);
			FlagMouseEnabled = true;
			TheAction = 0;
			ThePos = 0;
		} else {
			TheAction = g_vm->_obj[TheObj]._anim;
			ThePos = g_vm->_obj[TheObj]._ninv;
		}
		break;

	default:
		TheAction = g_vm->_obj[TheObj]._anim;
		ThePos = g_vm->_obj[TheObj]._ninv;
		break;
	}

	if (TheAction)
		doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, TheAction, g_vm->_obj[TheObj]._goRoom, ThePos, TheObj);

	g_vm->_obj[TheObj]._flag |= OBJFLAG_DONE;
}

/*-------------------------------------------------------------------------*/
/*                                 EXAMINE           					   */
/*-------------------------------------------------------------------------*/
void doMouseExamine(uint16 TheObj) {
	bool printsent = false;

	if (!TheObj)
		warning("doMouseExamine");

	switch (TheObj) {
	case oMAPPA12:
		doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a1211OSSERVAMAPPAPALAZZO, 0, 0, TheObj);
		_choice[4]._flag &= ~DLGCHOICE_HIDE;
		_choice[18]._flag &= ~DLGCHOICE_HIDE;
		_choice[33]._flag &= ~DLGCHOICE_HIDE;
		printsent = false;
		break;

	case oPORTAA13:
		g_vm->_obj[oBOX12]._mode |= OBJMODE_OBJSTATUS;
		printsent = true;
		break;

	case oPANELA12:
	case oFUSIBILE12:
		if (g_vm->_obj[oFUSIBILE12]._mode & OBJMODE_OBJSTATUS)
			g_vm->_obj[oFUSE12CU]._mode |= OBJMODE_OBJSTATUS;
		else
			g_vm->_obj[oFUSE12CU]._mode &= ~OBJMODE_OBJSTATUS;
		doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, r12CU, 0, 0, TheObj);
		CloseUpObj = TheObj;
		break;

	case oLETTERA13:
	case oPENPADA13:
		if (g_vm->_obj[oLETTERA13]._mode & OBJMODE_OBJSTATUS)
			g_vm->_obj[oLETTER13CU]._mode |= OBJMODE_OBJSTATUS;
		else
			g_vm->_obj[oLETTER13CU]._mode &= ~OBJMODE_OBJSTATUS;
		doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, r13CU, 0, 0, TheObj);
		CloseUpObj = TheObj;
		break;

	case oCUCININO14:
		doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a1413OSSERVAPIANOCOTTURA, 0, 0, TheObj);
		printsent = false;
		break;

	case oSCAFFALE14:
		if (!(g_vm->_obj[oPORTAR14]._mode & OBJMODE_OBJSTATUS))
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a145ESAMINASCAFFALE, 0, 0, TheObj);
		else
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a145CESAMINASCAFFALE, 0, 0, TheObj);
		printsent = false;
		break;

	case oTAVOLINOPP14:
		doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a1412SPOSTASEDIA, 0, 0, TheObj);
		printsent = false;
		break;

	case oBOCCETTE15:
		doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a1510TOCCABOCCETTE, 0, 0, TheObj);
		printsent = false;
		break;

	case oSPECCHIO15:
		doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a1511SISPECCHIA, 0, 0, TheObj);
		printsent = false;
		break;

	case oMONITORSA16:
		doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a1614GUARDAMONITORS14, 0, 0, TheObj);
		printsent = false;
		break;

	case oFINESTRAA15:
		if (g_vm->_obj[oTAPPARELLAA15]._mode & OBJMODE_OBJSTATUS)
			CharacterSay(1999);
		else
			printsent = true;
		break;

	case oMONITORSP16:
		doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a1615GUARDAMONITORS15, 0, 0, TheObj);
		printsent = false;
		break;

	case oCARTACCE16:
		doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a1613GUARDACARTACCE, 0, 0, TheObj);
		printsent = false;
		break;

	case oMAPPA16:
		if (IconPos(iFOGLIO14) != MAXICON)
			g_vm->_obj[oMAPPA16]._flag |= OBJFLAG_EXTRA;
		printsent = true;
		break;

	case oSCATOLONE17:
		doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a177SICHINA, 0, 0, TheObj);
		printsent = false;
		break;

	case oMURALES17:
		doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a179MUOVETESTA, 0, 0, TheObj);
		printsent = false;
		break;

	case oSCHERMO18:
		doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a181ESAMINACARTELLONE, 0, 0, TheObj);
		printsent = false;
		break;

	case oVETRINA1A:
		doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a1A1ESAMINAVETRINA, 0, 0, TheObj);
		printsent = false;
		break;

	case oTESSERA1A:
		if (((_choice[151]._flag & OBJFLAG_DONE) || (_choice[152]._flag & OBJFLAG_DONE)) && !(_choice[183]._flag & OBJFLAG_DONE))
			_choice[183]._flag &= ~DLGCHOICE_HIDE;
		g_vm->_obj[oTESSERA1A]._flag |= OBJFLAG_EXTRA;
		printsent = true;
		break;

	case oCARTACCE1B:
		doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a1B8FRUGACARTACCIE, 0, 0, TheObj);
		printsent = false;
		break;

	case oBIDONE1B:
		doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a1B10GUARDABIDONE, 0, 0, TheObj);
		printsent = false;
		break;

	case oGRATA1C:
		doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a1C2GUARDAGRATA, 0, 0, TheObj);
		printsent = false;
		break;

	case oSCAFFALE1D:
		doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a1D8SALESGABELLO, 0, 0, TheObj);
		printsent = false;
		break;

	case oBARILOTTO1D:
		doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a1D7SPINGEBARILOTTO, 0, 0, TheObj);
		printsent = false;
		break;

	case oCASSA1D:
		doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a1D10ESAMINABOTTIGLIE, 0, 0, TheObj);
		printsent = false;
		break;

	case oSCATOLETTA23:
		doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a2310, 0, 0, TheObj);
		printsent = false;
		break;

	case oPALMA26:
		doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a262, 0, 0, TheObj);
		printsent = false;
		break;

	case oINSEGNA26:
		doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a263, 0, 0, TheObj);
		printsent = false;
		break;

	case oTEMPIO28:
		doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a2810, 0, 0, TheObj);
		printsent = false;
		break;

	case oSERPENTET28:
	case oSERPENTEA28:
		doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a2811, 0, 0, TheObj);
		printsent = false;
		break;

	case oSERPENTE2B:
		doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a2B11ESAMINASERPENTE, 0, 0, TheObj);
		printsent = false;
		break;

	case oLEOPARDO2B:
		doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a2B9ESAMINALEOPARDO, 0, 0, TheObj);
		printsent = false;
		break;

	case oPELLICANO2B:
		doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a2B10ESAMINAPELLICANO, 0, 0, TheObj);
		printsent = false;
		break;

	case oBACHECA2B:
		doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a2B13OSSERVAREFARFALLE, 0, 0, TheObj);
		printsent = false;
		break;

	case oROBOT2F:
		doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a2F6TOCCADINOSAURO, 0, 0, TheObj);
		printsent = false;
		break;

	case oCREPACCIO2E:
		if (g_vm->_room[r2E]._flag & OBJFLAG_EXTRA)
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a2E7GUARDACREPACCIODILA, 0, 0, TheObj);
		else
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a2E6GUARDACREPACCIODIQUA, 0, 0, TheObj);
		printsent = false;
		break;

	case oGENERATORE34:
		doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a347ESAMINAGENERATORE, 0, 0, TheObj);
		printsent = false;
		break;

	case oTUBOF33:
		if ((g_vm->_obj[oVALVOLA34]._mode & OBJMODE_OBJSTATUS) && (g_vm->_obj[oVALVOLA34]._anim))
			CharacterSay(2000);
		else
			CharacterSay(g_vm->_obj[TheObj]._examine);
		printsent = false;
		break;

	case oTUBOT33:
		if ((g_vm->_obj[oVALVOLA34]._mode & OBJMODE_OBJSTATUS) && (g_vm->_obj[oVALVOLA34]._anim))
			CharacterSay(2001);
		else
			CharacterSay(g_vm->_obj[TheObj]._examine);
		printsent = false;
		break;

	case oTUBOA34:
		if ((g_vm->_obj[oVALVOLA34]._mode & OBJMODE_OBJSTATUS) && (g_vm->_obj[oVALVOLA34]._anim))
			CharacterSay(2002);
		else
			CharacterSay(g_vm->_obj[TheObj]._examine);
		printsent = false;
		break;

	case oTUBOF34:
		if ((g_vm->_obj[oVALVOLA34]._mode & OBJMODE_OBJSTATUS) && (g_vm->_obj[oVALVOLA34]._anim))
			CharacterSay(2000);
		else
			CharacterSay(g_vm->_obj[TheObj]._examine);
		printsent = false;
		break;

	case oTUBOFT34:
		if ((g_vm->_obj[oVALVOLA34]._mode & OBJMODE_OBJSTATUS) && (g_vm->_obj[oVALVOLA34]._anim))
			CharacterSay(2001);
		else
			CharacterSay(g_vm->_obj[TheObj]._examine);
		printsent = false;
		break;

	case oCASSE35:
		doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a3522ESAMINACASSE, 0, 0, TheObj);
		printsent = false;
		break;

	case oSCAFFALE35:
		if (g_vm->_room[r35]._flag & OBJFLAG_EXTRA)
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a3517ESAMINACIANFRUSAGLIE, 0, 0, TheObj);
		else
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a3517AESAMINACIANFRUSAGLIE, 0, 0, TheObj);
		printsent = false;
		break;

	case oGIORNALE35:
		if (g_vm->_room[r35]._flag & OBJFLAG_EXTRA) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a3521LEGGEGIORNALE, 0, 0, TheObj);
			printsent = false;
		}
		break;

	case oSCAFFALE36:
		doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, g_vm->_obj[oSCAFFALE36]._anim, 0, 0, TheObj);
		printsent = false;
		break;

	case oFESSURA41:
		doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a411, 0, 0, TheObj);
		printsent = false;
		break;

	case oCARTELLOV42:
		doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a424, 0, 0, TheObj);
		printsent = false;
		break;

	case oCARTELLOF42:
		doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a426, 0, 0, TheObj);
		printsent = false;
		break;

	case oCAMPANAT43:
		if (g_vm->_obj[oMARTELLOR43]._mode & OBJMODE_OBJSTATUS)
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a431R, 0, 0, TheObj);
		else
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a431, 0, 0, TheObj);
		printsent = false;
		break;

	case oTAMBURO43:
		if (g_vm->_obj[oMARTELLOR43]._mode & OBJMODE_OBJSTATUS)
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a432R, 0, 0, TheObj);
		else
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a432, 0, 0, TheObj);
		printsent = false;
		break;

	case oRAGNATELA45:
		doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a451, 0, 0, TheObj);
		printsent = false;
		break;

	case oQUADROS4A:
		doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a4A5, 0, 0, TheObj);
		printsent = false;
		break;

	case oCARTELLO55:
		doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a5511, 0, 0, TheObj);
		printsent = false;
		break;

	case oEXIT12CU:
		doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, g_vm->_obj[oEXIT12CU]._goRoom, 0, 0, TheObj);
		break;

	case oEXIT13CU:
		doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, g_vm->_obj[oEXIT13CU]._goRoom, 0, 0, TheObj);
		break;

	case oEXIT2BL:
		doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, g_vm->_obj[oEXIT2BL]._goRoom, 0, 0, TheObj);
		break;

	case oEXIT36F:
		doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, g_vm->_obj[oEXIT36F]._goRoom, 0, 0, TheObj);
		break;

	case oEXIT41D:
		doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, g_vm->_obj[oEXIT41D]._goRoom, 0, 0, TheObj);
		break;

	case oEXIT4CT:
		doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, g_vm->_obj[oEXIT4CT]._goRoom, 0, 0, TheObj);
		break;

	case oEXIT58T:
		Count58 = 0;
		for (int a = 0; a < 6; a++)
			g_vm->_obj[oLED158 + a]._mode &= ~OBJMODE_OBJSTATUS;
		doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, g_vm->_obj[oEXIT58T]._goRoom, 0, 0, TheObj);
		break;

	case oEXIT58M:
		doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, g_vm->_obj[oEXIT58M]._goRoom, 0, 0, TheObj);
		break;

	case oEXIT59L:
		doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, g_vm->_obj[oEXIT59L]._goRoom, 0, 0, TheObj);
		break;

	default:
		printsent = true;
		break;
	}

	if ((printsent) && (g_vm->_obj[TheObj]._examine))
		CharacterSay(g_vm->_obj[TheObj]._examine);
}

/*-------------------------------------------------------------------------*/
/*                                 OPERATE           					   */
/*-------------------------------------------------------------------------*/
void doMouseOperate(uint16 TheObj) {
	bool printsent = false;
	int a;

	if (!TheObj)
		warning("doMouseOperate");

	switch (TheObj) {
	case oFAX17:
		if (g_vm->_obj[oSCALA16]._anim) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, g_vm->_obj[TheObj]._anim, 0, 0, TheObj);
			g_vm->_inventoryObj[iSAM]._action = 1416;
			printsent = false;
		}
		break;

	case ocPOLIZIOTTO16:
	case oSCALA16:
		if (!(g_vm->_obj[oSCALA16]._flag & OBJFLAG_EXTRA)) {
			g_vm->_obj[oSCALA16]._flag |= OBJFLAG_EXTRA;
			_choice[61]._flag &= ~DLGCHOICE_HIDE;
			PlayDialog(dPOLIZIOTTO16);
			g_vm->_obj[oSCALA16]._action = 166;
			g_vm->_obj[ocPOLIZIOTTO16]._action = 166;
		} else
			printsent = true;
		break;

	case oPANNELLOC12:
		if (g_vm->_obj[oPANNELLOC12]._flag & OBJFLAG_EXTRA) {
			if (g_vm->_obj[oASCENSOREC12]._mode & OBJMODE_OBJSTATUS)
				doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a128RIUSABOTTONE, 0, 0, TheObj);
			else
				CharacterSay(24);
		} else {
			g_vm->_obj[oPANNELLOC12]._flag |= OBJFLAG_EXTRA;
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, g_vm->_obj[TheObj]._anim, 0, 0, TheObj);
		}
		break;
	case oPANNELLO13:
		if (g_vm->_obj[oASCENSOREA13]._mode & OBJMODE_OBJSTATUS) {
			CharacterSay(48);
			printsent = false;
		} else
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, g_vm->_obj[TheObj]._anim, 0, 0, TheObj);

		break;

	case oPANNELLO16:
		if (g_vm->_obj[oASCENSOREA16]._mode & OBJMODE_OBJSTATUS) {
			CharacterSay(48);
			printsent = false;
		} else
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, g_vm->_obj[TheObj]._anim, 0, 0, TheObj);
		break;

	case oLATTINA13:
		doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a132PRENDELATTINA, 0, 0, TheObj);
		AddIcon(iLATTINA13);
		break;

	case oPORTAA13:
		if (g_vm->_room[r14]._flag & OBJFLAG_DONE) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a1312METTELETTERARICALCA, r14, 14, g_vm->_useWith[WITH]);
			printsent = false;
		} else
			printsent = true;
		break;

	case oCESTINO14:
		if (g_vm->_obj[TheObj]._anim) {
			if (!(g_vm->_obj[oPORTAR14]._mode & OBJMODE_OBJSTATUS))
				doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, g_vm->_obj[TheObj]._anim, 0, 0, TheObj);
			else
				doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a143CPRENDECREDITCARD, 0, 0, TheObj);
			g_vm->_obj[TheObj]._anim = NULL;
		} else
			printsent = true;
		break;
	case oTASTOB15:
		if (g_vm->_obj[oTAPPARELLAA15]._mode & OBJMODE_OBJSTATUS) {
			CharacterSay(g_vm->_obj[oTASTOB15]._action);
			printsent = false;
		} else {
			if (!(g_vm->_obj[oNASTRO15]._flag & OBJFLAG_EXTRA))
				g_vm->_obj[oNASTRO15]._mode |= OBJMODE_OBJSTATUS;
			if (g_vm->_obj[TheObj]._anim)
				doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, g_vm->_obj[TheObj]._anim, 0, 0, TheObj);
			printsent = false;
		}
		break;

	case oTASTOA15:
		if (!(g_vm->_obj[oTAPPARELLAA15]._mode & OBJMODE_OBJSTATUS)) {
			CharacterSay(g_vm->_obj[oTASTOA15]._action);
			printsent = false;
		} else {
			g_vm->_obj[oNASTRO15]._mode &= ~OBJMODE_OBJSTATUS;
			if (g_vm->_obj[TheObj]._anim)
				doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, g_vm->_obj[TheObj]._anim, 0, 0, TheObj);
			printsent = false;
		}
		break;

	case oFINGERPADP16:
		printsent = true;
		break;

	case oPORTAC18:
		if (g_vm->_obj[ocGUARD18]._flag & OBJFLAG_PERSON) {
			_choice[151]._flag |= DLGCHOICE_HIDE;
			_choice[152]._flag &= ~DLGCHOICE_HIDE;
			PlayDialog(dGUARDIANO18);
			g_vm->_obj[ocGUARD18]._flag &= ~OBJFLAG_PERSON;
			g_vm->_obj[ocGUARD18]._action = 227;
			g_vm->_obj[oPORTAC18]._action = 220;
		} else
			printsent = true;
		break;

	case oGRATA1C:
		if (g_vm->_obj[oFAX17]._flag & OBJFLAG_EXTRA)
			doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, r21, 0, 10, TheObj);
		else
			printsent = true;
		break;

	case oBOTOLAC1B:
		if ((g_vm->_obj[oBOTOLAC1B]._anim == a1B3APREBOTOLA) && (g_vm->_obj[oTOMBINOA1B]._mode & OBJMODE_OBJSTATUS))
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a1B3AAPREBOTOLA, 0, 0, TheObj);
		else if (g_vm->_obj[oBOTOLAC1B]._anim == a1B3APREBOTOLA)
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a1B3APREBOTOLA, 0, 0, TheObj);
		else
			printsent = true;
		break;

	case oARMADIETTORC22:
		doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, g_vm->_obj[TheObj]._anim, 0, 0, TheObj);
		g_vm->_obj[oMANIGLIONE22]._anim = a227A;
		g_vm->_obj[oMANIGLIONEO22]._anim = a229A;
		g_vm->_obj[od22ALLA29]._anim = a2214A;
		g_vm->_obj[od22ALLA29I]._anim = a2215A;
		break;

	case oARMADIETTORA22:
		doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, g_vm->_obj[TheObj]._anim, 0, 0, TheObj);
		g_vm->_obj[oMANIGLIONE22]._anim = a227;
		g_vm->_obj[oMANIGLIONEO22]._anim = a229;
		g_vm->_obj[od22ALLA29]._anim = a2214;
		g_vm->_obj[od22ALLA29I]._anim = a2215;
		break;

	case oCATENAT21:
		if ((IconPos(iSBARRA21) != MAXICON) && ((_choice[436]._flag & OBJFLAG_DONE) || (_choice[466]._flag & OBJFLAG_DONE))) {
			if (g_vm->_room[g_vm->_curRoom]._flag & OBJFLAG_EXTRA) // va a destra
				PlayDialog(dF212);				// 436
			else									// va a sinistra
				PlayDialog(dF213);				// 466
			printsent = false;
		} else if ((_choice[451]._flag & OBJFLAG_DONE) || (_choice[481]._flag & OBJFLAG_DONE)) {
			if (g_vm->_room[g_vm->_curRoom]._flag & OBJFLAG_EXTRA) // va a destra
				PlayDialog(dF212B);				// 451
			else									// va a sinistra
				PlayDialog(dF213B);				// 481
			printsent = false;
		} else
			printsent = true;
		break;

	case oPULSANTEACS2D:
		doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, g_vm->_obj[TheObj]._anim, 0, 0, TheObj);
		g_vm->_obj[oPULSANTEBC2D]._anim = a2D7SCHIACCIATASTO6V;
		break;

	case oPULSANTEACA2D:
		doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, g_vm->_obj[TheObj]._anim, 0, 0, TheObj);
		g_vm->_obj[oPULSANTEBC2D]._anim = a2D7SCHIACCIATASTO6R;
		break;

	case oINTERRUTTORE29:
		if (g_vm->_curRoom == r29L)
			printsent = true;
		else if ((g_vm->_obj[oLAMPADINAS29]._mode & OBJMODE_OBJSTATUS)) {
			doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, r29L, 0, 0, TheObj);
			printsent = false;
		} else if (!(g_vm->_obj[g_vm->_curObj]._flag & OBJFLAG_EXTRA)) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a291USAINTERRUTTORELUCE, 0, 0, TheObj);
			g_vm->_obj[g_vm->_curObj]._flag |= OBJFLAG_EXTRA;
			printsent = false;
		} else
			printsent = true;
		break;

	case oLEVAS23:
		if (g_vm->_obj[oCAVI23]._mode & OBJMODE_OBJSTATUS)
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a238, 0, 0, TheObj);
		else
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a234, 0, 0, TheObj);
		break;

	case oLEVAG23:
		if (g_vm->_obj[oCAVIE23]._mode & OBJMODE_OBJSTATUS)
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a239, 0, 0, TheObj);
		else
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a235, 0, 0, TheObj);
		break;

	case oBOTOLAC25:
		doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, g_vm->_obj[TheObj]._anim, 0, 0, TheObj);
		g_vm->_obj[oTRONCHESE25]._anim = a254B;
		break;

	case oBOTOLAA25:
		doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, g_vm->_obj[TheObj]._anim, 0, 0, TheObj);
		g_vm->_obj[oTRONCHESE25]._anim = a254;
		break;

	case oPASSAGGIO24:
		if (g_vm->_room[r24]._flag & OBJFLAG_EXTRA) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a244, 0, 14, TheObj);
			SetRoom(r24, false);
			printsent = false;
		} else {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a243, 0, 13, TheObj);
			SetRoom(r24, true);
			printsent = false;
		}
		break;

	case oPORTA26:
		if (g_vm->_obj[TheObj]._anim)
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, g_vm->_obj[TheObj]._anim, 0, 4, TheObj);
		break;

	case oRUBINETTOC28:
		if (g_vm->_obj[TheObj]._anim) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, g_vm->_obj[TheObj]._anim, 0, 0, TheObj);
			g_vm->_obj[oBRACIERES28]._examine = 455;
			g_vm->_obj[oBRACIERES28]._flag |= OBJFLAG_EXTRA;
		} else
			printsent = true;
		break;

	case oEXIT12CU:
		doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, g_vm->_obj[oEXIT12CU]._goRoom, 0, 0, TheObj);
		break;

	case oEXIT13CU:
		doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, g_vm->_obj[oEXIT13CU]._goRoom, 0, 0, TheObj);
		break;

	case oEXIT2BL:
		doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, g_vm->_obj[oEXIT2BL]._goRoom, 0, 0, TheObj);
		break;

	case oEXIT36F:
		doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, g_vm->_obj[oEXIT36F]._goRoom, 0, 0, TheObj);
		break;

	case oEXIT41D:
		doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, g_vm->_obj[oEXIT41D]._goRoom, 0, 0, TheObj);
		break;

	case oEXIT4CT:
		doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, g_vm->_obj[oEXIT4CT]._goRoom, 0, 0, TheObj);
		break;

	case oEXIT58T:
		Count58 = 0;
		for (a = 0; a < 6; a++)
			g_vm->_obj[oLED158 + a]._mode &= ~OBJMODE_OBJSTATUS;
		doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, g_vm->_obj[oEXIT58T]._goRoom, 0, 0, TheObj);
		break;

	case oEXIT58M:
		doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, g_vm->_obj[oEXIT58M]._goRoom, 0, 0, TheObj);
		break;

	case oEXIT59L:
		doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, g_vm->_obj[oEXIT59L]._goRoom, 0, 0, TheObj);
		break;

	case oPANNELLOM2G:
		if (!(g_vm->_obj[oPANNELLOM2G]._flag & OBJFLAG_EXTRA)) {
			PlayDialog(dF2G1);
			g_vm->_obj[oCOPERCHIO2G]._mode &= ~OBJMODE_OBJSTATUS;
			//_obj[oPANNELLOM2G]._mode &= ~OBJMODE_OBJSTATUS;
			g_vm->_obj[oSERBATOIOC2G]._mode &= ~OBJMODE_OBJSTATUS;
			g_vm->_obj[oRAGAZZOP2G]._mode &= ~OBJMODE_OBJSTATUS;
			g_vm->_obj[oRAGAZZOS2G]._mode |= OBJMODE_OBJSTATUS;
			g_vm->_obj[oSERBATOIOA2G]._mode |= OBJMODE_OBJSTATUS;
			g_vm->_obj[oPANNELLOE2G]._mode |= OBJMODE_OBJSTATUS;
			g_vm->_obj[oPANNELLOM2G]._flag |= OBJFLAG_EXTRA;
			AnimTab[aBKG2G]._flag |= SMKANIM_OFF1;
			printsent = false;
		} else
			printsent = true;

		break;

	case oRUOTE2C:
		if (!(g_vm->_obj[od2CALLA2D]._mode & OBJMODE_OBJSTATUS)) {
			extern uint16 ruotepos[3];

			StopSmackAnim(g_vm->_room[g_vm->_curRoom]._bkgAnim);
			AnimTab[aBKG2C]._flag |= SMKANIM_OFF1;
			g_vm->_obj[oBASERUOTE2C]._mode |= OBJMODE_OBJSTATUS;
			g_vm->_obj[omRUOTE2C]._mode |= OBJMODE_OBJSTATUS;
			g_vm->_obj[oPULSANTE2C]._mode |= OBJMODE_OBJSTATUS;
			g_vm->_obj[ruotepos[0] * 3 + 0 + oRUOTA1A2C]._mode |= OBJMODE_OBJSTATUS;
			g_vm->_obj[ruotepos[1] * 3 + 1 + oRUOTA1A2C]._mode |= OBJMODE_OBJSTATUS;
			g_vm->_obj[ruotepos[2] * 3 + 2 + oRUOTA1A2C]._mode |= OBJMODE_OBJSTATUS;
			g_vm->_obj[oCAMPO2C]._mode &= ~OBJMODE_OBJSTATUS;
			g_vm->_obj[oTEMPIO2C]._mode &= ~OBJMODE_OBJSTATUS;
			g_vm->_obj[oLEONE2C]._mode &= ~OBJMODE_OBJSTATUS;
			g_vm->_obj[od2CALLA2D]._mode &= ~OBJMODE_OBJSTATUS;
			g_vm->_obj[oSFINGE2C]._mode &= ~OBJMODE_OBJSTATUS;
			g_vm->_obj[oSTATUA2C]._mode &= ~OBJMODE_OBJSTATUS;
			g_vm->_obj[oRUOTE2C]._mode &= ~OBJMODE_OBJSTATUS;
			g_vm->_obj[od2CALLA2E]._mode &= ~OBJMODE_OBJSTATUS;
			g_vm->_obj[oCARTELLOS2C]._mode &= ~OBJMODE_OBJSTATUS;
			g_vm->_obj[oCARTELLOA2C]._mode &= ~OBJMODE_OBJSTATUS;
			g_vm->_obj[od2CALLA26]._mode &= ~OBJMODE_OBJSTATUS;
			FlagShowCharacter = false;
			ReadExtraObj2C();
			RegenRoom();
		} else
			printsent = true;
		break;

	case oCATWALKA2E:
		if (!(g_vm->_obj[oCATWALKA2E]._flag & OBJFLAG_EXTRA)) {
			PlayDialog(dF2E1);
			g_vm->_obj[oDINOSAURO2E]._mode |= OBJMODE_OBJSTATUS;
			g_vm->_obj[oCATWALKA2E]._flag |= OBJFLAG_EXTRA;
			AnimTab[aBKG2E]._flag &= ~SMKANIM_OFF2;
			printsent = false;
		} else if (g_vm->_obj[TheObj]._anim) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, g_vm->_obj[TheObj]._anim, 0, 0, TheObj);
			printsent = false;
		} else
			printsent = true;
		break;

	case oBORSA22:
	case oPORTALAMPADE2B:
	case oMAPPAMONDO2B:
		if (g_vm->_obj[TheObj]._anim) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, g_vm->_obj[TheObj]._anim, 0, 0, TheObj);
			g_vm->_obj[TheObj]._anim = 0;
		} else
			printsent = true;
		break;

	case oTUBOF34:
		if (!(g_vm->_obj[oTUBOFT34]._mode & OBJMODE_OBJSTATUS))
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, g_vm->_obj[TheObj]._anim, 0, 0, TheObj);
		else
			printsent = true;
		break;

	case oFILOT31:
		g_vm->_obj[oFILOT31]._mode &= ~OBJMODE_OBJSTATUS;
		g_vm->_obj[oFILOS31]._mode |= OBJMODE_OBJSTATUS;
		RegenRoom();
		break;

	case oCOPERCHIOA31:
		if (g_vm->_obj[oFILOTC31]._mode & OBJMODE_OBJSTATUS) {
			NLPlaySound(wCOVER31);
			g_vm->_obj[oPANNELLOM31]._mode &= ~OBJMODE_OBJSTATUS;
			g_vm->_obj[oPANNELLOMA31]._mode |= OBJMODE_OBJSTATUS;
			g_vm->_obj[oPANNELLOM31]._anim = 0;
			g_vm->_obj[oPANNELLOM31]._examine = 715;
			g_vm->_obj[oPANNELLOM31]._action = 716;
			g_vm->_obj[oPANNELLOM31]._flag &= ~OBJFLAG_ROOMOUT;
			doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, r31, a3118CHIUDEPANNELLO, 3, TheObj);
		} else
			printsent = true;
		break;

	case oVALVOLAC34:
		if (g_vm->_obj[TheObj]._anim)
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, g_vm->_obj[TheObj]._anim, 0, 0, TheObj);
		else
			printsent = true;
		break;

	case oVALVOLA34:
		if (g_vm->_obj[TheObj]._anim)
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, g_vm->_obj[TheObj]._anim, 0, 0, TheObj);
		else
			printsent = true;
		break;

	case oPROIETTORE35:
		g_vm->_room[r35]._flag |= OBJFLAG_EXTRA;
		read3D("352.3d");		    // dopo scossa

		g_vm->_obj[oRIBELLEA35]._mode &= ~OBJMODE_OBJSTATUS;
		g_vm->_obj[oPORTAC35]._mode &= ~OBJMODE_OBJSTATUS;
		g_vm->_obj[omPORTAC35]._mode &= ~OBJMODE_OBJSTATUS;
		g_vm->_obj[oPRESA35]._mode &= ~OBJMODE_OBJSTATUS;

		g_vm->_obj[oPORTAA35]._mode |= OBJMODE_OBJSTATUS;
		g_vm->_obj[omPORTAA35]._mode |= OBJMODE_OBJSTATUS;
		g_vm->_obj[oRIBELLES35]._mode |= OBJMODE_OBJSTATUS;
		g_vm->_obj[oSEDIA35]._mode |= OBJMODE_OBJSTATUS;
		g_vm->_obj[oMONITOR35]._mode |= OBJMODE_OBJSTATUS;
		g_vm->_obj[omPIANO35]._mode |= OBJMODE_OBJSTATUS;

		g_vm->_obj[oFRONTOFFICEC35]._anim = a356PROVASPORTELLO;
		g_vm->_obj[oASCENSORE35]._flag |= OBJFLAG_ROOMOUT;
		g_vm->_obj[oASCENSORE35]._anim = a3514ENTRAASCENSORE;

		AnimTab[aBKG35]._flag |= SMKANIM_OFF1;
		PlayDialog(dF351);
		setPosition(7);
		break;

	case oCOMPUTER36:
		if (!(_choice[646]._flag & OBJFLAG_DONE)) {
			PlayDialog(dF361);
			g_vm->_obj[oCOMPUTER36]._action = 2004;
			printsent = false;
		} else
			printsent = true;
		break;

	case oPULSANTEA35:
	case oPULSANTEB35:
	case oPULSANTEC35:
	case oPULSANTED35:
	case oPULSANTEE35:
	case oPULSANTEF35:
	case oPULSANTEG35:
		g_vm->_obj[TheObj]._mode &= ~OBJMODE_OBJSTATUS;
		g_vm->_obj[TheObj + 7]._mode |= OBJMODE_OBJSTATUS;
		Comb35[Count35++] = TheObj;
		NLPlaySound(wPAD5);
		if (Count35 == 7) {
			if (((Comb35[0] == oPULSANTEF35) && (Comb35[1] == oPULSANTED35) && (Comb35[2] == oPULSANTEC35) &&
					(Comb35[3] == oPULSANTEG35) && (Comb35[4] == oPULSANTEB35) && (Comb35[5] == oPULSANTEA35) &&
					(Comb35[6] == oPULSANTEE35)) || ((Comb35[0] == oPULSANTEE35) &&
							(Comb35[1] == oPULSANTEA35) && (Comb35[2] == oPULSANTEB35) && (Comb35[3] == oPULSANTEG35) &&
							(Comb35[4] == oPULSANTEC35) && (Comb35[5] == oPULSANTED35) && (Comb35[6] == oPULSANTEF35))) {
				g_vm->_obj[oPULSANTEAA35]._mode &= ~OBJMODE_OBJSTATUS;
				g_vm->_obj[oPULSANTEBA35]._mode &= ~OBJMODE_OBJSTATUS;
				g_vm->_obj[oPULSANTECA35]._mode &= ~OBJMODE_OBJSTATUS;
				g_vm->_obj[oPULSANTEDA35]._mode &= ~OBJMODE_OBJSTATUS;
				g_vm->_obj[oPULSANTEEA35]._mode &= ~OBJMODE_OBJSTATUS;
				g_vm->_obj[oPULSANTEFA35]._mode &= ~OBJMODE_OBJSTATUS;
				g_vm->_obj[oPULSANTEGA35]._mode &= ~OBJMODE_OBJSTATUS;
				g_vm->_obj[oPULSANTIV35]._mode |= OBJMODE_OBJSTATUS;
				g_vm->_obj[oLEDS35]._mode &= ~OBJMODE_OBJSTATUS;

				g_vm->_obj[oFRONTOFFICEA35]._anim = 0;
				//					_obj[oFRONTOFFICEA35]._examine = 1843;
				g_vm->_obj[oFRONTOFFICEA35]._action = 1844;
				g_vm->_obj[oFRONTOFFICEA35]._flag |= OBJFLAG_EXTRA;
				g_vm->_obj[oPORTAMC36]._flag |= OBJFLAG_ROOMOUT;
				g_vm->_obj[oPORTAMC36]._anim = a3610APREPORTA;
				g_vm->_obj[oSCAFFALE36]._anim = a3615APRENDESCAFFALE;

				AnimTab[aBKG36]._flag |= SMKANIM_OFF2;
				AnimTab[aBKG36]._flag |= SMKANIM_OFF3;
				g_vm->_obj[oSCANNERLA36]._mode &= ~OBJMODE_OBJSTATUS;
				g_vm->_obj[oSCANNERLS36]._mode |= OBJMODE_OBJSTATUS;
				g_vm->_obj[oSCANNERMA36]._mode &= ~OBJMODE_OBJSTATUS;
				g_vm->_obj[oSCANNERMS36]._mode |= OBJMODE_OBJSTATUS;

				NLPlaySound(wWIN35);
			} else {
				g_vm->_obj[oPULSANTEA35]._mode |= OBJMODE_OBJSTATUS;
				g_vm->_obj[oPULSANTEB35]._mode |= OBJMODE_OBJSTATUS;
				g_vm->_obj[oPULSANTEC35]._mode |= OBJMODE_OBJSTATUS;
				g_vm->_obj[oPULSANTED35]._mode |= OBJMODE_OBJSTATUS;
				g_vm->_obj[oPULSANTEE35]._mode |= OBJMODE_OBJSTATUS;
				g_vm->_obj[oPULSANTEF35]._mode |= OBJMODE_OBJSTATUS;
				g_vm->_obj[oPULSANTEG35]._mode |= OBJMODE_OBJSTATUS;
				g_vm->_obj[oPULSANTEAA35]._mode &= ~OBJMODE_OBJSTATUS;
				g_vm->_obj[oPULSANTEBA35]._mode &= ~OBJMODE_OBJSTATUS;
				g_vm->_obj[oPULSANTECA35]._mode &= ~OBJMODE_OBJSTATUS;
				g_vm->_obj[oPULSANTEDA35]._mode &= ~OBJMODE_OBJSTATUS;
				g_vm->_obj[oPULSANTEEA35]._mode &= ~OBJMODE_OBJSTATUS;
				g_vm->_obj[oPULSANTEFA35]._mode &= ~OBJMODE_OBJSTATUS;
				g_vm->_obj[oPULSANTEGA35]._mode &= ~OBJMODE_OBJSTATUS;
			}
			Count35 = 0;
			Comb35[0] = 0;
			Comb35[1] = 0;
			Comb35[2] = 0;
			Comb35[3] = 0;
			Comb35[4] = 0;
			Comb35[5] = 0;
			Comb35[6] = 0;
		}
		g_vm->_curObj += 7;
		RegenRoom();
		break;

	case oCARD35:
		g_vm->_obj[oPULSANTEA35]._mode |= OBJMODE_OBJSTATUS;
		g_vm->_obj[oPULSANTEB35]._mode |= OBJMODE_OBJSTATUS;
		g_vm->_obj[oPULSANTEC35]._mode |= OBJMODE_OBJSTATUS;
		g_vm->_obj[oPULSANTED35]._mode |= OBJMODE_OBJSTATUS;
		g_vm->_obj[oPULSANTEE35]._mode |= OBJMODE_OBJSTATUS;
		g_vm->_obj[oPULSANTEF35]._mode |= OBJMODE_OBJSTATUS;
		g_vm->_obj[oPULSANTEG35]._mode |= OBJMODE_OBJSTATUS;
		g_vm->_obj[oPULSANTEAA35]._mode &= ~OBJMODE_OBJSTATUS;
		g_vm->_obj[oPULSANTEBA35]._mode &= ~OBJMODE_OBJSTATUS;
		g_vm->_obj[oPULSANTECA35]._mode &= ~OBJMODE_OBJSTATUS;
		g_vm->_obj[oPULSANTEDA35]._mode &= ~OBJMODE_OBJSTATUS;
		g_vm->_obj[oPULSANTEEA35]._mode &= ~OBJMODE_OBJSTATUS;
		g_vm->_obj[oPULSANTEFA35]._mode &= ~OBJMODE_OBJSTATUS;
		g_vm->_obj[oPULSANTEGA35]._mode &= ~OBJMODE_OBJSTATUS;
		AddIcon(iCARD36);

		doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, r35, a359RITIRACARD, 6, TheObj);
		break;

	case oSCAFFALE36:
		printsent = true;
		break;

	case oFOROC49:
	case oFORO849:
	case oFORO949:
	case oFORO1049:
	case oFORO1149:
	case oFORO1249:
	case oFORO149:
	case oFORO249:
	case oFORO349:
	case oFORO449:
	case oFORO549:
	case oFORO649:
	case oFORO749:
		for (a = oASTAC49; a <= oASTA749; a++)
			g_vm->_obj[a]._mode &= ~OBJMODE_OBJSTATUS;
		g_vm->_obj[oASTAC49 + TheObj - oFOROC49]._mode |= OBJMODE_OBJSTATUS;
		Comb49[3] = Comb49[2];
		Comb49[2] = Comb49[1];
		Comb49[1] = Comb49[0];
		Comb49[0] = TheObj;
		NLPlaySound(wASTA49);
		RegenRoom();
		if ((Comb49[3] ==  oFORO749) && (Comb49[2] ==  oFORO849) && (Comb49[1] ==  oFORO449) && (Comb49[0] ==  oFORO549)) {
			PaintScreen(0);
			NlDelay(60);
			g_vm->_obj[oOMBRAS49]._mode |= OBJMODE_OBJSTATUS;
			g_vm->_obj[oSCOMPARTO49]._mode |= OBJMODE_OBJSTATUS;
			g_vm->_obj[oAGENDA49]._mode |= OBJMODE_OBJSTATUS;
			g_vm->_obj[oMERIDIANA49]._mode &= ~OBJMODE_OBJSTATUS;
			g_vm->_obj[oAGENDA49]._examine = 1099;
			g_vm->_obj[oAGENDA49]._action = 1100;
			FlagCharacterExist = true;
			g_vm->_curObj = oAGENDA49;
			//FlagShowCharacter=true;
			//doEvent(MC_SYSTEM,ME_CHANGEROOM,MP_SYSTEM,r4A,0,1,TheObj);
			PlayScript(s49SUNDIAL);
		}
		printsent = false;
		break;

	case oASTAC49:
	case oASTA849:
	case oASTA949:
	case oASTA1049:
	case oASTA1149:
	case oASTA1249:
	case oASTA149:
	case oASTA249:
	case oASTA349:
	case oASTA449:
	case oASTA549:
	case oASTA649:
	case oASTA749:
		for (a = oASTAC49; a <= oASTA749; a++)
			g_vm->_obj[a]._mode &= ~OBJMODE_OBJSTATUS;
		g_vm->_obj[oASTAC49]._mode |= OBJMODE_OBJSTATUS;
		doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, r49, a496, 1, TheObj);
		printsent = false;
		break;

	case oNUMERO14C:
	case oNUMERO24C:
	case oNUMERO34C:
	case oNUMERO44C:
	case oNUMERO54C:
	case oNUMERO64C:
	case oNUMERO74C:
	case oNUMERO84C:
	case oNUMERO94C:
	case oNUMERO04C:
		for (a = 0; a < 6; a++) {
			if (Comb4CT[a] == 0) {
				g_vm->_obj[a + oAST14C]._mode |= OBJMODE_OBJSTATUS;
				Comb4CT[a] = TheObj - oNUMERO14C + 1;
				break;
			}
		}
		NLPlaySound(wPAD1 + TheObj - oNUMERO14C);
		RegenRoom();
		if (a < 5)
			break;
		PaintScreen(0);
		NlDelay(60);
		if ((Comb4CT[0] == 5) && (Comb4CT[1] == 6) && (Comb4CT[2] == 2) &&
				(Comb4CT[3] == 3) && (Comb4CT[4] == 9) && (Comb4CT[5] == 6)) {
			for (a = 0; a < 6; a++) {
				Comb4CT[a] = 0;
				g_vm->_obj[oAST14C + a]._mode &= ~OBJMODE_OBJSTATUS;
			}
			doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, r51, 0, 1, TheObj);
			FlagCharacterExist = true;
			//FlagShowCharacter=true;
		} else {
			for (a = 0; a < 6; a++) {
				Comb4CT[a] = 0;
				g_vm->_obj[oAST14C + a]._mode &= ~OBJMODE_OBJSTATUS;
			}
			doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, r4C, 0, 4, TheObj);
			FlagCharacterExist = true;
			//FlagShowCharacter=true;
		}
		printsent = false;
		break;

	case oPORTAC4A:
		if (!(_choice[245]._flag & OBJFLAG_DONE)  && !(_choice[766]._flag & OBJFLAG_DONE)) {
			_choice[245]._flag &= ~DLGCHOICE_HIDE;
			PlayDialog(dC4A1);
			setPosition(14);
			g_vm->_obj[oPORTAC4A]._action = 1117;
			g_vm->_obj[oPORTAC4A]._anim = 0;
			printsent = false;
		} else
			printsent = true;
		break;

	case oPULSANTE4A:
		if (g_vm->_obj[TheObj]._anim)
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, g_vm->_obj[TheObj]._anim, 0, 0, TheObj);
		else if (!(_choice[244]._flag & OBJFLAG_DONE)) {
			_choice[244]._flag &= ~DLGCHOICE_HIDE;
			PlayDialog(dC4A1);
			g_vm->_obj[oPULSANTE4A]._examine = 1108;
			g_vm->_obj[oPULSANTE4A]._action = 1109;
			printsent = false;
		} else
			printsent = true;
		break;

	case oFINESTRA56:
		if (g_vm->_obj[oPANNELLOC56]._mode & OBJMODE_OBJSTATUS)
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a5614, 0, 0, TheObj);
		else
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a5614P, 0, 0, TheObj);
		break;

	case oPULSANTECD:
		for (a = oPULSANTE1AD; a <= oPULSANTE33AD; a++) {
			if ((g_vm->_obj[a]._goRoom == g_vm->_obj[oEXIT41D]._goRoom) ||
			    ((g_vm->_obj[a]._goRoom == r45) && (g_vm->_obj[oEXIT41D]._goRoom == r45S))) {
				CharacterSay(903);
				break;
			} else if (g_vm->_obj[a]._goRoom == 0) {
				if (g_vm->_obj[oEXIT41D]._goRoom == r45S)
					g_vm->_obj[a]._goRoom = r45;
				else
					g_vm->_obj[a]._goRoom = g_vm->_obj[oEXIT41D]._goRoom;
				g_vm->_obj[a]._mode |= OBJMODE_OBJSTATUS;
				g_vm->_obj[a - 40]._mode &= ~OBJMODE_OBJSTATUS;
				RegenRoom();
				break;
			}
		}
		printsent = false;
		break;

	case oPORTAC54:
		if (!(_choice[826]._flag & OBJFLAG_DONE)) {
			PlayDialog(dF541);
			printsent = false;
		} else
			printsent = true;
		break;

	case oLAVATRICEL54:
		if ((g_vm->_obj[TheObj]._anim) && (g_vm->_obj[oSECCHIOS54]._flag & OBJFLAG_EXTRA) && (g_vm->_obj[oGRATAC54]._mode & OBJMODE_OBJSTATUS) && !(_choice[841]._flag & OBJFLAG_DONE))
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, g_vm->_obj[TheObj]._anim, 0, 0, TheObj);
		else
			printsent = true;
		break;

	case oSECCHIOS54:
		if (g_vm->_obj[oGRATAC54]._mode & OBJMODE_OBJSTATUS)
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a544G, 0, 0, TheObj);
		else
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, g_vm->_obj[TheObj]._anim, 0, 0, TheObj);
		printsent = false;
		break;

	case oTASTIERA56:
		if (_choice[262]._flag & OBJFLAG_DONE) {
			if (g_vm->_obj[od56ALLA59]._mode & OBJMODE_OBJSTATUS)
				CharacterSay(g_vm->_obj[TheObj]._action);
			else
				doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a567, 0, 0, TheObj);
			printsent = false;
		} else {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a564, 0, 0, TheObj);
			printsent = false;
		}
		break;

	case oLIBRIEG2B:
		if (g_vm->_room[r2C]._flag & OBJFLAG_DONE) { // se sono gia' stato nella 2C prendo libro
			printsent = false;
			if (g_vm->_obj[TheObj]._anim)
				doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a2B4PRENDELIBRO, 0, 0, TheObj);
			else
				printsent = true;
		} else {					// se non ci sono ancora stato dice che non gli serve
			CharacterSay(2014);
			printsent = false;
		}
		break;

	case oTASTIERA58:
		doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, r58T, 0, 0, TheObj);
		break;

	case oLAVAGNA59:
	case oSIMBOLI59:
		doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, r59L, 0, 0, TheObj);
		break;

	case oFINESTRAA5A:
		if ((_choice[871]._flag & OBJFLAG_DONE) && !(_choice[286]._flag & OBJFLAG_DONE)) {
			PlayDialog(dC5A1);
			printsent = false;
		} else
			printsent = true;
		break;

	case oFINESTRAB5A:
		if (!(_choice[256]._flag & OBJFLAG_DONE)) {
			CharacterSay(1999);
			printsent = false;
		} else
			printsent = true;
		break;

	case oTASTO158:
	case oTASTO258:
	case oTASTO358:
	case oTASTO458:
	case oTASTO558:
	case oTASTO658:
	case oTASTO758:
	case oTASTO858:
	case oTASTO958:
	case oTASTO058:
		printsent = false;
		Comb58[5] = Comb58[4];
		Comb58[4] = Comb58[3];
		Comb58[3] = Comb58[2];
		Comb58[2] = Comb58[1];
		Comb58[1] = Comb58[0];
		Comb58[0] = TheObj;

		NLPlaySound(wPAD1 + TheObj - oTASTO158);
		g_vm->_obj[oLED158 + Count58]._mode |= OBJMODE_OBJSTATUS;
		Count58 ++;
		RegenRoom();
		if (Count58 < 6)
			break;

		PaintScreen(0);
		NlDelay(60);
		Count58 = 0;
		for (a = 0; a < 6; a++)
			g_vm->_obj[oLED158 + a]._mode &= ~OBJMODE_OBJSTATUS;

		if ((Comb58[0] == oTASTO058) && (Comb58[1] == oTASTO258) && (Comb58[2] == oTASTO358) &&
				(Comb58[3] == oTASTO858) && (Comb58[4] == oTASTO558) && (Comb58[5] == oTASTO958)) {
			SoundFadOut();
			PlayDialog(dF582);
		} else
			doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, r58, 0, 2, TheObj);

		Comb58[5] = 0;
		Comb58[4] = 0;
		Comb58[3] = 0;
		Comb58[2] = 0;
		Comb58[1] = 0;
		Comb58[0] = 0;
		break;

	default:
		if (g_vm->_obj[TheObj]._anim)
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, g_vm->_obj[TheObj]._anim, 0, 0, TheObj);
		else
			printsent = true;
		break;
	}

	if ((printsent) && (g_vm->_obj[TheObj]._action))
		CharacterSay(g_vm->_obj[TheObj]._action);
}

/*-------------------------------------------------------------------------*/
/*                                  TAKE           						   */
/*-------------------------------------------------------------------------*/
void doMouseTake(uint16 TheObj) {
	bool del = true;

	if (!TheObj)
		warning("doMouseTake");

// FlagMouseEnabled = false;
	switch (TheObj) {
	case oTINFOIL11:
		del = false;
		break;
	case oNASTRO15:
		g_vm->_obj[oNASTRO15]._flag |= OBJFLAG_EXTRA;
		del = false;
		break;
	case oMONETA13:
		if (!(g_vm->_obj[oLATTINA13]._mode & OBJMODE_OBJSTATUS))
			g_vm->_obj[TheObj]._anim = a133CPRENDEMONETA;
		break;
	case oFOGLIETTO14:
		g_vm->_obj[oFOGLIETTO14]._flag |= OBJFLAG_EXTRA;
		g_vm->_obj[oMAPPA16]._examine = 152;
		del = false;
		break;
	case oPOSTERC22:
		g_vm->_obj[oARMADIETTOCC22]._anim = a221;
		g_vm->_obj[oARMADIETTOCA22]._anim = a222;
		break;
	case oKEY22:
		g_vm->_obj[oARMADIETTORA22]._examine = 2013;
		del = true;
		break;
	default:
		del = true;
		break;
	}
	uint16 TheAction = g_vm->_obj[TheObj]._anim;

	if (TheAction)
		doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, TheAction, 0, 0, TheObj);

	// spegne oggetto che viene preso
	if (del) {
		if (TheAction) {
			for (uint16 j = 0; j < MAXATFRAME; j++) {
				if ((AnimTab[TheAction]._atFrame[j]._type == ATFCLR) && (AnimTab[TheAction]._atFrame[j]._index == TheObj))
					break;

				if (AnimTab[TheAction]._atFrame[j]._type == 0) {
					AnimTab[TheAction]._atFrame[j]._child = 0;
					AnimTab[TheAction]._atFrame[j]._numFrame = 1;
					AnimTab[TheAction]._atFrame[j]._type = ATFCLR;
					AnimTab[TheAction]._atFrame[j]._index = TheObj;
					break;
				}
			}
		} else {
			g_vm->_obj[TheObj]._mode &= ~OBJMODE_OBJSTATUS;
			RegenRoom();
		}
	}
	AddIcon(g_vm->_obj[g_vm->_curObj]._ninv);
}

/*-------------------------------------------------------------------------*/
/*                                  TALK           						   */
/*-------------------------------------------------------------------------*/
void doMouseTalk(uint16 TheObj) {
	extern int16 TextStackTop;

	if (!TheObj)
		warning("doMouseTalk");

	switch (TheObj) {
	case oTICKETOFFICE16:
		if ((g_vm->_obj[oFINGERPADP16]._flag & OBJFLAG_ROOMOUT) && (_choice[50]._flag & OBJFLAG_DONE)) {
			CharacterSay(147);
			return ;
		}

		if ((_choice[49]._flag & DLGCHOICE_HIDE) && (_choice[50]._flag & DLGCHOICE_HIDE)) {
			if (g_vm->_obj[oMAPPA16]._flag & OBJFLAG_EXTRA) {
				_choice[46]._flag &= ~DLGCHOICE_HIDE;
				_choice[48]._flag &= ~DLGCHOICE_HIDE;
				g_vm->_obj[oTICKETOFFICE16]._flag |= OBJFLAG_EXTRA;
			} else {
				if (_choice[46]._flag & OBJFLAG_DONE) {
					CharacterSay(g_vm->_obj[oTICKETOFFICE16]._action);
					return ;
				}

				_choice[46]._flag &= ~DLGCHOICE_HIDE;
				_choice[47]._flag &= ~DLGCHOICE_HIDE;
			}
		}
		break;

	case ocGUARD18:
		g_vm->_obj[ocGUARD18]._flag &= ~OBJFLAG_PERSON;
		g_vm->_obj[ocGUARD18]._action = 227;
		g_vm->_obj[oPORTAC18]._action = 220;
		break;

	case ocNEGOZIANTE1A: {
		int c;
		for (c = _dialog[dNEGOZIANTE1A]._firstChoice; c < (_dialog[dNEGOZIANTE1A]._firstChoice + _dialog[dNEGOZIANTE1A]._choiceNumb); c++)
			if (!(_choice[c]._flag & DLGCHOICE_HIDE)) {
				PlayDialog(g_vm->_obj[TheObj]._goRoom);
				return;
			}
		if (g_vm->_obj[ocNEGOZIANTE1A]._action) {
			CharacterSay(g_vm->_obj[ocNEGOZIANTE1A]._action);
			return;
		}
	}
	break;

	case ocEVA19:
		g_vm->_inventoryObj[iSAM]._action = 1415;
		break;
	}

	PlayDialog(g_vm->_obj[TheObj]._goRoom);
}

/*-------------------------------------------------------------------------*/
/*                                USE WITH           					   */
/*-------------------------------------------------------------------------*/
void doUseWith() {
	if (g_vm->_useWithInv[USED]) {
		if (g_vm->_useWithInv[WITH])
			doInvInvUseWith();
		else
			doInvScrUseWith();
	} else
		doScrScrUseWith();

	g_vm->_useWith[USED] = 0;
	g_vm->_useWith[WITH] = 0;
	g_vm->_useWithInv[USED] = false;
	g_vm->_useWithInv[WITH] = false;
	FlagUseWithStarted = false;
}

/*-------------------------------------------------------------------------*/
/*                          USE WITH / INV - INV         				   */
/*-------------------------------------------------------------------------*/
void doInvInvUseWith() {
	bool updateinv = true, printsent = true;

	if ((!g_vm->_useWith[USED]) || (!g_vm->_useWith[WITH]))
		warning("doInvInvUseWith");

	StopSmackAnim(g_vm->_inventoryObj[g_vm->_useWith[USED]]._anim);

	switch (g_vm->_useWith[USED]) {
	//	TT
	case iSTAGNOLA:
		if (g_vm->_useWith[WITH] == iFUSE) {
			KillIcon(iSTAGNOLA);
			ReplaceIcon(iFUSE, iFUSES);
			StartCharacterAction(hUSEGG, 0, 0, 1441);
			printsent = false;
		}
		break;

	case iFUSE:
		if (g_vm->_useWith[WITH] == iSTAGNOLA) {
			KillIcon(iSTAGNOLA);
			ReplaceIcon(iFUSE, iFUSES);
			StartCharacterAction(hUSEGG, 0, 0, 1441);
			printsent = false;
		}

	// GG
	case iTOPO1C:
		if (g_vm->_useWith[WITH] == iPATTINO) {
			KillIcon(iPATTINO);
			KillIcon(iTOPO1C);
			AddIcon(iTOPO1D);
			StartCharacterAction(hUSEGG, 0, 0, 1497);
			printsent = false;
		}
		break;

	case iPATTINO:
		if (g_vm->_useWith[WITH] == iTOPO1C) {
			KillIcon(iPATTINO);
			KillIcon(iTOPO1C);
			AddIcon(iTOPO1D);
			StartCharacterAction(hUSEGG, 0, 0, 1497);
			printsent = false;
		}
		break;

	// GG
	case iBAR11:
		if (g_vm->_useWith[WITH] == iMAGNETE) {
			KillIcon(iBAR11);
			ReplaceIcon(iMAGNETE, iSBARRA21);
			StartCharacterAction(hUSEGG, 0, 0, 1438);
			printsent = false;
		}
		break;

	case iMAGNETE:
		if (g_vm->_useWith[WITH] == iBAR11) {
			KillIcon(iBAR11);
			ReplaceIcon(iMAGNETE, iSBARRA21);
			StartCharacterAction(hUSEGG, 0, 0, 1533);
			printsent = false;
		}
		break;

	case iSIGARO:
		if (g_vm->_useWith[WITH] == iSCOPA27) {
			KillIcon(iSCOPA27);
			ReplaceIcon(iSIGARO, iTORCIA32);
			StartCharacterAction(hUSEGG, 0, 0, 1575);
			printsent = false;
		}
		break;

	case iSCOPA27:
		if (g_vm->_useWith[WITH] == iSIGARO) {
			KillIcon(iSCOPA27);
			ReplaceIcon(iSIGARO, iTORCIA32);
			StartCharacterAction(hUSEGG, 0, 0, 1546);
			printsent = false;
		}
		break;

	case iPROIETTORE31:
		if (g_vm->_useWith[WITH] == iTRIPLA) {
			KillIcon(iTRIPLA);
			ReplaceIcon(iPROIETTORE31, iPROIETTORE35);
			StartCharacterAction(hUSEGG, 0, 0, 0);
			printsent = false;
		}
		break;

	case iTRIPLA:
		if (g_vm->_useWith[WITH] == iPROIETTORE31) {
			KillIcon(iTRIPLA);
			ReplaceIcon(iPROIETTORE31, iPROIETTORE35);
			StartCharacterAction(hUSEGG, 0, 0, 0);
			printsent = false;
		}
		break;

	case iSALNITRO:
	case iCARBONE:
	case iZOLFO:
	case iCARSAL:
	case iCARZOL:
	case iSALZOL:
		if ((g_vm->_useWith[WITH] == iSALNITRO) || (g_vm->_useWith[WITH] == iCARBONE) || (g_vm->_useWith[WITH] == iZOLFO) ||
		    (g_vm->_useWith[WITH] == iCARSAL) || (g_vm->_useWith[WITH] == iCARZOL) || (g_vm->_useWith[WITH] == iSALZOL)) {
			KillIcon(g_vm->_useWith[USED]);
			KillIcon(g_vm->_useWith[WITH]);

			if (((g_vm->_useWith[USED] == iSALNITRO) && (g_vm->_useWith[WITH] == iCARBONE)) ||
			    ((g_vm->_useWith[WITH] == iSALNITRO) && (g_vm->_useWith[USED] == iCARBONE)))
				AddIcon(iCARSAL);
			if (((g_vm->_useWith[USED] == iZOLFO) && (g_vm->_useWith[WITH] == iCARBONE)) ||
			    ((g_vm->_useWith[WITH] == iZOLFO) && (g_vm->_useWith[USED] == iCARBONE)))
				AddIcon(iCARZOL);
			if (((g_vm->_useWith[USED] == iZOLFO) && (g_vm->_useWith[WITH] == iSALNITRO)) ||
			    ((g_vm->_useWith[WITH] == iZOLFO) && (g_vm->_useWith[USED] == iSALNITRO)))
				AddIcon(iSALZOL);

			if ((g_vm->_useWith[USED] == iZOLFO) || (g_vm->_useWith[WITH] == iZOLFO))
				AddIcon(iBARATTOLO);
			if ((g_vm->_useWith[USED] >= iCARSAL) || (g_vm->_useWith[WITH] >= iCARSAL))
				AddIcon(iPOLVERE48);
			StartCharacterAction(hUSEGG, 0, 0, 1663);
			printsent = false;
		}
		break;

	case iPISTOLA4B:
		if (g_vm->_useWith[WITH] == iPOLVERE48) {
			ReplaceIcon(iPOLVERE48, iPOLVERE4P);
			ReplaceIcon(iPISTOLA4B, iPISTOLA4PC);
			StartCharacterAction(hUSEGG, 0, 0, 1676);
			printsent = false;
		} else if (g_vm->_useWith[WITH] == iPOLVERE4P) {
			KillIcon(iPOLVERE4P);
			ReplaceIcon(iPISTOLA4B, iPISTOLA4PC);
			StartCharacterAction(hUSEGG, 0, 0, 1700);
			printsent = false;
		}
		break;

	case iPOLVERE48:
		if (g_vm->_useWith[WITH] == iPISTOLA4B) {
			ReplaceIcon(iPOLVERE48, iPOLVERE4P);
			ReplaceIcon(iPISTOLA4B, iPISTOLA4PC);
			StartCharacterAction(hUSEGG, 0, 0, 1676);
			printsent = false;
		}
		break;

	case iPOLVERE4P:
		if (g_vm->_useWith[WITH] == iPISTOLA4B) {
			KillIcon(iPOLVERE4P);
			ReplaceIcon(iPISTOLA4B, iPISTOLA4PC);
			StartCharacterAction(hUSEGG, 0, 0, 1700);
			printsent = false;
		}
		break;

	case iBIGLIAA:
	case iBIGLIAB:
		if ((g_vm->_useWith[WITH] == iPISTOLA4PC) && !(g_vm->_inventoryObj[iPISTOLA4PC]._flag & OBJFLAG_EXTRA)) {
			KillIcon(g_vm->_useWith[USED]);
			ReplaceIcon(iPISTOLA4PC, iPISTOLA4PD);
			StartCharacterAction(hUSEGG, 0, 0, 1683);
			g_vm->_inventoryObj[iPISTOLA4PC]._flag |= OBJFLAG_EXTRA;
			printsent = false;
		} else if (g_vm->_useWith[WITH] == iPISTOLA4PC) {
			CharacterSay(1688);
			printsent = false;
		} else if (g_vm->_useWith[WITH] == iPISTOLA4B) {
			CharacterSay(2009);
			printsent = false;
		}
		break;

	case iBIGLIA4U:
		if (g_vm->_useWith[WITH] == iPISTOLA4PC) {
			KillIcon(iBIGLIA4U);
			ReplaceIcon(iPISTOLA4PC, iPISTOLA4PD);
			StartCharacterAction(hUSEGG, 0, 0, 1718);
			g_vm->_inventoryObj[iPISTOLA4PD]._flag |= OBJFLAG_EXTRA;
			printsent = false;
		} else if (g_vm->_useWith[WITH] == iPISTOLA4B) {
			CharacterSay(2011);
			printsent = false;
		}
		break;

	case iSIRINGA37:
		if (g_vm->_useWith[WITH] == iFIALE) {
			KillIcon(iSIRINGA37);
			ReplaceIcon(iFIALE, iSIRINGA59);
			StartCharacterAction(hUSEGG, 0, 0, 1756);
			printsent = false;
		}
		break;

	case iFIALE:
		if (g_vm->_useWith[WITH] == iSIRINGA37) {
			KillIcon(iSIRINGA37);
			ReplaceIcon(iFIALE, iSIRINGA59);
			StartCharacterAction(hUSEGG, 0, 0, 1756);
			printsent = false;
		}
		break;

	case iFILO:
		if (g_vm->_useWith[WITH] == iGUANTI57) {
			KillIcon(iFILO);
			ReplaceIcon(iGUANTI57, iGUANTI5A);
			StartCharacterAction(hUSEGG, 0, 0, 1756);
			printsent = false;
		} else if (g_vm->_useWith[WITH] == iSIRINGA59) {
			KillIcon(iFILO);
			ReplaceIcon(iSIRINGA59, iSIRINGA5A);
			StartCharacterAction(hUSEGG, 0, 0, 1756);
			printsent = false;
		}
		break;

	case iGUANTI57:
		if (g_vm->_useWith[WITH] == iFILO) {
			KillIcon(iFILO);
			ReplaceIcon(iGUANTI57, iGUANTI5A);
			StartCharacterAction(hUSEGG, 0, 0, 1756);
			printsent = false;
		} else if (g_vm->_useWith[WITH] == iSIRINGA5A) {
			KillIcon(iSIRINGA5A);
			ReplaceIcon(iGUANTI57, iARMAEVA);
			StartCharacterAction(hUSEGG, 0, 0, 1756);
			printsent = false;
		}
		break;

	case iSIRINGA59:
		if (g_vm->_useWith[WITH] == iFILO) {
			KillIcon(iFILO);
			ReplaceIcon(iSIRINGA59, iSIRINGA5A);
			StartCharacterAction(hUSEGG, 0, 0, 1756);
			printsent = false;
		} else if (g_vm->_useWith[WITH] == iGUANTI5A) {
			KillIcon(iSIRINGA59);
			ReplaceIcon(iGUANTI5A, iARMAEVA);
			StartCharacterAction(hUSEGG, 0, 0, 1756);
			printsent = false;
		}
		break;

	case iGUANTI5A:
		if (g_vm->_useWith[WITH] == iSIRINGA59) {
			KillIcon(iSIRINGA59);
			ReplaceIcon(iGUANTI5A, iARMAEVA);
			StartCharacterAction(hUSEGG, 0, 0, 1756);
			printsent = false;
		}
		break;

	case iSIRINGA5A:
		if (g_vm->_useWith[WITH] == iGUANTI57) {
			KillIcon(iSIRINGA5A);
			ReplaceIcon(iGUANTI57, iARMAEVA);
			StartCharacterAction(hUSEGG, 0, 0, 1756);
			printsent = false;
		}
		break;

	default:
		updateinv = false;
		break;
	}

	if (printsent)
		CharacterSay(g_vm->_inventoryObj[g_vm->_useWith[USED]]._action);
	if (updateinv)
		RegenInv(g_vm->_iconBase, INVENTORY_SHOW);
}

/*-------------------------------------------------------------------------*/
/*                          USE WITH / INV - SCR         				   */
/*-------------------------------------------------------------------------*/
void doInvScrUseWith() {
	bool updateinv = true, printsent = true;
	extern uint16 _curAnimFrame[];

	if ((!g_vm->_useWith[USED]) || (!g_vm->_useWith[WITH]))
		warning("doInvScrUseWith");

	StopSmackAnim(g_vm->_inventoryObj[g_vm->_useWith[USED]]._anim);
	if (_characterInMovement)
		return;
	switch (g_vm->_useWith[USED]) {
	case iBANCONOTE:
		if (g_vm->_useWith[WITH] == oDISTRIBUTORE13 && !(g_vm->_obj[oDISTRIBUTORE13]._flag & OBJFLAG_EXTRA)) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a131USABANCONOTA, 0, 0, g_vm->_useWith[WITH]);
			printsent = false;
			g_vm->_obj[oDISTRIBUTORE13]._flag |= OBJFLAG_EXTRA;
		} else if ((g_vm->_useWith[WITH] == oDISTRIBUTORE13) && (g_vm->_obj[oDISTRIBUTORE13]._flag & OBJFLAG_EXTRA) && (g_vm->_obj[oLATTINA13]._mode & OBJMODE_OBJSTATUS)) {
			CharacterSay(1410);
			printsent = false;
		} else if ((g_vm->_useWith[WITH] == oDISTRIBUTORE13) && (g_vm->_obj[oDISTRIBUTORE13]._flag & OBJFLAG_EXTRA)) {
			if (!(g_vm->_obj[oSCOMPARTO13]._flag & OBJFLAG_EXTRA)) {
				g_vm->_obj[oSCOMPARTO13]._flag |= OBJFLAG_EXTRA;
				doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a1311DABOTTADISTRIBUTORE, 0, 0, g_vm->_useWith[WITH]);
			} else
				CharacterSay(1411);

			printsent = false;
		} else if (g_vm->_useWith[WITH] == oTICKETOFFICE16) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a165USABANCONOTA, 0, 0, g_vm->_useWith[WITH]);
			g_vm->_inventoryObj[iBANCONOTE]._flag |= OBJFLAG_EXTRA;
			printsent = false;
		} else if ((g_vm->_useWith[WITH] == ocPOLIZIOTTO16) && (g_vm->_inventoryObj[iBANCONOTE]._flag & OBJFLAG_EXTRA)) {
			_choice[62]._flag &= ~DLGCHOICE_HIDE;
			PlayDialog(dPOLIZIOTTO16);
			printsent = false;
		}
		break;
	case iLETTER12:
		if (g_vm->_useWith[WITH] == oPENPADA13) {
			if (g_vm->_room[r14]._flag & OBJFLAG_DONE)
				doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a1312METTELETTERARICALCA, r14, 14, g_vm->_useWith[WITH]);
			else {
				doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a135METTELETTERA, 0, 0, g_vm->_useWith[WITH]);
				g_vm->_obj[oLETTERA13]._mode |= OBJMODE_OBJSTATUS;
				g_vm->_inventoryObj[iLETTER12]._flag |= OBJFLAG_EXTRA;
				KillIcon(iLETTER12);
				RegenRoom();
			}
			printsent = false;
		}
		break;
	case iFUSES:
		if (g_vm->_useWith[WITH] == oPANELA12) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a127USEWRAPPEDFUSED, 0, 0, g_vm->_useWith[WITH]);
			printsent = false;
		}
		break;
	case iFUSE:
		if (g_vm->_useWith[WITH] == oPANELA12) {
			CharacterSay(62);
			printsent = false;
		}
		break;
	case iKEY05:
		if (g_vm->_useWith[WITH] == oBOXES12) {
			if (!(g_vm->_obj[oBOXES12]._flag & OBJFLAG_EXTRA)) {
				doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a121BOXTEST, 0, 0, g_vm->_useWith[WITH]);
				g_vm->_obj[oBOXES12]._flag |= OBJFLAG_EXTRA;
				printsent = false;
			} else {
				CharacterSay(1426);
				printsent = false;
			}
		} else if (g_vm->_useWith[WITH] == oBOX12 && !(g_vm->_inventoryObj[iLETTER12]._flag & OBJFLAG_EXTRA)) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a122APREBOX, 0, 0, oBOX12);
			g_vm->_inventoryObj[iLETTER12]._flag |= OBJFLAG_EXTRA;
			printsent = false;
		} else if ((g_vm->_useWith[WITH] == oBOX12) && (g_vm->_inventoryObj[iLETTER12]._flag & OBJFLAG_EXTRA)) {
			CharacterSay(1429);
			printsent = false;
		} else if ((g_vm->_useWith[WITH] == oCARA11) || (g_vm->_useWith[WITH] == oCARB11) || (g_vm->_useWith[WITH] == oTAKE12) || (g_vm->_useWith[WITH] == oSTRONGBOXC15) || (g_vm->_useWith[WITH] == oDOOR18) || (g_vm->_useWith[WITH] == oPADLOCK1B) || (g_vm->_useWith[WITH] == oDOORC21) || (g_vm->_useWith[WITH] == oPANELC23) || (g_vm->_useWith[WITH] == oDOOR2A) || (g_vm->_useWith[WITH] == oDOORC33) || (g_vm->_useWith[WITH] == oFRONTOFFICEC35) || (g_vm->_useWith[WITH] == oCASSETTOC36) || (g_vm->_useWith[WITH] == oPORTAC54) || (g_vm->_useWith[WITH] == oPORTA57C55) || (g_vm->_useWith[WITH] == oPORTA58C55) || (g_vm->_useWith[WITH] == oPORTAS56) || (g_vm->_useWith[WITH] == oPORTAS57)) {
			printsent = false;
			CharacterSay(1426);
		}
		break;
	case iCARD03:
		if (g_vm->_useWith[WITH] == oSLOT12 || g_vm->_useWith[WITH] == oSLOT13 || g_vm->_useWith[WITH] == oSLOT16) {
			printsent = false;
			g_vm->_inventoryObj[iCARD03]._flag |= OBJFLAG_EXTRA;
			g_vm->_obj[oSLOT12]._flag |= OBJFLAG_PERSON;
			g_vm->_obj[oLIFTA12]._flag |= OBJFLAG_PERSON;
			doMouseTalk(g_vm->_useWith[WITH]);
		} else if ((g_vm->_useWith[WITH] == oTICKETOFFICE16) || (g_vm->_useWith[WITH] == oSLOT23) || (g_vm->_useWith[WITH] == oFRONTOFFICEA35) || (g_vm->_useWith[WITH] == oSLOTA58) || (g_vm->_useWith[WITH] == oSLOTB58)) {
			printsent = false;
			CharacterSay(1419);
		}
		break;
	case iPEN:
		printsent = false;
		if (((g_vm->_useWith[WITH] == oPENPADA13) || (g_vm->_useWith[WITH] == oLETTERA13)) && (g_vm->_obj[oLETTERA13]._mode & OBJMODE_OBJSTATUS))
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a137RICALCAFIRMA, r14, 14, g_vm->_useWith[WITH]);
		else if ((g_vm->_useWith[WITH] == oPENPADA13) && (g_vm->_room[r14]._flag & OBJFLAG_DONE))
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a1312METTELETTERARICALCA, r14, 14, g_vm->_useWith[WITH]);
		else if (g_vm->_useWith[WITH] == oPENPADA13) {
			if (!(g_vm->_obj[oBOX12]._mode & OBJMODE_OBJSTATUS)) {
				printsent = false;
				CharacterSay(2005);
			} else
				doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a134USAMAGNETICPEN, 0, 0, g_vm->_useWith[WITH]);
		} else
			printsent = true;
		break;

	case iACIDO15:
		if (g_vm->_useWith[WITH] == oBAR11) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a113USAFIALA, 0, 0, g_vm->_useWith[WITH]);
			ReplaceIcon(iACIDO15, iFIALAMETA);
			//AnimTab[a113USAFIALA]._atFrame[0]._index = 1483;
			printsent = false;
		} else if (g_vm->_useWith[WITH] == oPADLOCK1B) {
			if (g_vm->_obj[oTOMBINOA1B]._mode & OBJMODE_OBJSTATUS)
				doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a1B2AVERSAACIDO, 0, 0, g_vm->_useWith[WITH]);
			else
				doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a1B2VERSAACIDO, 0, 0, g_vm->_useWith[WITH]);
			g_vm->_obj[oBOTOLAC1B]._anim = a1B3APREBOTOLA;
			ReplaceIcon(iACIDO15, iFIALAMETA);
			AnimTab[a113USAFIALA]._atFrame[0]._index = 1483;
			printsent = false;
		} else if ((g_vm->_useWith[WITH] == ocGUARD18) || (g_vm->_useWith[WITH] == oMANHOLEC1B)) {
			printsent = false;
			CharacterSay(1476);
		}
		break;

	case iFIALAMETA:
		if (g_vm->_useWith[WITH] == oBAR11) {
			AnimTab[a113USAFIALA]._atFrame[0]._index = 1483;
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a113USAFIALA, 0, 0, g_vm->_useWith[WITH]);
			KillIcon(iFIALAMETA);
			printsent = false;
		} else if (g_vm->_useWith[WITH] == oPADLOCK1B) {
			AnimTab[a1B2AVERSAACIDO]._atFrame[2]._index = 1483;
			AnimTab[a1B2VERSAACIDO]._atFrame[2]._index = 1483;
			if (g_vm->_obj[oTOMBINOA1B]._mode & OBJMODE_OBJSTATUS)
				doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a1B2AVERSAACIDO, 0, 0, g_vm->_useWith[WITH]);
			else
				doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a1B2VERSAACIDO, 0, 0, g_vm->_useWith[WITH]);
			g_vm->_obj[oBOTOLAC1B]._anim = a1B3APREBOTOLA;
			KillIcon(iFIALAMETA);
			printsent = false;
		} else if ((g_vm->_useWith[WITH] == oDOOR2A) || (g_vm->_useWith[WITH] == oPORTA2B)) {
			printsent = false;
			CharacterSay(1508);
		}
		break;

	case iKEY15:
		if (g_vm->_useWith[WITH] == oSTRONGBOXC15) {
			// ParteFli
			PlayDialog(dF151);
			g_vm->_obj[oSTRONGBOXC15]._mode &= ~OBJMODE_OBJSTATUS;
			g_vm->_obj[oCASSAFORTEA15]._mode |= OBJMODE_OBJSTATUS;
			g_vm->_obj[oSLOT13]._mode &= ~OBJMODE_OBJSTATUS;
			g_vm->_obj[oASCENSOREA13]._mode &= ~OBJMODE_OBJSTATUS;
			g_vm->_obj[oASCENSOREC13]._mode |= OBJMODE_OBJSTATUS;
			AddIcon(iPISTOLA15);
			AddIcon(iACIDO15);
			AddIcon(iRAPPORTO);
			KillIcon(iKEY15);
			printsent = false;
			updateinv = false;
		} else if ((g_vm->_useWith[WITH] == oCARA11) || (g_vm->_useWith[WITH] == oCARB11) || (g_vm->_useWith[WITH] == oTAKE12) || (g_vm->_useWith[WITH] == oBOX12) || (g_vm->_useWith[WITH] == oDOOR18) || (g_vm->_useWith[WITH] == oPADLOCK1B) || (g_vm->_useWith[WITH] == oDOORC21) || (g_vm->_useWith[WITH] == oPANELC23) || (g_vm->_useWith[WITH] == oDOOR2A) || (g_vm->_useWith[WITH] == oDOORC33) || (g_vm->_useWith[WITH] == oFRONTOFFICEC35) || (g_vm->_useWith[WITH] == oCASSETTOC36) || (g_vm->_useWith[WITH] == oPORTAC54) || (g_vm->_useWith[WITH] == oPORTA57C55) || (g_vm->_useWith[WITH] == oPORTA58C55) || (g_vm->_useWith[WITH] == oPORTAS56) || (g_vm->_useWith[WITH] == oPORTAS57)) {
			printsent = false;
			CharacterSay(1469);
		}
		break;

	case iBAR11:
		if (g_vm->_useWith[WITH] == oMANHOLEC1B) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a1B1USASBARRA, 0, 0, g_vm->_useWith[WITH]);
			g_vm->_obj[oBOTOLAA1B]._anim = a1B6ASCENDEBOTOLA;
			//_obj[oBOTOLAC1B]._anim = a1B3AAPREBOTOLA;
			printsent = false;
		} else if (g_vm->_useWith[WITH] == oCATENAT21) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a216, 0, 0, g_vm->_useWith[WITH]);
			printsent = false;
		} else if (g_vm->_useWith[WITH] == oALTOPARLANTE25) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a251, 0, 0, g_vm->_useWith[WITH]);
			printsent = false;
		} else if (g_vm->_useWith[WITH] == oDOORC33) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a333LOSEBAR, 0, 0, g_vm->_useWith[WITH]);
			KillIcon(iBAR11);
			printsent = false;
		} else if (g_vm->_obj[g_vm->_useWith[WITH]]._flag & OBJFLAG_PERSON) {
			CharacterSay(1436);
			printsent = false;
		} else if ((g_vm->_useWith[WITH] == oTAKE12) || (g_vm->_useWith[WITH] == oSTRONGBOXC15) || (g_vm->_useWith[WITH] == oDOOR18) || (g_vm->_useWith[WITH] == oPADLOCK1B) || (g_vm->_useWith[WITH] == oDOORC21) || (g_vm->_useWith[WITH] == oPANELC23) || (g_vm->_useWith[WITH] == oDOOR2A) || (g_vm->_useWith[WITH] == oPORTA2B)) {
			printsent = false;
			CharacterSay(1435);
		}
		break;

	case iCARD14:
		if ((g_vm->_useWith[WITH] == oTICKETOFFICE16) && (g_vm->_obj[oMAPPA16]._flag & OBJFLAG_EXTRA)) {
			if (_choice[49]._flag & OBJFLAG_DONE) {
				CharacterSay(1457);
				printsent = false;
			} else {
				_choice[46]._flag |= DLGCHOICE_HIDE;
				_choice[47]._flag |= DLGCHOICE_HIDE;
				_choice[48]._flag |= DLGCHOICE_HIDE;
				_choice[49]._flag &= ~DLGCHOICE_HIDE;
				PlayScript(s16CARD);
//					doMouseTalk( _useWith[WITH] );
				printsent = false;
			}
		} else if (g_vm->_useWith[WITH] == oSLOT23) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a2311, 0, 0, g_vm->_useWith[WITH]);
			printsent = false;
		} else if ((g_vm->_useWith[WITH] == oSLOT12) || (g_vm->_useWith[WITH] == oSLOT13) || (g_vm->_useWith[WITH] == oSLOT16) || (g_vm->_useWith[WITH] == oFRONTOFFICEA35) || (g_vm->_useWith[WITH] == oSLOTA58) || (g_vm->_useWith[WITH] == oSLOTB58)) {
			printsent = false;
			CharacterSay(1419);
		}
		break;

	case iMONETA13:
		if ((g_vm->_useWith[WITH] == oTICKETOFFICE16) && (g_vm->_obj[oMAPPA16]._flag & OBJFLAG_EXTRA)) {
			_choice[46]._flag |= DLGCHOICE_HIDE;
			_choice[47]._flag |= DLGCHOICE_HIDE;
			_choice[48]._flag |= DLGCHOICE_HIDE;
			_choice[50]._flag &= ~DLGCHOICE_HIDE;
			PlayScript(S16MONEY);
//				doMouseTalk( _useWith[WITH] );
			printsent = false;
			KillIcon(iMONETA13);
			g_vm->_obj[oFINGERPADP16]._flag |= OBJFLAG_ROOMOUT;
		} else if (g_vm->_useWith[WITH] == oTICKETOFFICE16) {
			CharacterSay(146);
			printsent = false;
		}
		break;

	case iPLASTICA:
		if (g_vm->_useWith[WITH] == oTELEFAXF17) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a172USAPLASTICA, 0, 0, g_vm->_useWith[WITH]);
			g_vm->_obj[oTELEFAXF17]._examine = 1486;
			printsent = false;
		}
		break;

	case iFOTO:
		if ((g_vm->_useWith[WITH] == ocBARBONE17) && (_choice[81]._flag & OBJFLAG_DONE)) {
			CharacterSay(1463);
			printsent = false;
		} else if ((g_vm->_useWith[WITH] == ocBARBONE17) && (_choice[91]._flag & OBJFLAG_DONE)) {
			g_vm->_obj[ocBARBONE17]._action = 1462;
			CharacterSay(g_vm->_obj[ocBARBONE17]._action);
			printsent = false;
		} else if ((g_vm->_useWith[WITH] == ocBARBONE17) && (!(_choice[78]._flag & OBJFLAG_DONE) || ((_choice[79]._flag & OBJFLAG_DONE) || (_choice[83]._flag & OBJFLAG_DONE) && !(_choice[92]._flag & OBJFLAG_DONE)))) {
			_choice[78]._flag &= ~DLGCHOICE_HIDE;
			if ((_choice[79]._flag & OBJFLAG_DONE) || (_choice[83]._flag & OBJFLAG_DONE)) {
				_choice[92]._flag &= ~DLGCHOICE_HIDE;
				if (_choice[80]._flag & OBJFLAG_DONE)
					_choice[121]._flag &= ~DLGCHOICE_HIDE;
				else
					_choice[122]._flag &= ~DLGCHOICE_HIDE;
			} else
				_choice[91]._flag &= ~DLGCHOICE_HIDE;
			PlayDialog(dBARBONE171);
			updateinv = false;
			printsent = false;
		} else if (g_vm->_useWith[WITH] == ocPOLIZIOTTO16) {
			g_vm->_obj[ocPOLIZIOTTO16]._flag |= OBJFLAG_EXTRA;
			CharacterSay(1461);
			if ((_choice[61]._flag & OBJFLAG_DONE) && (_choice[62]._flag & OBJFLAG_DONE) && (g_vm->_obj[ocPOLIZIOTTO16]._flag & OBJFLAG_EXTRA))
				g_vm->_obj[ocPOLIZIOTTO16]._mode &= ~OBJMODE_OBJSTATUS;
			printsent = false;
		} else if (g_vm->_useWith[WITH] == ocGUARD18) {
			if (_choice[152]._flag & OBJFLAG_DONE)
				CharacterSay(1465);
			else
				CharacterSay(1464);
			printsent = false;
		} else if (g_vm->_useWith[WITH] == ocNEGOZIANTE1A) {
			CharacterSay(1466);
			printsent = false;
		} else if (g_vm->_useWith[WITH] == ocEVA19) {
			CharacterSay(1465);
			printsent = false;
		}

		break;

	case iLATTINA13:
		if (g_vm->_useWith[WITH] == ocBARBONE17) {
			if ((_choice[79]._flag & OBJFLAG_DONE) || (_choice[83]._flag & OBJFLAG_DONE)) {
				_choice[80]._flag &= ~DLGCHOICE_HIDE;
				if (_choice[81]._flag & OBJFLAG_DONE) {
					_choice[81]._flag &= ~DLGCHOICE_HIDE;
					_choice[80]._flag &= ~DLGCHOICE_EXITDLG;
				}
				PlayDialog(dBARBONE171);
				updateinv = false;
				KillIcon(iLATTINA13);
				printsent = false;
			} else { // if( !(_choice[79]._flag & OBJFLAG_DONE) )
				_choice[79]._flag &= ~DLGCHOICE_HIDE;
				if (!(_choice[78]._flag & OBJFLAG_DONE)) {
					_choice[106]._flag &= ~DLGCHOICE_HIDE;
					g_vm->_obj[ocBARBONE17]._action = 213;
				} else {
					_choice[107]._flag &= ~DLGCHOICE_HIDE;
					if (_choice[80]._flag & OBJFLAG_DONE)
						_choice[121]._flag &= ~DLGCHOICE_HIDE;
					else
						_choice[122]._flag &= ~DLGCHOICE_HIDE;
				}
				PlayDialog(dBARBONE171);
				updateinv = false;
				KillIcon(iLATTINA13);
				printsent = false;
			}
		}

		break;

	case iBOTTIGLIA14:
		if (g_vm->_useWith[WITH] == ocBARBONE17) {
			if ((_choice[79]._flag & OBJFLAG_DONE) || (_choice[83]._flag & OBJFLAG_DONE)) {
				_choice[80]._flag &= ~DLGCHOICE_HIDE;
				if (_choice[81]._flag & OBJFLAG_DONE) {
					_choice[81]._flag &= ~DLGCHOICE_HIDE;
					_choice[80]._flag &= ~DLGCHOICE_EXITDLG;
				}
				PlayDialog(dBARBONE171);
				updateinv = false;
				KillIcon(iBOTTIGLIA14);
				printsent = false;
			} else {
				_choice[83]._flag &= ~DLGCHOICE_HIDE;
				if (!(_choice[78]._flag & OBJFLAG_DONE)) {
					_choice[106]._flag &= ~DLGCHOICE_HIDE;
					g_vm->_obj[ocBARBONE17]._action = 213;
				} else {
					_choice[107]._flag &= ~DLGCHOICE_HIDE;
					if (_choice[80]._flag & OBJFLAG_DONE)
						_choice[121]._flag &= ~DLGCHOICE_HIDE;
					else
						_choice[122]._flag &= ~DLGCHOICE_HIDE;
				}
				PlayDialog(dBARBONE171);
				updateinv = false;
				KillIcon(iBOTTIGLIA14);
				printsent = false;
			}
		}

		break;

	case iBOTTIGLIA1D:
		if (g_vm->_useWith[WITH] == ocNEGOZIANTE1A) {
			printsent = false;
			if (_choice[183]._flag & OBJFLAG_DONE) {
				_choice[185]._flag &= ~DLGCHOICE_HIDE;
				_dialog[dNEGOZIANTE1A]._startLen = 0;
				PlayDialog(dNEGOZIANTE1A);
				updateinv = false;
				_dialog[dNEGOZIANTE1A]._startLen = 1;
				ReplaceIcon(iBOTTIGLIA1D, iTESSERA);
			} else
				CharacterSay(2006);
		}
		break;

	case iTESSERA:
		if ((g_vm->_useWith[WITH] == ocGUARD18) && !(_choice[155]._flag & OBJFLAG_DONE)) {
			_choice[155]._flag &= ~DLGCHOICE_HIDE;
			PlayDialog(dGUARDIANO18);
			updateinv = false;
			g_vm->_obj[ocGUARD18]._flag &= ~OBJFLAG_PERSON;
			g_vm->_obj[oPORTAC18]._flag |= OBJFLAG_ROOMOUT;
			printsent = false;
		} else if (g_vm->_useWith[WITH] == ocGUARD18) {
			CharacterSay(1494);
			printsent = false;
		}
		break;

	case iTOPO1D:
		if ((g_vm->_useWith[WITH] == oDONNA1D) &&
		    ((mx >= g_vm->_obj[oDONNA1D]._lim[0]) &&
		     (my >= g_vm->_obj[oDONNA1D]._lim[1] + TOP) &&
		     (mx <= g_vm->_obj[oDONNA1D]._lim[2]) &&
		     (my <= g_vm->_obj[oDONNA1D]._lim[3] + TOP))) {
			PlayDialog(dF1D1);
			updateinv = false;
			KillIcon(iTOPO1D);
			read3D("1d2.3d");		// dopo pattino
			g_vm->_obj[oDONNA1D]._mode &= ~OBJMODE_OBJSTATUS;
			g_vm->_room[g_vm->_curRoom]._flag |= OBJFLAG_EXTRA;
			AnimTab[aBKG1D]._flag |= SMKANIM_OFF1;
			printsent = false;
		}
		break;

	case iPISTOLA15:
		if ((g_vm->_useWith[WITH] == oDOORC21) && !(g_vm->_room[r21]._flag & OBJFLAG_EXTRA)) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a211, 0, 0, g_vm->_useWith[WITH]);
			g_vm->_inventoryObj[iPISTOLA15]._examine = 1472;
			g_vm->_inventoryObj[iPISTOLA15]._action = 1473;
			printsent = false;
		}
		break;

	case iCACCIAVITE:
		if (g_vm->_useWith[WITH] == oESSE21) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a213, 0, 0, g_vm->_useWith[WITH]);
			printsent = false;
		} else if (g_vm->_useWith[WITH] == oCOPERCHIOC31) {
			NLPlaySound(wCOVER31);
			g_vm->_obj[oCOPERCHIOC31]._mode &= ~OBJMODE_OBJSTATUS;
			g_vm->_obj[oCOPERCHIOA31]._mode |= OBJMODE_OBJSTATUS;
			RegenRoom();
			printsent = false;
		} else if (g_vm->_useWith[WITH] == oCOPERCHIOA31) {
			g_vm->_obj[oCOPERCHIOA31]._mode &= ~OBJMODE_OBJSTATUS;
			g_vm->_obj[oCOPERCHIOC31]._mode |= OBJMODE_OBJSTATUS;
			g_vm->_obj[oPANNELLOM31]._mode &= ~OBJMODE_OBJSTATUS;
			g_vm->_obj[oPANNELLOMA31]._mode |= OBJMODE_OBJSTATUS;
			if (g_vm->_obj[oFILOTC31]._mode & OBJMODE_OBJSTATUS) {
				g_vm->_obj[oPANNELLOM31]._anim = 0;
				g_vm->_obj[oPANNELLOM31]._examine = 715;
				g_vm->_obj[oPANNELLOM31]._action = 716;
				g_vm->_obj[oPANNELLOM31]._flag &= ~OBJFLAG_ROOMOUT;
			}
			NLPlaySound(wCOVER31);
			RegenRoom();
			printsent = false;
			doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, r31, a3118CHIUDEPANNELLO, 3, g_vm->_curObj);
		} else if (g_vm->_useWith[WITH] == oPANNELLO55) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a5512, 0, 0, g_vm->_useWith[WITH]);
			printsent = false;
		} else if (g_vm->_useWith[WITH] == oPANNELLOC56) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a568, 0, 0, g_vm->_useWith[WITH]);
			printsent = false;
		} else if ((g_vm->_useWith[WITH] == oSLOT23) || (g_vm->_useWith[WITH] == oPRESA35) || (g_vm->_useWith[WITH] == oSERRATURA33)) {
			printsent = false;
			CharacterSay(1520);
		}
		break;

	case iESSE:
		if (g_vm->_useWith[WITH] == oCATENA21) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a214, 0, 0, g_vm->_useWith[WITH]);
			printsent = false;
		}
		break;

	case iTANICHETTA27:
		if (g_vm->_useWith[WITH] == oMANIGLIONE22) {
			if (g_vm->_obj[oARMADIETTORC22]._mode & OBJMODE_OBJSTATUS)
				doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a228, 0, 0, g_vm->_useWith[WITH]);
			else
				doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a228A, 0, 0, g_vm->_useWith[WITH]);
			KillIcon(iTANICHETTA27);
			AddIcon(iTANICHETTA22);
			if (g_vm->_inventoryObj[iLATTINA28]._flag & OBJFLAG_EXTRA) {
				KillIcon(iTANICHETTA22);
				ReplaceIcon(iLATTINA28, iLATTINE);
			}
			printsent = false;
		} else
			break;

	case iKEY22:
		if (g_vm->_useWith[WITH] == oDOOR2A) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a2A2USEKEY, 0, 0, g_vm->_useWith[WITH]);
			printsent = false;
		} else if ((g_vm->_useWith[WITH] == oPANELC23) || (g_vm->_useWith[WITH] == oDOORC33) || (g_vm->_useWith[WITH] == oFRONTOFFICEC35) || (g_vm->_useWith[WITH] == oCASSETTOC36) || (g_vm->_useWith[WITH] == oPORTAC54) || (g_vm->_useWith[WITH] == oPORTA57C55) || (g_vm->_useWith[WITH] == oPORTA58C55) || (g_vm->_useWith[WITH] == oPORTAS56) || (g_vm->_useWith[WITH] == oPORTAS57)) {
			printsent = false;
			CharacterSay(1512);
		}
		break;

	case iLAMPADINA29:
		if ((g_vm->_useWith[WITH] == oPORTALAMPADE2B) && !(g_vm->_obj[g_vm->_useWith[WITH]]._anim)) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a2B6METTELAMPADINA, 0, 0, g_vm->_useWith[WITH]);
			printsent = false;
		}
		break;

	case iPIPEWRENCH:
		if (g_vm->_useWith[WITH] == oPANELC23) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a233, 0, 0, g_vm->_useWith[WITH]);
			printsent = false;
		} else if ((g_vm->_useWith[WITH] == oDOORC33) || (g_vm->_useWith[WITH] == oFRONTOFFICEC35) || (g_vm->_useWith[WITH] == oCASSETTOC36) || (g_vm->_useWith[WITH] == oPORTAC54) || (g_vm->_useWith[WITH] == oPORTA57C55) || (g_vm->_useWith[WITH] == oPORTA58C55) || (g_vm->_useWith[WITH] == oPORTAS56) || (g_vm->_useWith[WITH] == oPORTAS57)) {
			printsent = false;
			CharacterSay(1525);
		}
		break;

	case iCAVI:
		if ((g_vm->_useWith[WITH] == oCONTATTI23) && (g_vm->_obj[oLEVAS23]._mode & OBJMODE_OBJSTATUS)) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a236, 0, 0, g_vm->_useWith[WITH]);
			printsent = false;
		} else if (g_vm->_useWith[WITH] == oLEVAG23) {
			CharacterSay(2015);
			printsent = false;
		}
		break;

	case iTRONCHESE:
		if ((g_vm->_useWith[WITH] == oCAVO2H) && (g_vm->_obj[oCARTELLONE2H]._mode & OBJMODE_OBJSTATUS)) {
			PlayDialog(dF2H1);
			g_vm->_obj[oPASSAGGIO24]._mode |= OBJMODE_OBJSTATUS;
			g_vm->_obj[omPASSAGGIO24]._mode |= OBJMODE_OBJSTATUS;
			g_vm->_obj[oCARTELLONE24]._mode &= ~OBJMODE_OBJSTATUS;
			g_vm->_obj[oCARTELLONE2H]._mode &= ~OBJMODE_OBJSTATUS;
			g_vm->_obj[oPASSERELLA24]._mode &= ~OBJMODE_OBJSTATUS;
			g_vm->_obj[oMACERIE24]._mode &= ~OBJMODE_OBJSTATUS;
			//_obj[oPASSERELLA24]._flag &= ~OBJFLAG_ROOMOUT;
			//_obj[oPASSERELLA24]._anim = 0;
			printsent = false;
		} else if ((g_vm->_useWith[WITH] == oTUBOT34) && (g_vm->_obj[oVALVOLAC34]._mode & OBJMODE_OBJSTATUS)) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a341USAPINZE, 0, 0, g_vm->_useWith[WITH]);
			//_obj[oVALVOLAC34]._anim = 0;
			printsent = false;
		} else if ((g_vm->_useWith[WITH] == oTUBOT34) && (g_vm->_obj[oVALVOLA34]._mode & OBJMODE_OBJSTATUS)) {
			CharacterSay(2007);
			printsent = false;
		} else
			printsent = true;
		break;

	case iLATTINA28:
		if ((g_vm->_useWith[WITH] == oSERBATOIOA2G) && !(g_vm->_inventoryObj[iLATTINA28]._flag & OBJFLAG_EXTRA)) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a2G4VERSALATTINA, 0, 0, g_vm->_useWith[WITH]);
			g_vm->_inventoryObj[iLATTINA28]._flag |= OBJFLAG_EXTRA;
			g_vm->_inventoryObj[iLATTINA28]._examine = 1537;
			if (IconPos(iTANICHETTA22) != MAXICON) {
				KillIcon(iTANICHETTA22);
				ReplaceIcon(iLATTINA28, iLATTINE);
			}
			if ((g_vm->_inventoryObj[iBOMBOLA]._flag & OBJFLAG_EXTRA) && (g_vm->_inventoryObj[iLATTINA28]._flag & OBJFLAG_EXTRA)) {
				g_vm->_obj[oSERBATOIOA2G]._examine = 670;
				g_vm->_obj[oSERBATOIOA2G]._action = 671;
			} else {
				g_vm->_obj[oSERBATOIOA2G]._examine = 667;
				g_vm->_obj[oSERBATOIOA2G]._action = 669;
			}
			printsent = false;
		}
		break;

	case iBOMBOLA:
		if (g_vm->_useWith[WITH] == oSERBATOIOA2G) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a2G5METTEBOMBOLA, 0, 0, g_vm->_useWith[WITH]);
			g_vm->_inventoryObj[iBOMBOLA]._flag |= OBJFLAG_EXTRA;
			if ((g_vm->_inventoryObj[iBOMBOLA]._flag & OBJFLAG_EXTRA) && (g_vm->_inventoryObj[iLATTINA28]._flag & OBJFLAG_EXTRA)) {
				g_vm->_obj[oSERBATOIOA2G]._examine = 670;
				g_vm->_obj[oSERBATOIOA2G]._action = 671;
			} else {
				g_vm->_obj[oSERBATOIOA2G]._examine = 668;
				g_vm->_obj[oSERBATOIOA2G]._action = 669;
			}
			printsent = false;
		}
		break;

	case iCANDELOTTO:
		if ((g_vm->_useWith[WITH] == oSERBATOIOA2G) && (g_vm->_inventoryObj[iBOMBOLA]._flag & OBJFLAG_EXTRA) && (g_vm->_inventoryObj[iLATTINA28]._flag & OBJFLAG_EXTRA)) {
			doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, r2GV, 0, 0, g_vm->_useWith[WITH]);
			KillIcon(iCANDELOTTO);
			printsent = false;
		}
		break;

	case iFUCILE:
		if (g_vm->_useWith[WITH] == oDINOSAURO2E) {
			PlayDialog(dF2E2);
			g_vm->_obj[oDINOSAURO2E]._mode &= ~OBJMODE_OBJSTATUS;
			g_vm->_obj[oCATWALKA2E]._anim = a2E2PRIMAPALLONTANANDO;
			AnimTab[aBKG2E]._flag |= SMKANIM_OFF2;
			printsent = false;
		}
		break;

	case iPINZA:
	case iSBARRA21:
		if (g_vm->_useWith[WITH] == oCATENAT21) {
			if (g_vm->_room[g_vm->_curRoom]._flag & OBJFLAG_EXTRA) {
				if (g_vm->_useWith[USED] == iPINZA)
					PlayDialog(dF212B);
				else
					PlayDialog(dF212);
				printsent = false;
			} else {
				if (g_vm->_useWith[USED] == iPINZA)
					PlayDialog(dF213B);
				else
					PlayDialog(dF213);
				printsent = false;
			}
		} else if ((g_vm->_useWith[WITH] == oDOORC33) && (g_vm->_useWith[USED] == iSBARRA21)) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a333LOSEBAR, 0, 0, g_vm->_useWith[WITH]);
			printsent = false;
		} else if ((g_vm->_useWith[WITH] == oSNAKEU52) && (g_vm->_useWith[USED] == iPINZA)) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a527, 0, 0, g_vm->_useWith[WITH]);
			g_vm->_obj[oSCAVO51]._anim = a516;
			printsent = false;
		} else if ((g_vm->_useWith[WITH] == oSERPENTEA52) && (g_vm->_useWith[USED] == iPINZA)) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a528, 0, 0, g_vm->_useWith[WITH]);
			printsent = false;
		} else if ((g_vm->_useWith[WITH] == oSERPENTEB52) && (g_vm->_useWith[USED] == iPINZA) && (IconPos(iSERPENTEA) == MAXICON)) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a523, 0, 0, g_vm->_useWith[WITH]);
			printsent = false;
		}
		break;

	case iLAMPADINA2B:
		if (g_vm->_useWith[WITH] == oPORTALAMPADE29) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a293AVVITALAMPADINA, 0, 0, g_vm->_useWith[WITH]);
			printsent = false;
		}
		break;

	case iLATTINA27:
		if ((g_vm->_useWith[WITH] == oBRACIERES28) && (g_vm->_obj[oBRACIERES28]._flag & OBJFLAG_EXTRA)) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a286, 0, 0, g_vm->_useWith[WITH]);
			g_vm->_obj[oBRACIERES28]._examine = 456;
			printsent = false;
		} else
			printsent = true;
		break;

	case iTELECOMANDO2G:
		if (g_vm->_useWith[WITH] == oTASTO2F) {
			if (g_vm->_obj[oASCENSORE2F]._mode & OBJMODE_OBJSTATUS) {
				doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a2F10PANNELLOSICHIUDE, 0, 0, g_vm->_useWith[WITH]);
				g_vm->_obj[oBIDONE2F]._anim = a2F5CFRUGABIDONE;
			} else {
				if (!(g_vm->_inventoryObj[iTELECOMANDO2G]._flag & OBJFLAG_EXTRA))
					doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a2F9PPANNELLOSIAPRE, 0, 0, g_vm->_useWith[WITH]);
				else
					doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a2F9PANNELLOSIAPRE, 0, 0, g_vm->_useWith[WITH]);
				g_vm->_inventoryObj[iTELECOMANDO2G]._flag |= OBJFLAG_EXTRA;
				g_vm->_obj[oBIDONE2F]._anim = a2F5FRUGABIDONE;
			}
			printsent = false;
		}
		break;

	case iSAMROTTO:
		if (g_vm->_useWith[WITH] == oSERRATURA33) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a338POSASAM, 0, 0, g_vm->_useWith[WITH]);
			printsent = false;
		}
		break;

	case iTORCIA32:
		if ((g_vm->_useWith[WITH] == oSENSOREV32) && (g_vm->_obj[oFILOTC31]._mode & OBJMODE_OBJSTATUS)) {
			g_vm->_obj[oPANNELLOMA31]._mode |= OBJMODE_OBJSTATUS;
			g_vm->_obj[oPANNELLOM31]._mode &= ~OBJMODE_OBJSTATUS;
			g_vm->_obj[oPANNELLOMA31]._examine = 717;
			g_vm->_obj[oPANNELLOMA31]._action = 718;
			g_vm->_obj[oCORPO31]._mode |= OBJMODE_OBJSTATUS;
			g_vm->_obj[od31ALLA35]._mode |= OBJMODE_OBJSTATUS;
			g_vm->_obj[omd31ALLA35]._mode |= OBJMODE_OBJSTATUS;
			g_vm->_obj[oMONTACARICHI31]._mode |= OBJMODE_OBJSTATUS;
			g_vm->_obj[oPANNELLO31]._mode &= ~OBJMODE_OBJSTATUS;
			g_vm->_obj[oPANNELLON31]._mode &= ~OBJMODE_OBJSTATUS;
			g_vm->_room[r32]._flag |= OBJFLAG_EXTRA;
			doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, r31, 0, 11, g_vm->_useWith[WITH]);

			printsent = false;
		}
		break;

	case iPROIETTORE31:
		if (g_vm->_useWith[WITH] == oPRESA35) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a351PROVAPROIETTOREPRESA, 0, 0, g_vm->_useWith[WITH]);
			printsent = false;
		} else if (g_vm->_useWith[WITH] == oTRIPLA35) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a355ATTPROIETTORETRIPLAEPRESA, 0, 0, g_vm->_useWith[WITH]);
			printsent = false;
		} else if ((g_vm->_useWith[WITH] == oRIBELLEA35)) {
			printsent = false;
			CharacterSay(1578);
		}
		break;

	case iPROIETTORE35:
		if (g_vm->_useWith[WITH] == oPRESA35) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a352ATTPROIETTOREETRIPLAPRESA, 0, 0, g_vm->_useWith[WITH]);
			printsent = false;
		} else if ((g_vm->_useWith[WITH] == oRIBELLEA35)) {
			printsent = false;
			CharacterSay(1590);
		}
		break;

	case iTRIPLA:
		if (g_vm->_useWith[WITH] == oPRESA35) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a353ATTACCATRIPLAPRESA, 0, 0, g_vm->_useWith[WITH]);
			printsent = false;
		}
		break;

	case iLASER35:
		if (g_vm->_useWith[WITH] == oFRONTOFFICEC35) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a3511APRESPORTELLO, 0, 0, g_vm->_useWith[WITH]);
			printsent = false;
		} else if (g_vm->_useWith[WITH] == oSNAKEU52) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a522, 0, 0, g_vm->_useWith[WITH]);
			g_vm->_obj[oSCAVO51]._anim = a516;
			printsent = false;
		} else if (g_vm->_useWith[WITH] == oLUCCHETTO53) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a532, 0, 0, g_vm->_useWith[WITH]);
			printsent = false;
		} else if ((g_vm->_useWith[WITH] == oPORTAMC36) || (g_vm->_useWith[WITH] == oPORTALC36) || (g_vm->_useWith[WITH] == oSCANNERMA36) || (g_vm->_useWith[WITH] == oSCANNERLA36) || (g_vm->_useWith[WITH] == oCASSETTOC36) || (g_vm->_useWith[WITH] == oRETE52) || (g_vm->_useWith[WITH] == oTELECAMERA52) || (g_vm->_useWith[WITH] == oSERPENTET52) || (g_vm->_useWith[WITH] == oLAGO53)) {
			printsent = false;
			CharacterSay(1597);
		}
		break;

	case iKEY35:
		if (g_vm->_useWith[WITH] == oCASSETTOC36) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a364APRECASSETTO, 0, 0, g_vm->_useWith[WITH]);
			printsent = false;
		} else if ((g_vm->_useWith[WITH] == oFRONTOFFICEC35) || (g_vm->_useWith[WITH] == oPORTAC54) || (g_vm->_useWith[WITH] == oPORTA57C55) || (g_vm->_useWith[WITH] == oPORTA58C55) || (g_vm->_useWith[WITH] == oPORTAS56) || (g_vm->_useWith[WITH] == oPORTAS57)) {
			printsent = false;
			CharacterSay(1594);
		}
		break;

	case iSTETOSCOPIO:
		if (g_vm->_useWith[WITH] == oPORTALC36) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a368USASTETOSCOPIO, 0, 0, g_vm->_useWith[WITH]);
			printsent = false;
		}
		break;

	case iCARD36:
		if ((g_vm->_useWith[WITH] == oFRONTOFFICEA35) && !(g_vm->_obj[oFRONTOFFICEA35]._flag & OBJFLAG_EXTRA)) {
			doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, r35P, 0, 10, g_vm->_useWith[WITH]);
			KillIcon(iCARD36);
			printsent = false;
		} else if ((g_vm->_useWith[WITH] == oFRONTOFFICEA35) && (g_vm->_obj[oFRONTOFFICEA35]._flag & OBJFLAG_EXTRA)) {
			CharacterSay(1844);
			printsent = false;
		} else if ((g_vm->_useWith[WITH] == oSLOTA58) || (g_vm->_useWith[WITH] == oSLOTB58)) {
			printsent = false;
			CharacterSay(1602);
		}
		break;

	case iMONETA4L:
		if ((g_vm->_useWith[WITH] == oFESSURA41) &&
		    ((g_vm->_obj[oFUCILE42]._anim != 0) && (g_vm->_obj[oFUCILE42]._anim != a428) && (g_vm->_obj[oFUCILE42]._anim != a429))) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a412, 0, 0, g_vm->_useWith[WITH]);
			if (g_vm->_obj[oZAMPA41]._mode & OBJMODE_OBJSTATUS)
				g_vm->_obj[oSLOT41]._anim = a417;
			else if (Try41 <= 2)
				g_vm->_obj[oSLOT41]._anim = a414;
			else
				CharacterSay(2015);
			Try41 ++;
			printsent = false;
		} else if ((g_vm->_useWith[WITH] == oFESSURA41) &&
		           ((g_vm->_obj[oFUCILE42]._anim == 0) || (g_vm->_obj[oFUCILE42]._anim == a428) || (g_vm->_obj[oFUCILE42]._anim == a429))) {
			CharacterSay(2010);
			printsent = false;
		} else if (g_vm->_useWith[WITH] == oFESSURA42) {
			CharacterSay(924);
			printsent = false;
		} else if (g_vm->_useWith[WITH] == oCAMPANA4U) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a4U3, 0, 0, g_vm->_useWith[WITH]);
			g_vm->_obj[oCAMPANA4U]._flag |= OBJFLAG_EXTRA;
			if ((g_vm->_obj[oCAMPANA4U]._flag & OBJFLAG_EXTRA) && (g_vm->_inventoryObj[iBIGLIAA]._flag & OBJFLAG_EXTRA)) {
				g_vm->_obj[oCAMPANA4U]._examine = 1202;
				g_vm->_obj[oCAMPANA4U]._action = 1203;
			} else
				g_vm->_obj[oCAMPANA4U]._examine = 1200;
			printsent = false;
		}
		break;

	case iMARTELLO:
		if ((g_vm->_useWith[WITH] == oRAGNO41) && !(g_vm->_obj[oRAGNO41]._flag & OBJFLAG_EXTRA)) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a416, 0, 0, g_vm->_useWith[WITH]);
			g_vm->_obj[oRAGNO41]._flag |= OBJFLAG_EXTRA;
			g_vm->_obj[oRAGNO41]._anim = 0;
			if (g_vm->_obj[oSLOT41]._anim == a414)
				g_vm->_obj[oSLOT41]._anim = a417;
			printsent = false;
		} else if ((g_vm->_useWith[WITH] == oSLOT41) || (g_vm->_useWith[WITH] == oVETRINETTA42) || (g_vm->_useWith[WITH] == oTAMBURO43) || (g_vm->_useWith[WITH] == oSFIATO45) || (g_vm->_useWith[WITH] == oPORTAC4A) || (g_vm->_useWith[WITH] == oPORTAC4B) || (g_vm->_useWith[WITH] == oSERRATURA4B) || (g_vm->_useWith[WITH] == oLICANTROPO4P)) {
			printsent = false;
			CharacterSay(1619);
		}
		break;

	case iMONETE:
		if ((g_vm->_useWith[WITH] == oFESSURA42) && (g_vm->_obj[oFUCILE42]._anim == a427)) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a421, 0, 0, g_vm->_useWith[WITH]);
			if (g_vm->_obj[oPOLTIGLIA42]._mode & OBJMODE_OBJSTATUS)
				g_vm->_obj[oFUCILE42]._anim = a429;
			else
				g_vm->_obj[oFUCILE42]._anim = a428;
			printsent = false;
		}
		break;

	case iPOLTIGLIA:
		if (g_vm->_useWith[WITH] == oGUIDE42) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a423, 0, 0, g_vm->_useWith[WITH]);
			if (g_vm->_obj[oFUCILE42]._anim != a427)
				g_vm->_obj[oFUCILE42]._anim = a429;
			printsent = false;
		}
		break;

	case iMAZZA:
		if ((g_vm->_useWith[WITH] == oTAMBURO43) && !(g_vm->_obj[oTAMBURO43]._flag & OBJFLAG_EXTRA)) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a435, 0, 0, g_vm->_useWith[WITH]);
			g_vm->_obj[oTAMBURO43]._flag |= OBJFLAG_EXTRA;
			printsent = false;
		} else if ((g_vm->_useWith[WITH] == oPORTAC4B) || (g_vm->_useWith[WITH] == oSERRATURA4B) || (g_vm->_useWith[WITH] == oLICANTROPO4P)) {
			printsent = false;
			CharacterSay(1679);
		}
		break;

	case iPUPAZZO:
		if (g_vm->_useWith[WITH] == oCASSETTOAA44) {
			ReplaceIcon(iPUPAZZO, iTELECOMANDO44);
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a442, 0, 0, g_vm->_useWith[WITH]);
			printsent = false;
		}
		break;

	case iSTRACCIO:
		if (g_vm->_useWith[WITH] == oMANOPOLAR45) {
			doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, r45S, 0, 2, g_vm->_useWith[WITH]);
			printsent = false;
		} else if ((g_vm->_useWith[WITH] == oCAMPANA4U) && (g_vm->_inventoryObj[iBIGLIAA]._flag & OBJFLAG_EXTRA) && (g_vm->_obj[oCAMPANA4U]._flag & OBJFLAG_EXTRA)) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a4U5, 0, 0, g_vm->_useWith[WITH]);
			g_vm->_obj[oCAMPANA4U]._examine = 1204;
			g_vm->_obj[oCAMPANA4U]._action = 1205;
			printsent = false;
		} else if (g_vm->_useWith[WITH] == oCAMPANA4U) {
			CharacterSay(1713);
			printsent = false;
		} else if ((g_vm->_useWith[WITH] == oRAGNO46) || (g_vm->_useWith[WITH] == oLICANTROPO4P)) {
			printsent = false;
			CharacterSay(1711);
		}
		break;

	case iTESCHIO:
		if (g_vm->_useWith[WITH] == oPIASTRELLA48) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a4810, 0, 0, g_vm->_useWith[WITH]);
			printsent = false;
		}
		break;

	case iTORCIA47:
		if (g_vm->_useWith[WITH] == oTESCHIO48) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a4811, 0, 0, g_vm->_useWith[WITH]);
			printsent = false;
		} else if ((g_vm->_useWith[WITH] == oCALDAIAS45) || (g_vm->_useWith[WITH] == oRAGNO46)) {
			printsent = false;
			CharacterSay(1640);
		}
		break;

	case iFIAMMIFERO:
		if (g_vm->_useWith[WITH] == oTORCIAS48) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a4812, 0, 0, g_vm->_useWith[WITH]);
			g_vm->_obj[oTORCIAS48]._lim[0] = 0;
			g_vm->_obj[oTORCIAS48]._lim[1] = 0;
			g_vm->_obj[oTORCIAS48]._lim[2] = 0;
			g_vm->_obj[oTORCIAS48]._lim[3] = 0;
			printsent = false;
		}
		break;

	case iASTA:
		if (g_vm->_useWith[WITH] == oMERIDIANA49) {
			KillIcon(iASTA);
			StartCharacterAction(a491, r49M, 1, 0);
			printsent = false;
		}
		break;

	case iPISTOLA4PD:
		if ((g_vm->_useWith[WITH] == oLICANTROPO4P) && (g_vm->_inventoryObj[iPISTOLA4PD]._flag & OBJFLAG_EXTRA)) {
			ReplaceIcon(iPISTOLA4PD, iPISTOLA4B);
			g_vm->_obj[oLICANTROPO4P]._mode &= ~OBJMODE_OBJSTATUS;
			g_vm->_obj[oLICANTROPOM4P]._mode |= OBJMODE_OBJSTATUS;
			g_vm->_obj[oSANGUE4P]._mode |= OBJMODE_OBJSTATUS;
			g_vm->_room[r4P]._flag |= OBJFLAG_EXTRA;
			read3D("4p2.3d");		// dopo licantropo
			AnimTab[aBKG4P]._flag |= SMKANIM_OFF1;
			PlayDialog(dF4P2);
			printsent = false;
		} else if (g_vm->_useWith[WITH] == oLICANTROPO4P) {
			ReplaceIcon(iPISTOLA4PD, iPISTOLA4B);
			PlayDialog(dF4P1);
			printsent = false;
		} else if ((g_vm->_useWith[WITH] == oRAGNO46) || (g_vm->_useWith[WITH] == oPORTAC4B) || (g_vm->_useWith[WITH] == oSERRATURA4B)) {
			printsent = false;
			CharacterSay(1706);
		}
		break;

	case iBARATTOLO:
		if ((g_vm->_useWith[WITH] == oSANGUE4P) || (g_vm->_useWith[WITH] == oLICANTROPOM4P)) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a4P7, 0, 0, g_vm->_useWith[WITH]);
			printsent = false;
		}
		break;

	case iCAMPANA:
		if (g_vm->_useWith[WITH] == oPOZZA4U) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a4U2, 0, 0, g_vm->_useWith[WITH]);
			printsent = false;
		}
		break;

	case iBIGLIAA:
	case iBIGLIAB:
		if ((g_vm->_useWith[WITH] == oCAMPANA4U) && (g_vm->_inventoryObj[iBIGLIAA]._flag & OBJFLAG_EXTRA)) {
			CharacterSay(1684);
			printsent = false;
		} else if (g_vm->_useWith[WITH] == oCAMPANA4U) {
			KillIcon(g_vm->_useWith[USED]);
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a4U4, 0, 0, g_vm->_useWith[WITH]);
			g_vm->_inventoryObj[iBIGLIAA]._flag |= OBJFLAG_EXTRA;
			if ((g_vm->_obj[oCAMPANA4U]._flag & OBJFLAG_EXTRA) && (g_vm->_inventoryObj[iBIGLIAA]._flag & OBJFLAG_EXTRA)) {
				g_vm->_obj[oCAMPANA4U]._examine = 1202;
				g_vm->_obj[oCAMPANA4U]._action = 1203;
			} else
				g_vm->_obj[oCAMPANA4U]._examine = 1201;
			printsent = false;
		}
		break;

	case iPAPAVERINA:
		if ((g_vm->_useWith[WITH] == oCIOCCOLATINI4A) && ((_curAnimFrame[0] < 370) || (_curAnimFrame[0] > 480))) {
			PlayScript(s4AHELLEN);
			g_vm->_obj[oPULSANTE4A]._anim = a4A3;
			printsent = false;
		} else if (g_vm->_useWith[WITH] == oCIOCCOLATINI4A) {
			g_vm->_obj[oPULSANTE4A]._anim = a4A3;
			printsent = false;
			g_vm->_obj[oCIOCCOLATINI4A]._flag |= OBJFLAG_EXTRA;
		}
		break;

	case iSANGUE:
		if (g_vm->_useWith[WITH] == oSERRATURA4B) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a4B4, 0, 0, g_vm->_useWith[WITH]);
			g_vm->_obj[oPORTAC4B]._anim = a4B5;
			printsent = false;
		}
		break;

	case iDIAPA4B:
		if ((g_vm->_useWith[WITH] == oPROIETTORE4B) && (g_vm->_obj[oPROIETTORE4B]._anim < a4B9A)) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a4B6A, 0, 0, g_vm->_useWith[WITH]);
			g_vm->_obj[oPROIETTORE4B]._anim = a4B9A;
			printsent = false;
		}
		break;

	case iDIAPB4B:
		if ((g_vm->_useWith[WITH] == oPROIETTORE4B) && (g_vm->_obj[oPROIETTORE4B]._anim < a4B9A)) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a4B6B, 0, 0, g_vm->_useWith[WITH]);
			g_vm->_obj[oPROIETTORE4B]._anim = a4B9B;
			printsent = false;
		}
		break;

	case iDIAP4C:
		if ((g_vm->_useWith[WITH] == oPROIETTORE4B) && (g_vm->_obj[oPROIETTORE4B]._anim < a4B9A)) {
			//
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a4B6C, 0, 0, g_vm->_useWith[WITH]);
			g_vm->_obj[oPROIETTORE4B]._anim = a4B9C;
			printsent = false;
		}
		break;

	case iUOVO:
		if ((g_vm->_useWith[WITH] == oRETE52) || (g_vm->_useWith[WITH] == oSERPENTET52)) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a521, 0, 0, g_vm->_useWith[WITH]);
			printsent = false;
		}
		break;

	case iSERPENTEA:
		if (g_vm->_useWith[WITH] == oLAGO53) {
			printsent = false;
			if (!(g_vm->_obj[oLUCCHETTO53]._mode & OBJMODE_OBJSTATUS)) {
				StartCharacterAction(a533, r54, 11, 0);
				KillIcon(g_vm->_useWith[USED]);
			} else if (g_vm->_useWith[USED] != iSERPENTEB) {
				doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a534, 0, 0, g_vm->_useWith[WITH]);
				KillIcon(g_vm->_useWith[USED]);
				g_vm->_obj[oLAGO53]._examine = 1237;
			} else
				CharacterSay(1740);
		}
		break;

	case iSERPENTEB:
		if (g_vm->_useWith[WITH] == oLAGO53) {
			printsent = false;
			if (!(g_vm->_obj[oLUCCHETTO53]._mode & OBJMODE_OBJSTATUS)) {
				StartCharacterAction(a533C, r54, 11, 0);
				KillIcon(g_vm->_useWith[USED]);
			} else if (g_vm->_useWith[USED] != iSERPENTEB) {
				doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a534, 0, 0, g_vm->_useWith[WITH]);
				KillIcon(g_vm->_useWith[USED]);
				g_vm->_obj[oLAGO53]._examine = 1237;
			} else
				CharacterSay(1740);
		}
		break;

	case iSAPONE:
		if (g_vm->_useWith[WITH] == oSECCHIOA54) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a543, 0, 0, g_vm->_useWith[WITH]);
			printsent = false;
		}
		break;

	case iLATTINE:
		if (g_vm->_useWith[WITH] == oLAVATRICEF54) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a546, 0, 0, g_vm->_useWith[WITH]);
			g_vm->_obj[oLAVATRICEL54]._anim = a547;
			printsent = false;
		}
		break;

	case iCHIAVI:
		if (g_vm->_useWith[WITH] == oPORTAS56) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a563, 0, 0, g_vm->_useWith[WITH]);
			printsent = false;
		} else if (g_vm->_useWith[WITH] == oPORTA57C55) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a551, r57, 17, g_vm->_useWith[WITH]);
			printsent = false;
		} else if ((g_vm->_useWith[WITH] == oPORTA58C55) && (!(_choice[871]._flag & OBJFLAG_DONE) || (_choice[901]._flag & OBJFLAG_DONE))) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a552, 0, 0, g_vm->_useWith[WITH]);
			printsent = false;
		} else if ((g_vm->_useWith[WITH] == oPORTA58C55) && (_choice[871]._flag & OBJFLAG_DONE)) {
			CharacterSay(1287);
			printsent = false;
		}
		break;

	case iMDVD:
		if ((g_vm->_useWith[WITH] == oTASTIERA56) && (_choice[260]._flag & OBJFLAG_DONE) && !(_choice[262]._flag & OBJFLAG_DONE) && (g_vm->_inventoryObj[iMDVD]._examine != 1752)) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a566, 0, 0, g_vm->_useWith[WITH]);
			_choice[262]._flag &= ~DLGCHOICE_HIDE;
			g_vm->_inventoryObj[iMDVD]._examine = 1752;
			printsent = false;
		} else if ((g_vm->_useWith[WITH] == oTASTIERA56) && (g_vm->_inventoryObj[iMDVD]._examine == 1752)) {
			CharacterSay(1753);
			printsent = false;
		} else
			printsent = true;
		break;

	case iTESTER:
		if ((g_vm->_useWith[WITH] == oPANNELLOA) && (_choice[856]._flag & OBJFLAG_DONE)) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a569, 0, 0, g_vm->_useWith[WITH]);
			g_vm->_obj[oPANNELLOA]._flag |= OBJFLAG_EXTRA;
			printsent = false;
		}
		break;

	case iCUTTER:
		if ((g_vm->_useWith[WITH] == oPANNELLOA) && (g_vm->_obj[oPANNELLOA]._flag & OBJFLAG_EXTRA)) {
			PlayDialog(dF562);
			g_vm->_obj[oPANNELLOA]._mode &= ~OBJMODE_OBJSTATUS;
			g_vm->_obj[oCAVOTAGLIATO56]._mode |= OBJMODE_OBJSTATUS;
			g_vm->_obj[oPORTA58C55]._mode |= OBJMODE_OBJSTATUS;
			setPosition(6);
			KillIcon(iCUTTER);
			printsent = false;
		} else if (g_vm->_useWith[WITH] == oPANNELLOA) {
			CharacterSay(2012);
			printsent = false;
		}
		break;

	case iGUANTI59:
		if (g_vm->_useWith[WITH] == oBOMBOLA57) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a575, 0, 0, g_vm->_useWith[WITH]);
			printsent = false;
		}
		break;

	case iARMAEVA:
		if ((g_vm->_useWith[WITH] == oFINESTRAA5A) && (_choice[871]._flag & OBJFLAG_DONE) && !(_choice[286]._flag & OBJFLAG_DONE)) {
			KillIcon(iARMAEVA);
			PlayDialog(dC5A1);
			g_vm->_obj[oFINESTRAA58]._anim = a587;
			printsent = false;
			g_vm->_room[r5A]._flag |= OBJFLAG_EXTRA;
		} else if ((g_vm->_useWith[WITH] == oFINESTRAA5A) && (_choice[871]._flag & OBJFLAG_DONE)) {
			KillIcon(iARMAEVA);
			PlayDialog(dF5A1);
			g_vm->_obj[oFINESTRAA58]._anim = a587;
			printsent = false;
		}
		break;

	default:
		updateinv = false;
		break;
	}

	if (printsent)
		CharacterSay(g_vm->_inventoryObj[g_vm->_useWith[USED]]._action);

	if (updateinv)
		RegenInv(g_vm->_iconBase, INVENTORY_SHOW);
}

/*-------------------------------------------------------------------------*/
/*                          USE WITH / SCR - SCR         				   */
/*-------------------------------------------------------------------------*/
void doScrScrUseWith() {
	bool printsent = true;

	if ((!g_vm->_useWith[USED]) || (!g_vm->_useWith[WITH]))
		warning("doScrScrUseWith");

//	StopSmackAnim(_inventoryObj[_useWith[USED]]._anim);
	if (_characterInMovement)
		return;

	switch (g_vm->_useWith[USED]) {
	case oRAMPINO21:
		if (g_vm->_useWith[WITH] == oTUBO21) {
			PlayDialog(dF211);
			SetRoom(r21, true);
			AnimTab[aBKG21]._flag &= ~SMKANIM_OFF1;
			g_vm->_obj[oRAMPINO21]._mode &= ~OBJMODE_OBJSTATUS;
			g_vm->_obj[oTUBO21]._mode &= ~OBJMODE_OBJSTATUS;
			g_vm->_obj[oCATENAT21]._mode |= OBJMODE_OBJSTATUS;
			printsent = false;
		}
		break;

	case oCAVIE23:
		if (g_vm->_useWith[WITH] == oCAMPO23) {
			doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, r23B, 0, 0, g_vm->_useWith[WITH]);
			printsent = false;
		} else {
			StartCharacterAction(hBOH, 0, 0, 0);
			printsent = false;
		}
		break;

	case oTUBOP33:
		if (g_vm->_useWith[WITH] == oTUBOF33) {
			doEvent(MC_CHARACTER, ME_CHARACTERACTION, MP_DEFAULT, a337PRENDETUBO, 0, 0, g_vm->_useWith[WITH]);
			g_vm->_obj[oTUBOF34]._examine = 1832;
			g_vm->_obj[oTUBOFT34]._examine = 773;
			//g_vm->_obj[oVALVOLAC34]._anim = a344APREVALVOLA;
			printsent = false;
		}
		break;

	case oTUBOT33:
		if (((g_vm->_useWith[WITH] == oSAMA33) || (g_vm->_useWith[WITH] == oSERRATURA33 && g_vm->_obj[oSAMA33]._mode & OBJMODE_OBJSTATUS)) && (g_vm->_obj[oVALVOLA34]._mode & OBJMODE_OBJSTATUS)) {
			PlayDialog(dF331);
			setPosition(10);
			g_vm->_obj[oBOTOLAC33]._mode &= ~OBJMODE_OBJSTATUS;
			g_vm->_obj[oSERRATURA33]._mode &= ~OBJMODE_OBJSTATUS;
			g_vm->_obj[oBOTOLAA33]._mode |= OBJMODE_OBJSTATUS;
			g_vm->_obj[oBRUCIATURA33]._mode |= OBJMODE_OBJSTATUS;
			g_vm->_obj[oSERRATURAF33]._mode |= OBJMODE_OBJSTATUS;
			g_vm->_obj[oSAMD33]._mode |= OBJMODE_OBJSTATUS;
			g_vm->_obj[oTUBOS33]._mode |= OBJMODE_OBJSTATUS;

			g_vm->_obj[oBOTOLAC32]._mode &= ~OBJMODE_OBJSTATUS;
			g_vm->_obj[oBOTOLAB32]._mode |= OBJMODE_OBJSTATUS;
			g_vm->_obj[oBOTOLAA32]._mode |= OBJMODE_OBJSTATUS;
			g_vm->_obj[oPULSANTI32]._anim = 0;

			g_vm->_obj[oSIGARO31]._mode |= OBJMODE_OBJSTATUS;
			g_vm->_obj[oPANNELLOM31]._anim = a314APREPANNELLO;
			g_vm->_obj[oPANNELLOM31]._examine = 713;
			g_vm->_obj[oPANNELLOM31]._action = 714;
			g_vm->_obj[oPANNELLOM31]._flag |= OBJFLAG_ROOMOUT;
			g_vm->_obj[oPANNELLON31]._mode |= OBJMODE_OBJSTATUS;

			g_vm->_obj[oMANOMETROC34]._mode |= OBJMODE_OBJSTATUS;
			g_vm->_obj[oMANOMETRO34]._mode &= ~OBJMODE_OBJSTATUS;
			g_vm->_obj[oMANOMETROC34]._examine = 804;
			g_vm->_obj[oVALVOLAC34]._anim = 0;
			g_vm->_obj[oVALVOLA34]._anim = 0;
			g_vm->_obj[oVALVOLAC34]._action = 1831;
			g_vm->_obj[oVALVOLA34]._action = 1831;

			g_vm->_obj[oTUBOF34]._examine = 1832;
			g_vm->_obj[oTUBOFT34]._examine = 784;

			printsent = false;
		}
		break;

	case oFILOS31:
		if (g_vm->_useWith[WITH] == oCONTATTOP31) {
			g_vm->_obj[oFILOS31]._mode &= ~OBJMODE_OBJSTATUS;
			g_vm->_obj[oCONTATTOP31]._mode &= ~OBJMODE_OBJSTATUS;
			g_vm->_obj[oFILOTC31]._mode |= OBJMODE_OBJSTATUS;
			CharacterSay(746);
			RegenRoom();

			printsent = false;
		}
		break;

	}

	if (printsent)
		CharacterSay(g_vm->_obj[g_vm->_useWith[USED]]._action);
}

/*-------------------------------------------------------------------------*/
/*                               EXAMINE INV          					   */
/*-------------------------------------------------------------------------*/
void doInvExamine() {
	if (!g_vm->_curInventory)
		warning("doInvExamine");

	if (g_vm->_inventoryObj[g_vm->_curInventory]._examine)
		CharacterSay(g_vm->_inventoryObj[g_vm->_curInventory]._examine);
}

/*-------------------------------------------------------------------------*/
/*                               OPERATE INV          					   */
/*-------------------------------------------------------------------------*/
void doInvOperate() {
	bool printsent = true;

	if (!g_vm->_curInventory)
		warning("doInvOperate");

	switch (g_vm->_curInventory) {
	case iSAM:
		if ((_choice[166]._flag & OBJFLAG_DONE) && ((g_vm->_curRoom == r17) || (g_vm->_curRoom == r1B) || (g_vm->_curRoom == r1C) || (g_vm->_curRoom == r1D))) {
			if (g_vm->_obj[oNUMERO17]._mode & OBJMODE_OBJSTATUS) {
				if (!(_choice[196]._flag & OBJFLAG_DONE)) {
					if (g_vm->_curRoom == r17) {
						if (_choice[198]._flag & OBJFLAG_DONE) {
							g_vm->_inventoryObj[iSAM]._action = 1787;
							printsent = true;
						} else {
							_choice[197]._flag |= DLGCHOICE_HIDE;
							_choice[198]._flag &= ~DLGCHOICE_HIDE;
							PlayDialog(dSAM17);
							printsent = false;
							g_vm->_obj[oFAX17]._mode |= OBJMODE_OBJSTATUS;
						}
					} else {
						if (_choice[199]._flag & OBJFLAG_DONE) {
							g_vm->_inventoryObj[iSAM]._action = 1787;
							printsent = true;
						} else {
							_choice[197]._flag |= DLGCHOICE_HIDE;
							_choice[199]._flag &= ~DLGCHOICE_HIDE;
							PlayDialog(dSAM17);
							printsent = false;
							g_vm->_obj[oFAX17]._mode |= OBJMODE_OBJSTATUS;
						}
					}
				} else {
					if ((_choice[198]._flag & OBJFLAG_DONE) || (_choice[199]._flag & OBJFLAG_DONE)) {
						g_vm->_inventoryObj[iSAM]._action = 1787;
						printsent = true;
					} else if (g_vm->_curRoom == r17) {
						if (_choice[201]._flag & OBJFLAG_DONE) {
							g_vm->_inventoryObj[iSAM]._action = 1787;
							printsent = true;
						} else {
							_choice[201]._flag &= ~DLGCHOICE_HIDE;
							PlayDialog(dSAM17);
							printsent = false;
							g_vm->_obj[oFAX17]._mode |= OBJMODE_OBJSTATUS;
						}
					} else {
						if (_choice[200]._flag & OBJFLAG_DONE) {
							g_vm->_inventoryObj[iSAM]._action = 1787;
							printsent = true;
						} else {
							_choice[200]._flag &= ~DLGCHOICE_HIDE;
							PlayDialog(dSAM17);
							printsent = false;
							g_vm->_obj[oFAX17]._mode |= OBJMODE_OBJSTATUS;
						}
					}
				}
			} else if (_choice[197]._flag & OBJFLAG_DONE) {
				g_vm->_inventoryObj[iSAM]._action = 1786;
				printsent = true;
			} else {
				PlayDialog(dSAM17);
				printsent = false;
			}
		}
		break;

	case iMAPPA17:
		if (g_vm->_curRoom == r23A) {
			CharacterSay(361);
			printsent = false;
		} else
			printsent = true;
		break;

	case iLIBRO:
		doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, r2BL, 0, 0, g_vm->_useWith[WITH]);
		g_vm->_obj[oEXIT2BL]._goRoom = g_vm->_curRoom;
		actorStop();
		nextStep();
		printsent = false;
		break;

	case iFOGLIO36:
		doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, r36F, 0, 0, g_vm->_useWith[WITH]);
		g_vm->_obj[oEXIT36F]._goRoom = g_vm->_curRoom;
		actorStop();
		nextStep();
		printsent = false;
		break;

	case iDISLOCATORE:
		for (int a = oROOM41; a <= oROOM45B; a++)
			g_vm->_obj[a]._mode &= ~OBJMODE_OBJSTATUS;
		doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, r41D, 0, 0, g_vm->_useWith[WITH]);
		g_vm->_obj[oEXIT41D]._goRoom = g_vm->_curRoom;
		g_vm->_inventoryObj[iDISLOCATORE]._flag &= ~OBJFLAG_EXTRA;
		actorStop();
		nextStep();
		printsent = false;
		break;

	case iCODICE:
		g_vm->_obj[oEXIT58M]._goRoom = g_vm->_curRoom;
		doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, r58M, 0, 0, g_vm->_useWith[WITH]);
		actorStop();
		nextStep();
		printsent = false;
		break;

	case iTELECOMANDO44:
		if ((_actor._px < 5057.6) && (g_vm->_obj[oPUPAZZO44]._mode & OBJMODE_OBJSTATUS) && (g_vm->_curRoom == r43)) {
			doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, r46, 0, 7, g_vm->_useWith[WITH]);
			printsent = false;
		}
		break;

	case iMAPPA50:
		if ((g_vm->_curRoom >= r51) && (g_vm->_curRoom <= r5A))
			g_vm->_inventoryObj[iMAPPA50]._action = 1725 + (g_vm->_curRoom - r51);
		printsent = true;
		break;

	}

	if (g_vm->_inventoryObj[g_vm->_curInventory]._action && printsent)
		CharacterSay(g_vm->_inventoryObj[g_vm->_curInventory]._action);
}

/*-------------------------------------------------------------------------*/
/*                                 doDoing           					   */
/*-------------------------------------------------------------------------*/
void doDoing() {
	switch (g_vm->_curMessage->_event) {
	case ME_INITOPENCLOSE:
		if (_actor._curAction == hSTAND)
			REEVENT;
		else if (_actor._curFrame == 4)
			doEvent(g_vm->_curMessage->_class, ME_OPENCLOSE, g_vm->_curMessage->_priority, g_vm->_curMessage->_u16Param1, g_vm->_curMessage->_u16Param2, g_vm->_curMessage->_u8Param, g_vm->_curMessage->_u32Param);
		else
			REEVENT;

		break;
	case ME_OPENCLOSE: {
		uint16 curObj = g_vm->_curMessage->_u16Param1;
		uint16 curAnim = g_vm->_curMessage->_u16Param2;
		g_vm->_obj[curObj]._mode &= ~OBJMODE_OBJSTATUS;
		RegenRoom();
		if (curAnim)
			doEvent(MC_ANIMATION, ME_ADDANIM, MP_SYSTEM, curAnim, 0, 0, 0);

		g_vm->_curMessage->_event = ME_WAITOPENCLOSE;
		}
		// no break!
	case ME_WAITOPENCLOSE:
		RegenRoom();
		if (_actor._curAction == hSTAND)
			FlagMouseEnabled = true;
		break;
	}
}

/*-------------------------------------------------------------------------*/
/*                                doScript           					   */
/*-------------------------------------------------------------------------*/
void doScript() {
	static uint32 me_pausestarttime = 0;
	uint8 campo = g_vm->_curMessage->_u8Param;
	uint16 indice = g_vm->_curMessage->_u16Param1;
	uint16 indice2 = g_vm->_curMessage->_u16Param2;
	uint32 valore = g_vm->_curMessage->_u32Param;

	switch (g_vm->_curMessage->_event) {

	case ME_PAUSE:
		if (!me_pausestarttime) {
			me_pausestarttime = TheTime;
			doEvent(g_vm->_curMessage->_class, g_vm->_curMessage->_event, g_vm->_curMessage->_priority, g_vm->_curMessage->_u16Param1, g_vm->_curMessage->_u16Param2, g_vm->_curMessage->_u8Param, g_vm->_curMessage->_u32Param);
		} else {
			if (TheTime >= (me_pausestarttime + g_vm->_curMessage->_u16Param1))
				me_pausestarttime = 0;
			else
				doEvent(g_vm->_curMessage->_class, g_vm->_curMessage->_event, g_vm->_curMessage->_priority, g_vm->_curMessage->_u16Param1, g_vm->_curMessage->_u16Param2, g_vm->_curMessage->_u8Param, g_vm->_curMessage->_u32Param);
		}
		break;

	case ME_SETOBJ:
		switch (campo) {
		case C_ONAME:
			g_vm->_obj[indice]._name = (uint16)valore;
			break;
		case C_OEXAMINE:
			g_vm->_obj[indice]._examine = (uint16)valore;
			break;
		case C_OACTION:
			g_vm->_obj[indice]._action = (uint16)valore;
			break;
		case C_OGOROOM:
			g_vm->_obj[indice]._goRoom = (uint8)valore;
			break;
		case C_OMODE:
			if (valore)
				g_vm->_obj[indice]._mode |= (uint8)indice2;
			else
				g_vm->_obj[indice]._mode &= ~(uint8)indice2;
			break;
		case C_OFLAG:
			if (valore)
				g_vm->_obj[indice]._flag |= (uint8)indice2;
			else
				g_vm->_obj[indice]._flag &= ~(uint8)indice2;
			break;

		}
		break;

	case ME_SETINVOBJ:
		switch (campo) {
		case C_INAME:
			g_vm->_inventoryObj[indice]._name = (uint16)valore;
			break;
		case C_IEXAMINE:
			g_vm->_inventoryObj[indice]._examine = (uint16)valore;
			break;
		case C_IACTION:
			g_vm->_inventoryObj[indice]._action = (uint16)valore;
			break;
		case C_IFLAG:
			if (valore)
				g_vm->_inventoryObj[indice]._flag |= (uint8)indice2;
			else
				g_vm->_inventoryObj[indice]._flag &= ~(uint8)indice2;
			break;

		}
		break;

	case ME_ADDICON:
		AddIcon(indice);
		break;

	case ME_KILLICON:
		KillIcon(indice);
		break;

	case ME_PLAYDIALOG:
		PlayDialog(indice);
		break;

	case ME_CHARACTERSAY:
		CharacterSay(g_vm->_curMessage->_u32Param);
		break;

	case ME_PLAYSOUND:
		NLPlaySound(indice);
		break;

	case ME_STOPSOUND:
		NLStopSound(indice);
		break;

	case ME_REGENROOM:
		RegenRoom();
		break;

	case ME_CHANGER:
		doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, indice, indice2, valore, g_vm->_curObj);
		break;
	}
}

/*-------------------------------------------------------------------------*/
/*                               AtMouseClick           				   */
/*-------------------------------------------------------------------------*/
bool AtMouseClick(uint16 TheObj) {
	bool ret = true;

	if ((g_vm->_curRoom == r1D) && !(g_vm->_room[r1D]._flag & OBJFLAG_EXTRA) && (TheObj != oSCALA1D)) {
		g_vm->_curObj = oDONNA1D;
		goToPosition(g_vm->_obj[oDONNA1D]._position);
		return true;
	}

	if ((g_vm->_curRoom == r2B) && (g_vm->_room[r2B]._flag & OBJFLAG_EXTRA) && (TheObj != oCARTELLO2B) && (TheObj != od2BALLA28)) {
		g_vm->_curObj = oPORTA2B;
		goToPosition(g_vm->_obj[oCARTELLO2B]._position);
		return true;
	}

	switch (TheObj) {
	case oPORTAA13:
		if ((mright) && (g_vm->_room[r14]._flag & OBJFLAG_DONE)) {
			if (_characterGoToPosition != 4)
				goToPosition(4);
		} else if (_characterGoToPosition != g_vm->_obj[TheObj]._position)
			goToPosition(g_vm->_obj[TheObj]._position);

		ret = true;
		break;

	case oDIVANO14:
		if (mleft) {
			if (_characterGoToPosition != 2)
				goToPosition(2);
		} else if (_characterGoToPosition != g_vm->_obj[TheObj]._position)
			goToPosition(g_vm->_obj[TheObj]._position);

		ret = true;
		break;

	case oSCAFFALE1D:
		if (mright) {
			if (_characterGoToPosition != 9)
				goToPosition(9);
		} else if (_characterGoToPosition != g_vm->_obj[TheObj]._position)
			goToPosition(g_vm->_obj[TheObj]._position);

		ret = true;
		break;

	case oDIVANOR4A:
		if (mright) {
			if (_characterGoToPosition != 1)
				goToPosition(1);
		} else if (_characterGoToPosition != g_vm->_obj[TheObj]._position)
			goToPosition(g_vm->_obj[TheObj]._position);

		ret = true;
		break;

	case oDOORC21:
		if ((g_vm->_useWith[USED] == iPISTOLA15) && (FlagUseWithStarted)) {
			if (_characterGoToPosition != 1)
				goToPosition(1);
		} else if (_characterGoToPosition != g_vm->_obj[TheObj]._position)
			goToPosition(g_vm->_obj[TheObj]._position);

		ret = true;
		break;

	case oTUBO21:
		if ((g_vm->_useWith[USED] == oRAMPINO21) && (FlagUseWithStarted)) {
			if (_characterGoToPosition != 4)
				goToPosition(4);
		} else if (_characterGoToPosition != g_vm->_obj[TheObj]._position)
			goToPosition(g_vm->_obj[TheObj]._position);

		ret = true;
		break;

	case oCAMPO23:
		if ((g_vm->_useWith[USED] == oCAVIE23) && (FlagUseWithStarted)) {
			if (_characterGoToPosition != 2)
				goToPosition(2);
		} else if (_characterGoToPosition != g_vm->_obj[TheObj]._position)
			goToPosition(g_vm->_obj[TheObj]._position);

		ret = true;
		break;

	case oTASTO2F:
		if ((g_vm->_useWith[USED] == iTELECOMANDO2G) && (FlagUseWithStarted)) {
			if (_characterGoToPosition != 9)
				goToPosition(9);
		} else if (_characterGoToPosition != g_vm->_obj[TheObj]._position)
			goToPosition(g_vm->_obj[TheObj]._position);

		ret = true;
		break;

	case oSAMA33:
	case oSERRATURA33:
		if ((g_vm->_useWith[USED] == oTUBOT33) && (g_vm->_obj[oVALVOLA34]._mode & OBJMODE_OBJSTATUS) && (g_vm->_obj[oSAMA33]._mode & OBJMODE_OBJSTATUS) && (FlagUseWithStarted)) {
			if (_characterGoToPosition != 4)
				goToPosition(4);
		} else if (_characterGoToPosition != g_vm->_obj[TheObj]._position)
			goToPosition(g_vm->_obj[TheObj]._position);

		ret = true;
		break;

	case oFINESTRA33:
		if (mleft) {
			if (_characterGoToPosition != 7)
				goToPosition(7);
		} else if (_characterGoToPosition != g_vm->_obj[TheObj]._position)
			goToPosition(g_vm->_obj[TheObj]._position);

		ret = true;
		break;

	case oPRESA35:
		if (!(FlagUseWithStarted)) {
			if (_characterGoToPosition != 2)
				goToPosition(2);
		} else if (_characterGoToPosition != g_vm->_obj[TheObj]._position)
			goToPosition(g_vm->_obj[TheObj]._position);

		ret = true;
		break;

	case oTRIPLA35:
		if (!(FlagUseWithStarted) && (mleft)) {
			if (_characterGoToPosition != 2)
				goToPosition(2);
		} else if (_characterGoToPosition != g_vm->_obj[TheObj]._position)
			goToPosition(g_vm->_obj[TheObj]._position);

		ret = true;
		break;

	case oPORTALC36:
		if ((g_vm->_useWith[USED] == iSTETOSCOPIO) && (FlagUseWithStarted)) {
			if (_characterGoToPosition != 8)
				goToPosition(8);
		} else if (_characterGoToPosition != g_vm->_obj[TheObj]._position)
			goToPosition(g_vm->_obj[TheObj]._position);

		ret = true;
		break;

	case oSERBATOIOA2G:
		if ((g_vm->_useWith[USED] == iCANDELOTTO) && (g_vm->_inventoryObj[iBOMBOLA]._flag & OBJFLAG_EXTRA) && (g_vm->_inventoryObj[iLATTINA28]._flag & OBJFLAG_EXTRA) && (FlagUseWithStarted)) {
			if (_characterGoToPosition != 6)
				goToPosition(6);
		} else if (_characterGoToPosition != g_vm->_obj[TheObj]._position)
			goToPosition(g_vm->_obj[TheObj]._position);

		ret = true;
		break;

	case oRAGNO41:
		if ((g_vm->_useWith[USED] == iMARTELLO) && (FlagUseWithStarted)) {
			if (_characterGoToPosition != 1)
				goToPosition(1);
		} else if (_characterGoToPosition != g_vm->_obj[TheObj]._position)
			goToPosition(g_vm->_obj[TheObj]._position);

		ret = true;
		break;

	case oTAMBURO43:
		if ((g_vm->_useWith[USED] == iMAZZA) && (FlagUseWithStarted)) {
			if (_characterGoToPosition != 5)
				goToPosition(5);
		} else if (_characterGoToPosition != g_vm->_obj[TheObj]._position)
			goToPosition(g_vm->_obj[TheObj]._position);

		ret = true;
		break;

	case oLUCCHETTO53:
		if ((g_vm->_useWith[USED] == iLASER35) && (FlagUseWithStarted)) {
			if (_characterGoToPosition != 2)
				goToPosition(2);
		} else if (_characterGoToPosition != g_vm->_obj[TheObj]._position)
			goToPosition(g_vm->_obj[TheObj]._position);

		ret = true;
		break;

	case oPORTA58C55:
		if ((g_vm->_useWith[USED] == iCHIAVI) && (FlagUseWithStarted)) {
			if (_characterGoToPosition != 2)
				goToPosition(2);
		} else if (_characterGoToPosition != g_vm->_obj[TheObj]._position)
			goToPosition(g_vm->_obj[TheObj]._position);

		ret = true;
		break;

	default:
		if (g_vm->_obj[TheObj]._position == -1) {
			_characterGoToPosition = -1;
			ret = false;
		} else {
			if (_characterGoToPosition != g_vm->_obj[TheObj]._position)
				goToPosition(g_vm->_obj[TheObj]._position);
			ret = true;
		}
		break;
	}

	if (g_vm->_room[g_vm->_curRoom]._flag & OBJFLAG_EXTRA) {
		if ((TheObj == oTUBO21) || (TheObj == oCARTELLONE21) || (TheObj == oESSE21) || (TheObj == oRAMPINO21) || (TheObj == oCATENA21) || (TheObj == od21ALLA22) || (TheObj == oDOORC21) || (TheObj == oPORTAA21) || (TheObj == oCUNICOLO21) || (TheObj == od24ALLA23) || (TheObj == od2EALLA2C) || (TheObj == od2GVALLA26)) {
			_characterGoToPosition = -1;
			ret = true;
		}
	} else if ((TheObj == od21ALLA23) || (TheObj == od24ALLA26) || (TheObj == oENTRANCE2E) || (TheObj == oCARTELLO2B) || (TheObj == oFRONTOFFICEC35) || (TheObj == oFRONTOFFICEA35) || (TheObj == oASCENSORE35) || (TheObj == oGIORNALE35)) {
		_characterGoToPosition = -1;
		ret = true;
	}
	return ret;
}

/* -----------------26/11/97 10.37-------------------
 * 					AtEndChangeRoom
 * --------------------------------------------------*/
void AtEndChangeRoom() {
	//	Specific management of magnetic fields
	if ((g_vm->_curRoom == r2E) && (g_vm->_obj[oPULSANTEADS2D]._mode & OBJMODE_OBJSTATUS)) {
		AnimTab[aBKG2E]._flag |= SMKANIM_OFF1;
		g_vm->_obj[oCAMPO2E]._mode &= ~OBJMODE_OBJSTATUS;
		g_vm->_obj[oCARTELLOA2E]._mode &= ~OBJMODE_OBJSTATUS;
		g_vm->_obj[oCARTELLOS2E]._mode |= OBJMODE_OBJSTATUS;
	} else if ((g_vm->_curRoom == r2E) && (g_vm->_obj[oPULSANTEADA2D]._mode & OBJMODE_OBJSTATUS)) {
		AnimTab[aBKG2E]._flag &= ~SMKANIM_OFF1;
		g_vm->_obj[oCAMPO2E]._mode |= OBJMODE_OBJSTATUS;
		g_vm->_obj[oCARTELLOA2E]._mode |= OBJMODE_OBJSTATUS;
		g_vm->_obj[oCARTELLOS2E]._mode &= ~OBJMODE_OBJSTATUS;
	}
	if ((g_vm->_curRoom == r2C) && (g_vm->_obj[oPULSANTEACS2D]._mode & OBJMODE_OBJSTATUS)) {
		AnimTab[aBKG2C]._flag |= SMKANIM_OFF1;
		g_vm->_obj[oCAMPO2C]._mode &= ~OBJMODE_OBJSTATUS;
		g_vm->_obj[oCARTELLOA2C]._mode &= ~OBJMODE_OBJSTATUS;
		g_vm->_obj[oCARTELLOS2C]._mode |= OBJMODE_OBJSTATUS;
	} else if ((g_vm->_curRoom == r2C) && (g_vm->_obj[oPULSANTEACA2D]._mode & OBJMODE_OBJSTATUS)) {
		AnimTab[aBKG2C]._flag &= ~SMKANIM_OFF1;
		g_vm->_obj[oCAMPO2C]._mode |= OBJMODE_OBJSTATUS;
		g_vm->_obj[oCARTELLOS2C]._mode &= ~OBJMODE_OBJSTATUS;
		g_vm->_obj[oCARTELLOA2C]._mode |= OBJMODE_OBJSTATUS;
	}
	if ((g_vm->_curRoom == r28) && (g_vm->_obj[oPULSANTEBBS2D]._mode & OBJMODE_OBJSTATUS)) {
		AnimTab[aBKG28]._flag |= SMKANIM_OFF2;
		g_vm->_obj[oCESPUGLIO28]._anim = a283;
		g_vm->_obj[oSERPENTETM28]._mode |= OBJMODE_OBJSTATUS;
		g_vm->_obj[oSERPENTEAM28]._mode |= OBJMODE_OBJSTATUS;
		RegenRoom();
	} else if ((g_vm->_curRoom == r28) && (g_vm->_obj[oPULSANTEBBA2D]._mode & OBJMODE_OBJSTATUS)) {
		AnimTab[aBKG28]._flag &= ~SMKANIM_OFF2;
		g_vm->_obj[oCESPUGLIO28]._anim = a282;
		g_vm->_obj[oSERPENTETM28]._mode &= ~OBJMODE_OBJSTATUS;
		g_vm->_obj[oSERPENTEAM28]._mode &= ~OBJMODE_OBJSTATUS;
		RegenRoom();
	}
	if ((g_vm->_curRoom == r28) && (g_vm->_obj[oPULSANTEABS2D]._mode & OBJMODE_OBJSTATUS)) {
		AnimTab[aBKG28]._flag |= (SMKANIM_OFF1 | SMKANIM_OFF3);
		g_vm->_obj[oCAMPO28]._mode &= ~OBJMODE_OBJSTATUS;
		g_vm->_obj[oCARTELLOA28]._mode &= ~OBJMODE_OBJSTATUS;
		g_vm->_obj[oCARTELLOS28]._mode |= OBJMODE_OBJSTATUS;
		read3D("282.3d");
	} else if ((g_vm->_curRoom == r28) && (g_vm->_obj[oPULSANTEABA2D]._mode & OBJMODE_OBJSTATUS)) {
		AnimTab[aBKG28]._flag &= ~(SMKANIM_OFF1 | SMKANIM_OFF3);
		g_vm->_obj[oCAMPO28]._mode |= OBJMODE_OBJSTATUS;
		g_vm->_obj[oCARTELLOA28]._mode |= OBJMODE_OBJSTATUS;
		g_vm->_obj[oCARTELLOS28]._mode &= ~OBJMODE_OBJSTATUS;
	}
	// End of specific management for the magnetic fields

	// Actions
	if ((g_vm->_curRoom == r12) && ((g_vm->_oldRoom == r13) || (g_vm->_oldRoom == r16)))
		StartCharacterAction(a1213ARRIVACONASCENSORE, 0, 0, 0);
	else if ((g_vm->_curRoom == r16) && ((g_vm->_oldRoom == r13) || (g_vm->_oldRoom == r12)))
		StartCharacterAction(a1617SCENDECONASCENSORE, 0, 0, 0);
	else if ((g_vm->_curRoom == r13) && (g_vm->_oldRoom == r12))
		StartCharacterAction(a1315ARRIVAASCENSOREALTOENTRA, 0, 0, 0);
	else if ((g_vm->_curRoom == r13) && (g_vm->_oldRoom == r16))
		StartCharacterAction(a1314ARRIVAASCENSOREDABASSO, 0, 0, 0);
	else if ((g_vm->_curRoom == r14) && (g_vm->_oldRoom == r13))
		StartCharacterAction(a1414ENTERROOM, 0, 0, 0);
	else if ((g_vm->_curRoom == r18) && (g_vm->_oldRoom == r17) && !(g_vm->_room[r18]._flag & OBJFLAG_DONE))
		StartCharacterAction(a186GUARDAPIAZZA, 0, 0, 0);
	else if ((g_vm->_curRoom == r18) && (g_vm->_oldRoom == r19))
		StartCharacterAction(a189ENTRADALCLUB, 0, 0, 0);
	else if ((g_vm->_curRoom == r1A) && (g_vm->_oldRoom == r18) && (g_vm->_room[r1A]._flag & OBJFLAG_DONE)) {
		StartCharacterAction(a1A5ENTRA, 0, 0, 0);
		AnimTab[aBKG1A]._flag |= SMKANIM_OFF1;
	} else if ((g_vm->_curRoom == r1C) && (g_vm->_oldRoom == r1B))
		StartCharacterAction(a1C1ENTRACUNICOLO, 0, 0, 0);
	else if ((g_vm->_curRoom == r1B) && (g_vm->_oldRoom == r1C))
		StartCharacterAction(a1B11ESCETOMBINO, 0, 0, 0);
	else if ((g_vm->_curRoom == r1D) && (g_vm->_oldRoom == r1B) && (g_vm->_obj[oDONNA1D]._mode & OBJMODE_OBJSTATUS)) {
		StartCharacterAction(a1D1SCENDESCALE, 0, 1, 0);
		_actor._lim[0] = 60;
		_actor._lim[2] = 240;
	} else if ((g_vm->_curRoom == r1D) && (g_vm->_oldRoom == r1B) && !(g_vm->_obj[oDONNA1D]._mode & OBJMODE_OBJSTATUS)) {
		StartCharacterAction(a1D12SCENDESCALA, 0, 1, 0);
		_actor._lim[0] = 60;
		_actor._lim[2] = 240;
	} else if ((g_vm->_curRoom == r1B) && (g_vm->_oldRoom == r1D))
		StartCharacterAction(a1B4ESCEBOTOLA, 0, 0, 0);
	else if ((g_vm->_curRoom == r1B) && (g_vm->_oldRoom == r18) && (AnimTab[aBKG1B]._flag & SMKANIM_OFF1))
		StartCharacterAction(a1B12SCAPPATOPO, 0, 0, 0);
	else if ((g_vm->_curRoom == r24) && (g_vm->_oldRoom == r2H))
		StartCharacterAction(a242, 0, 0, 0);
	else if ((g_vm->_curRoom == r25) && (g_vm->_oldRoom == r2A))
		StartCharacterAction(a257, 0, 0, 0);
	else if ((g_vm->_curRoom == r28) && (g_vm->_oldRoom == r27))
		StartCharacterAction(aWALKIN, 0, 0, 0);
	else if ((g_vm->_curRoom == r29) && (g_vm->_oldRoom == r2A))
		StartCharacterAction(a298ESCEBOTOLA, 0, 0, 0);
	else if ((g_vm->_curRoom == r29L) && (g_vm->_oldRoom == r2A))
		StartCharacterAction(a2910ESCEBOTOLA, 0, 0, 0);
	else if ((g_vm->_curRoom == r2A) && (g_vm->_oldRoom == r25))
		StartCharacterAction(aWALKIN, 0, 0, 0);
	else if ((g_vm->_curRoom == r2A) && ((g_vm->_oldRoom == r29) || (g_vm->_oldRoom == r29L)))
		StartCharacterAction(aWALKIN, 0, 0, 0);
	else if ((g_vm->_curRoom == r2B) && (g_vm->_oldRoom == r2A))
		StartCharacterAction(a2B2ESCEPOZZO, 0, 2, 0);
	else if ((g_vm->_curRoom == r2H) && (g_vm->_oldRoom == r24))
		StartCharacterAction(a2H1ARRIVA, 0, 0, 0);
	else if ((g_vm->_curRoom == r2E) && (g_vm->_oldRoom == r2F))
		StartCharacterAction(a2E5SECONDAPARRIVANDO, 0, 0, 0);
	else if ((g_vm->_curRoom == r2F) && (g_vm->_oldRoom == r2E))
		StartCharacterAction(aWALKIN, 0, 0, 0);
	else if ((g_vm->_curRoom == r23B) && (g_vm->_oldRoom == r21))
		StartCharacterAction(aWALKIN, 0, 0, 0);
	else if ((g_vm->_curRoom == r23A) && (g_vm->_oldRoom == r21) && (g_vm->_room[r23A]._flag & OBJFLAG_DONE))
		StartCharacterAction(aWALKIN, 0, 0, 0);
	else if ((g_vm->_curRoom == r23A) && (g_vm->_oldRoom == r21) && (!(g_vm->_room[r23A]._flag & OBJFLAG_DONE)))
		FlagShowCharacter = false;
	else if ((g_vm->_curRoom == r21) && ((g_vm->_oldRoom == r23A) || (g_vm->_oldRoom == r23B)))
		StartCharacterAction(aWALKIN, 0, 0, 0);
	else if ((g_vm->_curRoom == r2BL) || (g_vm->_curRoom == r36F) || (g_vm->_curRoom == r41D) || (g_vm->_curRoom == r49M) || (g_vm->_curRoom == r4CT) ||
	         (g_vm->_curRoom == r58T) || (g_vm->_curRoom == r58M) || (g_vm->_curRoom == r59L) || (g_vm->_curRoom == rSYS) ||
	         (g_vm->_curRoom == r12CU) || (g_vm->_curRoom == r13CU)) { // Screens without inventory
		FlagShowCharacter = false;
		FlagCharacterExist = false;
		FlagInventoryLocked = true;
	} else if ((g_vm->_curRoom == r31P) || (g_vm->_curRoom == r35P)) { // Screens with inventory
		FlagShowCharacter = false;
		FlagCharacterExist = false;
	} else if ((g_vm->_curRoom == r2F) && (g_vm->_oldRoom == r31))
		StartCharacterAction(a2F4ESCEASCENSORE, 0, 0, 0);
	else if ((g_vm->_curRoom == r31) && (g_vm->_oldRoom == r2F))
		StartCharacterAction(a3114ESCEPASSAGGIO, 0, 0, 0);
	else if ((g_vm->_curRoom == r31) && !(g_vm->_room[r31]._flag & OBJFLAG_DONE))
		setPosition(14);
	else if ((g_vm->_curRoom == r32) && (g_vm->_oldRoom == r31))
		StartCharacterAction(a321SALEMONTACARICHI, 0, 0, 0);
	else if ((g_vm->_curRoom == r33) && (g_vm->_oldRoom == r32) && (g_vm->_obj[oBRUCIATURA33]._mode & OBJMODE_OBJSTATUS))
		StartCharacterAction(a3311SALESCALE, 0, 0, 0);
	else if ((g_vm->_curRoom == r33) && (g_vm->_oldRoom == r32) && !(g_vm->_obj[oBRUCIATURA33]._mode & OBJMODE_OBJSTATUS))
		StartCharacterAction(a3313CHIUDEBOTOLA, 0, 0, 0);
	else if ((g_vm->_curRoom == r32) && (g_vm->_oldRoom == r33))
		StartCharacterAction(a325SCENDESCALE, 0, 0, 0);
	else if ((g_vm->_curRoom == r36) && (g_vm->_oldRoom == r35))
		StartCharacterAction(a361ESCEASCENSORE, 0, 0, 0);
	else if ((g_vm->_curRoom == r35) && (g_vm->_oldRoom == r36))
		StartCharacterAction(a3515ESCEASCENSORE, 0, 0, 0);
	else if ((g_vm->_curRoom == r44) && (g_vm->_oldRoom == r45 || g_vm->_oldRoom == r45S) && !(g_vm->_inventoryObj[iDISLOCATORE]._flag & OBJFLAG_EXTRA))
		StartCharacterAction(a445, 0, 0, 0);
	else if ((g_vm->_curRoom == r45 || g_vm->_curRoom == r45S) && (g_vm->_oldRoom == r44) && !(g_vm->_inventoryObj[iDISLOCATORE]._flag & OBJFLAG_EXTRA))
		StartCharacterAction(a457, 0, 0, 0);
	else if ((g_vm->_curRoom == r46) && (g_vm->_oldRoom == r47 || g_vm->_oldRoom == r48 || g_vm->_oldRoom == r49) && !(g_vm->_inventoryObj[iDISLOCATORE]._flag & OBJFLAG_EXTRA))
		StartCharacterAction(aWALKIN, 0, 0, 0);
	else if ((g_vm->_curRoom == r47) && (g_vm->_oldRoom == r46) && !(g_vm->_inventoryObj[iDISLOCATORE]._flag & OBJFLAG_EXTRA))
		StartCharacterAction(a476, 0, 0, 0);
	else if ((g_vm->_curRoom == r48) && (g_vm->_oldRoom == r46) && !(g_vm->_inventoryObj[iDISLOCATORE]._flag & OBJFLAG_EXTRA))
		StartCharacterAction(a485, 0, 0, 0);
	else if ((g_vm->_curRoom == r49) && (g_vm->_oldRoom == r46) && !(g_vm->_inventoryObj[iDISLOCATORE]._flag & OBJFLAG_EXTRA))
		StartCharacterAction(a494, 0, 0, 0);
	else if ((g_vm->_curRoom == r54) && (g_vm->_oldRoom == r53)) {
		StartCharacterAction(a5411, 0, 11, 0);
		g_vm->_inventoryObj[iLASER35]._examine = 1599;
	} else if ((g_vm->_oldRoom == r41D) && (g_vm->_inventoryObj[iDISLOCATORE]._flag & OBJFLAG_EXTRA)) {
		setPosition(30);
		drawCharacter(CALCPOINTS);
	}

	// FullMotion
	if ((g_vm->_curRoom == r17) && (g_vm->_oldRoom == r16)) {
		PlayDialog(dF171);
		WaitSoundFadEnd();
	} else if ((g_vm->_curRoom == r16) && (g_vm->_oldRoom == r17)) {
		PlayDialog(dF161);
		WaitSoundFadEnd();
		KillIcon(iFOGLIO14);
	} else if ((g_vm->_curRoom == r21) && (g_vm->_oldRoom == r1C)) {
		PlayDialog(dF1C1);
		WaitSoundFadEnd();
	} else if ((g_vm->_curRoom == r23B) && (g_vm->_oldRoom == r23A)) {
		PlayDialog(dF231);
		WaitSoundFadEnd();
		KillIcon(iMAPPA17);
	} else if ((g_vm->_curRoom == r29L) && (g_vm->_oldRoom == r29)) {
		PlayDialog(dF291);
		WaitSoundFadEnd();
	} else if ((g_vm->_curRoom == r2GV) && (g_vm->_oldRoom == r2G)) {
		PlayDialog(dF2G2);
		WaitSoundFadEnd();
	} else if ((g_vm->_curRoom == r31) && (g_vm->_oldRoom == r32) && (g_vm->_room[r32]._flag & OBJFLAG_EXTRA)) {
		PlayDialog(dF321);
		FlagShowCharacter = false;
		WaitSoundFadEnd();
		g_vm->_room[r32]._flag &= ~OBJFLAG_EXTRA;
	} else if ((g_vm->_curRoom == r19) && !(g_vm->_room[r19]._flag & OBJFLAG_DONE)) {
		PlayScript(s19EVA);
		FlagNoPaintScreen = false;
		ClearText();
		RepaintString();
		WaitSoundFadEnd();
	}
	// CHECKME: This check is identical to the 3rd one, thus it's always false
	else if ((g_vm->_curRoom == r21) && (/*!( _room[g_vm->_curRoom]._flag & OBJFLAG_DONE ) || */ (g_vm->_oldRoom == r1C))) {
		setPosition(10);
		TendIn();
		FlagNoPaintScreen = false;
		ClearText();
		RepaintString();
	} else if ((g_vm->_curRoom == r46) && (g_vm->_oldRoom == r43) && !(g_vm->_inventoryObj[iDISLOCATORE]._flag & OBJFLAG_EXTRA)) {
		PlayDialog(dF431);
		WaitSoundFadEnd();
	} else if ((g_vm->_curRoom == r45S) && (g_vm->_oldRoom == r45) && !(g_vm->_inventoryObj[iDISLOCATORE]._flag & OBJFLAG_EXTRA)) {
		PlayDialog(dF451);
		WaitSoundFadEnd();
	} else if ((g_vm->_curRoom == r4A) && (g_vm->_oldRoom == r49) && !(g_vm->_inventoryObj[iDISLOCATORE]._flag & OBJFLAG_EXTRA)) {
		PlayDialog(dF491);
		WaitSoundFadEnd();
	} else if ((g_vm->_curRoom == r4A) && (g_vm->_oldRoom == r41D) && (g_vm->_inventoryObj[iDISLOCATORE]._flag & OBJFLAG_EXTRA) && (g_vm->_obj[ocHELLEN4A]._mode & OBJMODE_OBJSTATUS)) {
		WaitSoundFadEnd();
		PlayDialog(dC4A1);
	} else if ((g_vm->_curRoom == r4P) && (g_vm->_oldRoom == r4O) && !(g_vm->_room[r4P]._flag & OBJFLAG_DONE)) {
		PlayDialog(dF4PI);
		FlagShowCharacter = false;
		WaitSoundFadEnd();
	} else if ((g_vm->_curRoom == r51) && (g_vm->_oldRoom == r4CT)) {
		PlayDialog(dF4C1);
		FlagShowCharacter = false;
		WaitSoundFadEnd();
	} else if ((g_vm->_curRoom == r1A) && (g_vm->_oldRoom == r18)) {
		if (!(g_vm->_room[r1A]._flag & OBJFLAG_DONE)) {
			PlayDialog(dF1A1);
			WaitSoundFadEnd();
			g_vm->_obj[oTOPO1C]._flag |= OBJFLAG_TAKE;
			g_vm->_obj[oTOPO1C]._anim = a1C3RACCOGLIETOPO;
		} else {
			TendIn();
			FlagNoPaintScreen = false;
			RepaintString();
		}
	} else {
		TendIn();
		FlagNoPaintScreen = false;
		ClearText();
		RepaintString();
	}

//	Sentence
	if ((g_vm->_curRoom == r17) && (g_vm->_oldRoom == r18) && !(g_vm->_room[r17]._flag & OBJFLAG_DONE) && (g_vm->_obj[oRETE17]._mode & OBJMODE_OBJSTATUS))
		CharacterSay(189);
	if (((g_vm->_curRoom == r12CU) || (g_vm->_curRoom == r13CU)) && (CloseUpObj) && (g_vm->_obj[CloseUpObj]._examine))
		CharacterSay(g_vm->_obj[CloseUpObj]._examine);
	else if ((g_vm->_curRoom == r23A) && (g_vm->_oldRoom == r21) && (!(g_vm->_room[r23A]._flag & OBJFLAG_DONE))) {
		FlagShowCharacter = true;
		StartCharacterAction(aWALKIN, 0, 0, 361);
	} else if ((g_vm->_curRoom == r24) && !(g_vm->_room[r24]._flag & OBJFLAG_DONE))
		CharacterSay(381);
	else if ((g_vm->_curRoom == r2G) && !(g_vm->_room[r2G]._flag & OBJFLAG_DONE))
		CharacterSay(688);
	else if ((g_vm->_curRoom == r4C) && (g_vm->_oldRoom == r4CT))
		CharacterSay(1163);
	else if ((g_vm->_curRoom == r41) && (g_vm->_oldRoom == r36) && !(g_vm->_room[r41]._flag & OBJFLAG_DONE))
		CharacterSay(900);
	else if ((g_vm->_curRoom == r58) && (g_vm->_obj[oGUARDIA58]._mode & OBJMODE_OBJSTATUS) && (g_vm->_obj[oGUARDIA58]._anim)) {
		g_vm->_curObj = oGUARDIA58;
		doEvent(MC_MOUSE, ME_MRIGHT, MP_DEFAULT, 372, 335 + TOP, 0, oGUARDIA58);
	} else if ((g_vm->_curRoom == r59L))
		CharacterSay(1394);
	else if ((g_vm->_curRoom == r58) && (g_vm->_oldRoom == r58T))
		CharacterSay(1368);
	else if ((g_vm->_curRoom == r5A) && !(g_vm->_room[r5A]._flag & OBJFLAG_DONE))
		CharacterSay(1408);
	else if ((g_vm->_curRoom == rSYS) && (g_vm->_oldRoom == rSYS))
		DoSys(o00LOAD);
	g_vm->_inventoryObj[iDISLOCATORE]._flag &= ~OBJFLAG_EXTRA;
}

/* -----------------26/11/97 10.38-------------------
 * 					GestioneATFrame
 * --------------------------------------------------*/
#define ATF_WAITTEXT 1

static struct ATFHandle {
	int16 curframe, lastframe;
	uint16 object;
	uint16 status;
	SAnim *curanim;
}            // 0->character 1->background 2->icon
AnimType[3] = {	{true}, {true}, {true}	};

/* -----------------11/07/97 11.43-------------------
					ExecuteATFDO
 --------------------------------------------------*/
void ExecuteATFDO(ATFHandle *h, int doit, int obj) {
	switch (doit) {
	case fCLROBJSTATUS:
		g_vm->_obj[obj]._mode &= ~OBJMODE_OBJSTATUS;
		RegenRoom();
		break;
	case fSETOBJSTATUS:
		g_vm->_obj[obj]._mode |= OBJMODE_OBJSTATUS;
		RegenRoom();
		break;
	case fONETIME:
		g_vm->_obj[obj]._anim = 0;
		break;
	case fCREPACCIO:
		if (g_vm->_room[r2E]._flag & OBJFLAG_EXTRA)
			g_vm->_obj[oCREPACCIO2E]._position = 7;
		else
			g_vm->_obj[oCREPACCIO2E]._position = 6;
		break;
	case fSERPVIA:
		doEvent(g_vm->_snake52._class, g_vm->_snake52._event, g_vm->_snake52._priority, g_vm->_snake52._u16Param1, g_vm->_snake52._u16Param2, g_vm->_snake52._u8Param, g_vm->_snake52._u32Param);
		break;
	case fPIRANHA:
		g_vm->_obj[oLUCCHETTO53]._anim = 0;
		g_vm->_obj[oGRATAC53]._anim = 0;
		g_vm->_obj[oGRATAA53]._anim = 0;
		g_vm->_obj[oLUCCHETTO53]._action = 1240;
		g_vm->_obj[oGRATAC53]._action = 1243;
		g_vm->_obj[oGRATAA53]._action = 1246;
		g_vm->_obj[oLAGO53]._examine = 1237;
		break;
	case fMOREAU:
		g_vm->_obj[oFINESTRAB58]._anim = 0;
		g_vm->_obj[oFINESTRAB58]._action = 1358;
		break;
	case fPORTA58:
		doEvent(MC_MOUSE, ME_MLEFT, MP_DEFAULT, 468, 180 + TOP, true, oPORTA58C55);
		break;
	case fHELLEN:
		doEvent(MC_MOUSE, ME_MLEFT, MP_DEFAULT, 336, 263 + TOP, true, 0);
		break;
	case fVALVOLAON34:
		if (!(_choice[616]._flag & OBJFLAG_DONE) &&		// if the fmv is not done
		    (g_vm->_obj[oTUBOA34]._mode & OBJMODE_OBJSTATUS) && // if there's a cut pipe
		    !(g_vm->_obj[oTUBOFT34]._mode & OBJMODE_OBJSTATUS)) // if there's not tube outside
			CallSmackVolumePan(0, 2, 1);
		break;
	case fVALVOLAOFF34:
		CallSmackVolumePan(0, 2, 0);
		break;

	case fCHARACTEROFF:
		FlagCharacterExist = false;
		break;
	case fCHARACTERON:
		FlagCharacterExist = true;
		break;
	case fCHARACTERFOREGROUND:
		_forcedActorPos = FOREGROUND;
		break;
	case fCHARACTERBACKGROUND:
		_forcedActorPos = BACKGROUND;
		break;
	case fCHARACTERNORM:
		_forcedActorPos = 0;
		break;
	case fSETEXTRA:
		g_vm->_obj[obj]._flag |= OBJFLAG_EXTRA;
		break;
	case fCLREXTRA:
		g_vm->_obj[obj]._flag &= ~OBJFLAG_EXTRA;
		break;

	case fANIMOFF1:
		AnimTab[g_vm->_room[g_vm->_curRoom]._bkgAnim]._flag |= SMKANIM_OFF1;
		if ((g_vm->_curRoom == r11) || (g_vm->_curRoom == r1D) || (g_vm->_curRoom == r14) || (g_vm->_curRoom == r22) || (g_vm->_curRoom == r48) || (g_vm->_curRoom == r4P))
			CallSmackVolumePan(0, 1, 0);
		break;
	case fANIMOFF2:
		AnimTab[g_vm->_room[g_vm->_curRoom]._bkgAnim]._flag |= SMKANIM_OFF2;
		if ((g_vm->_curRoom == r2E))
			CallSmackVolumePan(0, 2, 0);
		break;
	case fANIMOFF3:
		AnimTab[g_vm->_room[g_vm->_curRoom]._bkgAnim]._flag |= SMKANIM_OFF3;
		break;
	case fANIMOFF4:
		AnimTab[g_vm->_room[g_vm->_curRoom]._bkgAnim]._flag |= SMKANIM_OFF4;
		if (g_vm->_curRoom == r28)
			CallSmackVolumePan(0, 1, 0);
		break;

	case fANIMON1:
		AnimTab[g_vm->_room[g_vm->_curRoom]._bkgAnim]._flag &= ~SMKANIM_OFF1;
		if ((g_vm->_curRoom == r14) || (g_vm->_curRoom == r1D) || (g_vm->_curRoom == r22) || (g_vm->_curRoom == r48) || (g_vm->_curRoom == r4P)) {
			CallSmackVolumePan(0, 1, 1);
		}
		break;
	case fANIMON2:
		AnimTab[g_vm->_room[g_vm->_curRoom]._bkgAnim]._flag &= ~SMKANIM_OFF2;
		if ((g_vm->_curRoom == r2E)) {
			CallSmackVolumePan(0, 2, 1);
		}
		break;
	case fANIMON3:
		AnimTab[g_vm->_room[g_vm->_curRoom]._bkgAnim]._flag &= ~SMKANIM_OFF3;
		break;
	case fANIMON4:
		AnimTab[g_vm->_room[g_vm->_curRoom]._bkgAnim]._flag &= ~SMKANIM_OFF4;
		break;
	case fENDDEMO:
		DemoOver();
		doEvent(MC_SYSTEM, ME_QUIT, MP_SYSTEM, 0, 0, 0, 0);
		break;
	case fSTOP2TXT:
		h->status |= ATF_WAITTEXT;
		// Sets a flag that is always cleared when you finish speaking
		// if the flag is cleared the anim no longer plays
		// (to be done in the smacker player)
		// also the counters in AtFrameNext stops
		break;
	}
}

/* -----------------11/07/97 11.42-------------------
					ProcessATF
 --------------------------------------------------*/
void ProcessATF(ATFHandle *h, int type, int atf) {
	static int dc = 0;
	extern char UStr[];

	switch (type) {
	case ATFTEXT:
		CharacterTalkInAction(h->curanim->_atFrame[atf]._index);
		break;
	case ATFTEXTACT:
		CharacterTalkInAction(g_vm->_obj[h->object]._action);
		break;
	case ATFTEXTEX:
		CharacterTalkInAction(g_vm->_obj[h->object]._examine);
		break;
	case ATFCLR:
		g_vm->_obj[h->curanim->_atFrame[atf]._index]._mode &= ~OBJMODE_OBJSTATUS;
		RegenRoom();
		break;
	case ATFCLRI:
		KillIcon(h->curanim->_atFrame[atf]._index);
		break;
	case ATFCEX:
		g_vm->_obj[h->object]._examine = h->curanim->_atFrame[atf]._index;
		break;
	case ATFCACT:
		g_vm->_obj[h->object]._action = h->curanim->_atFrame[atf]._index;
		break;
	case ATFSET:
		g_vm->_obj[h->curanim->_atFrame[atf]._index]._mode |= OBJMODE_OBJSTATUS;
		RegenRoom();
		break;
	case ATFSETI:
		AddIcon(h->curanim->_atFrame[atf]._index);
		break;
	case ATFDO:
		ExecuteATFDO(h, h->curanim->_atFrame[atf]._index, h->object);
		break;
	case ATFROOM:
		doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, h->curanim->_atFrame[atf]._index, 0, 0, h->object);
		break;
	case ATFSETPOS:
		setPosition(h->curanim->_atFrame[atf]._index);
		break;
	case ATFDIALOG:
		PlayDialog(h->curanim->_atFrame[atf]._index);
		break;
	case ATFCOBJANIM:
		g_vm->_obj[h->object]._anim = h->curanim->_atFrame[atf]._index;
		RegenRoom();
		break;
	case ATFCOBJBOX:
		g_vm->_obj[h->object]._nbox = h->curanim->_atFrame[atf]._index;
		RegenRoom();
		break;
	case ATFCOBJPOS:
		g_vm->_obj[h->object]._position = h->curanim->_atFrame[atf]._index;
		RegenRoom();
		break;
	case ATFSETFORE:
		g_vm->_obj[h->curanim->_atFrame[atf]._index]._nbox = FOREGROUND;
		RegenRoom();
		break;
	case ATFSETBACK:
		g_vm->_obj[h->curanim->_atFrame[atf]._index]._nbox = BACKGROUND;
		RegenRoom();
		break;
	case ATFSWITCH:
		g_vm->_obj[h->curanim->_atFrame[atf]._index]._mode ^= OBJMODE_OBJSTATUS;
		RegenRoom();
		break;
	case ATFSETROOMT:
		SetRoom(h->curanim->_atFrame[atf]._index, true);
		break;
	case ATFSETROOMF:
		SetRoom(h->curanim->_atFrame[atf]._index, false);
		break;
	case ATFREADBOX:
		if (h->curanim->_atFrame[atf]._index == 1) {
			sprintf(UStr, "%s.3d", g_vm->_room[g_vm->_curRoom]._baseName);
			read3D(UStr);
			g_vm->_room[g_vm->_curRoom]._flag &= ~OBJFLAG_EXTRA;
		} else if (h->curanim->_atFrame[atf]._index == 2) {
			sprintf(UStr, "%s2.3d", g_vm->_room[g_vm->_curRoom]._baseName);
			read3D(UStr);
			g_vm->_room[g_vm->_curRoom]._flag |= OBJFLAG_EXTRA;
			if ((g_vm->_curRoom == r37)) {
				CallSmackVolumePan(0, 1, 1);
			}
		}
		break;
	case ATFONESPEAK:
		switch (h->curanim->_atFrame[atf]._index) {
		case 1:
			if (g_vm->_room[r1D]._flag & OBJFLAG_EXTRA)
				break;

			SomeOneTalk(307 + dc, oDONNA1D, 0, 0);
			if (dc < 6)
				dc ++;
			break;

		case 2:
			SomeOneTalk(1788, ocNEGOZIANTE1A, 0, 0);
			break;
		}
		break;
	case ATFEND:
		DemoOver();
		doEvent(MC_SYSTEM, ME_QUIT, MP_SYSTEM, 0, 0, 0, 0);
		break;

	}

}

/* -----------------11/07/97 11.42-------------------
					InitAtFrameHandler
 --------------------------------------------------*/
void InitAtFrameHandler(uint16 an, uint16 obj) {
	SAnim *anim = &AnimTab[an];

	ATFHandle *handle = &AnimType[0];
	if (anim->_flag & SMKANIM_BKG)
		handle = &AnimType[1];
	if (anim->_flag & SMKANIM_ICON)
		handle = &AnimType[2];

	handle->curanim = anim;
	if (obj)
		handle->object = obj;
	else
		handle->object = g_vm->_curObj;
	handle->curframe = 0;
	handle->lastframe = -1;
	handle->status = 0;
}

/* -----------------11/07/97 11.41-------------------
					AtFrameNext
 --------------------------------------------------*/
void AtFrameNext() {
	if (!((AnimType[0].status & ATF_WAITTEXT) && FlagCharacterSpeak))
		AnimType[0].curframe++;
	if (!((AnimType[1].status & ATF_WAITTEXT) && FlagCharacterSpeak))
		AnimType[1].curframe++;
	if (!((AnimType[2].status & ATF_WAITTEXT) && FlagCharacterSpeak))
		AnimType[2].curframe++;
}

/* -----------------11/07/97 11.42-------------------
					AtFrameEnd
 --------------------------------------------------*/
void AtFrameEnd(int type) {
	ATFHandle *h = &AnimType[type];
	SAnim *anim = h->curanim;
	h->curframe = 0;

	// if this ATFrame has already been handled
	if (h->curframe == h->lastframe)
		return ;

	h->lastframe = h->curframe;

	for (int32 a = 0; a < MAXATFRAME; a++) {
		// if it's time to run this AtFrame
		if ((anim->_atFrame[a]._numFrame == 0) && (anim->_atFrame[a]._type)) {
			if ((anim->_atFrame[a]._child == 0) ||
			    ((anim->_atFrame[a]._child == 1) && !(AnimTab[g_vm->_room[g_vm->_curRoom]._bkgAnim]._flag & SMKANIM_OFF1)) ||
			    ((anim->_atFrame[a]._child == 2) && !(AnimTab[g_vm->_room[g_vm->_curRoom]._bkgAnim]._flag & SMKANIM_OFF2)) ||
			    ((anim->_atFrame[a]._child == 3) && !(AnimTab[g_vm->_room[g_vm->_curRoom]._bkgAnim]._flag & SMKANIM_OFF3)) ||
			    ((anim->_atFrame[a]._child == 4) && !(AnimTab[g_vm->_room[g_vm->_curRoom]._bkgAnim]._flag & SMKANIM_OFF4)))
				ProcessATF(h, anim->_atFrame[a]._type, a);
		}
	}

	h->curanim = nullptr;
}

/* -----------------11/07/97 11.44-------------------
					AtFrameHandler
 --------------------------------------------------*/
void AtFrameHandler(int type) {
	ATFHandle *h = &AnimType[type];
	SAnim *anim = h->curanim;
	if (anim == nullptr)
		return ;

	if (h->curframe == 0)
		h->curframe++;
	// if this ATFrame has already been applied
	if (h->curframe <= h->lastframe)
		return ;

	for (int32 a = 0; a < MAXATFRAME; a++) {
		// if it's time to run this AtFrame
		if ((anim->_atFrame[a]._numFrame > h->lastframe) && (anim->_atFrame[a]._numFrame <= h->curframe) && (anim->_atFrame[a]._numFrame != 0)) {
			if ((anim->_atFrame[a]._child == 0) ||
			    ((anim->_atFrame[a]._child == 1) && !(AnimTab[g_vm->_room[g_vm->_curRoom]._bkgAnim]._flag & SMKANIM_OFF1)) ||
			    ((anim->_atFrame[a]._child == 2) && !(AnimTab[g_vm->_room[g_vm->_curRoom]._bkgAnim]._flag & SMKANIM_OFF2)) ||
			    ((anim->_atFrame[a]._child == 3) && !(AnimTab[g_vm->_room[g_vm->_curRoom]._bkgAnim]._flag & SMKANIM_OFF3)) ||
			    ((anim->_atFrame[a]._child == 4) && !(AnimTab[g_vm->_room[g_vm->_curRoom]._bkgAnim]._flag & SMKANIM_OFF4)))
				ProcessATF(h, anim->_atFrame[a]._type, a);
		}
	}

	// set lastframe
	h->lastframe = h->curframe;
}

} // End of namespace Trecision
