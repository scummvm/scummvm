#ifndef __OS_TYPES_H__
#define __OS_TYPES_H__

#include <os_types.h>

extern uint8_t  DefaultCharset[1024];
extern uint8_t  ZXSpectrumCharacterSet[768];
extern uint8_t  charset[2048];

#ifndef NO_SAMPLES
extern uint8_t  clickSample[];
extern int      clickSampleSize;
extern uint8_t  beepSample[];
extern int      beepSampleSize;
#endif

#endif