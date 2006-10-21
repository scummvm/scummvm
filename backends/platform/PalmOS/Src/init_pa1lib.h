#ifndef INIT_PA1LIB_H
#define INIT_PA1LIB_H

// need to move this on a .h file
#define sonySysFileCSystem			'SsYs'  /* Sony overall System */
#define sonySysFtrCreatorSystem		sonySysFileCSystem

#define sonySysFtrNumSystemBase						10000
#define sonySysFtrNumSystemAOutSndStateOnHandlerP	(sonySysFtrNumSystemBase + 4)
#define sonySysFtrNumSystemAOutSndStateOffHandlerP	(sonySysFtrNumSystemBase + 5)

typedef void (*sndStateOnType)	(UInt8 /* kind */, UInt8 /* L volume 0-31 */, UInt8 /* R volume 0-31 */);
typedef void (*sndStateOffType)	(UInt8 /* kind */);

/* kind */
#define aOutSndKindSp       (0) /* Speaker volume */
#define aOutSndKindHp       (2) /* HeadPhone volume */

void Pa1libInit(UInt16 vol);
void Pa1libRelease();

#endif
