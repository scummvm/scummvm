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

#ifndef TONY_GFXCORE_H
#define TONY_GFXCORE_H

#include "common/system.h"
#include "common/coroutines.h"
#include "tony/utils.h"

namespace Tony {

/****************************************************************************\
*       Prototipi di classi
\****************************************************************************/

//    Class Name                Family Treee            Abstract?
class RMGfxTask;             //                             Yes
class RMGfxTaskSetPrior;     //     Task                    Yes
class RMGfxBuffer;           //
class RMGfxSourceBuffer;     //     TaskP+[Buffer]          Yes
class RMGfxTargetBuffer;     //     [Buffer]
class RMGfxSourceBufferPal;  //     Source                  Yes
class RMGfxSourceBuffer4;    //     SourcePal
class RMGfxSourceBuffer8;    //     SourcePal
class RMGfxSourceBuffer16;   //     Source
class RMGfxWoodyBuffer;      //     Source16+Target
class RMGfxClearTask;        //     Task


/**
 * Graphics buffer
 */
class RMGfxBuffer {
protected:
	int m_dimx, m_dimy;
	byte *m_buf;
	byte *m_origBuf;
	bool m_bUseDDraw;

public:
	RMGfxBuffer();
	RMGfxBuffer(int dimx, int dimy, int nBpp, bool bUseDDraw = false);
	virtual ~RMGfxBuffer();

	// Attributes
	int Dimx() {
		return m_dimx;
	}
	int Dimy() {
		return m_dimy;
	}

	// Creation
	virtual void Create(int dimx, int dimy, int nBpp, bool bUseDDraw = false);
	virtual void Destroy(void);

	// Buffer access
	void Lock(void);
	void Unlock(void);

	// These are valid only if the buffer is locked
	operator byte *();
	operator void *();

	// Getting the offset for a given Y position
	void OffsetY(int nLines, int nBpp);
};

/**
 * Graphics primitive
 */
class RMGfxPrimitive {
public:
	RMGfxTask *m_task;

protected:
	RMRect m_src;
	RMRect m_dst;

	bool m_bStretch;
	byte m_bFlag;

public:
	RMGfxPrimitive() {
		m_bFlag = 0;
		m_task = NULL;
		m_src.SetEmpty();
		m_dst.SetEmpty();
	}

	RMGfxPrimitive(RMGfxTask *task) {
		m_task = task;
		m_bFlag = 0;
	}

	RMGfxPrimitive(RMGfxTask *task, const RMRect &src, RMRect &dst) {
		m_task = task;
		m_src = src;
		m_dst = dst;
		m_bFlag = 0;
		m_bStretch = (src.Width() != dst.Width() || src.Height() != dst.Height());
	}

	RMGfxPrimitive(RMGfxTask *task, const RMPoint &src, RMRect &dst) {
		m_task = task;
		m_src.TopLeft() = src;
		m_dst = dst;
		m_bFlag = 0;
	}

	RMGfxPrimitive(RMGfxTask *task, const RMPoint &src, RMPoint &dst) {
		m_task = task;
		m_src.TopLeft() = src;
		m_dst.TopLeft() = dst;
		m_bFlag = 0;
	}

	RMGfxPrimitive(RMGfxTask *task, const RMRect &src, RMPoint &dst) {
		m_task = task;
		m_src = src;
		m_dst.TopLeft() = dst;
		m_bFlag = 0;
	}

	RMGfxPrimitive(RMGfxTask *task, const RMRect &dst) {
		m_task = task;
		m_dst = dst;
		m_src.SetEmpty();
		m_bFlag = 0;
	}

	RMGfxPrimitive(RMGfxTask *task, const RMPoint &dst) {
		m_task = task;
		m_dst.TopLeft() = dst;
		m_src.SetEmpty();
		m_bFlag = 0;
	}

	virtual ~RMGfxPrimitive() { }

	void SetFlag(byte bFlag)        {
		m_bFlag = bFlag;
	}
	void SetTask(RMGfxTask *task)   {
		m_task = task;
	}
	void SetSrc(const RMRect &src)        {
		m_src = src;
	}
	void SetSrc(const RMPoint &src)       {
		m_src.TopLeft() = src;
	}
	void SetDst(const RMRect &dst)        {
		m_dst = dst;
	}
	void SetDst(const RMPoint &dst)       {
		m_dst.TopLeft() = dst;
	}
	void SetStrecth(bool bStretch)  {
		m_bStretch = bStretch;
	}

