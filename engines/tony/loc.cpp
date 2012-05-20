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

#include "common/scummsys.h"
#include "tony/mpal/mpalutils.h"
#include "tony/adv.h"
#include "tony/loc.h"
#include "tony/tony.h"

namespace Tony {

using namespace ::Tony::MPAL;


/****************************************************************************\
*       RMPalette Methods
\****************************************************************************/

/**
 * Operator for reading palette information from a data stream.
 *
 * @param ds				Data stream
 * @param pal				Destination palette 
 *
 * @returns		Reference to the data stream
 */
RMDataStream &operator>>(RMDataStream &ds, RMPalette &pal) {
	ds.Read(pal.m_data,1024);
	return ds;
}

/****************************************************************************\
*       RMSlot Methods
\****************************************************************************/

/**
 * Operator for reading slot information from a data stream.
 *
 * @param ds				Data stream
 * @param slot				Destination slot
 *
 * @returns		Reference to the data stream
 */
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

/**
 * Operator for reading pattern information from a data stream
 *
 * @param ds				Data stream
 * @param pat				Destination pattern
 *
 * @returns		Reference to the data stream
 */
RMDataStream &operator>>(RMDataStream &ds, RMPattern &pat) {
	pat.ReadFromStream(ds);
	return ds;
}

void RMPattern::ReadFromStream(RMDataStream &ds, bool bLOX) {
	int i;

	// Pattern name
	if (!bLOX)
		ds >> m_name;

	// Velocity
	ds >> m_speed;

	// Position
	ds >> m_pos;

	// Flag for pattern looping
	ds >> m_bLoop;

	// Number of slots
	ds >> m_nSlots;

	// Create and read the slots
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
			else if (GLOBALS.bSkipSfxNoLoop)
				sfx[m_slots[i].m_data].Stop();
		}
	}
}

