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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on original Tony Tough source code
 *
 * Copyright (c) 1997-2003 Nayma Software
 */

#include "tony/gfxengine.h"
#include "tony/mpal/mpalutils.h"
#include "tony/tony.h"

namespace Tony {

/****************************************************************************\
*       RMGfxTask Methods
\****************************************************************************/

RMGfxTask::RMGfxTask() {
	m_nPrior = 0;
	m_nInList = 0;
}

int RMGfxTask::Priority() {
	return m_nPrior;
}

void RMGfxTask::RemoveThis(CORO_PARAM, bool &result) {
	result = true;
}


/****************************************************************************\
*       RMGfxTaskSetPrior Methods
\****************************************************************************/

void RMGfxTaskSetPrior::SetPriority(int nPrior) {
	m_nPrior = nPrior;
}


/****************************************************************************\
*       RMGfxBuffer Methods
\****************************************************************************/

RMGfxBuffer::RMGfxBuffer() {
	m_dimx = m_dimy = 0;
	m_bUseDDraw = false;
	m_origBuf = m_buf = NULL;
}

RMGfxBuffer::~RMGfxBuffer() {
	Destroy();
}

void RMGfxBuffer::Create(int dimx, int dimy, int nBpp, bool bUseDDraw) {
	// Destroy the buffer it is already exists
	if (m_buf != NULL)
		Destroy();

	// Copy the parameters in the private members
	m_dimx = dimx;
	m_dimy = dimy;
	m_bUseDDraw = bUseDDraw;

	if (!m_bUseDDraw) {
		// Allocate a buffer
		m_origBuf = m_buf = new byte[m_dimx * m_dimy * nBpp / 8];
		assert(m_buf != NULL);
		Common::fill(m_origBuf, m_origBuf + m_dimx * m_dimy * nBpp / 8, 0);
	}
}

void RMGfxBuffer::Destroy(void) {
	if (!m_bUseDDraw) {
		if (m_origBuf != NULL && m_origBuf == m_buf) {
			delete[] m_origBuf;
			m_origBuf = m_buf = NULL;
		}
	}
}

void RMGfxBuffer::Lock(void) {
	if (m_bUseDDraw) {
		// Manages acceleration
	}
}

void RMGfxBuffer::Unlock(void) {
	if (m_bUseDDraw) {
		// Manages acceleration
	}
}

void RMGfxBuffer::OffsetY(int nLines, int nBpp) {
	m_buf += nLines * Dimx() * nBpp / 8;
}


RMGfxBuffer::operator byte *() {
	return m_buf;
}

RMGfxBuffer::operator void *() {
	return (void *)m_buf;
}

RMGfxBuffer::RMGfxBuffer(int dimx, int dimy, int nBpp, bool bUseDDraw) {
	Create(dimx, dimy, nBpp, bUseDDraw);
}

/****************************************************************************\
*       RMGfxSourceBuffer Methods
\****************************************************************************/

int RMGfxSourceBuffer::Init(const byte *buf, int dimx, int dimy, bool bLoadPalette) {
	Create(dimx, dimy, Bpp());
	CopyMemory(m_buf, buf, dimx * dimy * Bpp() / 8);

	// Invokes the method for preparing the surface (inherited)
	PrepareImage();

	return dimx * dimy * Bpp() / 8;
}


void RMGfxSourceBuffer::Init(RMDataStream &ds, int dimx, int dimy, bool bLoadPalette) {
	Create(dimx, dimy, Bpp());
	ds.Read(m_buf, dimx * dimy * Bpp() / 8);

	// Invokes the method for preparing the surface (inherited)
	PrepareImage();
}

RMGfxSourceBuffer::~RMGfxSourceBuffer() {
}

void RMGfxSourceBuffer::PrepareImage(void) {
	// Do nothing. Can be overloaded if necessary
}

bool RMGfxSourceBuffer::Clip2D(int &x1, int &y1, int &u, int &v, int &width, int &height, bool bUseSrc, RMGfxTargetBuffer *buf) {
	int destw, desth;

	destw = buf->Dimx();
	desth = buf->Dimy();

	if (!bUseSrc) {
		u = v = 0;
		width = m_dimx;
		height = m_dimy;
	}

	if (x1 > destw - 1)
		return false;
		
	if (y1 > desth - 1)
		return false;

	if (x1 < 0) {
		width += x1;
		if (width < 0)
			return false;
		u -= x1;
		x1 = 0;
	}

	if (y1 < 0) {
		height += y1;
		if (height < 0)
			return false;
		v -= y1;
		y1 = 0;
	}

	if (x1 + width - 1 > destw - 1)
		width = destw - x1;

	if (y1 + height - 1 > desth - 1)
		height = desth - y1;

	return true;
}


/**
 * Initialises a surface by resource Id
 *
 * @param resID					Resource ID
 * @param dimx					Buffer X dimension
 * @param dimy					Buffer Y dimension
 */
 int RMGfxSourceBuffer::Init(uint32 resID, int dimx, int dimy, bool bLoadPalette) {
	return Init(RMRes(resID), dimx, dimy, bLoadPalette);
}

/****************************************************************************\
*       RMGfxWoodyBuffer Methods
\****************************************************************************/

RMGfxWoodyBuffer::~RMGfxWoodyBuffer() {

}

void RMGfxWoodyBuffer::Draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	// Draw the OT list
	CORO_INVOKE_0(DrawOT);

	// Draw itself into the target buffer
	CORO_INVOKE_2(RMGfxSourceBuffer16::Draw, bigBuf, prim);

	CORO_END_CODE;
}

RMGfxWoodyBuffer::RMGfxWoodyBuffer() {

}

RMGfxWoodyBuffer::RMGfxWoodyBuffer(int dimx, int dimy, bool bUseDDraw)
	: RMGfxBuffer(dimx, dimy, 16, bUseDDraw) {

}


/****************************************************************************\
*       RMGfxTargetBuffer Methods
\****************************************************************************/

RMGfxClearTask RMGfxTargetBuffer::taskClear;

RMGfxTargetBuffer::RMGfxTargetBuffer() {
	otlist = NULL;
	m_otSize = 0;
//	csModifyingOT = g_system->createMutex();
}

RMGfxTargetBuffer::~RMGfxTargetBuffer() {
	ClearOT();
//	g_system->deleteMutex(csModifyingOT);
}

void RMGfxTargetBuffer::ClearOT(void) {
	OTList *cur, *n;

//	g_system->lockMutex(csModifyingOT);

	cur = otlist;

	while (cur != NULL) {
		cur->prim->m_task->Unregister();
		delete cur->prim;
		n = cur->next;
		delete cur;
		cur = n;
	}

	otlist = NULL;

//	g_system->unlockMutex(csModifyingOT);
}