	bool HaveDst()                  {
		return !m_dst.IsEmpty();
	}
	RMRect &Dst()                   {
		return m_dst;
	}

	bool HaveSrc()                  {
		return !m_src.IsEmpty();
	}
	RMRect &Src()                   {
		return m_src;
	}

	// Flags
	bool IsFlipped()                {
		return m_bFlag & 1;
	}

	// Duplicate
	virtual RMGfxPrimitive *Duplicate() {
		return new RMGfxPrimitive(*this);
	}
};


/**
 * Graphic drawing task
 */
class RMGfxTask {
protected:
	int m_nPrior;
	int m_nInList;

public:
	// Standard constructor
	RMGfxTask();
	virtual ~RMGfxTask() { }

	virtual int Priority();
	virtual void Draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) = 0;
	virtual void RemoveThis(CORO_PARAM, bool &result);

	// Registration
	virtual void Register(void) {
		m_nInList++;
	}
	virtual void Unregister(void) {
		m_nInList--;
		assert(m_nInList >= 0);
	}
};


/**
 * Graphic drawing with priority
 */
class RMGfxTaskSetPrior : public RMGfxTask {
public:
	virtual ~RMGfxTaskSetPrior() { }
	void SetPriority(int nPrior);
};


/**
 * Task that cleans the destination buffer
 */
class RMGfxClearTask : public RMGfxTask {
public:
	virtual ~RMGfxClearTask() { }

	int Priority();
	virtual void Draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim);
	virtual void RemoveThis(CORO_PARAM, bool &result);
};


/**
 * Task that draws a coloured box
 */
class RMGfxBox : public RMGfxTaskSetPrior {
protected:
	uint16 wFillColor;

public:
	virtual ~RMGfxBox() { }

	void SetColor(byte r, byte g, byte b);
	virtual void Draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim);
	virtual void RemoveThis(CORO_PARAM, bool &result);
};


/**
 * Buffer source for the design, which is a task. This is an abstract base.
 */
class RMGfxSourceBuffer : public virtual RMGfxBuffer, public RMGfxTaskSetPrior {
public:
	// Load the data for the surface
	virtual int Init(uint32 resID, int dimx, int dimy, bool bLoadPalette = false);
	virtual int Init(const byte *buf, int dimx, int dimy, bool bLoadPalette = false);
	virtual void Init(RMDataStream &ds, int dimx, int dimy, bool bLoadPalette = false);

	virtual ~RMGfxSourceBuffer();

protected:
	virtual void PrepareImage(void);
	bool Clip2D(int &x1, int &y1, int &u, int &v, int &width, int &height, bool bUseSrc, RMGfxTargetBuffer *buf);
	void OffsetY(int nLines) {
		RMGfxBuffer::OffsetY(nLines, Bpp());
	}

public:
	virtual int Bpp() = 0;
};


/**
 * 16-bit colour source
 */
class RMGfxSourceBuffer16 : public RMGfxSourceBuffer {
protected:
	virtual void PrepareImage(void);
	bool m_bTrasp0;

public:
	RMGfxSourceBuffer16(bool bUseTrasp = false);
	RMGfxSourceBuffer16(int dimx, int dimy, bool bUseDDraw = false);
	virtual ~RMGfxSourceBuffer16();

	// Initialisation
	void Create(int dimx, int dimy, bool bUseDDraw = false);

	int Bpp();
	virtual void Draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim);
};


/**
 * Buffer source with palette
 */
class RMGfxSourceBufferPal : public RMGfxSourceBuffer {
protected:
	// The size of the palette is  (1 << Bpp()) * 4
	byte m_pal[256 * 3];
	uint16 m_palFinal[256];

	// Post process to prepare the palette for drawing
	virtual void PreparePalette(void);

public:
	virtual ~RMGfxSourceBufferPal();

