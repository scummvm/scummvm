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
#include "trecision/nl/lib/addtype.h"
#include "trecision/nl/3d/3dinc.h"
#include "trecision/nl/sysdef.h"
#include "trecision/nl/struct.h"
#include "trecision/nl/ll/llinc.h"
#include "trecision/nl/message.h"
#include "trecision/nl/extern.h"
#include "trecision/nl/define.h"

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
void doRoomIn(uint16 TheObj) {
	uint16 TheAction, ThePos;

	SemMouseEnabled = false;

	switch (TheObj) {
	/*		case oCANCELLATA1B:
				if( (_obj[oBOTTIGLIA1D]._mode&OBJMODE_OBJSTATUS) && (_obj[oRETE17]._mode&OBJMODE_OBJSTATUS) )
				{
					TheAction = _obj[TheObj]._anim;
					ThePos = _obj[TheObj]._ninv;
				}
				else
					TheAction = 0;
				break;
	*/
	default:
		TheAction = _obj[TheObj]._anim;
		ThePos = _obj[TheObj]._ninv;
		break;
	}
	doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, _obj[TheObj]._goRoom, TheAction, ThePos, TheObj);

	_obj[TheObj]._flag |= OBJFLAG_DONE;
}

/*-------------------------------------------------------------------------*/
/*                                 ROOMOUT         						   */
/*-------------------------------------------------------------------------*/
void doRoomOut(uint16 TheObj) {
	uint16 TheAction, ThePos;

	SemMouseEnabled = false;

	switch (TheObj) {
	case oSCALA32:
		if (_obj[oBOTOLAC32]._mode & OBJMODE_OBJSTATUS) {
			CharacterSay(_obj[TheObj]._action);
			SemMouseEnabled = true;
			TheAction = 0;
			ThePos = 0;
		} else {
			TheAction = _obj[TheObj]._anim;
			ThePos = _obj[TheObj]._ninv;
		}
		break;

	default:
		TheAction = _obj[TheObj]._anim;
		ThePos = _obj[TheObj]._ninv;
		break;
	}

	if (TheAction)
		doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, TheAction, _obj[TheObj]._goRoom, ThePos, TheObj);

	_obj[TheObj]._flag |= OBJFLAG_DONE;
}

/*-------------------------------------------------------------------------*/
/*                                 EXAMINE           					   */
/*-------------------------------------------------------------------------*/
void doMouseExamine(uint16 TheObj) {
	LLBOOL printsent = false;
	int a;

	if (!TheObj)
		warning("doMouseExamine");

	switch (TheObj) {
	case oMAPPA12:
		doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a1211OSSERVAMAPPAPALAZZO, 0, 0, TheObj);
		_choice[4]._flag &= ~DLGCHOICE_HIDE;
		_choice[18]._flag &= ~DLGCHOICE_HIDE;
		_choice[33]._flag &= ~DLGCHOICE_HIDE;
		printsent = false;
		break;

	case oPORTAA13:
		_obj[oCASSETTA12]._mode |= OBJMODE_OBJSTATUS;
		printsent = true;
		break;

	case oPANNELLOA12:
	case oFUSIBILE12:
		if (_obj[oFUSIBILE12]._mode & OBJMODE_OBJSTATUS)
			_obj[oFUSE12CU]._mode |= OBJMODE_OBJSTATUS;
		else
			_obj[oFUSE12CU]._mode &= ~OBJMODE_OBJSTATUS;
		doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, r12CU, 0, 0, TheObj);
		CloseUpObj = TheObj;
		break;

	case oLETTERA13:
	case oPENPADA13:
		if (_obj[oLETTERA13]._mode & OBJMODE_OBJSTATUS)
			_obj[oLETTER13CU]._mode |= OBJMODE_OBJSTATUS;
		else
			_obj[oLETTER13CU]._mode &= ~OBJMODE_OBJSTATUS;
		doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, r13CU, 0, 0, TheObj);
		CloseUpObj = TheObj;
		break;

	case oCUCININO14:
		doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a1413OSSERVAPIANOCOTTURA, 0, 0, TheObj);
		printsent = false;
		break;

	case oSCAFFALE14:
		if (!(_obj[oPORTAR14]._mode & OBJMODE_OBJSTATUS))
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a145ESAMINASCAFFALE, 0, 0, TheObj);
		else
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a145CESAMINASCAFFALE, 0, 0, TheObj);
		printsent = false;
		break;

	case oTAVOLINOPP14:
		doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a1412SPOSTASEDIA, 0, 0, TheObj);
		printsent = false;
		break;

	case oBOCCETTE15:
		doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a1510TOCCABOCCETTE, 0, 0, TheObj);
		printsent = false;
		break;

	case oSPECCHIO15:
		doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a1511SISPECCHIA, 0, 0, TheObj);
		printsent = false;
		break;

	case oMONITORSA16:
		doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a1614GUARDAMONITORS14, 0, 0, TheObj);
		printsent = false;
		break;

	case oFINESTRAA15:
		if (_obj[oTAPPARELLAA15]._mode & OBJMODE_OBJSTATUS)
			CharacterSay(1999);
		else
			printsent = true;
		break;

	case oMONITORSP16:
		doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a1615GUARDAMONITORS15, 0, 0, TheObj);
		printsent = false;
		break;

	case oCARTACCE16:
		doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a1613GUARDACARTACCE, 0, 0, TheObj);
		printsent = false;
		break;

	case oMAPPA16:
		if (IconPos(iFOGLIO14) != MAXICON)
			_obj[oMAPPA16]._flag |= OBJFLAG_EXTRA;
		printsent = true;
		break;

	case oSCATOLONE17:
		doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a177SICHINA, 0, 0, TheObj);
		printsent = false;
		break;

	case oMURALES17:
		doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a179MUOVETESTA, 0, 0, TheObj);
		printsent = false;
		break;

	case oSCHERMO18:
		doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a181ESAMINACARTELLONE, 0, 0, TheObj);
		printsent = false;
		break;

	case oVETRINA1A:
		doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a1A1ESAMINAVETRINA, 0, 0, TheObj);
		printsent = false;
		break;

	case oTESSERA1A:
		if (((_choice[151]._flag & OBJFLAG_DONE) || (_choice[152]._flag & OBJFLAG_DONE)) && !(_choice[183]._flag & OBJFLAG_DONE))
			_choice[183]._flag &= ~DLGCHOICE_HIDE;
		_obj[oTESSERA1A]._flag |= OBJFLAG_EXTRA;
		printsent = true;
		break;

	case oCARTACCE1B:
		doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a1B8FRUGACARTACCIE, 0, 0, TheObj);
		printsent = false;
		break;

	case oBIDONE1B:
		doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a1B10GUARDABIDONE, 0, 0, TheObj);
		printsent = false;
		break;

	case oGRATA1C:
		doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a1C2GUARDAGRATA, 0, 0, TheObj);
		printsent = false;
		break;

	case oSCAFFALE1D:
		doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a1D8SALESGABELLO, 0, 0, TheObj);
		printsent = false;
		break;

	case oBARILOTTO1D:
		doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a1D7SPINGEBARILOTTO, 0, 0, TheObj);
		printsent = false;
		break;

	case oCASSA1D:
		doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a1D10ESAMINABOTTIGLIE, 0, 0, TheObj);
		printsent = false;
		break;

	case oSCATOLETTA23:
		doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a2310, 0, 0, TheObj);
		printsent = false;
		break;

	case oPALMA26:
		doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a262, 0, 0, TheObj);
		printsent = false;
		break;

	case oINSEGNA26:
		doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a263, 0, 0, TheObj);
		printsent = false;
		break;

	case oTEMPIO28:
		doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a2810, 0, 0, TheObj);
		printsent = false;
		break;

	case oSERPENTET28:
	case oSERPENTEA28:
		doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a2811, 0, 0, TheObj);
		printsent = false;
		break;

	case oSERPENTE2B:
		doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a2B11ESAMINASERPENTE, 0, 0, TheObj);
		printsent = false;
		break;

	case oLEOPARDO2B:
		doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a2B9ESAMINALEOPARDO, 0, 0, TheObj);
		printsent = false;
		break;

	case oPELLICANO2B:
		doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a2B10ESAMINAPELLICANO, 0, 0, TheObj);
		printsent = false;
		break;

	case oBACHECA2B:
		doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a2B13OSSERVAREFARFALLE, 0, 0, TheObj);
		printsent = false;
		break;

	case oROBOT2F:
		doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a2F6TOCCADINOSAURO, 0, 0, TheObj);
		printsent = false;
		break;

	case oCREPACCIO2E:
		if (Room[r2E]._flag & OBJFLAG_EXTRA)
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a2E7GUARDACREPACCIODILA, 0, 0, TheObj);
		else
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a2E6GUARDACREPACCIODIQUA, 0, 0, TheObj);
		printsent = false;
		break;

	/*		case oFOGLI34:
				doEvent(MC_HOMO,ME_HOMOACTION, MP_DEFAULT, a348ESAMINAFOGLI, 0,0,TheObj);
				printsent = false;
				break;
	*/
	case oGENERATORE34:
		doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a347ESAMINAGENERATORE, 0, 0, TheObj);
		printsent = false;
		break;

	case oTUBOF33:
		if ((_obj[oVALVOLA34]._mode & OBJMODE_OBJSTATUS) && (_obj[oVALVOLA34]._anim))
			CharacterSay(2000);
		else
			CharacterSay(_obj[TheObj]._examine);
		printsent = false;
		break;

	case oTUBOT33:
		if ((_obj[oVALVOLA34]._mode & OBJMODE_OBJSTATUS) && (_obj[oVALVOLA34]._anim))
			CharacterSay(2001);
		else
			CharacterSay(_obj[TheObj]._examine);
		printsent = false;
		break;

	case oTUBOA34:
		if ((_obj[oVALVOLA34]._mode & OBJMODE_OBJSTATUS) && (_obj[oVALVOLA34]._anim))
			CharacterSay(2002);
		else
			CharacterSay(_obj[TheObj]._examine);
		printsent = false;
		break;

	case oTUBOF34:
		if ((_obj[oVALVOLA34]._mode & OBJMODE_OBJSTATUS) && (_obj[oVALVOLA34]._anim))
			CharacterSay(2000);
		else
			CharacterSay(_obj[TheObj]._examine);
		printsent = false;
		break;

	case oTUBOFT34:
		if ((_obj[oVALVOLA34]._mode & OBJMODE_OBJSTATUS) && (_obj[oVALVOLA34]._anim))
			CharacterSay(2001);
		else
			CharacterSay(_obj[TheObj]._examine);
		printsent = false;
		break;

	case oCASSE35:
		doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a3522ESAMINACASSE, 0, 0, TheObj);
		printsent = false;
		break;

	case oSCAFFALE35:
		if (Room[r35]._flag & OBJFLAG_EXTRA)
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a3517ESAMINACIANFRUSAGLIE, 0, 0, TheObj);
		else
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a3517AESAMINACIANFRUSAGLIE, 0, 0, TheObj);
		printsent = false;
		break;

	case oGIORNALE35:
		if (Room[r35]._flag & OBJFLAG_EXTRA) {
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a3521LEGGEGIORNALE, 0, 0, TheObj);
			printsent = false;
		}
		break;

	case oSCAFFALE36:
		doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, _obj[oSCAFFALE36]._anim, 0, 0, TheObj);
		printsent = false;
		break;

	case oFESSURA41:
		doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a411, 0, 0, TheObj);
		printsent = false;
		break;

	case oCARTELLOV42:
		doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a424, 0, 0, TheObj);
		printsent = false;
		break;

	case oCARTELLOF42:
		doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a426, 0, 0, TheObj);
		printsent = false;
		break;

	case oCAMPANAT43:
		if (_obj[oMARTELLOR43]._mode & OBJMODE_OBJSTATUS)
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a431R, 0, 0, TheObj);
		else
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a431, 0, 0, TheObj);
		printsent = false;
		break;

	case oTAMBURO43:
		if (_obj[oMARTELLOR43]._mode & OBJMODE_OBJSTATUS)
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a432R, 0, 0, TheObj);
		else
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a432, 0, 0, TheObj);
		printsent = false;
		break;

	case oRAGNATELA45:
		doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a451, 0, 0, TheObj);
		printsent = false;
		break;

	case oQUADROS4A:
		doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a4A5, 0, 0, TheObj);
		printsent = false;
		break;

	case oCARTELLO55:
		doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a5511, 0, 0, TheObj);
		printsent = false;
		break;

	case oEXIT12CU:
		doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, _obj[oEXIT12CU]._goRoom, 0, 0, TheObj);
		break;

	case oEXIT13CU:
		doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, _obj[oEXIT13CU]._goRoom, 0, 0, TheObj);
		break;

	case oEXIT2BL:
		doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, _obj[oEXIT2BL]._goRoom, 0, 0, TheObj);
		break;

	case oEXIT36F:
		doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, _obj[oEXIT36F]._goRoom, 0, 0, TheObj);
		break;

	case oEXIT41D:
		doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, _obj[oEXIT41D]._goRoom, 0, 0, TheObj);
		break;

	case oEXIT4CT:
		doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, _obj[oEXIT4CT]._goRoom, 0, 0, TheObj);
		break;

	case oEXIT58T:
		Count58 = 0;
		for (a = 0; a < 6; a++)
			_obj[oLED158 + a]._mode &= ~OBJMODE_OBJSTATUS;
		doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, _obj[oEXIT58T]._goRoom, 0, 0, TheObj);
		break;

	case oEXIT58M:
		doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, _obj[oEXIT58M]._goRoom, 0, 0, TheObj);
		break;

	case oEXIT59L:
		doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, _obj[oEXIT59L]._goRoom, 0, 0, TheObj);
		break;

	default:
		printsent = true;
		break;
	}

	if ((printsent) && (_obj[TheObj]._examine)) CharacterSay(_obj[TheObj]._examine);
}

