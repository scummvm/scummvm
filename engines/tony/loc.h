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

#ifndef TONY_LOC_H
#define TONY_LOC_H

#include "common/scummsys.h"
#include "common/system.h"
#include "common/file.h"
#include "tony/mpal/stubs.h"
#include "tony/sound.h"
#include "tony/utils.h"

namespace Tony {

/****************************************************************************\
*       Various defines
\****************************************************************************/

/**
 * Valid colour modes
 */
typedef enum {
	CM_256,
	CM_65K
} RMColorMode;


/****************************************************************************\
*       Class declarations
\****************************************************************************/

/**
 * Generic palette
 */
class RMPalette {
public:
	byte m_data[1024];

public:
    friend RMDataStream &operator>>(RMDataStream &ds, RMPalette &pal);
};


/**
 * Sound effect of an object
 */
class RMSfx {
public:
	RMString m_name;
	FPSFX *m_fx;
	bool m_bPlayingLoop;

public:
	RMSfx();
	virtual ~RMSfx();

    friend RMDataStream& operator>>(RMDataStream &ds, RMSfx &sfx);

	void Play(bool bLoop = false);
	void SetVolume(int vol);
	void Pause(bool bPause);
	void Stop(void);

    void ReadFromStream(RMDataStream& ds, bool bLOX = false);
};


/**
 * Object pattern
 */
class RMPattern {
public:
	// Type of slot
	enum RMSlotType {
		DUMMY1 = 0,
		DUMMY2,
		SPRITE,
		SOUND,
		COMMAND,
		SPECIAL
	};

	// Class slot
	class RMSlot {
	private:
		RMPoint m_pos; // Child co-ordinates

	public:
	    RMSlotType m_type;
		int m_data;
		byte m_flag;

	public:
		friend RMDataStream& operator>>(RMDataStream& ds, RMSlot& slot);

		RMPoint Pos() { return m_pos; }

		void ReadFromStream(RMDataStream& ds, bool bLOX = false);
	};

public:
    RMString m_name;

private:
    int m_speed;
    RMPoint m_pos; // Coordinate babbo
	RMPoint m_curPos; // Coordinate babbo+figlio
    int m_bLoop;
    int m_nSlots;
	int m_nCurSlot;
	int m_nCurSprite;

    RMSlot *m_slots;

	uint32 m_nStartTime;

public:
	RMPattern();
	virtual ~RMPattern();
		
	friend RMDataStream& operator>>(RMDataStream& ds, RMPattern& pat);

	// A warning that the pattern now and the current
	int Init(RMSfx* sfx, bool bPlayP0=false, byte* bFlag=NULL);

	// Update the pattern, checking to see if it's time to change slot and executing 
	// any associated commands
	int Update(uint32 hEndPattern, byte &bFlag, RMSfx *sfx);

	// Stop a sound effect
	void StopSfx(RMSfx *sfx);

	// Reads the position of the pattern
	RMPoint Pos() { return m_curPos; }

    void ReadFromStream(RMDataStream& ds, bool bLOX = false);

private:
	void UpdateCoord(void);
};


/**
 * Sprite (frame) animation of an item
 */
class RMSprite : public RMGfxTask {
public:
    RMString m_name;
    RMRect m_rcBox;

protected:
	RMGfxSourceBuffer* m_buf;

public:
	RMSprite();
	virtual ~RMSprite();

	void Init(RMGfxSourceBuffer* buf);
	friend RMDataStream& operator>>(RMDataStream& ds, RMSprite& sprite);
	virtual void Draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim);
	void SetPalette(byte *lpBuf);
	void GetSizeFromStream(RMDataStream& ds, int* dimx, int* dimy);
	void LOXGetSizeFromStream(RMDataStream& ds, int* dimx, int* dimy);

    void ReadFromStream(RMDataStream& ds, bool bLOX = false);
};


/**
 * Data on an item
 */
