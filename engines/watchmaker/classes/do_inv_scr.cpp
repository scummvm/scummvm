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

#include "watchmaker/classes/do_inv_scr.h"
#include "watchmaker/types.h"
#include "watchmaker/define.h"
#include "watchmaker/message.h"
#include "watchmaker/globvar.h"
#include "watchmaker/schedule.h"
#include "watchmaker/windows_hacks.h"
#include "watchmaker/classes/do_string.h"
#include "watchmaker/classes/do_inv.h"
#include "watchmaker/ll/ll_mesh.h"
#include "watchmaker/ll/ll_anim.h"
#include "watchmaker/ll/ll_diary.h"

namespace Watchmaker {

/* -----------------19/05/98 16.40-------------------
 *                  doInvScrUseWith
 * --------------------------------------------------*/
void doInvScrUseWith(WGame &game) {
	uint8 sent = FALSE;
	int32 sa = 0;
	Init &init = game.init;

	//messo fuori perche' andava in conflitto con l'usacon dell'icona
	if ((UseWith[USED] == i29FOTOPROGETTO) && (UseWith[WITH] == ocCUSTODE)) {
		Event(EventClass::MC_DIALOG, ME_DIALOGSTART, MP_DEFAULT, dR005, 0, 0, nullptr, nullptr, nullptr);
		return;
	}

	//messo fuori perche' se non parte l'RTV deve dire la frase
	if ((UseWith[USED] == i1cMEDAGLIONI2) && (UseWith[WITH] == o1CBOCCA)) {
		if (init.Dialog[dR1A5].flags & DIALOG_DONE) {
			Event(EventClass::MC_DIALOG, ME_DIALOGSTART, MP_DEFAULT, dR1C3, 0, 0, nullptr, nullptr, nullptr);
			return;
		} else {
			PlayerSpeak(game, init.InvObj[i1cMEDAGLIONI2].text[1]);
			return;
		}
	}

	//messo fuori perche' nel caso non si verificasse il gioco deve procedere normalmente con frase/azione dell'icona
	if ((UseWith[WITH] == o17CAVETTO) && ((UseWith[USED] == i00TELEFONO) || (UseWith[USED] == i00TELEFONOVIC))) {
//		se non ho sentito il numero...
		if (!(init.InvObj[i28WALKMANOK].flags & EXTRA2)) {
			PlayerSpeak(game,  init.Obj[o17COMPUTER].action[CurPlayer]);
			return;
		}
	}

	//caso delle chiavi del cuoco che aprono due porte: evito che si usino due volte sulla stessa porta
	if ((UseWith[USED] == i25MAZZODUECHIAVI) && ((UseWith[WITH] == o2Hp2G) || (UseWith[WITH] == o2Gp2H) || (UseWith[WITH] == oXT16p17) || (UseWith[WITH] == o17p16))) {
//		se ho gia' aperto la porta gli faccio dire una frase
		if ((init.Obj[o2Hp2G].flags & EXTRA) || (init.Obj[oXT16p17].flags & ON)) {
			PlayerSpeak(game, init.Obj[oADDTEXTOBJ].text[0]);
			return;
		}
	}

	switch (UseWith[USED]) {
	case i41OGGETTO:
		if (UseWith[WITH] == ocCUSTODE)
			Event(EventClass::MC_DIALOG, ME_DIALOGSTART, MP_DEFAULT, dR002, 0, 0, nullptr, nullptr, nullptr);
		else
			sent = TRUE;
		break;

	case i14OCCHIALI:
		if (UseWith[WITH] == ocCUSTODE)
			Event(EventClass::MC_DIALOG, ME_DIALOGSTART, MP_DEFAULT, dR003, 0, 0, nullptr, nullptr, nullptr);
		else
			sent = TRUE;
		break;

	case i1cMEDAGLIONE:
		if (UseWith[WITH] == o1CBOCCA)
			PlayerSpeak(game, init.InvObj[i1cMEDAGLIONE].text[1]);
		else
			sent = TRUE;
		break;

	case i34FIALAA:
		if (init.Obj[o25FORNOAP].flags & EXTRA2) {
			if (UseWith[WITH] == o25FORNOAP)
				sa = a257;
			else if (UseWith[WITH] == o25CONGELATORE2AP)
				sa = a2524;
		} else
			sent = TRUE;
		break;

	case i34FIALAB:
		if (init.Obj[o25FORNOAP].flags & EXTRA2) {
			if (UseWith[WITH] == o25FORNOAP)
				sa = a258;
			else if (UseWith[WITH] == o25CONGELATORE2AP)
				sa = a2525;
		} else
			sent = TRUE;
		break;

	case i25MEDAGLIONI4:
		if (UseWith[WITH] == o1CBOCCA) {
			sa = a1C2;
			Event(EventClass::MC_DIALOG, ME_DIALOGSTART, MP_WAIT_ANIM, dR1C1, 0, 0, nullptr, nullptr, nullptr);
		} else
			sent = TRUE;
		break;
	case i1cMEDAGLIONI3:
		if (UseWith[WITH] == o1CBOCCA) {
			sa = a1C2;
			Event(EventClass::MC_DIALOG, ME_DIALOGSTART, MP_WAIT_ANIM, dR1C2, 0, 0, nullptr, nullptr, nullptr);
		} else
			sent = TRUE;
		break;

	case i24COLTELLO:

		if ((UseWith[WITH] == o2GFILTRODXCH) || (UseWith[WITH] == o2GFILTROSXCH)) {
			switch (UseWith[WITH]) {
			case o2GFILTRODXCH:
				if (init.Obj[o2GOFF].flags & ON) {
					sa = a2G2;
					init.Obj[o2GOFF].anim[DARRELL] = init.Obj[o2GOFF].anim[VICTORIA] = aNULL;
					init.Obj[o2GOFF].flags |= EXTRA;
				} else PlayerSpeak(game,  init.InvObj[UseWith[USED]].text[1]);

				break;

			case o2GFILTROSXCH:
				if (init.Obj[o2GOFF].flags & ON) {
					sa = a2G3;
					init.Obj[o2GOFF].anim[DARRELL] = init.Obj[o2GOFF].anim[VICTORIA] = aNULL;
					init.Obj[o2GOFF].flags |= EXTRA;
				} else PlayerSpeak(game,  init.InvObj[UseWith[USED]].text[1]);
				break;
			}//switch
		} else
			sent = TRUE;
		break;

	case i25MAZZODUECHIAVI:
		if (UseWith[WITH] == o2Hp2G)
			sa = a2H6;
		else if (UseWith[WITH] == oXT16PORTA)
			sa = a162;
		else
			sent = TRUE;
		break;

	case i2iDETERSIVO:
		if (UseWith[WITH] == o2GFILTRODXAP) {
			sa = a2G4;
			init.InvObj[UseWith[USED]].flags &= ~USEWITH;
			init.InvObj[UseWith[USED]].flags |= USE;
			init.InvObj[i2iDETERSIVO].flags |= EXTRA;
		} else if (UseWith[WITH] == o2GFILTROSXAP) {
			sa = a2G5;
			init.InvObj[UseWith[USED]].flags &= ~USEWITH;
			init.InvObj[UseWith[USED]].flags |= USE;
			init.InvObj[i2iDETERSIVO].flags |= EXTRA;
		}
		break;

	case i2lBOTTIGLIAVINO:
		if (UseWith[WITH] == ocCUOCO) {
			if (!(init.InvObj[i2lBOTTIGLIAVINO].flags & EXTRA)) {
				PlayerSpeak(game, init.InvObj[i2lBOTTIGLIAVINO_NOCUOCO].text[0]);
				break;
			}

			if (!(init.InvObj[i2lBOTTIGLIAVINO].flags & EXTRA2) &&
			        (((CurPlayer == DARRELL) && (PlayerStand[VICTORIA].roomName.equalsIgnoreCase("r25-a.t3d"))) ||
			         ((CurPlayer == VICTORIA) && (PlayerStand[DARRELL].roomName.equalsIgnoreCase("r25-a.t3d"))))) {
				init.Dialog[dR009].flags &= ~(DIALOG_RTV | DIALOG_RTV2 | DIALOG_RTV3);
				if (init.Dialog[dR009].flags & DIALOG_DONE)
					init.Dialog[dR009].flags |= DIALOG_RTV2;
				else
					init.Dialog[dR009].flags |= DIALOG_RTV3;
				init.InvObj[i2lBOTTIGLIAVINO].flags |= EXTRA2;
			}
			Event(EventClass::MC_DIALOG, ME_DIALOGSTART, MP_DEFAULT, dR009, 0, 0, nullptr, nullptr, nullptr);
		}
		break;

	case i34LASTRE:
		if (UseWith[WITH] == ocSUPERVISORE)
			Event(EventClass::MC_DIALOG, ME_DIALOGSTART, MP_DEFAULT, dR007, 0, 0, nullptr, nullptr, nullptr);
		else
			sent = TRUE;
		break;

	case i22BRACCIALE:
		if (UseWith[WITH] == ocSERVETTA) {
			if (CurPlayer == VICTORIA) {
//					PlayerSpeak( init.InvObj[i22BRACCIALE].text[1] );
				Event(EventClass::MC_DIALOG, ME_DIALOGSTART, MP_DEFAULT, dR001, 0, 0, nullptr, nullptr, nullptr);
			} else {
				PlayerSpeak(game, init.InvObj[i22BRACCIALE].text[0]);
			}
		}
		break;

	case i29FOTOJUDE1:
		if ((UseWith[WITH] == ocSUPERVISORE) && (init.InvObj[i2aSANGUE].flags & EXTRA)) {
			if (CurPlayer == DARRELL)
				PlayerSpeak(game, init.InvObj[i29FOTOJUDE1].text[0]);

			if (CurPlayer == VICTORIA)
				Event(EventClass::MC_DIALOG, ME_DIALOGSTART, MP_DEFAULT, dR291, 0, 0, nullptr, nullptr, nullptr);
		} else   sent = TRUE;
		break;

	case i29STAMPAINGRANAGGIO:
		if (UseWith[WITH] == ocCUSTODE)
			Event(EventClass::MC_DIALOG, ME_DIALOGSTART, MP_DEFAULT, dR006, 0, 0, nullptr, nullptr, nullptr);
		else
			sent = TRUE;
		break;

	case i31ANELLOBRONZO:
		if (UseWith[WITH] == ocCUSTODE)
			Event(EventClass::MC_DIALOG, ME_DIALOGSTART, MP_DEFAULT, dR004, 0, 0, nullptr, nullptr, nullptr);
		else
			sent = TRUE;
		break;

	case i1aLUCCHETTO:
		if (UseWith[WITH] == oXT1AFINESTRA) {
//				morte Cacciatore
			Event(EventClass::MC_DIALOG, ME_DIALOGSTART, MP_DEFAULT, dR1A5, 0, 0, nullptr, nullptr, nullptr);
		} else
			sent = TRUE;
		break;

	case i48CHIAVIMANETTE:
		if (UseWith[WITH] == o48MANETTE)
			Event(EventClass::MC_DIALOG, ME_DIALOGSTART, MP_DEFAULT, dR483, 0, 0, nullptr, nullptr, nullptr);
		else
			sent = TRUE;
		break;

	case i2aSIRINGAVUOTA:
		if (UseWith[WITH] == o2AACIDO)
			sa = a2A14;
		else if (UseWith[WITH] == o2MACIDO)
			sa = a2M9;
		else
			sent = TRUE;
		break;

	case i19CRISTALLO:
		if (UseWith[WITH] == o31ALLOGGIAMENTO)
			sa = a3114;
		else if (UseWith[WITH] == o39RETTANGOLO)
			sa = a396;
		else
			sent = TRUE;
		break;

	case i39CRISTALLOATTIVATO:
		if (UseWith[WITH] == o31ALLOGGIAMENTO)
			sa = a3116;
		else if (UseWith[WITH] == o39RETTANGOLO)
			sa = a3913;
		else
			sent = TRUE;
		break;

	case i2dSESTERZO:
		if ((UseWith[WITH] == o36PIATTODX) && !(init.Obj[o36INCENSODX].flags & ON)) {
			if (init.Obj[o36INCENSOSX].flags & ON)
				sa = a36MSDA;
			else
				sa = a36MSDM;
		} else if ((UseWith[WITH] == o36PIATTOSX) && !(init.Obj[o36INCENSOSX].flags & ON)) {
			if (init.Obj[o36INCENSODX].flags & ON)
				sa = a36MSSA;
			else
				sa = a36MSSM;
		} else if (!(init.Obj[o3BCOPPA].flags & EXTRA)) {
			if (UseWith[WITH] == o3BCOPPABRACCIALI)
				sa = a3B2;
			else if (UseWith[WITH] == o3BCOPPA)
				sa = a3B3;
		} else
			sent = TRUE;
		break;
	case i36BUSTA1DOSEA:
	case i36BUSTA1DOSEB:
		if ((UseWith[WITH] == o36PIATTODX) && (init.Obj[o36INCENSODX].flags & ON)) {
			if (init.Obj[o36SESTERZOSX].flags & ON)
				sa = a36PBDM;
			else
				sa = a36PBDB;

			if (UseWith[USED] == i36BUSTA1DOSEA)
				ReplaceIcon(init, i36BUSTA1DOSEA, i36BUSTA2DOSIA);
			else
				ReplaceIcon(init, i36BUSTA1DOSEB, i36BUSTA2DOSIB);
		} else if ((UseWith[WITH] == o36PIATTOSX) && (init.Obj[o36INCENSOSX].flags & ON)) {
			if (init.Obj[o36SESTERZODX].flags & ON)
				sa = a36PBSM;
			else
				sa = a36PBSB;

			if (UseWith[USED] == i36BUSTA1DOSEA)
				ReplaceIcon(init, i36BUSTA1DOSEA, i36BUSTA2DOSIA);
			else
				ReplaceIcon(init, i36BUSTA1DOSEB, i36BUSTA2DOSIB);
		} else if (!(init.Obj[o3BCOPPA].flags & EXTRA)) {
			if ((UseWith[WITH] == o3BCOPPABRACCIALI) && !(init.Obj[o3B2DOSIBRACCIALI].flags & ON)) {
				if (UseWith[USED] == i36BUSTA1DOSEA)  ReplaceIcon(init, i36BUSTA1DOSEA, i2rBUSTAVUOTAA);
				else  ReplaceIcon(init, i36BUSTA1DOSEB, i2rBUSTAVUOTAB);

				if (init.Obj[o3B1DOSEBRACCIALI].flags & ON) {
					init.Obj[o3B1DOSEBRACCIALI].flags &= ~ON;
					init.Obj[o3B2DOSIBRACCIALI].flags |= ON;

					init.Obj[o3BCOPPABRACCIALI].examine[CurPlayer] = init.Obj[o3BCOPPABRACCIALI].examine[CurPlayer ^ 1] = init.Obj[o3BCOPPABRACCIALI].text[4];
				} else {
					init.Obj[o3B1DOSEBRACCIALI].flags |= ON;
					init.Obj[o3B2DOSIBRACCIALI].flags &= ~ON;

					init.Obj[o3BCOPPABRACCIALI].examine[CurPlayer] = init.Obj[o3BCOPPABRACCIALI].examine[CurPlayer ^ 1] = init.Obj[o3BCOPPABRACCIALI].text[3];
				}

				UpdateObjMesh(init, o3B1DOSEBRACCIALI);
				UpdateObjMesh(init, o3B2DOSIBRACCIALI);

				sa = a3B4;
			} else if ((UseWith[WITH] == o3BCOPPA) && !(init.Obj[o3B2DOSI].flags & ON)) {
				if (UseWith[USED] == i36BUSTA1DOSEA)  ReplaceIcon(init, i36BUSTA1DOSEA, i2rBUSTAVUOTAA);
				else  ReplaceIcon(init, i36BUSTA1DOSEB, i2rBUSTAVUOTAB);

				if (init.Obj[o3B1DOSE].flags & ON) {
					init.Obj[o3B1DOSE].flags &= ~ON;
					init.Obj[o3B2DOSI].flags |= ON;

					init.Obj[o3BCOPPA].examine[CurPlayer] = init.Obj[o3BCOPPA].examine[CurPlayer ^ 1] = init.Obj[o3BCOPPA].text[4];
				} else {
					init.Obj[o3B1DOSE].flags |= ON;
					init.Obj[o3B2DOSI].flags &= ~ON;

					init.Obj[o3BCOPPA].examine[CurPlayer] = init.Obj[o3BCOPPA].examine[CurPlayer ^ 1] = init.Obj[o3BCOPPA].text[3];
				}

				UpdateObjMesh(init, o3B1DOSE);
				UpdateObjMesh(init, o3B2DOSI);

				sa = a3B5;
			} else
				sent = TRUE;
		} else
			sent = TRUE;
		break;
	case i36BUSTA2DOSIA:
	case i36BUSTA2DOSIB:
		if (!(init.Obj[o3BCOPPA].flags & EXTRA)) {
			if ((UseWith[WITH] == o3BCOPPABRACCIALI) && !(init.Obj[o3B1DOSEBRACCIALI].flags & ON) && !(init.Obj[o3B2DOSIBRACCIALI].flags & ON)) {
				if (UseWith[USED] == i36BUSTA2DOSIA)  ReplaceIcon(init, i36BUSTA2DOSIA, i2rBUSTAVUOTAA);
				else  ReplaceIcon(init, i36BUSTA2DOSIB, i2rBUSTAVUOTAB);

				init.Obj[o3B2DOSIBRACCIALI].flags |= ON;
				UpdateObjMesh(init, o3B2DOSIBRACCIALI);

				sa = a3B6;
			} else if ((UseWith[WITH] == o3BCOPPA) && !(init.Obj[o3B1DOSE].flags & ON) && !(init.Obj[o3B2DOSI].flags & ON)) {
				if (UseWith[USED] == i36BUSTA2DOSIA)  ReplaceIcon(init, i36BUSTA2DOSIA, i2rBUSTAVUOTAA);
				else  ReplaceIcon(init, i36BUSTA2DOSIB, i2rBUSTAVUOTAB);

				init.Obj[o3B2DOSI].flags |= ON;
				UpdateObjMesh(init, o3B2DOSI);
				sa = a3B7;
			} else
				sent = TRUE;
		} else
			sent = TRUE;
		break;
	case i2bSACCHETTOINCENSO:
		if (UseWith[WITH] == o36PIATTODX) {
			if (init.Obj[o36SESTERZOSX].flags & ON) {                                            // se a sx sesterzo
				if (!(init.Obj[o36INCENSODX].flags & ON))                                         // ma a dx non incenso
					sa = a36MIDA;                                                           // lo mette
				else                                                                        // altrimenti
					sa = a36PIDM;                                                           // lo prende
			} else if (init.Obj[o36INCENSODX].flags & ON)                                        // se a dx incenso
				sa = a36PIDB;                                                           // lo prende
		} else if (UseWith[WITH] == o36PIATTOSX) {
			if (init.Obj[o36SESTERZODX].flags & ON) {                                            // se a dx sesterzo
				if (!(init.Obj[o36INCENSOSX].flags & ON))                                         // ma a sx non incenso
					sa = a36MISA;                                                           // lo mette
				else                                                                        // altrimenti
					sa = a36PISM;                                                           // lo prende
			} else if (init.Obj[o36INCENSOSX].flags & ON)                                        // se a sx incenso
				sa = a36PISB;                                                           // lo prende
		} else if (!(init.Obj[o3BCOPPA].flags & EXTRA)) {
			if ((UseWith[WITH] == o3BCOPPABRACCIALI) && (init.Obj[o3B2DOSIBRACCIALI].flags & ON))
				sa = a3B15;
			else if ((UseWith[WITH] == o3BCOPPABRACCIALI) && (init.Obj[o3B1DOSEBRACCIALI].flags & ON))
				sa = a3B13;
			else if ((UseWith[WITH] == o3BCOPPA) && (init.Obj[o3B2DOSI].flags & ON))
				sa = a3B16;
			else if ((UseWith[WITH] == o3BCOPPA) && (init.Obj[o3B1DOSE].flags & ON))
				sa = a3B14;
			else
				sent = TRUE;

			if (sa) { //se ha fatto l'azione di raccoglie controllo il testo delle coppe
				if (UseWith[WITH] == o3BCOPPABRACCIALI) {
					if (init.Obj[o3BSESTERZOBRACCIALI].flags & ON)
						init.Obj[o3BCOPPABRACCIALI].examine[CurPlayer] = init.Obj[o3BCOPPABRACCIALI].examine[CurPlayer ^ 1] = init.Obj[o3BCOPPABRACCIALI].text[2];
					else
						init.Obj[o3BCOPPABRACCIALI].examine[CurPlayer] = init.Obj[o3BCOPPABRACCIALI].examine[CurPlayer ^ 1] = init.Obj[o3BCOPPABRACCIALI].text[0];

					init.Obj[o3BCOPPABRACCIALI].action[CurPlayer] = init.Obj[o3BCOPPABRACCIALI].action[CurPlayer ^ 1] = 0;
				} else if (UseWith[WITH] == o3BCOPPA) {
					if (init.Obj[o3BSESTERZO].flags & ON)
						init.Obj[o3BCOPPA].examine[CurPlayer] = init.Obj[o3BCOPPA].examine[CurPlayer ^ 1] = init.Obj[o3BCOPPA].text[2];
					else
						init.Obj[o3BCOPPA].examine[CurPlayer] = init.Obj[o3BCOPPA].examine[CurPlayer ^ 1] = init.Obj[o3BCOPPA].text[0];

					init.Obj[o3BCOPPA].action[CurPlayer] = init.Obj[o3BCOPPA].action[CurPlayer ^ 1] = 0;
				}
			}//if sa
		}

		else
			sent = TRUE;
		break;
	case i2rBUSTAVUOTAA:
		if ((UseWith[WITH] == o36PIATTODX) && (init.Obj[o36INCENSODX].flags & ON)) {
			if (init.Obj[o36SESTERZOSX].flags & ON)
				sa = a36PBDM;
			else
				sa = a36PBDB;

			ReplaceIcon(init, i2rBUSTAVUOTAA, i36BUSTA1DOSEA);
		} else if ((UseWith[WITH] == o36PIATTOSX) && (init.Obj[o36INCENSOSX].flags & ON)) {
			if (init.Obj[o36SESTERZODX].flags & ON)
				sa = a36PBSM;
			else
				sa = a36PBSB;

			ReplaceIcon(init, i2rBUSTAVUOTAA, i36BUSTA1DOSEA);
		} else if (!(init.Obj[o3BCOPPA].flags & EXTRA)) {
			if ((UseWith[WITH] == o3BCOPPABRACCIALI) && (init.Obj[o3B2DOSIBRACCIALI].flags & ON)) {
				sa = a3B11;
				ReplaceIcon(init, (uint8)UseWith[USED], i36BUSTA2DOSIA);
			} else if ((UseWith[WITH] == o3BCOPPABRACCIALI) && (init.Obj[o3B1DOSEBRACCIALI].flags & ON)) {
				sa = a3B9;
				ReplaceIcon(init, (uint8)UseWith[USED], i36BUSTA1DOSEA);
			} else if ((UseWith[WITH] == o3BCOPPA) && (init.Obj[o3B2DOSI].flags & ON)) {
				sa = a3B12;
				ReplaceIcon(init, (uint8)UseWith[USED], i36BUSTA2DOSIA);
			} else if ((UseWith[WITH] == o3BCOPPA) && (init.Obj[o3B1DOSE].flags & ON)) {
				sa = a3B10;
				ReplaceIcon(init, (uint8)UseWith[USED], i36BUSTA1DOSEA);
			} else
				sent = TRUE;

			if (sa) { //se ha fatto l'azione di raccoglie controllo il testo delle coppe
				if (UseWith[WITH] == o3BCOPPABRACCIALI) {
					if (init.Obj[o3BSESTERZOBRACCIALI].flags & ON)
						init.Obj[o3BCOPPABRACCIALI].examine[CurPlayer] = init.Obj[o3BCOPPABRACCIALI].examine[CurPlayer ^ 1] = init.Obj[o3BCOPPABRACCIALI].text[2];
					else
						init.Obj[o3BCOPPABRACCIALI].examine[CurPlayer] = init.Obj[o3BCOPPABRACCIALI].examine[CurPlayer ^ 1] = init.Obj[o3BCOPPABRACCIALI].text[0];

					init.Obj[o3BCOPPABRACCIALI].action[CurPlayer] = init.Obj[o3BCOPPABRACCIALI].action[CurPlayer ^ 1] = 0;
				} else if (UseWith[WITH] == o3BCOPPA) {
					if (init.Obj[o3BSESTERZO].flags & ON)
						init.Obj[o3BCOPPA].examine[CurPlayer] = init.Obj[o3BCOPPA].examine[CurPlayer ^ 1] = init.Obj[o3BCOPPA].text[2];
					else
						init.Obj[o3BCOPPA].examine[CurPlayer] = init.Obj[o3BCOPPA].examine[CurPlayer ^ 1] = init.Obj[o3BCOPPA].text[0];

					init.Obj[o3BCOPPA].action[CurPlayer] = init.Obj[o3BCOPPA].action[CurPlayer ^ 1] = 0;
				}
			}//if sa
		} else
			sent = TRUE;
		break;
	case i2rBUSTAVUOTAB:
		if ((UseWith[WITH] == o36PIATTODX) && (init.Obj[o36INCENSODX].flags & ON)) {
			if (init.Obj[o36SESTERZOSX].flags & ON)
				sa = a36PBDM;
			else
				sa = a36PBDB;

			ReplaceIcon(init, i2rBUSTAVUOTAB, i36BUSTA1DOSEB);
		} else if ((UseWith[WITH] == o36PIATTOSX) && (init.Obj[o36INCENSOSX].flags & ON)) {
			if (init.Obj[o36SESTERZODX].flags & ON)
				sa = a36PBSM;
			else
				sa = a36PBSB;

			ReplaceIcon(init, i2rBUSTAVUOTAB, i36BUSTA1DOSEB);
		} else if (!(init.Obj[o3BCOPPA].flags & EXTRA)) {
			if ((UseWith[WITH] == o3BCOPPABRACCIALI) && (init.Obj[o3B2DOSIBRACCIALI].flags & ON)) {
				sa = a3B11;
				ReplaceIcon(init, (uint8)UseWith[USED], i36BUSTA2DOSIB);
			} else if ((UseWith[WITH] == o3BCOPPABRACCIALI) && (init.Obj[o3B1DOSEBRACCIALI].flags & ON)) {
				sa = a3B9;
				ReplaceIcon(init, (uint8)UseWith[USED], i36BUSTA1DOSEB);
			} else if ((UseWith[WITH] == o3BCOPPA) && (init.Obj[o3B2DOSI].flags & ON)) {
				sa = a3B12;
				ReplaceIcon(init, (uint8)UseWith[USED], i36BUSTA2DOSIB);
			} else if ((UseWith[WITH] == o3BCOPPA) && (init.Obj[o3B1DOSE].flags & ON)) {
				sa = a3B10;
				ReplaceIcon(init, (uint8)UseWith[USED], i36BUSTA1DOSEB);
			} else
				sent = TRUE;

			if (sa) { //se ha fatto l'azione di raccoglie controllo il testo delle coppe
				if (UseWith[WITH] == o3BCOPPABRACCIALI) {
					if (init.Obj[o3BSESTERZOBRACCIALI].flags & ON)
						init.Obj[o3BCOPPABRACCIALI].examine[CurPlayer] = init.Obj[o3BCOPPABRACCIALI].examine[CurPlayer ^ 1] = init.Obj[o3BCOPPABRACCIALI].text[2];
					else
						init.Obj[o3BCOPPABRACCIALI].examine[CurPlayer] = init.Obj[o3BCOPPABRACCIALI].examine[CurPlayer ^ 1] = init.Obj[o3BCOPPABRACCIALI].text[0];

					init.Obj[o3BCOPPABRACCIALI].action[CurPlayer] = init.Obj[o3BCOPPABRACCIALI].action[CurPlayer ^ 1] = 0;
				} else if (UseWith[WITH] == o3BCOPPA) {
					if (init.Obj[o3BSESTERZO].flags & ON)
						init.Obj[o3BCOPPA].examine[CurPlayer] = init.Obj[o3BCOPPA].examine[CurPlayer ^ 1] = init.Obj[o3BCOPPA].text[2];
					else
						init.Obj[o3BCOPPA].examine[CurPlayer] = init.Obj[o3BCOPPA].examine[CurPlayer ^ 1] = init.Obj[o3BCOPPA].text[0];

					init.Obj[o3BCOPPA].action[CurPlayer] = init.Obj[o3BCOPPA].action[CurPlayer ^ 1] = 0;
				}
			}//if sa
		} else
			sent = TRUE;
		break;

	case i46STELLA1:
		if (UseWith[WITH] == o45ALLOGGIAMENTO) {
			sa = a454;
			if (!(init.Obj[o45ALLOGGIAMENTO].flags & EXTRA2)) {
				IncCurTime(game, 10);
				init.Obj[o45ALLOGGIAMENTO].flags |= EXTRA2;
			}
		} else if (UseWith[WITH] == o46CERABRACERE)
			sa = a4619;
		else if ((UseWith[WITH] == o49INCAVODX) && !(init.Obj[o49STELLADXCIA].flags & ON)) {
			if (init.Obj[o49STELLASXCIA].flags & ON)
				sa = a493b;
			else
				sa = a493a;
		} else if ((UseWith[WITH] == o49INCAVOSX) && !(init.Obj[o49STELLASXCIA].flags & ON)) {
			if (init.Obj[o49STELLADXCIA].flags & ON)
				sa = a494b;
			else
				sa = a494a;
		} else
			sent = TRUE;
		break;

	case i46STELLA2:
		if ((UseWith[WITH] == o49INCAVODX) && !(init.Obj[o49STELLADX].flags & ON)) {
			if (init.Obj[o49STELLASX].flags & ON)
				sa = a493cb;
			else
				sa = a493ca;
		} else if ((UseWith[WITH] == o49INCAVOSX) && !(init.Obj[o49STELLASX].flags & ON)) {
			if (init.Obj[o49STELLADX].flags & ON)
				sa = a494cb;
			else
				sa = a494ca;
		} else
			sent = TRUE;
		break;

	default:
		if (init.InvObj[UseWith[USED]].uwobj && (init.InvObj[UseWith[USED]].uwobj == UseWith[WITH]) && init.InvObj[UseWith[USED]].anim2[CurPlayer])
			sa = init.InvObj[UseWith[USED]].anim2[CurPlayer];
		else
			sent = TRUE;
		break;
	}

	if (sent)
		if (!((bUseWith & UW_WITHI) && (UseWith[USED] == UseWith[WITH])))
			PlayerSpeak(game, init.InvObj[UseWith[USED]].action[CurPlayer]);

	if (sa)
		StartAnim(game, sa);
}

} // End of namespace Watchmaker