/*-------------------------------------------------------------------------*/
/*                                 OPERATE           					   */
/*-------------------------------------------------------------------------*/
void doMouseOperate(uint16 TheObj) {
	LLBOOL printsent = false;
	int a;

	if (!TheObj)
		warning("doMouseOperate");

	switch (TheObj) {
	case oFAX17:
		if (_obj[oSCALA16]._anim) {
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, _obj[TheObj]._anim, 0, 0, TheObj);
			InvObj[iSAM]._action = 1416;
			printsent = false;
		}
		break;

	case ocPOLIZIOTTO16:
	case oSCALA16:
		if (!(_obj[oSCALA16]._flag & OBJFLAG_EXTRA)) {
			_obj[oSCALA16]._flag |= OBJFLAG_EXTRA;
			_choice[61]._flag &= ~DLGCHOICE_HIDE;
			PlayDialog(dPOLIZIOTTO16);
			_obj[oSCALA16]._action = 166;
			_obj[ocPOLIZIOTTO16]._action = 166;
		} else
			printsent = true;
		break;

	case oPANNELLOC12:
		if (_obj[oPANNELLOC12]._flag & OBJFLAG_EXTRA) {
			if (_obj[oASCENSOREC12]._mode & OBJMODE_OBJSTATUS)
				doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a128RIUSABOTTONE, 0, 0, TheObj);
			else
				CharacterSay(24);
		} else {
			_obj[oPANNELLOC12]._flag |= OBJFLAG_EXTRA;
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, _obj[TheObj]._anim, 0, 0, TheObj);
		}
		break;
	case oPANNELLO13:
		if (_obj[oASCENSOREA13]._mode & OBJMODE_OBJSTATUS) {
			CharacterSay(48);
			printsent = false;
		} else
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, _obj[TheObj]._anim, 0, 0, TheObj);

		break;

	case oPANNELLO16:
		if (_obj[oASCENSOREA16]._mode & OBJMODE_OBJSTATUS) {
			CharacterSay(48);
			printsent = false;
		} else
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, _obj[TheObj]._anim, 0, 0, TheObj);
		break;

	case oLATTINA13:
		doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a132PRENDELATTINA, 0, 0, TheObj);
		AddIcon(iLATTINA13);
		break;

	case oPORTAA13:
		if (Room[r14]._flag & OBJFLAG_DONE) {
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a1312METTELETTERARICALCA, r14, 14, UseWith[WITH]);
			printsent = false;
		} else
			printsent = true;
		break;

	case oCESTINO14:
		if (_obj[TheObj]._anim) {
			if (!(_obj[oPORTAR14]._mode & OBJMODE_OBJSTATUS))
				doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, _obj[TheObj]._anim, 0, 0, TheObj);
			else
				doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a143CPRENDECREDITCARD, 0, 0, TheObj);
			_obj[TheObj]._anim = NULL;
		} else
			printsent = true;
		break;
	case oTASTOB15:
		if (_obj[oTAPPARELLAA15]._mode & OBJMODE_OBJSTATUS) {
			CharacterSay(_obj[oTASTOB15]._action);
			printsent = false;
		} else {
			if (!(_obj[oNASTRO15]._flag & OBJFLAG_EXTRA))
				_obj[oNASTRO15]._mode |= OBJMODE_OBJSTATUS;
			if (_obj[TheObj]._anim)
				doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, _obj[TheObj]._anim, 0, 0, TheObj);
			printsent = false;
		}
		break;

	case oTASTOA15:
		if (!(_obj[oTAPPARELLAA15]._mode & OBJMODE_OBJSTATUS)) {
			CharacterSay(_obj[oTASTOA15]._action);
			printsent = false;
		} else {
			_obj[oNASTRO15]._mode &= ~OBJMODE_OBJSTATUS;
			if (_obj[TheObj]._anim)
				doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, _obj[TheObj]._anim, 0, 0, TheObj);
			printsent = false;
		}
		break;

	case oFINGERPADP16:
		printsent = true;
		break;

	case oPORTAC18:
		if (_obj[ocGUARDIANO18]._flag & OBJFLAG_PERSON) {
			_choice[151]._flag |= DLGCHOICE_HIDE;
			_choice[152]._flag &= ~DLGCHOICE_HIDE;
			PlayDialog(dGUARDIANO18);
			_obj[ocGUARDIANO18]._flag &= ~OBJFLAG_PERSON;
			_obj[ocGUARDIANO18]._action = 227;
			_obj[oPORTAC18]._action = 220;
		} else
			printsent = true;
		break;

	case oGRATA1C:
		if (_obj[oFAX17]._flag & OBJFLAG_EXTRA)
			doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, r21, 0, 10, TheObj);
		else
			printsent = true;
		break;

	case oBOTOLAC1B:
		if ((_obj[oBOTOLAC1B]._anim == a1B3APREBOTOLA) && (_obj[oTOMBINOA1B]._mode & OBJMODE_OBJSTATUS))
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a1B3AAPREBOTOLA, 0, 0, TheObj);
		else if (_obj[oBOTOLAC1B]._anim == a1B3APREBOTOLA)
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a1B3APREBOTOLA, 0, 0, TheObj);
		else
			printsent = true;
		break;

	case oARMADIETTORC22:
		doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, _obj[TheObj]._anim, 0, 0, TheObj);
		_obj[oMANIGLIONE22]._anim = a227A;
		_obj[oMANIGLIONEO22]._anim = a229A;
		_obj[od22ALLA29]._anim = a2214A;
		_obj[od22ALLA29I]._anim = a2215A;
		break;

	case oARMADIETTORA22:
		doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, _obj[TheObj]._anim, 0, 0, TheObj);
		_obj[oMANIGLIONE22]._anim = a227;
		_obj[oMANIGLIONEO22]._anim = a229;
		_obj[od22ALLA29]._anim = a2214;
		_obj[od22ALLA29I]._anim = a2215;
		break;

	case oCATENAT21:
		if ((IconPos(iSBARRA21) != MAXICON) && ((_choice[436]._flag & OBJFLAG_DONE) || (_choice[466]._flag & OBJFLAG_DONE))) {
			if (Room[_curRoom]._flag & OBJFLAG_EXTRA)		// va a destra
				PlayDialog(dF212);				// 436
			else									// va a sinistra
				PlayDialog(dF213);				// 466
			printsent = false;
		} else if ((_choice[451]._flag & OBJFLAG_DONE) || (_choice[481]._flag & OBJFLAG_DONE)) {
			if (Room[_curRoom]._flag & OBJFLAG_EXTRA)		// va a destra
				PlayDialog(dF212B);				// 451
			else									// va a sinistra
				PlayDialog(dF213B);				// 481
			printsent = false;
		} else
			printsent = true;
		break;

	case oPULSANTEACS2D:
		doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, _obj[TheObj]._anim, 0, 0, TheObj);
		_obj[oPULSANTEBC2D]._anim = a2D7SCHIACCIATASTO6V;
		break;

	case oPULSANTEACA2D:
		doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, _obj[TheObj]._anim, 0, 0, TheObj);
		_obj[oPULSANTEBC2D]._anim = a2D7SCHIACCIATASTO6R;
		break;

	case oINTERRUTTORE29:
		if (_curRoom == r29L)
			printsent = true;
		else if ((_obj[oLAMPADINAS29]._mode & OBJMODE_OBJSTATUS)) {
			doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, r29L, 0, 0, TheObj);
			printsent = false;
		} else if (!(_obj[_curObj]._flag & OBJFLAG_EXTRA)) {
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a291USAINTERRUTTORELUCE, 0, 0, TheObj);
			_obj[_curObj]._flag |= OBJFLAG_EXTRA;
			printsent = false;
		} else
			printsent = true;
		break;

	case oLEVAS23:
		if (_obj[oCAVI23]._mode & OBJMODE_OBJSTATUS)
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a238, 0, 0, TheObj);
		else
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a234, 0, 0, TheObj);
		break;

	case oLEVAG23:
		if (_obj[oCAVIE23]._mode & OBJMODE_OBJSTATUS)
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a239, 0, 0, TheObj);
		else
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a235, 0, 0, TheObj);
		break;

	case oBOTOLAC25:
		doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, _obj[TheObj]._anim, 0, 0, TheObj);
		_obj[oTRONCHESE25]._anim = a254B;
		break;

	case oBOTOLAA25:
		doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, _obj[TheObj]._anim, 0, 0, TheObj);
		_obj[oTRONCHESE25]._anim = a254;
		break;

	case oPASSAGGIO24:
		if (Room[r24]._flag & OBJFLAG_EXTRA) {
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a244, 0, 14, TheObj);
			SetRoom(r24, false);
			printsent = false;
		} else {
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a243, 0, 13, TheObj);
			SetRoom(r24, true);
			printsent = false;
		}
		break;

	case oPORTA26:
		if (_obj[TheObj]._anim)
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, _obj[TheObj]._anim, 0, 4, TheObj);
		break;

	case oRUBINETTOC28:
		if (_obj[TheObj]._anim) {
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, _obj[TheObj]._anim, 0, 0, TheObj);
			_obj[oBRACIERES28]._examine = 455;
			_obj[oBRACIERES28]._flag |= OBJFLAG_EXTRA;
		} else
			printsent = true;
		break;

	case oEXIT12CU:
		doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, _obj[oEXIT12CU]._goRoom, 0, 0, TheObj);
		break;

	case oEXIT13CU:
		doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, _obj[oEXIT13CU]._goRoom, 0, 0, TheObj);
		break;

	case oEXIT2BL:
		doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, _obj[oEXIT2BL]._goRoom, 0, 0, TheObj);
		break;

	case oEXIT36F:
		doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, _obj[oEXIT36F]._goRoom, 0, 0, TheObj);
		break;

	case oEXIT41D:
		doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, _obj[oEXIT41D]._goRoom, 0, 0, TheObj);
		break;

	case oEXIT4CT:
		doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, _obj[oEXIT4CT]._goRoom, 0, 0, TheObj);
		break;

	case oEXIT58T:
		Count58 = 0;
		for (a = 0; a < 6; a++)
			_obj[oLED158 + a]._mode &= ~OBJMODE_OBJSTATUS;
		doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, _obj[oEXIT58T]._goRoom, 0, 0, TheObj);
		break;

	case oEXIT58M:
		doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, _obj[oEXIT58M]._goRoom, 0, 0, TheObj);
		break;

	case oEXIT59L:
		doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, _obj[oEXIT59L]._goRoom, 0, 0, TheObj);
		break;

	case oPANNELLOM2G:
		if (!(_obj[oPANNELLOM2G]._flag & OBJFLAG_EXTRA)) {
			PlayDialog(dF2G1);
			_obj[oCOPERCHIO2G]._mode &= ~OBJMODE_OBJSTATUS;
			//_obj[oPANNELLOM2G]._mode &= ~OBJMODE_OBJSTATUS;
			_obj[oSERBATOIOC2G]._mode &= ~OBJMODE_OBJSTATUS;
			_obj[oRAGAZZOP2G]._mode &= ~OBJMODE_OBJSTATUS;
			_obj[oRAGAZZOS2G]._mode |= OBJMODE_OBJSTATUS;
			_obj[oSERBATOIOA2G]._mode |= OBJMODE_OBJSTATUS;
			_obj[oPANNELLOE2G]._mode |= OBJMODE_OBJSTATUS;
			_obj[oPANNELLOM2G]._flag |= OBJFLAG_EXTRA;
			AnimTab[aBKG2G].flag |= SMKANIM_OFF1;
			printsent = false;
		} else
			printsent = true;

		break;

	case oRUOTE2C:
		if (!(_obj[od2CALLA2D]._mode & OBJMODE_OBJSTATUS)) {
			extern uint16 ruotepos[3];

			StopSmackAnim(Room[_curRoom]._bkgAnim);
			AnimTab[aBKG2C].flag |= SMKANIM_OFF1;
			_obj[oBASERUOTE2C]._mode |= OBJMODE_OBJSTATUS;
			_obj[omRUOTE2C]._mode |= OBJMODE_OBJSTATUS;
			_obj[oPULSANTE2C]._mode |= OBJMODE_OBJSTATUS;
			_obj[ruotepos[0] * 3 + 0 + oRUOTA1A2C]._mode |= OBJMODE_OBJSTATUS;
			_obj[ruotepos[1] * 3 + 1 + oRUOTA1A2C]._mode |= OBJMODE_OBJSTATUS;
			_obj[ruotepos[2] * 3 + 2 + oRUOTA1A2C]._mode |= OBJMODE_OBJSTATUS;
			_obj[oCAMPO2C]._mode &= ~OBJMODE_OBJSTATUS;
			_obj[oTEMPIO2C]._mode &= ~OBJMODE_OBJSTATUS;
			_obj[oLEONE2C]._mode &= ~OBJMODE_OBJSTATUS;
			_obj[od2CALLA2D]._mode &= ~OBJMODE_OBJSTATUS;
			_obj[oSFINGE2C]._mode &= ~OBJMODE_OBJSTATUS;
			_obj[oSTATUA2C]._mode &= ~OBJMODE_OBJSTATUS;
			_obj[oRUOTE2C]._mode &= ~OBJMODE_OBJSTATUS;
			_obj[od2CALLA2E]._mode &= ~OBJMODE_OBJSTATUS;
			_obj[oCARTELLOS2C]._mode &= ~OBJMODE_OBJSTATUS;
			_obj[oCARTELLOA2C]._mode &= ~OBJMODE_OBJSTATUS;
			_obj[od2CALLA26]._mode &= ~OBJMODE_OBJSTATUS;
			SemShowHomo = false;
			ReadExtraObj2C();
			RegenRoom();
		} else
			printsent = true;
		break;

	case oCATWALKA2E:
		if (!(_obj[oCATWALKA2E]._flag & OBJFLAG_EXTRA)) {
			PlayDialog(dF2E1);
			_obj[oDINOSAURO2E]._mode |= OBJMODE_OBJSTATUS;
			_obj[oCATWALKA2E]._flag |= OBJFLAG_EXTRA;
			AnimTab[aBKG2E].flag &= ~SMKANIM_OFF2;
			printsent = false;
		} else if (_obj[TheObj]._anim) {
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, _obj[TheObj]._anim, 0, 0, TheObj);
			printsent = false;
		} else
			printsent = true;
		break;

	case oBORSA22:
	case oPORTALAMPADE2B:
	case oMAPPAMONDO2B:
		if (_obj[TheObj]._anim) {
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, _obj[TheObj]._anim, 0, 0, TheObj);
			_obj[TheObj]._anim = 0;
		} else
			printsent = true;
		break;

	case oTUBOF34:
		if (!(_obj[oTUBOFT34]._mode & OBJMODE_OBJSTATUS))
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, _obj[TheObj]._anim, 0, 0, TheObj);
		else
			printsent = true;
		break;

	case oFILOT31:
		_obj[oFILOT31]._mode &= ~OBJMODE_OBJSTATUS;
		_obj[oFILOS31]._mode |= OBJMODE_OBJSTATUS;
		RegenRoom();
		break;

	case oCOPERCHIOA31:
		if (_obj[oFILOTC31]._mode & OBJMODE_OBJSTATUS) {
			NLPlaySound(wCOVER31);
			_obj[oPANNELLOM31]._mode &= ~OBJMODE_OBJSTATUS;
			_obj[oPANNELLOMA31]._mode |= OBJMODE_OBJSTATUS;
			_obj[oPANNELLOM31]._anim = 0;
			_obj[oPANNELLOM31]._examine = 715;
			_obj[oPANNELLOM31]._action = 716;
			_obj[oPANNELLOM31]._flag &= ~OBJFLAG_ROOMOUT;
			doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, r31, a3118CHIUDEPANNELLO, 3, TheObj);
		} else
			printsent = true;
		break;

	case oVALVOLAC34:
		if (_obj[TheObj]._anim)
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, _obj[TheObj]._anim, 0, 0, TheObj);
		else
			printsent = true;
		break;

	case oVALVOLA34:
		if (_obj[TheObj]._anim)
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, _obj[TheObj]._anim, 0, 0, TheObj);
		else
			printsent = true;
		break;

	case oPROIETTORE35:
		Room[r35]._flag |= OBJFLAG_EXTRA;
		read3D("352.3d");		    // dopo scossa

		_obj[oRIBELLEA35]._mode &= ~OBJMODE_OBJSTATUS;
		_obj[oPORTAC35]._mode &= ~OBJMODE_OBJSTATUS;
		_obj[omPORTAC35]._mode &= ~OBJMODE_OBJSTATUS;
		_obj[oPRESA35]._mode &= ~OBJMODE_OBJSTATUS;

		_obj[oPORTAA35]._mode |= OBJMODE_OBJSTATUS;
		_obj[omPORTAA35]._mode |= OBJMODE_OBJSTATUS;
		_obj[oRIBELLES35]._mode |= OBJMODE_OBJSTATUS;
		_obj[oSEDIA35]._mode |= OBJMODE_OBJSTATUS;
		_obj[oMONITOR35]._mode |= OBJMODE_OBJSTATUS;
		_obj[omPIANO35]._mode |= OBJMODE_OBJSTATUS;

		_obj[oSPORTELLOC35]._anim = a356PROVASPORTELLO;
		_obj[oASCENSORE35]._flag |= OBJFLAG_ROOMOUT;
		_obj[oASCENSORE35]._anim = a3514ENTRAASCENSORE;

		AnimTab[aBKG35].flag |= SMKANIM_OFF1;
		PlayDialog(dF351);
		setPosition(7);
		break;

	case oCOMPUTER36:
		if (!(_choice[646]._flag & OBJFLAG_DONE)) {
			PlayDialog(dF361);
			_obj[oCOMPUTER36]._action = 2004;
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
		_obj[TheObj]._mode &= ~OBJMODE_OBJSTATUS;
		_obj[TheObj + 7]._mode |= OBJMODE_OBJSTATUS;
		Comb35[Count35++] = TheObj;
		NLPlaySound(wPAD5);
		if (Count35 == 7) {
			if (((Comb35[0] == oPULSANTEF35) && (Comb35[1] == oPULSANTED35) && (Comb35[2] == oPULSANTEC35) &&
					(Comb35[3] == oPULSANTEG35) && (Comb35[4] == oPULSANTEB35) && (Comb35[5] == oPULSANTEA35) &&
					(Comb35[6] == oPULSANTEE35)) || ((Comb35[0] == oPULSANTEE35) &&
							(Comb35[1] == oPULSANTEA35) && (Comb35[2] == oPULSANTEB35) && (Comb35[3] == oPULSANTEG35) &&
							(Comb35[4] == oPULSANTEC35) && (Comb35[5] == oPULSANTED35) && (Comb35[6] == oPULSANTEF35))) {
				_obj[oPULSANTEAA35]._mode &= ~OBJMODE_OBJSTATUS;
				_obj[oPULSANTEBA35]._mode &= ~OBJMODE_OBJSTATUS;
				_obj[oPULSANTECA35]._mode &= ~OBJMODE_OBJSTATUS;
				_obj[oPULSANTEDA35]._mode &= ~OBJMODE_OBJSTATUS;
				_obj[oPULSANTEEA35]._mode &= ~OBJMODE_OBJSTATUS;
				_obj[oPULSANTEFA35]._mode &= ~OBJMODE_OBJSTATUS;
				_obj[oPULSANTEGA35]._mode &= ~OBJMODE_OBJSTATUS;
				_obj[oPULSANTIV35]._mode |= OBJMODE_OBJSTATUS;
				_obj[oLEDS35]._mode &= ~OBJMODE_OBJSTATUS;

				_obj[oSPORTELLOA35]._anim = 0;
//					_obj[oSPORTELLOA35]._examine = 1843;
				_obj[oSPORTELLOA35]._action = 1844;
				_obj[oSPORTELLOA35]._flag |= OBJFLAG_EXTRA;
				_obj[oPORTAMC36]._flag |= OBJFLAG_ROOMOUT;
				_obj[oPORTAMC36]._anim = a3610APREPORTA;
				_obj[oSCAFFALE36]._anim = a3615APRENDESCAFFALE;

				AnimTab[aBKG36].flag |= SMKANIM_OFF2;
				AnimTab[aBKG36].flag |= SMKANIM_OFF3;
				_obj[oSCANNERLA36]._mode &= ~OBJMODE_OBJSTATUS;
				_obj[oSCANNERLS36]._mode |= OBJMODE_OBJSTATUS;
				_obj[oSCANNERMA36]._mode &= ~OBJMODE_OBJSTATUS;
				_obj[oSCANNERMS36]._mode |= OBJMODE_OBJSTATUS;

				NLPlaySound(wWIN35);
			} else {
				_obj[oPULSANTEA35]._mode |= OBJMODE_OBJSTATUS;
				_obj[oPULSANTEB35]._mode |= OBJMODE_OBJSTATUS;
				_obj[oPULSANTEC35]._mode |= OBJMODE_OBJSTATUS;
				_obj[oPULSANTED35]._mode |= OBJMODE_OBJSTATUS;
				_obj[oPULSANTEE35]._mode |= OBJMODE_OBJSTATUS;
				_obj[oPULSANTEF35]._mode |= OBJMODE_OBJSTATUS;
				_obj[oPULSANTEG35]._mode |= OBJMODE_OBJSTATUS;
				_obj[oPULSANTEAA35]._mode &= ~OBJMODE_OBJSTATUS;
				_obj[oPULSANTEBA35]._mode &= ~OBJMODE_OBJSTATUS;
				_obj[oPULSANTECA35]._mode &= ~OBJMODE_OBJSTATUS;
				_obj[oPULSANTEDA35]._mode &= ~OBJMODE_OBJSTATUS;
				_obj[oPULSANTEEA35]._mode &= ~OBJMODE_OBJSTATUS;
				_obj[oPULSANTEFA35]._mode &= ~OBJMODE_OBJSTATUS;
				_obj[oPULSANTEGA35]._mode &= ~OBJMODE_OBJSTATUS;
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
		_curObj += 7;
		RegenRoom();
		break;

	case oCARD35:
		_obj[oPULSANTEA35]._mode |= OBJMODE_OBJSTATUS;
		_obj[oPULSANTEB35]._mode |= OBJMODE_OBJSTATUS;
		_obj[oPULSANTEC35]._mode |= OBJMODE_OBJSTATUS;
		_obj[oPULSANTED35]._mode |= OBJMODE_OBJSTATUS;
		_obj[oPULSANTEE35]._mode |= OBJMODE_OBJSTATUS;
		_obj[oPULSANTEF35]._mode |= OBJMODE_OBJSTATUS;
		_obj[oPULSANTEG35]._mode |= OBJMODE_OBJSTATUS;
		_obj[oPULSANTEAA35]._mode &= ~OBJMODE_OBJSTATUS;
		_obj[oPULSANTEBA35]._mode &= ~OBJMODE_OBJSTATUS;
		_obj[oPULSANTECA35]._mode &= ~OBJMODE_OBJSTATUS;
		_obj[oPULSANTEDA35]._mode &= ~OBJMODE_OBJSTATUS;
		_obj[oPULSANTEEA35]._mode &= ~OBJMODE_OBJSTATUS;
		_obj[oPULSANTEFA35]._mode &= ~OBJMODE_OBJSTATUS;
		_obj[oPULSANTEGA35]._mode &= ~OBJMODE_OBJSTATUS;
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
			_obj[a]._mode &= ~OBJMODE_OBJSTATUS;
		_obj[oASTAC49 + TheObj - oFOROC49]._mode |= OBJMODE_OBJSTATUS;
		Comb49[3] = Comb49[2];
		Comb49[2] = Comb49[1];
		Comb49[1] = Comb49[0];
		Comb49[0] = TheObj;
		NLPlaySound(wASTA49);
		RegenRoom();
		if ((Comb49[3] ==  oFORO749) && (Comb49[2] ==  oFORO849) && (Comb49[1] ==  oFORO449) && (Comb49[0] ==  oFORO549)) {
			PaintScreen(0);
			NlDelay(60);
			_obj[oOMBRAS49]._mode |= OBJMODE_OBJSTATUS;
			_obj[oSCOMPARTO49]._mode |= OBJMODE_OBJSTATUS;
			_obj[oAGENDA49]._mode |= OBJMODE_OBJSTATUS;
			_obj[oMERIDIANA49]._mode &= ~OBJMODE_OBJSTATUS;
			_obj[oAGENDA49]._examine = 1099;
			_obj[oAGENDA49]._action = 1100;
			SemCharacterExist = true;
			_curObj = oAGENDA49;
			//SemShowHomo=true;
			//doEvent(MC_SYSTEM,ME_CHANGEROOM,MP_SYSTEM,r4A,0,1,TheObj);
			PlayScript(s49MERIDIANA);
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
			_obj[a]._mode &= ~OBJMODE_OBJSTATUS;
		_obj[oASTAC49]._mode |= OBJMODE_OBJSTATUS;
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
				_obj[a + oAST14C]._mode |= OBJMODE_OBJSTATUS;
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
				_obj[oAST14C + a]._mode &= ~OBJMODE_OBJSTATUS;
			}
			doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, r51, 0, 1, TheObj);
			SemCharacterExist = true;
			//SemShowHomo=true;
		} else {
			for (a = 0; a < 6; a++) {
				Comb4CT[a] = 0;
				_obj[oAST14C + a]._mode &= ~OBJMODE_OBJSTATUS;
			}
			doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, r4C, 0, 4, TheObj);
			SemCharacterExist = true;
			//SemShowHomo=true;
		}
		printsent = false;
		break;

	case oPORTAC4A:
		if (!(_choice[245]._flag & OBJFLAG_DONE)  && !(_choice[766]._flag & OBJFLAG_DONE)) {
			_choice[245]._flag &= ~DLGCHOICE_HIDE;
			PlayDialog(dC4A1);
			setPosition(14);
			_obj[oPORTAC4A]._action = 1117;
			_obj[oPORTAC4A]._anim = 0;
			printsent = false;
		} else
			printsent = true;
		break;

	case oPULSANTE4A:
		if (_obj[TheObj]._anim)
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, _obj[TheObj]._anim, 0, 0, TheObj);
		else if (!(_choice[244]._flag & OBJFLAG_DONE)) {
			_choice[244]._flag &= ~DLGCHOICE_HIDE;
			PlayDialog(dC4A1);
			_obj[oPULSANTE4A]._examine = 1108;
			_obj[oPULSANTE4A]._action = 1109;
			printsent = false;
		} else
			printsent = true;
		break;

	case oFINESTRA56:
		if (_obj[oPANNELLOC56]._mode & OBJMODE_OBJSTATUS)
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a5614, 0, 0, TheObj);
		else
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a5614P, 0, 0, TheObj);
		break;

	case oPULSANTECD:
		for (a = oPULSANTE1AD; a <= oPULSANTE33AD; a++) {
			if ((_obj[a]._goRoom == _obj[oEXIT41D]._goRoom) ||
					((_obj[a]._goRoom == r45) && (_obj[oEXIT41D]._goRoom == r45S))) {
				CharacterSay(903);
				break;
			} else if (_obj[a]._goRoom == 0) {
				if (_obj[oEXIT41D]._goRoom == r45S)
					_obj[a]._goRoom = r45;
				else
					_obj[a]._goRoom = _obj[oEXIT41D]._goRoom;
				_obj[a]._mode |= OBJMODE_OBJSTATUS;
				_obj[a - 40]._mode &= ~OBJMODE_OBJSTATUS;
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
		if ((_obj[TheObj]._anim) && (_obj[oSECCHIOS54]._flag & OBJFLAG_EXTRA) && (_obj[oGRATAC54]._mode & OBJMODE_OBJSTATUS) && !(_choice[841]._flag & OBJFLAG_DONE))
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, _obj[TheObj]._anim, 0, 0, TheObj);
		else
			printsent = true;
		break;

	case oSECCHIOS54:
		if (_obj[oGRATAC54]._mode & OBJMODE_OBJSTATUS)
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a544G, 0, 0, TheObj);
		else
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, _obj[TheObj]._anim, 0, 0, TheObj);
		printsent = false;
		break;

	case oTASTIERA56:
		if (_choice[262]._flag & OBJFLAG_DONE) {
			if (_obj[od56ALLA59]._mode & OBJMODE_OBJSTATUS)
				CharacterSay(_obj[TheObj]._action);
			else
				doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a567, 0, 0, TheObj);
			printsent = false;
		} else {
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a564, 0, 0, TheObj);
			printsent = false;
		}
		break;

	case oLIBRIEG2B:
		if (Room[r2C]._flag & OBJFLAG_DONE) { // se sono gia' stato nella 2C prendo libro
			printsent = false;
			if (_obj[TheObj]._anim)
				doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a2B4PRENDELIBRO, 0, 0, TheObj);
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
		_obj[oLED158 + Count58]._mode |= OBJMODE_OBJSTATUS;
		Count58 ++;
		RegenRoom();
		if (Count58 < 6)
			break;

		PaintScreen(0);
		NlDelay(60);
		Count58 = 0;
		for (a = 0; a < 6; a++)
			_obj[oLED158 + a]._mode &= ~OBJMODE_OBJSTATUS;

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
		if (_obj[TheObj]._anim)
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, _obj[TheObj]._anim, 0, 0, TheObj);
		else
			printsent = true;
		break;
	}

	if ((printsent) && (_obj[TheObj]._action)) CharacterSay(_obj[TheObj]._action);
}

