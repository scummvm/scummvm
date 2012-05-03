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
/**************************************************************************
 *                                     様様様様様様様様様様様様様様様様様 *
 *                                             Nayma Software srl         *
 *                    e                -= We create much MORE than ALL =- *
 *        u-        z$$$c        '.    様様様様様様様様様様様様様様様様様 *
 *      .d"        d$$$$$b        "b.                                     *
 *   .z$*         d$$$$$$$L        ^*$c.                                  *
 *  #$$$.         $$$$$$$$$         .$$$" Project: Roasted Moths........  *
 *    ^*$b       4$$$$$$$$$F      .d$*"                                   *
 *      ^$$.     4$$$$$$$$$F     .$P"     Module:  GfxCore.CPP..........  *
 *        *$.    '$$$$$$$$$     4$P 4                                     *
 *     J   *$     "$$$$$$$"     $P   r    Author:  Giovanni Bajo........  *
 *    z$   '$$$P*4c.*$$$*.z@*R$$$    $.                                   *
 *   z$"    ""       #$F^      ""    '$c                                  *
 *  z$$beu     .ue="  $  "=e..    .zed$$c                                 *
 *      "#$e z$*"   .  `.   ^*Nc e$""                                     *
 *         "$$".  .r"   ^4.  .^$$"                                        *
 *          ^.@*"6L=\ebu^+C$"*b."                                         *
 *        "**$.  "c 4$$$  J"  J$P*"    OS:  [ ] DOS  [X] WIN95  [ ] PORT  *
 *            ^"--.^ 9$"  .--""      COMP:  [ ] WATCOM  [X] VISUAL C++    *
 *                    "                     [ ] EIFFEL  [ ] GCC/GXX/DJGPP *
 *                                                                        *
 * This source code is Copyright (C) Nayma Software.  ALL RIGHTS RESERVED *
 *                                                                        *
 **************************************************************************/

#include "tony/gfxEngine.h"
#include "tony/mpal/mpalutils.h"