void RMGfxTargetBuffer::DrawOT(CORO_PARAM) {
	CORO_BEGIN_CONTEXT;
	OTList *cur;
	OTList *prev;
	OTList *next;
	RMGfxPrimitive *myprim;
	bool result;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_ctx->prev = NULL;
	_ctx->cur = otlist;

	// Lock the buffer to access it
	Lock();
//	g_system->lockMutex(csModifyingOT);

	while (_ctx->cur != NULL) {
		// Call the task Draw method, passing it a copy of the original
		_ctx->myprim = _ctx->cur->prim->Duplicate();
		CORO_INVOKE_2(_ctx->cur->prim->m_task->Draw, *this, _ctx->myprim);
		delete _ctx->myprim;

		// Check if it's time to remove the task from the OT list
		CORO_INVOKE_1(_ctx->cur->prim->m_task->RemoveThis, _ctx->result);
		if (_ctx->result) {
			// De-register the task
			_ctx->cur->prim->m_task->Unregister();

			// Delete task, freeing the memory
			delete _ctx->cur->prim;
			_ctx->next = _ctx->cur->next;
			delete _ctx->cur;

			// If it was the first item, update the list head
			if (_ctx->prev == NULL)
				otlist = _ctx->next;
			// Otherwise update the next pinter of the previous item
			else
				_ctx->prev->next = _ctx->next;

			_ctx->cur = _ctx->next;
		} else {
			// Update the pointer to the previous item, and the current to the next
			_ctx->prev = _ctx->cur;
			_ctx->cur = _ctx->cur->next;
		}
	}

//	g_system->unlockMutex(csModifyingOT);

	//Unlock after writing
	Unlock();

	CORO_END_CODE;
}

void RMGfxTargetBuffer::AddPrim(RMGfxPrimitive *prim) {
	int nPrior;
	OTList *cur, *n;

//	g_system->lockMutex(csModifyingOT);

	// Warn of the OT listing
	prim->m_task->Register();

	// Check the priority
	nPrior = prim->m_task->Priority();
	n = new OTList(prim);

	// Empty list
	if (otlist == NULL) {
		otlist = n;
		otlist->next = NULL;
	}
	// Inclusion in the head
	else if (nPrior < otlist->prim->m_task->Priority()) {
		n->next = otlist;
		otlist = n;
	} else {
		cur = otlist;
		while (cur->next != NULL && nPrior > cur->next->prim->m_task->Priority())
			cur = cur->next;

		n->next = cur->next;
		cur->next = n;
	}

//	g_system->unlockMutex(csModifyingOT);
}

void RMGfxTargetBuffer::AddClearTask(void) {
	AddPrim(new RMGfxPrimitive(&taskClear));
}


/****************************************************************************\
*               RMGfxSourceBufferPal Methods
\****************************************************************************/

RMGfxSourceBufferPal::~RMGfxSourceBufferPal() {

}

int RMGfxSourceBufferPal::LoadPaletteWA(const byte *buf, bool bSwapped) {
	int i;

	if (bSwapped)
		for (i = 0; i < (1 << Bpp()); i++) {
			m_pal[i * 3 + 0] = buf[i * 3 + 2];
			m_pal[i * 3 + 1] = buf[i * 3 + 1];
			m_pal[i * 3 + 2] = buf[i * 3 + 0];
		}
	else
		CopyMemory(m_pal, buf, (1 << Bpp()) * 3);

	PreparePalette();

	return (1 << Bpp()) * 3;
}

int RMGfxSourceBufferPal::LoadPalette(const byte *buf) {
	int i;

	for (i = 0; i < 256; i++)
		CopyMemory(m_pal + i * 3, buf + i * 4, 3);

	PreparePalette();

	return (1 << Bpp()) * 4;
}


void RMGfxSourceBufferPal::PreparePalette(void) {
	int i;

	for (i = 0; i < 256; i++) {
		m_palFinal[i] = (((int)m_pal[i * 3 + 0] >> 3) <<  10) |
		                (((int)m_pal[i * 3 + 1] >> 3) <<  5) |
		                (((int)m_pal[i * 3 + 2] >> 3) <<  0);
	}
}

int RMGfxSourceBufferPal::Init(const byte *buf, int dimx, int dimy, bool bLoadPalette) {
	int read;

	// Load the RAW image
	read = RMGfxSourceBuffer::Init(buf, dimx, dimy);

	// Load the palette if necessary
	if (bLoadPalette)
		read += LoadPaletteWA(&buf[read]);

	return read;
}

void RMGfxSourceBufferPal::Init(RMDataStream &ds, int dimx, int dimy, bool bLoadPalette) {
	// Load the RAW image
	RMGfxSourceBuffer::Init(ds, dimx, dimy);

	// Load the palette if necessary
	if (bLoadPalette) {
		byte *suxpal = new byte[256 * 3];
		ds.Read(suxpal, 256 * 3);
		LoadPaletteWA(suxpal);
		delete[] suxpal;
	}
}

int RMGfxSourceBufferPal::LoadPalette(uint32 resID) {
	return LoadPalette(RMRes(resID));
}

int RMGfxSourceBufferPal::LoadPaletteWA(uint32 resID, bool bSwapped) {
	return LoadPaletteWA(RMRes(resID), bSwapped);
}

/****************************************************************************\
*               RMGfxSourceBuffer4 Methods
\****************************************************************************/

void RMGfxSourceBuffer4::Draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
}

RMGfxSourceBuffer4::RMGfxSourceBuffer4(int dimx, int dimy, bool bUseDDraw)
	: RMGfxBuffer(dimx, dimy, 4, bUseDDraw) {
	SetPriority(0);
}


/**
 * Returns the number of bits per pixel of the surface
 *
 * @returns		Bit per pixel
 */
int RMGfxSourceBuffer4::Bpp() {
	return 4;
}

void RMGfxSourceBuffer4::Create(int dimx, int dimy, bool bUseDDraw) {
	RMGfxBuffer::Create(dimx, dimy, 4, bUseDDraw);
}

/****************************************************************************\
*               RMGfxSourceBuffer8 Methods
\****************************************************************************/

RMGfxSourceBuffer8::~RMGfxSourceBuffer8() {

}