class RMItem : public RMGfxTask {
public:
    RMString m_name;

protected:
    int m_z;
    RMPoint m_pos;  // Coordinate nonno
	RMColorMode m_cm;
	RMPoint m_curScroll;

	byte m_FX;
	byte m_FXparm;

	virtual int GetCurPattern() { return m_nCurPattern; }

private:
	int m_nCurPattern;
	int m_mpalCode;
    RMPoint m_hot;
	RMRect m_rcBox;
    int m_nSprites,m_nSfx,m_nPatterns;
    byte m_bPal;
    RMPalette m_pal;

    RMSprite *m_sprites;
    RMSfx *m_sfx;
    RMPattern *m_patterns;

	byte m_bCurFlag;
	int m_nCurSprite;
	bool m_bIsActive;
	uint32 m_hEndPattern;
	bool m_bInitCurPattern;

public:
	RMPoint CalculatePos(void);

public:
	RMItem();
	virtual ~RMItem();

    friend RMDataStream& operator>>(RMDataStream &ds, RMItem &item);

	// Processa l'oggetto per fare andare avanti eventuale animazioni. Ritorna TRUE se dovrเ
	// essere ridisegnato il prossimo frame
	bool DoFrame(RMGfxTargetBuffer *bigBuf, bool bAddToList = true);

	// Setta la posizione corrente di scrolling
	void SetScrollPosition(const RMPoint &scroll);

	// Overloading della funzione per la rimozione da ot list
	virtual void RemoveThis(CORO_PARAM, bool &result);
	
	// Overloading del draw
	virtual void Draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim);

	// Overloading della prioritเ: ่ la Z
	virtual int Priority() { return m_z; }

	// Numero di pattern
	int NumPattern() { return m_nPatterns; }

	// Setta un nuovo pattern di animazione, cambiando bruscamente da quello corrente
	virtual void SetPattern(int nPattern, bool bPlayP0 = false);

	// Setta un nuovo status.
	void SetStatus(int nStatus);

	bool IsIn(const RMPoint &pt, int *size = NULL);
	RMPoint Hotspot() { return m_hot; }
	bool GetName(RMString &name);
	int MpalCode() { return m_mpalCode; }

	// Scarica l'item
	void Unload(void);

	// Aspetta la fine del pattern in play
	void WaitForEndPattern(CORO_PARAM, uint32 hCustomSkip = CORO_INVALID_PID_VALUE);

	// Setta un nuovo hotspot per l'oggetto
	void ChangeHotspot(const RMPoint &pt);

	void SetInitCurPattern(bool status) { m_bInitCurPattern=status; }

	void PlaySfx(int nSfx);

void ReadFromStream(RMDataStream& ds, bool bLOX=false);

	void PauseSound(bool bPause);

protected:
	// Crea una primitiva che ha come task l'item stesso
	virtual RMGfxPrimitive *NewItemPrimitive();

	// Alloca la memoria per gli sprites
	virtual RMGfxSourceBuffer* NewItemSpriteBuffer(int dimx, int dimy, bool bPreRLE);
};


#define MAXBOXES	50		// Non si puo' cambiare, comanda cosi' il boxed
#define MAXHOTSPOT 20		// Idem

class RMBox {
public:
	struct T_HOTSPOT {
		int hotx, hoty;          // coordinate HotSpot
		int destination;         // destinazione HotSpot
	};

public:
	int left,top,right,bottom;		// Vertici BoundingBox
	int adj[MAXBOXES];				// Lista di adjacenza
	int numhotspot;					// Numero HotSpot
	uint8 Zvalue;					// Zvalue per quel BoundingBox
	T_HOTSPOT hotspot[MAXHOTSPOT];	// Lista degli HotSpot

	bool attivo;
	bool bReversed;

private:
	void ReadFromStream(RMDataStream &ds);

public:
	friend RMDataStream &operator>>(RMDataStream &ds, RMBox &box);
};


