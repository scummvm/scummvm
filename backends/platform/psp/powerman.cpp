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

#include "./powerman.h"
#include "./trace.h"

DECLARE_SINGLETON(PowerManager);

 /*******************************************
*
*	Constructor
*
********************************************/ 
PowerManager::PowerManager() {
	_flagMutex = NULL;					/* Init mutex handle */
	_listMutex = NULL;					/* Init mutex handle */
	_condSuspendable = NULL;			/* Init condition variable */
	_condPM = NULL;
	
	_condSuspendable = SDL_CreateCond();
	if (_condSuspendable <= 0) {
		PSPDebugTrace("PowerManager::PowerManager(): Couldn't create condSuspendable\n");
	}
	
	_condPM = SDL_CreateCond();
	if (_condPM <= 0) {
		PSPDebugTrace("PowerManager::PowerManager(): Couldn't create condPM\n");
	}

	_flagMutex = SDL_CreateMutex();
	if (_flagMutex <= 0) {
		PSPDebugTrace("PowerManager::PowerManager(): Couldn't create flagMutex\n");
	}

	_listMutex = SDL_CreateMutex();
	if (_listMutex <= 0) {
		PSPDebugTrace("PowerManager::PowerManager(): Couldn't create listMutex\n");
	}

	_suspendFlag = false;
	_criticalCounter = 0;
 }
 
/*******************************************
*
*	Function to register to be notified when suspend/resume time comes
*
********************************************/ 
int PowerManager::registerSuspend(Suspendable *item) {
	// Register in list
	PSPDebugTrace("In registerSuspend\n");

	if (SDL_mutexP(_listMutex) != 0) {
		PSPDebugTrace("PowerManager::registerSuspend(): Couldn't lock _listMutex %d\n", _listMutex);
	}

	_suspendList.push_front(item);

	if (SDL_mutexV(_listMutex) != 0) {
		PSPDebugTrace("PowerManager::registerSuspend(): Couldn't unlock _listMutex %d\n", _listMutex);
	}

	PSPDebugTrace("Out of registerSuspend\n");

	return 0;
}

/*******************************************
*
*	Function to unregister to be notified when suspend/resume time comes
*
********************************************/  
int PowerManager::unregisterSuspend(Suspendable *item) {

	PSPDebugTrace("In unregisterSuspend\n");

	 // Unregister from stream list
	if (SDL_mutexP(_listMutex) != 0) {
		PSPDebugTrace("PowerManager::unregisterSuspend(): Couldn't unlock _listMutex %d\n", _listMutex);
	}

	_suspendList.remove(item);

	if (SDL_mutexV(_listMutex) != 0) {
		PSPDebugTrace("PowerManager::unregisterSuspend(): Couldn't unlock _listMutex %d\n", _listMutex);
	}

	PSPDebugTrace("Out of unregisterSuspend\n");

	return 0;
 }
 
 /*******************************************
*
*	Destructor
*
********************************************/ 
 PowerManager::~PowerManager() {
	SDL_DestroyCond(_condSuspendable);
	_condSuspendable = 0;
	
	SDL_DestroyCond(_condPM);
	_condPM = 0;

	SDL_DestroyMutex(_flagMutex);
	_flagMutex = 0;

	SDL_DestroyMutex(_listMutex);
	_listMutex = 0;
 }
 
 
 /*******************************************
*
*	Function to be called by threads wanting to block on the PSP entering suspend
*
********************************************/  
 int PowerManager::blockOnSuspend()  {
	return beginCriticalSection(true);
}

 /*
  * Function to block on a suspend, then start a non-suspendable critical section
  */
int PowerManager::beginCriticalSection(bool justBlock) {
	int ret = PowerManager::NotBlocked;

	if (SDL_mutexP(_flagMutex) != 0) {
		PSPDebugTrace("PowerManager::blockOnSuspend(): Couldn't lock flagMutex %d\n", _flagMutex);
		ret = PowerManager::Error;
	}

	// Check the access flag
	if (_suspendFlag == true) {
		PSPDebugTrace("Blocking!!\n");
		ret = PowerManager::Blocked;

		// If it's true, we wait for a signal to continue
		if( SDL_CondWait(_condSuspendable, _flagMutex) != 0) {
			PSPDebugTrace("PowerManager::blockOnSuspend(): Couldn't wait on cond %d\n", _condSuspendable);
		}

		PSPDebugTrace("We got blocked!!\n");
	}
	
	// Now put the pm to sleep
	if (justBlock == false)
		_criticalCounter++;

	if (SDL_mutexV(_flagMutex) != 0) {
		PSPDebugTrace("PowerManager::blockOnSuspend(): Couldn't unlock flagMutex %d\n", _flagMutex);
		ret = PowerManager::Error;
	}

	return ret;
}

