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

static inline void dgThreadYield()
{
#ifdef _WIN32
  Sleep(0);
#else
#ifndef TARGET_OS_IPHONE
  sched_yield();
#endif
#endif
}

static inline void dgSpinLock(dgInt32 *spin)
{
#ifdef _WIN32
  while (InterlockedExchange((long*) spin, 1))
  {
    Sleep(0);
  }
#elif defined (__APPLE__)
#ifndef TARGET_OS_IPHONE
  while( ! OSAtomicCompareAndSwap32(0, 1, (int32_t*) spin) )
  {
    sched_yield();
  }
#endif
#else
  while(! __sync_bool_compare_and_swap((int32_t*)spin, 0, 1) )
  {
    sched_yield();
  }
#endif
}

inline void dgSpinUnlock(dgInt32 *spin)
{
  *spin = 0;
}

static inline void dgInterlockedIncrement(dgInt32* Addend)
{
#ifdef _WIN32
  InterlockedIncrement((long*) Addend);
#elif defined (__APPLE__)
  OSAtomicAdd32 (1, (int32_t*)Addend);
#else
  __sync_fetch_and_add ((int32_t*)Addend, 1 );
#endif
}

static inline void dgInterlockedDecrement(dgInt32* Addend)
{
#ifdef _WIN32
  InterlockedDecrement((long*) Addend);
#elif defined (__APPLE__)
  OSAtomicAdd32 (-1, (int32_t*)Addend);
#else
  __sync_fetch_and_sub ((int32_t*)Addend, 1 );
#endif
}

dgThreads::dgThreads()
{
#ifdef _WIN32
  SYSTEM_INFO sysInfo;
  GetSystemInfo(&sysInfo);
  m_numberOfCPUCores = dgInt32(sysInfo.dwNumberOfProcessors);

  m_numOfThreads = 0;
  m_exit = NULL;
  m_workToDo = NULL;
  m_emptySlot = NULL;

  m_topIndex = 0;
  m_bottomIndex = 0;
  m_workInProgress = 0;
  m_globalSpinLock = 0;

  memset(m_threadhandles, 0, sizeof(m_threadhandles));
#elif defined (__APPLE__)
  int mib[2];
  size_t len;
  int procesorcount;

  mib[0] = CTL_HW;
  mib[1] = HW_NCPU;
  len = sizeof (procesorcount);
  procesorcount = 0;
  m_numberOfCPUCores = sysctl(mib, 2, &procesorcount, &len, NULL, 0);
  m_numberOfCPUCores = procesorcount;

  m_numOfThreads = 0;
  m_exit = false;

  memset (m_threadhandles, 0, sizeof (m_threadhandles));

  m_topIndex = 0;
  m_bottomIndex = 0;
  m_workInProgress = 0;
  m_globalSpinLock = 0;
  m_workToDoSpinLock = 0;
#else
  m_numberOfCPUCores = sysconf(_SC_NPROCESSORS_ONLN);

  m_numOfThreads = 0;
  m_exit = false;

  memset (m_threadhandles, 0, sizeof (m_threadhandles));
  m_topIndex = 0;
  m_bottomIndex = 0;
  m_workInProgress = 0;
  m_globalSpinLock = 0;
  m_workToDoSpinLock = 0;
#endif

  m_getPerformanceCount = NULL;
  for (dgInt32 i = 0; i < DG_MAXIMUN_THREADS; i++)
  {
    m_localData[i].m_ticks = 0;
    m_localData[i].m_threadIndex = i;
    m_localData[i].m_manager = this;
  }
}

dgThreads::~dgThreads()
{
  if (m_numOfThreads)
  {
    DestroydgThreads();
  }
}

dgInt32 dgThreads::GetThreadCount() const
{
  return (m_numOfThreads == 0) ? 1 : m_numOfThreads;
}

void dgThreads::ClearTimers()
{
  for (dgInt32 i = 0; i < m_numOfThreads; i++)
  {
    m_localData[i].m_ticks = 0;
  }
}

void dgThreads::SetPerfomanceCounter(OnGetPerformanceCountCallback callback)
{
  m_getPerformanceCount = callback;
}

dgUnsigned32 dgThreads::GetPerfomanceTicks(dgUnsigned32 threadIndex) const
{

  if (dgInt32(threadIndex) <= m_numOfThreads)
  {
    return dgUnsigned32(m_localData[threadIndex].m_ticks);
  }
  else
  {
    return 0;
  }
}

