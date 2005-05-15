#include "native.h"
#include "endianutils.h"

#define MAIN_TYPE	CostumeProc3Type
#include "macros.h"

#define MSETPTR(type, member)		v1.member = _GETPTR(v1comp, V1CodecType, member, type);
#define MSET32(type, member)		v1.member = _GET32(v1comp, V1CodecType, member, type);
#define MSET8(type, member)			v1.member = _GET8(v1comp, V1CodecType, member, type);

UInt32 CostumeRenderer_proc3(void *userData68KP) {
// import variables
	V1Type v1;
	
	SETPTRV	(V1CodecType *	, v1, v1comp				)

	SETPTR	(const byte *	,_srcptr					)
	SET32	(int			,_height					)
	SET8	(byte			,_scaleIndexX				)
	SET8	(byte			,_scaleIndexY				)
	SET8	(byte			,_scaleX					)
	SET8	(byte			,_scaleY					)
	SET32	(int32			,_numStrips					)
	SET32	(int			,_out_pitch					)
	SET32	(int			,_out_w						)
	SET32	(int			,_out_h						)
	SETPTR	(byte *			,_shadow_table				)
	SETPTR	(byte *			,_palette					)
	SET8	(byte			,_shadow_mode				)
	
	MSETPTR	(const byte *	,scaletable					)
	MSET8	(byte			,mask						)
	MSET8	(byte			,shr						)
	MSET8	(byte			,repcolor					)
	MSET8	(byte			,replen						)
	MSET32	(int			,scaleXstep					)
	MSET32	(int			,x							)
	MSET32	(int			,y							)
	MSET32	(int			,scaleXindex				)
	MSET32	(int			,scaleYindex				)
	MSET32	(int			,skip_width					)
	MSETPTR	(byte *			,destptr					)
	MSETPTR	(byte *			,mask_ptr					)
// end of import

	const byte *mask, *src;
	byte *dst;
	byte len, maskbit;
	int y;
	uint color, height, pcolor;
	const byte *scaleytab;
	bool masked;

	y = v1.y;
	src = _srcptr;
	dst = v1.destptr;
	len = v1.replen;
	color = v1.repcolor;
	height = _height;

	scaleytab = &v1.scaletable[_scaleIndexY];
	maskbit = revBitMask(v1.x & 7);
	mask = v1.mask_ptr + v1.x / 8;

	if (len)
		goto StartPos;

	do {
		len = *src++;
		color = len >> v1.shr;
		len &= v1.mask;
		if (!len)
			len = *src++;

		do {
			if (_scaleY == 255 || *scaleytab++ < _scaleY) {
				masked = (y < 0 || y >= _out_h) || (v1.mask_ptr && (mask[0] & maskbit));

				if (color && !masked) {
					// FIXME: Fully implement _shadow_mode.in Sam & Max
					// For now, it's enough for transparency.
					if (_shadow_mode & 0x20) {
						pcolor = _shadow_table[*dst];
					} else {
						pcolor = _palette[color];
						if (pcolor == 13 && _shadow_table)
							pcolor = _shadow_table[*dst];
					}
					*dst = pcolor;
				}
				dst += _out_w;
				mask += _numStrips;
				y++;
			}
			if (!--height) {
				if (!--v1.skip_width)
					//return _scaleIndexX;
					goto end_jump;
				height = _height;
				y = v1.y;

				scaleytab = &v1.scaletable[_scaleIndexY];


				if (_scaleX == 255 || v1.scaletable[_scaleIndexX] < _scaleX) {
					v1.x += v1.scaleXstep;
					if (v1.x < 0 || v1.x >= _out_w)
						//return _scaleIndexX;
						goto end_jump;
					maskbit = revBitMask(v1.x & 7);
					v1.destptr += v1.scaleXstep;
				}
				_scaleIndexX += v1.scaleXstep;
				dst = v1.destptr;
				mask = v1.mask_ptr + v1.x / 8;
			}
		StartPos:;
		} while (--len);
	} while (1);

end_jump:
//	v1comp->x		= ByteSwap32(v1.x);
//	v1comp->destptr	= (byte *)ByteSwap32(v1.destptr);

	return _scaleIndexX;
}