/*-------------------------------------------------------------------------*/
/*                                  TAKE           						   */
/*-------------------------------------------------------------------------*/
void doMouseTake(uint16 TheObj) {
	LLBOOL del = true;

	if (!TheObj)
		warning("doMouseTake");

// SemMouseEnabled = false;
	switch (TheObj) {
	case oTINFOIL11:
		del = false;
		break;
	case oNASTRO15:
		_obj[oNASTRO15]._flag |= OBJFLAG_EXTRA;
		del = false;
		break;
	case oMONETA13:
		if (!(_obj[oLATTINA13]._mode & OBJMODE_OBJSTATUS))
			_obj[TheObj]._anim = a133CPRENDEMONETA;
		break;
	case oFOGLIETTO14:
		_obj[oFOGLIETTO14]._flag |= OBJFLAG_EXTRA;
		_obj[oMAPPA16]._examine = 152;
		del = false;
		break;
	case oPOSTERC22:
		_obj[oARMADIETTOCC22]._anim = a221;
		_obj[oARMADIETTOCA22]._anim = a222;
		break;
	case oKEY22:
		_obj[oARMADIETTORA22]._examine = 2013;
		del = true;
		break;
	default:
		del = true;
		break;
	}
	uint16 TheAction = _obj[TheObj]._anim;

	if (TheAction)
		doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, TheAction, 0, 0, TheObj);

	// spegne oggetto che viene preso
	if (del) {
		if (TheAction) {
			for (uint16 j = 0; j < MAXATFRAME; j++) {
				if ((AnimTab[TheAction].atframe[j].type == ATFCLR) && (AnimTab[TheAction].atframe[j].index == TheObj))
					break;

				if (AnimTab[TheAction].atframe[j].type == 0) {
					AnimTab[TheAction].atframe[j].child = 0;
					AnimTab[TheAction].atframe[j].nframe = 1;
					AnimTab[TheAction].atframe[j].type = ATFCLR;
					AnimTab[TheAction].atframe[j].index = TheObj;
					break;
				}
			}
		} else {
			_obj[TheObj]._mode &= ~OBJMODE_OBJSTATUS;
			RegenRoom();
		}
	}
	AddIcon(_obj[_curObj]._ninv);
}

/*-------------------------------------------------------------------------*/
/*                                  TALK           						   */
/*-------------------------------------------------------------------------*/
void doMouseTalk(uint16 TheObj) {
	extern int16 TextStackTop;

	if (!TheObj)
		warning("doMouseTalk");

	switch (TheObj) {
	case oBIGLIETTERIA16:
		if ((_obj[oFINGERPADP16]._flag & OBJFLAG_ROOMOUT) && (_choice[50]._flag & OBJFLAG_DONE)) {
			CharacterSay(147);
			return ;
		} else if ((_choice[49]._flag & DLGCHOICE_HIDE) && (_choice[50]._flag & DLGCHOICE_HIDE)) {
			if (_obj[oMAPPA16]._flag & OBJFLAG_EXTRA) {
				_choice[46]._flag &= ~DLGCHOICE_HIDE;
				_choice[48]._flag &= ~DLGCHOICE_HIDE;
				_obj[oBIGLIETTERIA16]._flag |= OBJFLAG_EXTRA;
			} else {
				if (_choice[46]._flag & OBJFLAG_DONE) {
					CharacterSay(_obj[oBIGLIETTERIA16]._action);
					return ;
				} else {
					_choice[46]._flag &= ~DLGCHOICE_HIDE;
					_choice[47]._flag &= ~DLGCHOICE_HIDE;
				}
			}
		}
		break;

	case ocGUARDIANO18:
		_obj[ocGUARDIANO18]._flag &= ~OBJFLAG_PERSON;
		_obj[ocGUARDIANO18]._action = 227;
		_obj[oPORTAC18]._action = 220;
		break;

	case ocNEGOZIANTE1A: {
		int c;
		for (c = _dialog[dNEGOZIANTE1A]._firstChoice; c < (_dialog[dNEGOZIANTE1A]._firstChoice + _dialog[dNEGOZIANTE1A]._choiceNumb); c++)
			if (!(_choice[c]._flag & DLGCHOICE_HIDE)) {
				PlayDialog(_obj[TheObj]._goRoom);
				return;
			}
		if (_obj[ocNEGOZIANTE1A]._action) {
			CharacterSay(_obj[ocNEGOZIANTE1A]._action);
			return;
		}
	}
	break;

	case ocEVA19:
		InvObj[iSAM]._action = 1415;
		break;
	}

	PlayDialog(_obj[TheObj]._goRoom);
}

/*-------------------------------------------------------------------------*/
/*                                USE WITH           					   */
/*-------------------------------------------------------------------------*/
void doUseWith() {
	if (UseWithInv[USED]) {
		if (UseWithInv[WITH]) doInvInvUseWith();
		else doInvScrUseWith();
	} else doScrScrUseWith();

	UseWith[USED] = 0;
	UseWith[WITH] = 0;
	UseWithInv[USED] = false;
	UseWithInv[WITH] = false;
	SemUseWithStarted = false;
}