int RMPattern::Init(RMSfx *sfx, bool bPlayP0, byte *bFlag) {
	int i;

	// Read the current time
	m_nStartTime = _vm->GetTime();
	m_nCurSlot = 0;

	// Find the first frame of the pattern
	i = 0;
	while (m_slots[i].m_type != SPRITE) {
		assert(i + 1 < m_nSlots);
		i++;
	}

	m_nCurSlot = i;
	m_nCurSprite = m_slots[i].m_data;
	if (bFlag)
		*bFlag = m_slots[i].m_flag;
	
	// Calculate the current coordinates
	UpdateCoord();
	
	// Check for sound:
	//  If the slot is 0, play
	//  If speed = 0, must playing unless it goes into loop '_', or if specified by the parameter
	//  If speed != 0, play only the loop
	for (i = 0;i < m_nSlots; i++) {
		if (m_slots[i].m_type == SOUND) {
			if (i == 0) {
				if (sfx[m_slots[i].m_data].m_name[0] == '_') {
	  			sfx[m_slots[i].m_data].SetVolume(m_slots[i].Pos().x);
					sfx[m_slots[i].m_data].Play(true);
				} else {
	  				sfx[m_slots[i].m_data].SetVolume(m_slots[i].Pos().x);
					sfx[m_slots[i].m_data].Play();
				}
			} else if (m_speed == 0) {
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

int RMPattern::Update(uint32 hEndPattern, byte &bFlag, RMSfx *sfx) {
	int CurTime = _vm->GetTime();

	// If the speed is 0, then the pattern never advances
	if (m_speed == 0) {
		CoroScheduler.pulseEvent(hEndPattern);
		bFlag=m_slots[m_nCurSlot].m_flag;
		return m_nCurSprite;
	}

	// Is it time to change the slots?
	while (m_nStartTime + m_speed <= (uint32)CurTime) {
		m_nStartTime += m_speed; 
		if (m_slots[m_nCurSlot].m_type == SPRITE)
			m_nCurSlot++;
		if (m_nCurSlot == m_nSlots) {
			m_nCurSlot = 0;
			bFlag = m_slots[m_nCurSlot].m_flag;

			CoroScheduler.pulseEvent(hEndPattern);

			// @@@ If there is no loop pattern, and there's a warning that it's the final
			// frame, then remain on the last frame
			if (!m_bLoop) {
				m_nCurSlot = m_nSlots - 1;
				bFlag = m_slots[m_nCurSlot].m_flag;
				return m_nCurSprite;			
			}
		}

		for (;;) {
			switch (m_slots[m_nCurSlot].m_type) {
			case SPRITE:
				// Read the next sprite
				m_nCurSprite = m_slots[m_nCurSlot].m_data;
			
				// Update the parent & child coordinates
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

	// Return the current sprite
	bFlag=m_slots[m_nCurSlot].m_flag;
	return m_nCurSprite;
}

RMPattern::RMPattern() {
	m_slots = NULL;
	m_speed = 0;
	m_bLoop  = 0;
    m_nSlots = 0;
	m_nCurSlot = 0;
	m_nCurSprite = 0;
	m_nStartTime = 0;
    m_slots = NULL;
}

RMPattern::~RMPattern() {
	if (m_slots != NULL) {
		delete[] m_slots;
		m_slots = NULL;
	}
}




/****************************************************************************\
*       RMSprite Methods
\****************************************************************************/

/**
 * Operator for reading sprite information from a data stream.
 *
 * @param ds				Data stream
 * @param sprite			Destination slot
 *
 * @returns		Reference to the data stream
 */
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
	
	// Sprite name
	if (!bLOX)
		ds >> m_name;

	// Dimensions
	ds >> dimx >> dimy;

	// Bounding box
	ds >> m_rcBox;

	// Unused space
	if (!bLOX)
		ds += 32;

	// Create buffer and read
	m_buf->Init(ds, dimx, dimy);
}

void RMSprite::Draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	m_buf->Draw(coroParam, bigBuf, prim);
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
*       RMSfx Methods
\****************************************************************************/

/**
 * Operator for reading SFX information from a data stream.
 *
 * @param ds				Data stream
 * @param sfx				Destination SFX
 *
 * @returns		Reference to the data stream
 */
RMDataStream &operator>>(RMDataStream &ds, RMSfx &sfx) {
	sfx.ReadFromStream(ds);
	return ds;
}

void RMSfx::ReadFromStream(RMDataStream &ds, bool bLOX) {
	char id[4];
	int size;
	byte *raw;
 
	// sfx name
	ds >> m_name;
 
	ds >> size;

	// Upload the sound effect identifier from the buffer
	ds.Read(id, 4);

	// Ensure it's a RIFF
	assert(id[0] == 'R' && id[1] == 'I' && id[2] == 'F' && id[3] == 'F');

	// Read the size
	ds >> size;

	// Read the raw WAV data
	raw = new byte[size]; 
	ds.Read(raw, size);

	// Create the sound effect
	m_fx = _vm->CreateSFX(raw);
	m_fx->SetLoop(false);

	// Close the read buffer which is no longer needed
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
			m_bPlayingLoop = true;
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
*       RMItem Methods
\****************************************************************************/

/**
 * Operator for reading item information from a data stream.
 *
 * @param ds				Data stream
 * @param tem				Destination item
 *
 * @returns		Reference to the data stream
 */
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

bool RMItem::IsIn(const RMPoint &pt, int *size)  { 
	RMRect rc;
	
	if (!m_bIsActive) 
		return false; 
	
	// Search for the right bounding box to use - use the sprite's if it has one, otherwise use the generic one
	if (m_nCurPattern != 0 && !m_sprites[m_nCurSprite].m_rcBox.IsEmpty())
		rc=m_sprites[m_nCurSprite].m_rcBox + CalculatePos();
	else if (!m_rcBox.IsEmpty())
		rc = m_rcBox;
	// If no box, return immediately
	else
		return false;
	
	if (size != NULL) 
		*size = rc.Size(); 
		
	return rc.PtInRect(pt + m_curScroll); 
}


void RMItem::ReadFromStream(RMDataStream &ds, bool bLOX) {
	int i, dimx, dimy;
	byte cm;

	// MPAL code
	ds >> m_mpalCode;

	// Object name
	ds >> m_name;

	// Z (signed)
	ds >> m_z;

	// Parent position
	ds >> m_pos;

	// Hotspot
	ds >> m_hot;

	// Bounding box
	ds >> m_rcBox;

	// Number of sprites, sound effects, and patterns
	ds >> m_nSprites >> m_nSfx >> m_nPatterns;

	// Color mode
	ds >> cm; m_cm=(RMColorMode)cm;

	// Flag for the presence of custom palette differences
	ds >> m_bPal;

	if (m_cm == CM_256) {
		//  If there is a palette, read it in
		if (m_bPal)
			ds >> m_pal;
	}

	// MPAL data
	if (!bLOX)
		ds += 20;
 
	ds >> m_FX;
	ds >> m_FXparm;

	if (!bLOX)
	ds += 106;
 
	// Create sub-classes
	if (m_nSprites > 0)
		 m_sprites = new RMSprite[m_nSprites];
	if (m_nSfx > 0)
		m_sfx = new RMSfx[m_nSfx];
	m_patterns = new RMPattern[m_nPatterns+1];

	// Read in class data
	if (!ds.IsError())
		for (i = 0; i < m_nSprites && !ds.IsError(); i++) {
		 // Download the sprites
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

	// Read the pattern from pattern 1
	if (!ds.IsError())
		for (i = 1;i <= m_nPatterns && !ds.IsError(); i++) {
			if (bLOX)
				m_patterns[i].ReadFromStream(ds, true);
			else
				m_patterns[i].ReadFromStream(ds, false);
		}

	// Initialise the current pattern
	if (m_bInitCurPattern)
		SetPattern(mpalQueryItemPattern(m_mpalCode));

	// Initailise the current activation state
	m_bIsActive = mpalQueryItemIsActive(m_mpalCode);
}


RMGfxPrimitive *RMItem::NewItemPrimitive() {
	return new RMGfxPrimitive(this);
}

void RMItem::SetScrollPosition(const RMPoint &scroll) {
	m_curScroll = scroll;
}

bool RMItem::DoFrame(RMGfxTargetBuffer *bigBuf, bool bAddToList) {
	int oldSprite = m_nCurSprite;

	// Pattern 0 = Do not draw anything!
	if (m_nCurPattern == 0)
		return false;

	// We do an update of the pattern, which also returns the current frame
	if (m_nCurPattern != 0) {
		m_nCurSprite = m_patterns[m_nCurPattern].Update(m_hEndPattern, m_bCurFlag, m_sfx);

		// WORKAROUND: Currently, m_nCurSprite = -1 is used to flag that an item should be removed.
		// However, this seems to be done inside a process waiting on an event pulsed inside the pattern
		// Update method. So the value of m_nCurSprite = -1 is being destroyed with the return value
		// replacing it. It may be that the current coroutine PulseEvent implementation is wrong somehow.
		// In any case, a special check here is done for items that have ended
		if (m_nCurPattern == 0)
			m_nCurSprite = -1;
	}

	// If the function returned -1, it means that the pattern has finished
	if (m_nCurSprite == -1) {
		// We have pattern 0, so leave. The class will self de-register from the OT list
		m_nCurPattern = 0;
		return false;
	}

	// If we are not in the OT list, add ourselves
	if (!m_nInList && bAddToList)
		bigBuf->AddPrim(NewItemPrimitive());

	return oldSprite != m_nCurSprite;
}

RMPoint RMItem::CalculatePos(void) {
	return m_pos + m_patterns[m_nCurPattern].Pos();
}

void RMItem::Draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	// If CurSprite == -1, then the pattern is finished
	if (m_nCurSprite == -1)
	  return;
	
	// Set the flag
	prim->SetFlag(m_bCurFlag);

	// Offset direction for scrolling
	prim->Dst().Offset(-m_curScroll);

	// We must offset the cordinates of the item inside the primitive
	// It is estimated as nonno + (babbo + figlio)
	prim->Dst().Offset(CalculatePos());

	// No stretching, please
	prim->SetStrecth(false);

	// Now we turn to the generic surface drawing routines
	CORO_INVOKE_2(m_sprites[m_nCurSprite].Draw, bigBuf, prim);

	CORO_END_CODE;
}


void RMItem::RemoveThis(CORO_PARAM, bool &result) {
	// Remove from the OT list if the current frame is -1 (pattern over)
	result = (m_nCurSprite == -1);
}


void RMItem::SetStatus(int nStatus) {
	m_bIsActive = (nStatus > 0);
}

void RMItem::SetPattern(int nPattern, bool bPlayP0) {
	int i;

	assert(nPattern >= 0 && nPattern <= m_nPatterns);

	if (m_sfx)
		if (m_nCurPattern>0)
			m_patterns[m_nCurPattern].StopSfx(m_sfx);
	
	// Remember the current pattern
	m_nCurPattern = nPattern;

	// Start the pattern to start the animation
	if (m_nCurPattern != 0)
		m_nCurSprite = m_patterns[m_nCurPattern].Init(m_sfx, bPlayP0, &m_bCurFlag);
	else {
		m_nCurSprite = -1;
		
		// Look for the sound effect for pattern 0
		if (bPlayP0)
			for (i = 0;i < m_nSfx; i++)
				if (strcmp(m_sfx[i].m_name, "p0") == 0)
					m_sfx[i].Play();
	}
}


bool RMItem::GetName(RMString &name) {
	char buf[256];

	mpalQueryItemName(m_mpalCode, buf);
	name = buf;
	if (buf[0] == '\0')
		return false;
	return true; 
}


void RMItem::Unload(void) {
	if (m_patterns != NULL) {
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
	m_sfx = NULL;
	m_curScroll.Set(0, 0);
	m_bInitCurPattern = true;
	m_nCurPattern = 0;
	m_z = 0;
	m_cm = CM_256;
	m_FX = 0;
	m_FXparm = 0;
	m_mpalCode = 0;
	m_nSprites = 0;
	m_nSfx = 0;
	m_nPatterns = 0;
	m_bPal = 0;
	m_nCurSprite = 0;

	m_hEndPattern = CoroScheduler.createEvent(false, false);
}

RMItem::~RMItem() {
	Unload();
	CoroScheduler.closeEvent(m_hEndPattern);
}


void RMItem::WaitForEndPattern(CORO_PARAM, uint32 hCustomSkip) {
	CORO_BEGIN_CONTEXT;
		uint32 h[2];
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (m_nCurPattern != 0) {
		if (hCustomSkip == CORO_INVALID_PID_VALUE)
			CORO_INVOKE_2(CoroScheduler.waitForSingleObject, m_hEndPattern, CORO_INFINITE);
		else {
			_ctx->h[0] = hCustomSkip;
			_ctx->h[1] = m_hEndPattern;
			CORO_INVOKE_4(CoroScheduler.waitForMultipleObjects, 2, &_ctx->h[0], false, CORO_INFINITE);
		}
	}

	CORO_END_CODE;
}

void RMItem::ChangeHotspot(const RMPoint &pt) {
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
*       RMWipe Methods
\****************************************************************************/


RMWipe::RMWipe() {
	m_hUnregistered = CoroScheduler.createEvent(false, false);
	m_hEndOfFade = CoroScheduler.createEvent(false, false);
}

RMWipe::~RMWipe() {
	CoroScheduler.closeEvent(m_hUnregistered);
	CoroScheduler.closeEvent(m_hEndOfFade);
}

int RMWipe::Priority(void) {
	return 200;
}

void RMWipe::Unregister(void) {
	RMGfxTask::Unregister();
	assert(m_nInList == 0);
	CoroScheduler.setEvent(m_hUnregistered);
}

void RMWipe::RemoveThis(CORO_PARAM, bool &result) {
	result = m_bUnregister;
}

void RMWipe::WaitForFadeEnd(CORO_PARAM) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_2(CoroScheduler.waitForSingleObject, m_hEndOfFade, CORO_INFINITE);	

	m_bEndFade = true;
	m_bFading = false;

	CORO_INVOKE_0(MainWaitFrame);
	CORO_INVOKE_0(MainWaitFrame);

	CORO_END_CODE;
}

void RMWipe::CloseFade(void) {
	m_wip0r.Unload();
}

void RMWipe::InitFade(int type) {
	// Activate the fade
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
	
	if (m_bFading) {
		m_wip0r.DoFrame(&bigBuf, false);

		m_nFadeStep++;
	
		if (m_nFadeStep == 10) {
			CoroScheduler.setEvent(m_hEndOfFade);
		}
	}
}

void RMWipe::Draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (m_bFading) {
		CORO_INVOKE_2(m_wip0r.Draw, bigBuf, prim);
	}

	if (m_bEndFade)
		Common::fill((byte *)bigBuf, (byte *)bigBuf + bigBuf.Dimx() * bigBuf.Dimy() * 2, 0x0);

	CORO_END_CODE;
}



/****************************************************************************\
*       RMCharacter Methods
\****************************************************************************/

/****************************************************************************/
/* Find the shortest path between two nodes of the graph connecting the BOX */
/* Returns path along the vector path path[]                                */
/****************************************************************************/

short RMCharacter::FindPath(short source, short destination) {
	static RMBox BOX[MAXBOXES];			// Matrix of adjacent boxes
	static short COSTO[MAXBOXES];       // Cost per node
	static short VALIDO[MAXBOXES];      // 0:Invalid 1:Valid 2:Saturated
	static short NEXT[MAXBOXES];        // Prossimo Nodo
	short i, j, k, costominimo, fine, errore = 0;
	RMBoxLoc *cur;

	g_system->lockMutex(csMove);  

	if (source == -1 || destination == -1) {
		g_system->unlockMutex(csMove); 
		return 0;
	}

	// Get the boxes
	cur = theBoxes->GetBoxes(curLocation);

	// Make a backup copy to work on
	for (i = 0; i < cur->numbbox; i++)
		memcpy(&BOX[i], &cur->boxes[i], sizeof(RMBox));

	// Invalidate all nodes
	for (i = 0; i < cur->numbbox; i++) 
		VALIDO[i] = 0;
	
	// Prepare source and variables for the procedure
	COSTO[source] = 0;
	VALIDO[source] = 1;
	fine = 0;
 
 	 // Find the shortest path
	while(!fine) {
		costominimo = 32000;				// Reset the minimum cost
		errore = 1;							// Possible error

		// 1st cycle: explore possible new nodes
		for (i = 0; i < cur->numbbox; i++)
			if (VALIDO[i] == 1) {
				errore = 0;					// Failure de-bunked
				j = 0;
				while (((BOX[i].adj[j]) != 1) && (j < cur->numbbox)) 
					j++;
      
				if (j >= cur->numbbox) 
					VALIDO[i] = 2;                     // nodo saturated?
				else {
					NEXT[i] = j;
					if (COSTO[i] + 1 < costominimo) 
						costominimo = COSTO[i] + 1;
				}
			}

		if (errore) 
			fine = 1;                                 // All nodes saturated

		// 2nd cycle: adding new nodes that were found, saturate old nodes
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

	// Remove the path from the adjacent modified matrixes
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


void RMCharacter::GoTo(CORO_PARAM, RMPoint destcoord, bool bReversed) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (m_pos == destcoord) {
		if (minpath == 0) {
			CORO_INVOKE_0(Stop);
			CoroScheduler.pulseEvent(hEndOfPath);
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
    
		// Change the pattern for the new direction
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
    
		bNeedToStop = true;
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

	CORO_END_CODE;
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


RMPoint RMCharacter::NearestPoint(const RMPoint &punto) {
	return Searching(1, 1, 1, 1, punto);
}


short RMCharacter::ScanLine(const RMPoint &punto) {
	int Ldx, Ldy, Lcount;
	float Lfx, Lfy, Lslope;
	RMPoint Lstart, Lend, Lscan;
	signed char Lspeed, Lstatus;

	Lstart = m_pos;
	Lend = punto;
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
		Lstatus = 1;
	} else {
		Lslope = Lfx / Lfy;
		if (Lend.y < Lstart.y) Lspeed =- Lspeed;
		Lstatus = 0;
	}

	Lscan = Lstart;   // Start scanning
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

/**
 * Calculates intersections between the straight line and the closest BBOX
 */
RMPoint RMCharacter::InvScanLine(const RMPoint &punto) {
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


/**
 * Returns the HotSpot coordinate closest to the player
 */

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

void RMCharacter::Draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (bDrawNow) {
		prim->Dst() += m_fixedScroll;

		CORO_INVOKE_2(RMItem::Draw, bigBuf, prim);
	}

	CORO_END_CODE;
}

void RMCharacter::NewBoxEntered(int nBox) {
	RMBoxLoc *cur;
	bool bOldReverse;
	
	// Recall on ExitBox
	mpalQueryDoAction(3, curLocation, curbox);

	cur = theBoxes->GetBoxes(curLocation);
	bOldReverse = cur->boxes[curbox].bReversed;
	curbox = nBox;

	// If Z is changed, we must remove it from the OT
	if (cur->boxes[curbox].Zvalue != m_z) {
		bRemoveFromOT = true;
		m_z = cur->boxes[curbox].Zvalue;
	}

	// Movement management is reversed, only if we are not in the shortest path. If we are in the shortest
	// path, directly do the DoFrame
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

	// Recall On EnterBox
	mpalQueryDoAction(2, curLocation, curbox);
}
	
void RMCharacter::DoFrame(CORO_PARAM, RMGfxTargetBuffer* bigBuf, int loc) {
	CORO_BEGIN_CONTEXT;
		bool bEndNow;
		RMBoxLoc *cur;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_ctx->bEndNow = false;
	bEndOfPath = false;
	bDrawNow = (curLocation == loc);

	g_system->lockMutex(csMove);

	// If we're walking..
	if (status != STAND) {
		// If we are going horizontally
		if (walkstatus == 1) {
			dx = walkspeed * walkcount;
			dy = slope * dx;
			m_pos.x = linestart.x + dx;
			m_pos.y = linestart.y + dy;

			// Right
			if (((walkspeed > 0) && (m_pos.x > lineend.x)) || ((walkspeed < 0) && (m_pos.x < lineend.x))) {
				m_pos = lineend;
				status = STAND;
				_ctx->bEndNow = true;
			}
		}
    
		// If we are going vertical
		if (walkstatus == 0) {
			dy = walkspeed * walkcount;
			dx = slope * dy;
			m_pos.x = linestart.x + dx;
			m_pos.y = linestart.y + dy;

			// Down
			if (((walkspeed > 0) && (m_pos.y > lineend.y)) || ((walkspeed < 0) && (m_pos.y < lineend.y))) {
				m_pos = lineend;
				status = STAND;
				_ctx->bEndNow = true;
			}
		}

		// Check if the character came out of the BOX in error, in which case he returns immediately
		if (InWhichBox(m_pos) == -1) {
			m_pos.x = linestart.x + olddx;
			m_pos.y = linestart.y + olddy;
		}

		// If we have just moved to a temporary location, and is over the shortest path, we stop permanently
		if (_ctx->bEndNow && minpath == 0) {
			if (!bEndOfPath)
				CORO_INVOKE_0(Stop);
			bEndOfPath = true;
			CoroScheduler.pulseEvent(hEndOfPath);
		}
		
		walkcount++;

		// Update the character Z. @@@ Should remove only if the Z was changed
		
		// Check if the box was changed
		if (!theBoxes->IsInBox(curLocation, curbox, m_pos))
			NewBoxEntered(InWhichBox(m_pos));

		// Update the old coordinates
		olddx = dx;
		olddy = dy;
	}

	// If we stop
	if (status == STAND) {
		// Check if there is still the shortest path to calculate
		if (minpath == 1) {
			_ctx->cur = theBoxes->GetBoxes(curLocation);

			// If we still have to go through a box
			if (pathcount < pathlenght) {
				// Check if the box we're going into is active
				if (_ctx->cur->boxes[path[pathcount-1]].attivo) {
					// Move in a straight line towards the nearest hotspot, taking into account the reversing
					// NEWBOX = path[pathcount-1]
					CORO_INVOKE_2(GoTo, NearestHotSpot(path[pathcount-1], path[pathcount]), _ctx->cur->boxes[path[pathcount-1]].bReversed);
					pathcount++;
				} else {
					// If the box is off, we can only block all
					// @@@ Whilst this should not happen, because have improved
					// the search for the minimum path
					minpath = 0;
					if (!bEndOfPath)
						CORO_INVOKE_0(Stop);
					bEndOfPath = true;
					CoroScheduler.pulseEvent(hEndOfPath);
				}
			} else {
				// If we have already entered the last box, we just have to move in a straight line towards the 
				// point of arrival
				// NEWBOX = InWhichBox(pathend)
				minpath = 0;
				CORO_INVOKE_2(GoTo, pathend, _ctx->cur->boxes[InWhichBox(pathend)].bReversed);
			}
		}
	}

	g_system->unlockMutex(csMove);

	// Invoke the DoFrame of the item
	RMItem::DoFrame(bigBuf);

	CORO_END_CODE;
}

void RMCharacter::Stop(CORO_PARAM) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	bMoving = false;

	// You never know..
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
		SetPattern(PAT_STANDDOWN);
		break;
	}

	CORO_END_CODE;
}

inline int RMCharacter::InWhichBox(const RMPoint &pt) { 
	return theBoxes->WhichBox(curLocation, pt); 
}


void RMCharacter::Move(CORO_PARAM, RMPoint pt, bool *result) {
	CORO_BEGIN_CONTEXT;
		RMPoint dest;
		int numbox;
		RMBoxLoc *cur;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	bMoving = true;
	
	// 0, 0 does not do anything, just stops the character
	if (pt.x == 0 && pt.y == 0) {
		minpath = 0;
		status = STAND;
		CORO_INVOKE_0(Stop);
		if (result)
			*result = true;
		return;
	}

	// If clicked outside the box
 	_ctx->numbox = InWhichBox(pt);
	if (_ctx->numbox == -1) {
		// Find neareste point inside the box
		_ctx->dest = NearestPoint(pt);

		// ???!??
		if (_ctx->dest == pt)
			_ctx->dest = InvScanLine(pt);

		pt = _ctx->dest;
		_ctx->numbox = InWhichBox(pt);
	}

	_ctx->cur = theBoxes->GetBoxes(curLocation);

	minpath = 0;
	status = STAND;
	bMovingWithoutMinpath = true;
	if (ScanLine(pt)) 
		CORO_INVOKE_2(GoTo, pt, _ctx->cur->boxes[_ctx->numbox].bReversed);
	else if (FindPath(InWhichBox(m_pos), InWhichBox(pt))) {
		bMovingWithoutMinpath = false;
		minpath = 1;
		pathcount = 1;
		pathend = pt;
	} else {
		// @@@ This case is whether a hotspot is inside a box, but there is
		// a path to get there. We use the InvScanLine to search around a point
		_ctx->dest = InvScanLine(pt);
		pt = _ctx->dest;
		
		if (ScanLine(pt)) 
			CORO_INVOKE_2(GoTo, pt, _ctx->cur->boxes[_ctx->numbox].bReversed);
		else if (FindPath(InWhichBox(m_pos), InWhichBox(pt))) {
			bMovingWithoutMinpath = false;
			minpath = 1;
			pathcount = 1;
			pathend = pt;
			if (result)
				*result = true;
		} else {
			if (result)
				*result = false;
		}

		return;
	}

	if (result)
		*result = true;

	CORO_END_CODE;
}

void RMCharacter::SetPosition(const RMPoint &pt, int newloc) {
	RMBoxLoc *box;
	
	minpath = 0;
	status = STAND;
	m_pos = pt;
	
	if (newloc != -1)
		curLocation = newloc;

	// Update the character's Z value
	box = theBoxes->GetBoxes(curLocation);
	curbox = InWhichBox(m_pos);
	assert(curbox != -1);
	m_z = box->boxes[curbox].Zvalue;
	bRemoveFromOT = true;
}

void RMCharacter::WaitForEndMovement(CORO_PARAM) { 
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (bMoving) 
		CORO_INVOKE_2(CoroScheduler.waitForSingleObject, hEndOfPath, CORO_INFINITE); 

	CORO_END_CODE;
}

void RMCharacter::RemoveThis(CORO_PARAM, bool &result) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (bRemoveFromOT)
		result = true;
	else
		CORO_INVOKE_1(RMItem::RemoveThis, result);

	CORO_END_CODE;
}

RMCharacter::RMCharacter() {
	csMove = g_system->createMutex();
	hEndOfPath = CoroScheduler.createEvent(false, false);
	minpath = 0;
	curSpeed = 3;
	bRemoveFromOT = false;
	bMoving = false;
	curLocation = 0;
	curbox = 0;
	dx = dy = 0;
	olddx = olddy = 0;
	fx = fy = slope = 0;
	walkspeed = walkstatus = 0;
	nextbox = 0;
	pathlenght = pathcount = 0;
	status = STAND;
	theBoxes = NULL;
	walkcount = 0;
	bEndOfPath = false;
	bMovingWithoutMinpath = false;
	bDrawNow = false;
	bNeedToStop = false;

	m_pos.Set(0, 0);
}

RMCharacter::~RMCharacter() {
	g_system->deleteMutex(csMove);
	CoroScheduler.closeEvent(hEndOfPath);
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

	// Adjacency
	for (i = 0; i < MAXBOXES; i++) {
		ds >> adj[i];
	}

	// Misc
	ds >> numhotspot;
	ds >> Zvalue;
	ds >> b;
	attivo = b;
	ds >> b;
	bReversed = b;

	// Reversed expansion space
	ds += 30;

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

RMBoxLoc::RMBoxLoc() {
	boxes = NULL;
}

RMBoxLoc::~RMBoxLoc() {
	delete[] boxes;
}

void RMBoxLoc::ReadFromStream(RMDataStream &ds) {
	int i;
	char buf[2];
	byte ver;

	// ID and version
	ds >> buf[0] >> buf[1] >> ver;
	assert(buf[0] == 'B' && buf[1] == 'X');
	assert(ver == 3);

	// Number of boxes
	ds >> numbbox;

	// Allocate memory for the boxes
	boxes = new RMBox[numbbox];

	// Read in boxes
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

RMGameBoxes::RMGameBoxes() {
	m_nLocBoxes = 0;
	Common::fill(m_allBoxes, m_allBoxes + GAME_BOXES_SIZE, (RMBoxLoc *)NULL);
}

RMGameBoxes::~RMGameBoxes() {
	for (int i = 1; i <= m_nLocBoxes; ++i)
		delete m_allBoxes[i];
}

void RMGameBoxes::Init(void) {
	int i;
	RMString fn;
	RMDataStream ds;

	// Load boxes from disk
	m_nLocBoxes = 130;
	for (i = 1; i <= m_nLocBoxes; i++) {
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

bool RMGameBoxes::IsInBox(int nLoc, int nBox, const RMPoint &pt) {
	RMBoxLoc *cur = GetBoxes(nLoc);

	if ((pt.x >= cur->boxes[nBox].left) && (pt.x <= cur->boxes[nBox].right) &&
			(pt.y >= cur->boxes[nBox].top)  && (pt.y <= cur->boxes[nBox].bottom)) 
		return true;
	else
		return false;
}

int RMGameBoxes::WhichBox(int nLoc, const RMPoint &punto) {
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
		state += 4;
				
		for (j = 0; j < m_allBoxes[i]->numbbox; j++)
			*state++ = m_allBoxes[i]->boxes[j].attivo;
	}
}

void RMGameBoxes::LoadState(byte *state) {
	int i, j;
	int nloc, nbox;

	// Load number of items
	nloc = READ_LE_UINT32(state);
	state += 4;

	assert(nloc <= m_nLocBoxes);

	// For each location, read the number of boxes and their status
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
*       RMLocation Methods
\****************************************************************************/

/**
 * Standard constructor
 */
RMLocation::RMLocation() {
	m_nItems = 0;
	m_items = NULL;
	m_buf = NULL;
}


/**
 * Load a location (.LOC) from a file that is provided.
 *
 * @param lpszFileName			Name of the file
 */
bool RMLocation::Load(const char *lpszFileName) {
	Common::File f;
	bool bRet;

	// Open the file for reading
	if (!f.open(lpszFileName))
		return false;

	// Passes to the method variation for loading from the opened file
	bRet = Load(f);

	// Close the file
	f.close();

	return bRet;
}


/**
 * Load a location (.LOC) from a given open file
 *
 * @param hFile					File reference
 *
 * @returns		True if succeeded OK, false in case of error.
 */
bool RMLocation::Load(Common::File &file) {
	int size;
	bool bRet;

	// Calcola la lunghezza del file
	size = file.size();
	file.seek(0);

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


/**
 * Load a location (.LOC) from a given data stream
 *
 * @param ds						Data stream
 * @returns		True if succeeded OK, false in case of error.
 */
bool RMLocation::Load(RMDataStream &ds) {
	char id[3];
	int dimx, dimy;
	byte ver;
	byte cm;
	int i;

	// Check the ID
	ds >> id[0] >> id[1] >> id[2];
	
	// Check if we are in a LOX
	if (id[0] == 'L' && id[1] == 'O' && id[2] == 'X')
		return LoadLOX(ds);
	
	// Otherwise, check that it is a normal LOC
	if (id[0] != 'L' || id[1] != 'O' || id[2] != 'C')
	  return false;

	// Version
	ds >> ver;
	assert(ver == 6);

	// Location name
	ds >> m_name;

	// Skip the MPAL bailouts (64 bytes)
	ds >> TEMPNumLoc;
	ds >> TEMPTonyStart.x >> TEMPTonyStart.y;
	ds += 64 - 4 * 3;

	// Skip flag associated with the background (?)
	ds += 1;

	// Location dimensions
	ds >> dimx >> dimy;
	m_curScroll.Set(0, 0);

	// Read the colour mode
	ds >> cm; m_cmode = (RMColorMode)cm;

	// Initialise the source buffer and read the location
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

	// Initialise the surface, loading the palette if necessary
	m_buf->Init(ds, dimx, dimy, true);
 
	// Check the size of the location
//	assert(dimy!=512);

	// Number of objects
	ds >> m_nItems;

	// Create and read in the objects
	if (m_nItems > 0)
		m_items = new RMItem[m_nItems];


	_vm->FreezeTime();
	for (i = 0;i < m_nItems && !ds.IsError(); i++)
		ds >> m_items[i];
	_vm->UnfreezeTime();

	return ds.IsError();
}


bool RMLocation::LoadLOX(RMDataStream &ds) {
	int dimx, dimy;
	byte ver;
	int i;

	// Version
	ds >> ver;
	assert(ver == 1);

	// Location name
	ds >> m_name;

	// Location number
	ds >> TEMPNumLoc;
	ds >> TEMPTonyStart.x >> TEMPTonyStart.y;

	// Dimensions
	ds >> dimx >> dimy;
	m_curScroll.Set(0, 0);

	// It's always 65K (16-bit) mode
	m_cmode = CM_65K;
	m_buf = new RMGfxSourceBuffer16;

	// Initialise the surface, loading in the palette if necessary
	m_buf->Init(ds, dimx, dimy, true);
 
	// Number of items
	ds >> m_nItems;

	// Create and read objects
	if (m_nItems > 0)
		m_items = new RMItem[m_nItems];
	
	for (i = 0; i < m_nItems && !ds.IsError(); i++)
		m_items[i].ReadFromStream(ds, true);

	return ds.IsError();
}


/**
 * Draw method overloaded from RMGfxSourceBUffer8
 */
void RMLocation::Draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	// Set the position of the source scrolling
	if (m_buf->Dimy()>RM_SY || m_buf->Dimx()>RM_SX) {
		prim->SetSrc(RMRect(m_curScroll,m_curScroll+RMPoint(640,480)));
	}

	prim->SetDst(m_fixedScroll);
	
	// Invoke the drawing method fo the image class, which will draw the location background
	CORO_INVOKE_2(m_buf->Draw, bigBuf, prim);

	CORO_END_CODE;
}


/**
 * Prepare a frame, adding the location to the OT list, and all the items that have changed animation frame.
 */
void RMLocation::DoFrame(RMGfxTargetBuffer *bigBuf) {
	int i;

	// If the location is not in the OT list, add it in
	if (!m_nInList)
		bigBuf->AddPrim(new RMGfxPrimitive(this));

	// Process all the location items
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

RMItem *RMLocation::WhichItemIsIn(const RMPoint &pt) {
	int found = -1;
	int foundSize = 0;
	int size;
		
	for (int i = 0; i < m_nItems; i++) {
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
	// Clear memory
	if (m_items) {
		delete[] m_items;
		m_items = NULL;
	}

	// Destroy the buffer
	if (m_buf) {
		delete m_buf;
		m_buf = NULL;
	}
}

void RMLocation::UpdateScrolling(const RMPoint &ptShowThis) {
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
*       RMMessage Methods
\****************************************************************************/

RMMessage::RMMessage(uint32 dwId) {	
	Load(dwId);
}

RMMessage::RMMessage() {
	lpMessage = NULL;
}

RMMessage::~RMMessage() {
	if (lpMessage)
		GlobalFree(lpMessage);
}

void RMMessage::Load(uint32 dwId) {
	lpMessage = mpalQueryMessage(dwId);
	assert(lpMessage != NULL);
	
	if (lpMessage)
		ParseMessage();
}

void RMMessage::ParseMessage(void) {
	char *p;
	
	assert(lpMessage != NULL);
	
	nPeriods = 1;
	p = lpPeriods[0] = lpMessage;
	
	for (;;) {
		// Find the end of the current period
		while (*p != '\0')
			p++;

		// If there is another '0' at the end of the string, the end has been found
		p++;
		if (*p == '\0')
			break;

		// Otherwise there is another line, and remember it's start
		lpPeriods[nPeriods++] = p;
	}
}

} // End of namespace Tony
