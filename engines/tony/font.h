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

#ifndef TONY_FONT_H
#define TONY_FONT_H

#include "common/system.h"
#include "common/coroutines.h"
#include "tony/gfxcore.h"
#include "tony/resid.h"

namespace Tony {

class RMInput;
class RMInventory;
class RMItem;
class RMLoc;
class RMLocation;
class RMPointer;

/**
 * Gestisce un font, in cui ha varie surface per ogni lettera
 */
class RMFont : public RMGfxTaskSetPrior {
protected:
	int nLetters;
	RMGfxSourceBuffer8RLEByte *m_letter;
public:
	int m_fontDimx, m_fontDimy;

private:
	int m_dimx, m_dimy;

	class RMFontPrimitive : public RMGfxPrimitive {
	public:
		RMFontPrimitive() : RMGfxPrimitive() {}
		RMFontPrimitive(RMGfxTask *task) : RMGfxPrimitive(task) {}
		virtual ~RMFontPrimitive() { }
		virtual RMGfxPrimitive *Duplicate() {
			return new RMFontPrimitive(*this);
		}

		int m_nChar;
	};

protected:
	// Caricamento del font
	void Load(uint32 resID, int nChars, int dimx, int dimy, uint32 palResID = RES_F_PAL);
	void Load(const byte *buf, int nChars, int dimx, int dimy, uint32 palResID = RES_F_PAL);

	// Scaricamente del font (anche da distruttore)
	void Unload(void);

protected:
	// Conversione (da overloadare)
	virtual int ConvertToLetter(byte nChar) = 0;

	// Lunghezza dei caratteri (da overloadare)
	virtual int LetterLength(byte nChar, byte nNext = 0) = 0;

public:
	virtual int LetterHeight(void) = 0;

public:
	RMFont();
	virtual ~RMFont();

	// Inizializzazione e chiusura
	virtual void Init(void) = 0;
	virtual void Close(void);

	// Funzione del task da overloadare
	virtual void Draw(CORO_PARAM, RMGfxTargetBuffer &bigBug, RMGfxPrimitive *prim);

	// Crea una primitiva per una lettera
	RMGfxPrimitive *MakeLetterPrimitive(byte bChar, int &nLength);

	// Lunghezza in pixel di una stringa con il font corrente
	int StringLen(const RMString &text);
	int StringLen(char bChar, char bNext = 0);
};


class RMFontColor : public virtual RMFont {
private:
	byte m_r, m_g, m_b;

public:
	RMFontColor();
	virtual ~RMFontColor();
	virtual void SetBaseColor(byte r, byte g, byte b);
};


class RMFontWithTables : public virtual RMFont {
protected:
	int cTable[256];
	int lTable[256];
	int lDefault;
	int hDefault;
	signed char l2Table[256][256];

protected:
	// Overload dei metodi
	int ConvertToLetter(byte nChar) {
		return cTable[nChar];
	}
	int LetterLength(byte nChar, byte nNext = 0) {
		return (nChar != -1 ? lTable[nChar] + l2Table[nChar][nNext] : lDefault);
	}

public:
	int LetterHeight() {
		return hDefault;
	}
	virtual ~RMFontWithTables() {}
};


class RMFontParla : public RMFontColor, public RMFontWithTables {
public:
	void Init(void);
	virtual ~RMFontParla() {}
};

class RMFontObj : public RMFontColor, public RMFontWithTables {
private:
	void SetBothCase(int nChar, int nNext, signed char spiazz);

public:
	void Init(void);
	virtual ~RMFontObj() {}
};

class RMFontMacc : public RMFontColor, public RMFontWithTables {
public:
	void Init(void);
	virtual ~RMFontMacc() {}
};

class RMFontCredits : public RMFontColor, public RMFontWithTables {
public:
	void Init(void);
	virtual ~RMFontCredits() {}
	virtual void SetBaseColor(byte r, byte g, byte b) {}
};

/**
 * Gestisce una scritta su schermo, con tutte le possibilita' di formattazione disponibile
 */
class RMText : public RMGfxWoodyBuffer {
private:
	static RMFontColor *m_fonts[4];
	static RMGfxClearTask m_clear;
	int maxLineLength;

public:
	enum HORALIGN {
	    HLEFT,
	    HLEFTPAR,
	    HCENTER,
	    HRIGHT
	};

	enum VERALIGN {
	    VTOP,
	    VCENTER,
	    VBOTTOM
	};

private:
	HORALIGN aHorType;
	VERALIGN aVerType;
	byte m_r, m_g, m_b;

protected:
	virtual void ClipOnScreen(RMGfxPrimitive *prim);

public:
	RMText();
	virtual ~RMText();
	static void InitStatics();
	static void Unload();

	// Setta il tipo di allineamento
	void SetAlignType(HORALIGN aHor, VERALIGN aVer) {
		aHorType = aHor;
		aVerType = aVer;
	}

	// Setta la lunghezza massima di una linea in pixel (utilizzato per formattare il testo)
	void SetMaxLineLength(int max);

	// Scrive un testo
	void WriteText(const RMString &text, int font, int *time = NULL);
	void WriteText(const RMString &text, RMFontColor *font, int *time = NULL);

	// Overloading della funzione ereditata da RMGfxTask per decidere
	// quando eliminare un oggetto dalla OTLIST
	virtual void RemoveThis(CORO_PARAM, bool &result);