/*-------------------------------------------------------------------------*/
/*                          USE WITH / INV - INV         				   */
/*-------------------------------------------------------------------------*/
void doInvInvUseWith() {
	LLBOOL updateinv = true, printsent = true;

	if ((!UseWith[USED]) || (!UseWith[WITH]))
		warning("doInvInvUseWith");

	StopSmackAnim(InvObj[UseWith[USED]]._anim);

	switch (UseWith[USED]) {
	//	TT
	case iSTAGNOLA:
		if (UseWith[WITH] == iFUSIBILE) {
			KillIcon(iSTAGNOLA);
			ReplaceIcon(iFUSIBILE, iFUSIBILES);
			StartCharacterAction(hUSEGG, 0, 0, 1441);
			printsent = false;
		}
		break;

	case iFUSIBILE:
		if (UseWith[WITH] == iSTAGNOLA) {
			KillIcon(iSTAGNOLA);
			ReplaceIcon(iFUSIBILE, iFUSIBILES);
			StartCharacterAction(hUSEGG, 0, 0, 1441);
			printsent = false;
		}

	// GG
	case iTOPO1C:
		if (UseWith[WITH] == iPATTINO) {
			KillIcon(iPATTINO);
			KillIcon(iTOPO1C);
			AddIcon(iTOPO1D);
			StartCharacterAction(hUSEGG, 0, 0, 1497);
			printsent = false;
		}
		break;

	case iPATTINO:
		if (UseWith[WITH] == iTOPO1C) {
			KillIcon(iPATTINO);
			KillIcon(iTOPO1C);
			AddIcon(iTOPO1D);
			StartCharacterAction(hUSEGG, 0, 0, 1497);
			printsent = false;
		}
		break;

	// GG
	case iSBARRA11:
		if (UseWith[WITH] == iMAGNETE) {
			KillIcon(iSBARRA11);
			ReplaceIcon(iMAGNETE, iSBARRA21);
			StartCharacterAction(hUSEGG, 0, 0, 1438);
			printsent = false;
		}
		break;

	case iMAGNETE:
		if (UseWith[WITH] == iSBARRA11) {
			KillIcon(iSBARRA11);
			ReplaceIcon(iMAGNETE, iSBARRA21);
			StartCharacterAction(hUSEGG, 0, 0, 1533);
			printsent = false;
		}
		break;

	case iSIGARO:
		if (UseWith[WITH] == iSCOPA27) {
			KillIcon(iSCOPA27);
			ReplaceIcon(iSIGARO, iTORCIA32);
			StartCharacterAction(hUSEGG, 0, 0, 1575);
			printsent = false;
		}
		break;

	case iSCOPA27:
		if (UseWith[WITH] == iSIGARO) {
			KillIcon(iSCOPA27);
			ReplaceIcon(iSIGARO, iTORCIA32);
			StartCharacterAction(hUSEGG, 0, 0, 1546);
			printsent = false;
		}
		break;

	case iPROIETTORE31:
		if (UseWith[WITH] == iTRIPLA) {
			KillIcon(iTRIPLA);
			ReplaceIcon(iPROIETTORE31, iPROIETTORE35);
			StartCharacterAction(hUSEGG, 0, 0, 0);
			printsent = false;
		}
		break;

	case iTRIPLA:
		if (UseWith[WITH] == iPROIETTORE31) {
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
		if ((UseWith[WITH] == iSALNITRO) || (UseWith[WITH] == iCARBONE) || (UseWith[WITH] == iZOLFO) ||
				(UseWith[WITH] == iCARSAL) || (UseWith[WITH] == iCARZOL) || (UseWith[WITH] == iSALZOL)) {
			KillIcon(UseWith[USED]);
			KillIcon(UseWith[WITH]);

			if (((UseWith[USED] == iSALNITRO) && (UseWith[WITH] == iCARBONE)) ||
					((UseWith[WITH] == iSALNITRO) && (UseWith[USED] == iCARBONE)))
				AddIcon(iCARSAL);
			if (((UseWith[USED] == iZOLFO) && (UseWith[WITH] == iCARBONE)) ||
					((UseWith[WITH] == iZOLFO) && (UseWith[USED] == iCARBONE)))
				AddIcon(iCARZOL);
			if (((UseWith[USED] == iZOLFO) && (UseWith[WITH] == iSALNITRO)) ||
					((UseWith[WITH] == iZOLFO) && (UseWith[USED] == iSALNITRO)))
				AddIcon(iSALZOL);

			if ((UseWith[USED] == iZOLFO) || (UseWith[WITH] == iZOLFO))
				AddIcon(iBARATTOLO);
			if ((UseWith[USED] >= iCARSAL) || (UseWith[WITH] >= iCARSAL))
				AddIcon(iPOLVERE48);
			StartCharacterAction(hUSEGG, 0, 0, 1663);
			printsent = false;
		}
		break;

	case iPISTOLA4B:
		if (UseWith[WITH] == iPOLVERE48) {
			ReplaceIcon(iPOLVERE48, iPOLVERE4P);
			ReplaceIcon(iPISTOLA4B, iPISTOLA4PC);
			StartCharacterAction(hUSEGG, 0, 0, 1676);
			printsent = false;
		} else if (UseWith[WITH] == iPOLVERE4P) {
			KillIcon(iPOLVERE4P);
			ReplaceIcon(iPISTOLA4B, iPISTOLA4PC);
			StartCharacterAction(hUSEGG, 0, 0, 1700);
			printsent = false;
		}
		break;

	case iPOLVERE48:
		if (UseWith[WITH] == iPISTOLA4B) {
			ReplaceIcon(iPOLVERE48, iPOLVERE4P);
			ReplaceIcon(iPISTOLA4B, iPISTOLA4PC);
			StartCharacterAction(hUSEGG, 0, 0, 1676);
			printsent = false;
		}
		break;

	case iPOLVERE4P:
		if (UseWith[WITH] == iPISTOLA4B) {
			KillIcon(iPOLVERE4P);
			ReplaceIcon(iPISTOLA4B, iPISTOLA4PC);
			StartCharacterAction(hUSEGG, 0, 0, 1700);
			printsent = false;
		}
		break;

	case iBIGLIAA:
	case iBIGLIAB:
		if ((UseWith[WITH] == iPISTOLA4PC) && !(InvObj[iPISTOLA4PC]._flag & OBJFLAG_EXTRA)) {
			KillIcon(UseWith[USED]);
			ReplaceIcon(iPISTOLA4PC, iPISTOLA4PD);
			StartCharacterAction(hUSEGG, 0, 0, 1683);
			InvObj[iPISTOLA4PC]._flag |= OBJFLAG_EXTRA;
			printsent = false;
		} else if (UseWith[WITH] == iPISTOLA4PC) {
			CharacterSay(1688);
			printsent = false;
		} else if (UseWith[WITH] == iPISTOLA4B) {
			CharacterSay(2009);
			printsent = false;
		}
		break;

	case iBIGLIA4U:
		if (UseWith[WITH] == iPISTOLA4PC) {
			KillIcon(iBIGLIA4U);
			ReplaceIcon(iPISTOLA4PC, iPISTOLA4PD);
			StartCharacterAction(hUSEGG, 0, 0, 1718);
			InvObj[iPISTOLA4PD]._flag |= OBJFLAG_EXTRA;
			printsent = false;
		} else if (UseWith[WITH] == iPISTOLA4B) {
			CharacterSay(2011);
			printsent = false;
		}
		break;

	case iSIRINGA37:
		if (UseWith[WITH] == iFIALE) {
			KillIcon(iSIRINGA37);
			ReplaceIcon(iFIALE, iSIRINGA59);
			StartCharacterAction(hUSEGG, 0, 0, 1756);
			printsent = false;
		}
		break;

	case iFIALE:
		if (UseWith[WITH] == iSIRINGA37) {
			KillIcon(iSIRINGA37);
			ReplaceIcon(iFIALE, iSIRINGA59);
			StartCharacterAction(hUSEGG, 0, 0, 1756);
			printsent = false;
		}
		break;

	case iFILO:
		if (UseWith[WITH] == iGUANTI57) {
			KillIcon(iFILO);
			ReplaceIcon(iGUANTI57, iGUANTI5A);
			StartCharacterAction(hUSEGG, 0, 0, 1756);
			printsent = false;
		} else if (UseWith[WITH] == iSIRINGA59) {
			KillIcon(iFILO);
			ReplaceIcon(iSIRINGA59, iSIRINGA5A);
			StartCharacterAction(hUSEGG, 0, 0, 1756);
			printsent = false;
		}
		break;

	case iGUANTI57:
		if (UseWith[WITH] == iFILO) {
			KillIcon(iFILO);
			ReplaceIcon(iGUANTI57, iGUANTI5A);
			StartCharacterAction(hUSEGG, 0, 0, 1756);
			printsent = false;
		} else if (UseWith[WITH] == iSIRINGA5A) {
			KillIcon(iSIRINGA5A);
			ReplaceIcon(iGUANTI57, iARMAEVA);
			StartCharacterAction(hUSEGG, 0, 0, 1756);
			printsent = false;
		}
		break;

	case iSIRINGA59:
		if (UseWith[WITH] == iFILO) {
			KillIcon(iFILO);
			ReplaceIcon(iSIRINGA59, iSIRINGA5A);
			StartCharacterAction(hUSEGG, 0, 0, 1756);
			printsent = false;
		} else if (UseWith[WITH] == iGUANTI5A) {
			KillIcon(iSIRINGA59);
			ReplaceIcon(iGUANTI5A, iARMAEVA);
			StartCharacterAction(hUSEGG, 0, 0, 1756);
			printsent = false;
		}
		break;

	case iGUANTI5A:
		if (UseWith[WITH] == iSIRINGA59) {
			KillIcon(iSIRINGA59);
			ReplaceIcon(iGUANTI5A, iARMAEVA);
			StartCharacterAction(hUSEGG, 0, 0, 1756);
			printsent = false;
		}
		break;

	case iSIRINGA5A:
		if (UseWith[WITH] == iGUANTI57) {
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

	if (printsent) CharacterSay(InvObj[UseWith[USED]]._action);
	if (updateinv) RegenInv(TheIconBase, INVENTORY_SHOW);
}

/*-------------------------------------------------------------------------*/
/*                          USE WITH / INV - SCR         				   */
/*-------------------------------------------------------------------------*/
void doInvScrUseWith() {
	LLBOOL updateinv = true, printsent = true;
	extern uint16 _curAnimFrame[];

	if ((!UseWith[USED]) || (!UseWith[WITH]))
		warning("doInvScrUseWith");

	StopSmackAnim(InvObj[UseWith[USED]]._anim);
	if (_homoInMovement)
		return;
	switch (UseWith[USED]) {
	case iBANCONOTE:
		if (UseWith[WITH] == oDISTRIBUTORE13 && !(_obj[oDISTRIBUTORE13]._flag & OBJFLAG_EXTRA)) {
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a131USABANCONOTA, 0, 0, UseWith[WITH]);
			printsent = false;
			_obj[oDISTRIBUTORE13]._flag |= OBJFLAG_EXTRA;
		} else if ((UseWith[WITH] == oDISTRIBUTORE13) && (_obj[oDISTRIBUTORE13]._flag & OBJFLAG_EXTRA) && (_obj[oLATTINA13]._mode & OBJMODE_OBJSTATUS)) {
			CharacterSay(1410);
			printsent = false;
		} else if ((UseWith[WITH] == oDISTRIBUTORE13) && (_obj[oDISTRIBUTORE13]._flag & OBJFLAG_EXTRA)) {
			if (!(_obj[oSCOMPARTO13]._flag & OBJFLAG_EXTRA)) {
				_obj[oSCOMPARTO13]._flag |= OBJFLAG_EXTRA;
				doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a1311DABOTTADISTRIBUTORE, 0, 0, UseWith[WITH]);
			} else
				CharacterSay(1411);

			printsent = false;
		} else if (UseWith[WITH] == oBIGLIETTERIA16) {
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a165USABANCONOTA, 0, 0, UseWith[WITH]);
			InvObj[iBANCONOTE]._flag |= OBJFLAG_EXTRA;
			printsent = false;
		} else if ((UseWith[WITH] == ocPOLIZIOTTO16) && (InvObj[iBANCONOTE]._flag & OBJFLAG_EXTRA)) {
			_choice[62]._flag &= ~DLGCHOICE_HIDE;
			PlayDialog(dPOLIZIOTTO16);
			printsent = false;
		}
		break;
	case iLETTERA12:
		if (UseWith[WITH] == oPENPADA13) {
			if (Room[r14]._flag & OBJFLAG_DONE)
				doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a1312METTELETTERARICALCA, r14, 14, UseWith[WITH]);
			else {
				doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a135METTELETTERA, 0, 0, UseWith[WITH]);
				_obj[oLETTERA13]._mode |= OBJMODE_OBJSTATUS;
				InvObj[iLETTERA12]._flag |= OBJFLAG_EXTRA;
				KillIcon(iLETTERA12);
				RegenRoom();
			}
			printsent = false;
		}
		break;
	case iFUSIBILES:
		if (UseWith[WITH] == oPANNELLOA12) {
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a127USAFUSIBILEAVVOLTO, 0, 0, UseWith[WITH]);
			printsent = false;
		}
		break;
	case iFUSIBILE:
		if (UseWith[WITH] == oPANNELLOA12) {
			CharacterSay(62);
			printsent = false;
		}
		break;
	case iKEY05:
		if (UseWith[WITH] == oCASSETTE12) {
			if (!(_obj[oCASSETTE12]._flag & OBJFLAG_EXTRA)) {
				doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a121PROVACASSETTA, 0, 0, UseWith[WITH]);
				_obj[oCASSETTE12]._flag |= OBJFLAG_EXTRA;
				printsent = false;
			} else {
				CharacterSay(1426);
				printsent = false;
			}
		} else if (UseWith[WITH] == oCASSETTA12 && !(InvObj[iLETTERA12]._flag & OBJFLAG_EXTRA)) {
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a122APRECASSETTA, 0, 0, oCASSETTA12);
			InvObj[iLETTERA12]._flag |= OBJFLAG_EXTRA;
			printsent = false;
		} else if ((UseWith[WITH] == oCASSETTA12) && (InvObj[iLETTERA12]._flag & OBJFLAG_EXTRA)) {
			CharacterSay(1429);
			printsent = false;
		} else if ((UseWith[WITH] == oCARA11) || (UseWith[WITH] == oCARB11) || (UseWith[WITH] == oPORTAR12) || (UseWith[WITH] == oCASSAFORTEC15) || (UseWith[WITH] == oPORTONE18) || (UseWith[WITH] == oLUCCHETTO1B) || (UseWith[WITH] == oPORTAC21) || (UseWith[WITH] == oPANNELLOC23) || (UseWith[WITH] == oPORTA2A) || (UseWith[WITH] == oPORTAC33) || (UseWith[WITH] == oSPORTELLOC35) || (UseWith[WITH] == oCASSETTOC36) || (UseWith[WITH] == oPORTAC54) || (UseWith[WITH] == oPORTA57C55) || (UseWith[WITH] == oPORTA58C55) || (UseWith[WITH] == oPORTAS56) || (UseWith[WITH] == oPORTAS57)) {
			printsent = false;
			CharacterSay(1426);
		}
		break;
	case iCARD03:
		if (UseWith[WITH] == oSLOT12 || UseWith[WITH] == oSLOT13 || UseWith[WITH] == oSLOT16) {
			printsent = false;
			InvObj[iCARD03]._flag |= OBJFLAG_EXTRA;
			_obj[oSLOT12]._flag |= OBJFLAG_PERSON;
			_obj[oASCENSOREA12]._flag |= OBJFLAG_PERSON;
			doMouseTalk(UseWith[WITH]);
		} else if ((UseWith[WITH] == oBIGLIETTERIA16) || (UseWith[WITH] == oSLOT23) || (UseWith[WITH] == oSPORTELLOA35) || (UseWith[WITH] == oSLOTA58) || (UseWith[WITH] == oSLOTB58)) {
			printsent = false;
			CharacterSay(1419);
		}
		break;
	case iPEN:
		printsent = false;
		if (((UseWith[WITH] == oPENPADA13) || (UseWith[WITH] == oLETTERA13)) && (_obj[oLETTERA13]._mode & OBJMODE_OBJSTATUS))
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a137RICALCAFIRMA, r14, 14, UseWith[WITH]);
		else if ((UseWith[WITH] == oPENPADA13) && (Room[r14]._flag & OBJFLAG_DONE))
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a1312METTELETTERARICALCA, r14, 14, UseWith[WITH]);
		else if (UseWith[WITH] == oPENPADA13) {
			if (!(_obj[oCASSETTA12]._mode & OBJMODE_OBJSTATUS)) {
				printsent = false;
				CharacterSay(2005);
			} else
				doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a134USAMAGNETICPEN, 0, 0, UseWith[WITH]);
		} else
			printsent = true;
		break;

	case iACIDO15:
		if (UseWith[WITH] == oBAR11) {
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a113USAFIALA, 0, 0, UseWith[WITH]);
			ReplaceIcon(iACIDO15, iFIALAMETA);
			//AnimTab[a113USAFIALA].atframe[0].index = 1483;
			printsent = false;
		} else if (UseWith[WITH] == oLUCCHETTO1B) {
			if (_obj[oTOMBINOA1B]._mode & OBJMODE_OBJSTATUS)
				doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a1B2AVERSAACIDO, 0, 0, UseWith[WITH]);
			else
				doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a1B2VERSAACIDO, 0, 0, UseWith[WITH]);
			_obj[oBOTOLAC1B]._anim = a1B3APREBOTOLA;
			ReplaceIcon(iACIDO15, iFIALAMETA);
			AnimTab[a113USAFIALA].atframe[0].index = 1483;
			printsent = false;
		} else if ((UseWith[WITH] == ocGUARDIANO18) || (UseWith[WITH] == oTOMBINOC1B)) {
			printsent = false;
			CharacterSay(1476);
		}
		break;

	case iFIALAMETA:
		if (UseWith[WITH] == oBAR11) {
			AnimTab[a113USAFIALA].atframe[0].index = 1483;
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a113USAFIALA, 0, 0, UseWith[WITH]);
			KillIcon(iFIALAMETA);
			printsent = false;
		} else if (UseWith[WITH] == oLUCCHETTO1B) {
			AnimTab[a1B2AVERSAACIDO].atframe[2].index = 1483;
			AnimTab[a1B2VERSAACIDO].atframe[2].index = 1483;
			if (_obj[oTOMBINOA1B]._mode & OBJMODE_OBJSTATUS)
				doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a1B2AVERSAACIDO, 0, 0, UseWith[WITH]);
			else
				doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a1B2VERSAACIDO, 0, 0, UseWith[WITH]);
			_obj[oBOTOLAC1B]._anim = a1B3APREBOTOLA;
			KillIcon(iFIALAMETA);
			printsent = false;
		} else if ((UseWith[WITH] == oPORTA2A) || (UseWith[WITH] == oPORTA2B)) {
			printsent = false;
			CharacterSay(1508);
		}
		break;

	case iKEY15:
		if (UseWith[WITH] == oCASSAFORTEC15) {
			// ParteFli
			PlayDialog(dF151);
			_obj[oCASSAFORTEC15]._mode &= ~OBJMODE_OBJSTATUS;
			_obj[oCASSAFORTEA15]._mode |= OBJMODE_OBJSTATUS;
			_obj[oSLOT13]._mode &= ~OBJMODE_OBJSTATUS;
			_obj[oASCENSOREA13]._mode &= ~OBJMODE_OBJSTATUS;
			_obj[oASCENSOREC13]._mode |= OBJMODE_OBJSTATUS;
			AddIcon(iPISTOLA15);
			AddIcon(iACIDO15);
			AddIcon(iRAPPORTO);
			KillIcon(iKEY15);
			printsent = false;
			updateinv = false;
		} else if ((UseWith[WITH] == oCARA11) || (UseWith[WITH] == oCARB11) || (UseWith[WITH] == oPORTAR12) || (UseWith[WITH] == oCASSETTA12) || (UseWith[WITH] == oPORTONE18) || (UseWith[WITH] == oLUCCHETTO1B) || (UseWith[WITH] == oPORTAC21) || (UseWith[WITH] == oPANNELLOC23) || (UseWith[WITH] == oPORTA2A) || (UseWith[WITH] == oPORTAC33) || (UseWith[WITH] == oSPORTELLOC35) || (UseWith[WITH] == oCASSETTOC36) || (UseWith[WITH] == oPORTAC54) || (UseWith[WITH] == oPORTA57C55) || (UseWith[WITH] == oPORTA58C55) || (UseWith[WITH] == oPORTAS56) || (UseWith[WITH] == oPORTAS57)) {
			printsent = false;
			CharacterSay(1469);
		}
		break;

	case iSBARRA11:
		if (UseWith[WITH] == oTOMBINOC1B) {
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a1B1USASBARRA, 0, 0, UseWith[WITH]);
			_obj[oBOTOLAA1B]._anim = a1B6ASCENDEBOTOLA;
			//_obj[oBOTOLAC1B]._anim = a1B3AAPREBOTOLA;
			printsent = false;
		} else if (UseWith[WITH] == oCATENAT21) {
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a216, 0, 0, UseWith[WITH]);
			printsent = false;
		} else if (UseWith[WITH] == oALTOPARLANTE25) {
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a251, 0, 0, UseWith[WITH]);
			printsent = false;
		} else if (UseWith[WITH] == oPORTAC33) {
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a333PERDESBARRA, 0, 0, UseWith[WITH]);
			KillIcon(iSBARRA11);
			printsent = false;
		} else if (_obj[UseWith[WITH]]._flag & OBJFLAG_PERSON) {
			CharacterSay(1436);
			printsent = false;
		} else if ((UseWith[WITH] == oPORTAR12) || (UseWith[WITH] == oCASSAFORTEC15) || (UseWith[WITH] == oPORTONE18) || (UseWith[WITH] == oLUCCHETTO1B) || (UseWith[WITH] == oPORTAC21) || (UseWith[WITH] == oPANNELLOC23) || (UseWith[WITH] == oPORTA2A) || (UseWith[WITH] == oPORTA2B)) {
			printsent = false;
			CharacterSay(1435);
		}
		break;

	case iCARD14:
		if ((UseWith[WITH] == oBIGLIETTERIA16) && (_obj[oMAPPA16]._flag & OBJFLAG_EXTRA)) {
			if (_choice[49]._flag & OBJFLAG_DONE) {
				CharacterSay(1457);
				printsent = false;
			} else {
				_choice[46]._flag |= DLGCHOICE_HIDE;
				_choice[47]._flag |= DLGCHOICE_HIDE;
				_choice[48]._flag |= DLGCHOICE_HIDE;
				_choice[49]._flag &= ~DLGCHOICE_HIDE;
				PlayScript(s16CARD);
//					doMouseTalk( UseWith[WITH] );
				printsent = false;
			}
		} else if (UseWith[WITH] == oSLOT23) {
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a2311, 0, 0, UseWith[WITH]);
			printsent = false;
		} else if ((UseWith[WITH] == oSLOT12) || (UseWith[WITH] == oSLOT13) || (UseWith[WITH] == oSLOT16) || (UseWith[WITH] == oSPORTELLOA35) || (UseWith[WITH] == oSLOTA58) || (UseWith[WITH] == oSLOTB58)) {
			printsent = false;
			CharacterSay(1419);
		}
		break;

	case iMONETA13:
		if ((UseWith[WITH] == oBIGLIETTERIA16) && (_obj[oMAPPA16]._flag & OBJFLAG_EXTRA)) {
			_choice[46]._flag |= DLGCHOICE_HIDE;
			_choice[47]._flag |= DLGCHOICE_HIDE;
			_choice[48]._flag |= DLGCHOICE_HIDE;
			_choice[50]._flag &= ~DLGCHOICE_HIDE;
			PlayScript(s16MONETA);
//				doMouseTalk( UseWith[WITH] );
			printsent = false;
			KillIcon(iMONETA13);
			_obj[oFINGERPADP16]._flag |= OBJFLAG_ROOMOUT;
		} else if (UseWith[WITH] == oBIGLIETTERIA16) {
			CharacterSay(146);
			printsent = false;
		}
		break;

	case iPLASTICA:
		if (UseWith[WITH] == oTELEFAXF17) {
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a172USAPLASTICA, 0, 0, UseWith[WITH]);
			_obj[oTELEFAXF17]._examine = 1486;
			printsent = false;
		}
		break;

	case iFOTO:
		if ((UseWith[WITH] == ocBARBONE17) && (_choice[81]._flag & OBJFLAG_DONE)) {
			CharacterSay(1463);
			printsent = false;
		} else if ((UseWith[WITH] == ocBARBONE17) && (_choice[91]._flag & OBJFLAG_DONE)) {
			_obj[ocBARBONE17]._action = 1462;
			CharacterSay(_obj[ocBARBONE17]._action);
			printsent = false;
		} else if ((UseWith[WITH] == ocBARBONE17) && (!(_choice[78]._flag & OBJFLAG_DONE) || ((_choice[79]._flag & OBJFLAG_DONE) || (_choice[83]._flag & OBJFLAG_DONE) && !(_choice[92]._flag & OBJFLAG_DONE)))) {
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
		} else if (UseWith[WITH] == ocPOLIZIOTTO16) {
			_obj[ocPOLIZIOTTO16]._flag |= OBJFLAG_EXTRA;
			CharacterSay(1461);
			if ((_choice[61]._flag & OBJFLAG_DONE) && (_choice[62]._flag & OBJFLAG_DONE) && (_obj[ocPOLIZIOTTO16]._flag & OBJFLAG_EXTRA))
				_obj[ocPOLIZIOTTO16]._mode &= ~OBJMODE_OBJSTATUS;
			printsent = false;
		} else if (UseWith[WITH] == ocGUARDIANO18) {
			if (_choice[152]._flag & OBJFLAG_DONE)
				CharacterSay(1465);
			else
				CharacterSay(1464);
			printsent = false;
		} else if (UseWith[WITH] == ocNEGOZIANTE1A) {
			CharacterSay(1466);
			printsent = false;
		} else if (UseWith[WITH] == ocEVA19) {
			CharacterSay(1465);
			printsent = false;
		}
		/*			else
					{
						_obj[ocBARBONE17]._action = 211;
						CharacterSay(_obj[ocBARBONE17]._action);
						printsent=false;
					}
		*/			break;

	case iLATTINA13:
		if (UseWith[WITH] == ocBARBONE17) {
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
					_obj[ocBARBONE17]._action = 213;
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
		/*			else
					{
						_obj[ocBARBONE17]._action = 213;
						CharacterSay(_obj[ocBARBONE17]._action);
						printsent=false;
					}
		*/			break;

	case iBOTTIGLIA14:
		if (UseWith[WITH] == ocBARBONE17) {
			if ((_choice[79]._flag & OBJFLAG_DONE) || (_choice[83]._flag & OBJFLAG_DONE))
//				if( _choice[83]._flag & OBJFLAG_DONE )
			{
				_choice[80]._flag &= ~DLGCHOICE_HIDE;
				if (_choice[81]._flag & OBJFLAG_DONE) {
					_choice[81]._flag &= ~DLGCHOICE_HIDE;
					_choice[80]._flag &= ~DLGCHOICE_EXITDLG;
				}
				PlayDialog(dBARBONE171);
				updateinv = false;
				KillIcon(iBOTTIGLIA14);
				printsent = false;
			} else { //if( !(_choice[83]._flag & OBJFLAG_DONE) )
				_choice[83]._flag &= ~DLGCHOICE_HIDE;
				if (!(_choice[78]._flag & OBJFLAG_DONE)) {
					_choice[106]._flag &= ~DLGCHOICE_HIDE;
					_obj[ocBARBONE17]._action = 213;
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
		/*			else
					{
						_obj[ocBARBONE17]._action = 213;
						CharacterSay(_obj[ocBARBONE17]._action);
						printsent=false;
					}
		*/			break;

	case iBOTTIGLIA1D:
		if (UseWith[WITH] == ocNEGOZIANTE1A) {
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
		if ((UseWith[WITH] == ocGUARDIANO18) && !(_choice[155]._flag & OBJFLAG_DONE)) {
			_choice[155]._flag &= ~DLGCHOICE_HIDE;
			PlayDialog(dGUARDIANO18);
			updateinv = false;
			_obj[ocGUARDIANO18]._flag &= ~OBJFLAG_PERSON;
			_obj[oPORTAC18]._flag |= OBJFLAG_ROOMOUT;
			printsent = false;
		} else if (UseWith[WITH] == ocGUARDIANO18) {
			CharacterSay(1494);
			printsent = false;
		}
		break;

	case iTOPO1D:
		if ((UseWith[WITH] == oDONNA1D) &&
				((mx >= _obj[oDONNA1D]._lim[0]) &&
				 (my >= _obj[oDONNA1D]._lim[1] + TOP) &&
				 (mx <= _obj[oDONNA1D]._lim[2]) &&
				 (my <= _obj[oDONNA1D]._lim[3] + TOP))) {
			PlayDialog(dF1D1);
			updateinv = false;
			KillIcon(iTOPO1D);
			read3D("1d2.3d");		// dopo pattino
			_obj[oDONNA1D]._mode &= ~OBJMODE_OBJSTATUS;
			Room[_curRoom]._flag |= OBJFLAG_EXTRA;
			AnimTab[aBKG1D].flag |= SMKANIM_OFF1;
			printsent = false;
		}
		break;

	case iPISTOLA15:
		if ((UseWith[WITH] == oPORTAC21) && !(Room[r21]._flag & OBJFLAG_EXTRA)) {
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a211, 0, 0, UseWith[WITH]);
			InvObj[iPISTOLA15]._examine = 1472;
			InvObj[iPISTOLA15]._action = 1473;
			printsent = false;
		}
		break;

	case iCACCIAVITE:
		if (UseWith[WITH] == oESSE21) {
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a213, 0, 0, UseWith[WITH]);
			printsent = false;
		} else if (UseWith[WITH] == oCOPERCHIOC31) {
			NLPlaySound(wCOVER31);
			_obj[oCOPERCHIOC31]._mode &= ~OBJMODE_OBJSTATUS;
			_obj[oCOPERCHIOA31]._mode |= OBJMODE_OBJSTATUS;
			RegenRoom();
			printsent = false;
		} else if (UseWith[WITH] == oCOPERCHIOA31) {
			_obj[oCOPERCHIOA31]._mode &= ~OBJMODE_OBJSTATUS;
			_obj[oCOPERCHIOC31]._mode |= OBJMODE_OBJSTATUS;
			_obj[oPANNELLOM31]._mode &= ~OBJMODE_OBJSTATUS;
			_obj[oPANNELLOMA31]._mode |= OBJMODE_OBJSTATUS;
			if (_obj[oFILOTC31]._mode & OBJMODE_OBJSTATUS) {
				_obj[oPANNELLOM31]._anim = 0;
				_obj[oPANNELLOM31]._examine = 715;
				_obj[oPANNELLOM31]._action = 716;
				_obj[oPANNELLOM31]._flag &= ~OBJFLAG_ROOMOUT;
			}
			NLPlaySound(wCOVER31);
			RegenRoom();
			printsent = false;
			doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, r31, a3118CHIUDEPANNELLO, 3, _curObj);
		} else if (UseWith[WITH] == oPANNELLO55) {
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a5512, 0, 0, UseWith[WITH]);
			printsent = false;
		} else if (UseWith[WITH] == oPANNELLOC56) {
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a568, 0, 0, UseWith[WITH]);
			printsent = false;
		} else if ((UseWith[WITH] == oSLOT23) || (UseWith[WITH] == oPRESA35) || (UseWith[WITH] == oSERRATURA33)) {
			printsent = false;
			CharacterSay(1520);
		}
		break;

	case iESSE:
		if (UseWith[WITH] == oCATENA21) {
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a214, 0, 0, UseWith[WITH]);
			printsent = false;
		}
		break;

	case iTANICHETTA27:
		if (UseWith[WITH] == oMANIGLIONE22) {
			if (_obj[oARMADIETTORC22]._mode & OBJMODE_OBJSTATUS)
				doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a228, 0, 0, UseWith[WITH]);
			else
				doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a228A, 0, 0, UseWith[WITH]);
			KillIcon(iTANICHETTA27);
			AddIcon(iTANICHETTA22);
			if (InvObj[iLATTINA28]._flag & OBJFLAG_EXTRA) {
				KillIcon(iTANICHETTA22);
				ReplaceIcon(iLATTINA28, iLATTINE);
			}
			printsent = false;
		} else
			break;

	case iKEY22:
		if (UseWith[WITH] == oPORTA2A) {
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a2A2USEKEY, 0, 0, UseWith[WITH]);
			printsent = false;
		} else if ((UseWith[WITH] == oPANNELLOC23) || (UseWith[WITH] == oPORTAC33) || (UseWith[WITH] == oSPORTELLOC35) || (UseWith[WITH] == oCASSETTOC36) || (UseWith[WITH] == oPORTAC54) || (UseWith[WITH] == oPORTA57C55) || (UseWith[WITH] == oPORTA58C55) || (UseWith[WITH] == oPORTAS56) || (UseWith[WITH] == oPORTAS57)) {
			printsent = false;
			CharacterSay(1512);
		}
		break;

	case iLAMPADINA29:
		if ((UseWith[WITH] == oPORTALAMPADE2B) && !(_obj[UseWith[WITH]]._anim)) {
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a2B6METTELAMPADINA, 0, 0, UseWith[WITH]);
			printsent = false;
		}
		break;

	case iPIPEWRENCH:
		if (UseWith[WITH] == oPANNELLOC23) {
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a233, 0, 0, UseWith[WITH]);
			printsent = false;
		} else if ((UseWith[WITH] == oPORTAC33) || (UseWith[WITH] == oSPORTELLOC35) || (UseWith[WITH] == oCASSETTOC36) || (UseWith[WITH] == oPORTAC54) || (UseWith[WITH] == oPORTA57C55) || (UseWith[WITH] == oPORTA58C55) || (UseWith[WITH] == oPORTAS56) || (UseWith[WITH] == oPORTAS57)) {
			printsent = false;
			CharacterSay(1525);
		}
		break;

	case iCAVI:
		if ((UseWith[WITH] == oCONTATTI23) && (_obj[oLEVAS23]._mode & OBJMODE_OBJSTATUS)) {
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a236, 0, 0, UseWith[WITH]);
			printsent = false;
		} else if (UseWith[WITH] == oLEVAG23) {
			CharacterSay(2015);
			printsent = false;
		}
		break;

	case iTRONCHESE:
		if ((UseWith[WITH] == oCAVO2H) && (_obj[oCARTELLONE2H]._mode & OBJMODE_OBJSTATUS)) {
			PlayDialog(dF2H1);
			_obj[oPASSAGGIO24]._mode |= OBJMODE_OBJSTATUS;
			_obj[omPASSAGGIO24]._mode |= OBJMODE_OBJSTATUS;
			_obj[oCARTELLONE24]._mode &= ~OBJMODE_OBJSTATUS;
			_obj[oCARTELLONE2H]._mode &= ~OBJMODE_OBJSTATUS;
			_obj[oPASSERELLA24]._mode &= ~OBJMODE_OBJSTATUS;
			_obj[oMACERIE24]._mode &= ~OBJMODE_OBJSTATUS;
			//_obj[oPASSERELLA24]._flag &= ~OBJFLAG_ROOMOUT;
			//_obj[oPASSERELLA24]._anim = 0;
			printsent = false;
		} else if ((UseWith[WITH] == oTUBOT34) && (_obj[oVALVOLAC34]._mode & OBJMODE_OBJSTATUS)) {
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a341USAPINZE, 0, 0, UseWith[WITH]);
			//_obj[oVALVOLAC34]._anim = 0;
			printsent = false;
		} else if ((UseWith[WITH] == oTUBOT34) && (_obj[oVALVOLA34]._mode & OBJMODE_OBJSTATUS)) {
			CharacterSay(2007);
			printsent = false;
		} else
			printsent = true;
		break;

		break;

	case iLATTINA28:
		if ((UseWith[WITH] == oSERBATOIOA2G) && !(InvObj[iLATTINA28]._flag & OBJFLAG_EXTRA)) {
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a2G4VERSALATTINA, 0, 0, UseWith[WITH]);
			InvObj[iLATTINA28]._flag |= OBJFLAG_EXTRA;
			InvObj[iLATTINA28]._examine = 1537;
			if (IconPos(iTANICHETTA22) != MAXICON) {
				KillIcon(iTANICHETTA22);
				ReplaceIcon(iLATTINA28, iLATTINE);
			}
			if ((InvObj[iBOMBOLA]._flag & OBJFLAG_EXTRA) && (InvObj[iLATTINA28]._flag & OBJFLAG_EXTRA)) {
				_obj[oSERBATOIOA2G]._examine = 670;
				_obj[oSERBATOIOA2G]._action = 671;
			} else {
				_obj[oSERBATOIOA2G]._examine = 667;
				_obj[oSERBATOIOA2G]._action = 669;
			}
			printsent = false;
		}
		break;

	case iBOMBOLA:
		if (UseWith[WITH] == oSERBATOIOA2G) {
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a2G5METTEBOMBOLA, 0, 0, UseWith[WITH]);
			InvObj[iBOMBOLA]._flag |= OBJFLAG_EXTRA;
			if ((InvObj[iBOMBOLA]._flag & OBJFLAG_EXTRA) && (InvObj[iLATTINA28]._flag & OBJFLAG_EXTRA)) {
				_obj[oSERBATOIOA2G]._examine = 670;
				_obj[oSERBATOIOA2G]._action = 671;
			} else {
				_obj[oSERBATOIOA2G]._examine = 668;
				_obj[oSERBATOIOA2G]._action = 669;
			}
			printsent = false;
		}
		break;

	case iCANDELOTTO:
		if ((UseWith[WITH] == oSERBATOIOA2G) && (InvObj[iBOMBOLA]._flag & OBJFLAG_EXTRA) && (InvObj[iLATTINA28]._flag & OBJFLAG_EXTRA)) {
			doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, r2GV, 0, 0, UseWith[WITH]);
			KillIcon(iCANDELOTTO);
			printsent = false;
		}
		break;

	case iFUCILE:
		if (UseWith[WITH] == oDINOSAURO2E) {
			PlayDialog(dF2E2);
			_obj[oDINOSAURO2E]._mode &= ~OBJMODE_OBJSTATUS;
			_obj[oCATWALKA2E]._anim = a2E2PRIMAPALLONTANANDO;
			AnimTab[aBKG2E].flag |= SMKANIM_OFF2;
			printsent = false;
		}
		break;

	case iPINZA:
	case iSBARRA21:
		if (UseWith[WITH] == oCATENAT21) {
			if (Room[_curRoom]._flag & OBJFLAG_EXTRA) {
				if (UseWith[USED] == iPINZA)
					PlayDialog(dF212B);
				else
					PlayDialog(dF212);
				printsent = false;
			} else {
				if (UseWith[USED] == iPINZA)
					PlayDialog(dF213B);
				else
					PlayDialog(dF213);
				printsent = false;
			}
		} else if ((UseWith[WITH] == oPORTAC33) && (UseWith[USED] == iSBARRA21)) {
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a333PERDESBARRA, 0, 0, UseWith[WITH]);
			printsent = false;
		} else if ((UseWith[WITH] == oSERPENTEU52) && (UseWith[USED] == iPINZA)) {
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a527, 0, 0, UseWith[WITH]);
			_obj[oSCAVO51]._anim = a516;
			printsent = false;
		} else if ((UseWith[WITH] == oSERPENTEA52) && (UseWith[USED] == iPINZA)) {
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a528, 0, 0, UseWith[WITH]);
			printsent = false;
		} else if ((UseWith[WITH] == oSERPENTEB52) && (UseWith[USED] == iPINZA) && (IconPos(iSERPENTEA) == MAXICON)) {
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a523, 0, 0, UseWith[WITH]);
			printsent = false;
		}
		break;

	case iLAMPADINA2B:
		if (UseWith[WITH] == oPORTALAMPADE29) {
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a293AVVITALAMPADINA, 0, 0, UseWith[WITH]);
			printsent = false;
		}
		break;

	case iLATTINA27:
		if ((UseWith[WITH] == oBRACIERES28) && (_obj[oBRACIERES28]._flag & OBJFLAG_EXTRA)) {
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a286, 0, 0, UseWith[WITH]);
			_obj[oBRACIERES28]._examine = 456;
			printsent = false;
		} else
			printsent = true;
		break;

	case iTELECOMANDO2G:
		if (UseWith[WITH] == oTASTO2F) {
			if (_obj[oASCENSORE2F]._mode & OBJMODE_OBJSTATUS) {
				doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a2F10PANNELLOSICHIUDE, 0, 0, UseWith[WITH]);
				_obj[oBIDONE2F]._anim = a2F5CFRUGABIDONE;
			} else {
				if (!(InvObj[iTELECOMANDO2G]._flag & OBJFLAG_EXTRA))
					doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a2F9PPANNELLOSIAPRE, 0, 0, UseWith[WITH]);
				else
					doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a2F9PANNELLOSIAPRE, 0, 0, UseWith[WITH]);
				InvObj[iTELECOMANDO2G]._flag |= OBJFLAG_EXTRA;
				_obj[oBIDONE2F]._anim = a2F5FRUGABIDONE;
			}
			printsent = false;
		}
		break;

	case iSAMROTTO:
		if (UseWith[WITH] == oSERRATURA33) {
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a338POSASAM, 0, 0, UseWith[WITH]);
			printsent = false;
		}
		break;

	case iTORCIA32:
		if ((UseWith[WITH] == oSENSOREV32) && (_obj[oFILOTC31]._mode & OBJMODE_OBJSTATUS)) {
			_obj[oPANNELLOMA31]._mode |= OBJMODE_OBJSTATUS;
			_obj[oPANNELLOM31]._mode &= ~OBJMODE_OBJSTATUS;
			_obj[oPANNELLOMA31]._examine = 717;
			_obj[oPANNELLOMA31]._action = 718;
			_obj[oCORPO31]._mode |= OBJMODE_OBJSTATUS;
			_obj[od31ALLA35]._mode |= OBJMODE_OBJSTATUS;
			_obj[omd31ALLA35]._mode |= OBJMODE_OBJSTATUS;
			_obj[oMONTACARICHI31]._mode |= OBJMODE_OBJSTATUS;
			_obj[oPANNELLO31]._mode &= ~OBJMODE_OBJSTATUS;
			_obj[oPANNELLON31]._mode &= ~OBJMODE_OBJSTATUS;
			Room[r32]._flag |= OBJFLAG_EXTRA;
			doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, r31, 0, 11, UseWith[WITH]);

			printsent = false;
		}
		break;

	case iPROIETTORE31:
		if (UseWith[WITH] == oPRESA35) {
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a351PROVAPROIETTOREPRESA, 0, 0, UseWith[WITH]);
			printsent = false;
		} else if (UseWith[WITH] == oTRIPLA35) {
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a355ATTPROIETTORETRIPLAEPRESA, 0, 0, UseWith[WITH]);
			printsent = false;
		} else if ((UseWith[WITH] == oRIBELLEA35)) {
			printsent = false;
			CharacterSay(1578);
		}
		break;

	case iPROIETTORE35:
		if (UseWith[WITH] == oPRESA35) {
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a352ATTPROIETTOREETRIPLAPRESA, 0, 0, UseWith[WITH]);
			printsent = false;
		} else if ((UseWith[WITH] == oRIBELLEA35)) {
			printsent = false;
			CharacterSay(1590);
		}
		break;

	case iTRIPLA:
		if (UseWith[WITH] == oPRESA35) {
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a353ATTACCATRIPLAPRESA, 0, 0, UseWith[WITH]);
			printsent = false;
		}
		break;

	case iLASER35:
		if (UseWith[WITH] == oSPORTELLOC35) {
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a3511APRESPORTELLO, 0, 0, UseWith[WITH]);
			printsent = false;
		} else if (UseWith[WITH] == oSERPENTEU52) {
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a522, 0, 0, UseWith[WITH]);
			_obj[oSCAVO51]._anim = a516;
			printsent = false;
		} else if (UseWith[WITH] == oLUCCHETTO53) {
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a532, 0, 0, UseWith[WITH]);
			printsent = false;
		} else if ((UseWith[WITH] == oPORTAMC36) || (UseWith[WITH] == oPORTALC36) || (UseWith[WITH] == oSCANNERMA36) || (UseWith[WITH] == oSCANNERLA36) || (UseWith[WITH] == oCASSETTOC36) || (UseWith[WITH] == oRETE52) || (UseWith[WITH] == oTELECAMERA52) || (UseWith[WITH] == oSERPENTET52) || (UseWith[WITH] == oLAGO53)) {
			printsent = false;
			CharacterSay(1597);
		}
		break;

	case iKEY35:
		if (UseWith[WITH] == oCASSETTOC36) {
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a364APRECASSETTO, 0, 0, UseWith[WITH]);
			printsent = false;
		} else if ((UseWith[WITH] == oSPORTELLOC35) || (UseWith[WITH] == oPORTAC54) || (UseWith[WITH] == oPORTA57C55) || (UseWith[WITH] == oPORTA58C55) || (UseWith[WITH] == oPORTAS56) || (UseWith[WITH] == oPORTAS57)) {
			printsent = false;
			CharacterSay(1594);
		}
		break;

	case iSTETOSCOPIO:
		if (UseWith[WITH] == oPORTALC36) {
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a368USASTETOSCOPIO, 0, 0, UseWith[WITH]);
			printsent = false;
		}
		break;

	case iCARD36:
		if ((UseWith[WITH] == oSPORTELLOA35) && !(_obj[oSPORTELLOA35]._flag & OBJFLAG_EXTRA)) {
			doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, r35P, 0, 10, UseWith[WITH]);
			KillIcon(iCARD36);
			printsent = false;
		} else if ((UseWith[WITH] == oSPORTELLOA35) && (_obj[oSPORTELLOA35]._flag & OBJFLAG_EXTRA)) {
			CharacterSay(1844);
			printsent = false;
		} else if ((UseWith[WITH] == oSLOTA58) || (UseWith[WITH] == oSLOTB58)) {
			printsent = false;
			CharacterSay(1602);
		}
		break;

	case iMONETA4L:
		if ((UseWith[WITH] == oFESSURA41) &&
				((_obj[oFUCILE42]._anim != 0) && (_obj[oFUCILE42]._anim != a428) && (_obj[oFUCILE42]._anim != a429))) {
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a412, 0, 0, UseWith[WITH]);
			if (_obj[oZAMPA41]._mode & OBJMODE_OBJSTATUS)
				_obj[oSLOT41]._anim = a417;
			else if (Try41 <= 2)
				_obj[oSLOT41]._anim = a414;
			else
				CharacterSay(2015);
			Try41 ++;
			printsent = false;
		} else if ((UseWith[WITH] == oFESSURA41) &&
				   ((_obj[oFUCILE42]._anim == 0) || (_obj[oFUCILE42]._anim == a428) || (_obj[oFUCILE42]._anim == a429))) {
			CharacterSay(2010);
			printsent = false;
		} else if (UseWith[WITH] == oFESSURA42) {
			CharacterSay(924);
			printsent = false;
		} else if (UseWith[WITH] == oCAMPANA4U) {
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a4U3, 0, 0, UseWith[WITH]);
			_obj[oCAMPANA4U]._flag |= OBJFLAG_EXTRA;
			if ((_obj[oCAMPANA4U]._flag & OBJFLAG_EXTRA) && (InvObj[iBIGLIAA]._flag & OBJFLAG_EXTRA)) {
				_obj[oCAMPANA4U]._examine = 1202;
				_obj[oCAMPANA4U]._action = 1203;
			} else
				_obj[oCAMPANA4U]._examine = 1200;
			printsent = false;
		}
		break;

	case iMARTELLO:
		if ((UseWith[WITH] == oRAGNO41) && !(_obj[oRAGNO41]._flag & OBJFLAG_EXTRA)) {
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a416, 0, 0, UseWith[WITH]);
			_obj[oRAGNO41]._flag |= OBJFLAG_EXTRA;
			_obj[oRAGNO41]._anim = 0;
			if (_obj[oSLOT41]._anim == a414)
				_obj[oSLOT41]._anim = a417;
			printsent = false;
		} else if ((UseWith[WITH] == oSLOT41) || (UseWith[WITH] == oVETRINETTA42) || (UseWith[WITH] == oTAMBURO43) || (UseWith[WITH] == oSFIATO45) || (UseWith[WITH] == oPORTAC4A) || (UseWith[WITH] == oPORTAC4B) || (UseWith[WITH] == oSERRATURA4B) || (UseWith[WITH] == oLICANTROPO4P)) {
			printsent = false;
			CharacterSay(1619);
		}
		break;

	case iMONETE:
		if ((UseWith[WITH] == oFESSURA42) && (_obj[oFUCILE42]._anim == a427)) {
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a421, 0, 0, UseWith[WITH]);
			if (_obj[oPOLTIGLIA42]._mode & OBJMODE_OBJSTATUS)
				_obj[oFUCILE42]._anim = a429;
			else
				_obj[oFUCILE42]._anim = a428;
			printsent = false;
		}
		break;

	case iPOLTIGLIA:
		if (UseWith[WITH] == oGUIDE42) {
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a423, 0, 0, UseWith[WITH]);
			if (_obj[oFUCILE42]._anim != a427)
				_obj[oFUCILE42]._anim = a429;
			printsent = false;
		}
		break;

	case iMAZZA:
		if ((UseWith[WITH] == oTAMBURO43) && !(_obj[oTAMBURO43]._flag & OBJFLAG_EXTRA)) {
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a435, 0, 0, UseWith[WITH]);
			_obj[oTAMBURO43]._flag |= OBJFLAG_EXTRA;
			printsent = false;
		} else if ((UseWith[WITH] == oPORTAC4B) || (UseWith[WITH] == oSERRATURA4B) || (UseWith[WITH] == oLICANTROPO4P)) {
			printsent = false;
			CharacterSay(1679);
		}
		break;

	case iPUPAZZO:
		if (UseWith[WITH] == oCASSETTOAA44) {
			ReplaceIcon(iPUPAZZO, iTELECOMANDO44);
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a442, 0, 0, UseWith[WITH]);
			printsent = false;
		}
		break;

	case iSTRACCIO:
		if (UseWith[WITH] == oMANOPOLAR45) {
			doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, r45S, 0, 2, UseWith[WITH]);
			printsent = false;
		} else if ((UseWith[WITH] == oCAMPANA4U) && (InvObj[iBIGLIAA]._flag & OBJFLAG_EXTRA) && (_obj[oCAMPANA4U]._flag & OBJFLAG_EXTRA)) {
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a4U5, 0, 0, UseWith[WITH]);
			_obj[oCAMPANA4U]._examine = 1204;
			_obj[oCAMPANA4U]._action = 1205;
			printsent = false;
		} else if (UseWith[WITH] == oCAMPANA4U) {
			CharacterSay(1713);
			printsent = false;
		} else if ((UseWith[WITH] == oRAGNO46) || (UseWith[WITH] == oLICANTROPO4P)) {
			printsent = false;
			CharacterSay(1711);
		}
		break;

	case iTESCHIO:
		if (UseWith[WITH] == oPIASTRELLA48) {
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a4810, 0, 0, UseWith[WITH]);
			printsent = false;
		}
		break;

	case iTORCIA47:
		if (UseWith[WITH] == oTESCHIO48) {
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a4811, 0, 0, UseWith[WITH]);
			printsent = false;
		} else if ((UseWith[WITH] == oCALDAIAS45) || (UseWith[WITH] == oRAGNO46)) {
			printsent = false;
			CharacterSay(1640);
		}
		break;

	case iFIAMMIFERO:
		if (UseWith[WITH] == oTORCIAS48) {
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a4812, 0, 0, UseWith[WITH]);
			_obj[oTORCIAS48]._lim[0] = 0;
			_obj[oTORCIAS48]._lim[1] = 0;
			_obj[oTORCIAS48]._lim[2] = 0;
			_obj[oTORCIAS48]._lim[3] = 0;
			printsent = false;
		}
		break;

	case iASTA:
		if (UseWith[WITH] == oMERIDIANA49) {
			KillIcon(iASTA);
			StartCharacterAction(a491, r49M, 1, 0);
			printsent = false;
		}
		break;

	case iPISTOLA4PD:
		if ((UseWith[WITH] == oLICANTROPO4P) && (InvObj[iPISTOLA4PD]._flag & OBJFLAG_EXTRA)) {
			ReplaceIcon(iPISTOLA4PD, iPISTOLA4B);
			_obj[oLICANTROPO4P]._mode &= ~OBJMODE_OBJSTATUS;
			_obj[oLICANTROPOM4P]._mode |= OBJMODE_OBJSTATUS;
			_obj[oSANGUE4P]._mode |= OBJMODE_OBJSTATUS;
			Room[r4P]._flag |= OBJFLAG_EXTRA;
			read3D("4p2.3d");		// dopo licantropo
			AnimTab[aBKG4P].flag |= SMKANIM_OFF1;
			PlayDialog(dF4P2);
			printsent = false;
		} else if (UseWith[WITH] == oLICANTROPO4P) {
			ReplaceIcon(iPISTOLA4PD, iPISTOLA4B);
			PlayDialog(dF4P1);
			printsent = false;
		} else if ((UseWith[WITH] == oRAGNO46) || (UseWith[WITH] == oPORTAC4B) || (UseWith[WITH] == oSERRATURA4B)) {
			printsent = false;
			CharacterSay(1706);
		}
		break;

	case iBARATTOLO:
		if ((UseWith[WITH] == oSANGUE4P) || (UseWith[WITH] == oLICANTROPOM4P)) {
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a4P7, 0, 0, UseWith[WITH]);
			printsent = false;
		}
		break;

	case iCAMPANA:
		if (UseWith[WITH] == oPOZZA4U) {
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a4U2, 0, 0, UseWith[WITH]);
			printsent = false;
		}
		break;

	case iBIGLIAA:
	case iBIGLIAB:
		if ((UseWith[WITH] == oCAMPANA4U) && (InvObj[iBIGLIAA]._flag & OBJFLAG_EXTRA)) {
			CharacterSay(1684);
			printsent = false;
		} else if (UseWith[WITH] == oCAMPANA4U) {
			KillIcon(UseWith[USED]);
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a4U4, 0, 0, UseWith[WITH]);
			InvObj[iBIGLIAA]._flag |= OBJFLAG_EXTRA;
			if ((_obj[oCAMPANA4U]._flag & OBJFLAG_EXTRA) && (InvObj[iBIGLIAA]._flag & OBJFLAG_EXTRA)) {
				_obj[oCAMPANA4U]._examine = 1202;
				_obj[oCAMPANA4U]._action = 1203;
			} else
				_obj[oCAMPANA4U]._examine = 1201;
			printsent = false;
		}
		break;

	case iPAPAVERINA:
		if ((UseWith[WITH] == oCIOCCOLATINI4A) && ((_curAnimFrame[0] < 370) || (_curAnimFrame[0] > 480))) {
			PlayScript(s4AHELLEN);
			_obj[oPULSANTE4A]._anim = a4A3;
			printsent = false;
		} else if (UseWith[WITH] == oCIOCCOLATINI4A) {
			_obj[oPULSANTE4A]._anim = a4A3;
			printsent = false;
			_obj[oCIOCCOLATINI4A]._flag |= OBJFLAG_EXTRA;
		}
		break;

	case iSANGUE:
		if (UseWith[WITH] == oSERRATURA4B) {
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a4B4, 0, 0, UseWith[WITH]);
			_obj[oPORTAC4B]._anim = a4B5;
			printsent = false;
		}
		break;

	case iDIAPA4B:
		if ((UseWith[WITH] == oPROIETTORE4B) && (_obj[oPROIETTORE4B]._anim < a4B9A)) {
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a4B6A, 0, 0, UseWith[WITH]);
			_obj[oPROIETTORE4B]._anim = a4B9A;
			printsent = false;
		}
		break;

	case iDIAPB4B:
		if ((UseWith[WITH] == oPROIETTORE4B) && (_obj[oPROIETTORE4B]._anim < a4B9A)) {
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a4B6B, 0, 0, UseWith[WITH]);
			_obj[oPROIETTORE4B]._anim = a4B9B;
			printsent = false;
		}
		break;

	case iDIAP4C:
		if ((UseWith[WITH] == oPROIETTORE4B) && (_obj[oPROIETTORE4B]._anim < a4B9A)) {
			//
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a4B6C, 0, 0, UseWith[WITH]);
			_obj[oPROIETTORE4B]._anim = a4B9C;
			printsent = false;
		}
		break;

	case iUOVO:
		if ((UseWith[WITH] == oRETE52) || (UseWith[WITH] == oSERPENTET52)) {
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a521, 0, 0, UseWith[WITH]);
			printsent = false;
		}
		break;

	case iSERPENTEA:
		if (UseWith[WITH] == oLAGO53) {
			printsent = false;
			if (!(_obj[oLUCCHETTO53]._mode & OBJMODE_OBJSTATUS)) {
				StartCharacterAction(a533, r54, 11, 0);
				KillIcon(UseWith[USED]);
			} else if (UseWith[USED] != iSERPENTEB) {
				doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a534, 0, 0, UseWith[WITH]);
				KillIcon(UseWith[USED]);
				_obj[oLAGO53]._examine = 1237;
			} else
				CharacterSay(1740);
		}
		break;

	case iSERPENTEB:
		if (UseWith[WITH] == oLAGO53) {
			printsent = false;
			if (!(_obj[oLUCCHETTO53]._mode & OBJMODE_OBJSTATUS)) {
				StartCharacterAction(a533C, r54, 11, 0);
				KillIcon(UseWith[USED]);
			} else if (UseWith[USED] != iSERPENTEB) {
				doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a534, 0, 0, UseWith[WITH]);
				KillIcon(UseWith[USED]);
				_obj[oLAGO53]._examine = 1237;
			} else
				CharacterSay(1740);
		}
		break;

	case iSAPONE:
		if (UseWith[WITH] == oSECCHIOA54) {
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a543, 0, 0, UseWith[WITH]);
			printsent = false;
		}
		break;

	case iLATTINE:
		if (UseWith[WITH] == oLAVATRICEF54) {
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a546, 0, 0, UseWith[WITH]);
			_obj[oLAVATRICEL54]._anim = a547;
			printsent = false;
		}
		break;

	case iCHIAVI:
		if (UseWith[WITH] == oPORTAS56) {
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a563, 0, 0, UseWith[WITH]);
			printsent = false;
		} else if (UseWith[WITH] == oPORTA57C55) {
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a551, r57, 17, UseWith[WITH]);
			printsent = false;
		} else if ((UseWith[WITH] == oPORTA58C55) && (!(_choice[871]._flag & OBJFLAG_DONE) || (_choice[901]._flag & OBJFLAG_DONE))) {
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a552, 0, 0, UseWith[WITH]);
			printsent = false;
		} else if ((UseWith[WITH] == oPORTA58C55) && (_choice[871]._flag & OBJFLAG_DONE)) {
			CharacterSay(1287);
			printsent = false;
		}
		break;

	case iMDVD:
		if ((UseWith[WITH] == oTASTIERA56) && (_choice[260]._flag & OBJFLAG_DONE) && !(_choice[262]._flag & OBJFLAG_DONE) && (InvObj[iMDVD]._examine != 1752)) {
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a566, 0, 0, UseWith[WITH]);
			_choice[262]._flag &= ~DLGCHOICE_HIDE;
			InvObj[iMDVD]._examine = 1752;
			printsent = false;
		} else if ((UseWith[WITH] == oTASTIERA56) && (InvObj[iMDVD]._examine == 1752)) {
			CharacterSay(1753);
			printsent = false;
		} else
			printsent = true;
		break;

	case iTESTER:
		if ((UseWith[WITH] == oPANNELLOA) && (_choice[856]._flag & OBJFLAG_DONE)) {
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a569, 0, 0, UseWith[WITH]);
			_obj[oPANNELLOA]._flag |= OBJFLAG_EXTRA;
			printsent = false;
		}
		break;

	case iCUTTER:
		if ((UseWith[WITH] == oPANNELLOA) && (_obj[oPANNELLOA]._flag & OBJFLAG_EXTRA)) {
			PlayDialog(dF562);
			_obj[oPANNELLOA]._mode &= ~OBJMODE_OBJSTATUS;
			_obj[oCAVOTAGLIATO56]._mode |= OBJMODE_OBJSTATUS;
			_obj[oPORTA58C55]._mode |= OBJMODE_OBJSTATUS;
			setPosition(6);
			KillIcon(iCUTTER);
			printsent = false;
		} else if (UseWith[WITH] == oPANNELLOA) {
			CharacterSay(2012);
			printsent = false;
		}
		break;

	case iGUANTI59:
		if (UseWith[WITH] == oBOMBOLA57) {
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a575, 0, 0, UseWith[WITH]);
			printsent = false;
		}
		break;

	case iARMAEVA:
		if ((UseWith[WITH] == oFINESTRAA5A) && (_choice[871]._flag & OBJFLAG_DONE) && !(_choice[286]._flag & OBJFLAG_DONE)) {
			KillIcon(iARMAEVA);
			PlayDialog(dC5A1);
			_obj[oFINESTRAA58]._anim = a587;
			printsent = false;
			Room[r5A]._flag |= OBJFLAG_EXTRA;
		} else if ((UseWith[WITH] == oFINESTRAA5A) && (_choice[871]._flag & OBJFLAG_DONE)) {
			KillIcon(iARMAEVA);
			PlayDialog(dF5A1);
			_obj[oFINESTRAA58]._anim = a587;
			printsent = false;
		}
		break;

	default:
		updateinv = false;
		break;
	}

	if (printsent) CharacterSay(InvObj[UseWith[USED]]._action);
	if (updateinv) RegenInv(TheIconBase, INVENTORY_SHOW);
}

