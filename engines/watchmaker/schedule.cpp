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

#include "watchmaker/schedule.h"
#include "watchmaker/sysdef.h"
#include "watchmaker/types.h"
#include "watchmaker/message.h"
#include "watchmaker/utils.h"
#include "watchmaker/struct.h"
#include "watchmaker/globvar.h"
#include "watchmaker/classes/do_camera.h"
#include "watchmaker/classes/do_system.h"
#include "watchmaker/classes/do_player.h"
#include "watchmaker/classes/do_inv.h"
#include "watchmaker/classes/do_anim.h"
#include "watchmaker/classes/do_dialog.h"
#include "watchmaker/classes/do_action.h"
#include "watchmaker/classes/do_mouse.h"
#include "watchmaker/classes/do_string.h"
#include "watchmaker/t2d/t2d.h"

namespace Watchmaker {

void MessageSystem::addWaitingMsgs(uint16 flags) {
	for (int8 a = 0; a < MAXWAITINGMSGS; a++) {
		_waitingMsg[a].flags &= ~flags;
	}

	for (int8 a = 0; a < MAXWAITINGMSGS; a++) {
		if (_waitingMsg[a].classe != EventClass::MC_IDLE && (_waitingMsg[a].flags < MP_WAITA)) {
			_vm->_messageSystem.doEvent(_waitingMsg[a].classe, _waitingMsg[a].event, _waitingMsg[a].flags,
										_waitingMsg[a].wparam1, _waitingMsg[a].wparam2,
										_waitingMsg[a].bparam, &_waitingMsg[a].lparam[0], &_waitingMsg[a].lparam[1], &_waitingMsg[a].lparam[2]);
			_waitingMsg[a].reset();
		}
	}
}

/* -----------------25/09/98 16.33-------------------
 *                  DeleteWaitingMsgs
 * --------------------------------------------------*/
void MessageSystem::deleteWaitingMsgs(uint16 flags) {
	for (int a = 0; a < MAXWAITINGMSGS; a++)
		_waitingMsg[a].flags &= ~flags;

	for (int a = 0; a < MAXWAITINGMSGS; a++)
		if (_waitingMsg[a].classe != EventClass::MC_IDLE && (_waitingMsg[a].flags < MP_WAITA))
			_waitingMsg[a].reset();
}

/* -----------------08/02/99 10.09-------------------
 *                  _GetMessage
 * --------------------------------------------------*/
bool _GetMessage(pqueue *lq) {
	if (!(lq->len)) return false;

	TheMessage = lq->event[lq->head++];
	if (lq->head == MAX_MESSAGES) lq->head = 0;
	lq->len--;

	if (TheMessage->flags & MP_CLEARA)
		_vm->_messageSystem.addWaitingMsgs(MP_WAITA);

	return true;
}

void MessageSystem::initQueue(pqueue *lq) {
	lq->head = 0;
	lq->tail = 0;
	lq->len  = 0;
}

void MessageSystem::init() {
	TheTime = 0;
	initQueue(&Game);
	for (int i = 0; i < MAX_MESSAGES; i++) {
		Game.event[i] = &GameMessage[i];
	}

	for (int i = 0; i < MAXWAITINGMSGS; i++) {
		_waitingMsg[i].reset();
	}
}

const char *eventToString(EventClass classe) {
	switch (classe) {
		case EventClass::MC_IDLE: return "MC_IDLE";
		case EventClass::MC_MOUSE: return "MC_MOUSE";
		case EventClass::MC_SYSTEM: return "MC_SYSTEM";
		case EventClass::MC_CAMERA: return "MC_CAMERA";
		case EventClass::MC_STRING: return "MC_STRING";
		case EventClass::MC_ACTION: return "MC_ACTION";
		case EventClass::MC_PLAYER: return "MC_PLAYER";
		case EventClass::MC_INVENTORY: return "MC_INVENTORY";
		case EventClass::MC_DIALOG: return "MC_DIALOG";
		case EventClass::MC_ANIM: return "MC_ANIM";
		case EventClass::MC_T2D: return "MC_T2D";
		default:
			error("Invalid EventClass %d", (int)classe);
	}
}

void MessageSystem::doEvent(EventClass classe, uint8 event, uint16 flags, int16 wparam1, int16 wparam2,
           uint8 bparam, void *p0, void *p1, void *p2) {
	pqueue *lq;
	message *lm;

	warning("Event(%s, event=%d, flags=%d, wparam1=%d, wparam2=%d, bparam=%d", eventToString(classe), event, flags, wparam1, wparam2, bparam);
	if (classe == EventClass::MC_IDLE && !event)
		return ;

	lq = &Game;

	// se deve andare in attesa
	if (flags >= MP_WAITA) {
		int8 a;
		for (a = 0; a < MAXWAITINGMSGS; a++)
			if (_waitingMsg[a].classe == EventClass::MC_IDLE)
				break;
		if (a < MAXWAITINGMSGS) {
			_waitingMsg[a].classe     = classe;
			_waitingMsg[a].event     = event;
			_waitingMsg[a].flags     = flags;
			_waitingMsg[a].wparam1   = wparam1;
			_waitingMsg[a].wparam2   = wparam2;
			_waitingMsg[a].bparam    = bparam;
			if (p0 != nullptr)
				_waitingMsg[a].lparam[0] = *(int32 *)p0;
			else
				_waitingMsg[a].lparam[0] = 0;
			if (p1 != nullptr)
				_waitingMsg[a].lparam[1] = *(int32 *)p1;
			else
				_waitingMsg[a].lparam[1] = 0;
			if (p2 != nullptr)
				_waitingMsg[a].lparam[2] = *(int32 *)p2;
			else
				_waitingMsg[a].lparam[2] = 0;
			return;
		} else {
			for (a = 0; a < MAXWAITINGMSGS; a++)
				warning("%d: %d %d %d %d", a, _waitingMsg[a].classe, _waitingMsg[a].event, _waitingMsg[a].flags, _waitingMsg[a].lparam[1]);

			warning("ERRORE! CODA WAITING PIENA! - messy %d %d MAX %d", classe, event, MAXWAITINGMSGS);
			return;
		}
	}

	if (lq->len >= MAX_MESSAGES) {
		uint8 pos;

		for (pos = lq->head; pos != lq->tail; pos = (pos == MAX_MESSAGES - 1) ? 0 : pos + 1)
			warning("EVENT %d %d", lq->event[pos]->classe, lq->event[pos]->event);

		warning("ERRORE! CODA GAME PIENA! - messy %d %d MAX %d", classe, event, MAX_MESSAGES);
		return;
	}

	lm = lq->event[lq->tail++];

	if (lm == nullptr)  return;

	lm->classe     = classe;
	lm->event     = event;
	lm->flags     = flags;
	lm->wparam1   = wparam1;
	lm->wparam2   = wparam2;
	lm->bparam    = bparam;
	if (p0 != nullptr) lm->lparam[0] = *(int32 *)p0;
	else    lm->lparam[0] = 0;
	if (p1 != nullptr) lm->lparam[1] = *(int32 *)p1;
	else    lm->lparam[1] = 0;
	if (p2 != nullptr) lm->lparam[2] = *(int32 *)p2;
	else    lm->lparam[2] = 0;

	if ((lq->head == 0) && (lq->tail == 0)) return;
	if (lq->tail == MAX_MESSAGES) lq->tail = 0;
	lq->len++;

	if (lq->len > _maxmsg)
		_maxmsg = lq->len;
}

/* -----------------08/02/99 10.11-------------------
 *                  Scheduler
 * --------------------------------------------------*/
void MessageSystem::scheduler() {
	static uint8 Counter;

	TheMessage = nullptr;

	// spara un idle ogni trenta messaggi
	if (Counter++ <= 30) {
		if (!(_GetMessage(&Game))) TheMessage = &idlemessage;
	} else {
		Counter = 0;
		TheMessage = &idlemessage;
	}
	//warning("Scheduler: %s %d", eventToString(TheMessage->classe), TheMessage->event);
}

/* -----------------08/02/99 10.11-------------------
 *                  ProcessTheMessage
 * --------------------------------------------------*/
void ProcessTheMessage(WGame &game) {
SUPEREVENT:
	if (TheMessage->classe != EventClass::MC_IDLE)
		warning("doEvent: %s - %d", eventToString(TheMessage->classe), TheMessage->event);

	switch (TheMessage->classe) {
	case EventClass::MC_IDLE:
		break;

	case EventClass::MC_CAMERA:
		game._cameraMan->doCamera(game);
		break;

	case EventClass::MC_SYSTEM:
		doSystem(game);
		break;

	case EventClass::MC_MOUSE:
		doMouse(game);
		break;

	case EventClass::MC_STRING:
		doString(game);
		break;

	case EventClass::MC_PLAYER:
		doPlayer(game);
		break;

	case EventClass::MC_ACTION:
		doAction(game);
		break;

	case EventClass::MC_INVENTORY:
		doInventory(game);
		break;

	case EventClass::MC_DIALOG:
		doDialog(game);
		break;

	case EventClass::MC_ANIM:
		doAnimation(game);
		break;

	case EventClass::MC_T2D:
		doT2D(game);
		break;
	}

	if (game._messageSystem.SuperEventActivate) {
		game._messageSystem.SuperEventActivate = false;
		goto SUPEREVENT;
	}
}

/* -----------------19/03/98 9.59--------------------
 *                  ReEvent
 * --------------------------------------------------*/
void ReEvent() {
	if (TheMessage == nullptr)
		return ;

	_vm->_messageSystem.doEvent(TheMessage->classe, TheMessage->event, TheMessage->flags, TheMessage->wparam1, TheMessage->wparam2,
			TheMessage->bparam, &TheMessage->lparam[0], &TheMessage->lparam[1], &TheMessage->lparam[2]);
}

/* -----------------08/02/99 10.11-------------------
 *                  RemoveEvent
 * --------------------------------------------------*/
void MessageSystem::removeEvent(EventClass classe, uint8 event) {
	pqueue *lq = &Game;
	uint8 pos, b, c;

	for (pos = lq->head; pos != lq->tail; pos = (pos == MAX_MESSAGES - 1) ? 0 : pos + 1) {
		if ((lq->event[pos]->classe == classe) && ((event == ME_ALL) || (lq->event[pos]->event == event))) {
			lq->tail = (lq->tail == 0) ? MAX_MESSAGES - 1 : lq->tail - 1;
			lq->len--;

			for (b = pos; b != lq->tail; b = (b == MAX_MESSAGES - 1) ? 0 : b + 1) {
				c = (b == MAX_MESSAGES - 1) ? 0 : b + 1;
				memcpy(lq->event[b], lq->event[c], sizeof(message));
			}
			pos = (pos == 0) ? MAX_MESSAGES - 1 : pos - 1;
		}
	}

	for (b = 0; b < MAXWAITINGMSGS; b++)
		if ((_waitingMsg[b].classe == classe) && ((event == ME_ALL) || (_waitingMsg[b].event == event)))
			_waitingMsg[b].reset();
}

/* -----------------18/12/00 16.32-------------------
 *                  RemoveEvent_bparam
 * --------------------------------------------------*/
//come RemoveEvent(), solo che controllo anche bparam
void MessageSystem::removeEvent_bparam(EventClass classe, uint8 event, uint8 bparam) {
	uint8 pos, b, c;
	pqueue *lq = &Game;

	for (pos = lq->head; pos != lq->tail; pos = (pos == MAX_MESSAGES - 1) ? 0 : pos + 1) {
		if ((lq->event[pos]->classe == classe) && ((event == ME_ALL) || (lq->event[pos]->event == event)) && (lq->event[pos]->bparam == bparam)) {
			lq->tail = (lq->tail == 0) ? MAX_MESSAGES - 1 : lq->tail - 1;
			lq->len--;

			for (b = pos; b != lq->tail; b = (b == MAX_MESSAGES - 1) ? 0 : b + 1) {
				c = (b == MAX_MESSAGES - 1) ? 0 : b + 1;
				memcpy(lq->event[b], lq->event[c], sizeof(message));
			}
			pos = (pos == 0) ? MAX_MESSAGES - 1 : pos - 1;
		}
	}

	for (b = 0; b < MAXWAITINGMSGS; b++)
		if ((_waitingMsg[b].classe == classe) && ((event == ME_ALL) || (_waitingMsg[b].event == event)) && (_waitingMsg[b].bparam == bparam))
			_waitingMsg[b].reset();
}

/* -----------------08/02/99 10.11-------------------
 *                  TestEmptyQueue
 * --------------------------------------------------*/
bool TestEmptyQueue(pqueue *lq, EventClass classe) {
	uint8 pos;

	for (pos = lq->head; pos != lq->tail; pos = (pos == MAX_MESSAGES - 1) ? 0 : pos + 1)
		if (lq->event[pos]->classe == classe)
			return false;

	return true;
}

} // End of namespace Watchmaker