void RMGfxSourceBuffer8::Draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	int x, y, width, height, u, v;
	int bufx = bigBuf.Dimx();
	uint16 *buf = bigBuf;
	byte *raw = m_buf;

	// Destination buffer
	RMRect dst;
	if (prim->HaveDst())
		dst = prim->Dst();

	// Clipping
	if (prim->HaveSrc()) {
		u = prim->Src().x1;
		v = prim->Src().y1;

		width = prim->Src().Width();
		height = prim->Src().Height();
	}

	if (!Clip2D(dst.x1, dst.y1, u, v, width, height, prim->HaveSrc(), &bigBuf))
		return;

	// Starting offset into the buffer
	buf += dst.y1 * bufx + dst.x1;

	// Normal step
	if (m_bTrasp0) {
		for (y = 0; y < height; y++) {
			raw = m_buf + (y + v) * m_dimx + u;

			for (x = 0; x < width; x++) {
				if (*raw) *buf = m_palFinal[*raw];
				buf++;
				raw++;
			}

			buf += bufx - width;
		}
	} else {
		for (y = 0; y < height; y++) {
			raw = m_buf + (y + v) * m_dimx + u;

			for (x = 0; x < width; x += 2) {
				buf[0] = m_palFinal[raw[0]];
				buf[1] = m_palFinal[raw[1]];

				buf += 2;
				raw += 2;
			}

			buf += bufx - width;
		}
	}
}

RMGfxSourceBuffer8::RMGfxSourceBuffer8(int dimx, int dimy, bool bUseDDraw)
	: RMGfxBuffer(dimx, dimy, 8, bUseDDraw) {
	SetPriority(0);
}

RMGfxSourceBuffer8::RMGfxSourceBuffer8(bool bTrasp0) {
	m_bTrasp0 = bTrasp0;
}


/**
 * Returns the number of bits per pixel of the surface
 *
 * @returns		Bit per pixel
 */
int RMGfxSourceBuffer8::Bpp() {
	return 8;
}

void RMGfxSourceBuffer8::Create(int dimx, int dimy, bool bUseDDraw) {
	RMGfxBuffer::Create(dimx, dimy, 8, bUseDDraw);
}

#define GETRED(x)   (((x) >> 10) & 0x1F)
#define GETGREEN(x) (((x) >> 5) & 0x1F)
#define GETBLUE(x) ((x) & 0x1F)


/****************************************************************************\
*               RMGfxSourceBuffer8AB Methods
\****************************************************************************/

RMGfxSourceBuffer8AB::~RMGfxSourceBuffer8AB() {

}

int RMGfxSourceBuffer8AB::CalcTrasp(int fore, int back) {
	int r, g, b;

	r = (GETRED(fore) >> 2) + (GETRED(back) >> 1);
	g = (GETGREEN(fore) >> 2) + (GETGREEN(back) >> 1);
	b = (GETBLUE(fore) >> 2) + (GETBLUE(back) >> 1);

	if (r > 0x1F) r = 0x1F;
	if (g > 0x1F) g = 0x1F;
	if (b > 0x1F) b = 0x1F;

	return (r << 10) | (g << 5) | b;
}


void RMGfxSourceBuffer8AB::Draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	int x, y, width, height, u, v;
	int bufx = bigBuf.Dimx();
	uint16 *buf = bigBuf;
	byte *raw = m_buf;

	// Destination buffer
	RMRect dst;
	if (prim->HaveDst())
		dst = prim->Dst();

	// Clipping
	if (prim->HaveSrc()) {
		u = prim->Src().x1;
		v = prim->Src().y1;

		width = prim->Src().Width();
		height = prim->Src().Height();
	}

	if (!Clip2D(dst.x1, dst.y1, u, v, width, height, prim->HaveSrc(), &bigBuf))
		return;

	// Starting offset into the buffer
	buf += dst.y1 * bufx + dst.x1;

	// Passaggio normale
	if (m_bTrasp0) {
		for (y = 0; y < height; y++) {
			raw = m_buf + (y + v) * m_dimx + u;

			for (x = 0; x < width; x++) {
				if (*raw) *buf = CalcTrasp(m_palFinal[*raw], *buf);
				buf++;
				raw++;
			}

			buf += bufx - width;
		}
	} else {
		for (y = 0; y < height; y++) {
			raw = m_buf + (y + v) * m_dimx + u;

			for (x = 0; x < width; x += 2) {
				buf[0] = CalcTrasp(m_palFinal[raw[0]], buf[0]);
				buf[1] = CalcTrasp(m_palFinal[raw[1]], buf[1]);

				buf += 2;
				raw += 2;
			}

			buf += bufx - width;
		}
	}

	return;
}



/****************************************************************************\
*               RMGfxSourceBuffer8RLE Methods
\****************************************************************************/

byte RMGfxSourceBuffer8RLE::MegaRLEBuf[512 * 1024];

void RMGfxSourceBuffer8RLE::SetAlphaBlendColor(int color) {
	alphaBlendColor = color;
}

RMGfxSourceBuffer8RLE::RMGfxSourceBuffer8RLE() {
	alphaBlendColor = -1;
	bNeedRLECompress = true;
	m_buf = NULL;
}

RMGfxSourceBuffer8RLE::~RMGfxSourceBuffer8RLE() {
	if (m_buf != NULL) {
		delete[] m_buf;
		m_buf = NULL;
	}
}


int RMGfxSourceBuffer8RLE::Init(const byte *buf, int dimx, int dimy, bool bLoadPalette) {
	return RMGfxSourceBufferPal::Init(buf, dimx, dimy, bLoadPalette);
}

void RMGfxSourceBuffer8RLE::Init(RMDataStream &ds, int dimx, int dimy, bool bLoadPalette) {
	if (bNeedRLECompress) {
		RMGfxSourceBufferPal::Init(ds, dimx, dimy, bLoadPalette);
	} else {
		int size;

		ds >> size;
		m_buf = new byte[size];
		ds.Read(m_buf, size);

		m_dimx = dimx;
		m_dimy = dimy;
	}
}

void RMGfxSourceBuffer8RLE::PreparePalette(void) {
	// Invoke the parent method
	RMGfxSourceBuffer8::PreparePalette();

	// Handle RGB alpha blending
	if (alphaBlendColor != -1) {
		alphaR = (m_palFinal[alphaBlendColor] >> 10) & 0x1F;
		alphaG = (m_palFinal[alphaBlendColor] >> 5) & 0x1F;
		alphaB = (m_palFinal[alphaBlendColor]) & 0x1F;
	}
}

void RMGfxSourceBuffer8RLE::PrepareImage(void) {
	// Invoke the parent method
	RMGfxSourceBuffer::PrepareImage();

	// Compress
	CompressRLE();
}