void dgThreads::CreateThreaded(dgInt32 threads)
{
  if (m_numOfThreads)
  {
    DestroydgThreads();
  }

#ifdef _WIN32
  if ((threads > 1) && (m_numberOfCPUCores > 1))
  {
    m_numOfThreads = GetMin(threads, m_numberOfCPUCores);

    m_emptySlot = CreateSemaphoreA(NULL, DG_MAXQUEUE, DG_MAXQUEUE, NULL);
    m_workToDo = CreateSemaphoreA(NULL, 0, DG_MAXQUEUE, NULL);
    m_exit = CreateEventA(NULL, TRUE, FALSE, NULL);

    InitializeCriticalSection(&m_criticalSection);

    m_topIndex = 0;
    m_bottomIndex = 0;
    m_workInProgress = 0;
    for (dgInt32 i = 0; i < m_numOfThreads; i++)
    {
      m_threadhandles[i] = (HANDLE) _beginthreadex(NULL, 0, ThreadExecute,
          &m_localData[i], 0, NULL);
    }
  }
#else
  if ((threads > 1) && (m_numberOfCPUCores > 1))
  {
#ifdef TARGET_OS_IPHONE
    m_numOfThreads = 0;
#else
    m_numOfThreads = (threads<m_numberOfCPUCores ? threads : m_numberOfCPUCores);
#endif

    m_emptySlot = DG_MAXQUEUE;
    m_workToDo = 0;
    m_workToDoSpinLock = 0;
    m_exit = false;
    m_criticalSection = 0;

    m_topIndex = 0;
    m_bottomIndex = 0;
    m_workInProgress = 0;

#ifndef TARGET_OS_IPHONE
    for(dgInt32 i=0; i < m_numOfThreads; i++)
    {
      pthread_create( &m_threadhandles[i], NULL, ThreadExecute, &m_localData[i]);
    }
#endif
  }
#endif
}

void dgThreads::DestroydgThreads()
{
#ifdef _WIN32
  _ASSERTE(m_workInProgress == 0);

  while (m_workInProgress > 0)
  {
    Sleep(10);
  }
  SetEvent(m_exit);
  DeleteCriticalSection(&m_criticalSection);
  WaitForMultipleObjects(DWORD(m_numOfThreads), m_threadhandles, TRUE,
      INFINITE);

  for (dgInt32 i = 0; i < m_numOfThreads; i++)
  {
    CloseHandle(m_threadhandles[i]);
  }

  CloseHandle(m_exit);
  CloseHandle(m_emptySlot);
  CloseHandle(m_workToDo);

  m_exit = NULL;
  m_emptySlot = NULL;
  m_workToDo = NULL;
  memset(&m_criticalSection, 0, sizeof(CRITICAL_SECTION));
  for (dgInt32 i = 0; i < m_numOfThreads; i++)
  {
    m_threadhandles[i] = NULL;
  }

  m_topIndex = 0;
  m_bottomIndex = 0;
  m_workInProgress = 0;
  m_numOfThreads = 0;
#else
  while(m_workInProgress > 0)
  {
    usleep(100000);
  }
  dgSpinLock( &m_criticalSection );
  m_exit = true;
  m_workToDo = DG_MAXQUEUE+1;
  dgSpinUnlock( &m_criticalSection );

#ifndef TARGET_OS_IPHONE
  for(dgInt32 i=0; i<m_numOfThreads; i++ )
  {
    pthread_join( m_threadhandles[i], NULL );
  }
#endif

  m_exit = false;
  m_emptySlot = 0;
  m_workToDo = 0;
  m_workToDoSpinLock = 0;

  m_topIndex = 0;
  m_bottomIndex = 0;
  m_workInProgress = 0;
  m_numOfThreads = 0;
#endif
}

//Queues up another to work
dgInt32 dgThreads::SubmitJob(dgWorkerThread* const job)
{
  if (!m_numOfThreads)
  {
    _ASSERTE(job->m_threadIndex != -1);
    job->ThreadExecute();
  }
  else
  {

#ifdef _WIN32
    dgInterlockedIncrement(&m_workInProgress);
    if (WaitForSingleObject(m_emptySlot, INFINITE) != WAIT_OBJECT_0)
    {
      return (0);
    }

    EnterCriticalSection(&m_criticalSection);
    m_queue[m_topIndex] = job;
    m_topIndex = (m_topIndex + 1) % DG_MAXQUEUE;
    ReleaseSemaphore(m_workToDo, 1, NULL);
    LeaveCriticalSection(&m_criticalSection);
#else
    dgInterlockedIncrement(&m_workInProgress);
    while ( m_emptySlot == 0 )
    {
      dgThreadYield();
    }
    dgInterlockedDecrement( &m_emptySlot );

    dgSpinLock(&m_criticalSection);
    m_queue[m_topIndex] = job;
    m_topIndex = (m_topIndex + 1) % DG_MAXQUEUE;
    dgInterlockedIncrement( &m_workToDo );
    dgSpinUnlock( &m_criticalSection );
#endif
  }
  return 1;
}

