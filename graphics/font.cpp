/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 */

#include "common/stream.h"
#include "common/file.h"
#include "common/endian.h"
#include "graphics/font.h"

namespace Graphics {

void free_font(NewFontData* pf);

NewFont::~NewFont() {
	if (font) {
		free_font(font);
	}
}

int NewFont::getCharWidth(byte chr) const {
	// If no width table is specified, return the maximum width
	if (!desc.width)
		return desc.maxwidth;
	// If this character is not included in the font, use the default char.
	if (chr < desc.firstchar || desc.firstchar + desc.size < chr) {
		chr = desc.defaultchar;
	}
	return desc.width[chr - desc.firstchar];
}


template <typename PixelType>
void drawCharIntern(byte *ptr, uint pitch, const bitmap_t *src, int h, int minX, int maxX, const PixelType color) {
	const bitmap_t maxXMask = ~((1 << (16-maxX)) - 1);
	while (h-- > 0) {
		bitmap_t buffer = READ_UINT16(src);
		src++;

		buffer &= maxXMask;
		buffer <<= minX;
		PixelType *tmp = (PixelType *)ptr;
		while (buffer != 0) {
			if ((buffer & 0x8000) != 0)
				*tmp = color;
			tmp++;
			buffer <<= 1;
		}

		ptr += pitch;
	}
}

void NewFont::drawChar(Surface *dst, byte chr, const int tx, const int ty, const uint32 color) const {
	assert(dst != 0);

	assert(desc.bits != 0 && desc.maxwidth <= 16);
	assert(dst->bytesPerPixel == 1 || dst->bytesPerPixel == 2);

	// If this character is not included in the font, use the default char.
	if (chr < desc.firstchar || chr >= desc.firstchar + desc.size) {
		chr = desc.defaultchar;
	}

	chr -= desc.firstchar;

	int bbw, bbh, bbx, bby;

	// Get the bounding box of the character
	if (!desc.bbx) {
		bbw = desc.fbbw;
		bbh = desc.fbbh;
		bbx = desc.fbbx;
		bby = desc.fbby;
	} else {
		bbw = desc.bbx[chr].w;
		bbh = desc.bbx[chr].h;
		bbx = desc.bbx[chr].x;
		bby = desc.bbx[chr].y;
	}

	byte *ptr = (byte *)dst->getBasePtr(tx + bbx, ty + desc.ascent - bby - bbh);

	const bitmap_t *tmp = desc.bits + (desc.offset ? desc.offset[chr] : (chr * desc.fbbh));

	int y = MIN(bbh, ty + desc.ascent - bby);
	tmp += bbh - y;
	y -= MAX(0, ty + desc.ascent - bby - dst->h);

	if (dst->bytesPerPixel == 1)
		drawCharIntern<byte>(ptr, dst->pitch, tmp, y, MAX(0, -(tx + bbx)), MIN(bbw, dst->w - tx - bbx), color);
	else if (dst->bytesPerPixel == 2)
		drawCharIntern<uint16>(ptr, dst->pitch, tmp, y, MAX(0, -(tx + bbx)), MIN(bbw, dst->w - tx - bbx), color);
}


#pragma mark -

/* BEGIN font.h*/
/* bitmap_t helper macros*/
#define BITMAP_WORDS(x)			(((x)+15)/16)	/* image size in words*/
#define BITMAP_BYTES(x)			(BITMAP_WORDS(x)*sizeof(bitmap_t))
#define BITMAP_BITSPERIMAGE		(sizeof(bitmap_t) * 8)
#define BITMAP_BITVALUE(n)		((bitmap_t) (((bitmap_t) 1) << (n)))
#define BITMAP_FIRSTBIT			(BITMAP_BITVALUE(BITMAP_BITSPERIMAGE - 1))
#define BITMAP_TESTBIT(m)	((m) & BITMAP_FIRSTBIT)
#define BITMAP_SHIFTBIT(m)	((bitmap_t) ((m) << 1))

/* builtin C-based proportional/fixed font structure */
/* based on The Microwindows Project http://microwindows.org */
struct NewFontData {
	char *	name;		/* font name*/
	int		maxwidth;	/* max width in pixels*/
	int		height;		/* height in pixels*/
	int		ascent;		/* ascent (baseline) height*/
	int		firstchar;	/* first character in bitmap*/
	int		size;		/* font size in glyphs*/
	bitmap_t*	bits;		/* 16-bit right-padded bitmap data*/
	unsigned long* offset;	/* offsets into bitmap data*/
	unsigned char* width;	/* character widths or NULL if fixed*/
	BBX*		bbx;	/* character bounding box or NULL if fixed */
	int		defaultchar;	/* default char (not glyph index)*/
	long	bits_size;	/* # words of bitmap_t bits*/