namespace Tony {

static char rcsid[] = "$Id: $";


/****************************************************************************\
*       Metodi di RMGfxTask
\****************************************************************************/

RMGfxTask::RMGfxTask() {
	m_nPrior = 0;
	m_nInList = 0;
}

int RMGfxTask::Priority() {
	return m_nPrior;
}

bool RMGfxTask::RemoveThis() {
 return true;
}


/****************************************************************************\
*       Metodi di RMGfxTaskSetPrior
\****************************************************************************/

void RMGfxTaskSetPrior::SetPriority(int nPrior) {
	m_nPrior = nPrior;
}


/****************************************************************************\
*       Metodi di RMGfxBuffer
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
	// Distruggi il buffer se esiste di gia'
	if (m_buf != NULL)
		Destroy();

	// Copia i parametri nei membri privati
	m_dimx = dimx;
	m_dimy = dimy;
	m_bUseDDraw = bUseDDraw;

	if (!m_bUseDDraw) {
		// Alloca il buffer
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
	 // Gestisce l'accelerazione
	}
}

void RMGfxBuffer::Unlock(void) {
	if (m_bUseDDraw) {
		// Gestisce l'accelerazione
	}
}

void RMGfxBuffer::OffsetY(int nLines, int nBpp) {
	m_buf += nLines* Dimx() * nBpp / 8;
}


inline RMGfxBuffer::operator byte *() {
	return m_buf;
}

inline RMGfxBuffer::operator void *() {
	return (void *)m_buf;
}

inline RMGfxBuffer::RMGfxBuffer(int dimx, int dimy, int nBpp, bool bUseDDraw) {
	Create(dimx, dimy, nBpp, bUseDDraw);
}

/****************************************************************************\
*       Metodi di RMGfxSourceBuffer
\****************************************************************************/

/****************************************************************************\
*
* Function:
*
* Description:
*
* Input:
*
* Return:
*
\****************************************************************************/

int RMGfxSourceBuffer::Init(const byte *buf, int dimx, int dimy, bool bLoadPalette) {
	Create(dimx, dimy, Bpp());
	CopyMemory(m_buf, buf, dimx * dimy * Bpp() / 8);
	
	// Richiama la funzione di preparazione della surface (ereditata)
	PrepareImage();

	return dimx * dimy * Bpp() / 8;
}


void RMGfxSourceBuffer::Init(RMDataStream& ds, int dimx, int dimy, bool bLoadPalette) {
	Create(dimx, dimy,Bpp());
	ds.Read(m_buf, dimx * dimy*Bpp() / 8);
	
	// Richiama la funzione di preparazione della surface (ereditata)
	PrepareImage();
}

RMGfxSourceBuffer::~RMGfxSourceBuffer() {
}

void RMGfxSourceBuffer::PrepareImage(void) {
	// Non fa nulla: puo' essere overloadata se necessaria
}

bool RMGfxSourceBuffer::Clip2D(int &x1, int &y1, int &u, int &v, int &width, int &height, bool bUseSrc, RMGfxTargetBuffer* buf) {
	int destw, desth;

/*
   OBSOLETE CODE: 

	if (buf == NULL)
	{
		destw=RM_SX;
		desth=RM_SY;
	}
	else
	{
		destw = buf->Dimx();
		desth = buf->Dimy();
	}
*/
	destw = buf->Dimx();
	desth = buf->Dimy();

	if (!bUseSrc) {
		u = v = 0;
		width = m_dimx;
		height = m_dimy;
	}
	
	if (x1 > destw - 1)	return false;
	if (y1 > desth - 1) return false;

	if (x1 < 0)	 {
		width += x1;
		if (width < 0) return false;
		u -= x1;
		x1 = 0;	
	}

	if (y1 < 0) {	
		height += y1;
		if (height < 0) return false;
		v -= y1;
		y1 = 0;
	}

	if (x1 + width - 1 > destw - 1)
		width = destw - x1;

	if (y1 + height - 1 > desth - 1)
		height = desth - y1;

	return true;
}

/****************************************************************************\
*
* Function:     void RMGfxSourceBuffer::Init(uint32 resID, int dimx, int dimy);
*
* Description:  Carica una surface partendo dall'ID della risorsa
*
* Input:        uint32 resID             ID della risorsa
*               int dimx, dimy					Dimensione del buffer
*
\****************************************************************************/

inline int RMGfxSourceBuffer::Init(uint32 resID, int dimx, int dimy, bool bLoadPalette) {
	return Init(RMRes(resID), dimx, dimy, bLoadPalette);
}

/****************************************************************************\
*       Metodi di RMGfxWoodyBuffer
\****************************************************************************/

RMGfxWoodyBuffer::~RMGfxWoodyBuffer() {

}

void RMGfxWoodyBuffer::Draw(RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	// Prima si fa disegnare tramite la propria OT list
	DrawOT();

	// Poi disegna se stesso nel target buffer
	RMGfxSourceBuffer16::Draw(bigBuf, prim);
}

inline RMGfxWoodyBuffer::RMGfxWoodyBuffer() {

}

inline RMGfxWoodyBuffer::RMGfxWoodyBuffer(int dimx, int dimy, bool bUseDDraw)
	  : RMGfxBuffer(dimx,dimy,16,bUseDDraw) {

}


/****************************************************************************\
*       Metodi di RMGfxTargetBuffer
\****************************************************************************/

RMGfxClearTask RMGfxTargetBuffer::taskClear;

RMGfxTargetBuffer::RMGfxTargetBuffer() {
	otlist = NULL;
//	InitializeCriticalSection(&csModifyingOT);
}

RMGfxTargetBuffer::~RMGfxTargetBuffer() {
	ClearOT();
//	DeleteCriticalSection(&csModifyingOT);
}


void RMGfxTargetBuffer::ClearOT(void) {
	OTList *cur, *n;

	g_system->lockMutex(csModifyingOT);

	cur = otlist;

	while (cur != NULL) {
		cur->prim->m_task->Unregister();
		delete cur->prim;
		n = cur->next;
		delete cur;
		cur = n;
	}

	otlist = NULL;

	g_system->unlockMutex(csModifyingOT);
}

void RMGfxTargetBuffer::DrawOT(void) {
	OTList *cur;
	OTList *prev;
	OTList *next;
	RMGfxPrimitive *myprim;

	prev = NULL;
	cur = otlist;

	// Lock del buffer per accederci
	Lock();
	g_system->lockMutex(csModifyingOT);

 	while (cur != NULL) {
		// Richiama la draw sul task, passandogli una copia della primitiva
		myprim=cur->prim->Duplicate();
		cur->prim->m_task->Draw(*this, myprim);
		delete myprim;

		// Controlla se e' arrivato il momento di rimuovere il task dalla OTlist
		if (cur->prim->m_task->RemoveThis()) {
			// Deregistra il task
			cur->prim->m_task->Unregister();

 			// Cancella il task liberando la memoria
			delete cur->prim;
			next=cur->next;
			delete cur;

			// Se era il primo elemento, aggiorna la testa della lista
			if (prev == NULL)
				otlist = next;
			// Altrimenti aggiorna il puntatore al successivo dell'elemento precedente
			else
				prev->next = next;

			cur = next;
		} else {
			// Aggiorna il puntatore al precedente e scorre la lista
			prev = cur;
			cur = cur->next;
		}
	}

	g_system->unlockMutex(csModifyingOT);

	// Unlock dopo la scrittura
	Unlock();
}

void RMGfxTargetBuffer::AddPrim(RMGfxPrimitive *prim) {
	int nPrior;
	OTList *cur, *n;

	g_system->lockMutex(csModifyingOT);

	// Avverte che e' in lista di OT
	prim->m_task->Register();

	// Controlla la sua priorita'
	nPrior = prim->m_task->Priority();
	n = new OTList(prim);

	// Lista vuota
	if (otlist == NULL) {
		otlist = n;
		otlist->next = NULL;
	}
	// Inserimento in testa
	else if (nPrior < otlist->prim->m_task->Priority())
	{
		n->next = otlist;
		otlist = n;
	} else {
		cur = otlist;
		while (cur->next != NULL && nPrior > cur->next->prim->m_task->Priority())
			cur=cur->next;

		n->next = cur->next;
		cur->next = n;
	}

	g_system->unlockMutex(csModifyingOT);
}

inline void RMGfxTargetBuffer::AddClearTask(void) {
	AddPrim(new RMGfxPrimitive(&taskClear));
}


/****************************************************************************\
*				Metodi di RMGfxSourceBufferPal
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
		CopyMemory(m_pal,buf,(1<<Bpp())*3);
	
	PreparePalette();

	return (1<<Bpp())*3;
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

	for (i = 0; i < 256; i++)
	{
		m_palFinal[i] = (((int)m_pal[i * 3 + 0] >> 3) <<  10) |
									(((int)m_pal[i * 3 + 1] >> 3) <<  5) | 
									(((int)m_pal[i * 3 + 2] >> 3) <<  0);
	}
}

int RMGfxSourceBufferPal::Init(const byte *buf, int dimx, int dimy, bool bLoadPalette) {
	int read;
	
	// Carica l'immagine RAW
	read = RMGfxSourceBuffer::Init(buf, dimx, dimy);
	
	// Carica la palette se necessario
	if (bLoadPalette)
		read += LoadPaletteWA(&buf[read]);

	return read;
}

void RMGfxSourceBufferPal::Init(RMDataStream &ds, int dimx, int dimy, bool bLoadPalette) {
	// Carica l'immagine RAW
	RMGfxSourceBuffer::Init(ds, dimx, dimy);
	
	// Carica la palette se necessario
	if (bLoadPalette) {
		byte *suxpal = new byte[256 * 3];
		ds.Read(suxpal, 256 * 3);
		LoadPaletteWA(suxpal);
		delete[] suxpal;
	}
}

inline int RMGfxSourceBufferPal::LoadPalette(uint32 resID) {
	return LoadPalette(RMRes(resID));
}

inline int RMGfxSourceBufferPal::LoadPaletteWA(uint32 resID, bool bSwapped) {
	return LoadPaletteWA(RMRes(resID), bSwapped);
}

/****************************************************************************\
*				Metodi di RMGfxSourceBuffer4
\****************************************************************************/

void RMGfxSourceBuffer4::Draw(RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
}

inline RMGfxSourceBuffer4::RMGfxSourceBuffer4(int dimx, int dimy, bool bUseDDraw)
		: RMGfxBuffer(dimx,dimy,4,bUseDDraw) {
	SetPriority(0);
}

/****************************************************************************\
*
* Function:     int RMGfxSourceBuffer4::Bpp();
*
* Description:  Ritorna il numero di bit per pixel della surface
*
* Return:       Bit per pixel
*
\****************************************************************************/

inline int RMGfxSourceBuffer4::Bpp() {
	return 4;
}

inline void RMGfxSourceBuffer4::Create(int dimx, int dimy, bool bUseDDraw) {
	RMGfxBuffer::Create(dimx,dimy,4,bUseDDraw);
}

/****************************************************************************\
*				Metodi di RMGfxSourceBuffer8
\****************************************************************************/

RMGfxSourceBuffer8::~RMGfxSourceBuffer8() {

}

void RMGfxSourceBuffer8::Draw(RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	int x, y, width, height, u, v;
	int bufx = bigBuf.Dimx();
	uint16 *buf = bigBuf;
	byte *raw = m_buf;

	// Destination nel buffer
	RMRect dst;
	if (prim->HaveDst())
		dst=prim->Dst();
	
	// Esegue il clip
	if (prim->HaveSrc()) {
		u = prim->Src().x1;
		v = prim->Src().y1;
		
		width = prim->Src().Width();
		height = prim->Src().Height();
	}

	if (!Clip2D(dst.x1, dst.y1, u, v, width, height, prim->HaveSrc(), &bigBuf))
		return;

	// Offset iniziale nel buffer
	buf += dst.y1 * bufx + dst.x1;

	// Passaggio normale
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

			for (x = 0;x<width;x+=2)
			{
				buf[0] = m_palFinal[raw[0]];
				buf[1] = m_palFinal[raw[1]];
				
				buf+=2;
				raw+=2;
			}

			buf += bufx-width;
		}
	}
}