void RMGfxSourceBuffer8RLE::SetAlreadyCompressed(void) {
	bNeedRLECompress = false;
}


void RMGfxSourceBuffer8RLE::CompressRLE(void) {
	int x, y;
	byte *startline;
	byte *cur;
	byte curdata;
	byte *src;
	byte *startsrc;
	int rep;

	// Perform RLE compression for lines
	cur = MegaRLEBuf;
	src = m_buf;
	for (y = 0; y < m_dimy; y++) {
		// Save the beginning of the line
		startline = cur;

		// Leave space for the length of the line
		cur += 2;

		// It starts from the empty space
		curdata = 0;
		rep = 0;
		startsrc = src;
		for (x = 0; x < m_dimx;) {
			if ((curdata == 0 && *src == 0) || (curdata == 1 && *src == alphaBlendColor)
			        || (curdata == 2 && (*src != alphaBlendColor && *src != 0))) {
				src++;
				rep++;
				x++;
			} else {
				if (curdata == 0) {
					RLEWriteTrasp(cur, rep);
					curdata++;
				} else if (curdata == 1) {
					RLEWriteAlphaBlend(cur, rep);
					curdata++;
				} else {
					RLEWriteData(cur, rep, startsrc);
					curdata = 0;
				}

				rep = 0;
				startsrc = src;
			}
		}

		// Pending data?
		if (curdata == 1) {
			RLEWriteAlphaBlend(cur, rep);
			RLEWriteData(cur, 0, NULL);
		}

		if (curdata == 2) {
			RLEWriteData(cur, rep, startsrc);
		}

		// End of line
		RLEWriteEOL(cur);

		// Write the length of the line
		WRITE_LE_UINT16(startline, (uint16)(cur - startline));
	}

	// Delete the original image
	delete[] m_buf;

	// Copy the compressed image
	x = cur - MegaRLEBuf;
	m_buf = new byte[x];
	Common::copy(MegaRLEBuf, MegaRLEBuf + x, m_buf);
}

void RMGfxSourceBuffer8RLE::Draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	int y;
	byte *src;
	uint16 *buf = bigBuf;
	int x1, y1, u, v, width, height;

	// Clipping
	x1 = prim->Dst().x1;
	y1 = prim->Dst().y1;
	if (!Clip2D(x1, y1, u, v, width, height, false, &bigBuf))
		return;

	// Go forward through the RLE lines
	src = m_buf;
	for (y = 0; y < v; y++)
		src += READ_LE_UINT16(src);

	// Calculate the position in the destination buffer
	buf += y1 * bigBuf.Dimx();

	// Loop
	if (prim->IsFlipped()) {
// Eliminate horizontal clipping
//		width = m_dimx;
//		x1=prim->Dst().x1;

		// Clipping
		u = m_dimx - (width + u);
		x1 = (prim->Dst().x1 + m_dimx - 1) - u;

		for (y = 0; y < height; y++) {
			// Decompressione
			RLEDecompressLineFlipped(buf + x1, src + 2, u, width);

			// Next line
			src += READ_LE_UINT16(src);

			// Skip to the next line
			buf += bigBuf.Dimx();
		}
	} else {
		for (y = 0; y < height; y++) {
			// Decompression
			RLEDecompressLine(buf + x1, src + 2, u, width);

			// Next line
			src += READ_LE_UINT16(src);

			// Skip to the next line
			buf += bigBuf.Dimx();
		}
	}
}


/****************************************************************************\
*               RMGfxSourceBuffer8RLEByte Methods
\****************************************************************************/

RMGfxSourceBuffer8RLEByte::~RMGfxSourceBuffer8RLEByte() {

}

void RMGfxSourceBuffer8RLEByte::RLEWriteTrasp(byte *&cur, int rep) {
	assert(rep < 255);
	*cur ++ = rep;
}

void RMGfxSourceBuffer8RLEByte::RLEWriteAlphaBlend(byte *&cur, int rep) {
	assert(rep < 255);
	*cur ++ = rep;
}

void RMGfxSourceBuffer8RLEByte::RLEWriteData(byte *&cur, int rep, byte *src) {
	assert(rep < 256);

	*cur ++ = rep;
	if (rep > 0) {
		CopyMemory(cur, src, rep);
		cur += rep;
		src += rep;
	}

	return;
}

void RMGfxSourceBuffer8RLEByte::RLEWriteEOL(byte *&cur) {
	*cur ++ = 0xFF;
}

void RMGfxSourceBuffer8RLEByte::RLEDecompressLine(uint16 *dst, byte *src, int nStartSkip, int nLength) {
	int i, n;
	int r, g, b;

	if (nStartSkip == 0)
		goto RLEByteDoTrasp;

	while (1) {
		assert(nStartSkip > 0);

		// TRASP
		n =  *src++;
		if (n == 0xFF)
			return;

		if (n >= nStartSkip) {
			dst += n - nStartSkip;
			nLength -= n - nStartSkip;
			if (nLength > 0)
				goto RLEByteDoAlpha;
			else
				return;
		}
		nStartSkip -= n;


		assert(nStartSkip > 0);

		// ALPHA
		n = *src++;
		if (n >= nStartSkip) {
			n -= nStartSkip;
			goto RLEByteDoAlpha2;
		}
		nStartSkip -= n;

		assert(nStartSkip > 0);

		// DATA
		n = *src++;
		if (n >= nStartSkip) {
			src += nStartSkip;
			n -= nStartSkip;
			goto RLEByteDoCopy2;
		}
		nStartSkip -= n;
		src += n;
	}


	while (1) {
RLEByteDoTrasp:
		// Get the trasp of s**t
		n = *src++;

		// EOL?
		if (n == 0xFF)
			return;

		dst += n;
		nLength -= n;
		if (nLength <= 0)
			return;

RLEByteDoAlpha:
		// Alpha
		n = *src++;

RLEByteDoAlpha2:
		if (n > nLength)
			n = nLength;
		for (i = 0; i < n; i++) {
			r = (*dst >> 10) & 0x1F;
			g = (*dst >> 5) & 0x1F;
			b = *dst & 0x1F;

			r = (r >> 2) + (alphaR >> 1);
			g = (g >> 2) + (alphaG >> 1);
			b = (b >> 2) + (alphaB >> 1);

			*dst ++ = (r << 10) | (g << 5) | b;
		}

		nLength -= n;
		if (!nLength)
			return;
		assert(nLength > 0);

//RLEByteDoCopy:
		// Copy the stuff
		n = *src++;

RLEByteDoCopy2:
		if (n > nLength)
			n = nLength;

		for (i = 0; i < n; i++)
			*dst ++ = m_palFinal[*src++];

		nLength -= n;
		if (!nLength)
			return;
		assert(nLength > 0);
	}
}

