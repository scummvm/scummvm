#ifndef __GPSTDLIB_H__
#define __GPSTDLIB_H__

#ifdef __cplusplus
extern "C" {
#endif

/************************************************************************/
/*SYS INITIALIZE FUNCTION*/
void _gp_sdk_init(void);
void _gp_key_polling_time_set(int loop_cnt);


/************************************************************************/
int GpAppPathSet(const char * p_path, int n_len);
char * GpAppPathGet(int * n_len /* optional */);
int GpUserInfoGet(char * p_id, char * p_pwd);
int GpArgSet(int len, char * p_arg);
int GpAppExecute(char * p_code_ptr, const char * s_path);
void GpAppExit(void);
unsigned int GpTickCountGet(void);
void GpNetTpsSet(int tps);
int GpUSBLineCheck(void);

/*Keyboard*/
void GpKeyInit(void);
int GpKeyGet(void);							/*return key data*/
int GpKeyGetEx(int * key_status);			/*return GpKeyChanged()*/
int GpKeyChanged(void);

/*CPU Speed*/
int GpClockSpeedChange(int master_speed, int div_factor, int clk_mode);
unsigned int GpMClkGet(void);
unsigned int GpHClkGet(void);
unsigned int GpPClkGet(void);
/***********************************************************************/

/***********************************************************************/
	#include "gpmem.h"

extern GPMEMFUNC gp_mem_func;
extern GPSTRFUNC gp_str_func;
/***********************************************************************/

/***********************************************************************/
void GpSrand(int seed);
unsigned int GpRand(void);
unsigned int GpRandN(unsigned n);
/***********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /*__k41lib_h__*/
