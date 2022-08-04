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

#if !defined(AFX_DG_THREADS_42YH_HY78GT_YHJ63Y__INCLUDED_)
#define AFX_DG_THREADS_42YH_HY78GT_YHJ63Y__INCLUDED_

#define DG_MAXQUEUE		16


class dgWorkerThread
{
	public:
	dgWorkerThread() {m_threadIndex = -1;}
	virtual void ThreadExecute() = 0;
	virtual ~dgWorkerThread () {}

	dgInt32 m_threadIndex;
};


class dgThreads
{
public:
	dgThreads();
	~dgThreads();

	void CreateThreaded (dgInt32 threadCount);
	void DestroydgThreads();

	void ClearTimers();
	void SetPerfomanceCounter(OnGetPerformanceCountCallback callback);
	dgUnsigned32 GetPerfomanceTicks (dgUnsigned32 threadIndex) const;

	dgInt32 GetThreadCount() const ;
	dgInt32 SubmitJob(dgWorkerThread* const job);
	void SynchronizationBarrier ();
	void CalculateChunkSizes (dgInt32 elements, dgInt32* const chunkSizes) const;

	void dgGetLock() const;
	void dgReleaseLock() const;

	void dgGetIndirectLock(dgInt32* lockVar);
	void dgReleaseIndirectLock(dgInt32* lockVar);

private:
	struct dgLocadData
	{
		dgInt32 m_ticks;
		dgInt32 m_threadIndex;
		dgThreads* m_manager;
	};

	void DoWork(dgInt32 threadIndex);
	dgInt32 GetWork(dgWorkerThread** cWork);


	static void* ThreadExecute(void *Param);

	dgInt32 m_numOfThreads;
	dgInt32 m_numberOfCPUCores;
	dgInt32 m_topIndex;
	dgInt32 m_bottomIndex;
	dgInt32 m_workInProgress;
	mutable dgInt32 m_globalSpinLock;

	bool m_exit;
	dgInt32 m_emptySlot;
	dgInt32 m_workToDo;
	dgInt32 m_criticalSection;
	dgInt32 m_workToDoSpinLock;
	dgWorkerThread* m_queue[DG_MAXQUEUE];
	//pthread_t m_threadhandles[DG_MAXIMUN_THREADS];

	OnGetPerformanceCountCallback m_getPerformanceCount;
	dgLocadData m_localData[DG_MAXIMUN_THREADS];
};



#endif