void RMGfxSourceBuffer8RLEByte::RLEDecompressLineFlipped(uint16 *dst, byte *src, int nStartSkip, int nLength) {
	int i, n;
	int r, g, b;

	if (nStartSkip == 0)
		goto RLEByteFlippedDoTrasp;

	while (1) {
		assert(nStartSkip > 0);

		// TRASP
		n = *src++;
		if (n == 0xFF)
			return;

		if (n >= nStartSkip) {
			dst -= n - nStartSkip;
			nLength -= n - nStartSkip;
			if (nLength > 0)
				goto RLEByteFlippedDoAlpha;
			else
				return;
		}
		nStartSkip -= n;


		assert(nStartSkip > 0);

		// ALPHA
		n = *src++;
		if (n >= nStartSkip) {
			n -= nStartSkip;
			goto RLEByteFlippedDoAlpha2;
		}
		nStartSkip -= n;

		assert(nStartSkip > 0);

		// DATA
		n = *src++;
		if (n >= nStartSkip) {
			src += nStartSkip;
			n -= nStartSkip;
			goto RLEByteFlippedDoCopy2;
		}
		nStartSkip -= n;
		src += n;
	}


	while (1) {
RLEByteFlippedDoTrasp:
		// Get the trasp of s**t
		n = *src++;

		// EOL?
		if (n == 0xFF)
			return;

		dst -= n;
		nLength -= n;
		if (nLength <= 0)
			return;

RLEByteFlippedDoAlpha:
		// Alpha
		n = *src++;

RLEByteFlippedDoAlpha2:
		if (n > nLength)
			n = nLength;
		for (i = 0; i < n; i++) {
			r = (*dst >> 10) & 0x1F;
			g = (*dst >> 5) & 0x1F;
			b = *dst & 0x1F;

			r = (r >> 2) + (alphaR >> 1);
			g = (g >> 2) + (alphaG >> 1);
			b = (b >> 2) + (alphaB >> 1);

			*dst-- = (r << 10) | (g << 5) | b;
		}

		nLength -= n;
		if (!nLength)
			return;
		assert(nLength > 0);

//RLEByteFlippedDoCopy:
		// Copy the data
		n = *src++;

RLEByteFlippedDoCopy2:
		if (n > nLength)
			n = nLength;

		for (i = 0; i < n; i++)
			*dst -- = m_palFinal[*src++];

		nLength -= n;
		if (!nLength)
			return;
		assert(nLength > 0);
	}
}


/****************************************************************************\
*               RMGfxSourceBuffer8RLEWord Methods
\****************************************************************************/

RMGfxSourceBuffer8RLEWord::~RMGfxSourceBuffer8RLEWord() {

}

void RMGfxSourceBuffer8RLEWord::RLEWriteTrasp(byte *&cur, int rep) {
	WRITE_LE_UINT16(cur, rep);
	cur += 2;
}

void RMGfxSourceBuffer8RLEWord::RLEWriteAlphaBlend(byte *&cur, int rep) {
	WRITE_LE_UINT16(cur, rep);
	cur += 2;
}

void RMGfxSourceBuffer8RLEWord::RLEWriteData(byte *&cur, int rep, byte *src) {
	WRITE_LE_UINT16(cur, rep);
	cur += 2;

	if (rep > 0) {
		CopyMemory(cur, src, rep);
		cur += rep;
		src += rep;
	}
}

void RMGfxSourceBuffer8RLEWord::RLEWriteEOL(byte *&cur) {
	*cur ++ = 0xFF;
	*cur ++ = 0xFF;
}

void RMGfxSourceBuffer8RLEWord::RLEDecompressLine(uint16 *dst, byte *src, int nStartSkip, int nLength) {
	int i, n;
	int r, g, b;

	if (nStartSkip == 0)
		goto RLEWordDoTrasp;

	while (1) {
		assert(nStartSkip > 0);

		// TRASP
		n = READ_LE_UINT16(src);
		src += 2;

		if (n == 0xFFFF)
			return;

		if (n >= nStartSkip) {
			dst += n - nStartSkip;
			nLength -= n - nStartSkip;

			if (nLength > 0)
				goto RLEWordDoAlpha;
			else
				return;
		}
		nStartSkip -= n;

		assert(nStartSkip > 0);

		// ALPHA
		n = READ_LE_UINT16(src);
		src += 2;

		if (n >= nStartSkip) {
			n -= nStartSkip;
			goto RLEWordDoAlpha2;
		}
		nStartSkip -= n;

		// DATA
		n = READ_LE_UINT16(src);
		src += 2;

		if (n >= nStartSkip) {
			src += nStartSkip;
			n -= nStartSkip;
			goto RLEWordDoCopy2;
		}
		nStartSkip -= n;
		src += n;
	}


	while (1) {
RLEWordDoTrasp:
		// Get the trasp of s**t
		n = READ_LE_UINT16(src);
		src += 2;

		// EOL?
		if (n == 0xFFFF)
			return;

		dst += n;

		nLength -= n;
		if (nLength <= 0)
			return;

RLEWordDoAlpha:
		n = READ_LE_UINT16(src);
		src += 2;

RLEWordDoAlpha2:

		if (n > nLength)
			n = nLength;

		for (i = 0; i < n; i++) {
			r = (*dst >> 10) & 0x1F;
			g = (*dst >> 5) & 0x1F;
			b = *dst & 0x1F;

			r = (r >> 2) + (alphaR >> 1);
			g = (g >> 2) + (alphaG >> 1);
			b = (b >> 2) + (alphaB >> 1);

			*dst++ = (r << 10) | (g << 5) | b;
		}

		nLength -= n;
		if (!nLength)
			return;

		assert(nLength > 0);

//RLEWordDoCopy:
		// Copy the data
		n = READ_LE_UINT16(src);
		src += 2;

RLEWordDoCopy2:
		if (n > nLength)
			n = nLength;

		for (i = 0; i < n; i++)
			*dst ++ = m_palFinal[*src++];

		nLength -= n;
		if (!nLength)
			return;

		assert(nLength > 0);

	}
}