class RMBoxLoc {
public:
	int numbbox;
	RMBox *boxes;

private:
	void ReadFromStream(RMDataStream& ds);

public:
	RMBoxLoc();
	virtual ~RMBoxLoc();

	friend RMDataStream& operator >>(RMDataStream &ds, RMBoxLoc &bl);
	void RecalcAllAdj(void);
};

#define GAME_BOXES_SIZE 200

class RMGameBoxes {	
protected:
	RMBoxLoc *m_allBoxes[GAME_BOXES_SIZE];
	int m_nLocBoxes;

public:
	RMGameBoxes();
	~RMGameBoxes();

	void Init(void);		
	void Close(void);

	// Prende i box di una locazione
	RMBoxLoc *GetBoxes(int nLoc);
	
	// Calcola in quale box si trova il punto
	int WhichBox(int nLoc, const RMPoint &pt);

	// Controlla che il punto sia dentro un certo box
	bool IsInBox(int nLoc, int nBox, const RMPoint &pt);
	
	// Cambia lo stato di un box
	void ChangeBoxStatus(int nLoc, int nBox, int status);

	// Salvataggi
	int GetSaveStateSize(void);
	void SaveState(byte *buf);
	void LoadState(byte *buf);
};

class RMCharacter : protected RMItem {
public:
	enum PATTERNS {
		PAT_STANDUP = 1,
		PAT_STANDDOWN,
		PAT_STANDLEFT,
		PAT_STANDRIGHT,
		PAT_WALKUP,
		PAT_WALKDOWN,
		PAT_WALKLEFT,
		PAT_WALKRIGHT	
	};

private:
	enum STATUS {
		STAND,
		WALK
	};

	signed short walkcount;
	int dx, dy, olddx, olddy;
	float fx, fy, slope;
	RMPoint linestart, lineend, pathend;
	signed char walkspeed, walkstatus;
	char minpath;
	short nextbox;
	short path[MAXBOXES];
	short pathlenght, pathcount;
	int curbox;
	
	STATUS status;
	int curSpeed;
	bool bEndOfPath;
	uint32 hEndOfPath;
	OSystem::MutexRef csMove;
	int curLocation;
	bool bRemoveFromOT;
	bool bMovingWithoutMinpath;
	RMGameBoxes *theBoxes;
	
	RMPoint m_fixedScroll;
	
private:
	int InWhichBox(const RMPoint &pt); 
	
	short FindPath(short source, short destination);
	RMPoint Searching(char UP, char DOWN, char RIGHT, char LEFT, RMPoint punto);
	RMPoint NearestPoint(const RMPoint &punto);
	
	void GoTo(CORO_PARAM, RMPoint destcoord, bool bReversed=false);
	short ScanLine(const RMPoint &punto);
	RMPoint InvScanLine(const RMPoint &punto);
	RMPoint NearestHotSpot(int sourcebox, int destbox);

	void NewBoxEntered(int nBox);

protected:
	bool bMoving;
	bool bDrawNow;
	bool bNeedToStop;
//		virtual RMGfxPrimitive* NewItemPrimitive();

public:
	RMCharacter();
	virtual ~RMCharacter();

	void LinkToBoxes(RMGameBoxes* theBoxes);

	virtual void RemoveThis(CORO_PARAM, bool &result);
	
	// Aggiorna la posizione del personaggio	
	void DoFrame(CORO_PARAM, RMGfxTargetBuffer *bigBuf, int loc);		

	// Overloading del Draw
	virtual void Draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim);

	// TRUE se si ่ appena fermato
	bool EndOfPath() { return bEndOfPath; }

	// Cambia il pattern del personaggio per fermarlo
	virtual void Stop(CORO_PARAM);

	// Controlla se il personaggio si sta muovendo
	bool IsMoving() { return bMoving; }

	// Muove il personaggio a una certa posizione
	void Move(CORO_PARAM, RMPoint pt, bool *result = NULL);

	// Posiziona il personaggio a una certa posizione SENZA farlo muovere
	void SetPosition(const RMPoint &pt, int newloc = -1);

	// Aspetta la fine del movimento
	void WaitForEndMovement(CORO_PARAM);

	void SetFixedScroll(const RMPoint &fix) { m_fixedScroll = fix; }
	void SetSpeed(int speed) { curSpeed = speed; }
};


