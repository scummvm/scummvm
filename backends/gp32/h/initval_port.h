#ifndef __INITVAL_PORT_H__
#define __INITVAL_PORT_H__

/*******************************************************************/
/*at loading time, thread stack define -- implemented in gpstart.c*/
#define GPMAIN_STACK_SIZE		(100<<10)		/*100KB	-- access code = 0*/
#define NET_STACK_SIZE			(64<<10)		/*64KB	-- access code = 1*/
#define USER_STACK_SIZE			(4 << 10)		/*4KB   -- access code = 2*/
/*******************************************************************/

/*************************************************************
* Heap Management Library Attach                             *
*************************************************************/
#define USE_GP_MEM		1									/* If you don't use gpmem.alf, change USE_GP_MEM to 0 */

/*************************************************************
* Button Checking Loop count	                             *
*************************************************************/
#define KEYPOLLING_NUM	20									/* You can change polling number, but the valus must be as small as possible. */

/*************************************************************
* Processor Clock speed		                                 *
*************************************************************/
#define DEFAULT_MCLK	67800000
#define CHANGE_MCLK		0									/* If the CHANGE_MCLK is zero, the clock speed of process is 40MHz */
#if CHANGE_MCLK												/* If the CHANGE_MCLK is non-zero, select CLOCKSPEED */
	#define YOUR_SELECT_CLK		0
	#if (YOUR_SELECT_CLK == 0)
		#define CLK_SPEED       59250000
		#define DIV_FACTOR      0x47022
		#define CLK_MODE        1
	#elif (YOUR_SELECT_CLK == 1 )
	#else
	#endif
#endif /*CHANGE_MCLK*/

#endif /*__initval_port_h__*/