int PowerManager::endCriticalSection() {
	int ret = 0;

	if (SDL_mutexP(_flagMutex) != 0) {
		PSPDebugTrace("PowerManager::endCriticalSection(): Couldn't lock flagMutex %d\n", _flagMutex);
		ret = PowerManager::Error;
	}

	// We're done with our critical section
	_criticalCounter--;
	
	if (_criticalCounter <= 0) {
		if(_suspendFlag == true) PSPDebugTrace("Waking up the PM and suspendFlag is true\n");

		SDL_CondBroadcast(_condPM);

		if (_criticalCounter < 0) {
			PSPDebugTrace("PowerManager::endCriticalSection(): Error! Critical counter is %d\n", _criticalCounter);
		}
	}

	if (SDL_mutexV(_flagMutex) != 0) {
		PSPDebugTrace("PowerManager::endCriticalSection(): Couldn't unlock flagMutex %d\n", _flagMutex);
		ret = PowerManager::Error;
	}

	return ret;
}

 /*******************************************
*
*	Callback function to be called to put every Suspendable to suspend
*
********************************************/  
int PowerManager::suspend() {
	int ret = 0;

	// First we set the suspend flag to true
	if (SDL_mutexP(_flagMutex) != 0) {
		PSPDebugTrace("PowerManager::suspend(): Couldn't lock flagMutex %d\n", _flagMutex);
		ret = -1;
	}

	_suspendFlag = true;
	
	if (_criticalCounter > 0)
		SDL_CondWait(_condPM, _flagMutex);

	if (SDL_mutexV(_flagMutex) != 0) {
		PSPDebugTrace("PowerManager::suspend(): Couldn't unlock flagMutex %d\n", _flagMutex);
		ret = -1;
	}

	// Loop over list, calling suspend()
	if (SDL_mutexP(_listMutex) != 0) {
		PSPDebugTrace("PowerManager::suspend(): Couldn't lock listMutex %d\n", _listMutex);
		ret = -1;
	}

	Common::List<Suspendable *>::iterator i = _suspendList.begin();

	for (; i != _suspendList.end(); i++) {
		(*i)->suspend();
	}

	if (SDL_mutexV(_listMutex) != 0) {
		PSPDebugTrace("PowerManager::suspend(): Couldn't unlock listMutex %d\n", _listMutex);
		ret = -1;
	}

	return ret;
}

/*******************************************
*
*	Callback function to resume every Suspendable
*
********************************************/  
int PowerManager::resume() {
	int ret = 0;

	// First we notify our Suspendables. Loop over list, calling resume()
	if (SDL_mutexP(_listMutex) != 0) {
		PSPDebugTrace("PowerManager::resume(): Couldn't lock listMutex %d\n", _listMutex);
		ret = -1;
	}

	Common::List<Suspendable *>::iterator i = _suspendList.begin();

	for (; i != _suspendList.end(); i++) {
		(*i)->resume();
	}

	if (SDL_mutexV(_listMutex) != 0) {
		PSPDebugTrace("PowerManager::resume(): Couldn't unlock listMutex %d\n", _listMutex);
		ret = -1;
	}

	// Now we set the suspend flag to false
	if (SDL_mutexP(_flagMutex) != 0) {
		PSPDebugTrace("PowerManager::resume(): Couldn't lock flagMutex %d\n", _flagMutex);
		ret = -1;
	}
	_suspendFlag = false;

	// Signal the other threads to wake up
	if (SDL_CondBroadcast(_condSuspendable) != 0) {
		PSPDebugTrace("PowerManager::resume(): Couldn't broadcast condition %d\n", _condSuspendable);
		ret = -1;
	}

	if (SDL_mutexV(_flagMutex) != 0) {
		PSPDebugTrace("PowerManager::resume(): Couldn't unlock flagMutex %d\n", _flagMutex);
		ret = -1;
	}

	return ret;
}