#ifdef _WIN32
dgUnsigned32 _stdcall dgThreads::ThreadExecute(void *param)
#else
void* dgThreads::ThreadExecute(void *param)
#endif
{
  dgLocadData& data = *(dgLocadData*) param;
  data.m_manager->DoWork(data.m_threadIndex);
  return 0;
}

dgInt32 dgThreads::GetWork(dgWorkerThread** job)
{
#ifdef _WIN32
  HANDLE hWaitHandles[2];
  hWaitHandles[0] = m_workToDo;
  hWaitHandles[1] = m_exit;

  if ((WaitForMultipleObjects(2, hWaitHandles, FALSE, INFINITE) - WAIT_OBJECT_0)
      == 1)
  {
    return 0;
  }

  EnterCriticalSection(&m_criticalSection);
  *job = m_queue[m_bottomIndex];
  m_bottomIndex = (m_bottomIndex + 1) % DG_MAXQUEUE;
  ReleaseSemaphore(m_emptySlot, 1, NULL);
  LeaveCriticalSection(&m_criticalSection);
#else
  for (;;)
  {
    while ( m_workToDo == 0 )
    {
      dgThreadYield();
    }
    dgSpinLock( &m_workToDoSpinLock );
    if ( m_workToDo > 0 )
    {
      break;
    }
    dgSpinUnlock( &m_workToDoSpinLock );
  }
  dgInterlockedDecrement( &m_workToDo );
  dgSpinUnlock( &m_workToDoSpinLock );
  if ( m_exit )
  {
    return 0;
  }

  dgSpinLock( &m_criticalSection );
  dgWorkerThread* cWorker = m_queue[m_bottomIndex];
  *job = cWorker;

  m_bottomIndex = (m_bottomIndex+1) % (DG_MAXQUEUE);
  dgInterlockedIncrement( &m_emptySlot );
  dgSpinUnlock( &m_criticalSection );
#endif

  return 1;
}

void dgThreads::DoWork(dgInt32 mythreadIndex)
{
  dgWorkerThread* job;

#ifdef _WIN32
#ifndef __USE_DOUBLE_PRECISION__
  dgUnsigned32 controlWorld;
  controlWorld = dgControlFP (0xffffffff, 0);
  dgControlFP(_PC_53, _MCW_PC);
#endif
#endif

  if (!m_getPerformanceCount)
  {
    while (GetWork(&job))
    {
      job->ThreadExecute();
      dgInterlockedDecrement(&m_workInProgress);
    }
  }
  else
  {
    while (GetWork(&job))
    {
      dgUnsigned32 ticks = m_getPerformanceCount();

      job->ThreadExecute();
      dgInterlockedDecrement(&m_workInProgress);

      m_localData[mythreadIndex].m_ticks += (m_getPerformanceCount() - ticks);
    }
  }

#ifdef _WIN32
#ifndef __USE_DOUBLE_PRECISION__
  dgControlFP(controlWorld, _MCW_PC);
#endif
#endif

}

void dgThreads::SynchronizationBarrier()
{
  while (m_workInProgress)
  {
    dgThreadYield();
  }
}

void dgThreads::CalculateChunkSizes(dgInt32 elements,
    dgInt32* const chunkSizes) const
{
  dgInt32 step;
  dgInt32 fraction;

  if (m_numOfThreads)
  {
    step = elements / m_numOfThreads;
    fraction = elements - step * m_numOfThreads;
    for (dgInt32 i = 0; i < m_numOfThreads; i++)
    {
      chunkSizes[i] = step + (fraction > 0);
      fraction--;
    }
  }
  else
  {
    chunkSizes[0] = elements;
  }
}

void dgThreads::dgGetLock() const
{
  _ASSERTE(sizeof (dgInt32) == sizeof (long));
  dgSpinLock(&m_globalSpinLock);

  //spinLock( &m_globalSpinLock );
// linux and mac may need to yeald time
//	while(! __sync_bool_compare_and_swap(&m_globalSpinLock, 0, 1) ) {
//		ThreadYield();
//	}
}

void dgThreads::dgReleaseLock() const
{
  dgSpinUnlock(&m_globalSpinLock);
}

void dgThreads::dgGetIndirectLock(dgInt32* lockVar)
{
  _ASSERTE(sizeof (dgInt32) == sizeof (long));
  dgSpinLock(lockVar);
}

void dgThreads::dgReleaseIndirectLock(dgInt32* lockVar)
{
  _ASSERTE(sizeof (dgInt32) == sizeof (long));
  dgSpinUnlock(lockVar);
}