void RMGfxSourceBuffer8RLEWord::RLEDecompressLineFlipped(uint16 *dst, byte *src, int nStartSkip, int nLength) {
	int i, n;
	int r, g, b;

	if (nStartSkip == 0)
		goto RLEWordFlippedDoTrasp;

	while (1) {
		assert(nStartSkip > 0);

		// TRASP
		n = READ_LE_UINT16(src);
		src += 2;

		if (n == 0xFFFF)
			return;

		if (n >= nStartSkip) {
			dst -= n - nStartSkip;
			nLength -= n - nStartSkip;

			if (nLength > 0)
				goto RLEWordFlippedDoAlpha;
			else
				return;
		}
		nStartSkip -= n;

		assert(nStartSkip > 0);

		// ALPHA
		n = READ_LE_UINT16(src);
		src += 2;

		if (n >= nStartSkip) {
			n -= nStartSkip;
			goto RLEWordFlippedDoAlpha2;
		}
		nStartSkip -= n;

		// DATA
		n = READ_LE_UINT16(src);
		src += 2;

		if (n >= nStartSkip) {
			src += nStartSkip;
			n -= nStartSkip;
			goto RLEWordFlippedDoCopy2;
		}
		nStartSkip -= n;
		src += n;
	}


	while (1) {
RLEWordFlippedDoTrasp:
		// Get the trasp of s**t
		n = READ_LE_UINT16(src);
		src += 2;

		// EOL?
		if (n == 0xFFFF)
			return;

		dst -= n;

		nLength -= n;
		if (nLength <= 0)
			return;

RLEWordFlippedDoAlpha:
		n = READ_LE_UINT16(src);
		src += 2;

RLEWordFlippedDoAlpha2:

		if (n > nLength)
			n = nLength;

		for (i = 0; i < n; i++) {
			r = (*dst >> 10) & 0x1F;
			g = (*dst >> 5) & 0x1F;
			b = *dst & 0x1F;

			r = (r >> 2) + (alphaR >> 1);
			g = (g >> 2) + (alphaG >> 1);
			b = (b >> 2) + (alphaB >> 1);

			*dst-- = (r << 10) | (g << 5) | b;
		}

		nLength -= n;
		if (!nLength)
			return;

		assert(nLength > 0);

//RLEWordFlippedDoCopy:
		// Copy the data
		n = READ_LE_UINT16(src);
		src += 2;

RLEWordFlippedDoCopy2:
		if (n > nLength)
			n = nLength;

		for (i = 0; i < n; i++)
			*dst -- = m_palFinal[*src++];

		nLength -= n;
		if (!nLength)
			return;

		assert(nLength > 0);
	}
}

/****************************************************************************\
*               Metodi di RMGfxSourceBuffer8RLEWord
\****************************************************************************/

RMGfxSourceBuffer8RLEWordAB::~RMGfxSourceBuffer8RLEWordAB() {

}

void RMGfxSourceBuffer8RLEWordAB::RLEDecompressLine(uint16 *dst, byte *src,  int nStartSkip, int nLength) {
	int i, n;
	int r, g, b, r2, g2, b2;

	if (!GLOBALS.bCfgTransparence) {
		RMGfxSourceBuffer8RLEWord::RLEDecompressLine(dst, src, nStartSkip, nLength);
		return;
	}

	if (nStartSkip == 0)
		goto RLEWordDoTrasp;

	while (1) {
		assert(nStartSkip > 0);

		// TRASP
		n = READ_LE_UINT16(src);
		src += 2;

		if (n == 0xFFFF)
			return;

		if (n >= nStartSkip) {
			dst += n - nStartSkip;
			nLength -= n - nStartSkip;

			if (nLength > 0)
				goto RLEWordDoAlpha;
			else
				return;
		}
		nStartSkip -= n;

		assert(nStartSkip > 0);

		// ALPHA
		n = READ_LE_UINT16(src);
		src += 2;

		if (n >= nStartSkip) {
			n -= nStartSkip;
			goto RLEWordDoAlpha2;
		}
		nStartSkip -= n;

		// DATA
		n = READ_LE_UINT16(src);
		src += 2;

		if (n >= nStartSkip) {
			src += nStartSkip;
			n -= nStartSkip;
			goto RLEWordDoCopy2;
		}
		nStartSkip -= n;
		src += n;
	}


	while (1) {
RLEWordDoTrasp:
		// Get the trasp of s**t
		n = READ_LE_UINT16(src);
		src += 2;

		// EOL?
		if (n == 0xFFFF)
			return;

		dst += n;

		nLength -= n;
		if (nLength <= 0)
			return;

RLEWordDoAlpha:
		n = READ_LE_UINT16(src);
		src += 2;

RLEWordDoAlpha2:

		if (n > nLength)
			n = nLength;

		// @@@ SHOULD NOT BE THERE !!!!!
		for (i = 0; i < n; i++) {
			r = (*dst >> 10) & 0x1F;
			g = (*dst >> 5) & 0x1F;
			b = *dst & 0x1F;

			r = (r >> 2) + (alphaR >> 1);
			g = (g >> 2) + (alphaG >> 1);
			b = (b >> 2) + (alphaB >> 1);

			*dst++ = (r << 10) | (g << 5) | b;
		}

		nLength -= n;
		if (!nLength)
			return;

		assert(nLength > 0);

//RLEWordDoCopy:
		// Copy the data
		n = READ_LE_UINT16(src);
		src += 2;

RLEWordDoCopy2:
		if (n > nLength)
			n = nLength;

		for (i = 0; i < n; i++) {
			r = (*dst >> 10) & 0x1F;
			g = (*dst >> 5) & 0x1F;
			b = *dst & 0x1F;

			r2 = (m_palFinal[*src] >> 10) & 0x1F;
			g2 = (m_palFinal[*src] >> 5) & 0x1F;
			b2 = m_palFinal[*src] & 0x1F;

			r = (r >> 1) + (r2 >> 1);
			g = (g >> 1) + (g2 >> 1);
			b = (b >> 1) + (b2 >> 1);

			*dst ++ = (r << 10) | (g << 5) | b;
			src++;
		}

		nLength -= n;
		if (!nLength)
			return;

		assert(nLength > 0);

	}
}


/****************************************************************************\
*               Metodi di RMGfxSourceBuffer8AA
\****************************************************************************/

byte RMGfxSourceBuffer8AA::MegaAABuf[256 * 1024];
byte RMGfxSourceBuffer8AA::MegaAABuf2[64 * 1024];

void RMGfxSourceBuffer8AA::PrepareImage(void) {
	// Invoke the parent method
	RMGfxSourceBuffer::PrepareImage();

	// Prepare the buffer for anti-aliasing
	CalculateAA();
}


