/*
** Gobliiins 1
** Original game by CoktelVision
**
** Reverse engineered by Ivan Dubrov <WFrag@yandex.ru>
**
*/
#include "gob/gob.h"
#include "gob/pack.h"
#include "gob/debug.h"

namespace Gob {

int32 unpackData(char *sourceBuf, char *destBuf) {
	uint32 realSize;
	uint32 counter;
	uint16 cmd;
	byte *src;
	byte *dest;
	byte *tmpBuf;
	int16 off;
	byte len;
	byte i;
	int16 j;
	uint16 tmpIndex;

	realSize = READ_LE_UINT32(sourceBuf);
	counter = READ_LE_UINT32(sourceBuf);

	tmpBuf = (byte *)malloc(4114);

	/*
	 * Can use assembler unpacker for small blocks - for speed.
	 * Don't need this anymore :)
	 */
	/*
	 * if(realSize < 65000)
	 * {
	 * asm_unpackData(sourceBuf, destBuf, tmpBuf);
	 * free(tmpBuf);
	 * return realSize;
	 * }
	 */

	if (tmpBuf == 0)
		return 0;

	for (j = 0; j < 4078; j++)
		tmpBuf[j] = 0x20;
	tmpIndex = 4078;

	src = (byte *)(sourceBuf + 4);
	dest = (byte *)destBuf;

	cmd = 0;
	while (1) {
		cmd >>= 1;
		if ((cmd & 0x0100) == 0) {
			cmd = *src | 0xff00;
			src++;
		}
		if ((cmd & 1) != 0) {	/* copy */
			*dest++ = *src;
			tmpBuf[tmpIndex] = *src;
			src++;
			tmpIndex++;
			tmpIndex %= 4096;
			counter--;
			if (counter == 0)
				break;
		} else {	/* copy string */

			off = *src++;
			off |= (*src & 0xf0) << 4;
			len = (*src & 0x0f) + 3;
			src++;
			for (i = 0; i < len; i++) {
				*dest++ = tmpBuf[(off + i) % 4096];
				counter--;
				if (counter == 0) {
					free(tmpBuf);
					return realSize;
				}
				tmpBuf[tmpIndex] = tmpBuf[(off + i) % 4096];
				tmpIndex++;
				tmpIndex %= 4096;
			}
		}
	}
	free(tmpBuf);
	return realSize;
}

}				// End of namespace Gob