	// Overloading del Draw per centrare la scritta, se necessario
	virtual void Draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim);

	// Setta il colore di base
	void SetColor(byte r, byte g, byte b) {
		m_r = r;
		m_g = g;
		m_b = b;
	}
};

/**
 * Gestisce il testo di un dialogo
 */
class RMTextDialog : public RMText {
protected:
	int m_startTime;
	int m_time;
	bool m_bSkipStatus;
	RMPoint dst;
	uint32 hEndDisplay;
	bool m_bShowed;
	bool m_bForceTime;
	bool m_bForceNoTime;
	uint32 hCustomSkip;
	uint32 hCustomSkip2;
	RMInput *m_input;
	bool m_bAlwaysDisplay;
	bool m_bNoTab;

public:
	RMTextDialog();
	virtual ~RMTextDialog();

	// Scrive un testo
	void WriteText(const RMString &text, int font, int *time = NULL);
	void WriteText(const RMString &text, RMFontColor *font, int *time = NULL);

	// Overloading della funzione ereditata da RMGfxTask per decidere
	// quando eliminare un oggetto dalla OTLIST
	virtual void RemoveThis(CORO_PARAM, bool &result);

	// Overloading della funzione di deregistrazione, utilizzata per capire
	// quando ci leviamo di torno
	virtual void Unregister(void);

	// Overloading del Draw per centrare la scritta, se necessario
	virtual void Draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim);

	// Setta la posizione
	void SetPosition(const RMPoint &pt) {
		dst = pt;
	}

	// Aspetta che venga finita la visualizzazione
	void WaitForEndDisplay(CORO_PARAM);
	void SetCustomSkipHandle(uint32 hCustomSkip);
	void SetCustomSkipHandle2(uint32 hCustomSkip);
	void SetSkipStatus(bool bEnabled);
	void SetForcedTime(uint32 dwTime);
	void SetNoTab(void);
	void ForceTime(void);
	void ForceNoTime(void);
	void SetAlwaysDisplay(void);

	// Setta il dispositivo di input, per permettere skip da mouse
	void SetInput(RMInput *input);

	void Show(void);
	void Hide(CORO_PARAM);
};

class RMTextDialogScrolling : public RMTextDialog {
protected:
	RMLocation *curLoc;
	RMPoint startScroll;

	virtual void ClipOnScreen(RMGfxPrimitive *prim);

public:
	RMTextDialogScrolling();
	RMTextDialogScrolling(RMLocation *loc);
	virtual ~RMTextDialogScrolling();

	virtual void Draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim);
};


/****************************************************************************\
*       class RMTextItemName
*       --------------------
* Description: Gestisce il nome dell'oggetto selezionato su schermo
\****************************************************************************/

class RMTextItemName : protected RMText {
protected:
	RMPoint m_mpos;
	RMPoint m_curscroll;
	RMItem *m_item;
	RMString m_itemName;

public:
	RMTextItemName();
	virtual ~RMTextItemName();

	void SetMouseCoord(const RMPoint &m) {
		m_mpos = m;
	}

	void DoFrame(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMLocation &loc, RMPointer &ptr, RMInventory &inv);
	virtual void Draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim);

	RMPoint GetHotspot();
	RMItem *GetSelectedItem();
	bool IsItemSelected();
	bool IsNormalItemSelected();

	virtual void RemoveThis(CORO_PARAM, bool &result) {
		result = true;
	}
};


/**
 * Gestisce la schermata di scelta delle voci di un dialogo
 */
class RMDialogChoice : public RMGfxWoodyBuffer {
private:
	int m_curSelection;
	int m_numChoices;
	RMText *m_drawedStrings;
	RMPoint *m_ptDrawStrings;
	int m_curAdded;
	bool m_bShow;
	RMGfxSourceBuffer8 DlgText;
	RMGfxSourceBuffer8 DlgTextLine;
	RMPoint m_ptDrawPos;
	uint32 hUnreg;
	bool bRemoveFromOT;

protected:
	void Prepare(CORO_PARAM);
	void SetSelected(CORO_PARAM, int pos);

public:
	virtual void RemoveThis(CORO_PARAM, bool &result);
	virtual void Draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim);
	void Unregister(void);

public:
	// Inizializzazione
	RMDialogChoice();
	virtual ~RMDialogChoice();

	// Inizializzazione e chiusura
	void Init(void);
	void Close(void);

	// Setta il numero delle frasi possibili, che dovranno essere poi aggiunte
	// con AddChoice()
	void SetNumChoices(int num);

	// Aggiunge una stringa con la scelta
	void AddChoice(const RMString &string);

	// Mostra e nasconde la scelta, con eventuali animazioni
	// NOTA: Se non viene passato parametro alla Show(), è obbligo del
	// chiamante assicurarsi che la classe venga inserita alla OTlist
	void Show(CORO_PARAM, RMGfxTargetBuffer *bigBuf = NULL);
	void Hide(CORO_PARAM);

	// Polling di aggiornamento
	void DoFrame(CORO_PARAM, RMPoint ptMousePos);

	// Ritorna la voce attualmente selezionata, o -1 se nessuna è selezionata
	int GetSelection(void);
};

} // End of namespace Tony

#endif
