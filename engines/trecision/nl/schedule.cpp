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
#include "trecision/trecision.h"

namespace Trecision {

int maxmesg, maxmesh, maxmesa;

/*-------------------------------------------------------------------------*/
/*                               GETMESSAGE           					   */
/*-------------------------------------------------------------------------*/
bool GetMessage(MessageQueue *lq) {
	if (!(lq->len))
		return true;

	g_vm->TheMessage = lq->event[lq->head++];
	if (lq->head == MAXMESSAGE)
		lq->head = 0;
	lq->len--;

	return false;
}

/*-------------------------------------------------------------------------*/
/*                            INITMESSAGESYSTEM          				   */
/*-------------------------------------------------------------------------*/
void InitMessageSystem() {
	InitQueue(&g_vm->_gameQueue);
	InitQueue(&g_vm->_animQueue);
	InitQueue(&g_vm->_characterQueue);
	for (uint8 i = 0; i < MAXMESSAGE; i++) {
		g_vm->_gameQueue.event[i] = &g_vm->_gameMsg[i];
		g_vm->_characterQueue.event[i] = &g_vm->_characterMsg[i];
		g_vm->_animQueue.event[i] = &g_vm->_animMsg[i];
	}
}

/*-------------------------------------------------------------------------*/
/*                                INITQUEUE           					   */
/*-------------------------------------------------------------------------*/
void InitQueue(MessageQueue *lq) {
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
	MessageQueue *lq;

	if (cls <= CLASS_GAME)
		lq = &g_vm->_gameQueue;
	else if (cls <= CLASS_ANIM)
		lq = &g_vm->_animQueue;
	else
		lq = &g_vm->_characterQueue;

	if (lq->len >= MAXMESSAGE)
		return;

	Message *lm = lq->event[lq->tail++];

	lm->cls  = cls;
	lm->event  = event;
	lm->priority = priority;
	lm->wparam1  = wparam1;
	lm->wparam2  = wparam2;
	lm->bparam  = bparam;
	lm->lparam  = lparam;
	lm->timestamp = TheTime;

	if (lq->tail == MAXMESSAGE)
		lq->tail = 0;
	lq->len++;

	if (lq == &g_vm->_gameQueue)
		if (lq->len > maxmesg)
			maxmesg = lq->len;
	if (lq == &g_vm->_animQueue)
		if (lq->len > maxmesa)
			maxmesa = lq->len;
	if (lq == &g_vm->_characterQueue)
		if (lq->len > maxmesh)
			maxmesh = lq->len;

	OrderEvent(lq);
}

/*-------------------------------------------------------------------------*/
/*                                SCHEDULER           					   */
/*-------------------------------------------------------------------------*/
void Scheduler() {
	static uint8 token = CLASS_CHAR;
	static uint8 Counter;
	bool retry = true;

	while (retry) {
		retry = false;
		switch (token) {
		case CLASS_GAME:
			if (Counter++ <= 30) {
				token = CLASS_ANIM;
				if (GetMessage(&g_vm->_gameQueue))
					g_vm->TheMessage = &g_vm->_idleMsg;
			} else {
				Counter = 0;
				g_vm->TheMessage = &g_vm->_idleMsg;
			}
			break;

		case CLASS_ANIM:
			token = CLASS_CHAR;
			if (GetMessage(&g_vm->_animQueue))
				retry = true;
			break;

		case CLASS_CHAR:
			token = CLASS_GAME;
			if ((SemPaintHomo) || (GetMessage(&g_vm->_characterQueue)))
				retry = true;
			break;

		}
	}
}

/*-------------------------------------------------------------------------*/
/*                            PROCESSTHEMESSAGE          				   */
/*-------------------------------------------------------------------------*/
void ProcessTheMessage() {
	switch (g_vm->TheMessage->cls) {
	case MC_CHARACTER:
		doCharacter();
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
}

/*-------------------------------------------------------------------------*/
/*                               ORDEREVENT           					   */
/*-------------------------------------------------------------------------*/
void OrderEvent(MessageQueue *lq) {
#define PredEvent(i)       (((i)==0)?MAXMESSAGE-1:((i)-1))

	for (uint8 pos = PredEvent(lq->tail); pos != lq->head; pos = PredEvent(pos))
		if (lq->event[pos]->priority > lq->event[PredEvent(pos)]->priority) {
			if (lq->event[pos]->priority < MP_HIGH)
				lq->event[pos]->priority++;
			SwapMessage(lq->event[pos], lq->event[PredEvent(pos)]);
		}
}

/*-------------------------------------------------------------------------*/
/*                               TESTEMPTYQUEUE          				   */
/*-------------------------------------------------------------------------*/
bool TestEmptyQueue(MessageQueue *lq, uint8 cls) {
	for (uint8 pos = lq->head; pos != lq->tail; pos = (pos == MAXMESSAGE - 1) ? 0 : pos + 1) {
		if (lq->event[pos]->cls != cls)
			return false;
	}

	return true;
}

/*-------------------------------------------------------------------------*/
/*                       TESTEMPTYHOMOQUEUE4SCRIPT          			   */
/*-------------------------------------------------------------------------*/
bool TestEmptyHomoQueue4Script(MessageQueue *lq) {
	for (uint8 pos = lq->head; pos != lq->tail; pos = (pos == MAXMESSAGE - 1) ? 0 : pos + 1) {
		/*		if (!(( lq->event[pos]->cls == MC_CHARACTER) &&
					(( lq->event[pos]->event == ME_CHARACTERACTION) ||
			( lq->event[pos]->event == ME_CHARACTERCONTINUEACTION)) &&
			( lq->event[pos]->lparam == false) &&
			( lq->event[pos]->wparam1 > DEFAULTACTIONS)))
		*/
		if (((lq->event[pos]->cls == MC_CHARACTER) && (lq->event[pos]->event == ME_CHARACTERACTION)) ||
				((lq->event[pos]->cls == MC_CHARACTER) && (lq->event[pos]->event == ME_CHARACTERGOTO)) ||
				((lq->event[pos]->cls == MC_CHARACTER) && (lq->event[pos]->event == ME_CHARACTERGOTOACTION)) ||
				((lq->event[pos]->cls == MC_CHARACTER) && (lq->event[pos]->event == ME_CHARACTERGOTOEXAMINE)) ||
				((lq->event[pos]->cls == MC_CHARACTER) && (lq->event[pos]->event == ME_CHARACTERCONTINUEACTION)))
			return false;
	}

//	true quando:
//	1) la coda e' vuota
//	2) oppure c'e' una azione particolare in corso

	return true;
}

/*-------------------------------------------------------------------------*/
/*                               SWAPMESSAGE          					   */
/*-------------------------------------------------------------------------*/
void SwapMessage(Message *m1, Message *m2) {
	Message *tmp = m1;
	m1 = m2;
	m2 = tmp;
}

} // End of namespace Trecision
