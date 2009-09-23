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

#include <psppower.h>
#include <pspthreadman.h>
 
#include "./powerman.h"
#include "./trace.h"
#include "engine.h"

DECLARE_SINGLETON(PowerManager);

#ifdef __PSP_DEBUG_SUSPEND__
void PowerManager::debugPM() {
	PSPDebugTrace("PM status is %d. Listcount is %d. CriticalCount is %d. ThreadId is %x. Error = %d\n", _PMStatus, _listCounter,
		_criticalCounter, sceKernelGetThreadId(), _error);
}
#else
	#define debugPM()
	#define PMStatusSet(x)
#endif /* __PSP_DEBUG_SUSPEND__ */


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
		PSPDebugSuspend("PowerManager::PowerManager(): Couldn't create condSuspendable\n");
	}
	
	_condPM = SDL_CreateCond();
	if (_condPM <= 0) {
		PSPDebugSuspend("PowerManager::PowerManager(): Couldn't create condPM\n");
	}

	_flagMutex = SDL_CreateMutex();
	if (_flagMutex <= 0) {
		PSPDebugSuspend("PowerManager::PowerManager(): Couldn't create flagMutex\n");
	}

	_listMutex = SDL_CreateMutex();
	if (_listMutex <= 0) {
		PSPDebugSuspend("PowerManager::PowerManager(): Couldn't create listMutex\n");
	}

	_suspendFlag = false;
	_criticalCounter = 0;
	_pauseFlag = 0; _pauseFlagOld = 0; _pauseClientState = 0;

#ifdef __PSP_DEBUG_SUSPEND__	
	_listCounter = 0;
	PMStatusSet(kInitDone);
	_error = 0;	
#endif
 }
 
/*******************************************
*
*	Function to register to be notified when suspend/resume time comes
*
********************************************/ 
int PowerManager::registerSuspend(Suspendable *item) {
	// Register in list
	PSPDebugSuspend("In registerSuspend\n");
	debugPM();

	if (SDL_mutexP(_listMutex) != 0) {
		PSPDebugTrace("PowerManager::registerSuspend(): Couldn't lock _listMutex %d\n", _listMutex);
	}

	_suspendList.push_front(item);
#ifdef __PSP_DEBUG_SUSPEND__
	_listCounter++;
#endif

	if (SDL_mutexV(_listMutex) != 0) {
		PSPDebugTrace("PowerManager::registerSuspend(): Couldn't unlock _listMutex %d\n", _listMutex);
	}

	PSPDebugSuspend("Out of registerSuspend\n");
	debugPM();
	
	return 0;
}