void RMGfxSourceBuffer8AA::CalculateAA(void) {
	// I suck, you suck, he sucks, we suck, they all suck  ---> ANTI ALIASING SUX!
	// ************************************************************

	int x, y;
	byte *src, *srcaa;

	/* First pass: fill the edges */
	Common::fill(MegaAABuf, MegaAABuf + m_dimx * m_dimy, 0);

	src = m_buf;
	srcaa = MegaAABuf;
	for (y = 0; y < m_dimy; y++) {
		for (x = 0; x < m_dimx; x++) {
			if (*src == 0) {
				if ((y > 0 && src[-m_dimx] != 0) ||
				        (y < m_dimy - 1 && src[m_dimx] != 0) ||
				        (x > 0 && src[-1] != 0) ||
				        (x < m_dimx - 1 && src[1] != 0))
					*srcaa = 1;
			}

			src++;
			srcaa++;
		}
	}

	src = m_buf;
	srcaa = MegaAABuf;
	for (y = 0; y < m_dimy; y++) {
		for (x = 0; x < m_dimx; x++) {
			if (*src != 0) {
				if ((y > 0 && srcaa[-m_dimx] == 1) ||
				        (y < m_dimy - 1 && srcaa[m_dimx] == 1) ||
				        (x > 0 && srcaa[-1] == 1) ||
				        (x < m_dimx - 1 && srcaa[1] == 1))
					*srcaa = 2;
			}

			src++;
			srcaa++;
		}
	}

	if (m_aabuf != NULL)
		delete[] m_aabuf;

	m_aabuf = new byte[m_dimx * m_dimy];
	CopyMemory(m_aabuf, MegaAABuf, m_dimx * m_dimy);
}

RMGfxSourceBuffer8AA::RMGfxSourceBuffer8AA() : RMGfxSourceBuffer8() {
	m_aabuf = NULL;
}

RMGfxSourceBuffer8AA::~RMGfxSourceBuffer8AA() {
	if (m_aabuf != NULL)
		delete[] m_aabuf;
}

void RMGfxSourceBuffer8AA::DrawAA(RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	int x, y;
	byte *src;
	uint16 *mybuf;
	uint16 *buf;
	int x1, y1, u, v, width, height;
	int r, g, b;
	int step;

	// Clip the sprite
	x1 = prim->Dst().x1;
	y1 = prim->Dst().y1;
	if (!Clip2D(x1, y1, u, v, width, height, false, &bigBuf))
		return;

	// Go forward through the RLE lines
	src = m_buf;
	for (y = 0; y < v; y++)
		src += READ_LE_UINT16(src);

	// Eliminate horizontal clipping

	if (prim->IsFlipped()) {
		u = m_dimx - (width + u);
		x1 = (prim->Dst().x1 + m_dimx - 1) - u;
	}
//	width = m_dimx;
//	x1=prim->Dst().x1;


	// Poisition into the destination buffer
	buf = bigBuf;
	buf += y1 * bigBuf.Dimx();

	if (prim->IsFlipped())
		step = -1;
	else
		step = 1;

	// Loop
	buf += bigBuf.Dimx(); // Skip the first line
	for (y = 1; y < height - 1; y++) {
		/*
		        if (prim->IsFlipped())
		            mybuf=&buf[x1+m_dimx-1];
		        else
		*/
		mybuf = &buf[x1];

		for (x = 0; x < width; x++, mybuf += step)
			if (m_aabuf[(y + v) * m_dimx + x + u] == 2 && x != 0 && x != width - 1) {
				r = GETRED(mybuf[1]) + GETRED(mybuf[-1]) + GETRED(mybuf[-bigBuf.Dimx()]) + GETRED(mybuf[bigBuf.Dimx()]);
				g = GETGREEN(mybuf[1]) + GETGREEN(mybuf[-1]) + GETGREEN(mybuf[-bigBuf.Dimx()]) + GETGREEN(mybuf[bigBuf.Dimx()]);
				b = GETBLUE(mybuf[1]) + GETBLUE(mybuf[-1]) + GETBLUE(mybuf[-bigBuf.Dimx()]) + GETBLUE(mybuf[bigBuf.Dimx()]);

				r += GETRED(mybuf[0]);
				g += GETGREEN(mybuf[0]);
				b += GETBLUE(mybuf[0]);

				r /= 5;
				g /= 5;
				b /= 5;

				if (r > 31) r = 31;
				if (g > 31) g = 31;
				if (b > 31) b = 31;

				mybuf[0] = (r << 10) | (g << 5) | b;
			}

		// Skip to the next line
		buf += bigBuf.Dimx();
	}

// Position into the destination buffer
	buf = bigBuf;
	buf += y1 * bigBuf.Dimx();

	// Looppone
	buf += bigBuf.Dimx();
	for (y = 1; y < height - 1; y++) {
		/*
		        if (prim->IsFlipped())
		            mybuf=&buf[x1+m_dimx-1];
		        else
		*/
		mybuf = &buf[x1];

		for (x = 0; x < width; x++, mybuf += step)
			if (m_aabuf[(y + v) * m_dimx + x + u] == 1 && x != 0 && x != width - 1) {
				r = GETRED(mybuf[1]) + GETRED(mybuf[-1]) + GETRED(mybuf[-bigBuf.Dimx()]) + GETRED(mybuf[bigBuf.Dimx()]);
				g = GETGREEN(mybuf[1]) + GETGREEN(mybuf[-1]) + GETGREEN(mybuf[-bigBuf.Dimx()]) + GETGREEN(mybuf[bigBuf.Dimx()]);
				b = GETBLUE(mybuf[1]) + GETBLUE(mybuf[-1]) + GETBLUE(mybuf[-bigBuf.Dimx()]) + GETBLUE(mybuf[bigBuf.Dimx()]);

				r += GETRED(mybuf[0]) * 2;
				g += GETGREEN(mybuf[0]) * 2;
				b += GETBLUE(mybuf[0]) * 2;

				r /= 6;
				g /= 6;
				b /= 6;

				if (r > 31) r = 31;
				if (g > 31) g = 31;
				if (b > 31) b = 31;

				mybuf[0] = (r << 10) | (g << 5) | b;
			}

		// Skippa alla linea successiva
		buf += bigBuf.Dimx();
	}
}



void RMGfxSourceBuffer8AA::Draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_2(RMGfxSourceBuffer8::Draw, bigBuf, prim);
	DrawAA(bigBuf, prim);

	CORO_END_CODE;
}


/****************************************************************************\
*               RMGfxSourceBuffer8RLEAA Methods
\****************************************************************************/

RMGfxSourceBuffer8RLEByteAA::~RMGfxSourceBuffer8RLEByteAA() {

}