inline RMGfxSourceBuffer8::RMGfxSourceBuffer8(int dimx, int dimy, bool bUseDDraw)
		: RMGfxBuffer(dimx,dimy,8,bUseDDraw) {
	SetPriority(0);
}

inline RMGfxSourceBuffer8::RMGfxSourceBuffer8(bool bTrasp0) {
	m_bTrasp0=bTrasp0;
}


/****************************************************************************\
*
* Function:     int RMGfxSourceBuffer8::Bpp();
*
* Description:  Ritorna il numero di bit per pixel della surface
*
* Return:       Bit per pixel
*
\****************************************************************************/

inline int RMGfxSourceBuffer8::Bpp() {
	return 8;
}

inline void RMGfxSourceBuffer8::Create(int dimx, int dimy, bool bUseDDraw) {	
	RMGfxBuffer::Create(dimx, dimy, 8, bUseDDraw);
}

#define GETRED(x)	(((x) >> 10) & 0x1F)
#define GETGREEN(x) (((x) >> 5) & 0x1F)
#define GETBLUE(x) ((x) & 0x1F)


/****************************************************************************\
*				Metodi di RMGfxSourceBuffer8AB
\****************************************************************************/

RMGfxSourceBuffer8AB::~RMGfxSourceBuffer8AB() {

}

