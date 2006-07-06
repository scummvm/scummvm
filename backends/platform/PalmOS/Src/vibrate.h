/**********************************************************************
**                                                                   **
** vibrate.h                                                         **
**                                                                   **
** Definitions for setting/retrieving the state of the vibrator on   **
** PalmOS 4.x devices. These calls are not officially supported by   **
** Palm Inc and subsidiaries. It is not guaranteed that these calls  **
** will work at all or at least the same on every device. YOU ARE    **
** USING THESE ENTIRELY ON YOUR VERY OWN RISK !                      **
**                                                                   **
** Please send corrections to dseifert@dseifert.com                  **
**********************************************************************/

#include <PalmOS.h>

/* HwrVibrateAttributes takes three arguments:
**     Boolean set        Should the setting be set (1) or retrieved (0)
**     UInt32  setting    what should be set
**     void*   value      pointer to the value to set, or to store the
**                        retrieved setting
*/

Err HwrVibrateAttributes(Boolean set, UInt32 setting, void *value)
              SYS_TRAP(sysTrapHwrVibrateAttributes);

/* to determine whether the vibrator is supported on a specific device, you
** need to make sure that you are running on PalmOS 4.x (so that the
** trap exists), that the attention manager exists and you need to check
** whether HwrVibrateAttributes(0, 1, &active) returns an error code of
** 0. ('active' is a Boolean).
*/


/***************************************************************/
/* For the second parameter, the following defines can be used */
/***************************************************************/

/* *value points to a Boolean stating the state of the vibrator */
#define kHwrVibrateActive                     1

/* *value points to a UInt16, specifying the length of one cycle.
** value is in ticks (1/100 seconds) */
#define kHwrVibrateRate                       2

/* *value points to a UInt32, specifying the pattern of vibrating
**
** example:
**    0xFFFFFFFF    stay on, no vibrating
**    0x0F0F0F0F    vibrate four times in equal intervals
**    0xAAAAAAAA    vibrate really fast (not recommended)
**    0x0F0F0000    vibrate twice, then pause
*/
#define kHwrVibratePattern                    3

/* *value points to a UInt16, specifying the delay between two
** cycles in ticks */
#define kHwrVibrateDelay                      4

/* *value points to a UInt16 specifying the amount of repeats. */
#define kHwrVibrateRepeatCount                5