/*-------------------------------------------------------------------------*/
/*                          USE WITH / SCR - SCR         				   */
/*-------------------------------------------------------------------------*/
void doScrScrUseWith() {
	LLBOOL printsent = true;

	if ((!UseWith[USED]) || (!UseWith[WITH]))
		warning("doScrScrUseWith");

//	StopSmackAnim(InvObj[UseWith[USED]]._anim);
	if (_homoInMovement)
		return;

	switch (UseWith[USED]) {
	case oRAMPINO21:
		if (UseWith[WITH] == oTUBO21) {
			PlayDialog(dF211);
			SetRoom(r21, true);
			AnimTab[aBKG21].flag &= ~SMKANIM_OFF1;
			_obj[oRAMPINO21]._mode &= ~OBJMODE_OBJSTATUS;
			_obj[oTUBO21]._mode &= ~OBJMODE_OBJSTATUS;
			_obj[oCATENAT21]._mode |= OBJMODE_OBJSTATUS;
			printsent = false;
		}
		break;

	case oCAVIE23:
		if (UseWith[WITH] == oCAMPO23) {
			doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, r23B, 0, 0, UseWith[WITH]);
			printsent = false;
		} else {
			StartCharacterAction(hBOH, 0, 0, 0);
			printsent = false;
		}
		break;

	case oTUBOP33:
		if (UseWith[WITH] == oTUBOF33) {
			doEvent(MC_HOMO, ME_HOMOACTION, MP_DEFAULT, a337PRENDETUBO, 0, 0, UseWith[WITH]);
			_obj[oTUBOF34]._examine = 1832;
			_obj[oTUBOFT34]._examine = 773;
			//_obj[oVALVOLAC34]._anim = a344APREVALVOLA;
			printsent = false;
		}
		break;

	case oTUBOT33:
		if (((UseWith[WITH] == oSAMA33) || (UseWith[WITH] == oSERRATURA33 && _obj[oSAMA33]._mode & OBJMODE_OBJSTATUS)) && (_obj[oVALVOLA34]._mode & OBJMODE_OBJSTATUS)) {
			PlayDialog(dF331);
			setPosition(10);
			_obj[oBOTOLAC33]._mode &= ~OBJMODE_OBJSTATUS;
			_obj[oSERRATURA33]._mode &= ~OBJMODE_OBJSTATUS;
			_obj[oBOTOLAA33]._mode |= OBJMODE_OBJSTATUS;
			_obj[oBRUCIATURA33]._mode |= OBJMODE_OBJSTATUS;
			_obj[oSERRATURAF33]._mode |= OBJMODE_OBJSTATUS;
			_obj[oSAMD33]._mode |= OBJMODE_OBJSTATUS;
			_obj[oTUBOS33]._mode |= OBJMODE_OBJSTATUS;

			_obj[oBOTOLAC32]._mode &= ~OBJMODE_OBJSTATUS;
			_obj[oBOTOLAB32]._mode |= OBJMODE_OBJSTATUS;
			_obj[oBOTOLAA32]._mode |= OBJMODE_OBJSTATUS;
			_obj[oPULSANTI32]._anim = 0;

			_obj[oSIGARO31]._mode |= OBJMODE_OBJSTATUS;
			_obj[oPANNELLOM31]._anim = a314APREPANNELLO;
			_obj[oPANNELLOM31]._examine = 713;
			_obj[oPANNELLOM31]._action = 714;
			_obj[oPANNELLOM31]._flag |= OBJFLAG_ROOMOUT;
			_obj[oPANNELLON31]._mode |= OBJMODE_OBJSTATUS;

			_obj[oMANOMETROC34]._mode |= OBJMODE_OBJSTATUS;
			_obj[oMANOMETRO34]._mode &= ~OBJMODE_OBJSTATUS;
			_obj[oMANOMETROC34]._examine = 804;
			_obj[oVALVOLAC34]._anim = 0;
			_obj[oVALVOLA34]._anim = 0;
			_obj[oVALVOLAC34]._action = 1831;
			_obj[oVALVOLA34]._action = 1831;

			_obj[oTUBOF34]._examine = 1832;
			_obj[oTUBOFT34]._examine = 784;

			printsent = false;
		}
		break;

	case oFILOS31:
		if (UseWith[WITH] == oCONTATTOP31) {
			_obj[oFILOS31]._mode &= ~OBJMODE_OBJSTATUS;
			_obj[oCONTATTOP31]._mode &= ~OBJMODE_OBJSTATUS;
			_obj[oFILOTC31]._mode |= OBJMODE_OBJSTATUS;
			CharacterSay(746);
			RegenRoom();

			printsent = false;
		}
		break;

	}

	if (printsent) CharacterSay(_obj[UseWith[USED]]._action);
}