	/* unused by runtime system, read in by convbdf*/
	char *	facename;	/* facename of font*/
	char *	copyright;	/* copyright info for loadable fonts*/
	int		pixel_size;
	int		descent;
	int		fbbw, fbbh, fbbx, fbby;
};
/* END font.h*/

#define isprefix(buf,str)	(!strncmp(buf, str, strlen(str)))
#define strequal(s1,s2)		(!strcmp(s1, s2))

#define EXTRA	300

int start_char = 0;
int limit_char = 255;

NewFontData* bdf_read_font(Common::SeekableReadStream &fp);
int bdf_read_header(Common::SeekableReadStream &fp, NewFontData* pf);
int bdf_read_bitmaps(Common::SeekableReadStream &fp, NewFontData* pf);
char * bdf_getline(Common::SeekableReadStream &fp, char *buf, int len);
bitmap_t bdf_hexval(unsigned char *buf);

void free_font(NewFontData* pf) {
	if (!pf)
		return;
	free(pf->name);
	free(pf->facename);
	free(pf->bits);
	free(pf->offset);
	free(pf->width);
	free(pf->bbx);
	free(pf);
}

/* build incore structure from .bdf file*/
NewFontData* bdf_read_font(Common::SeekableReadStream &fp) {
	NewFontData* pf;
	uint32 pos = fp.pos();

	pf = (NewFontData*)calloc(1, sizeof(NewFontData));
	if (!pf)
		goto errout;

	if (!bdf_read_header(fp, pf)) {
		warning("Error reading font header");
		goto errout;
	}

	fp.seek(pos, SEEK_SET);

	if (!bdf_read_bitmaps(fp, pf)) {
		warning("Error reading font bitmaps");
		goto errout;
	}

	return pf;

 errout:
	free_font(pf);
	return NULL;
}

/* read bdf font header information, return 0 on error*/
int bdf_read_header(Common::SeekableReadStream &fp, NewFontData* pf) {
	int encoding = 0;
	int nchars = 0, maxwidth, maxheight;
	int firstchar = 65535;
	int lastchar = -1;
	char buf[256];
	char facename[256];
	char copyright[256];

	/* set certain values to errors for later error checking*/
	pf->defaultchar = -1;
	pf->ascent = -1;
	pf->descent = -1;

	for (;;) {
		if (!bdf_getline(fp, buf, sizeof(buf))) {
			warning("Error: EOF on file");
			return 0;
		}
		if (isprefix(buf, "FONT ")) {		/* not required*/
			if (sscanf(buf, "FONT %[^\n]", facename) != 1) {
				warning("Error: bad 'FONT'");
				return 0;
			}
			pf->facename = strdup(facename);
			continue;
		}
		if (isprefix(buf, "COPYRIGHT ")) {	/* not required*/
			if (sscanf(buf, "COPYRIGHT \"%[^\"]", copyright) != 1) {
				warning("Error: bad 'COPYRIGHT'");
				return 0;
			}
			pf->copyright = strdup(copyright);
			continue;
		}
		if (isprefix(buf, "DEFAULT_CHAR ")) {	/* not required*/
			if (sscanf(buf, "DEFAULT_CHAR %d", &pf->defaultchar) != 1) {
				warning("Error: bad 'DEFAULT_CHAR'");
				return 0;
			}
		}
		if (isprefix(buf, "FONT_DESCENT ")) {
			if (sscanf(buf, "FONT_DESCENT %d", &pf->descent) != 1) {
				warning("Error: bad 'FONT_DESCENT'");
				return 0;
			}
			continue;
		}
		if (isprefix(buf, "FONT_ASCENT ")) {
			if (sscanf(buf, "FONT_ASCENT %d", &pf->ascent) != 1) {
				warning("Error: bad 'FONT_ASCENT'");
				return 0;
			}
			continue;
		}
		if (isprefix(buf, "FONTBOUNDINGBOX ")) {
			if (sscanf(buf, "FONTBOUNDINGBOX %d %d %d %d",
					   &pf->fbbw, &pf->fbbh, &pf->fbbx, &pf->fbby) != 4) {
				warning("Error: bad 'FONTBOUNDINGBOX'");
				return 0;
			}
			continue;
		}
		if (isprefix(buf, "CHARS ")) {
			if (sscanf(buf, "CHARS %d", &nchars) != 1) {
				warning("Error: bad 'CHARS'");
				return 0;
			}
			continue;
		}

		/*
		 * Reading ENCODING is necessary to get firstchar/lastchar
		 * which is needed to pre-calculate our offset and widths
		 * array sizes.
		 */
		if (isprefix(buf, "ENCODING ")) {
			if (sscanf(buf, "ENCODING %d", &encoding) != 1) {
				warning("Error: bad 'ENCODING'");
				return 0;
			}
			if (encoding >= 0 &&
				encoding <= limit_char &&
				encoding >= start_char) {

				if (firstchar > encoding)
					firstchar = encoding;
				if (lastchar < encoding)
					lastchar = encoding;
			}
			continue;
		}
		if (strequal(buf, "ENDFONT"))
			break;
	}

	/* calc font height*/
	if (pf->ascent < 0 || pf->descent < 0 || firstchar < 0) {
		warning("Error: Invalid BDF file, requires FONT_ASCENT/FONT_DESCENT/ENCODING");
		return 0;
	}
	pf->height = pf->ascent + pf->descent;

	/* calc default char*/
	if (pf->defaultchar < 0 ||
		pf->defaultchar < firstchar ||
		pf->defaultchar > limit_char )
		pf->defaultchar = firstchar;

	/* calc font size (offset/width entries)*/
	pf->firstchar = firstchar;
	pf->size = lastchar - firstchar + 1;

	/* use the font boundingbox to get initial maxwidth*/
	/*maxwidth = pf->fbbw - pf->fbbx;*/
	maxwidth = pf->fbbw;
	maxheight = pf->fbbh;

	/* initially use font bounding box for bits allocation*/
	pf->bits_size = nchars * BITMAP_WORDS(maxwidth) * maxheight;

	/* allocate bits, offset, and width arrays*/
	pf->bits = (bitmap_t *)malloc(pf->bits_size * sizeof(bitmap_t) + EXTRA);
	pf->offset = (unsigned long *)malloc(pf->size * sizeof(unsigned long));
	pf->width = (unsigned char *)malloc(pf->size * sizeof(unsigned char));
	pf->bbx = (BBX *)malloc(pf->size * sizeof(BBX));

	if (!pf->bits || !pf->offset || !pf->width) {
		warning("Error: no memory for font load");
		return 0;
	}

	return 1;
}

/* read bdf font bitmaps, return 0 on error*/
int bdf_read_bitmaps(Common::SeekableReadStream &fp, NewFontData* pf) {
	long ofs = 0;
	int maxwidth = 0;
	int i, k, encoding = 0, width = 0;
	int bbw = 0, bbh = 0, bbx = 0, bby = 0;
	int proportional = 0;
	int need_bbx = 0;
	int encodetable = 0;
	long l;
	char buf[256];

	/* initially mark offsets as not used*/
	for (i = 0; i < pf->size; ++i)
		pf->offset[i] = (unsigned long)-1;

	for (;;) {
		if (!bdf_getline(fp, buf, sizeof(buf))) {
			warning("Error: EOF on file");
			return 0;
		}
		if (isprefix(buf, "STARTCHAR")) {
			encoding = width = bbw = bbh = bbx = bby = -1;
			continue;
		}
		if (isprefix(buf, "ENCODING ")) {
			if (sscanf(buf, "ENCODING %d", &encoding) != 1) {
				warning("Error: bad 'ENCODING'");
				return 0;
			}
			if (encoding < start_char || encoding > limit_char)
				encoding = -1;
			continue;
		}
		if (isprefix(buf, "DWIDTH ")) {
			if (sscanf(buf, "DWIDTH %d", &width) != 1) {
				warning("Error: bad 'DWIDTH'");
				return 0;
			}
			/* use font boundingbox width if DWIDTH <= 0*/
			if (width <= 0)
				width = pf->fbbw - pf->fbbx;
			continue;
		}
		if (isprefix(buf, "BBX ")) {
			if (sscanf(buf, "BBX %d %d %d %d", &bbw, &bbh, &bbx, &bby) != 4) {
				warning("Error: bad 'BBX'");
				return 0;
			}
			continue;
		}
		if (strequal(buf, "BITMAP")) {
			bitmap_t *ch_bitmap = pf->bits + ofs;
			int ch_words;

			if (encoding < 0)
				continue;

			/* set bits offset in encode map*/
			if (pf->offset[encoding-pf->firstchar] != (unsigned long)-1) {
				warning("Error: duplicate encoding for character %d (0x%02x), ignoring duplicate",
						encoding, encoding);
				continue;
			}
			pf->offset[encoding-pf->firstchar] = ofs;
			pf->width[encoding-pf->firstchar] = width;

			pf->bbx[encoding-pf->firstchar].w = bbw;
			pf->bbx[encoding-pf->firstchar].h = bbh;
			pf->bbx[encoding-pf->firstchar].x = bbx;
			pf->bbx[encoding-pf->firstchar].y = bby;

			if (width > maxwidth)
				maxwidth = width;

			/* clear bitmap*/
			memset(ch_bitmap, 0, BITMAP_BYTES(bbw) * bbh);

			ch_words = BITMAP_WORDS(bbw);

			/* read bitmaps*/
			for (i = 0; i < bbh; ++i) {
				if (!bdf_getline(fp, buf, sizeof(buf))) {
					warning("Error: EOF reading BITMAP data");
					return 0;
				}
				if (isprefix(buf, "ENDCHAR"))
					break;

				for (k = 0; k < ch_words; ++k) {
					bitmap_t value;

					value = bdf_hexval((unsigned char *)buf);
					if (bbw > 8) {
						WRITE_UINT16(ch_bitmap, value);
					} else {
						WRITE_UINT16(ch_bitmap, value << 8);
					}
					ch_bitmap++;
				}
			}

			ofs += ch_words * bbh;
			continue;
		}
		if (strequal(buf, "ENDFONT"))
			break;
	}

	/* set max width*/
	pf->maxwidth = maxwidth;

	/* change unused offset/width values to default char values*/
	for (i = 0; i < pf->size; ++i) {
		int defchar = pf->defaultchar - pf->firstchar;

		if (pf->offset[i] == (unsigned long)-1) {
			pf->offset[i] = pf->offset[defchar];
			pf->width[i] = pf->width[defchar];
			pf->bbx[i].w = pf->bbx[defchar].w;
			pf->bbx[i].h = pf->bbx[defchar].h;
			pf->bbx[i].x = pf->bbx[defchar].x;
			pf->bbx[i].y = pf->bbx[defchar].y;
		}
	}

	/* determine whether font doesn't require encode table*/
	l = 0;
	for (i = 0; i < pf->size; ++i) {
		if (pf->offset[i] != (unsigned long)l) {
			encodetable = 1;
			break;
		}
		l += BITMAP_WORDS(pf->bbx[i].w) * pf->bbx[i].h;
	}
	if (!encodetable) {
		free(pf->offset);
		pf->offset = NULL;
	}

	/* determine whether font is fixed-width*/
	for (i = 0; i < pf->size; ++i) {
		if (pf->width[i] != maxwidth) {
			proportional = 1;
			break;
		}
	}
	if (!proportional) {
		free(pf->width);
		pf->width = NULL;
	}

	/* determine if the font needs a bbx table */
	for (i = 0; i < pf->size; ++i) {
		if (pf->bbx[i].w != pf->fbbw || pf->bbx[i].h != pf->fbbh || pf->bbx[i].x != pf->fbbx || pf->bbx[i].y != pf->fbby) {
			need_bbx = 1;
			break;
		}
	}
	if (!need_bbx) {
		free(pf->bbx);
		pf->bbx = NULL;
	}

	/* reallocate bits array to actual bits used*/
	if (ofs < pf->bits_size) {
		pf->bits = (bitmap_t *)realloc(pf->bits, ofs * sizeof(bitmap_t));
		pf->bits_size = ofs;
	}
	else {
		if (ofs > pf->bits_size) {
			warning("Warning: DWIDTH spec > max FONTBOUNDINGBOX");
			if (ofs > pf->bits_size+EXTRA) {
				warning("Error: Not enough bits initially allocated");
				return 0;
			}
			pf->bits_size = ofs;
		}
	}

	return 1;
}

/* read the next non-comment line, returns buf or NULL if EOF*/
// TODO: Can we use SeekableReadStream::readLine resp. readLine_NEW instead?
char *bdf_getline(Common::SeekableReadStream &fp, char *buf, int len) {
	int c;
	char *b;

	for (;;) {
		b = buf;
		while (!fp.eos()) {
			c = fp.readByte();
			if (c == '\r')
				continue;
			if (c == '\n')
				break;
			if (b - buf >= (len - 1))
				break;
			*b++ = c;
		}
		*b = '\0';
		if (fp.eos() && b == buf)
			return NULL;
		if (b != buf && !isprefix(buf, "COMMENT"))
			break;
	}
	return buf;
}

/* return hex value of buffer */
bitmap_t bdf_hexval(unsigned char *buf) {
	bitmap_t val = 0;

	for (unsigned char *ptr = buf; *ptr; ptr++) {
		int c = *ptr;

		if (c >= '0' && c <= '9')
			c -= '0';
		else if (c >= 'A' && c <= 'F')
			c = c - 'A' + 10;
		else if (c >= 'a' && c <= 'f')
			c = c - 'a' + 10;
		else
			c = 0;
		val = (val << 4) | c;
	}
	return val;
}

NewFont *NewFont::loadFont(Common::SeekableReadStream &stream) {
	NewFontData *data = bdf_read_font(stream);
	if (!data || stream.err()) {
		free_font(data);
		return 0;
	}

	FontDesc desc;
	desc.name = data->name;
	desc.maxwidth = data->maxwidth;
	desc.height = data->height;
	desc.fbbw = data->fbbw;
	desc.fbbh = data->fbbh;
	desc.fbbx = data->fbbx;
	desc.fbby = data->fbby;
	desc.ascent = data->ascent;
	desc.firstchar = data->firstchar;
	desc.size = data->size;
	desc.bits = data->bits;
	desc.offset = data->offset;
	desc.width = data->width;
	desc.bbx = data->bbx;
	desc.defaultchar = data->defaultchar;
	desc.bits_size = data->bits_size;

	return new NewFont(desc, data);
}

bool NewFont::cacheFontData(const NewFont &font, const Common::String &filename) {
	Common::DumpFile cacheFile;
	if (!cacheFile.open(filename)) {
		warning("Couldn't open file '%s' for writing", filename.c_str());
		return false;
	}

	cacheFile.writeUint16BE(font.desc.maxwidth);
	cacheFile.writeUint16BE(font.desc.height);
	cacheFile.writeUint16BE(font.desc.fbbw);
	cacheFile.writeUint16BE(font.desc.fbbh);
	cacheFile.writeUint16BE(font.desc.fbbx);
	cacheFile.writeUint16BE(font.desc.fbby);
	cacheFile.writeUint16BE(font.desc.ascent);
	cacheFile.writeUint16BE(font.desc.firstchar);
	cacheFile.writeUint16BE(font.desc.size);
	cacheFile.writeUint16BE(font.desc.defaultchar);
	cacheFile.writeUint32BE(font.desc.bits_size);

	for (long i = 0; i < font.desc.bits_size; ++i) {
		cacheFile.writeUint16BE(font.desc.bits[i]);
	}

	if (font.desc.offset) {
		cacheFile.writeByte(1);
		for (int i = 0; i < font.desc.size; ++i) {
			cacheFile.writeUint32BE(font.desc.offset[i]);
		}
	} else {
		cacheFile.writeByte(0);
	}

	if (font.desc.width) {
		cacheFile.writeByte(1);
		for (int i = 0; i < font.desc.size; ++i) {
			cacheFile.writeByte(font.desc.width[i]);
		}
	} else {
		cacheFile.writeByte(0);
	}

	if (font.desc.bbx) {
		cacheFile.writeByte(1);
		for (int i = 0; i < font.desc.size; ++i) {
			cacheFile.writeByte(font.desc.bbx[i].w);
			cacheFile.writeByte(font.desc.bbx[i].h);
			cacheFile.writeByte(font.desc.bbx[i].x);
			cacheFile.writeByte(font.desc.bbx[i].y);
		}
	} else {
		cacheFile.writeByte(0);
	}

	return !cacheFile.err();
}

NewFont *NewFont::loadFromCache(Common::SeekableReadStream &stream) {
	NewFont *font = 0;

	NewFontData *data = (NewFontData *)malloc(sizeof(NewFontData));
	if (!data)
		return 0;

	memset(data, 0, sizeof(NewFontData));

	data->maxwidth = stream.readUint16BE();
	data->height = stream.readUint16BE();
	data->fbbw = stream.readUint16BE();
	data->fbbh = stream.readUint16BE();
	data->fbbx = stream.readUint16BE();
	data->fbby = stream.readUint16BE();
	data->ascent = stream.readUint16BE();
	data->firstchar = stream.readUint16BE();
	data->size = stream.readUint16BE();
	data->defaultchar = stream.readUint16BE();
	data->bits_size = stream.readUint32BE();

	data->bits = (bitmap_t *)malloc(sizeof(bitmap_t) * data->bits_size);
	if (!data->bits) {
		free(data);
		return 0;
	}

	for (long i = 0; i < data->bits_size; ++i) {
		data->bits[i] = stream.readUint16BE();
	}

	bool hasOffsetTable = (stream.readByte() != 0);
	if (hasOffsetTable) {
		data->offset = (unsigned long *)malloc(sizeof(unsigned long) * data->size);
		if (!data->offset) {
			free(data->bits);
			free(data);
			return 0;
		}

		for (int i = 0; i < data->size; ++i) {
			data->offset[i] = stream.readUint32BE();
		}
	}

	bool hasWidthTable = (stream.readByte() != 0);
	if (hasWidthTable) {
		data->width = (unsigned char *)malloc(sizeof(unsigned char) * data->size);
		if (!data->width) {
			free(data->bits);
			free(data->offset);
			free(data);
			return 0;
		}

		for (int i = 0; i < data->size; ++i) {
			data->width[i] = stream.readByte();
		}
	}

	bool hasBBXTable = (stream.readByte() != 0);
	if (hasBBXTable) {
		data->bbx = (BBX *)malloc(sizeof(BBX) * data->size);
		if (!data->bbx) {
			free(data->bits);
			free(data->offset);
			free(data->width);
			free(data);
			return 0;
		}

		for (int i = 0; i < data->size; ++i) {
			data->bbx[i].w = (int8)stream.readByte();
			data->bbx[i].h = (int8)stream.readByte();
			data->bbx[i].x = (int8)stream.readByte();
			data->bbx[i].y = (int8)stream.readByte();
		}
	}

	FontDesc desc;
	desc.name = data->name;
	desc.maxwidth = data->maxwidth;
	desc.height = data->height;
	desc.fbbw = data->fbbw;
	desc.fbbh = data->fbbh;
	desc.fbbx = data->fbbx;
	desc.fbby = data->fbby;
	desc.ascent = data->ascent;
	desc.firstchar = data->firstchar;
	desc.size = data->size;
	desc.bits = data->bits;
	desc.offset = data->offset;
	desc.width = data->width;
	desc.bbx = data->bbx;
	desc.defaultchar = data->defaultchar;
	desc.bits_size = data->bits_size;

	font = new NewFont(desc, data);
	if (!font || stream.err()) {
		delete font;
		free(data->bits);
		free(data->offset);
		free(data->width);
		free(data);
		return 0;
	}

	return font;
}

#pragma mark -


int Font::getStringWidth(const Common::String &str) const {
	int space = 0;

	for (uint i = 0; i < str.size(); ++i)
		space += getCharWidth(str[i]);
	return space;
}

void Font::drawString(Surface *dst, const Common::String &s, int x, int y, int w, uint32 color, TextAlign align, int deltax, bool useEllipsis) const {
	assert(dst != 0);
	const int leftX = x, rightX = x + w;
	uint i;
	int width = getStringWidth(s);
	Common::String str;

	if (useEllipsis && width > w) {
		// String is too wide. So we shorten it "intellegently", by replacing
		// parts of it by an ellipsis ("..."). There are three possibilities
		// for this: replace the start, the end, or the middle of the string.
		// What is best really depends on the context; but unless we want to
		// make this configurable, replacing the middle probably is a good
		// compromise.
		const int ellipsisWidth = getStringWidth("...");

		// SLOW algorithm to remove enough of the middle. But it is good enough
		// for now.
		const int halfWidth = (w - ellipsisWidth) / 2;
		int w2 = 0;

		for (i = 0; i < s.size(); ++i) {
			int charWidth = getCharWidth(s[i]);
			if (w2 + charWidth > halfWidth)
				break;
			w2 += charWidth;
			str += s[i];
		}
		// At this point we know that the first 'i' chars are together 'w2'
		// pixels wide. We took the first i-1, and add "..." to them.
		str += "...";

		// The original string is width wide. Of those we already skipped past
		// w2 pixels, which means (width - w2) remain.
		// The new str is (w2+ellipsisWidth) wide, so we can accomodate about
		// (w - (w2+ellipsisWidth)) more pixels.
		// Thus we skip ((width - w2) - (w - (w2+ellipsisWidth))) =
		// (width + ellipsisWidth - w)
		int skip = width + ellipsisWidth - w;
		for (; i < s.size() && skip > 0; ++i) {
			skip -= getCharWidth(s[i]);
		}

		// Append the remaining chars, if any
		for (; i < s.size(); ++i) {
			str += s[i];
		}

		width = getStringWidth(str);

	} else {
		str = s;
	}

	if (align == kTextAlignCenter)
		x = x + (w - width)/2;
	else if (align == kTextAlignRight)
		x = x + w - width;
	x += deltax;

	for (i = 0; i < str.size(); ++i) {
		w = getCharWidth(str[i]);
		if (x+w > rightX)
			break;
		if (x >= leftX)
			drawChar(dst, str[i], x, y, color);
		x += w;
	}
}


struct WordWrapper {
	Common::StringList &lines;
	int actualMaxLineWidth;