	virtual int Init(const byte *buf, int dimx, int dimy, bool bLoadPalette = false);
	virtual void Init(RMDataStream &ds, int dimx, int dimy, bool bLoadPalette = false);

	int LoadPaletteWA(uint32 resID, bool bSwapped = false);
	int LoadPaletteWA(const byte *buf, bool bSwapped = false);
	int LoadPalette(uint32 resID);
	int LoadPalette(const byte *buf);
};


/**
 * Buffer source with a 256 colour palette
 */
class RMGfxSourceBuffer8 : public RMGfxSourceBufferPal {
protected:
	bool m_bTrasp0;

public:
	RMGfxSourceBuffer8(bool bTrasp0 = true);
	RMGfxSourceBuffer8(int dimx, int dimy, bool bUseDDraw = false);
	virtual ~RMGfxSourceBuffer8();

	// Initialisation
	void Create(int dimx, int dimy, bool bUseDDraw = false);

	int Bpp();
	virtual void Draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim);
};


/**
 * Buffer source with a 256 colour palette, and alpha blending
 */
class RMGfxSourceBuffer8AB : public RMGfxSourceBuffer8 {
protected:
	int CalcTrasp(int f, int b);

public:
	virtual ~RMGfxSourceBuffer8AB();
	virtual void Draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim);
};


/**
 * Buffer source with a 256 colour palette, RLE compressed
 */

class RMGfxSourceBuffer8RLE : public virtual RMGfxSourceBuffer8 {
protected:
	int alphaBlendColor;
	int alphaR, alphaB, alphaG;
	bool bNeedRLECompress;

protected:
	static byte MegaRLEBuf[];

	virtual void RLEWriteTrasp(byte *&cur, int rep) = 0;
	virtual void RLEWriteData(byte *&cur, int rep, byte *src) = 0;
	virtual void RLEWriteEOL(byte *&cur) = 0;
	virtual void RLEWriteAlphaBlend(byte *&cur, int rep) = 0;
	virtual void RLEDecompressLine(uint16 *dst, byte *src, int nStartSkip, int nLength) = 0;
	virtual void RLEDecompressLineFlipped(uint16 *dst, byte *src, int nStartSkip, int nLength) = 0;

	// Perform image compression in RLE
	void CompressRLE(void);

protected:
	// Overriding initialisation methods
	virtual void PrepareImage(void);
	virtual void PreparePalette(void);

public:
	RMGfxSourceBuffer8RLE();
	virtual ~RMGfxSourceBuffer8RLE();

	// Overload of the initialisation method
	virtual void Init(RMDataStream &ds, int dimx, int dimy, bool bLoadPalette = false);
	virtual int Init(const byte *buf, int dimx, int dimy, bool bLoadPalette = false);

	// Draw image with RLE decompression
	virtual void Draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim);

	// Sets the color that will be alpha blended
	void SetAlphaBlendColor(int color);

	// Warn if the data is already compressed
	void SetAlreadyCompressed(void);
};

class RMGfxSourceBuffer8RLEByte : public RMGfxSourceBuffer8RLE {
protected:
	void RLEWriteTrasp(byte *  &cur, int rep);
	void RLEWriteAlphaBlend(byte *  &cur, int rep);
	void RLEWriteData(byte *  &cur, int rep, byte *src);
	void RLEWriteEOL(byte *  &cur);
	void RLEDecompressLine(uint16 *dst, byte *src, int nStartSkip, int nLength);
	void RLEDecompressLineFlipped(uint16 *dst, byte *src, int nStartSkip, int nLength);

public:
	virtual ~RMGfxSourceBuffer8RLEByte();
};

class RMGfxSourceBuffer8RLEWord : public RMGfxSourceBuffer8RLE {
protected:
	void RLEWriteTrasp(byte *  &cur, int rep);
	void RLEWriteAlphaBlend(byte *  &cur, int rep);
	void RLEWriteData(byte *  &cur, int rep, byte *src);
	void RLEWriteEOL(byte *  &cur);
	virtual void RLEDecompressLine(uint16 *dst, byte *src, int nStartSkip, int nLength);
	virtual void RLEDecompressLineFlipped(uint16 *dst, byte *src, int nStartSkip, int nLength);

public:
	virtual ~RMGfxSourceBuffer8RLEWord();
};