/*-------------------------------------------------------------------------*/
/*                               EXAMINE INV          					   */
/*-------------------------------------------------------------------------*/
void doInvExamine() {
	if (!CurInventory)
		warning("doInvExamine");

	if (InvObj[CurInventory]._examine) CharacterSay(InvObj[CurInventory]._examine);
}

/*-------------------------------------------------------------------------*/
/*                               OPERATE INV          					   */
/*-------------------------------------------------------------------------*/
void doInvOperate() {
	LLBOOL printsent = true;
	int a;

	if (!CurInventory)
		warning("doInvOperate");

	switch (CurInventory) {
	case iSAM:
		if ((_choice[166]._flag & OBJFLAG_DONE) && ((_curRoom == r17) || (_curRoom == r1B)
				|| (_curRoom == r1C) || (_curRoom == r1D))) {
			if (_obj[oNUMERO17]._mode & OBJMODE_OBJSTATUS) {
				if (!(_choice[196]._flag & OBJFLAG_DONE)) {
					if (_curRoom == r17) {
						if (_choice[198]._flag & OBJFLAG_DONE) {
							InvObj[iSAM]._action = 1787;
							printsent = true;
						} else {
							_choice[197]._flag |= DLGCHOICE_HIDE;
							_choice[198]._flag &= ~DLGCHOICE_HIDE;
							PlayDialog(dSAM17);
							printsent = false;
							_obj[oFAX17]._mode |= OBJMODE_OBJSTATUS;
						}
					} else {
						if (_choice[199]._flag & OBJFLAG_DONE) {
							InvObj[iSAM]._action = 1787;
							printsent = true;
						} else {
							_choice[197]._flag |= DLGCHOICE_HIDE;
							_choice[199]._flag &= ~DLGCHOICE_HIDE;
							PlayDialog(dSAM17);
							printsent = false;
							_obj[oFAX17]._mode |= OBJMODE_OBJSTATUS;
						}
					}
				} else {
					if ((_choice[198]._flag & OBJFLAG_DONE) || (_choice[199]._flag & OBJFLAG_DONE)) {
						InvObj[iSAM]._action = 1787;
						printsent = true;
					} else if (_curRoom == r17) {
						if (_choice[201]._flag & OBJFLAG_DONE) {
							InvObj[iSAM]._action = 1787;
							printsent = true;
						} else {
							_choice[201]._flag &= ~DLGCHOICE_HIDE;
							PlayDialog(dSAM17);
							printsent = false;
							_obj[oFAX17]._mode |= OBJMODE_OBJSTATUS;
						}
					} else {
						if (_choice[200]._flag & OBJFLAG_DONE) {
							InvObj[iSAM]._action = 1787;
							printsent = true;
						} else {
							_choice[200]._flag &= ~DLGCHOICE_HIDE;
							PlayDialog(dSAM17);
							printsent = false;
							_obj[oFAX17]._mode |= OBJMODE_OBJSTATUS;
						}
					}
				}
			} else if (_choice[197]._flag & OBJFLAG_DONE) {
				InvObj[iSAM]._action = 1786;
				printsent = true;
			} else {
				PlayDialog(dSAM17);
				printsent = false;
			}
		}
		break;

	case iMAPPA17:
		if (_curRoom == r23A) {
			CharacterSay(361);
			printsent = false;
		} else
			printsent = true;
		break;

	case iLIBRO:
		doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, r2BL, 0, 0, UseWith[WITH]);
		_obj[oEXIT2BL]._goRoom = _curRoom;
		actorStop();
		nextStep();
		printsent = false;
		break;

	case iFOGLIO36:
		doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, r36F, 0, 0, UseWith[WITH]);
		_obj[oEXIT36F]._goRoom = _curRoom;
		actorStop();
		nextStep();
		printsent = false;
		break;

	case iDISLOCATORE:
		for (a = oROOM41; a <= oROOM45B; a++)
			_obj[a]._mode &= ~OBJMODE_OBJSTATUS;
		doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, r41D, 0, 0, UseWith[WITH]);
		_obj[oEXIT41D]._goRoom = _curRoom;
		InvObj[iDISLOCATORE]._flag &= ~OBJFLAG_EXTRA;
		actorStop();
		nextStep();
		printsent = false;
		break;

	case iCODICE:
		_obj[oEXIT58M]._goRoom = _curRoom;
		doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, r58M, 0, 0, UseWith[WITH]);
		actorStop();
		nextStep();
		printsent = false;
		break;

	case iTELECOMANDO44:
		if ((_actor._px < 5057.6) && (_obj[oPUPAZZO44]._mode & OBJMODE_OBJSTATUS) && (_curRoom == r43)) {
			doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, r46, 0, 7, UseWith[WITH]);
			printsent = false;
		}
		break;

	case iMAPPA50:
		if ((_curRoom >= r51) && (_curRoom <= r5A))
			InvObj[iMAPPA50]._action = 1725 + (_curRoom - r51);
		printsent = true;
		break;

	}

	if ((InvObj[CurInventory]._action) && (printsent)) CharacterSay(InvObj[CurInventory]._action);
}

/*-------------------------------------------------------------------------*/
/*                                 doDoing           					   */
/*-------------------------------------------------------------------------*/
void doDoing() {
	uint16 TheObj, TheAnim;

	warning("doDoing");

	if (TheMessage->event <= ME_WAITOPENCLOSE) {
		TheObj  = TheMessage->wparam1;
		TheAnim  = TheMessage->wparam2;
//		TheAliasObj = _obj[TheObj].alias;
	}

	switch (TheMessage->event) {

	case ME_INITOPENCLOSE:
		if (_actor._curAction == hSTAND) REEVENT;
		else {
			uint16 cont = false;
			switch (TheObj) {
//     case oFINESTRAAP18:
//      if(_actor._curFrame == 8) cont = true;
//      break;

			default:
				if (_actor._curFrame == 4) cont = true;
				break;
			}
			if (cont) doEvent(TheMessage->cls, ME_OPENCLOSE, TheMessage->priority, TheMessage->wparam1, TheMessage->wparam2, TheMessage->bparam, TheMessage->lparam);
			else REEVENT;
		}
		break;

	case ME_OPENCLOSE:
		_obj[TheObj]._mode &= ~OBJMODE_OBJSTATUS;
		RegenRoom();
		if (TheAnim) {
//    if(TheObj == oFRIGO87V) doEvent(MC_ANIMATION,ME_ADDANIM,MP_SYSTEM,TheAnim,0,0,true);
//    else
			doEvent(MC_ANIMATION, ME_ADDANIM, MP_SYSTEM, TheAnim, 0, 0, 0);
		}
		TheMessage->event = ME_WAITOPENCLOSE;
// senza break!
	case ME_WAITOPENCLOSE:
		if (TheAnim) switch (TheMessage->bparam) {
				/*				case 0:
									if(AnimObj[TheAnim]._flag & ONOFF)
									{
										TheMessage->bparam = 1;
										ObjOpen(TheObj);
									}
									REEVENT;
									return;
								case 1:
									if(!(AnimObj[TheAnim]._flag & ONOFF)) TheMessage->bparam = 2;
									REEVENT;
									return;
								case 2:
									break;
				*/
			}
//			_obj[TheAliasObj]._mode |= OBJMODE_OBJSTATUS;
		RegenRoom();
		if (_actor._curAction == hSTAND) SemMouseEnabled = true;
		break;
	}
}

/*-------------------------------------------------------------------------*/
/*                                doScript           					   */
/*-------------------------------------------------------------------------*/
void doScript() {
	static uint32 me_pausestarttime = 0;
	uint8 campo  = TheMessage->bparam;
	uint16 indice  = TheMessage->wparam1;
	uint16 indice2  = TheMessage->wparam2;
	uint32 valore  = TheMessage->lparam;

	switch (TheMessage->event) {

	case ME_PAUSE:
		if (!me_pausestarttime) {
			me_pausestarttime = TheTime;
			doEvent(TheMessage->cls, TheMessage->event, TheMessage->priority, TheMessage->wparam1, TheMessage->wparam2, TheMessage->bparam, TheMessage->lparam);
		} else {
			if (TheTime >= (me_pausestarttime + TheMessage->wparam1)) me_pausestarttime = 0;
			else doEvent(TheMessage->cls, TheMessage->event, TheMessage->priority, TheMessage->wparam1, TheMessage->wparam2, TheMessage->bparam, TheMessage->lparam);
		}
		break;

	case ME_SETROOM:
		switch (campo) {
//				case C_RANIM: Room[indice]._anim[indice2] = (uint8)valore; break;
		}
		break;

	case ME_SETOBJ:
		switch (campo) {
		case C_ONAME:
			_obj[indice]._name = (uint16)valore;
			break;
		case C_OEXAMINE:
			_obj[indice]._examine = (uint16)valore;
			break;
		case C_OACTION:
			_obj[indice]._action = (uint16)valore;
			break;
		case C_OGOROOM:
			_obj[indice]._goRoom = (uint8)valore;
			break;
		case C_OMODE:
			if (valore)  _obj[indice]._mode |= (uint8)indice2;
			else  _obj[indice]._mode &= ~(uint8)indice2;
			break;
		case C_OFLAG:
			if (valore)  _obj[indice]._flag |= (uint8)indice2;
			else  _obj[indice]._flag &= ~(uint8)indice2;
			break;

		}
		break;

	case ME_SETINVOBJ:
		switch (campo) {
		case C_INAME:
			InvObj[indice]._name = (uint16)valore;
			break;
		case C_IEXAMINE:
			InvObj[indice]._examine = (uint16)valore;
			break;
		case C_IACTION:
			InvObj[indice]._action = (uint16)valore;
			break;
		case C_IFLAG:
			if (valore)  InvObj[indice]._flag |= (uint8)indice2;
			else  InvObj[indice]._flag &= ~(uint8)indice2;
			break;

		}
		break;

	case ME_SETANIM:
		switch (campo) {
		case C_AFLAG:
//					if(valore)  AnimObj[indice]._flag |= (uint8)indice2;
//					else  AnimObj[indice]._flag &= ~(uint8)indice2;
			break;
		case C_ADELAY:
//					AnimObj[indice].delay = valore;
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

//		case ME_SOMEONETALK:
//			SomeOneTalk(dBattuta[TheMessage->lparam]+1,TheMessage->wparam1,TheMessage->wparam2,false);
//			_curSubTitle = TheMessage->lparam;
//			break;

	case ME_HOMOSAY:
		CharacterSay(TheMessage->lparam);
		break;

	case ME_SETSEM:
		switch (TheMessage->wparam1) {
		case C_SEMDIAG:
//F					SemDialogActive = (LLBOOL)TheMessage->wparam2;
			break;
		}
		break;

	case ME_SETDIALOG:
		/* switch(campo)
		{
		case C_DDEFANI:
		_dialog[indice].def_anim = valore;
		break;
		case C_DLINKANI:
		_dialog[indice].def_linkani = valore;
		break;
		case C_DSPEAKANI:
		_dialog[indice].def_speakani = valore;
		break;
		case C_DLISTENANI:
		_dialog[indice].def_listenani = valore;
		break;
		case C_DSCELTADISATTIVATO:
		_choice[indice].disattivato = valore;
		break;
		} */
		break;

	case ME_SETADIALOG: {
		/*   extern struct
		{
		uint16 _interlocutor;
		uint16 def_anim;
		uint16 def_speakani;
		uint16 def_listenani;
		uint16 def_linkani;
		} OtherDialogo;
		switch(campo)
		{
		case C_DADEFANI:
		OtherDialogo.def_anim = valore;
		break;
		case C_DALINKANI:
		OtherDialogo.def_linkani = valore;
		break;
		case C_DASPEAKANI:
		OtherDialogo.def_speakani = valore;
		break;
		case C_DALISTENANI:
		OtherDialogo.def_listenani = valore;
		break;
		}  */
	}
	break;

	case ME_SETANIMAZIONESPENTA:
//			AnimazioneSpenta = (uint16)valore;
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
		doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, indice, indice2, valore, _curObj);
		break;
	}
}