inline int RMGfxSourceBuffer8AB::CalcTrasp(int fore, int back)
{
	int r,g,b;

	r = (GETRED(fore) >> 2) + (GETRED(back) >> 1);
	g = (GETGREEN(fore) >> 2) + (GETGREEN(back) >> 1);
	b = (GETBLUE(fore) >> 2) + (GETBLUE(back) >> 1);

	if (r > 0x1F) r = 0x1F;
	if (g > 0x1F) g = 0x1F;
	if (b > 0x1F) b = 0x1F;

	return (r<<10)|(g<<5)|b;
}


void RMGfxSourceBuffer8AB::Draw(RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	int x, y, width, height, u, v;
	int bufx=bigBuf.Dimx();
	uint16 *buf = bigBuf;
	byte *raw = m_buf;

	// Destination nel buffer
	RMRect dst;
	if (prim->HaveDst())
		dst = prim->Dst();
	
	// Esegue il clip
	if (prim->HaveSrc()) {
		u=prim->Src().x1;
		v=prim->Src().y1;
		
		width = prim->Src().Width();
		height = prim->Src().Height();
	}

	if (!Clip2D(dst.x1, dst.y1, u, v, width, height, prim->HaveSrc(), &bigBuf))
		return;

	// Offset iniziale nel buffer
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
			raw = m_buf + (y+v) * m_dimx + u;

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
*				Metodi di RMGfxSourceBuffer8RLE
\****************************************************************************/

byte RMGfxSourceBuffer8RLE::MegaRLEBuf[512 * 1024];

void RMGfxSourceBuffer8RLE::SetAlphaBlendColor(int color) {
	alphaBlendColor = color;
}

RMGfxSourceBuffer8RLE::RMGfxSourceBuffer8RLE() {
	alphaBlendColor = -1;
	bNeedRLECompress = true;
}

RMGfxSourceBuffer8RLE::~RMGfxSourceBuffer8RLE() {
	if (m_buf != NULL) {
		delete m_buf;
		m_buf = NULL;
	}
}


int RMGfxSourceBuffer8RLE::Init(const byte *buf, int dimx, int dimy, bool bLoadPalette) {
	return RMGfxSourceBufferPal::Init(buf, dimx, dimy, bLoadPalette);
}

void RMGfxSourceBuffer8RLE::Init(RMDataStream &ds, int dimx, int dimy, bool bLoadPalette) {
	if (bNeedRLECompress) {
		RMGfxSourceBufferPal::Init(ds, dimx, dimy,bLoadPalette);
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
	// Richiama il metodo padre
	RMGfxSourceBuffer8::PreparePalette();

	// Si salva gli RGB di alpha blending
	if (alphaBlendColor != -1) {
		alphaR = (m_palFinal[alphaBlendColor] >> 10) & 0x1F;
		alphaG = (m_palFinal[alphaBlendColor] >> 5) & 0x1F;
  		alphaB = (m_palFinal[alphaBlendColor]) & 0x1F;
	}
}

void RMGfxSourceBuffer8RLE::PrepareImage(void) {
	// Richiama il metodo padre
	RMGfxSourceBuffer::PrepareImage();

	// Comprime
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
	
	// Esegue la compressione RLE, per linee
	cur = MegaRLEBuf;
	src = m_buf;
	for (y = 0;y < m_dimy; y++) {
		// Si salva l'inizio della linea
		startline = cur;
		
		// Lascia lo spazio per la lunghezza della linea
		cur += 2;

		// Si parte dallo spazio vuoto
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
				}
				else if (curdata == 1) {
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
		
		// Dati in sospeso?
		if (curdata == 1) {
			RLEWriteAlphaBlend(cur, rep);
			RLEWriteData(cur, 0, NULL);
		}
		
		if (curdata == 2) {
			RLEWriteData(cur, rep, startsrc);
		}

		// Fine linea
		RLEWriteEOL(cur);

		// Scrive la lunghezza della linea
		WRITE_LE_UINT16(startline, (uint16)(cur - startline));
	}
	
	// Cancella l'immagine originale
	delete[] m_buf; 

	// Ci copia l'immagine compressa
	x = cur - MegaRLEBuf;
	m_buf = new byte[x];
	CopyMemory(m_buf, MegaRLEBuf, x);
}

void RMGfxSourceBuffer8RLE::Draw(RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	int y;
	byte *src;
	uint16 *buf = bigBuf;
	int x1, y1, u, v, width, height;
	
	// Clippiamo lo sprite
	x1=prim->Dst().x1;
	y1=prim->Dst().y1;
	if (!Clip2D(x1,y1,u,v,width,height,false,&bigBuf))
		return;

	// Andiamo avanti di V linee nell'immagine RLE
	src = m_buf;
	for (y = 0;y<v;y++)
		src += READ_LE_UINT16(src);

	// Posizionamoci nel buffer di destinazione
	buf += y1*bigBuf.Dimx();

	// Looppone
	if (prim->IsFlipped())
	{
// Annulliamo il clipping orizzontale
//		width = m_dimx;
//		x1=prim->Dst().x1;

	  // Flippiamo il clipping
		u = m_dimx - (width+u);
		x1 = (prim->Dst().x1 + m_dimx - 1) - u;

		for (y = 0;y<height;y++)
		{
			// Decompressione
			RLEDecompressLineFlipped(buf+x1, src+2,u,width);

			// Prossima linea
			src += READ_LE_UINT16(src);

			// Skippa alla linea successiva
			buf+=bigBuf.Dimx();		
		}
	}
	else
	{
		for (y = 0;y<height;y++)
		{
			// Decompressione
			RLEDecompressLine(buf+x1, src+2,u,width);

			// Prossima linea
			src += READ_LE_UINT16(src);

			// Skippa alla linea successiva
			buf+=bigBuf.Dimx();		
		}
	}
}


/****************************************************************************\
*				Metodi di RMGfxSourceBuffer8RLEByte
\****************************************************************************/

RMGfxSourceBuffer8RLEByte::~RMGfxSourceBuffer8RLEByte() {

}

void RMGfxSourceBuffer8RLEByte::RLEWriteTrasp(byte *&cur, int rep) {
	assert(rep < 255);
	*cur ++= rep;
}

void RMGfxSourceBuffer8RLEByte::RLEWriteAlphaBlend(byte *&cur, int rep) {
	assert(rep < 255);
	*cur ++= rep;
}

void RMGfxSourceBuffer8RLEByte::RLEWriteData(byte *&cur, int rep, byte *src) {
	assert(rep < 256);
	
	*cur ++= rep;
	if (rep > 0) {
		CopyMemory(cur, src, rep);
		cur += rep;
		src += rep;
	}

	return;
}

void RMGfxSourceBuffer8RLEByte::RLEWriteEOL(byte *&cur) {
	*cur ++= 0xFF;
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
		
		if (n >= nStartSkip) 		{
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
		// Via il trasp di merda
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
		for (i = 0; i < n; i++)
		{
			r=(*dst >> 10) & 0x1F;
			g=(*dst >> 5) & 0x1F;
			b=*dst & 0x1F;

			r=(r >> 2) + (alphaR >> 1);
			g=(g >> 2) + (alphaG >> 1);
			b=(b >> 2) + (alphaB >> 1);

			*dst++=(r<<10)|(g<<5)|b;
		}

		nLength -= n;
		if (!nLength)
			return;
		assert(nLength > 0);

//RLEByteDoCopy:
		// Copia la roba
		n = *src++;

RLEByteDoCopy2:
		if (n > nLength)
			n = nLength;
		
		for (i = 0; i < n; i++)	
			*dst ++= m_palFinal[*src++];

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
		if (n >= nStartSkip){
			src += nStartSkip;
			n -= nStartSkip;
			goto RLEByteFlippedDoCopy2;
		}
		nStartSkip -= n;
		src += n;
	}


	while (1) {
RLEByteFlippedDoTrasp:
		// Via il trasp di merda
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
			r=(*dst >> 10) & 0x1F;
			g=(*dst >> 5) & 0x1F;
			b=*dst & 0x1F;

			r=(r >> 2) + (alphaR >> 1);
			g=(g >> 2) + (alphaG >> 1);
			b=(b >> 2) + (alphaB >> 1);

			*dst--=(r << 10) | (g << 5) | b;
		}

		nLength -= n;
		if (!nLength)
			return;
		assert(nLength > 0);

//RLEByteFlippedDoCopy:
		// Copia la roba
		n = *src++;

RLEByteFlippedDoCopy2:
		if (n > nLength)
			n = nLength;
		
		for (i = 0; i < n; i++)	
			*dst --= m_palFinal[*src++];

		nLength -= n;
		if (!nLength)
			return;
		assert(nLength > 0);
	}
}


/****************************************************************************\
*				Metodi di RMGfxSourceBuffer8RLEWord
\****************************************************************************/

RMGfxSourceBuffer8RLEWord::~RMGfxSourceBuffer8RLEWord(){

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
		CopyMemory(cur, src,rep);
		cur += rep;
		src +=rep;
	}
}