/*******************************************
*
*	Function to unregister to be notified when suspend/resume time comes
*
********************************************/  
int PowerManager::unregisterSuspend(Suspendable *item) {

	PSPDebugSuspend("In unregisterSuspend\n");
	debugPM();

	 // Unregister from stream list
	if (SDL_mutexP(_listMutex) != 0) {
		PSPDebugTrace("PowerManager::unregisterSuspend(): Couldn't unlock _listMutex %d\n", _listMutex);
	}

	_suspendList.remove(item);
#ifdef __PSP_DEBUG_SUSPEND__
	_listCounter--;
#endif	
	
	if (SDL_mutexV(_listMutex) != 0) {
		PSPDebugTrace("PowerManager::unregisterSuspend(): Couldn't unlock _listMutex %d\n", _listMutex);
	}

	PSPDebugSuspend("Out of unregisterSuspend\n");
	debugPM();
	
	return 0;
 }
 
 /*******************************************
*
*	Destructor
*
********************************************/ 
 PowerManager::~PowerManager() {

#ifdef __PSP_DEBUG_SUSPEND__
	PMStatusSet(kDestroyPM);
#endif
 
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
*	Unsafe function to poll for a pause event (first stage of suspending)
*   Only for pausing the engine, which doesn't need high synchronization ie. we don't care if it misreads
*   the flag a couple of times since there is NO mutex protection (for performance reasons). 
*   Polling the engine happens regularly.
*	On the other hand, we don't know if there will be ANY polling which prevents us from using proper events.
*
********************************************/  
void PowerManager::pollPauseEngine() {

	bool pause = _pauseFlag;		// We copy so as not to have multiple values

	if ((pause != _pauseFlagOld) && g_engine) { // Check to see if we have an engine
		if (pause && _pauseClientState == PowerManager::Unpaused) {
			_pauseClientState = PowerManager::Pausing;	// Tell PM we're in the middle of pausing
			g_engine->pauseEngine(true);
			PSPDebugSuspend("Pausing engine in PowerManager::pollPauseEngine()\n");
			_pauseClientState = PowerManager::Paused;		// Tell PM we're done pausing
		}
		else if (!pause && _pauseClientState == PowerManager::Paused) {
			g_engine->pauseEngine(false);
			PSPDebugSuspend("Unpausing for resume in PowerManager::pollPauseEngine()\n");
			_pauseClientState = PowerManager::Unpaused;	// Tell PM we're in the middle of pausing
		}
			
		_pauseFlagOld = pause;
	}
} 
 
/*******************************************
*
*	Function to be called by threads wanting to block on the PSP entering suspend
*   Use this for small critical sections where you can easily restore the previous state.
*
********************************************/  
 int PowerManager::blockOnSuspend()  {
	return beginCriticalSection(true);
}

/*******************************************
*
*	Function to block on a suspend, then start a non-suspendable critical section
*   Use this for large or REALLY critical critical-sections.
*	Make sure to call endCriticalSection or the PSP won't suspend.
********************************************/  

  int PowerManager::beginCriticalSection(bool justBlock) {
	int ret = NotBlocked;

	if (SDL_mutexP(_flagMutex) != 0) {
		PSPDebugTrace("PowerManager::blockOnSuspend(): Couldn't lock flagMutex %d\n", _flagMutex);
		ret = Error;
	}

	// Check the access flag
	if (_suspendFlag == true) {
		PSPDebugSuspend("We're being blocked!\n");
		debugPM();
		ret = Blocked;

		// If it's true, we wait for a signal to continue
		if (SDL_CondWait(_condSuspendable, _flagMutex) != 0) {
			PSPDebugTrace("PowerManager::blockOnSuspend(): Couldn't wait on cond %d\n", _condSuspendable);
		}

		PSPDebugSuspend("We got blocked!!\n");
		debugPM();
	}
	
	// Now prevent the PM from suspending until we're done
	if (justBlock == false)
		_criticalCounter++;

	if (SDL_mutexV(_flagMutex) != 0) {
		PSPDebugTrace("PowerManager::blockOnSuspend(): Couldn't unlock flagMutex %d\n", _flagMutex);
		ret = Error;
	}

	return ret;
}

int PowerManager::endCriticalSection() {
	int ret = 0;

	if (SDL_mutexP(_flagMutex) != 0) {
		PSPDebugTrace("PowerManager::endCriticalSection(): Couldn't lock flagMutex %d\n", _flagMutex);
		ret = Error;
	}

	// We're done with our critical section
	_criticalCounter--;
	
	if (_criticalCounter <= 0) {
		if (_suspendFlag == true) {	// If the PM is sleeping, this flag must be set
			PSPDebugSuspend("Unblocked thread waking up the PM.\n");
			debugPM();

			SDL_CondBroadcast(_condPM);
			
			PSPDebugSuspend("Woke up the PM\n");
			debugPM();
		}

		if (_criticalCounter < 0) {	// Check for bad usage of critical sections
			PSPDebugTrace("PowerManager::endCriticalSection(): Error! Critical counter is %d\n", _criticalCounter);
			debugPM();
		}
	}

	if (SDL_mutexV(_flagMutex) != 0) {
		PSPDebugTrace("PowerManager::endCriticalSection(): Couldn't unlock flagMutex %d\n", _flagMutex);
		ret = Error;
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
	
	if (_pauseFlag) return ret;	// Very important - make sure we only suspend once

	scePowerLock(0);			// Critical to make sure PSP doesn't suspend before we're done

	// The first stage of suspend is pausing the engine if possible. We don't want to cause files
	// to block, or we might not get the engine to pause. On the other hand, we might wait for polling
	// and it'll never happen. We also want to do this w/o mutexes (for speed) which is ok in this case.
	_pauseFlag = true;		

	PMStatusSet(kWaitForClientPause);
	
	// Now we wait, giving the engine thread some time to find our flag.
	for (int i = 0; i < 10 && _pauseClientState == Unpaused; i++)
		sceKernelDelayThread(50000);	// We wait 50 msec x 10 times = 0.5 seconds
	
	if (_pauseClientState == Pausing) {	// Our event has been acknowledged. Let's wait until the client is done.
		PMStatusSet(kWaitForClientToFinishPausing);

		while (_pauseClientState != Paused)
			sceKernelDelayThread(50000);	// We wait 50 msec at a time
	}

	// It's possible that the polling thread missed our pause event, but there's nothing we can do about that. 
	// We can't know if there's polling going on or not. It's usually not a critical thing anyway.
	
	PMStatusSet(kGettingFlagMutexSuspend);
	
	// Now we set the suspend flag to true to cause reading threads to block
	
	if (SDL_mutexP(_flagMutex) != 0) {
		PSPDebugTrace("PowerManager::suspend(): Couldn't lock flagMutex %d\n", _flagMutex);
		_error = Error;
		ret = Error;
	}

	PMStatusSet(kGotFlagMutexSuspend);
	
	_suspendFlag = true;
	
	// Check if anyone is in a critical section. If so, we'll wait for them
	if (_criticalCounter > 0) {
		PMStatusSet(kWaitCritSectionSuspend);
		SDL_CondWait(_condPM, _flagMutex);
		PMStatusSet(kDoneWaitingCritSectionSuspend);
	}

	if (SDL_mutexV(_flagMutex) != 0) {
		PSPDebugTrace("PowerManager::suspend(): Couldn't unlock flagMutex %d\n", _flagMutex);
		_error = Error;
		ret = Error;
	}

	PMStatusSet(kGettingListMutexSuspend);
	
	// Loop over list, calling suspend()
	if (SDL_mutexP(_listMutex) != 0) {
		PSPDebugTrace("PowerManager::suspend(): Couldn't lock listMutex %d\n", _listMutex);
		_error = Error;
		ret = Error;
	}
	PMStatusSet(kIteratingListSuspend);
	// Iterate
	Common::List<Suspendable *>::iterator i = _suspendList.begin();

	for (; i != _suspendList.end(); i++) {
		(*i)->suspend();
	}
	
	PMStatusSet(kDoneIteratingListSuspend);

	if (SDL_mutexV(_listMutex) != 0) {
		PSPDebugTrace("PowerManager::suspend(): Couldn't unlock listMutex %d\n", _listMutex);
		_error = Error;
		ret = Error;
	}
	PMStatusSet(kDoneSuspend);

	scePowerUnlock(0);				// Allow the PSP to go to sleep now
	
	return ret;
}

/*******************************************
*
*	Callback function to resume every Suspendable
*
********************************************/  
int PowerManager::resume() {
	int ret = 0;

	// Make sure we can't get another suspend
	scePowerLock(0);
	
	if (!_pauseFlag) return ret;				// Make sure we can only resume once
	
	PMStatusSet(kGettingListMutexResume);
	
	// First we notify our Suspendables. Loop over list, calling resume()
	if (SDL_mutexP(_listMutex) != 0) {
		PSPDebugTrace("PowerManager::resume(): Couldn't lock listMutex %d\n", _listMutex);
		_error = Error;
		ret = Error;
	}
	PMStatusSet(kIteratingListResume);
	// Iterate
	Common::List<Suspendable *>::iterator i = _suspendList.begin();

	for (; i != _suspendList.end(); i++) {
		(*i)->resume();
	}
	
	PMStatusSet(kDoneIteratingListResume);

	if (SDL_mutexV(_listMutex) != 0) {
		PSPDebugTrace("PowerManager::resume(): Couldn't unlock listMutex %d\n", _listMutex);
		_error = Error;
		ret = Error;
	}
	
	PMStatusSet(kGettingFlagMutexResume);

	// Now we set the suspend flag to false
	if (SDL_mutexP(_flagMutex) != 0) {
		PSPDebugTrace("PowerManager::resume(): Couldn't lock flagMutex %d\n", _flagMutex);
		_error = Error;
		ret = Error;
	}
	PMStatusSet(kGotFlagMutexResume);
	
	_suspendFlag = false;

	PMStatusSet(kSignalSuspendedThreadsResume);
	
	// Signal the other threads to wake up
	if (SDL_CondBroadcast(_condSuspendable) != 0) {
		PSPDebugTrace("PowerManager::resume(): Couldn't broadcast condition %d\n", _condSuspendable);
		_error = Error;
		ret = Error;
	}
	PMStatusSet(kDoneSignallingSuspendedThreadsResume);

	if (SDL_mutexV(_flagMutex) != 0) {
		PSPDebugTrace("PowerManager::resume(): Couldn't unlock flagMutex %d\n", _flagMutex);
		_error = Error;
		ret = Error;
	}
	PMStatusSet(kDoneResume);
	
	_pauseFlag = false;	// Signal engine to unpause

	scePowerUnlock(0);	// Allow new suspends
	
	return ret;
}