void RMGfxSourceBuffer8RLEByteAA::PrepareImage(void) {
	RMGfxSourceBuffer::PrepareImage();
	CalculateAA();
	CompressRLE();
}

void RMGfxSourceBuffer8RLEByteAA::Draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_2(RMGfxSourceBuffer8RLE::Draw, bigBuf, prim);
	if (GLOBALS.bCfgAntiAlias)
		DrawAA(bigBuf, prim);

	CORO_END_CODE;
}

int RMGfxSourceBuffer8RLEByteAA::Init(const byte *buf, int dimx, int dimy, bool bLoadPalette) {
	return RMGfxSourceBuffer8RLE::Init(buf, dimx, dimy, bLoadPalette);
}

void RMGfxSourceBuffer8RLEByteAA::Init(RMDataStream &ds, int dimx, int dimy, bool bLoadPalette) {
	RMGfxSourceBuffer8RLE::Init(ds, dimx, dimy, bLoadPalette);

	if (!bNeedRLECompress) {
		// Load the anti-aliasing mask
		m_aabuf = new byte[dimx * dimy];
		ds.Read(m_aabuf, dimx * dimy);
	}
}


RMGfxSourceBuffer8RLEWordAA::~RMGfxSourceBuffer8RLEWordAA() {

}

void RMGfxSourceBuffer8RLEWordAA::PrepareImage(void) {
	RMGfxSourceBuffer::PrepareImage();
	CalculateAA();
	CompressRLE();
}

void RMGfxSourceBuffer8RLEWordAA::Draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_2(RMGfxSourceBuffer8RLE::Draw, bigBuf, prim);
	if (GLOBALS.bCfgAntiAlias)
		DrawAA(bigBuf, prim);

	CORO_END_CODE;
}

int RMGfxSourceBuffer8RLEWordAA::Init(byte *buf, int dimx, int dimy, bool bLoadPalette) {
	return RMGfxSourceBuffer8RLE::Init(buf, dimx, dimy, bLoadPalette);
}

void RMGfxSourceBuffer8RLEWordAA::Init(RMDataStream &ds, int dimx, int dimy, bool bLoadPalette) {
	RMGfxSourceBuffer8RLE::Init(ds, dimx, dimy, bLoadPalette);

	if (!bNeedRLECompress) {
		// Load the anti-aliasing mask
		m_aabuf = new byte[dimx * dimy];
		ds.Read(m_aabuf, dimx * dimy);
	}
}


/****************************************************************************\
*               RMGfxSourceBuffer16 Methods
\****************************************************************************/

RMGfxSourceBuffer16::RMGfxSourceBuffer16(bool bTrasp0) {
	m_bTrasp0 = bTrasp0;
}

RMGfxSourceBuffer16::~RMGfxSourceBuffer16() {
}

void RMGfxSourceBuffer16::Draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	int x, y;
	uint16 *buf = bigBuf;
	uint16 *raw = (uint16 *)m_buf;
	int dimx, dimy;
	int u, v;
	int x1, y1;

	dimx = m_dimx;
	dimy = m_dimy;
	u = 0;
	v = 0;
	x1 = 0;
	y1 = 0;

	if (prim->HaveSrc()) {
		u = prim->Src().x1;
		v = prim->Src().y1;
		dimx = prim->Src().Width();
		dimy = prim->Src().Height();
	}

	if (prim->HaveDst()) {
		x1 = prim->Dst().x1;
		y1 = prim->Dst().y1;
	}

	if (!Clip2D(x1, y1, u, v, dimx, dimy, true, &bigBuf))
		return;

	raw += v * m_dimx + u;
	buf += y1 * bigBuf.Dimx() + x1;

	if (m_bTrasp0) {
		for (y = 0; y < dimy; y++) {
			for (x = 0; x < dimx;) {
				while (x < dimx && raw[x] == 0)
					x++;

				while (x < dimx && raw[x] != 0) {
					buf[x] = raw[x];
					x++;
				}
			}

			raw += m_dimx;
			buf += bigBuf.Dimx();
		}
	} else {
		for (y = 0; y < dimy; y++) {
			Common::copy(raw, raw + dimx, buf);
			buf += bigBuf.Dimx();
			raw += m_dimx;
		}
	}
}

void RMGfxSourceBuffer16::PrepareImage(void) {
	// Colour space conversion if necessary!
	int i;
	uint16 *buf = (uint16 *)m_buf;

	for (i = 0; i < m_dimx * m_dimy; i++)
		WRITE_LE_UINT16(&buf[i], FROM_LE_16(buf[i]) & 0x7FFF);
}


RMGfxSourceBuffer16::RMGfxSourceBuffer16(int dimx, int dimy, bool bUseDDraw)
	: RMGfxBuffer(dimx, dimy, 16, bUseDDraw) {
	SetPriority(0);
}


/**
 * Returns the number of bits per pixel of the surface
 *
 * @returns		Bit per pixel
 */
int RMGfxSourceBuffer16::Bpp() {
	return 16;
}

void RMGfxSourceBuffer16::Create(int dimx, int dimy, bool bUseDDraw) {
	RMGfxBuffer::Create(dimx, dimy, 16, bUseDDraw);
}

/****************************************************************************\
*               RMGfxBox Methods
\****************************************************************************/

void RMGfxBox::RemoveThis(CORO_PARAM, bool &result) {
	result = true;
}

void RMGfxBox::SetColor(byte r, byte g, byte b) {
	r >>= 3;
	g >>= 3;
	b >>= 3;
	wFillColor = (r << 10) | (g << 5) | b;
}

void RMGfxBox::Draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	int i, j;
	uint16 *buf = bigBuf;
	RMRect rcDst;

	// It takes the destination rectangle
	rcDst = prim->Dst();
	buf += rcDst.y1 * bigBuf.Dimx() + rcDst.x1;

	// Loop through the pixels
	for (j = 0; j < rcDst.Height(); j++) {
		for (i = 0; i < rcDst.Width(); i++)
			*buf ++ = wFillColor;

		buf += bigBuf.Dimx() - rcDst.Width();
	}
}


/****************************************************************************\
*       RMGfxClearTask Methods
\****************************************************************************/

int RMGfxClearTask::Priority() {
	// Maximum priority (must be done first)
	return 1;
}

void RMGfxClearTask::Draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *) {
	// Clean the target buffer
	Common::fill((byte *)bigBuf, (byte *)bigBuf + (bigBuf.Dimx() * bigBuf.Dimy() * 2), 0x0);
}

void RMGfxClearTask::RemoveThis(CORO_PARAM, bool &result) {
	// The task is fine to be removed
	result = true;
}

} // End of namespace Tony