class RMGfxSourceBuffer8RLEWordAB : public RMGfxSourceBuffer8RLEWord {
protected:
	virtual void RLEDecompressLine(uint16 *dst, byte *src, int nStartSkip, int nLength);

public:
	virtual ~RMGfxSourceBuffer8RLEWordAB();
};


/**
 * Buffer source with a 256 color palette, with anti-aliasing
 */
class RMGfxSourceBuffer8AA : public virtual RMGfxSourceBuffer8 {
protected:
	static byte MegaAABuf[];
	static byte MegaAABuf2[];
	byte *m_aabuf;

	// Calculate the buffer for the anti-aliasing
	void CalculateAA(void);

	// Draw the AA
	void DrawAA(RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim);

protected:
	void PrepareImage(void);

public:
	RMGfxSourceBuffer8AA();
	virtual ~RMGfxSourceBuffer8AA();

	// Draw with anti-aliasing
	virtual void Draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim);
};


class RMGfxSourceBuffer8RLEByteAA : public RMGfxSourceBuffer8RLEByte, public RMGfxSourceBuffer8AA {
protected:
	void PrepareImage(void);

public:
	virtual void Draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim);

	// Overloaded initialisation methods
	virtual void Init(RMDataStream &ds, int dimx, int dimy, bool bLoadPalette = false);
	virtual int Init(const byte *buf, int dimx, int dimy, bool bLoadPalette = false);

	virtual ~RMGfxSourceBuffer8RLEByteAA();
};

class RMGfxSourceBuffer8RLEWordAA : public RMGfxSourceBuffer8RLEWord, public RMGfxSourceBuffer8AA {
protected:
	void PrepareImage(void);

public:
	virtual void Draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim);

	// Overloaded initialisation methods
	virtual void Init(RMDataStream &ds, int dimx, int dimy, bool bLoadPalette = false);
	virtual int Init(byte *buf, int dimx, int dimy, bool bLoadPalette = false);

	virtual ~RMGfxSourceBuffer8RLEWordAA();
};


/**
 * Source buffer with 16 colours
 */
class RMGfxSourceBuffer4 : public RMGfxSourceBufferPal {
public:
	RMGfxSourceBuffer4();
	RMGfxSourceBuffer4(int dimx, int dimy, bool bUseDDraw = false);

	// Initialisation
	void Create(int dimx, int dimy, bool bUseDDraw = false);

	int Bpp();
	virtual void Draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim);
};


/**
 * Destination buffer which manages its own internal list of tasks
 */
class RMGfxTargetBuffer : public virtual RMGfxBuffer {
private:
	static RMGfxClearTask taskClear;

	struct OTList {
		RMGfxPrimitive *prim;
		OTList *next;

		OTList();
		OTList(RMGfxPrimitive *pr) {
			prim = pr;
		}
	};

private:
//	OSystem::MutexRef csModifyingOT;

protected:
	OTList *otlist;
	int m_otSize;

public:
	RMGfxTargetBuffer();
	virtual ~RMGfxTargetBuffer();

	// management of the OT list
	void ClearOT(void);
	void DrawOT(CORO_PARAM);
	void AddPrim(RMGfxPrimitive *prim); // The pointer must be delted

	// Adds a task to clear the screen
	void AddClearTask(void);

	operator byte *() {
		return m_buf;
	}
	operator void *() {
		return (void *)m_buf;
	}
	operator uint16 *() {
		// FIXME: This may not be endian safe
		return (uint16 *)m_buf;
	}

	// Offseting buffer
	void OffsetY(int nLines) {
		RMGfxBuffer::OffsetY(nLines, 16);
	}
};


/**
 * Ring buffer, which is both source and by destination
 */
class RMGfxWoodyBuffer: public RMGfxSourceBuffer16, public RMGfxTargetBuffer {
public:
	RMGfxWoodyBuffer();
	RMGfxWoodyBuffer(int dimx, int dimy, bool bUseDDraw = false);
	virtual ~RMGfxWoodyBuffer();

	virtual void Draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim);
};

} // End of namespace Tony

#endif