	WordWrapper(Common::StringList &l) : lines(l), actualMaxLineWidth(0) {
	}

	void add(Common::String &line, int &w) {
		if (actualMaxLineWidth < w)
			actualMaxLineWidth = w;

		lines.push_back(line);

		line.clear();
		w = 0;
	}
};

int Font::wordWrapText(const Common::String &str, int maxWidth, Common::StringList &lines) const {
	WordWrapper wrapper(lines);
	Common::String line;
	Common::String tmpStr;
	int lineWidth = 0;
	int tmpWidth = 0;

	// The rough idea behind this algorithm is as follows:
	// We accumulate characters into the string tmpStr. Whenever a full word
	// has been gathered together this way, we 'commit' it to the line buffer
	// 'line', i.e. we add tmpStr to the end of line, then clear it. Before
	// we do that, we check whether it would cause 'line' to exceed maxWidth;
	// in that case, we first add line to lines, then reset it.
	//
	// If a newline character is read, then we also add line to lines and clear it.
	//
	// Special care has to be taken to account for 'words' that exceed the width
	// of a line. If we encounter such a word, we have to wrap it over multiple
	// lines.

	for (Common::String::const_iterator x = str.begin(); x != str.end(); ++x) {
		const byte c = *x;
		const int w = getCharWidth(c);
		const bool wouldExceedWidth = (lineWidth + tmpWidth + w > maxWidth);

		// If this char is a whitespace, then it represents a potential
		// 'wrap point' where wrapping could take place. Everything that
		// came before it can now safely be added to the line, as we know
		// that it will not have to be wrapped.
		if (isspace(c)) {
			line += tmpStr;
			lineWidth += tmpWidth;

			tmpStr.clear();
			tmpWidth = 0;

			// If we encounter a line break (\n), or if the new space would
			// cause the line to overflow: start a new line
			if (c == '\n' || wouldExceedWidth) {
				wrapper.add(line, lineWidth);
				continue;
			}
		}

		// If the max line width would be exceeded by adding this char,
		// insert a line break.
		if (wouldExceedWidth) {
			// Commit what we have so far, *if* we have anything.
			// If line is empty, then we are looking at a word
			// which exceeds the maximum line width.
			if (lineWidth > 0) {
				wrapper.add(line, lineWidth);
				// Trim left side
				while (tmpStr.size() && isspace(tmpStr[0])) {
					tmpWidth -= getCharWidth(tmpStr[0]);
					tmpStr.deleteChar(0);
				}
			} else {
				wrapper.add(tmpStr, tmpWidth);
			}
		}

		tmpWidth += w;
		tmpStr += c;
	}

	// If some text is left over, add it as the final line
	line += tmpStr;
	lineWidth += tmpWidth;
	if (lineWidth > 0) {
		wrapper.add(line, lineWidth);
	}
	return wrapper.actualMaxLineWidth;
}


} // End of namespace Graphics