/*-------------------------------------------------------------------------*/
/*                               AtMouseClick           				   */
/*-------------------------------------------------------------------------*/
LLBOOL AtMouseClick(uint16 TheObj) {
	LLBOOL ret = true;

	if ((_curRoom == r1D) && !(Room[r1D]._flag & OBJFLAG_EXTRA) && (TheObj != oSCALA1D)) {
		_curObj = oDONNA1D;
		goToPosition(_obj[oDONNA1D]._position);
		return (true);
	} else if ((_curRoom == r2B) && (Room[r2B]._flag & OBJFLAG_EXTRA) && (TheObj != oCARTELLO2B) && (TheObj != od2BALLA28)) {
		_curObj = oPORTA2B;
		goToPosition(_obj[oCARTELLO2B]._position);
		return (true);
	}

	switch (TheObj) {
	case oPORTAA13:
		if ((mright) && (Room[r14]._flag & OBJFLAG_DONE)) {
			if (_homoGoToPosition != 4) goToPosition(4);
		} else {
			if (_homoGoToPosition != _obj[TheObj]._position) goToPosition(_obj[TheObj]._position);
		}
		ret = true;
		break;

	case oDIVANO14:
		if (mleft) {
			if (_homoGoToPosition != 2) goToPosition(2);
		} else {
			if (_homoGoToPosition != _obj[TheObj]._position) goToPosition(_obj[TheObj]._position);
		}
		ret = true;
		break;

	case oSCAFFALE1D:
		if (mright) {
			if (_homoGoToPosition != 9) goToPosition(9);
		} else {
			if (_homoGoToPosition != _obj[TheObj]._position) goToPosition(_obj[TheObj]._position);
		}
		ret = true;
		break;

	case oDIVANOR4A:
		if (mright) {
			if (_homoGoToPosition != 1) goToPosition(1);
		} else {
			if (_homoGoToPosition != _obj[TheObj]._position) goToPosition(_obj[TheObj]._position);
		}
		ret = true;
		break;

	case oPORTAC21:
		if ((UseWith[USED] == iPISTOLA15) && (SemUseWithStarted)) {
			if (_homoGoToPosition != 1) goToPosition(1);
		} else {
			if (_homoGoToPosition != _obj[TheObj]._position) goToPosition(_obj[TheObj]._position);
		}
		ret = true;
		break;

	case oTUBO21:
		if ((UseWith[USED] == oRAMPINO21) && (SemUseWithStarted)) {
			if (_homoGoToPosition != 4) goToPosition(4);
		} else {
			if (_homoGoToPosition != _obj[TheObj]._position) goToPosition(_obj[TheObj]._position);
		}
		ret = true;
		break;

	case oCAMPO23:
		if ((UseWith[USED] == oCAVIE23) && (SemUseWithStarted)) {
			if (_homoGoToPosition != 2) goToPosition(2);
		} else {
			if (_homoGoToPosition != _obj[TheObj]._position) goToPosition(_obj[TheObj]._position);
		}
		ret = true;
		break;

	case oTASTO2F:
		if ((UseWith[USED] == iTELECOMANDO2G) && (SemUseWithStarted)) {
			if (_homoGoToPosition != 9) goToPosition(9);
		} else {
			if (_homoGoToPosition != _obj[TheObj]._position) goToPosition(_obj[TheObj]._position);
		}
		ret = true;
		break;

	case oSAMA33:
	case oSERRATURA33:
		if ((UseWith[USED] == oTUBOT33) && (_obj[oVALVOLA34]._mode & OBJMODE_OBJSTATUS) && (_obj[oSAMA33]._mode & OBJMODE_OBJSTATUS) && (SemUseWithStarted)) {
			if (_homoGoToPosition != 4) goToPosition(4);
		} else {
			if (_homoGoToPosition != _obj[TheObj]._position) goToPosition(_obj[TheObj]._position);
		}
		ret = true;
		break;

	case oFINESTRA33:
		if (mleft) {
			if (_homoGoToPosition != 7) goToPosition(7);
		} else {
			if (_homoGoToPosition != _obj[TheObj]._position) goToPosition(_obj[TheObj]._position);
		}
		ret = true;
		break;

	case oPRESA35:
		if (!(SemUseWithStarted)) {
			if (_homoGoToPosition != 2) goToPosition(2);
		} else {
			if (_homoGoToPosition != _obj[TheObj]._position) goToPosition(_obj[TheObj]._position);
		}
		ret = true;
		break;

	case oTRIPLA35:
		if (!(SemUseWithStarted) && (mleft)) {
			if (_homoGoToPosition != 2) goToPosition(2);
		} else {
			if (_homoGoToPosition != _obj[TheObj]._position) goToPosition(_obj[TheObj]._position);
		}
		ret = true;
		break;

	case oPORTALC36:
		if ((UseWith[USED] == iSTETOSCOPIO) && (SemUseWithStarted)) {
			if (_homoGoToPosition != 8) goToPosition(8);
		} else {
			if (_homoGoToPosition != _obj[TheObj]._position) goToPosition(_obj[TheObj]._position);
		}
		ret = true;
		break;

	case oSERBATOIOA2G:
		if ((UseWith[USED] == iCANDELOTTO) && (InvObj[iBOMBOLA]._flag & OBJFLAG_EXTRA) && (InvObj[iLATTINA28]._flag & OBJFLAG_EXTRA)  && (SemUseWithStarted)) {
			if (_homoGoToPosition != 6) goToPosition(6);
		} else {
			if (_homoGoToPosition != _obj[TheObj]._position) goToPosition(_obj[TheObj]._position);
		}
		ret = true;
		break;

	case oRAGNO41:
		if ((UseWith[USED] == iMARTELLO) && (SemUseWithStarted)) {
			if (_homoGoToPosition != 1) goToPosition(1);
		} else {
			if (_homoGoToPosition != _obj[TheObj]._position) goToPosition(_obj[TheObj]._position);
		}
		ret = true;
		break;

	case oTAMBURO43:
		if ((UseWith[USED] == iMAZZA) && (SemUseWithStarted)) {
			if (_homoGoToPosition != 5) goToPosition(5);
		} else {
			if (_homoGoToPosition != _obj[TheObj]._position) goToPosition(_obj[TheObj]._position);
		}
		ret = true;
		break;

	case oLUCCHETTO53:
		if ((UseWith[USED] == iLASER35) && (SemUseWithStarted)) {
			if (_homoGoToPosition != 2) goToPosition(2);
		} else {
			if (_homoGoToPosition != _obj[TheObj]._position) goToPosition(_obj[TheObj]._position);
		}
		ret = true;
		break;

	case oPORTA58C55:
		if ((UseWith[USED] == iCHIAVI) && (SemUseWithStarted)) {
			if (_homoGoToPosition != 2) goToPosition(2);
		} else {
			if (_homoGoToPosition != _obj[TheObj]._position) goToPosition(_obj[TheObj]._position);
		}
		ret = true;
		break;

	default:
		if (_obj[TheObj]._position == -1) {
			_homoGoToPosition = -1;
			ret = false;
		} else {
			if (_homoGoToPosition != _obj[TheObj]._position)
				goToPosition(_obj[TheObj]._position);
			ret = true;
		}
		break;
	}

	if (Room[_curRoom]._flag & OBJFLAG_EXTRA) {
		if ((TheObj == oTUBO21) || (TheObj == oCARTELLONE21) || (TheObj == oESSE21) || (TheObj == oRAMPINO21) || (TheObj == oCATENA21) || (TheObj == od21ALLA22) || (TheObj == oPORTAC21) || (TheObj == oPORTAA21) || (TheObj == oCUNICOLO21) || (TheObj == od24ALLA23) || (TheObj == od2EALLA2C) || (TheObj == od2GVALLA26)) {
			_homoGoToPosition = -1;
			ret = true;
		}
	} else {
		if ((TheObj == od21ALLA23) || (TheObj == od24ALLA26) || (TheObj == oENTRANCE2E) || (TheObj == oCARTELLO2B) || (TheObj == oSPORTELLOC35) || (TheObj == oSPORTELLOA35) || (TheObj == oASCENSORE35) || (TheObj == oGIORNALE35)) {
			_homoGoToPosition = -1;
			ret = true;
		}
	}
	return ret;
}

/* -----------------26/11/97 10.37-------------------
 * 					AtEndChangeRoom
 * --------------------------------------------------*/
void AtEndChangeRoom() {
//	gestione particolare campi magnetici
	if ((_curRoom == r2E) && (_obj[oPULSANTEADS2D]._mode & OBJMODE_OBJSTATUS)) {
		AnimTab[aBKG2E].flag |= SMKANIM_OFF1;
		_obj[oCAMPO2E]._mode &= ~OBJMODE_OBJSTATUS;
		_obj[oCARTELLOA2E]._mode &= ~OBJMODE_OBJSTATUS;
		_obj[oCARTELLOS2E]._mode |= OBJMODE_OBJSTATUS;
	} else if ((_curRoom == r2E) && (_obj[oPULSANTEADA2D]._mode & OBJMODE_OBJSTATUS)) {
		AnimTab[aBKG2E].flag &= ~SMKANIM_OFF1;
		_obj[oCAMPO2E]._mode |= OBJMODE_OBJSTATUS;
		_obj[oCARTELLOA2E]._mode |= OBJMODE_OBJSTATUS;
		_obj[oCARTELLOS2E]._mode &= ~OBJMODE_OBJSTATUS;
	}
	if ((_curRoom == r2C) && (_obj[oPULSANTEACS2D]._mode & OBJMODE_OBJSTATUS)) {
		AnimTab[aBKG2C].flag |= SMKANIM_OFF1;
		_obj[oCAMPO2C]._mode &= ~OBJMODE_OBJSTATUS;
		_obj[oCARTELLOA2C]._mode &= ~OBJMODE_OBJSTATUS;
		_obj[oCARTELLOS2C]._mode |= OBJMODE_OBJSTATUS;
	} else if ((_curRoom == r2C) && (_obj[oPULSANTEACA2D]._mode & OBJMODE_OBJSTATUS)) {
		AnimTab[aBKG2C].flag &= ~SMKANIM_OFF1;
		_obj[oCAMPO2C]._mode |= OBJMODE_OBJSTATUS;
		_obj[oCARTELLOS2C]._mode &= ~OBJMODE_OBJSTATUS;
		_obj[oCARTELLOA2C]._mode |= OBJMODE_OBJSTATUS;
	}
	if ((_curRoom == r28) && (_obj[oPULSANTEBBS2D]._mode & OBJMODE_OBJSTATUS)) {
		AnimTab[aBKG28].flag |= SMKANIM_OFF2;
		_obj[oCESPUGLIO28]._anim = a283;
		_obj[oSERPENTETM28]._mode |= OBJMODE_OBJSTATUS;
		_obj[oSERPENTEAM28]._mode |= OBJMODE_OBJSTATUS;
		RegenRoom();
	} else if ((_curRoom == r28) && (_obj[oPULSANTEBBA2D]._mode & OBJMODE_OBJSTATUS)) {
		AnimTab[aBKG28].flag &= ~SMKANIM_OFF2;
		_obj[oCESPUGLIO28]._anim = a282;
		_obj[oSERPENTETM28]._mode &= ~OBJMODE_OBJSTATUS;
		_obj[oSERPENTEAM28]._mode &= ~OBJMODE_OBJSTATUS;
		RegenRoom();
	}
	if ((_curRoom == r28) && (_obj[oPULSANTEABS2D]._mode & OBJMODE_OBJSTATUS)) {
		AnimTab[aBKG28].flag |= (SMKANIM_OFF1 | SMKANIM_OFF3);
		_obj[oCAMPO28]._mode &= ~OBJMODE_OBJSTATUS;
		_obj[oCARTELLOA28]._mode &= ~OBJMODE_OBJSTATUS;
		_obj[oCARTELLOS28]._mode |= OBJMODE_OBJSTATUS;
		read3D("282.3d");
	} else if ((_curRoom == r28) && (_obj[oPULSANTEABA2D]._mode & OBJMODE_OBJSTATUS)) {
		AnimTab[aBKG28].flag &= ~(SMKANIM_OFF1 | SMKANIM_OFF3);
		_obj[oCAMPO28]._mode |= OBJMODE_OBJSTATUS;
		_obj[oCARTELLOA28]._mode |= OBJMODE_OBJSTATUS;
		_obj[oCARTELLOS28]._mode &= ~OBJMODE_OBJSTATUS;
	}
//	fine gestione particolare campi magnetici

//	Azioni
	if ((_curRoom == r12) && ((OldRoom == r13) || (OldRoom == r16)))
		StartCharacterAction(a1213ARRIVACONASCENSORE, 0, 0, 0);
	else if ((_curRoom == r16) && ((OldRoom == r13) || (OldRoom == r12)))
		StartCharacterAction(a1617SCENDECONASCENSORE, 0, 0, 0);
	else if ((_curRoom == r13) && (OldRoom == r12))
		StartCharacterAction(a1315ARRIVAASCENSOREALTOENTRA, 0, 0, 0);
//	else if((_curRoom==r16) && (OldRoom==r17))
// 		StartCharacterAction(a1612ARRIVADALBINARIO,0,0);
	else if ((_curRoom == r13) && (OldRoom == r16))
		StartCharacterAction(a1314ARRIVAASCENSOREDABASSO, 0, 0, 0);
	else if ((_curRoom == r14) && (OldRoom == r13))
		StartCharacterAction(a1414ENTERROOM, 0, 0, 0);
	else if ((_curRoom == r18) && (OldRoom == r17) && !(Room[r18]._flag & OBJFLAG_DONE))
		StartCharacterAction(a186GUARDAPIAZZA, 0, 0, 0);
	else if ((_curRoom == r18) && (OldRoom == r19))
		StartCharacterAction(a189ENTRADALCLUB, 0, 0, 0);
	else if ((_curRoom == r1A) && (OldRoom == r18) && (Room[r1A]._flag & OBJFLAG_DONE)) {
		StartCharacterAction(a1A5ENTRA, 0, 0, 0);
		AnimTab[aBKG1A].flag |= SMKANIM_OFF1;
	} else if ((_curRoom == r1C) && (OldRoom == r1B))
		StartCharacterAction(a1C1ENTRACUNICOLO, 0, 0, 0);
	else if ((_curRoom == r1B) && (OldRoom == r1C))
		StartCharacterAction(a1B11ESCETOMBINO, 0, 0, 0);
	else if ((_curRoom == r1D) && (OldRoom == r1B) && (_obj[oDONNA1D]._mode & OBJMODE_OBJSTATUS)) {
		StartCharacterAction(a1D1SCENDESCALE, 0, 1, 0);
		_actor._lim[0] = 60;
		_actor._lim[2] = 240;
	} else if ((_curRoom == r1D) && (OldRoom == r1B) && !(_obj[oDONNA1D]._mode & OBJMODE_OBJSTATUS)) {
		StartCharacterAction(a1D12SCENDESCALA, 0, 1, 0);
		_actor._lim[0] = 60;
		_actor._lim[2] = 240;
	} else if ((_curRoom == r1B) && (OldRoom == r1D))
		StartCharacterAction(a1B4ESCEBOTOLA, 0, 0, 0);
	else if ((_curRoom == r1B) && (OldRoom == r18) && (AnimTab[aBKG1B].flag & SMKANIM_OFF1))
		StartCharacterAction(a1B12SCAPPATOPO, 0, 0, 0);
	else if ((_curRoom == r24) && (OldRoom == r2H))
		StartCharacterAction(a242, 0, 0, 0);
	else if ((_curRoom == r25) && (OldRoom == r2A))
		StartCharacterAction(a257, 0, 0, 0);
	else if ((_curRoom == r28) && (OldRoom == r27))
		StartCharacterAction(aWALKIN, 0, 0, 0);
	else if ((_curRoom == r29) && (OldRoom == r2A))
		StartCharacterAction(a298ESCEBOTOLA, 0, 0, 0);
	else if ((_curRoom == r29L) && (OldRoom == r2A))
		StartCharacterAction(a2910ESCEBOTOLA, 0, 0, 0);
	else if ((_curRoom == r2A) && (OldRoom == r25))
		StartCharacterAction(aWALKIN, 0, 0, 0);
	else if ((_curRoom == r2A) && ((OldRoom == r29) || (OldRoom == r29L)))
		StartCharacterAction(aWALKIN, 0, 0, 0);
	else if ((_curRoom == r2B) && (OldRoom == r2A))
		StartCharacterAction(a2B2ESCEPOZZO, 0, 2, 0);
	else if ((_curRoom == r2H) && (OldRoom == r24))
		StartCharacterAction(a2H1ARRIVA, 0, 0, 0);
	else if ((_curRoom == r2E) && (OldRoom == r2F))
		StartCharacterAction(a2E5SECONDAPARRIVANDO, 0, 0, 0);
	else if ((_curRoom == r2F) && (OldRoom == r2E))
		StartCharacterAction(aWALKIN, 0, 0, 0);
	else if ((_curRoom == r23B) && (OldRoom == r21))
		StartCharacterAction(aWALKIN, 0, 0, 0);
	else if ((_curRoom == r23A) && (OldRoom == r21) && (Room[r23A]._flag & OBJFLAG_DONE))
		StartCharacterAction(aWALKIN, 0, 0, 0);
	else if ((_curRoom == r23A) && (OldRoom == r21) && (!(Room[r23A]._flag & OBJFLAG_DONE)))
		SemShowHomo = false;
	else if ((_curRoom == r21) && ((OldRoom == r23A) || (OldRoom == r23B)))
		StartCharacterAction(aWALKIN, 0, 0, 0);
	else if ((_curRoom == r2BL) || (_curRoom == r36F) || (_curRoom == r41D) || (_curRoom == r49M) || (_curRoom == r4CT) ||
			 (_curRoom == r58T) || (_curRoom == r58M) || (_curRoom == r59L) || (_curRoom == rSYS) ||
			 (_curRoom == r12CU) || (_curRoom == r13CU)) {     // Screens without inventory
		SemShowHomo = false;
		SemCharacterExist = false;
		SemInventoryLocked = true;
	} else if ((_curRoom == r31P) || (_curRoom == r35P)) {   // Screens with inventory
		SemShowHomo = false;
		SemCharacterExist = false;
	} else if ((_curRoom == r2F) && (OldRoom == r31))
		StartCharacterAction(a2F4ESCEASCENSORE, 0, 0, 0);
	else if ((_curRoom == r31) && (OldRoom == r2F))
		StartCharacterAction(a3114ESCEPASSAGGIO, 0, 0, 0);
	else if ((_curRoom == r31) && !(Room[r31]._flag & OBJFLAG_DONE))
		setPosition(14);
	else if ((_curRoom == r32) && (OldRoom == r31))
		StartCharacterAction(a321SALEMONTACARICHI, 0, 0, 0);
	else if ((_curRoom == r33) && (OldRoom == r32) && (_obj[oBRUCIATURA33]._mode & OBJMODE_OBJSTATUS))
		StartCharacterAction(a3311SALESCALE, 0, 0, 0);
	else if ((_curRoom == r33) && (OldRoom == r32) && !(_obj[oBRUCIATURA33]._mode & OBJMODE_OBJSTATUS))
		StartCharacterAction(a3313CHIUDEBOTOLA, 0, 0, 0);
	else if ((_curRoom == r32) && (OldRoom == r33))
		StartCharacterAction(a325SCENDESCALE, 0, 0, 0);
	else if ((_curRoom == r36) && (OldRoom == r35))
		StartCharacterAction(a361ESCEASCENSORE, 0, 0, 0);
	else if ((_curRoom == r35) && (OldRoom == r36))
		StartCharacterAction(a3515ESCEASCENSORE, 0, 0, 0);
	else if ((_curRoom == r44) && (OldRoom == r45  || OldRoom == r45S) && !(InvObj[iDISLOCATORE]._flag & OBJFLAG_EXTRA))
		StartCharacterAction(a445, 0, 0, 0);
	else if ((_curRoom == r45 || _curRoom == r45S) && (OldRoom == r44) && !(InvObj[iDISLOCATORE]._flag & OBJFLAG_EXTRA))
		StartCharacterAction(a457, 0, 0, 0);
	else if ((_curRoom == r46) && (OldRoom == r47  || OldRoom == r48  || OldRoom == r49) && !(InvObj[iDISLOCATORE]._flag & OBJFLAG_EXTRA))
		StartCharacterAction(aWALKIN, 0, 0, 0);
	else if ((_curRoom == r47) && (OldRoom == r46) && !(InvObj[iDISLOCATORE]._flag & OBJFLAG_EXTRA))
		StartCharacterAction(a476, 0, 0, 0);
	else if ((_curRoom == r48) && (OldRoom == r46) && !(InvObj[iDISLOCATORE]._flag & OBJFLAG_EXTRA))
		StartCharacterAction(a485, 0, 0, 0);
	else if ((_curRoom == r49) && (OldRoom == r46) && !(InvObj[iDISLOCATORE]._flag & OBJFLAG_EXTRA))
		StartCharacterAction(a494, 0, 0, 0);
	else if ((_curRoom == r54) && (OldRoom == r53)) {
		StartCharacterAction(a5411, 0, 11, 0);
		InvObj[iLASER35]._examine = 1599;
	} else if ((OldRoom == r41D) && (InvObj[iDISLOCATORE]._flag & OBJFLAG_EXTRA)) {
		setPosition(30);/*StartCharacterAction(hLAST,0,30,0);*/drawCharacter(CALCPOINTS);
	}

//	FullMotion
	if ((_curRoom == r17) && (OldRoom == r16)) {
		PlayDialog(dF171);
		WaitSoundFadEnd();
	} else if ((_curRoom == r16) && (OldRoom == r17)) {
		PlayDialog(dF161);
		WaitSoundFadEnd();
		KillIcon(iFOGLIO14);
	} else if ((_curRoom == r21) && (OldRoom == r1C)) {
		PlayDialog(dF1C1);
		WaitSoundFadEnd();
	} else if ((_curRoom == r23B) && (OldRoom == r23A)) {
		PlayDialog(dF231);
		WaitSoundFadEnd();
		KillIcon(iMAPPA17);
	} else if ((_curRoom == r29L) && (OldRoom == r29)) {
		PlayDialog(dF291);
		WaitSoundFadEnd();
	} else if ((_curRoom == r2GV) && (OldRoom == r2G)) {
		PlayDialog(dF2G2);
		WaitSoundFadEnd();
	} else if ((_curRoom == r31) && (OldRoom == r32) && (Room[r32]._flag & OBJFLAG_EXTRA)) {
		PlayDialog(dF321);
		SemShowHomo = false;
		WaitSoundFadEnd();
		Room[r32]._flag &= ~OBJFLAG_EXTRA;
	} else if ((_curRoom == r19) && !(Room[r19]._flag & OBJFLAG_DONE)) {
		PlayScript(s19EVA);
		SemNoPaintScreen = false;
		ClearText();
		RepaintString();
		WaitSoundFadEnd();
	}
	// CHECKME: This check is identical to the 3rd one, thus it's always false
	else if ((_curRoom == r21) && (/*!( Room[_curRoom]._flag & OBJFLAG_DONE ) || */(OldRoom == r1C))) {
		setPosition(10);
		TendIn();
		SemNoPaintScreen = false;
		ClearText();
		RepaintString();
	} else if ((_curRoom == r46) && (OldRoom == r43) && !(InvObj[iDISLOCATORE]._flag & OBJFLAG_EXTRA)) {
		PlayDialog(dF431);
		WaitSoundFadEnd();
	} else if ((_curRoom == r45S) && (OldRoom == r45) && !(InvObj[iDISLOCATORE]._flag & OBJFLAG_EXTRA)) {
		PlayDialog(dF451);
		WaitSoundFadEnd();
	} else if ((_curRoom == r4A) && (OldRoom == r49) && !(InvObj[iDISLOCATORE]._flag & OBJFLAG_EXTRA)) {
		PlayDialog(dF491);
		WaitSoundFadEnd();
	} else if ((_curRoom == r4A) && (OldRoom == r41D) && (InvObj[iDISLOCATORE]._flag & OBJFLAG_EXTRA) && (_obj[ocHELLEN4A]._mode & OBJMODE_OBJSTATUS)) {
		WaitSoundFadEnd();
		PlayDialog(dC4A1);
	} else if ((_curRoom == r4P) && (OldRoom == r4O) && !(Room[r4P]._flag & OBJFLAG_DONE)) {
		PlayDialog(dF4PI);
		SemShowHomo = false;
		WaitSoundFadEnd();
	} else if ((_curRoom == r51) && (OldRoom == r4CT) /*&& !( InvObj[iDISLOCATORE]._flag & OBJFLAG_EXTRA )*/) {
		PlayDialog(dF4C1);
		SemShowHomo = false;
		WaitSoundFadEnd();
	}
	/*	else if((_curRoom==r18) && (OldRoom==r1B))
		{
			if( !(_obj[oBOTTIGLIA1D]._mode&OBJMODE_OBJSTATUS) && !(_obj[oRETE17]._mode&OBJMODE_OBJSTATUS) )
			{	PlayDialog(dF181);WaitSoundFadEnd();setPosition(_obj[oCANCELLATA1B]._ninv); }
			else
			{	StartCharacterAction(_obj[oCANCELLATA1B]._anim,0,0,0);setPosition(_obj[oCANCELLATA1B]._ninv); }
		}
	*/	else if ((_curRoom == r1A) && (OldRoom == r18)) {
		if (!(Room[r1A]._flag & OBJFLAG_DONE)) {
			PlayDialog(dF1A1);
			WaitSoundFadEnd();
			_obj[oTOPO1C]._flag |= OBJFLAG_TAKE;
			_obj[oTOPO1C]._anim = a1C3RACCOGLIETOPO;
		} else {
			TendIn();
			SemNoPaintScreen = false;
			RepaintString();
		}
	} else {
		TendIn();
		SemNoPaintScreen = false;
		ClearText();
		RepaintString();
	}

//	Sentence
	if ((_curRoom == r17) && (OldRoom == r18) && !(Room[r17]._flag & OBJFLAG_DONE) && (_obj[oRETE17]._mode & OBJMODE_OBJSTATUS))
		CharacterSay(189);
	if (((_curRoom == r12CU) || (_curRoom == r13CU)) && (CloseUpObj) && (_obj[CloseUpObj]._examine))
		CharacterSay(_obj[CloseUpObj]._examine);
	else if ((_curRoom == r23A) && (OldRoom == r21) && (!(Room[r23A]._flag & OBJFLAG_DONE))) {
		SemShowHomo = true;
		StartCharacterAction(aWALKIN, 0, 0, 361);
	} else if ((_curRoom == r24) && !(Room[r24]._flag & OBJFLAG_DONE))
		CharacterSay(381);
	else if ((_curRoom == r2G) && !(Room[r2G]._flag & OBJFLAG_DONE))
		CharacterSay(688);
	else if ((_curRoom == r4C) && (OldRoom == r4CT))
		CharacterSay(1163);
	else if ((_curRoom == r41) && (OldRoom == r36) && !(Room[r41]._flag & OBJFLAG_DONE))
		CharacterSay(900);
	else if ((_curRoom == r58) && (_obj[oGUARDIA58]._mode & OBJMODE_OBJSTATUS) && (_obj[oGUARDIA58]._anim)) {
		_curObj = oGUARDIA58;
		doEvent(MC_MOUSE, ME_MRIGHT, MP_DEFAULT, 372, 335 + TOP, 0, oGUARDIA58);
	} else if ((_curRoom == r59L))
		CharacterSay(1394);
	else if ((_curRoom == r58) && (OldRoom == r58T))
		CharacterSay(1368);
	else if ((_curRoom == r5A) && !(Room[r5A]._flag & OBJFLAG_DONE))
		CharacterSay(1408);
	else if ((_curRoom == rSYS) && (OldRoom == rSYS))
		DoSys(o00LOAD);
	InvObj[iDISLOCATORE]._flag &= ~OBJFLAG_EXTRA;
}

