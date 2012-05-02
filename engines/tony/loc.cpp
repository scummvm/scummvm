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
 *                                     ออออออออออออออออออออออออออออออออออ *
 *                                             Nayma Software srl         *
 *                    e                -= We create much MORE than ALL =- *
 *        u-        z$$$c        '.    ออออออออออออออออออออออออออออออออออ *
 *      .d"        d$$$$$b        "b.                                     *
 *   .z$*         d$$$$$$$L        ^*$c.                                  *
 *  #$$$.         $$$$$$$$$         .$$$" Project: Roasted Moths........  *
 *    ^*$b       4$$$$$$$$$F      .d$*"                                   *
 *      ^$$.     4$$$$$$$$$F     .$P"     Module:  Loc.CPP..............  *
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

#include "common/scummsys.h"
#include "tony/mpal/mpalutils.h"
#include "tony/adv.h"
#include "tony/loc.h"
#include "tony/tony.h"

namespace Tony {

using namespace ::Tony::MPAL;

static char rcsid[] = "$Id: $";

extern bool bSkipSfxNoLoop;


/****************************************************************************\
*       Metodi di RMPalette
\****************************************************************************/

/****************************************************************************\
*
* Function:     friend RMDataStream &operator>>(RMDataStream &ds,
*                 RMPalette& pal);
*
* Description:  Operatore di estrazione di palette da data stream
*
* Input:        RMDataStream &ds        Data stream
*               RMPalette& pal          Palette di destinazione
*
* Return:       Reference allo stream
*
\****************************************************************************/

RMDataStream &operator>>(RMDataStream &ds, RMPalette &pal) {
	ds.Read(pal.m_data,1024);
	return ds;
}

/****************************************************************************\
*       Metodi di RMSlot
\****************************************************************************/

/****************************************************************************\
*
* Function:     friend RMDataStream &operator>>(RMDataStream &ds,
*                 RMSlot& slot)
*
* Description:  Operatore per estrarre uno slot di un pattern da un data
*               stream
*
* Input:        RMDataStream &ds        Data stream
*               RMSlot& slot            Slot di destinazione
*
* Return:       Reference allo stream
*
\****************************************************************************/

RMDataStream &operator>>(RMDataStream &ds, RMPattern::RMSlot &slot) {
	slot.ReadFromStream(ds);
	return ds;
}


void RMPattern::RMSlot::ReadFromStream(RMDataStream &ds, bool bLOX) {
	byte type;

	// Type
	ds >> type;
	m_type = (RMPattern::RMSlotType)type;
 
	// Dati
	ds >> m_data;

	// Posizione
	ds >> m_pos;

	// Flag generica
	ds >> m_flag;
}


/****************************************************************************\
*       Metodi di RMPattern
\****************************************************************************/

/****************************************************************************\
*
* Function:     friend RMDataStream &operator>>(RMDataStream &ds,
*                 RMPattern& pat)
*
* Description:  Operatore per estrarre un pattern da un data stream
*
* Input:        RMDataStream &ds        Data stream
*               RMPattern& pat          Pattern di destinazione
*
* Return:       Reference allo stream
*
\****************************************************************************/

RMDataStream &operator>>(RMDataStream &ds, RMPattern &pat) {
	pat.ReadFromStream(ds);
	return ds;
}

void RMPattern::ReadFromStream(RMDataStream &ds, bool bLOX) {
	int i;

	// Nome del pattern
	if (!bLOX)
		ds >> m_name;

	// Velocita'
	ds >> m_speed;

	// Posizione
	ds >> m_pos;

	// Flag di loop del pattern
	ds >> m_bLoop;

	// Numero di slot
	ds >> m_nSlots;

	// Creazione e lettura degli slot
	m_slots = new RMSlot[m_nSlots];

	for (i = 0; i < m_nSlots && !ds.IsError(); i++) {
		if (bLOX)
			m_slots[i].ReadFromStream(ds, true);
		else
			m_slots[i].ReadFromStream(ds, false);
	}
}

void RMPattern::UpdateCoord(void) {
	m_curPos = m_pos + m_slots[m_nCurSlot].Pos();
}

void RMPattern::StopSfx(RMSfx *sfx) {
	for (int i = 0; i < m_nSlots; i++) {
		if (m_slots[i].m_type == SOUND) {
			if (sfx[m_slots[i].m_data].m_name[0] == '_')
				sfx[m_slots[i].m_data].Stop();
			else if (bSkipSfxNoLoop)
				sfx[m_slots[i].m_data].Stop();
		}
	}
}

int RMPattern::Init(RMSfx *sfx, bool bPlayP0, byte *bFlag) {
	int i;

	// Prendiamo il tempo corrente
	m_nStartTime = _vm->GetTime();
	m_nCurSlot = 0;

	// Cerca il primo frame nel pattern
	i = 0;
	while (m_slots[i].m_type != SPRITE) {
		assert(i + 1 < m_nSlots);
		i++;
	}

	m_nCurSlot = i;
	m_nCurSprite = m_slots[i].m_data;
	if (bFlag)
		*bFlag = m_slots[i].m_flag;
	
	// Calcola le coordinate correnti
	UpdateCoord();
	
	// Controlla per il sonoro: 
	//  Se sta alla slot 0, lo playa
	//  Se speed = 0, deve suonare solo se va in loop '_', oppure se specificato dal parametro
	//  Se speed! = 0, suona solo quelli in loop
	for (i = 0;i < m_nSlots; i++) {
		if (m_slots[i].m_type == SOUND) {
			if (i == 0)
			{
				if (sfx[m_slots[i].m_data].m_name[0]=='_')
				{
	  			sfx[m_slots[i].m_data].SetVolume(m_slots[i].Pos().x);
					sfx[m_slots[i].m_data].Play(true);
				}
				else
				{
	  			sfx[m_slots[i].m_data].SetVolume(m_slots[i].Pos().x);
					sfx[m_slots[i].m_data].Play();
				}
			}
			else if (m_speed == 0) {
				if (bPlayP0) {
	  				sfx[m_slots[i].m_data].SetVolume(m_slots[i].Pos().x);
					sfx[m_slots[i].m_data].Play();
				} else if (sfx[m_slots[i].m_data].m_name[0] == '_') {
	  				sfx[m_slots[i].m_data].SetVolume(m_slots[i].Pos().x);
					sfx[m_slots[i].m_data].Play(true);
				}
			} else {
				if (m_bLoop && sfx[m_slots[i].m_data].m_name[0] == '_') {
	  				sfx[m_slots[i].m_data].SetVolume(m_slots[i].Pos().x);
					sfx[m_slots[i].m_data].Play(true);
				}
			}
		}
	}

	return m_nCurSprite;
}

int RMPattern::Update(HANDLE hEndPattern, byte &bFlag, RMSfx *sfx) {
	int CurTime = _vm->GetTime();

	// Se la speed e' 0, il pattern non avanza mai	
	if (m_speed == 0) {
		PulseEvent(hEndPattern);
		bFlag=m_slots[m_nCurSlot].m_flag;
		return m_nCurSprite;
	}

	// E' arrivato il momento di cambiare slot?
	while (m_nStartTime + m_speed <= (uint32)CurTime) {
		m_nStartTime += m_speed; 
		if (m_slots[m_nCurSlot].m_type == SPRITE)
			m_nCurSlot++;
		if (m_nCurSlot == m_nSlots) {
			m_nCurSlot = 0;
			bFlag = m_slots[m_nCurSlot].m_flag;
			PulseEvent(hEndPattern);

			// @@@ Se non c'e' loop avverte che il pattern e' finito
			// Se non c'e' loop rimane sull'ultimo frame
			if (!m_bLoop) {
				m_nCurSlot = m_nSlots - 1;
				bFlag = m_slots[m_nCurSlot].m_flag;
				return m_nCurSprite;			
			}
		}

		for (;;) {
			switch (m_slots[m_nCurSlot].m_type) {
			case SPRITE:
				// Legge il prossimo sprite
				m_nCurSprite = m_slots[m_nCurSlot].m_data;
			
				// Aggiorna le coordinate babbo+figlio
				UpdateCoord();
				break;

			case SOUND:
				if (sfx != NULL) {
					sfx[m_slots[m_nCurSlot].m_data].SetVolume(m_slots[m_nCurSlot].Pos().x);

					if (sfx[m_slots[m_nCurSlot].m_data].m_name[0] != '_')
						sfx[m_slots[m_nCurSlot].m_data].Play(false);
					else
						sfx[m_slots[m_nCurSlot].m_data].Play(true);
				}
				break;

			case COMMAND:
				assert(0);
				break;
			
			default:
				assert(0);
				break;
			}

			if (m_slots[m_nCurSlot].m_type == SPRITE)
				break;
			m_nCurSlot++;
		} 
	}

	// Ritorna lo sprite corrente
	bFlag=m_slots[m_nCurSlot].m_flag;
	return m_nCurSprite;
}

RMPattern::RMPattern() {
	m_slots = NULL;
}

RMPattern::~RMPattern() {
	if (m_slots != NULL)
	{
		delete[] m_slots;
		m_slots = NULL;
	}
}




/****************************************************************************\
*       Metodi di RMSprite
\****************************************************************************/

/****************************************************************************\
*
* Function:     friend RMDataStream &operator>>(RMDataStream &ds,
*                 RMSprite& sprite)
*
* Description:  Operatore per estrarre uno sprite da un data stream
*
* Input:        RMDataStream &ds        Data stream
*               RMItem &item            Sprite di destinazione
*
* Return:       Reference allo stream
*
\****************************************************************************/

RMDataStream &operator>>(RMDataStream &ds, RMSprite &sprite) {
	sprite.ReadFromStream(ds);
	return ds;
}

void RMSprite::Init(RMGfxSourceBuffer *buf) {
	m_buf = buf;
}

void RMSprite::LOXGetSizeFromStream(RMDataStream &ds, int *dimx, int *dimy) {
	int pos = ds.Pos();

	ds >> *dimx >> *dimy;

	ds.Seek(pos, ds.START);
}

void RMSprite::GetSizeFromStream(RMDataStream &ds, int *dimx, int *dimy) {
	int pos = ds.Pos();

	ds >> m_name;
	ds >> *dimx >> *dimy;

	ds.Seek(pos, ds.START);
}

void RMSprite::ReadFromStream(RMDataStream &ds, bool bLOX) {
	int dimx,dimy;
	
	// Nome dello sprite
	if (!bLOX)
		ds >> m_name;

	// Dimensioni
	ds >> dimx >> dimy;

	// Bouding box
	ds >> m_rcBox;

	// Spazio inutilizzato
	if (!bLOX)
		ds+=32;

	// Crezione del buffer e lettura
	m_buf->Init(ds, dimx,dimy);
}

void RMSprite::Draw(RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	m_buf->Draw(bigBuf, prim);
}

void RMSprite::SetPalette(byte *buf) {
	((RMGfxSourceBufferPal*)m_buf)->LoadPalette(buf);
}

RMSprite::RMSprite() {
	m_buf= NULL;
}

RMSprite::~RMSprite() {
	if (m_buf) {
		delete m_buf;
		m_buf = NULL;
	}
}


/****************************************************************************\
*       Metodi di RMSfx
\****************************************************************************/

/****************************************************************************\
*
* Function:     friend RMDataStream &operator>>(RMDataStream &ds,
*                 RMSfx &sfx)
*
* Description:  Operatore per estrarre uno sfx da un data stream
*
* Input:        RMDataStream &ds        Data stream
*               RMSfx &sfx              Sfx di destinazione
*
* Return:       Reference allo stream
*
\****************************************************************************/

RMDataStream &operator>>(RMDataStream &ds, RMSfx &sfx) {
	sfx.ReadFromStream(ds);
	return ds;
}

void RMSfx::ReadFromStream(RMDataStream &ds, bool bLOX) {
	char id[4];
	int size;
	byte *raw;
 
	// Nome dello sfx
	ds >> m_name;
 
	ds >> size;

	// Carica l'effetto sonoro dal buffer
	ds.Read(id,4);

	// Controlla che sia un riff
	assert(id[0] == 'R' && id[1] == 'I' && id[2] == 'F' && id[3] == 'F');

	// Legge la dimensione
	ds >> size;

	// Carica il wav
	raw = new byte[size]; 
	ds.Read(raw, size);

	// Crea l'effetto sonoro
	m_fx = _vm->CreateSFX(raw);
	m_fx->SetLoop(false);

	// Cancella il buffer che non serve pi๙ a nessuno
	delete[] raw;
}

RMSfx::RMSfx() {
	m_fx = NULL;
	m_bPlayingLoop = false;
}

RMSfx::~RMSfx() {
	if (m_fx) {
		m_fx->Release();
		m_fx = NULL;
	}
}

void RMSfx::Play(bool bLoop) {
	if (m_fx && !m_bPlayingLoop) {
		m_fx->SetLoop(bLoop);
		m_fx->Play();

		if (bLoop)
			m_bPlayingLoop=true;
	}
}

void RMSfx::SetVolume(int vol) {
	if (m_fx) {
		m_fx->SetVolume(vol);
	}
}

void RMSfx::Pause(bool bPause) {
	if (m_fx) {
		m_fx->Pause(bPause);
	}
}

void RMSfx::Stop(void) {
	if (m_fx) {
		m_fx->Stop();
		m_bPlayingLoop = false;
	}
}



/****************************************************************************\
*       Metodi di RMItem
\****************************************************************************/

/****************************************************************************\
*
* Function:     friend RMDataStream &operator>>(RMDataStream &ds,
*                 RMItem &item)
*
* Description:  Operatore per estrarre un item da un data stream
*
* Input:        RMDataStream &ds        Data stream
*               RMItem &item            Item di destinazione
*
* Return:       Reference allo stream
*
\****************************************************************************/

RMDataStream &operator>>(RMDataStream &ds, RMItem &item) {
	item.ReadFromStream(ds);
	return ds;
}


RMGfxSourceBuffer *RMItem::NewItemSpriteBuffer(int dimx, int dimy, bool bPreRLE) {
	if (m_cm == CM_256) {
		RMGfxSourceBuffer8RLE *spr;
		
		if (m_FX == 2) {	// AB
			spr = new RMGfxSourceBuffer8RLEWordAB;
		} else if (m_FX == 1) {	// OMBRA+AA
			if (dimx == -1 || dimx > 255)
				spr = new RMGfxSourceBuffer8RLEWordAA;
			else
				spr = new RMGfxSourceBuffer8RLEByteAA;
				
			spr->SetAlphaBlendColor(m_FXparm);
			if (bPreRLE)
				spr->SetAlreadyCompressed();
		} else {
			if (dimx == -1 || dimx > 255)
				spr = new RMGfxSourceBuffer8RLEWord;
			else
				spr = new RMGfxSourceBuffer8RLEByte;

			if (bPreRLE)
				spr->SetAlreadyCompressed();
		}

		return spr;
	} else
		return new RMGfxSourceBuffer16;
}

bool RMItem::IsIn(RMPoint pt, int *size)  { 
	RMRect rc;
	
	if (!m_bIsActive) 
		return false; 
	
	// Cerca il rettangolo giusto da usare, che ่ quello dello sprite se ce l'ha, altrimenti
	// quello generico dell'oggeto
	if (m_nCurPattern != 0 && !m_sprites[m_nCurSprite].m_rcBox.IsEmpty())
		rc=m_sprites[m_nCurSprite].m_rcBox + CalculatePos();
	else if (!m_rcBox.IsEmpty())
		rc = m_rcBox;
	// Se non ha box, esce subito
	else
		return false;
	
	if (size != NULL) 
		*size = rc.Size(); 
		
	return rc.PtInRect(pt + m_curScroll); 
}


void RMItem::ReadFromStream(RMDataStream &ds, bool bLOX) {
	int i, dimx, dimy;
	byte cm;

	// Codice mpal
	ds >> m_mpalCode;

	// Nome dell'oggetto
	ds >> m_name;

	// Z (signed)
	ds >> m_z;

	// Posizione nonno
	ds >> m_pos;

	// Hotspot
	ds >> m_hot;

	// Bounding box
	ds >> m_rcBox;

	// Numero sprite, effetti sonori e pattern
	ds >> m_nSprites >> m_nSfx >> m_nPatterns;

	// Color mode
	ds >> cm; m_cm=(RMColorMode)cm;

	// Flag di presenza della palette differnziata
	ds >> m_bPal;

	if (m_cm == CM_256) {
		//  Se c'e' la palette, leggiamola
		if (m_bPal)
			ds >> m_pal;
	}

	// Dati MPAL
	if (!bLOX)
		ds += 20;
 
	ds >> m_FX;
	ds >> m_FXparm;

	if (!bLOX)
	ds += 106;
 
	// Creazione delle classi
	if (m_nSprites > 0)
		 m_sprites = new RMSprite[m_nSprites];
	if (m_nSfx > 0)
		m_sfx = new RMSfx[m_nSfx];
	m_patterns = new RMPattern[m_nPatterns+1];

	// Lettura delle classi
	if (!ds.IsError())
		for (i = 0; i < m_nSprites && !ds.IsError(); i++) {
		 // Carica lo sprite
		 if (bLOX) {
			 m_sprites[i].LOXGetSizeFromStream(ds, &dimx, &dimy);
			 m_sprites[i].Init(NewItemSpriteBuffer(dimx, dimy, true));
			 m_sprites[i].ReadFromStream(ds, true);
		 } else {
			 m_sprites[i].GetSizeFromStream(ds, &dimx, &dimy);
			 m_sprites[i].Init(NewItemSpriteBuffer(dimx, dimy, false));
			 m_sprites[i].ReadFromStream(ds, false);
		 }

		 if (m_cm == CM_256 && m_bPal)
				m_sprites[i].SetPalette(m_pal.m_data);
   }

	if (!ds.IsError())
		for (i = 0;i < m_nSfx && !ds.IsError(); i++) {
			if (bLOX)
				m_sfx[i].ReadFromStream(ds, true);
			else
				m_sfx[i].ReadFromStream(ds, false);
		}

	// Leggiamo i pattern a partire dal pattern 1
	if (!ds.IsError())
		for (i = 1;i <= m_nPatterns && !ds.IsError(); i++) {
			if (bLOX)
				m_patterns[i].ReadFromStream(ds, true);
			else
				m_patterns[i].ReadFromStream(ds, false);
		}

	// Inizializza il curpattern
	if (m_bInitCurPattern)
		SetPattern(mpalQueryItemPattern(m_mpalCode));

	// Inizializza lo stato di attivazione
	m_bIsActive=mpalQueryItemIsActive(m_mpalCode);
}


RMGfxPrimitive *RMItem::NewItemPrimitive() {
	return new RMGfxPrimitive(this);
}

void RMItem::SetScrollPosition(RMPoint scroll) {
	m_curScroll = scroll;
}

bool RMItem::DoFrame(RMGfxTargetBuffer *bigBuf, bool bAddToList) {
	int oldSprite = m_nCurSprite;

	// Pattern 0 = Non disegnare nulla!
	if (m_nCurPattern == 0)
		return false;

	// Facciamo un update del pattern, che ci ritorna anche il frame corrente
	if (m_nCurPattern != 0)
		m_nCurSprite=m_patterns[m_nCurPattern].Update(m_hEndPattern,m_bCurFlag, m_sfx);

	// Se la funzione ha ritornato -1, vuol dire che il pattern e' finito
	if (m_nCurSprite == -1) {
		// Mettiamo il pattern 0, e usciamo. La classe si auto-deregistrera' della OT list
		m_nCurPattern = 0;
		return false;
	}

	// Se non siamo in OT list, mettiamoci
	if (!m_nInList && bAddToList)
		bigBuf->AddPrim(NewItemPrimitive());

	return oldSprite != m_nCurSprite;
}

RMPoint RMItem::CalculatePos(void) {
	return m_pos + m_patterns[m_nCurPattern].Pos();
}

void RMItem::Draw(RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	// Se CurSprite == -1, allora e' finito il pattern
	if (m_nCurSprite == -1)
	  return;
	
	// Settiamo la flag
	prim->SetFlag(m_bCurFlag);

	// Offset inverso per lo scrolling
	prim->Dst().Offset(-m_curScroll);

	// Dobbiamo sparaflashare le coordinate dell'item dentro la primitiva.
	// Si calcola nonno+(babbo+figlio)
	prim->Dst().Offset(CalculatePos());

	// No stretching, please
	prim->SetStrecth(false);

	// Ora la passiamo alla routine di drawing generica per surface
	m_sprites[m_nCurSprite].Draw(bigBuf, prim);
}


bool RMItem::RemoveThis() {
	// Rimuove dalla OT list se il frame corrente e' -1 (pattern finito)
	return (m_nCurSprite == -1);
}


void RMItem::SetStatus(int nStatus) {
	m_bIsActive = (nStatus>0);
}

void RMItem::SetPattern(int nPattern, bool bPlayP0) {
	int i;

	assert(nPattern >= 0 && nPattern <= m_nPatterns);

	if (m_sfx)
		if (m_nCurPattern>0)
			m_patterns[m_nCurPattern].StopSfx(m_sfx);
	
	// Si ricorda il pattern corrente
	m_nCurPattern = nPattern;

	// Inizia il pattern per cominciare l'animazione
	if (m_nCurPattern != 0)
		m_nCurSprite = m_patterns[m_nCurPattern].Init(m_sfx, bPlayP0, &m_bCurFlag);
	else {
		m_nCurSprite = -1;
		
		// Cerca l'effetto sonoro per il pattern 0
		if (bPlayP0)
			for (i = 0;i < m_nSfx; i++)
				if (strcmp(m_sfx[i].m_name, "p0") == 0)
					m_sfx[i].Play();
	}
}


bool RMItem::GetName(RMString& name)
{
	char buf[256];

	mpalQueryItemName(m_mpalCode, buf);
	name = buf;
	if (buf[0] == '\0')
		return false;
	return true; 
}


void RMItem::Unload(void) {
	if (m_patterns != NULL)
	{
		delete[] m_patterns;
		m_patterns = NULL;
	}
	
	if (m_sprites != NULL) {
		delete[] m_sprites;
		m_sprites = NULL;
	}
	
	if (m_sfx != NULL) {
		delete[] m_sfx;
		m_sfx = NULL;
	}
}

RMItem::RMItem() {
	m_bCurFlag = 0;
	m_patterns = NULL;
	m_sprites = NULL;
	m_sfx= NULL;
	m_curScroll.Set(0, 0);
	m_bInitCurPattern=true;
	m_nCurPattern = 0;

	m_hEndPattern = CreateEvent(NULL, false, false, NULL);
}

RMItem::~RMItem() {
	Unload();	
	CloseHandle(m_hEndPattern);
}

void RMItem::WaitForEndPattern(HANDLE hCustomSkip) {
	if (m_nCurPattern != 0) {
		if (hCustomSkip == INVALID_HANDLE_VALUE)
			WaitForSingleObject(m_hEndPattern,INFINITE);
		else {
			HANDLE h[2];

			h[0] = hCustomSkip;
			h[1] = m_hEndPattern;
			WaitForMultipleObjects(2, h, false, INFINITE);
		}
	}
}

void RMItem::ChangeHotspot(RMPoint pt) {
	m_hot = pt;
}

void RMItem::PlaySfx(int nSfx) {
	if (nSfx < m_nSfx)
		m_sfx[nSfx].Play();
}

void RMItem::PauseSound(bool bPause) {
	int i;

	for (i = 0; i < m_nSfx; i++)
		m_sfx[i].Pause(bPause);
}



/****************************************************************************\
*       Metodi di RMWipe
\****************************************************************************/


RMWipe::RMWipe() {
	m_hUnregistered=CreateEvent(NULL,false,false,NULL);
	m_hEndOfFade=CreateEvent(NULL,false,false,NULL);
}

RMWipe::~RMWipe() {
	CloseHandle(m_hUnregistered);
	CloseHandle(m_hEndOfFade);
}

int RMWipe::Priority(void) {
	return 200;
}

void RMWipe::Unregister(void) {
	RMGfxTask::Unregister();
	assert(m_nInList == 0);
	SetEvent(m_hUnregistered);
}

bool RMWipe::RemoveThis(void) {
	return m_bUnregister;
}

void RMWipe::WaitForFadeEnd(void) {
	WaitForSingleObject(m_hEndOfFade, INFINITE);	
	m_bEndFade = true;
	m_bFading = false;
	MainWaitFrame();
	MainWaitFrame();
}

void RMWipe::CloseFade(void) {
//	m_bUnregister=true;
//	WaitForSingleObject(m_hUnregistered,INFINITE);
	m_wip0r.Unload();
}

void RMWipe::InitFade(int type) {
	// Attiva il fade
	m_bUnregister = false;
	m_bEndFade = false;

	m_nFadeStep = 0;

	m_bMustRegister = true;

	RMRes res(RES_W_CERCHIO);
	RMDataStream ds;

	ds.OpenBuffer(res);
	ds >> m_wip0r;
	ds.Close();
	
	m_wip0r.SetPattern(1);

	m_bFading = true;
}

void RMWipe::DoFrame(RMGfxTargetBuffer &bigBuf) {
	if (m_bMustRegister) {
		bigBuf.AddPrim(new RMGfxPrimitive(this));
		m_bMustRegister = false;
	}
	
	if (m_bFading)
	{
		m_wip0r.DoFrame(&bigBuf, false);

		m_nFadeStep++;
	
		if (m_nFadeStep == 10) {
			SetEvent(m_hEndOfFade);
		}
	}
}

void RMWipe::Draw(RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	if (m_bFading) {
		m_wip0r.Draw(bigBuf, prim);
	}

	if (m_bEndFade)
		Common::fill((byte *)bigBuf, (byte *)bigBuf + bigBuf.Dimx() * bigBuf.Dimy() * 2, 0x0);
}



/****************************************************************************\
*       Metodi di RMCharacter
\****************************************************************************/

/***************************************************************************/
/* Cerca il percorso minimo tra due nodi del grafo di connessione dei BOX  */
/* Restituisce il percorso lungo pathlenght nel vettore path[]             */
/***************************************************************************/

short RMCharacter::FindPath(short source, short destination) {
	// FIXME: Refactor
	static RMBox BOX[MAXBOXES];            // Matrice di Adjacenza
	static short COSTO[MAXBOXES];               // Costi per Nodo
	static short VALIDO[MAXBOXES];              // 0:non valido 1:valido 2:saturo
	static short NEXT[MAXBOXES];                // Prossimo Nodo
	short i, j, k, costominimo, fine, errore = 0;
	RMBoxLoc *cur;

	g_system->lockMutex(csMove);  

	if (source == -1 || destination == -1) {
		g_system->unlockMutex(csMove); 
		return 0;
	}

	// Si fa dare i box
	cur = theBoxes->GetBoxes(curLocation);

	// Effettua una copia di riserva per lavorarci
	for (i = 0; i < cur->numbbox; i++)
		memcpy(&BOX[i], &cur->boxes[i], sizeof(RMBox));

	// Invalida tutti i Nodi
	for (i = 0; i < cur->numbbox; i++) 
		VALIDO[i] = 0;
	
	// Prepara sorgente e variabili globali alla procedura
	COSTO[source] = 0;
	VALIDO[source] = 1;
	fine = 0;
 
 	 // Ricerca del percorso minimo
	while(!fine) {
		costominimo = 32000;                  // risetta il costo minimo
		errore = 1;                           // errore possibile

		// 1ฐ ciclo : ricerca di possibili nuovi nodi
		for (i = 0; i < cur->numbbox; i++)
			if (VALIDO[i] == 1) {
				errore = 0;                                        // errore sfatato
				j = 0;
				while (((BOX[i].adj[j]) != 1) && (j < cur->numbbox)) 
					j++;
      
				if (j >= cur->numbbox) 
					VALIDO[i] = 2;                     // nodo saturo?
				else {
					NEXT[i] = j;
					if (COSTO[i] + 1 < costominimo) 
						costominimo = COSTO[i] + 1;
				}
			}

		if (errore) 
			fine = 1;                                 // tutti i nodi saturi

		// 2ฐ ciclo : aggiunta nuovi nodi trovati , saturazione nodi vecchi
		for (i = 0; i < cur->numbbox; i++)
			if ((VALIDO[i] == 1) && ((COSTO[i] + 1) == costominimo)) {
				BOX[i].adj[NEXT[i]] = 2;
				COSTO[NEXT[i]] = costominimo;
				VALIDO[NEXT[i]] = 1;
				for (j = 0; j < cur->numbbox; j++)
					if (BOX[j].adj[NEXT[i]] == 1) 
						BOX[j].adj[NEXT[i]] = 0;
				
				if (NEXT[i] == destination) 
					fine = 1;
			}
	}

	// Estrazione del percorso dalla matrice di adiacenza modificata
	if (!errore) {
		pathlenght = COSTO[destination];
		k = pathlenght;
		path[k] = destination;
		
		while (path[k] != source) {
			i = 0;
			while (BOX[i].adj[path[k]] != 2)
				i++;
			k--;
			path[k] = i;
		}
   
		pathlenght++;
	}

	g_system->unlockMutex(csMove);

	return !errore;
}


void RMCharacter::GoTo(RMPoint destcoord, bool bReversed) {
	if (m_pos == destcoord) {
		if (minpath == 0) {
			Stop();
			PulseEvent(hEndOfPath);
			return;
		}
	}

	status = WALK;
	linestart = m_pos;
	lineend = destcoord;
	dx = linestart.x - lineend.x;
	dy = linestart.y - lineend.y;
	fx = dx;
	fy = dy;
	dx = ABS(dx);
	dy = ABS(dy);
	walkspeed = curSpeed;
	walkcount = 0;

	if (bReversed) {
		while (0) ;	
	}

	int nPatt = GetCurPattern();

	if (dx > dy) {
		slope = fy / fx;
		if (lineend.x < linestart.x) 
			walkspeed = -walkspeed;
		walkstatus = 1;
    
		// Cambia il proprio pattern per la nuova direzione
		bNeedToStop = true;
		if ((walkspeed < 0 && !bReversed) || (walkspeed >= 0 && bReversed))  {
   		if (nPatt != PAT_WALKLEFT)
				SetPattern(PAT_WALKLEFT);  
		} else {
   			if (nPatt != PAT_WALKRIGHT)
				SetPattern(PAT_WALKRIGHT);
		}
	} else {
		slope = fx / fy;
		if (lineend.y < linestart.y) 
			walkspeed = -walkspeed;
		walkstatus = 0;
    
		bNeedToStop=true;
		if ((walkspeed < 0 && !bReversed) || (walkspeed >= 0 && bReversed)) {
   			if (nPatt != PAT_WALKUP)
				SetPattern(PAT_WALKUP);  
		} else {
   			if (nPatt != PAT_WALKDOWN)
				SetPattern(PAT_WALKDOWN);
		}
	}

	olddx = dx;
	olddy = dy;

	// ResetEvent(hTonyEndMovement);  @@@
}


RMPoint RMCharacter::Searching(char UP, char DOWN, char RIGHT, char LEFT, RMPoint punto) {
	short passi, minimo;
	RMPoint nuovo, trovato;
	minimo = 32000;

	if (UP) {
		nuovo = punto;
		passi = 0;
		while((InWhichBox(nuovo) == -1) && (nuovo.y >= 0)) { nuovo.y--; passi++; }
		if ((InWhichBox(nuovo) != -1) && (passi < minimo)&&
				FindPath(InWhichBox(m_pos), InWhichBox(nuovo))) {
			minimo = passi;
			nuovo.y--;       // to avoid error?
			trovato = nuovo;
		}
	}

	if (DOWN) {
		nuovo = punto;
		passi = 0;
		while ((InWhichBox(nuovo) == -1) && (nuovo.y < 480)) { nuovo.y++; passi++; }
		if ((InWhichBox(nuovo) != -1) && (passi < minimo) &&
				FindPath(InWhichBox(m_pos), InWhichBox(nuovo))) {
			minimo = passi;
			nuovo.y++;     // to avoid error?
			trovato = nuovo;
		}
	}

	if (RIGHT) {
		nuovo = punto;
		passi = 0;
		while ((InWhichBox(nuovo) == -1) && (nuovo.x < 640)) { nuovo.x++; passi++; }
		if ((InWhichBox(nuovo) != -1) && (passi < minimo) &&
				FindPath(InWhichBox(m_pos), InWhichBox(nuovo))) {
			minimo = passi;
			nuovo.x++;     // to avoid error?
			trovato = nuovo;
		}
	}

	if (LEFT) {
		nuovo = punto;
		passi = 0;
		while ((InWhichBox(nuovo) == -1) && (nuovo.x >= 0)) { nuovo.x--; passi++; }
		if ((InWhichBox(nuovo) != -1) && (passi < minimo) &&
				FindPath(InWhichBox(m_pos), InWhichBox(nuovo))) {
			minimo = passi;
			nuovo.x--;     // to avoid error?
			trovato = nuovo;
		}
	}

	if (minimo == 32000) trovato = punto;
	return trovato;
}


RMPoint RMCharacter::NearestPoint(RMPoint punto) {
/*
 RMPoint tofind;
 signed short difx,dify;

 difx = m_pos.x-punto.x;
 dify = m_pos.y-punto.y;

 if ((difx>0) && (dify>0)) tofind=Searching(0,1,1,0,punto);
 if ((difx>0) && (dify<0)) tofind=Searching(1,0,1,0,punto);
 if ((difx<0) && (dify>0)) tofind=Searching(0,1,0,1,punto);
 if ((difx<0) && (dify<0)) tofind=Searching(1,0,0,1,punto);

 // potrebbero essere tolti? Pensaci @@@@
 if ((difx= = 0) && (dify>0)) tofind=Searching(0,1,1,1,punto);
 if ((difx= = 0) && (dify<0)) tofind=Searching(1,0,1,1,punto);
 if ((dify= = 0) && (difx>0)) tofind=Searching(1,1,1,0,punto);
 if ((dify= = 0) && (difx<0)) tofind=Searching(1,1,0,1,punto);

 if ((dify= = 0) && (difx= = 0)) tofind=punto;

 return tofind;
*/
	return Searching(1, 1, 1, 1, punto);
}


short RMCharacter::ScanLine(RMPoint punto) {
	int Ldx, Ldy, Lcount;
	float Lfx, Lfy, Lslope;
	RMPoint Lstart, Lend, Lscan;
	signed char Lspeed, Lstatus;

	Lstart = m_pos;
	Lend = punto;
	Ldx = Lstart.x-Lend.x;
	Ldy = Lstart.y-Lend.y;
	Lfx = Ldx;
	Lfy = Ldy;
	Ldx = ABS(Ldx);
	Ldy = ABS(Ldy);
	Lspeed = 1;
	Lcount = 0;

	if (Ldx > Ldy) {
		Lslope = Lfy / Lfx;
		if (Lend.x < Lstart.x) Lspeed = -Lspeed;
		Lstatus = 1;
	} else {
		Lslope = Lfx / Lfy;
		if (Lend.y < Lstart.y) Lspeed =- Lspeed;
		Lstatus = 0;
	}

	Lscan = Lstart;   // Inizio scansione
	while (InWhichBox(Lscan) != -1) {
		Lcount++;
		if (Lstatus) {
			Ldx = Lspeed * Lcount;
			Ldy = Lslope * Ldx;
		} else {
			Ldy = Lspeed * Lcount;
			Ldx = Lslope * Ldy;
		}

		Lscan.x = Lstart.x + Ldx;
		Lscan.y = Lstart.y + Ldy;
   
		if ((ABS(Lscan.x - Lend.x) <= 1) && (ABS(Lscan.y - Lend.y) <= 1)) return 1;
	}

	return 0;
}

// Calcola intersezioni tra la traiettoria rettilinea ed il pi๙ vicino BBOX
RMPoint RMCharacter::InvScanLine(RMPoint punto) {
	int Ldx, Ldy, Lcount;
	float Lfx, Lfy, Lslope;
	RMPoint Lstart, Lend, Lscan;
	signed char Lspeed, Lstatus, Lbox = -1;

	Lstart = punto;      // Exchange!
	Lend = m_pos;    // :-)
	Ldx = Lstart.x - Lend.x;
	Ldy = Lstart.y - Lend.y;
	Lfx = Ldx;
	Lfy = Ldy;
	Ldx = ABS(Ldx);
	Ldy = ABS(Ldy);
	Lspeed = 1;
	Lcount = 0;
 
	if (Ldx > Ldy) {
		Lslope = Lfy / Lfx;
		if (Lend.x < Lstart.x) Lspeed = -Lspeed;
		Lstatus=1;
	} else {
		Lslope = Lfx / Lfy;
		if (Lend.y < Lstart.y) Lspeed = -Lspeed;
		Lstatus = 0;
	}
	Lscan = Lstart;

	for (;;) {
		if (InWhichBox(Lscan) != -1) {
			if (InWhichBox(Lscan) != Lbox) {
				if (InWhichBox(m_pos) == InWhichBox(Lscan) || FindPath(InWhichBox(m_pos),InWhichBox(Lscan)))
					return Lscan;
				else 
					Lbox = InWhichBox(Lscan);
			}
		}

		Lcount++;
		if (Lstatus) {
			Ldx = Lspeed * Lcount;
			Ldy = Lslope * Ldx;
        } else {
			Ldy = Lspeed * Lcount;
			Ldx = Lslope * Ldy;
         }
		Lscan.x = Lstart.x + Ldx;
		Lscan.y = Lstart.y + Ldy;
	}
}


/***************************************************************************/
/* Ritorna la coordinata dell'HotSpot di uscita pi๙ vicino al giocatore    */
/***************************************************************************/

RMPoint RMCharacter::NearestHotSpot(int sourcebox, int destbox) {
	RMPoint puntocaldo;
	short cc;
	int x, y, distanzaminima;
	distanzaminima = 10000000;
	RMBoxLoc *cur = theBoxes->GetBoxes(curLocation);
 
	for (cc = 0; cc < cur->boxes[sourcebox].numhotspot; cc++)
		if ((cur->boxes[sourcebox].hotspot[cc].destination) == destbox) {
			x = ABS(cur->boxes[sourcebox].hotspot[cc].hotx - m_pos.x);
			y = ABS(cur->boxes[sourcebox].hotspot[cc].hoty - m_pos.y);
      
			if ((x * x + y * y) < distanzaminima) {
				distanzaminima = x * x + y * y;
				puntocaldo.x = cur->boxes[sourcebox].hotspot[cc].hotx;
				puntocaldo.y = cur->boxes[sourcebox].hotspot[cc].hoty;
			}
		}
 
	return puntocaldo;
}

void RMCharacter::Draw(RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	if (bDrawNow) {
		prim->Dst() += m_fixedScroll;

		RMItem::Draw(bigBuf, prim);
	}
}

void RMCharacter::NewBoxEntered(int nBox) {
	RMBoxLoc *cur;
	bool bOldReverse;
	
	// Richiama la On ExitBox
	mpalQueryDoAction(3, curLocation, curbox);

	cur = theBoxes->GetBoxes(curLocation);
	bOldReverse = cur->boxes[curbox].bReversed;
	curbox = nBox;

	// Se ่ cambiata la Z, dobbiamo rimuoverlo dalla OT
	if (cur->boxes[curbox].Zvalue != m_z) {
		bRemoveFromOT = true;
		m_z = cur->boxes[curbox].Zvalue;
	}

	// Gestisce l'inversione del movimento SOLO se non siamo nel percorso minimo: se siamo in percorso
	//  minimo ่ direttamente la DoFrame a farlo
	if (bMovingWithoutMinpath) {
		if ((cur->boxes[curbox].bReversed && !bOldReverse) || (!cur->boxes[curbox].bReversed && bOldReverse)) {
			switch (GetCurPattern()) {
			case PAT_WALKUP:
				SetPattern(PAT_WALKDOWN);
				break;
			case PAT_WALKDOWN:
				SetPattern(PAT_WALKUP);
				break;
			case PAT_WALKRIGHT:
				SetPattern(PAT_WALKLEFT);
				break;
			case PAT_WALKLEFT:
				SetPattern(PAT_WALKRIGHT);
				break;
			}
		}
	}

	// Richiama la On EnterBox
	mpalQueryDoAction(2, curLocation, curbox);
}
	
void RMCharacter::DoFrame(RMGfxTargetBuffer* bigBuf, int loc) {
	bool bEndNow;

	bEndNow = false;
	bEndOfPath = false;
	bDrawNow = (curLocation == loc);

	g_system->lockMutex(csMove);

	// Se stiamo camminando...
	if (status != STAND) {
		// Se stiamo andando in orizzontale
		if (walkstatus == 1) {
			dx = walkspeed * walkcount;
			dy = slope * dx;
			m_pos.x = linestart.x + dx;
			m_pos.y = linestart.y + dy;

			// Destra
			if (((walkspeed > 0) && (m_pos.x > lineend.x)) || ((walkspeed < 0) && (m_pos.x < lineend.x))) {
				m_pos = lineend;
				status = STAND;
				bEndNow = true;
			}
		}
    
		// Se stiamo andando in verticale
		if (walkstatus == 0) {
			dy = walkspeed * walkcount;
			dx = slope * dy;
			m_pos.x = linestart.x + dx;
			m_pos.y = linestart.y + dy;

			// Basso
			if (((walkspeed > 0) && (m_pos.y > lineend.y)) || ((walkspeed < 0) && (m_pos.y < lineend.y))) {
				m_pos = lineend;
				status = STAND;
				bEndNow = true;
			}
		}

		// Controlla se il personaggio ่ uscito dai BOX per errore, nel qual caso
		//  lo fa rientrare subito
		if (InWhichBox(m_pos) == -1) {
			m_pos.x = linestart.x + olddx;
			m_pos.y = linestart.y + olddy;
		}

		// Se siamo appena arrivati alla destinazione temporanea ed ่ finito il percorso minimo, 
		// ci fermiamo definitivamente
		if (bEndNow && minpath == 0) {
			if (!bEndOfPath)
				Stop();
			bEndOfPath = true;
			PulseEvent(hEndOfPath);
		}
		
		walkcount++;

		// Aggiorna la Z del personaggio @@@ bisognerebbe rimuoverlo solo se ่ cambiata la Z
		
		// Controlla se abbiamo cambiato box
		if (!theBoxes->IsInBox(curLocation, curbox, m_pos))
			NewBoxEntered(InWhichBox(m_pos));

		// Aggiorna le vecchie coordinate
		olddx = dx;
		olddy = dy;
	}

	// Se siamo fermi
	if (status == STAND) {
		// Controlliamo se c'่ ancora percorso minimo da calcolare
		if (minpath == 1) {
			RMBoxLoc *cur = theBoxes->GetBoxes(curLocation);

			// Se dobbiamo ancora attraversare un box
			if (pathcount < pathlenght) {
				// Controlliamo che il box su cui stiamo entrando sia attivo
				if (cur->boxes[path[pathcount-1]].attivo) {
					// Muoviti in linea retta verso l'hotspot pi๙ vicino, tenendo conto del reversing please
					// NEWBOX = path[pathcount-1]
					GoTo(NearestHotSpot(path[pathcount-1], path[pathcount]), cur->boxes[path[pathcount-1]].bReversed);
					pathcount++;
				} else {
					// Se il box ่ disattivato, possiamo solo bloccare tutto
					// @@@ Questo non dovrebbe pi๙ avvenire, dato che abbiamo migliorato
					// la ricerca del percorso minimo
					minpath = 0;
					if (!bEndOfPath)
						Stop();
					bEndOfPath = true;
					PulseEvent(hEndOfPath);
				}
			} else {
				// Se siamo giเ entrati nell'ultimo box, dobbiamo solo muoverci in linea retta verso il
				//  punto di arrivo
				// NEWBOX = InWhichBox(pathend)
				minpath = 0;
				GoTo(pathend, cur->boxes[InWhichBox(pathend)].bReversed);
			}
		}
	}

	g_system->unlockMutex(csMove);

	// Richiama il DoFrame dell'item
	RMItem::DoFrame(bigBuf);
}

void RMCharacter::Stop(void) {
	bMoving = false;

	// Non si sa mai...
	status = STAND;
	minpath = 0;

	if (!bNeedToStop)
		return;

	bNeedToStop = false;

	switch (GetCurPattern()) {
	case PAT_WALKUP:
		SetPattern(PAT_STANDUP);
		break;

	case PAT_WALKDOWN:
		SetPattern(PAT_STANDDOWN);
		break;

	case PAT_WALKLEFT:
		SetPattern(PAT_STANDLEFT);
		break;

	case PAT_WALKRIGHT:
		SetPattern(PAT_STANDRIGHT);
		break;
	
	default:
//			assert(0);
//			MessageBox(NULL,"E' lo stesso errore di prima, ma non crasha","Ehi!",MB_OK);
		SetPattern(PAT_STANDDOWN);
		break;
	}
}

inline int RMCharacter::InWhichBox(RMPoint pt) { 
	return theBoxes->WhichBox(curLocation,pt); 
}


bool RMCharacter::Move(RMPoint pt) {
	RMPoint dest;
	int numbox;

	bMoving = true;
	
	// Se 0,0, non fare nulla, anzi fermati
	if (pt.x == 0 && pt.y == 0) {
		minpath = 0;
		status = STAND;
		Stop();
		return true;
	}

	// Se clicko fuori dai box
 	numbox = InWhichBox(pt);
	if (numbox == -1) {
		// Trova il punto pi๙ vicino dentro i box
		dest = NearestPoint(pt);

		// ???!??
		if (dest == pt)
			dest = InvScanLine(pt);

		pt = dest;
		numbox = InWhichBox(pt);
	}

	RMBoxLoc *cur = theBoxes->GetBoxes(curLocation);

	minpath = 0;
	status = STAND;
	bMovingWithoutMinpath = true;
	if (ScanLine(pt)) 
		GoTo(pt, cur->boxes[numbox].bReversed);
	else if (FindPath(InWhichBox(m_pos), InWhichBox(pt))) {
		bMovingWithoutMinpath = false;
		minpath = 1;
		pathcount = 1;
		pathend = pt;
	} else {
		// @@@ Questo caso ่ se un hotspot ่ dentro un box
		//  ma non c'่ un path per arrivarci. Usiamo quindi
		//  la invscanline per cercare un punto intorno
		dest = InvScanLine(pt);
		pt = dest;
		
		if (ScanLine(pt)) 
			GoTo(pt,cur->boxes[numbox].bReversed);
		else if (FindPath(InWhichBox(m_pos), InWhichBox(pt))) {
			bMovingWithoutMinpath = false;
			minpath = 1;
			pathcount = 1;
			pathend = pt;
			return true;
		} else
			return false;
	}

	return true;
}

void RMCharacter::SetPosition(RMPoint pt, int newloc) {
	RMBoxLoc *box;
	
	minpath = 0;
	status = STAND;
	m_pos = pt;
	
	if (newloc != -1)
		curLocation = newloc;

	// Aggiorna la Z del personaggio
	box = theBoxes->GetBoxes(curLocation);
	curbox = InWhichBox(m_pos);
	assert(curbox != -1);
	m_z = box->boxes[curbox].Zvalue;
	bRemoveFromOT = true;
}

bool RMCharacter::RemoveThis(void) {
	if (bRemoveFromOT)
		return true;

	return RMItem::RemoveThis();
}

RMCharacter::RMCharacter() {
//	InitializeCriticalSection(&csMove);
	hEndOfPath = CreateEvent(NULL, false, false, NULL);
	minpath = 0;
	curSpeed = 3;
	bRemoveFromOT = false;
	bMoving = false;

	m_pos.Set(0, 0);
}

RMCharacter::~RMCharacter() {
//	DeleteCriticalSection(&csMove);
	CloseHandle(hEndOfPath);
}

void RMCharacter::LinkToBoxes(RMGameBoxes *boxes) {
	theBoxes = boxes;
}

/****************************************************************************\
*       RMBox Methods
\****************************************************************************/

void RMBox::ReadFromStream(RMDataStream &ds) {
	uint16 w;
	int i;
	byte b;

	// Bbox
	ds >> left;
	ds >> top;
	ds >> right;
	ds >> bottom;

	// Adiacenza
	for (i = 0; i < MAXBOXES; i++)
	{
		ds >> adj[i];
	}

	// Misc
	ds >> numhotspot;
	ds >> Zvalue;
	ds >> b;
	attivo = b;
	ds >> b;
	bReversed = b;

	// Spazio di espansione
	ds+=30;

	// Hotspots
	for (i = 0; i < numhotspot; i++) {
		ds >> w; hotspot[i].hotx = w;
		ds >> w; hotspot[i].hoty = w;
		ds >> w; hotspot[i].destination = w;
	}
}

RMDataStream &operator>>(RMDataStream &ds, RMBox &box) {
	box.ReadFromStream(ds);

	return ds;
}

/****************************************************************************\
*       RMBoxLoc Methods
\****************************************************************************/

void RMBoxLoc::ReadFromStream(RMDataStream &ds) {
	int i;
	char buf[2];
	byte ver;

	// ID and versione
	ds >> buf[0] >> buf[1] >> ver;
	assert(buf[0] == 'B' && buf[1] == 'X');
	assert(ver == 3);

	// Numero dei box
	ds >> numbbox;

	// Alloca la memoria per i box
	boxes = new RMBox[numbbox];

	// Li legge da disco
	for (i = 0; i < numbbox; i++)
		ds >> boxes[i];
}


void RMBoxLoc::RecalcAllAdj(void) {
	int i, j;

	for (i = 0; i < numbbox; i++) {
		Common::fill(boxes[i].adj, boxes[i].adj + MAXBOXES, 0);

		for (j = 0; j < boxes[i].numhotspot; j++)
			if (boxes[boxes[i].hotspot[j].destination].attivo)
				boxes[i].adj[boxes[i].hotspot[j].destination] = 1;
	}
}

RMDataStream &operator>>(RMDataStream &ds, RMBoxLoc &bl) {
	bl.ReadFromStream(ds);

	return ds;
}

/****************************************************************************\
*       RMGameBoxes methods
\****************************************************************************/

void RMGameBoxes::Init(void) {
	int i;
	RMString fn;
	RMDataStream ds;

	// Load boxes from disk
	m_nLocBoxes = 130;
	for (i=1; i <= m_nLocBoxes; i++) {
		RMRes res(10000 + i);

		ds.OpenBuffer(res);
		
		m_allBoxes[i] = new RMBoxLoc();
		ds >> *m_allBoxes[i];

		m_allBoxes[i]->RecalcAllAdj();

		ds.Close();
	}
}

void RMGameBoxes::Close(void) {
}

RMBoxLoc *RMGameBoxes::GetBoxes(int nLoc) {
	return m_allBoxes[nLoc];
}

bool RMGameBoxes::IsInBox(int nLoc, int nBox, RMPoint pt) {
	RMBoxLoc *cur = GetBoxes(nLoc);

	if ((pt.x >= cur->boxes[nBox].left) && (pt.x <= cur->boxes[nBox].right) &&
			(pt.y >= cur->boxes[nBox].top)  && (pt.y <= cur->boxes[nBox].bottom)) 
		return true;
	else
		return false;
}

int RMGameBoxes::WhichBox(int nLoc, RMPoint punto) {
	int i;
	RMBoxLoc *cur = GetBoxes(nLoc);
	
	if (!cur) return -1;

	for (i = 0; i<cur->numbbox; i++)
		if (cur->boxes[i].attivo)
		  if ((punto.x >= cur->boxes[i].left) && (punto.x <= cur->boxes[i].right) &&
					(punto.y >= cur->boxes[i].top)  && (punto.y <= cur->boxes[i].bottom)) 
				return i;

	return -1;
}

void RMGameBoxes::ChangeBoxStatus(int nLoc, int nBox, int status) {
	m_allBoxes[nLoc]->boxes[nBox].attivo=status;
	m_allBoxes[nLoc]->RecalcAllAdj();
}


int RMGameBoxes::GetSaveStateSize(void) {
	int size;
	int i;

	size=4;

	for (i=1; i <= m_nLocBoxes; i++) {
		size += 4;
		size += m_allBoxes[i]->numbbox;
	}

	return size;
}

void RMGameBoxes::SaveState(byte *state) {
	int i,j;
	
	// Save the number of locations with boxes
	WRITE_LE_UINT32(state, m_nLocBoxes);
	state += 4;

	// For each location, write out the number of boxes and their status
	for (i=1; i <= m_nLocBoxes; i++) {
		WRITE_LE_UINT32(state, m_allBoxes[i]->numbbox);
		state+=4;
				
		for (j = 0; j < m_allBoxes[i]->numbbox; j++)
			*state++ = m_allBoxes[i]->boxes[j].attivo;
	}
}

void RMGameBoxes::LoadState(byte *state) {
	int i,j;
	int nloc,nbox;

	// Load number of locations with box
	nloc = *(int*)state;
	state+=4;

	// Controlla che siano meno di quelli correnti
	assert(nloc <= m_nLocBoxes);

	// Per ogni locazione, salva il numero di box e il loro stato
	for (i = 1; i <= nloc; i++) {
		nbox = READ_LE_UINT32(state);
		state += 4;

		for (j = 0; j<nbox ; j++) {
			if (j < m_allBoxes[i]->numbbox)	
				m_allBoxes[i]->boxes[j].attivo = *state;

			state++;
		}

		m_allBoxes[i]->RecalcAllAdj();
	}
}

/****************************************************************************\
*       Metodi di RMLocation
\****************************************************************************/

/****************************************************************************\
*
* Function:     RMLocation::RMLocation();
*
* Description:  Costruttore standard
*
\****************************************************************************/

RMLocation::RMLocation() {
	m_nItems = 0;
	m_items = NULL;
	m_buf = NULL;
}


/****************************************************************************\
*
* Function:     bool RMLocation::Load(char *lpszFileName);
*
* Description:  Carica una locazione (.LOC) da un file di cui viene fornito
*               il pathname.
*
* Input:        char *lpszFileName      Nome del file di dati
*
* Return:       true se tutto OK, false in caso di errore
*
\****************************************************************************/

bool RMLocation::Load(const char *lpszFileName) {
	Common::File f;
	bool bRet;

	// Apre il file in lettura
	if (!f.open(lpszFileName))
		return false;

	// Lo passa alla routine di loading da file aperto
	bRet = Load(f);

	// Chiude il file
	f.close();

	return bRet;
}


/****************************************************************************\
*
* Function:     bool RMLocation::Load(HANDLE hFile);
*
* Description:  Carica una locazione (.LOC) da un handle di file aperto
*
* Input:        HANDLE hFile            Handle del file
*
* Return:       true se tutto OK, false in caso di errore
*
\****************************************************************************/

bool RMLocation::Load(Common::File &file) {
	int size;
//	byte *buf;
//	uint32 dwReadBytes;
	bool bRet;

	// Calcola la lunghezza del file
	size = file.size();
	file.seek(0);

/*
 // Alloca la memoria per caricare il file in memoria
 buf=(LPBYTE)GlobalAlloc(GMEM_FIXED,size);

 // Legge il file in memoria
 ReadFile(hFile,buf,size,&dwReadBytes,0);

 // Parsing del file, utilizzando la funzione di load da memorira
 bRet=Load(buf);

 // Free della memoria
 GlobalFree(buf);
*/

	RMFileStreamSlow fs;

	fs.OpenFile(file);
	bRet = Load(fs);
	fs.Close();
 
	return bRet;
}


bool RMLocation::Load(const byte *buf) {
	RMDataStream ds;
	bool bRet;

	ds.OpenBuffer(buf);
	bRet = Load(ds);
	ds.Close();
	return bRet;
}



/****************************************************************************\
*
* Function:     bool RMLocation::Load(byte *buf);
*
* Description:  Carica una locazione (.LOC) parsando il file gia' caricato
*               in memoria.
*
* Input:        byte *buf              Buffer con il file caricato
*
* Return:       true se ok, false in caso di errore
*
\****************************************************************************/

bool RMLocation::Load(RMDataStream &ds) {
	char id[3];
	int dimx, dimy;
	byte ver;
	byte cm;
	int i;

	// Controlla l'ID
	ds >> id[0] >> id[1] >> id[2];
	
	// Controlla se siamo in un LOX
	if (id[0] == 'L' && id[1] == 'O' && id[2] == 'X')
		return LoadLOX(ds);
	
	// Altrimenti, controlla che sia un LOC normale	
	if (id[0] != 'L' || id[1] != 'O' || id[2] != 'C')
	  return false;

	// Versione
	ds >> ver;
	assert(ver == 6);

	// Nome della locazione
	ds >> m_name;

	// Skippa i salvataggi MPAL (64 bytes)
	ds >> TEMPNumLoc;
	ds >> TEMPTonyStart.x >> TEMPTonyStart.y;
	ds += 64 - 4 * 3;

	// Skippa il flag di background associato (?!)
	ds += 1;

	// Dimensioni della locazione
	ds >> dimx >> dimy;
	m_curScroll.Set(0, 0);

	// Legge il color mode
	ds >> cm; m_cmode = (RMColorMode)cm;

	// Inizializza il source buffer e leggi la locazione dentro
	switch (m_cmode)	 {
	case CM_256:
		m_buf = new RMGfxSourceBuffer8;
		break;

	case CM_65K:
		m_buf = new RMGfxSourceBuffer16;
		break;
	
	default:
		assert(0);
		break;
	};

	// Inizializza la surface, caricando anche la palette se necessario
	m_buf->Init(ds, dimx, dimy, true);
 
	// Controlla le dimensioni della locazione
//	assert(dimy!=512);

	// Numero oggetti
	ds >> m_nItems;

	// Creazione e lettura degli oggetti
	if (m_nItems > 0)
		m_items = new RMItem[m_nItems];


	_vm->FreezeTime();
	for (i = 0;i < m_nItems && !ds.IsError(); i++)
		ds >> m_items[i];
	_vm->UnfreezeTime();

	// Setta i pattern iniziali @@@ doppione!!
	//for (i = 0;i<m_nItems;i++)
	//	m_items[i].SetPattern(mpalQueryItemPattern(m_items[i].MpalCode()));

	return ds.IsError();
}


bool RMLocation::LoadLOX(RMDataStream &ds) {
	int dimx, dimy;
	byte ver;
	int i;

	// Versione
	ds >> ver;
	assert(ver == 1);

	// Nome locazione
	ds >> m_name;

	// Numero loc
	ds >> TEMPNumLoc;
	ds >> TEMPTonyStart.x >> TEMPTonyStart.y;

	// Dimensioni
	ds >> dimx >> dimy;
	m_curScroll.Set(0, 0);

	// Color mode ่ sempre 65K
	m_cmode = CM_65K;
	m_buf = new RMGfxSourceBuffer16;

	// Inizializza la surface, caricando anche la palette se necessario
	m_buf->Init(ds, dimx, dimy, true);
 
	// Controlla le dimensioni della locazione
//	assert(dimy!=512);

	// Numero oggetti
	ds >> m_nItems;

	// Creazione e lettura degli oggetti
	if (m_nItems > 0)
		m_items = new RMItem[m_nItems];
	
	for (i = 0; i < m_nItems && !ds.IsError(); i++)
		m_items[i].ReadFromStream(ds, true);

	return ds.IsError();
}



/****************************************************************************\
*
* Function:     void RMLocation::Draw(RMGfxTargetBuffer* bigBuf, 
*									RMGfxPrimitive* prim);
*
* Description:  Metodo di drawing in overloading da RMGfxSourceBuffer8
*
\****************************************************************************/

void RMLocation::Draw(RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	// Setta la posizione sorgente dello scrolling
	if (m_buf->Dimy()>RM_SY || m_buf->Dimx()>RM_SX) {
		prim->SetSrc(RMRect(m_curScroll,m_curScroll+RMPoint(640,480)));
	}

	prim->SetDst(m_fixedScroll);
	
	// Richiama il metodo di drawing della classe dell'immagine, che disegnerเ il background
	// della locazione
	m_buf->Draw(bigBuf, prim);
}


/****************************************************************************\
*
* Function:     void RMLocation::DoFrame(void);
*
* Description:  Prepara un frame, aggiungendo alla OTList la locazione stessa
*								e tutti gli item che hanno cambiato frame di animazione
*
\****************************************************************************/

void RMLocation::DoFrame(RMGfxTargetBuffer *bigBuf) {
	int i;

	// Se la locazione non e' in OT list, la aggiunge
	if (!m_nInList)
		bigBuf->AddPrim(new RMGfxPrimitive(this));

	// Processa tutti gli item della locazione
	for (i = 0;i < m_nItems; i++)
		m_items[i].DoFrame(bigBuf);
}


RMItem *RMLocation::GetItemFromCode(uint32 dwCode) {
	int i;
	
	for (i = 0; i < m_nItems; i++)
		if (m_items[i].MpalCode() == (int)dwCode)
			return &m_items[i];
	
	return NULL;	
}

RMItem *RMLocation::WhichItemIsIn(RMPoint pt) {
	int found = -1;
	int foundSize = 0;
	int size;
		
	for (int i = 0; i < m_nItems; i++) 	{
		size = 0;
		if (m_items[i].IsIn(pt, &size)) {
			if (found == -1 || size < foundSize) {
				foundSize = size;
				found = i;
			} 
		}
	}

	if (found == -1)
		return NULL;
	else
		return &m_items[found];
}


RMLocation::~RMLocation() {
	Unload();
}

void RMLocation::Unload(void) {
	// Cancella la memoria
	if (m_items) {
		delete[] m_items;
		m_items = NULL;
	}

	// Cancella il buffer
	if (m_buf) {
		delete m_buf;
		m_buf = NULL;
	}
}

void RMLocation::UpdateScrolling(RMPoint ptShowThis) {
	RMPoint oldScroll = m_curScroll;

	if (m_curScroll.x + 250 > ptShowThis.x) {
		m_curScroll.x = ptShowThis.x - 250;
	} else if (m_curScroll.x + RM_SX - 250 < ptShowThis.x) {
		m_curScroll.x = ptShowThis.x + 250 - RM_SX;
	} else if (ABS(m_curScroll.x + RM_SX / 2 - ptShowThis.x) > 32 && m_buf->Dimx() > RM_SX) {
		if (m_curScroll.x + RM_SX / 2 < ptShowThis.x)
			m_curScroll.x++;
		else
			m_curScroll.x--;
	}

	if (m_curScroll.y + 180 > ptShowThis.y) {
		m_curScroll.y = ptShowThis.y - 180;
	} else if (m_curScroll.y + RM_SY - 180 < ptShowThis.y) {
		m_curScroll.y = ptShowThis.y + 180 - RM_SY;
	} else if (ABS(m_curScroll.y + RM_SY / 2 - ptShowThis.y) > 16 && m_buf->Dimy() > RM_SY) {
		if (m_curScroll.y + RM_SY / 2 < ptShowThis.y)
			m_curScroll.y++;
		else
			m_curScroll.y--;
	}

	if (m_curScroll.x < 0) m_curScroll.x = 0;
	if (m_curScroll.y < 0) m_curScroll.y = 0;
	if (m_curScroll.x + RM_SX > m_buf->Dimx()) m_curScroll.x = m_buf->Dimx() - RM_SX;
	if (m_curScroll.y + RM_SY > m_buf->Dimy()) m_curScroll.y = m_buf->Dimy() - RM_SY;

	if (oldScroll != m_curScroll)
		for (int i = 0; i < m_nItems; i++)
			m_items[i].SetScrollPosition(m_curScroll);
}

void RMLocation::SetFixedScroll(const RMPoint &scroll) {
	m_fixedScroll = scroll;

	for (int i = 0; i < m_nItems; i++)
		m_items[i].SetScrollPosition(m_curScroll - m_fixedScroll);
}

void RMLocation::SetScrollPosition(const RMPoint &scroll) {
	RMPoint pt = scroll;
	if (pt.x < 0) pt.x = 0;
	if (pt.y < 0) pt.y = 0;
	if (pt.x + RM_SX>m_buf->Dimx()) pt.x = m_buf->Dimx() - RM_SX;
	if (pt.y + RM_SY>m_buf->Dimy()) pt.y = m_buf->Dimy() - RM_SY;

	m_curScroll = pt;

	for (int i = 0; i < m_nItems; i++)
		m_items[i].SetScrollPosition(m_curScroll);
}


void RMLocation::PauseSound(bool bPause) {
	int i;

	for (i = 0; i < m_nItems; i++)
		m_items[i].PauseSound(bPause);
}


/****************************************************************************\
*       Metodi di RMMessage
\****************************************************************************/

RMMessage::RMMessage(uint32 dwId) {	
	lpMessage=mpalQueryMessage(dwId);
	assert(lpMessage != NULL);
	
	if (lpMessage)
		ParseMessage();
}

RMMessage::~RMMessage() {
	if (lpMessage)
		GlobalFree(lpMessage);
}

void RMMessage::ParseMessage(void) {
	char *p;
	
	assert(lpMessage != NULL);
	
	nPeriods = 1;
	p = lpPeriods[0] = lpMessage;
	
	for (;;) {
		// Trova la fine del periodo corrente
		while (*p != '\0')
			p++;

		// Se c'e' un altro '\0' siamo alla fine del messaggio
		p++;
		if (*p == '\0')
			break;

		// Altrimenti c'e' un altro periodo, e ci ricordiamo il suo inizio
		lpPeriods[nPeriods++] = p;
	}
}

} // End of namespace Tony