void RMGfxSourceBuffer8RLEWord::RLEWriteEOL(byte *&cur) {
	*cur ++= 0xFF;
	*cur ++= 0xFF;
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
		
		if (n>=nStartSkip)
		{
			src += nStartSkip;
			n -= nStartSkip;
			goto RLEWordDoCopy2;
		}
		nStartSkip -= n;
		src += n;
	}


	while (1)
	{
RLEWordDoTrasp:
		// Via il trasp di merda
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
			r=(*dst >> 10) & 0x1F;
			g=(*dst >> 5) & 0x1F;
			b=*dst & 0x1F;

			r=(r >> 2) + (alphaR >> 1);
			g=(g >> 2) + (alphaG >> 1);
			b=(b >> 2) + (alphaB >> 1);

			*dst++=(r<<10)|(g<<5)|b;
		}

		nLength -= n;
		if (!nLength)
			return;

		assert(nLength > 0);

//RLEWordDoCopy:
		// Copia la roba
		n = READ_LE_UINT16(src);
		src += 2;
		
RLEWordDoCopy2:
		if (n > nLength)
			n = nLength;

		for (i = 0; i < n; i++)	
			*dst ++= m_palFinal[*src++];

		nLength -= n;
		if (!nLength)
			return;
		
		assert(nLength > 0);

	}
}