class RMWipe : public RMGfxTask {
private:
	bool m_bFading;
	bool m_bEndFade;
	bool m_bUnregister;
	uint32 m_hUnregistered;
	int m_nFadeStep;
	uint32 m_hEndOfFade;
	bool m_bMustRegister;

	RMItem m_wip0r;

public:
	RMWipe();
	virtual ~RMWipe();

	void DoFrame(RMGfxTargetBuffer& bigBuf);
	virtual void Draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim);
	
	void InitFade(int type);
	void CloseFade(void);
	void WaitForFadeEnd(CORO_PARAM);

	virtual void Unregister(void);
	virtual void RemoveThis(CORO_PARAM, bool &result);
	virtual int Priority(void);
};


/**
 * Location
 */
class RMLocation : public RMGfxTaskSetPrior {
public:
    RMString m_name;                  // Nome

  private:
    RMColorMode m_cmode;              // Color mode
	RMGfxSourceBuffer* m_buf;					// Immagine della locazione

	int m_nItems;                     // Numero oggetti
RMItem* m_items;                  // Oggetti

	RMPoint m_curScroll;							// Posizione corrente di scroll
	RMPoint m_fixedScroll;

public:
	// @@@@@@@@@@@@@@@@@@@@@@@
	RMPoint TEMPTonyStart;
	RMPoint TEMPGetTonyStart() { return TEMPTonyStart; }

	int TEMPNumLoc;
	int TEMPGetNumLoc() { return TEMPNumLoc; }

public:
	RMLocation();
	virtual ~RMLocation();

	// Caricamento da disco
	bool Load(const char *lpszFileName);
	bool Load(Common::File &file);
	bool Load(const byte *buf);
	bool Load(RMDataStream &ds);
	bool LoadLOX(RMDataStream &ds);

	// Scaricamento
	void Unload(void);

	// Overloading del Draw
	virtual void Draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim);

	// Prepara un frame disegnando la locazione e tutti i suoi item
	void DoFrame(RMGfxTargetBuffer *bigBuf);

	// Si fa dare il numero dell'item
	RMItem *WhichItemIsIn(const RMPoint &pt);

	// Si fa dare un elemento dal suo codice MPAL
	RMItem* GetItemFromCode(uint32 dwCode);

	// Setta la posizione corrente di scrolling
	void SetScrollPosition(const RMPoint &scroll);

	// Setta un offset aggiuntivo di scrolling da aggiungere sempre
	void SetFixedScroll(const RMPoint &scroll);

	// Aggiorna le coordinate di scrolling in modo da visualizzare sempre il punto fornito
	void UpdateScrolling(const RMPoint &ptShowThis);

	// Legge la posizione di scrolling corrente
	RMPoint ScrollPosition() { return m_curScroll; }

	// Pausa sonoro
	void PauseSound(bool bPause);
};


/**
 * MPAL message, composed of more ASCIIZ
 */
class RMMessage {
private:
	char *lpMessage;
	char *lpPeriods[256];
	int nPeriods;

private:
	void ParseMessage(void);

public:
	RMMessage();
	RMMessage(uint32 dwId);
	virtual ~RMMessage();

	void Load(uint32 dwId);
	bool IsValid() { return lpMessage != NULL; }
	int NumPeriods() { return nPeriods; }
	char *Period(int num) { return lpPeriods[num]; }
	char *operator[](int num) { return lpPeriods[num]; }
};


} // End of namespace Tony

#endif /* TONY_H */
