/* Copyright (c) <2003-2011> <Julio Jerez, Newton Game Dynamics>
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software
 * in a product, an acknowledgment in the product documentation would be
 * appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source distribution.
 */

#include "dgStdafx.h"
#include "dgTypes.h"
#include "dgThreads.h"

inline void dgSpinUnlock(dgInt32 *spin) {
	*spin = 0;
}

dgThreads::dgThreads() {
	m_numberOfCPUCores = 0;

	m_numOfThreads = 0;
	m_exit = false;

	m_topIndex = 0;
	m_bottomIndex = 0;
	m_workInProgress = 0;
	m_globalSpinLock = 0;
	m_workToDoSpinLock = 0;

	m_getPerformanceCount = NULL;
	for (dgInt32 i = 0; i < DG_MAXIMUN_THREADS; i++) {
		m_localData[i].m_ticks = 0;
		m_localData[i].m_threadIndex = i;
		m_localData[i].m_manager = this;
	}
}

dgThreads::~dgThreads() {

}

dgInt32 dgThreads::GetThreadCount() const {
	return (m_numOfThreads == 0) ? 1 : m_numOfThreads;
}

void dgThreads::ClearTimers() {

}

void dgThreads::SetPerfomanceCounter(OnGetPerformanceCountCallback callback) {
	m_getPerformanceCount = callback;
}

dgUnsigned32 dgThreads::GetPerfomanceTicks(dgUnsigned32 threadIndex) const {

	if (dgInt32(threadIndex) <= m_numOfThreads) {
		return dgUnsigned32(m_localData[threadIndex].m_ticks);
	} else {
		return 0;
	}
}

void dgThreads::CreateThreaded(dgInt32 threads) {

}

void dgThreads::DestroydgThreads() {

}

//Queues up another to work
dgInt32 dgThreads::SubmitJob(dgWorkerThread *const job) {
	NEWTON_ASSERT(job->m_threadIndex != -1);
	job->ThreadExecute();
	return 1;
}

void *dgThreads::ThreadExecute(void *param) {
	dgLocadData &data = *(dgLocadData *) param;
	data.m_manager->DoWork(data.m_threadIndex);
	return 0;
}

dgInt32 dgThreads::GetWork(dgWorkerThread **job) {
	dgWorkerThread *cWorker = m_queue[m_bottomIndex];
	*job = cWorker;
	return 1;
}

void dgThreads::DoWork(dgInt32 mythreadIndex) {
//	dgWorkerThread *job;

//	job->ThreadExecute();
}

void dgThreads::SynchronizationBarrier() {

}

void dgThreads::CalculateChunkSizes(dgInt32 elements,
                                    dgInt32 *const chunkSizes) const {
	dgInt32 step;
	dgInt32 fraction;

	if (m_numOfThreads) {
		step = elements / m_numOfThreads;
		fraction = elements - step * m_numOfThreads;
		for (dgInt32 i = 0; i < m_numOfThreads; i++) {
			chunkSizes[i] = step + (fraction > 0);
			fraction--;
		}
	} else {
		chunkSizes[0] = elements;
	}
}

void dgThreads::dgGetLock() const {
	NEWTON_ASSERT(sizeof(dgInt32) == sizeof(long));

	//spinLock( &m_globalSpinLock );
// linux and mac may need to yeald time
//	while(! __sync_bool_compare_and_swap(&m_globalSpinLock, 0, 1) ) {
//		ThreadYield();
//	}
}

void dgThreads::dgReleaseLock() const {
	dgSpinUnlock(&m_globalSpinLock);
}

void dgThreads::dgGetIndirectLock(dgInt32 *lockVar) {
	NEWTON_ASSERT(sizeof(dgInt32) == sizeof(long));
}

void dgThreads::dgReleaseIndirectLock(dgInt32 *lockVar) {
	NEWTON_ASSERT(sizeof(dgInt32) == sizeof(long));
	dgSpinUnlock(lockVar);
}