void RMGfxSourceBuffer8RLEWord::RLEDecompressLineFlipped(uint16 *dst, byte *src, int nStartSkip, int nLength) {
	int i,n;
	int r,g,b;

  if (nStartSkip == 0)
	  goto RLEWordFlippedDoTrasp;

	while (1) {
		assert(nStartSkip > 0);

		// TRASP
		n = READ_LE_UINT16(src);
		src += 2;

		if (n == 0xFFFF)
			return;
		
		if (n>=nStartSkip)
		{
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

		if (n>=nStartSkip)
		{
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
		// Via il trasp di merda
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
			r=(*dst >> 10) & 0x1F;
			g=(*dst >> 5) & 0x1F;
			b=*dst & 0x1F;

			r=(r >> 2) + (alphaR >> 1);
			g=(g >> 2) + (alphaG >> 1);
			b=(b >> 2) + (alphaB >> 1);

			*dst--=(r << 10) | (g << 5) | b;
		}

		nLength -= n;
		if (!nLength)
			return;

		assert(nLength > 0);

//RLEWordFlippedDoCopy:
		// Copia la roba
		n = READ_LE_UINT16(src);
		src += 2;
		
RLEWordFlippedDoCopy2:
		if (n > nLength)
			n = nLength;

		for (i = 0;i<n;i++)	
			*dst --= m_palFinal[*src++];

		nLength -= n;
		if (!nLength)
			return;
		
		assert(nLength > 0);
	}
}

/****************************************************************************\
*				Metodi di RMGfxSourceBuffer8RLEWord
\****************************************************************************/

RMGfxSourceBuffer8RLEWordAB::~RMGfxSourceBuffer8RLEWordAB() {

}

void RMGfxSourceBuffer8RLEWordAB::RLEDecompressLine(uint16 *dst, byte *src,  int nStartSkip, int nLength) {
	int i, n;
	int r, g, b, r2, g2, b2;

	if (!bCfgTransparence) 	{
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
		
		if (n >= nStartSkip)
		{
			src += nStartSkip;
			n -= nStartSkip;
			goto RLEWordDoCopy2;
		}
		nStartSkip -= n;
		src += n;
	}


	while (1) {
RLEWordDoTrasp:
		// Via il trasp di merda
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

		// @@@ NON DOVREBBE ESSERCI!!!!! 
		for (i = 0; i < n; i++) {
			r=(*dst >> 10) & 0x1F;
			g=(*dst >> 5) & 0x1F;
			b=*dst & 0x1F;

			r=(r >> 2) + (alphaR >> 1);
			g=(g >> 2) + (alphaG >> 1);
			b=(b >> 2) + (alphaB >> 1);

			*dst++=(r<<10)|(g<<5)|b;
		}

		nLength -= n;
		if (!nLength)
			return;

		assert(nLength > 0);

//RLEWordDoCopy:
		// Copia la roba
		n = READ_LE_UINT16(src);
		src += 2;
		
RLEWordDoCopy2:
		if (n > nLength)
			n = nLength;

		for (i = 0; i < n; i++)	{
			r=(*dst >> 10) & 0x1F;
			g=(*dst >> 5) & 0x1F;
			b=*dst & 0x1F;

			r2=(m_palFinal[*src] >> 10) & 0x1F;
			g2=(m_palFinal[*src] >> 5) & 0x1F;
			b2 = m_palFinal[*src] & 0x1F;

			r=(r >> 1) + (r2 >> 1);
			g=(g >> 1) + (g2 >> 1);
			b=(b >> 1) + (b2 >> 1);

			*dst ++= (r << 10) | (g << 5) | b;
			src++;
		}

		nLength -= n;
		if (!nLength)
			return;
		
		assert(nLength > 0);

	}
}


/****************************************************************************\
*				Metodi di RMGfxSourceBuffer8AA
\****************************************************************************/

byte RMGfxSourceBuffer8AA::MegaAABuf[256*1024];
byte RMGfxSourceBuffer8AA::MegaAABuf2[64*1024];

void RMGfxSourceBuffer8AA::PrepareImage(void) {
	// Richiama il metodo padre
	RMGfxSourceBuffer::PrepareImage();
		
	// Prepara il buffer di antialiasing
	CalculateAA();
}


void RMGfxSourceBuffer8AA::CalculateAA(void) {
	// Io suxo tu suxi egli suxa noi suxiamo voi suxate essi suxano  ---> ANTI ALIASING SUX!
	// ************************************************************

	int x,y;
	byte *src, *srcaa;

	/* Prima passata: cerchiamo i bordi */
	Common::fill(MegaAABuf, MegaAABuf + m_dimx * m_dimy, 0);
	
	src = m_buf;
	srcaa=MegaAABuf;
	for (y = 0;y < m_dimy; y++) {
		for (x = 0; x < m_dimx; x++) {
			if (*src == 0) {
				if ((y > 0 && src[-m_dimx] != 0) ||
						(y<m_dimy-1 && src[m_dimx] != 0) ||
						(x > 0 && src[-1] != 0) ||
						(x<m_dimx-1 && src[1] != 0))
					*srcaa = 1;
			}

			src++;
			srcaa++;
		}			
	}

	src = m_buf;
	srcaa=MegaAABuf;
	for (y = 0; y < m_dimy; y++) {
		for (x = 0; x < m_dimx; x++) {
			if (*src != 0) {
				if ((y > 0 && srcaa[-m_dimx]==1) ||
						(y<m_dimy-1 && srcaa[m_dimx]==1) ||
						(x > 0 && srcaa[-1]==1) ||
						(x<m_dimx-1 && srcaa[1]==1))
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
	int x,y;
	byte *src;
	uint16 *mybuf;
	uint16 *buf;
	int x1, y1, u, v, width, height;
	int r, g, b;
	int step;
	
	// Clippiamo lo sprite
	x1 = prim->Dst().x1;
	y1 = prim->Dst().y1;
	if (!Clip2D(x1, y1, u, v, width, height, false, &bigBuf))
		return;

	// Andiamo avanti di V linee nell'immagine RLE
	src = m_buf;
	for (y = 0; y < v; y++)
		src += READ_LE_UINT16(src);

	// Annulliamo il clipping orizzontale

    // Flippiamo il clipping
	if (prim->IsFlipped()) {
		u = m_dimx - (width + u);
		x1 = (prim->Dst().x1 + m_dimx - 1) - u;
	}
//	width = m_dimx;
//	x1=prim->Dst().x1;

	
	// Posizionamoci nel buffer di destinazione
	buf = bigBuf;
	buf += y1 * bigBuf.Dimx();

	if (prim->IsFlipped())
		step = -1;
	else
		step = 1;

	// Looppone
	buf += bigBuf.Dimx(); // skippa la prima linea
	for (y = 1; y < height - 1; y++) {
/*
		if (prim->IsFlipped())
			mybuf=&buf[x1+m_dimx-1];
		else
*/
			mybuf=&buf[x1];

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

				mybuf[0]=(r<<10)|(g<<5)|b;
			}
		
		// Skippa alla linea successiva
		buf += bigBuf.Dimx();		
	}

// Posizionamoci nel buffer di destinazione
	buf = bigBuf;
	buf += y1*bigBuf.Dimx();

	// Looppone
	buf+=bigBuf.Dimx();		
	for (y = 1; y < height - 1; y++) {
/*
		if (prim->IsFlipped())
			mybuf=&buf[x1+m_dimx-1];
		else
*/
			mybuf=&buf[x1];

		for (x = 0;x < width; x++, mybuf += step)
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

				mybuf[0]=(r<<10)|(g<<5)|b;
			}
		
		// Skippa alla linea successiva
		buf += bigBuf.Dimx();		
	}
}



void RMGfxSourceBuffer8AA::Draw(RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	RMGfxSourceBuffer8::Draw(bigBuf, prim);
	DrawAA(bigBuf, prim);
}


/****************************************************************************\
*				Metodi di RMGfxSourceBuffer8RLEAA
\****************************************************************************/

RMGfxSourceBuffer8RLEByteAA::~RMGfxSourceBuffer8RLEByteAA() {

}

void RMGfxSourceBuffer8RLEByteAA::PrepareImage(void) {
	RMGfxSourceBuffer::PrepareImage();
	CalculateAA();
	CompressRLE();
}

void RMGfxSourceBuffer8RLEByteAA::Draw(RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	RMGfxSourceBuffer8RLE::Draw(bigBuf,prim);
	if (bCfgAntiAlias)
		DrawAA(bigBuf,prim);
}

int RMGfxSourceBuffer8RLEByteAA::Init(const byte *buf, int dimx, int dimy, bool bLoadPalette) {
	return RMGfxSourceBuffer8RLE::Init(buf, dimx, dimy, bLoadPalette);
}

void RMGfxSourceBuffer8RLEByteAA::Init(RMDataStream& ds, int dimx, int dimy, bool bLoadPalette) {
	RMGfxSourceBuffer8RLE::Init(ds, dimx, dimy,bLoadPalette);

	if (!bNeedRLECompress) {
		// Carica la maschera di antialiasing
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

void RMGfxSourceBuffer8RLEWordAA::Draw(RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	RMGfxSourceBuffer8RLE::Draw(bigBuf,prim);
	if (bCfgAntiAlias)
		DrawAA(bigBuf,prim);
}

int RMGfxSourceBuffer8RLEWordAA::Init(byte *buf, int dimx, int dimy, bool bLoadPalette) {
	return RMGfxSourceBuffer8RLE::Init(buf, dimx, dimy,bLoadPalette);
}

void RMGfxSourceBuffer8RLEWordAA::Init(RMDataStream& ds, int dimx, int dimy, bool bLoadPalette) {
	RMGfxSourceBuffer8RLE::Init(ds, dimx, dimy,bLoadPalette);

	if (!bNeedRLECompress) {
		// Carica la maschera di antialiasing
		m_aabuf = new byte[dimx * dimy];
		ds.Read(m_aabuf, dimx * dimy);
	}
}


/****************************************************************************\
*				Metodi di RMGfxSourceBuffer16
\****************************************************************************/

RMGfxSourceBuffer16::RMGfxSourceBuffer16(bool bTrasp0) {
	m_bTrasp0=bTrasp0;
}

RMGfxSourceBuffer16::~RMGfxSourceBuffer16() {
}

void RMGfxSourceBuffer16::Draw(RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	int x,y;
	uint16 *buf = bigBuf;
	uint16 *raw = (uint16*)m_buf;
	int dimx, dimy;
	int u,v;
	int x1,y1;

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

	raw += v * m_dimx+u;
	buf += y1 * bigBuf.Dimx() + x1;

	if (m_bTrasp0) {
		for (y = 0; y < dimy; y++) {
			for (x = 0; x < dimx;) {
				while (x<dimx && raw[x] == 0)
					x++;

				while (x<dimx && raw[x] != 0) {
					buf[x] = raw[x];
					x++;
				}
			}

			raw += m_dimx;
			buf += bigBuf.Dimx();
		}
	} else {
		for (y = 0; y < dimy; y++) {
			CopyMemory(buf, raw, dimx * 2);
			buf += bigBuf.Dimx();
			raw += m_dimx;
		}
	}
}

void RMGfxSourceBuffer16::PrepareImage(void) {
	// Color space conversion se necessario!
	int i;
	uint16 *buf = (uint16 *)m_buf;

	for (i = 0; i < m_dimx * m_dimy; i++)
		WRITE_LE_UINT16(&buf[i], FROM_LE_16(buf[i]) & 0x7FFF);
}


inline RMGfxSourceBuffer16::RMGfxSourceBuffer16(int dimx, int dimy, bool bUseDDraw)
		: RMGfxBuffer(dimx,dimy,16,bUseDDraw) {
	SetPriority(0);
}

/****************************************************************************\
*
* Function:     int RMGfxSourceBuffer16::Bpp();
*
* Description:  Ritorna il numero di bit per pixel della surface
*
* Return:       Bit per pixel
*
\****************************************************************************/

inline int RMGfxSourceBuffer16::Bpp() {
	return 16;
}

inline void RMGfxSourceBuffer16::Create(int dimx, int dimy, bool bUseDDraw) {
	RMGfxBuffer::Create(dimx,dimy,16,bUseDDraw);
}

/****************************************************************************\
*				Metodi di RMGfxBox
\****************************************************************************/

bool RMGfxBox::RemoveThis(void) {
	return true;
}

void RMGfxBox::SetColor(byte r, byte g, byte b) {
	r >>= 3;
	g >>= 3;
	b >>= 3;
	wFillColor = (r << 10) | (g << 5) | b;
}

void RMGfxBox::Draw(RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	int i, j;
	uint16 *buf = bigBuf;
	RMRect rcDst;
	
	// Prende il rettangolo di destinazione
	rcDst = prim->Dst();
	buf += rcDst.y1*bigBuf.Dimx() + rcDst.x1;

	// Fa il suo bravo loop di put
	for (j = 0; j < rcDst.Height(); j++) {
		for (i = 0; i < rcDst.Width(); i++)
			*buf ++= wFillColor;

		buf += bigBuf.Dimx() - rcDst.Width();
	}
}


/****************************************************************************\
*       Metodi di RMGfxClearTask
\****************************************************************************/

inline int RMGfxClearTask::Priority() {
	// Priorita' massima (deve essere fatto per primo)
	return 1;
}

inline void RMGfxClearTask::Draw(RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *) {
	// Pulisce tutto il target buffer
	Common::fill((byte *)bigBuf, (byte *)bigBuf + (bigBuf.Dimx() * bigBuf.Dimy() * 2), 0x0);
}

inline bool RMGfxClearTask::RemoveThis() {
	// Il task di clear si disattiva sempre
	return true;
}

} // End of namespace Tony
