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
#include <stdlib.h>
#include <string.h>
#include "trecision/nl/lib/addtype.h"
#include "trecision/nl/sysdef.h"
#include "trecision/nl/struct.h"
#include "trecision/nl/message.h"
#include "trecision/nl/extern.h"
#include "trecision/nl/define.h"

namespace Trecision {

int maxmesg, maxmesh, maxmesa;

/*-------------------------------------------------------------------------*/
/*                               GETMESSAGE           					   */
/*-------------------------------------------------------------------------*/
LLBOOL GetMessage(pqueue *lq) {
	if (!(lq->len)) return true;

	TheMessage = lq->event[lq->head++];
	if (lq->head == MAXMESSAGE) lq->head = 0;
	lq->len--;

	return false;
}

/*-------------------------------------------------------------------------*/
/*                            INITMESSAGESYSTEM          				   */
/*-------------------------------------------------------------------------*/
void InitMessageSystem() {
	InitQueue(&Game);
	InitQueue(&Anim);
	InitQueue(&Homo);
	for (uint8 i = 0; i < MAXMESSAGE; i++) {
		Game.event[i] = &GameMessage[i];
		Homo.event[i] = &HomoMessage[i];
		Anim.event[i] = &AnimMessage[i];
	}
}

/*-------------------------------------------------------------------------*/
/*                                INITQUEUE           					   */
/*-------------------------------------------------------------------------*/
void InitQueue(pqueue *lq) {
	lq->head = 0;
	lq->tail = 0;
	lq->len  = 0;
}

/*-------------------------------------------------------------------------*/
/*                                  EVENT           					   */
/*-------------------------------------------------------------------------*/
void doEvent(uint8 cls,  uint8 event,  uint8 priority,
			 uint16 wparam1, uint16 wparam2,
			 uint8 bparam, uint32 lparam) {
	pqueue *lq;

	if (cls <= CLASS_GAME)   lq = &Game;
	else if (cls <= CLASS_ANIM) lq = &Anim;
	else        lq = &Homo;

	if (lq->len >= MAXMESSAGE)
		return;

	message *lm = lq->event[lq->tail++];

	lm->cls  = cls;
	lm->event  = event;
	lm->priority = priority;
	lm->wparam1  = wparam1;
	lm->wparam2  = wparam2;
	lm->bparam  = bparam;
	lm->lparam  = lparam;
	lm->timestamp = TheTime;

	if (lq->tail == MAXMESSAGE) lq->tail = 0;
	lq->len++;

	if (lq == &Game)
		if (lq->len > maxmesg) maxmesg = lq->len;
	if (lq == &Anim)
		if (lq->len > maxmesa) maxmesa = lq->len;
	if (lq == &Homo)
		if (lq->len > maxmesh) maxmesh = lq->len;

	OrderEvent(lq);
}

/*-------------------------------------------------------------------------*/
/*                                SCHEDULER           					   */
/*-------------------------------------------------------------------------*/
void Scheduler() {
	static uint8 token = CLASS_HOMO;
	static uint8 Counter;
	LLBOOL retry = true;

	while (retry) {
		retry = false;
		switch (token) {

		case CLASS_GAME:
			if (Counter++ <= 30) {
				token = CLASS_ANIM;
				if (GetMessage(&Game)) TheMessage = &idlemessage;
			} else {
				Counter = 0;
				TheMessage = &idlemessage;
			}
			break;

		case CLASS_ANIM:
			token = CLASS_HOMO;
			if (GetMessage(&Anim)) retry = true;
			break;

		case CLASS_HOMO:
			token = CLASS_GAME;
			if ((SemPaintHomo) || (GetMessage(&Homo))) retry = true;
			break;

		}
	}
}

/*-------------------------------------------------------------------------*/
/*                            PROCESSTHEMESSAGE          				   */
/*-------------------------------------------------------------------------*/
void ProcessTheMessage() {
SUPEREVENT:
	switch (TheMessage->cls) {
	case MC_HOMO:
		doHomo();
		break;

	case MC_IDLE:
		doIdle();
		break;

	case MC_MOUSE:
		doMouse();
		break;

	case MC_SYSTEM:
		doSystem();
		break;

//F		case MC_ANIMATION: doAnimation(); break;

	case MC_INVENTORY:
		doInventory();
		break;

	case MC_ACTION:
		doAction();
		break;

	case MC_STRING:
		doString();
		break;

	case MC_DOING:
		doDoing();
		break;

	case MC_DIALOG:
		doDialog();
		break;

	case MC_SCRIPT:
		doScript();
		break;
	}

	if (SuperEventActivate) {
		SuperEventActivate = false;
		goto SUPEREVENT;
	}
}

/*-------------------------------------------------------------------------*/
/*                               ORDEREVENT           					   */
/*-------------------------------------------------------------------------*/
void OrderEvent(pqueue *lq) {
	uint8 pos;

#define PredEvent(i)       (((i)==0)?MAXMESSAGE-1:((i)-1))

	for (pos = PredEvent(lq->tail); pos != lq->head; pos = PredEvent(pos))
		if (lq->event[pos]->priority > lq->event[PredEvent(pos)]->priority) {
			if (lq->event[pos]->priority < MP_HIGH) lq->event[pos]->priority++;
			SwapMessage(lq->event[pos], lq->event[PredEvent(pos)]);
		}
}

/*-------------------------------------------------------------------------*/
/*                               TESTEMPTYQUEUE          				   */
/*-------------------------------------------------------------------------*/
LLBOOL TestEmptyQueue(pqueue *lq, uint8 cls) {
	uint8 pos;

	for (pos = lq->head; pos != lq->tail; pos = (pos == MAXMESSAGE - 1) ? 0 : pos + 1)
		if (lq->event[pos]->cls != cls) return false;

	return true;
}

/*-------------------------------------------------------------------------*/
/*                       TESTEMPTYHOMOQUEUE4SCRIPT          			   */
/*-------------------------------------------------------------------------*/
LLBOOL TestEmptyHomoQueue4Script(pqueue *lq) {
	uint8 pos;

	for (pos = lq->head; pos != lq->tail; pos = (pos == MAXMESSAGE - 1) ? 0 : pos + 1)

		/*		if (!(( lq->event[pos]->cls == MC_HOMO) &&
					(( lq->event[pos]->event == ME_HOMOACTION) ||
			( lq->event[pos]->event == ME_HOMOCONTINUEACTION)) &&
			( lq->event[pos]->lparam == false) &&
			( lq->event[pos]->wparam1 > DEFAULTACTIONS)))
		*/
		if (((lq->event[pos]->cls == MC_HOMO) && (lq->event[pos]->event == ME_HOMOACTION)) ||
				((lq->event[pos]->cls == MC_HOMO) && (lq->event[pos]->event == ME_HOMOGOTO)) ||
				((lq->event[pos]->cls == MC_HOMO) && (lq->event[pos]->event == ME_HOMOGOTOACTION)) ||
				((lq->event[pos]->cls == MC_HOMO) && (lq->event[pos]->event == ME_HOMOGOTOEXAMINE)) ||
				((lq->event[pos]->cls == MC_HOMO) && (lq->event[pos]->event == ME_HOMOCONTINUEACTION)))
			return false;

//	true quando:
//	1) la coda e' vuota
//	2) oppure c'e' una azione particolare in corso

	return true;
}

/*-------------------------------------------------------------------------*/
/*                               SWAPMESSAGE          					   */
/*-------------------------------------------------------------------------*/
void SwapMessage(message *m1, message *m2) {
	message *tmp = m1;
	m1 = m2;
	m2 = tmp;
}

} // End of namespace Trecision