/* -----------------26/11/97 10.38-------------------
 * 					GestioneATFrame
 * --------------------------------------------------*/
/*PELS: GESTIONE ATFRAME PER ANIMAZIONI  ***************************/
#define ATF_WAITTEXT 1

static struct ATFHandle {
	int16 curframe, lastframe;
	uint16 object;
	uint16 status;
	struct SAnim *curanim;
}            // 0->homo 1->background 2->icon
AnimType[3] = {	{true}, {true}, {true}	};

//static uint16 atfnotvalid=true,atfrm=0,toframe=0,curframe;
//static struct SAnim *curanim=NULL;

/*PELS: Funzione di esecuzione comandi ATFDO *************/
/*PELS: si potrebbe evitare il switccione con una tavola di ptr a funz.  */
/*PELS: oppure fare pochi comandi ma a _flag in modo da poterne eseguire piu' di uno
per frame ... abbiamo tutto index => 16 comandi => 16 if da fare...
probabile che faremo meno comandi */

/* -----------------11/07/97 11.43-------------------
					ExecuteATFDO
 --------------------------------------------------*/
void ExecuteATFDO(struct ATFHandle *h, int doit, int obj) {
	switch (doit) {
	case fCLROBJSTATUS:
		_obj[obj]._mode &= ~OBJMODE_OBJSTATUS;
		RegenRoom();
		break;
	case fSETOBJSTATUS:
		_obj[obj]._mode |= OBJMODE_OBJSTATUS;
		RegenRoom();
		break;
	case fONETIME:
		_obj[obj]._anim = 0;
		break;
	case fCREPACCIO:
		if (Room[r2E]._flag & OBJFLAG_EXTRA)
			_obj[oCREPACCIO2E]._position = 7;
		else
			_obj[oCREPACCIO2E]._position = 6;
		break;
	case fSERPVIA: {
		extern message Serp52;
		doEvent(Serp52.cls,  Serp52.event,  Serp52.priority, Serp52.wparam1, Serp52.wparam2, Serp52.bparam, Serp52.lparam);
	}
	break;
	case fPIRANHA:
		_obj[oLUCCHETTO53]._anim = 0;
		_obj[oGRATAC53]._anim = 0;
		_obj[oGRATAA53]._anim = 0;
		_obj[oLUCCHETTO53]._action = 1240;
		_obj[oGRATAC53]._action = 1243;
		_obj[oGRATAA53]._action = 1246;
		_obj[oLAGO53]._examine = 1237;
		break;
	case fMOREAU:
		_obj[oFINESTRAB58]._anim = 0;
		_obj[oFINESTRAB58]._action = 1358;
		break;
	case fPORTA58:
		doEvent(MC_MOUSE, ME_MLEFT, MP_DEFAULT, 468, 180 + TOP, true, oPORTA58C55);
		break;
	case fHELLEN:
//			SemMouseEnabled = false;
		doEvent(MC_MOUSE, ME_MLEFT, MP_DEFAULT, 336, 263 + TOP, true, 0);
		break;
	case fVALVOLAON34: {
		extern short SFxVol;
		if (!(_choice[616]._flag & OBJFLAG_DONE) &&		// non ho fatto fmv e
				(_obj[oTUBOA34]._mode & OBJMODE_OBJSTATUS)	&&	// c'e' il tubo tagliato e
				!(_obj[oTUBOFT34]._mode & OBJMODE_OBJSTATUS))	// non c'e' il tubo fuori il tubo fuori
			CallSmackVolumePan(0, 2, (32768 * 2 * SFxVol) / 125);
	}
	break;
	case fVALVOLAOFF34:
		CallSmackVolumePan(0, 2, 0);
		break;

	case fHOMOOFF:
		SemCharacterExist = false;
		break;
	case fHOMOON:
		SemCharacterExist = true;
		break;
	case fHOMOFORE:
		_forcedActorPos = FOREGROUND;
		break;
	case fHOMOBACK:
		_forcedActorPos = BACKGROUND;
		break;
	case fHOMONORM:
		_forcedActorPos = 0;
		break;
	case fSETEXTRA:
		_obj[obj]._flag |= OBJFLAG_EXTRA;
		break;
	case fCLREXTRA:
		_obj[obj]._flag &= ~OBJFLAG_EXTRA;
		break;

	case fANIMOFF1:
		AnimTab[Room[_curRoom]._bkgAnim].flag |= SMKANIM_OFF1;
		if ((_curRoom == r11) || (_curRoom == r1D) || (_curRoom == r14) || (_curRoom == r22) || (_curRoom == r48) || (_curRoom == r4P))
			CallSmackVolumePan(0, 1, 0);
		break;
	case fANIMOFF2:
		AnimTab[Room[_curRoom]._bkgAnim].flag |= SMKANIM_OFF2;
		if ((_curRoom == r2E))
			CallSmackVolumePan(0, 2, 0);
		break;
	case fANIMOFF3:
		AnimTab[Room[_curRoom]._bkgAnim].flag |= SMKANIM_OFF3;
		break;
	case fANIMOFF4:
		AnimTab[Room[_curRoom]._bkgAnim].flag |= SMKANIM_OFF4;
		if (_curRoom == r28)
			CallSmackVolumePan(0, 1, 0);
		break;

	case fANIMON1:
		AnimTab[Room[_curRoom]._bkgAnim].flag &= ~SMKANIM_OFF1;
		if ((_curRoom == r14) || (_curRoom == r1D) || (_curRoom == r22) || (_curRoom == r48) || (_curRoom == r4P)) {
			extern short SFxVol;
			CallSmackVolumePan(0, 1, (32768 * 2 * SFxVol) / 125);
		}
		break;
	case fANIMON2:
		AnimTab[Room[_curRoom]._bkgAnim].flag &= ~SMKANIM_OFF2;
		if ((_curRoom == r2E)) {
			extern short SFxVol;
			CallSmackVolumePan(0, 2, (32768 * 2 * SFxVol) / 125);
		}
		break;
	case fANIMON3:
		AnimTab[Room[_curRoom]._bkgAnim].flag &= ~SMKANIM_OFF3;
		break;
	case fANIMON4:
		AnimTab[Room[_curRoom]._bkgAnim].flag &= ~SMKANIM_OFF4;
		break;

	case fENDDEMO:
		//if( QuitGame() )
		DemoOver();
		doEvent(MC_SYSTEM, ME_QUIT, MP_SYSTEM, 0, 0, 0, 0);
		break;
	case fSTOP2TXT:
		h->status |= ATF_WAITTEXT;
//setta un _flag che viene sempre cancellato
// quando si finisce di parlare
// se il _flag e' attivato la anim non playia piu'
// ( da fare nel player smacker )
// anche i contatori in AtFrameNext si fermano
		break;
	}
}

/* -----------------11/07/97 11.42-------------------
					ProcessATF
 --------------------------------------------------*/
void ProcessATF(struct ATFHandle *h, int type, int atf) {
	static int dc = 0;
	extern char UStr[];

	switch (type) {
	case ATFTEXT:
		CharacterTalkInAction(h->curanim->atframe[atf].index);
		break;
	case ATFTEXTACT:
		CharacterTalkInAction(_obj[h->object]._action);
		break;
	case ATFTEXTEX:
		CharacterTalkInAction(_obj[h->object]._examine);
		break;
	case ATFCLR:
		_obj[h->curanim->atframe[atf].index]._mode &= ~OBJMODE_OBJSTATUS;
		RegenRoom();
		break;
	case ATFCLRI:
		KillIcon(h->curanim->atframe[atf].index);
		break;
	case ATFCEX:
		_obj[h->object]._examine = h->curanim->atframe[atf].index;
		break;
	case ATFCACT:
		_obj[h->object]._action = h->curanim->atframe[atf].index;
		break;
	case ATFSET:
		_obj[h->curanim->atframe[atf].index]._mode |= OBJMODE_OBJSTATUS;
		RegenRoom();
		break;
	case ATFSETI:
		AddIcon(h->curanim->atframe[atf].index);
		break;
	case ATFDO:
		ExecuteATFDO(h, h->curanim->atframe[atf].index, h->object);
		break;
	case ATFROOM:
		doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, h->curanim->atframe[atf].index, 0, 0, h->object);
		break;
	case ATFSETPOS:
		setPosition(h->curanim->atframe[atf].index);
		break;
	case ATFDIALOG:
		PlayDialog(h->curanim->atframe[atf].index);
		break;
	case ATFCOBJANIM:
		_obj[h->object]._anim = h->curanim->atframe[atf].index;
		RegenRoom();
		break;
	case ATFCOBJBOX:
		_obj[h->object]._nbox = h->curanim->atframe[atf].index;
		RegenRoom();
		break;
	case ATFCOBJPOS:
		_obj[h->object]._position = h->curanim->atframe[atf].index;
		RegenRoom();
		break;
	case ATFSETFORE:
		_obj[h->curanim->atframe[atf].index]._nbox = FOREGROUND;
		RegenRoom();
		break;
	case ATFSETBACK:
		_obj[h->curanim->atframe[atf].index]._nbox = BACKGROUND;
		RegenRoom();
		break;
	case ATFSWITCH:
		_obj[h->curanim->atframe[atf].index]._mode ^= OBJMODE_OBJSTATUS;
		RegenRoom();
		break;
	case ATFSETROOMT:
		SetRoom(h->curanim->atframe[atf].index, true);
		break;
	case ATFSETROOMF:
		SetRoom(h->curanim->atframe[atf].index, false);
		break;
	case ATFREADBOX:
		if (h->curanim->atframe[atf].index == 1) {
			sprintf(UStr, "%s.3d", Room[_curRoom]._baseName);
			read3D(UStr);
			Room[_curRoom]._flag &= ~OBJFLAG_EXTRA;
		} else if (h->curanim->atframe[atf].index == 2) {
			sprintf(UStr, "%s2.3d", Room[_curRoom]._baseName);
			read3D(UStr);
			Room[_curRoom]._flag |= OBJFLAG_EXTRA;
			if ((_curRoom == r37)) {
				extern short SFxVol;
				CallSmackVolumePan(0, 1, (32768 * 2 * SFxVol) / 125);
			}
		}
		break;
	case ATFONESPEAK:
		switch (h->curanim->atframe[atf].index) {
		case 1:
			if (Room[r1D]._flag & OBJFLAG_EXTRA)
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
		//if( QuitGame() )
		DemoOver();
		doEvent(MC_SYSTEM, ME_QUIT, MP_SYSTEM, 0, 0, 0, 0);
		break;

	}

}

/* -----------------11/07/97 11.42-------------------
					InitAtFrameHandler
 --------------------------------------------------*/
void InitAtFrameHandler(uint16 an, uint16 obj) {
	struct SAnim *anim = &AnimTab[an];
	struct ATFHandle *handle;

	handle = &AnimType[0];
	if (anim->flag & SMKANIM_BKG)
		handle = &AnimType[1];
	if (anim->flag & SMKANIM_ICON)
		handle = &AnimType[2];

	handle->curanim = anim;
	if (obj)
		handle->object = obj;
	else
		handle->object = _curObj;
	handle->curframe = 0;
	handle->lastframe = -1;
	handle->status = 0;
}

/* -----------------11/07/97 11.41-------------------
					AtFrameNext
 --------------------------------------------------*/
void AtFrameNext() {
//	extern uint16 _curAnimFrame[];

	if (!((AnimType[0].status & ATF_WAITTEXT) && SemCharacterSpeak))
		AnimType[0].curframe++;
	if (!((AnimType[1].status & ATF_WAITTEXT) && SemCharacterSpeak))
		AnimType[1].curframe++;
	if (!((AnimType[2].status & ATF_WAITTEXT) && SemCharacterSpeak))
		AnimType[2].curframe++;
}

/* -----------------11/07/97 11.42-------------------
					AtFrameEnd
 --------------------------------------------------*/
void AtFrameEnd(int type) {
	struct ATFHandle *h;
	struct SAnim *anim;
	int32 a;

	h = &AnimType[type];
	anim = h->curanim;
	h->curframe = 0;
	/*	if((h->curanim->atframe[0].type==0) && (_obj[h->_object]._action))
		{
			CharacterTalkInAction(_obj[h->_object]._action);
			return;
		}
	*/
	// se ho gia' fatto questo atframe
	if (h->curframe == h->lastframe)
		return ;
	else
		h->lastframe = h->curframe;

	for (a = 0; a < MAXATFRAME; a++) {
		// se e' il momento di eseguire questo AtFrame
		if ((anim->atframe[a].nframe == 0) && (anim->atframe[a].type)) {
			if ((anim->atframe[a].child == 0) ||
					((anim->atframe[a].child == 1) && !(AnimTab[Room[_curRoom]._bkgAnim].flag & SMKANIM_OFF1)) ||
					((anim->atframe[a].child == 2) && !(AnimTab[Room[_curRoom]._bkgAnim].flag & SMKANIM_OFF2)) ||
					((anim->atframe[a].child == 3) && !(AnimTab[Room[_curRoom]._bkgAnim].flag & SMKANIM_OFF3)) ||
					((anim->atframe[a].child == 4) && !(AnimTab[Room[_curRoom]._bkgAnim].flag & SMKANIM_OFF4)))
				ProcessATF(h, anim->atframe[a].type, a);
		}
	}

	h->curanim = NULL;
}

/* -----------------11/07/97 11.44-------------------
					AtFrameHandler
 --------------------------------------------------*/
void AtFrameHandler(int type) {
	struct ATFHandle *h;
	struct SAnim *anim;
	int32 a;

	h = &AnimType[type];
	anim = h->curanim;
	if (anim == NULL)
		return ;
	if (h->curframe == 0)
		h->curframe ++;
	// se ho gia' fatto questo atframe
	if (h->curframe <= h->lastframe)
		return ;

	for (a = 0; a < MAXATFRAME; a++) {
		// se e' il momento di eseguire questo AtFrame
		if ((anim->atframe[a].nframe > h->lastframe) &&
				(anim->atframe[a].nframe <= h->curframe) &&
				(anim->atframe[a].nframe != 0)) {
			if ((anim->atframe[a].child == 0) ||
					((anim->atframe[a].child == 1) && !(AnimTab[Room[_curRoom]._bkgAnim].flag & SMKANIM_OFF1)) ||
					((anim->atframe[a].child == 2) && !(AnimTab[Room[_curRoom]._bkgAnim].flag & SMKANIM_OFF2)) ||
					((anim->atframe[a].child == 3) && !(AnimTab[Room[_curRoom]._bkgAnim].flag & SMKANIM_OFF3)) ||
					((anim->atframe[a].child == 4) && !(AnimTab[Room[_curRoom]._bkgAnim].flag & SMKANIM_OFF4)))
				ProcessATF(h, anim->atframe[a].type, a);
		}
	}

	// setta lastframe
	h->lastframe = h->curframe;
}

} // End of namespace Trecision
