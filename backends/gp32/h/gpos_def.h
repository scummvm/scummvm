#ifndef __gpos_def_h__
#define __gpos_def_h__

typedef enum{
	H_THREAD_SOUND,
	H_THREAD_IDLE,
	H_THREAD_GPMAIN,
	H_THREAD_NET,
	H_THREAD_TMR0,
	H_THREAD_TMR1,
	H_THREAD_TMR2,
	H_THREAD_TMR3
}H_THREAD;

/*************************************************************
* OS launching flag			                                 *
*************************************************************/
#define ALLOW_PRIORITY_REVERSION	0x1						//BIT [0] flag : if set, allow reversion.

/*******************************************************************/
/*Schedule related define*/
#define GPOS_DFT_RUNNING_TIME	20	//20*2 = 40ms
#define GPOS_DFT_SUSPEND_TIME	5	//5*2 = 10ms
/*******************************************************************/

/*******************************************************************/
/*Priority define*/
#define GPOS_PRIO_ABOVE_NORMAL	2
#define GPOS_PRIO_NORMAL		3
#define GPOS_PRIO_BELOW_NORMAL	4
#define GPOS_PRIO_LOW			5
/*******************************************************************/

/*******************************************************************/
/*GP kernel related return values list*/
#define GPOS_ERR_OK					0
#define GPOS_ERR_NOEFFECT			1
#define GPOS_ERR_INVALIDARG			2
#define GPOS_ERR_ALREADY_USED		3
#define GPOS_ERR_INVALID_ACCESS		4
#define GPOS_ERR_OUTOFMEM			5
#define GPOS_ERR_STACK				6
#define GPOS_ERR_PENDED_THREAD		7
/*******************************************************************/

/*******************************************************************/
/*User thread define*/
#define GPC_MAXPROGTIMER	4

int GpTimerOptSet(int idx, int tmr_tps, int max_exec_tick, void (*irq_tmrfunc)(void));
int GpTimerSet(int idx);
int GpTimerPause(int idx);
int GpTimerResume(int idx);
void GpTimerKill(int idx);
/*******************************************************************/

/*******************************************************************/
/*GP kernel control define*/
void GpMain(void * arg);
void GpKernelInitialize(void);
void GpKernelStart(void);
void GpKernelOptSet(int flag);
void GpKernelLock(void);
void GpKernelUnlock(void);
void GpThreadSleep(unsigned int delay);
int GpThreadOptSet(H_THREAD th, int priority, int stk_size);	/*return ok or err*/
H_THREAD GpThreadHandleGet(void);
void GpNetThreadAct(void (*t_func)(void));
void GpNetThreadDelete(void);
/*******************************************************************/

int GpPredefinedStackGet(H_THREAD th);

#endif
