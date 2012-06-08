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

#include "common/textconsole.h"
#include "tony/game.h"
#include "tony/tony.h"

namespace Tony {


/****************************************************************************\
*       Defines
\****************************************************************************/

/* Massimo numero di bytes da decodificare in una singola chiamata a CODEC */
#define MAXDECODESIZE          (44100 * 2 * 2)

#define RELEASE(x)             {if ((x) != NULL) { (x)->Release(); x = NULL; }}

/****************************************************************************\
*****************************************************************************
*       class CODEC (ABSTRACT)
*       -----------
* Description: classe base per CODEC.
*****************************************************************************
\****************************************************************************/

class CODEC {
protected:
	bool bEndReached;

public:
	bool bLoop;
	CODEC(bool bLoop = true);
	virtual ~CODEC();
	virtual uint32 Decompress(HANDLE hStream, void *lpBuf, uint32 dwSize) = 0;
	virtual uint32 Decompress(Common::File &fp, void *lpBuf, uint32 dwSize) = 0;
	virtual void LoopReset() = 0;
	bool EndOfStream();
};


/****************************************************************************\
*****************************************************************************
*       class CODECRAW
*       --------------
* Description: CODEC di play da disco di campioni puri
*****************************************************************************
\****************************************************************************/

class CODECRAW : public CODEC {
public:
	CODECRAW(bool bLoop = true);
	virtual ~CODECRAW();
	virtual uint32 Decompress(HANDLE hStream, void *lpBuf, uint32 dwSize);
	virtual uint32 Decompress(Common::File &fp, void *lpBuf, uint32 dwSize);
	virtual void LoopReset();
};


/****************************************************************************\
*****************************************************************************
*       class CODECADPCM
*       ----------------
* Description: CODEC per play di compressione ADPCM
*****************************************************************************
\****************************************************************************/

class CODECADPCM : public CODECRAW {
protected:
	byte *lpTemp;
	static const int indexTable[16];
	static const int stepSizeTable[89];

public:
	CODECADPCM(bool bLoop = true, byte *lpTempBuffer = NULL);
	virtual ~CODECADPCM();
	virtual uint32 Decompress(HANDLE hStream, void *lpBuf, uint32 dwSize) = 0;
	virtual uint32 Decompress(Common::File &fp, void *lpBuf, uint32 dwSize) = 0;
	virtual void LoopReset() = 0;
};

class CODECADPCMSTEREO : public CODECADPCM {
protected:
	int valpred[2], index[2];

public:
	CODECADPCMSTEREO(bool bLoop = true, byte *lpTempBuffer = NULL);
	virtual ~CODECADPCMSTEREO();
	virtual uint32 Decompress(HANDLE hStream, void *lpBuf, uint32 dwSize);
	virtual uint32 Decompress(Common::File &fp, void *lpBuf, uint32 dwSize);
	virtual void LoopReset();
};

class CODECADPCMMONO : public CODECADPCM {
protected:
	int valpred, index;

public:
	CODECADPCMMONO(bool bLoop = true, byte *lpTempBuffer = NULL);
	virtual ~CODECADPCMMONO();
	virtual uint32 Decompress(HANDLE hStream, void *lpBuf, uint32 dwSize);
	virtual uint32 Decompress(Common::File &fp, void *lpBuf, uint32 dwSize);
	virtual void LoopReset();
};

/****************************************************************************\
*       Metodi per CODEC
\****************************************************************************/


/****************************************************************************\
*
* Function:     CODEC::CODEC(bool loop = true);
*
* Description:  Costruttore standard. E' possibile specificare se si vuole
*               attivare o disattivare il loop (che di default e' attivo).
*
* Input:        bool loop               true se si vuole attivare il loop,
*                                       false per disattivarlo
*
\****************************************************************************/

CODEC::CODEC(bool loop) {
	bLoop = loop;
	bEndReached = false;
}

CODEC::~CODEC() {

}

/****************************************************************************\
*
* Function:     bool CODEC::EndOfStream()
*
* Description:  Informa se abbiamo raggiunto la fine dello stream
*
* Return:       true se siamo arrivati alla fine, false altrimenti
*
\****************************************************************************/

bool CODEC::EndOfStream() {
	return bEndReached;
}


/****************************************************************************\
*       Metodi per CODECRAW
\****************************************************************************/

/****************************************************************************\
*
* Function:     CODECRAW::CODECRAW(bool loop = true);
*
* Description:  Costruttore standard. Richiama solamente il costruttore della
*               classe astratta CODEC.
*
* Input:        bool loop               true se si vuole attivare il loop,
*                                       false per disattivarlo
*
\****************************************************************************/

CODECRAW::CODECRAW(bool loop) : CODEC(loop) {
}

CODECRAW::~CODECRAW() {

}

/****************************************************************************\
*
* Function:     CODECRAW::LoopReset();
*
* Description:  Resetta il playing prima di iniziare di nuovo il file.
*               Nel caso dei file RAW non fa nulla.
*
\****************************************************************************/

void CODECRAW::LoopReset() {
}

/****************************************************************************\
*
* Function:     uint32 CODECRAW::Decompress(HANDLE hStream, void *lpBuf,
*                 uint32 dwSize)
*
* Description:  Gestisce il formato RAW: semplicemente copia dal file
*               stream nel buffer.
*
* Return:       Se e' stata raggiunta la fine del file, indica quale byte,
*               tra quelli letti, e' l'inizio del nuovo loop. Altrimenti
*               il valore non e' determinato.
*
\****************************************************************************/

uint32 CODECRAW::Decompress(HANDLE hStream, void *buf, uint32 dwSize) {
#if 0
	byte *lpBuf = (byte *)buf;
	uint32 dwRead;
	uint32 dwEOF;

	bEndReached = false;
	dwEOF = 0;
	ReadFile(hStream, lpBuf, dwSize, &dwRead, NULL);

	if (dwRead < dwSize) {
		dwEOF = dwRead;
		bEndReached = true;

		if (!bLoop) {
			ZeroMemory(lpBuf + dwRead, dwSize - dwRead);
		} else {
			SetFilePointer(hStream, 0, NULL, FILE_BEGIN);
			ReadFile(hStream, lpBuf + dwRead, dwSize - dwRead, &dwRead, NULL);
		}
	}

	return dwEOF;
#endif
	return 0;
}

uint32 CODECRAW::Decompress(Common::File &fp, void *buf, uint32 dwSize) {
	byte *lpBuf = (byte *)buf;
	uint32 dwRead;
	uint32 dwEOF;

	bEndReached = false;
	dwEOF = 0;

	dwRead = fp.read(lpBuf, dwSize);

	if (dwRead < dwSize) {
		dwEOF = dwRead;
		bEndReached = true;

		if (!bLoop) {
			Common::fill(lpBuf + dwRead, lpBuf + dwRead + (dwSize - dwRead), 0);
		} else {
			fp.seek(0);
			fp.read(lpBuf + dwRead, dwSize - dwRead);
		}
	}

	return dwEOF;
}

/****************************************************************************\
*       Metodi per CODECADPCM
\****************************************************************************/

const int CODECADPCM::indexTable[16] = {
	-1, -1, -1, -1, 2, 4, 6, 8,
	-1, -1, -1, -1, 2, 4, 6, 8,
};

const int CODECADPCM::stepSizeTable[89] = {
	7, 8, 9, 10, 11, 12, 13, 14, 16, 17,
	19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
	50, 55, 60, 66, 73, 80, 88, 97, 107, 118,
	130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
	337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
	876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066,
	2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
	5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
	15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
};


/****************************************************************************\
*
* Function:     CODECADPCM::CODECADPCM()
*
* Description:  Costruttore. Inizializza le tabelle e alloca la memoria
*               temporanea.
*
\****************************************************************************/

CODECADPCM::CODECADPCM(bool loop, byte *lpTempBuffer) : CODECRAW(loop) {
	/* Alloca la memoria temporanea */
	if (lpTempBuffer != NULL) {
		lpTemp = lpTempBuffer;
	} else {
		lpTemp = (byte *)globalAlloc(GMEM_FIXED | GMEM_ZEROINIT, MAXDECODESIZE);

		if (lpTemp == NULL) {
			error("Insufficient memory!");
			return;
		}
	}
}


CODECADPCMMONO::CODECADPCMMONO(bool loop, byte *lpTempBuffer) : CODECADPCM(loop, lpTempBuffer) {
	/* Inizializza per il playing */
	LoopReset();
}

CODECADPCMMONO::~CODECADPCMMONO() {
}


CODECADPCMSTEREO::CODECADPCMSTEREO(bool loop, byte *lpTempBuffer) : CODECADPCM(loop, lpTempBuffer) {
	/* Inizializza per il playing */
	LoopReset();
}

CODECADPCMSTEREO::~CODECADPCMSTEREO() {

}

/****************************************************************************\
*
* Function:     CODECADPCM::~CODECADPCM()
*
* Description:  Distruttore. Libera la memoria temporanea.
*
\****************************************************************************/

CODECADPCM::~CODECADPCM() {
	globalFree(lpTemp);
}


/****************************************************************************\
*
* Function:     CODECADPCM::LoopReset()
*
* Description:  Resetta il player prima di ogni play o loop.
*
\****************************************************************************/

void CODECADPCMSTEREO::LoopReset() {
	valpred[0] = 0;
	valpred[1] = 0;
	index[0] = 0;
	index[1] = 0;
}

void CODECADPCMMONO::LoopReset() {
	valpred = 0;
	index = 0;
}

/****************************************************************************\
*
* Function:     CODECADPCM::Decompress(HANDLE hStream, void *lpBuf,
*                 uint32 dwSize);
*
* Description:  Gestisce il formato ADPCM 16:4. La routine dovrebbe essere
*               ottimizzata in Assembler per garantire migliori performance.
*
\****************************************************************************/

uint32 CODECADPCMMONO::Decompress(HANDLE hFile, void *buf, uint32 dwSize) {
#if 0
	uint16 *lpBuf = (uint16 *)buf;
	byte *inp;
	int bufferstep;
	int cache;
	int delta;
	int sign;
	int vpdiff;
	uint32 eof, i;
	int step;
	uint32 dwRead;

	bufferstep = 1;
	step = stepSizeTable[index];

	/* Richiama il CODEC RAW per leggere da disco lo stream con loop. La
	   chiamata e' possibile perche' abbiamo ereditato CODECADPCM da CODECRAW,
	   e non semplicemente da CODEC. */
	eof = CODECRAW::Decompress(hFile, lpTemp, dwSize / 4);
	inp = lpTemp;

	eof *= 2;
	/* Se bisogna loopare subito lo fa */
	if (EndOfStream() && eof == 0) {
		LoopReset();
		bufferstep = 1;
		step = stepSizeTable[index];
	} else if (!EndOfStream())
		eof = 0;

	dwSize /= 2;
	for (i = 0; i < dwSize; i++) {
		/* Controlla se siamo alla fine del file, e bisogna loopare */
		if (eof != 0 && i == eof) {
			LoopReset();
			bufferstep = 1;
			step = stepSizeTable[index];
		}

		/* Legge il delta (4 bit) */
		if (bufferstep) {
			cache = *inp++;
			delta = (cache >> 4) & 0xF;
		} else
			delta = cache & 0xF;

		/* Trova il nuovo indice */
		index += indexTable[delta];
		if (index < 0) index = 0;
		if (index > 88) index = 88;

		/* Legge il segno e lo separa dall'ampliamento */
		sign = delta & 8;
		delta = delta & 7;

		/* Trova la differenza dal valore precedente */
		vpdiff = step >> 3;
		if (delta & 4) vpdiff += step;
		if (delta & 2) vpdiff += step >> 1;
		if (delta & 1) vpdiff += step >> 2;

		if (sign)
			valpred -= vpdiff;
		else
			valpred += vpdiff;

		/* Controlla i limiti del valore trovato */
		if (valpred > 32767)
			valpred = 32767;
		else if (valpred < -32768)
			valpred = -32768;

		/* Aggiorna lo step */
		step = stepSizeTable[index];

		/* Scrive il valore trovato */
		*lpBuf++ = (signed short)valpred;

		bufferstep = !bufferstep;
	}

	return eof / 2;
#endif
	return 0;
}

uint32 CODECADPCMMONO::Decompress(Common::File &fp, void *buf, uint32 dwSize) {
	uint16 *lpBuf = (uint16 *)buf;
	byte *inp;
	int bufferstep;
	int cache = 0;
	int delta;
	int sign;
	int vpdiff;
	uint32 eof, i;
	int step;

	bufferstep = 1;
	step = stepSizeTable[index];

	/* Richiama il CODEC RAW per leggere da disco lo stream con loop. La
	   chiamata e' possibile perche' abbiamo ereditato CODECADPCM da CODECRAW,
	   e non semplicemente da CODEC. */
	eof = CODECRAW::Decompress(fp, lpTemp, dwSize / 4);
	inp = lpTemp;

	eof *= 2;

	/* Se bisogna loopare subito lo fa */
	if (EndOfStream() && eof == 0) {
		LoopReset();
		bufferstep = 1;
		step = stepSizeTable[index];
	} else if (!EndOfStream())
		eof = 0;

	dwSize /= 2;
	for (i = 0; i < dwSize; i++) {
		/* Controlla se siamo alla fine del file, e bisogna loopare */
		if (eof != 0 && i == eof) {
			LoopReset();
			bufferstep = 1;
			step = stepSizeTable[index];
		}

		/* Legge il delta (4 bit) */
		if (bufferstep) {
			cache = *inp++;
			delta = (cache >> 4) & 0xF;
		} else
			delta = cache & 0xF;

		/* Trova il nuovo indice */
		index += indexTable[delta];
		if (index < 0) index = 0;
		if (index > 88) index = 88;

		/* Legge il segno e lo separa dall'ampliamento */
		sign = delta & 8;
		delta = delta & 7;

		/* Trova la differenza dal valore precedente */
		vpdiff = step >> 3;
		if (delta & 4) vpdiff += step;
		if (delta & 2) vpdiff += step >> 1;
		if (delta & 1) vpdiff += step >> 2;

		if (sign)
			valpred -= vpdiff;
		else
			valpred += vpdiff;

		/* Controlla i limiti del valore trovato */
		if (valpred > 32767)
			valpred = 32767;
		else if (valpred < -32768)
			valpred = - 32768;

		/* Aggiorna lo step */
		step = stepSizeTable[index];

		/* Scrive il valore trovato */
		*lpBuf ++ = (signed short)valpred;

		bufferstep = !bufferstep;
	}

	return eof / 2;
}

uint32 CODECADPCMSTEREO::Decompress(HANDLE hFile, void *buf, uint32 dwSize) {
	uint16 *lpBuf = (uint16 *)buf;
	byte *inp;
	int bufferstep;
	int cache = 0;
	int delta;
	int sign;
	int vpdiff;
	uint32 eof, i;
	int step[2];

	bufferstep = 1;
	step[0] = stepSizeTable[index[0]];
	step[1] = stepSizeTable[index[1]];

	/* Richiama il CODEC RAW per leggere da disco lo stream con loop. La
	   chiamata e' possibile perche' abbiamo ereditato CODECADPCM da CODECRAW,
	   e non semplicemente da CODEC. */
	eof = CODECRAW::Decompress(hFile, lpTemp, dwSize / 4);
	inp = lpTemp;

	eof *= 2;

	/* Se bisogna loopare subito lo fa */
	if (EndOfStream() && eof == 0) {
		LoopReset();
		bufferstep = 1;
		step[0] = stepSizeTable[index[0]];
		step[1] = stepSizeTable[index[1]];
	} else if (!EndOfStream())
		eof = 0;

	dwSize /= 2;
	for (i = 0; i < dwSize; i++) {
		/* Controlla se siamo alla fine del file, e bisogna loopare */
		if (eof != 0 && i == eof) {
			LoopReset();
			bufferstep = 1;
			step[0] = stepSizeTable[index[0]];
			step[1] = stepSizeTable[index[1]];
		}

		/* Legge il delta (4 bit) */
		if (bufferstep) {
			cache = *inp++;
			delta = cache & 0xF;
		} else
			delta = (cache >> 4) & 0xF;

		/* Trova il nuovo indice */
		index[bufferstep] += indexTable[delta];
		if (index[bufferstep] < 0) index[bufferstep] = 0;
		if (index[bufferstep] > 88) index[bufferstep] = 88;

		/* Legge il segno e lo separa dall'ampliamento */
		sign = delta & 8;
		delta = delta & 7;

		/* Trova la differenza dal valore precedente */
		vpdiff = step[bufferstep] >> 3;
		if (delta & 4) vpdiff += step[bufferstep];
		if (delta & 2) vpdiff += step[bufferstep] >> 1;
		if (delta & 1) vpdiff += step[bufferstep] >> 2;

		if (sign)
			valpred[bufferstep] -= vpdiff;
		else
			valpred[bufferstep] += vpdiff;

		/* Controlla i limiti del valore trovato */
		if (valpred[bufferstep] > 32767)
			valpred[bufferstep] = 32767;
		else if (valpred[bufferstep] < -32768)
			valpred[bufferstep] = -32768;

		/* Aggiorna lo step */
		step[bufferstep] = stepSizeTable[index[bufferstep]];

		/* Scrive il valore trovato */
		*lpBuf ++ = (signed short)valpred[bufferstep];

		bufferstep = !bufferstep;
	}

	return eof / 2;
}


uint32 CODECADPCMSTEREO::Decompress(Common::File &fp, void *buf, uint32 dwSize) {
	uint16 *lpBuf = (uint16 *)buf;
	byte *inp;
	int bufferstep;
	int cache = 0;
	int delta;
	int sign;
	int vpdiff;
	uint32 eof, i;
	int step[2];

	bufferstep = 1;
	step[0] = stepSizeTable[index[0]];
	step[1] = stepSizeTable[index[1]];

	/* Richiama il CODEC RAW per leggere da disco lo stream con loop. La
	   chiamata e' possibile perche' abbiamo ereditato CODECADPCM da CODECRAW,
	   e non semplicemente da CODEC. */
	eof = CODECRAW::Decompress(fp, lpTemp, dwSize / 4);
	inp = lpTemp;

	eof *= 2;
	/* Se bisogna loopare subito lo fa */
	if (EndOfStream() && eof == 0) {
		LoopReset();
		bufferstep = 1;
		step[0] = stepSizeTable[index[0]];
		step[1] = stepSizeTable[index[1]];
	} else if (!EndOfStream())
		eof = 0;

	dwSize /= 2;
	for (i = 0; i < dwSize; i++) {
		/* Controlla se siamo alla fine del file, e bisogna loopare */
		if (eof != 0 && i == eof) {
			LoopReset();
			bufferstep = 1;
			step[0] = stepSizeTable[index[0]];
			step[1] = stepSizeTable[index[1]];
		}

		/* Legge il delta (4 bit) */
		if (bufferstep) {
			cache = *inp++;
			delta = cache & 0xF;
		} else
			delta = (cache >> 4) & 0xF;

		/* Trova il nuovo indice */
		index[bufferstep] += indexTable[delta];
		if (index[bufferstep] < 0) index[bufferstep] = 0;
		if (index[bufferstep] > 88) index[bufferstep] = 88;

		/* Legge il segno e lo separa dall'ampliamento */
		sign = delta & 8;
		delta = delta & 7;

		/* Trova la differenza dal valore precedente */
		vpdiff = step[bufferstep] >> 3;
		if (delta & 4) vpdiff += step[bufferstep];
		if (delta & 2) vpdiff += step[bufferstep] >> 1;
		if (delta & 1) vpdiff += step[bufferstep] >> 2;

		if (sign)
			valpred[bufferstep] -= vpdiff;
		else
			valpred[bufferstep] += vpdiff;

		/* Controlla i limiti del valore trovato */
		if (valpred[bufferstep] > 32767)
			valpred[bufferstep] = 32767;
		else if (valpred[bufferstep] < -32768)
			valpred[bufferstep] = -32768;

		/* Aggiorna lo step */
		step[bufferstep] = stepSizeTable[index[bufferstep]];

		/* Scrive il valore trovato */
		*lpBuf ++ = (signed short)valpred[bufferstep];

		bufferstep = !bufferstep;
	}

	return eof / 2;
}


/****************************************************************************\
*       Metodi per FPSOUND
\****************************************************************************/

/****************************************************************************\
*
* Function:     FPSOUND::FPSOUND();
*
* Description:  Costruttore di default. Inizializza gli attributi.
*
\****************************************************************************/

FPSOUND::FPSOUND() {
	lpDS = NULL;
	lpDSBPrimary = NULL;
	hwnd = 0;
	bSoundSupported = false;
}


/****************************************************************************\
*
* Function:     bool FPSOUND::Init(HWND hWnd);
*
* Description:  Inizializza l'oggetto, e prepara tutto il necessario per
*               creare stream e effetti sonori.
*
* Input:        HWND hWnd               Handle della finestra principale
*
* Return:       True se tutto OK, false in caso di errore.
*
\****************************************************************************/

bool FPSOUND::Init(/*HWND hWnd*/) {
#ifdef REFACTOR_ME
	HRESULT err;
	static DSBUFFERDESC dsbdesc;
	static PCMWAVEFORMAT pcmwf;
	static char errbuf[128];

	/* Salva l'handle della finestra nella variabile globale. DirectSound ha
	   bisogno dell'handle per effetuare il multitasking sonoro. */
	hwnd = hWnd;

	/* Di default, disabilita il sonoro. Se non troveremo problemi, lo
	   riabiliteremo alla fine della routine */
	bSoundSupported = false;

	/* Crea un oggetto DirectSound. Usiamo il driver sonoro settato di default.
	   In realta' sarebbe possibile richiedere una lista delle schede sonore
	   presenti, e lasciare scegliere all'utente quale utilizzare, ma mi sembra
	   una perdita di tempo. */
	if ((err = DirectSoundCreate(NULL, &lpDS, NULL)) != DS_OK) {
		return false;
	}

	/* Richiede le caratteristiche del driver sonoro */
	dscaps.dwSize = sizeof(dscaps);
	lpDS->GetCaps(&dscaps);

	/* Controlla se siamo in emulazione, e in caso affermativo avverte l'utente */
	if ((dscaps.dwFlags & DSCAPS_EMULDRIVER))
		error("The current sound driver is not directly supported by DirectSound. This will slow down sound performance of the game.");

	/* Setta il livello di cooperazione a esclusivo. In questo modo il gioco
	   sara' il solo ad accedere alla scheda sonora mentre e' in escuzione, ed
	eventuali player in background saranno automaticamente stoppati.
	Inoltre in questo modo e' possibile settare il formato di output sonoro
	del primary buffer */
	if ((err = lpDS->SetCooperativeLevel(hWnd, DSSCL_PRIORITY)) != DS_OK) {
		MessageBox(hwnd, "Cannot set exclusive mode!", "soundInit()", MB_OK);
		return false;
	}


	/* Crea il primary buffer. In realta' DirectSound la farebbe automaticamente,
	   ma noi vogliamo il pointer al primary perche' dobbiamo settare il
	   formato di output */
	ZeroMemory(&dsbdesc, sizeof(dsbdesc));
	dsbdesc.dwSize = sizeof(dsbdesc);
	dsbdesc.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_PRIMARYBUFFER;
	if (lpDS->CreateSoundBuffer(&dsbdesc, &lpDSBPrimary, NULL) != DS_OK) {
		MessageBox(hwnd, "Cannot create primary buffer!", "soundInit()", MB_OK);
		return false;
	}

	/* Settiamo il formato del buffer primario. L'ideale sarebbe 16bit 44khz
	   stereo, ma dobbiamo anche controllare che cio' sia permesso dalla scheda
	   sonora, guardando nelle caratteristiche che abbiamo richiesto sopra.
	   Inoltre in seguito sara' possibile lasciare scegliere all'utente il
	   formato da utilizzare */
	pcmwf.wBitsPerSample = ((dscaps.dwFlags & DSCAPS_PRIMARY16BIT) != 0 ? 16 : 8);
	pcmwf.wf.wFormatTag = WAVE_FORMAT_PCM;
	pcmwf.wf.nChannels = ((dscaps.dwFlags & DSCAPS_PRIMARYSTEREO) != 0 ? 2 : 1);
	pcmwf.wf.nSamplesPerSec = 44100;
	pcmwf.wf.nBlockAlign = (pcmwf.wBitsPerSample / 8) * pcmwf.wf.nChannels;
	pcmwf.wf.nAvgBytesPerSec = (uint32)pcmwf.wf.nBlockAlign * (uint32)pcmwf.wf.nSamplesPerSec;

	if ((err = lpDSBPrimary->SetFormat((LPWAVEFORMATEX) & pcmwf)) != DS_OK)  {
		wsprintf(errbuf, "Error setting the output format (%lx)", err);
		MessageBox(hwnd, errbuf, "soundInit()", MB_OK);
		return false;
	}

	/* Controlla che il driver DirectSound supporti buffer secondari con
	   play di stream 16bit, 44khz stereo */
	if (dscaps.dwMaxSecondarySampleRate != 0 && dscaps.dwMaxSecondarySampleRate < 44100) {
		wsprintf(errbuf, "Driver does not support 16bit 44khz stereo mixing! (%lu)", dscaps.dwMaxSecondarySampleRate);
		MessageBox(hwnd, errbuf, "soundInit()", MB_OK);
		return false;
	}

	/* Tutto OK. */
	bSoundSupported = true;
#endif
	return true;
}


/****************************************************************************\
*
* Function:     FPSOUND::~FPSOUND();
*
* Description:  Deinizializza l'oggetto, disallocando la memoria.
*
\****************************************************************************/

FPSOUND::~FPSOUND() {
#ifdef REFACTOR_ME
	RELEASE(lpDSBPrimary);
	RELEASE(lpDS);
#endif
}


/****************************************************************************\
*
* Function:     bool CreateStream(FPSTREAM** lplpStream);
*
* Description:  Alloca un oggetti di tipo FPSTREAM, e ritorna il suo
*               puntatore dopo averlo inizializzato.
*
* Input:        FPSTREAM** lplpStream   Conterra' il pointer all'oggetto
*                                       appena creato.
*
* Return:       true se tutto OK, false in caso di errore
*
* Note:         L'utilizzo di funzioni del tipo CreateStream(), CreateSfx(),
*               sono dovute al fatto che i costruttori delle classi FPSTREAM
*               e FPSFX richiedono che DirectSound sia gia' stato
*               inzializzato. In questo modo quindi si evitano dei bugs
*               che si verrebbero a creare se venisse dichiarata un oggetto
*               di tipo FPSTREAM o FPSFX globale (o cmq prima della
*               inizializzazione di DirectSound).
*
\****************************************************************************/

bool FPSOUND::CreateStream(FPSTREAM **lplpStream) {
	(*lplpStream) = new FPSTREAM(lpDS, hwnd, bSoundSupported);

	return (*lplpStream != NULL);
}



/****************************************************************************\
*
* Function:     bool CreateSfx(FPSFX** lplpSfx);
*
* Description:  Alloca un oggetti di tipo FPSFX e ritorna il suo
*               puntatore dopo averlo inizializzato.
*
* Input:        FPSFX** lplpSfx         Conterra' il pointer all'oggetto
*                                       appena creato.
*
* Return:       true se tutto OK, false in caso di errore
*
* Note:         Vedi le note di CreateStream()
*
\****************************************************************************/

bool FPSOUND::CreateSfx(FPSFX **lplpSfx) {
	(*lplpSfx) = new FPSFX(lpDS, hwnd, bSoundSupported);

	return (*lplpSfx != NULL);
}



/****************************************************************************\
*
* Function:     void SetMasterVolume(int dwVolume);
*
* Description:  Setta il volume generale
*
* Input:        int dwVolume          Volume da settare (0-63)
*
\****************************************************************************/

void FPSOUND::SetMasterVolume(int dwVolume) {
#ifdef REFACTOR_ME

	if (!bSoundSupported)
		return;

	if (dwVolume > 63) dwVolume = 63;
	if (dwVolume < 0) dwVolume = 0;

	lpDSBPrimary->SetVolume(dwVolume * (DSBVOLUME_MAX - DSBVOLUME_MIN) / 64 + DSBVOLUME_MIN);
#endif
}


/****************************************************************************\
*
* Function:     void GetMasterVolume(int *lpdwVolume);
*
* Description:  Richiede il volume generale
*
* Input:        int *lpdwVolume        Variabile che conterra' il volume (0-63)
*
\****************************************************************************/

void FPSOUND::GetMasterVolume(int *lpdwVolume) {
#ifdef REFACTOR_ME
	if (!bSoundSupported)
		return;

	lpDSBPrimary->GetVolume((uint32 *)lpdwVolume);
	*lpdwVolume -= (DSBVOLUME_MIN);
	*lpdwVolume *= 64;
	*lpdwVolume /= (DSBVOLUME_MAX - DSBVOLUME_MIN);
#endif
}


/****************************************************************************\
*       Metodi di FPSFX
\****************************************************************************/

/****************************************************************************\
*
* Function:     FPSFX(LPDIRECTSOUND lpDS, bool bSoundOn);
*
* Description:  Costruttore di default. *NON* bisogna dichiarare direttamente
*               un oggetto, ma crearlo piuttosto tramite FPSOUND::CreateSfx()
*
\****************************************************************************/

FPSFX::FPSFX(LPDIRECTSOUND lpds, HWND hWnd, bool bSoundOn) {
#ifdef REFACTOR_ME

	static char errbuf[128];

	//hwnd=hWnd;
	bSoundSupported = bSoundOn;
	bFileLoaded = false;
	bIsPlaying = false;
	bPaused = false;
	lpDSBuffer = NULL;
	lpDSNotify = NULL;
	lpDS = lpds;
	lastVolume = 63;
	hEndOfBuffer = CORO_INVALID_PID_VALUE;
	bIsVoice = false;

	if (bSoundSupported == false)
		return;

	/* Poiché non abbiamo ancora nessun dato sull'effetto sonoro, non possiamo fare nulla */
#else
	bIsVoice = false;
	lastVolume = 0;
	dwFreq = 0;
	hEndOfBuffer = CORO_INVALID_PID_VALUE;
	bFileLoaded = false;
	bSoundSupported = false;
	bLoop = false;
	bPaused = false;
	bStereo = false;
	b16bit = false;
	bIsPlaying = false;
	bIsVoice = false;
#endif
}


/****************************************************************************\
*
* Function:     ~FPSFX();
*
* Description:  Distruttore di default. Si preoccupa anche di fermare il sound
*                               effect eventualmente in esecuzione, e disallocare la memoria
*                               da esso occupata.
*
\****************************************************************************/

FPSFX::~FPSFX() {
#ifdef REFACTOR_ME

	if (!bSoundSupported)
		return;

	if (bIsPlaying)
		Stop();

	RELEASE(lpDSNotify);

	if (hEndOfBuffer != CORO_INVALID_PID_VALUE)
		CloseHandle(hEndOfBuffer);

	RELEASE(lpDSBuffer);
#endif
}


/****************************************************************************\
*
* Function:     Release();
*
* Description:  Rilascia la memoria dell'oggetto. Deve essere richiamata quando
*               l'oggetto non serve piu' e **SOLO SE** l'oggetto e' stato
*               creato con la FPSOUND::CreateStream().
*
* Note:         Eventuali puntatori all'oggetto non sono piu' validi dopo
*               questa chiamata.
*
\****************************************************************************/

void FPSFX::Release() {
	delete this;
//	return NULL;
}



/****************************************************************************\
*
* Function:     bool LoadFile(byte *lpBuf);
*
* Description:  Apre un file di effetto sonoro e lo carica.
*
* Input:        byte *lpBuf            Buffer dove si trova l'sfx
*               uint32 dwCoded           CODEC da utilizzare per decomprimere
*                                       i campioni sonori
*
* Return:       true se tutto OK, false in caso di errore
*
\****************************************************************************/

bool FPSFX::LoadFile(byte *lpBuf, uint32 dwCodec) {
#ifdef REFACTOR_ME
	static PCMWAVEFORMAT pcmwf;
	static DSBUFFERDESC dsbdesc;
	static HRESULT err;
	static char errbuf[128];
	uint32 dwHi;
	struct WAVH {
		int nChunckSize;
		uint16 wFormatTag;
		uint16 nChannels;
		int nSamplesPerSec;
		int nAvgBytesPerSec;
		uint16 nBlockAlign;
		uint16 nBitsPerSample;
	} *WAVHeader;
	uint32 dwSize;
	void *lpLock;

	if (!bSoundSupported)
		return true;

	/* Nel buffer troviamo un file WAV completo, almeno per ora */
	if (dwCodec != FPCODEC_WAV)
		return false;

	if (lpBuf[0] != 'W' || lpBuf[1] != 'A' || lpBuf[2] != 'V' || lpBuf[3] != 'E')
		return false;
	if (lpBuf[4] != 'f' || lpBuf[5] != 'm' || lpBuf[6] != 't' || lpBuf[7] != ' ')
		return false;

	WAVHeader = (WAVH *)(lpBuf + 8);
	lpBuf += 8 + sizeof(WAVH);

	if (lpBuf[0] != 'd' || lpBuf[1] != 'a' || lpBuf[2] != 't' || lpBuf[3] != 'a')
		return false;
	lpBuf += 4;

	dwSize = READ_LE_UINT32(lpBuf);
	lpBuf += 4;

	b16bit = (WAVHeader->nBitsPerSample == 16);
	bStereo = (WAVHeader->nChannels == 2);
	dwFreq = WAVHeader->nSamplesPerSec;

	/* Setta le strutture necessarie per la creazione di un secondary buffer
	   Attiviamo inoltre il controllo del volume, in modo da poter abbassare
	   e alzare il volume della musica indipendentemente da quello generale.
	   Proviamo a buttarlo in sound ram. */
	pcmwf.wBitsPerSample = (b16bit ? 16 : 8);
	pcmwf.wf.wFormatTag = WAVE_FORMAT_PCM;
	pcmwf.wf.nChannels = (bStereo ? 2 : 1);
	pcmwf.wf.nSamplesPerSec = dwFreq;
	pcmwf.wf.nBlockAlign = (pcmwf.wBitsPerSample / 8) * pcmwf.wf.nChannels;
	pcmwf.wf.nAvgBytesPerSec = (uint32)pcmwf.wf.nBlockAlign * (uint32)pcmwf.wf.nSamplesPerSec;

	dsbdesc.dwSize = sizeof(dsbdesc);
	dsbdesc.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRLPOSITIONNOTIFY;
	dsbdesc.dwBufferBytes = dwSize;
	dsbdesc.lpwfxFormat = (LPWAVEFORMATEX)&pcmwf;

	if ((err = lpDS->CreateSoundBuffer(&dsbdesc, &lpDSBuffer, NULL)) != DS_OK) {
		wsprintf(errbuf, "Error creating the secondary buffer (%lx)", err);
		MessageBox(hwnd, errbuf, "FPSFX::FPSFX()", MB_OK);
		return false;
	}

	// Riempie il buffer
	if ((err = lpDSBuffer->Lock(0, dwSize, &lpLock, (uint32 *)&dwHi, NULL, NULL, 0)) != DS_OK) {
		MessageBox(hwnd, "Cannot lock sfx buffer!", "FPSFX::LoadFile()", MB_OK);
		return false;
	}

	/* Decomprime i dati dello stream direttamente dentro il buffer lockato */
	copyMemory(lpLock, lpBuf, dwSize);

	/* Unlocka il buffer */
	lpDSBuffer->Unlock(lpLock, dwSize, NULL, NULL);

	/* Setta volume iniziale */
	SetVolume(lastVolume);

	bFileLoaded = true;
#endif
	return true;
}


/****************************************************************************\
*
* Function:     bool LoadFile(LPSTR lpszFileName, uint32 dwCodec=FPCODEC_RAW);
*
* Description:  Apre un file di effetto sonoro e lo carica.
*
* Input:        LPSTR lpszFile          Nome del file di sfx da aprire
*               uint32 dwCodec           CODEC da utilizzare per decomprimere
*                                       i campioni sonori
*
* Return:       true se tutto OK, false in caso di errore
*
\****************************************************************************/

bool FPSFX::LoadVoiceFromVDB(Common::File &vdbFP) {
#ifdef REFACTOR_ME
	uint32 dwSize;
	static PCMWAVEFORMAT pcmwf;
	static DSBUFFERDESC dsbdesc;
	byte *lpTempBuffer;
	static HRESULT err;
	uint32 dwHi;
	void *lpBuf;
	static char errbuf[128];

	if (!bSoundSupported)
		return true;

	b16bit = true;
	bStereo = false;
	bIsVoice = true;

// fread(&dwSize,1,4,vdbFP);
// fread(&dwFreq,1,4,vdbFP);
	ReadFile(vdbFP, &dwSize, 4, &dwHi, NULL);
	ReadFile(vdbFP, &dwFreq, 4, &dwHi, NULL);

	dwSize *= 4;

	/* Setta le strutture necessarie per la creazione di un secondary buffer
	   Attiviamo inoltre il controllo del volume, in modo da poter abbassare
	   e alzare il volume della musica indipendentemente da quello generale.
	   Proviamo a buttarlo in sound ram. */
	pcmwf.wBitsPerSample = (b16bit ? 16 : 8);
	pcmwf.wf.wFormatTag = WAVE_FORMAT_PCM;
	pcmwf.wf.nChannels = (bStereo ? 2 : 1);
	pcmwf.wf.nSamplesPerSec = dwFreq;
	pcmwf.wf.nBlockAlign = (pcmwf.wBitsPerSample / 8) * pcmwf.wf.nChannels;
	pcmwf.wf.nAvgBytesPerSec = (uint32)pcmwf.wf.nBlockAlign * (uint32)pcmwf.wf.nSamplesPerSec;

	dsbdesc.dwSize = sizeof(dsbdesc);
	dsbdesc.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRLPOSITIONNOTIFY;
	dsbdesc.dwBufferBytes = dwSize;
	dsbdesc.lpwfxFormat = (LPWAVEFORMATEX)&pcmwf;

	if ((err = lpDS->CreateSoundBuffer(&dsbdesc, &lpDSBuffer, NULL)) != DS_OK) {
		wsprintf(errbuf, "Error creating the secondary buffer (%lx)", err);
		MessageBox(hwnd, errbuf, "FPSFX::FPSFX()", MB_OK);
		return false;
	}

	/* Alloca un buffer temporaneo */
	lpTempBuffer = (byte *)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, dwSize);
	if (lpTempBuffer == NULL)
		return false;

	lpCodec = new CODECADPCMMONO(bLoop, lpTempBuffer);

	/* Riempie il buffer */
	if ((err = lpDSBuffer->Lock(0, dwSize, &lpBuf, (uint32 *)&dwHi, NULL, NULL, 0)) != DS_OK) {
		MessageBox(hwnd, "Cannot lock sfx buffer!", "FPSFX::LoadFile()", MB_OK);
		return false;
	}

	/* Decomprime i dati dello stream direttamente dentro il buffer lockato */
	lpCodec->Decompress(vdbFP, lpBuf, dwSize);

	/* Unlocka il buffer */
	lpDSBuffer->Unlock(lpBuf, dwSize, NULL, NULL);

	delete lpCodec;

	/* Crea il notify per avvertire quando raggiungiamo la fine della voce */
	err = lpDSBuffer->QueryInterface(IID_IDirectSoundNotify, (void **)&lpDSNotify);
	if (FAILED(err)) {
		wsprintf(errbuf, "Error creating notify object! (%lx)", err);
		MessageBox(hwnd, errbuf, "FPSFX::LoadVoiceFromVDB()", MB_OK);
		return false;
	}

	hEndOfBuffer = CreateEvent(NULL, false, false, NULL);

	dspnHot[0].dwOffset = DSBPN_OFFSETSTOP;
	dspnHot[0].hEventNotify = hEndOfBuffer;

	lpDSNotify->SetNotificationPositions(1, dspnHot);

	/* Tutto a posto, possiamo uscire */
	bFileLoaded = true;
	SetVolume(62);

#endif
	return true;
}


bool FPSFX::LoadFile(const char *lpszFileName, uint32 dwCodec) {
#ifdef REFACTOR_ME
	static PCMWAVEFORMAT pcmwf;
	static DSBUFFERDESC dsbdesc;
	static HRESULT err;
	static char errbuf[128];
	Common::File file;
	uint32 dwSize;
	byte *lpTempBuffer;
	void *lpBuf;
	uint32 dwHi;
	struct {
		char id[4];
		int freq;
		int nChan;
	} ADPHead;

	if (!bSoundSupported)
		return true;

	/* Apre il file di stream in lettura */
	if (!file.open(lpszFileName)) {
		warning("FPSFX::LoadFile() : Cannot open sfx file!");
		return false;
	}

	/* Leggiamo l'header */
	file.read(ADPHead.id, 4);
	ADPHead.freq = file.readUint32LE();
	ADPHead.nChan = file.readUint32LE();

	if (ADPHead.id[0] != 'A' || ADPHead.id[1] != 'D' || ADPHead.id[2] != 'P' || ADPHead.id[3] != 0x10) {
		warning("FPSFX::LoadFile() : Invalid ADP header!");
		file.close();
		return false;
	}

	b16bit = true;
	bStereo = (ADPHead.nChan == 2);
	dwFreq = ADPHead.freq;

	/* Si salva la lunghezza dello stream */
	dwSize = file.size() - 12 /*sizeof(ADPHead)*/;
	file.seek(0);

	if (dwCodec == FPCODEC_ADPCM)
		dwSize *= 4;

	/* Setta le strutture necessarie per la creazione di un secondary buffer
	   Attiviamo inoltre il controllo del volume, in modo da poter abbassare
	   e alzare il volume della musica indipendentemente da quello generale.
	   Proviamo a buttarlo in sound ram. */
	pcmwf.wBitsPerSample = (b16bit ? 16 : 8);
	pcmwf.wf.wFormatTag = WAVE_FORMAT_PCM;
	pcmwf.wf.nChannels = (bStereo ? 2 : 1);
	pcmwf.wf.nSamplesPerSec = dwFreq;
	pcmwf.wf.nBlockAlign = (pcmwf.wBitsPerSample / 8) * pcmwf.wf.nChannels;
	pcmwf.wf.nAvgBytesPerSec = (uint32)pcmwf.wf.nBlockAlign * (uint32)pcmwf.wf.nSamplesPerSec;

	dsbdesc.dwSize = sizeof(dsbdesc);
	dsbdesc.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRLPOSITIONNOTIFY;
	dsbdesc.dwBufferBytes = dwSize;
	dsbdesc.lpwfxFormat = (LPWAVEFORMATEX) & pcmwf;

	if ((err = lpDS->CreateSoundBuffer(&dsbdesc, &lpDSBuffer, NULL)) != DS_OK) {
		wsprintf(errbuf, "Error creating the secondary buffer (%lx)", err);
		MessageBox(hwnd, errbuf, "FPSFX::FPSFX()", MB_OK);
		return false;
	}

	/* Alloca un buffer temporaneo */
	lpTempBuffer = (byte *)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, dwSize);
	if (lpTempBuffer == NULL)
		return false;

	switch (dwCodec) {
	case FPCODEC_RAW:
		lpCodec = new CODECRAW(bLoop);
		break;

	case FPCODEC_ADPCM:
		if (bStereo)
			lpCodec = new CODECADPCMSTEREO(bLoop, lpTempBuffer);
		else
			lpCodec = new CODECADPCMMONO(bLoop, lpTempBuffer);
		break;

	default:
		return false;
		G
	}

	/* Riempie il buffer */
	if ((err = lpDSBuffer->Lock(0, dwSize, &lpBuf, (uint32 *)&dwHi, NULL, NULL, 0)) != DS_OK) {
		MessageBox(hwnd, "Cannot lock sfx buffer!", "FPSFX::LoadFile()", MB_OK);
		return false;
	}

	/* Decomprime i dati dello stream direttamente dentro il buffer lockato */
	lpCodec->Decompress(file, lpBuf, dwSize);

	/* Unlocka il buffer */
	lpDSBuffer->Unlock(lpBuf, dwSize, NULL, NULL);

	delete lpCodec;
	file.close();

	/* Tutto a posto, possiamo uscire */
	bFileLoaded = true;
#endif
	return true;
}


/****************************************************************************\
*
* Function:     bool Play();
*
* Description:  Suona lo sfx caricato.
*
* Return:       true se tutto OK, false in caso di errore.
*
\****************************************************************************/

bool FPSFX::Play() {
#ifdef REFACTOR_ME
	if (bFileLoaded) {
		if (hEndOfBuffer != CORO_INVALID_PID_VALUE)
			ResetEvent(hEndOfBuffer);

		lpDSBuffer->SetCurrentPosition(0);
		bIsPlaying = true;

		if (!bPaused) {
			lpDSBuffer->Play(0, 0, (bLoop ? DSBPLAY_LOOPING : 0));
		}
	}
#endif
	return true;
}


/****************************************************************************\
*
* Function:     bool Stop(void);
*
* Description:  Ferma il play dello sfx.
*
* Return:       true se tutto OK, false in caso di errore.
*
\****************************************************************************/

bool FPSFX::Stop(void) {
#ifdef REFACTOR_ME
	if (bFileLoaded) {
		if (bPaused || bIsPlaying) {
			lpDSBuffer->Stop();
		}

		bIsPlaying = false;
		bPaused = false;
	}
#endif
	return true;
}



/****************************************************************************\
*
* Function:     bool SetLoop(bool bLoop);
*
* Description:  Attiva o disattiva il loop dello sfx.
*
* Input:        bool bLoop              true per attivare il loop, false per
*                                       disattivarlo
*
* Note:         Il loop deve essere attivato PRIMA di eseguire il play
*               dello sfx. Qualsiasi modifica effettuata durante il play
*               non avra' effetto fino a che lo sfx non viene fermato,
*               e poi rimesso in play.
*
\****************************************************************************/

void FPSFX::SetLoop(bool bLop) {
	bLoop = bLop;
}

void FPSFX::Pause(bool bPause) {
#ifdef REFACTOR_ME
	if (bFileLoaded) {
		if (bPause && bIsPlaying) {
			lpDSBuffer->Stop();
		} else if (!bPause && bPaused) {
			if (bIsPlaying && bLoop)
				lpDSBuffer->Play(0, 0, (bLoop ? DSBPLAY_LOOPING : 0));
		}

		// Trucchetto per risettare il volume secondo le
		// possibili nuove configurazioni sonore
		SetVolume(lastVolume);
		bPaused = bPause;
	}
#endif
}


/****************************************************************************\
*
* Function:     void SetVolume(int dwVolume);
*
* Description:  Cambia il volume dello sfx
*
* Input:        int dwVolume            Volume da settare (0-63)
*
\****************************************************************************/

void FPSFX::SetVolume(int dwVolume) {
	if (dwVolume > 63) dwVolume = 63;
	if (dwVolume < 0) dwVolume = 0;

	lastVolume = dwVolume;

	if (bIsVoice) {
		if (!GLOBALS._bCfgDubbing) dwVolume = 0;
		else {
			dwVolume -= (10 - GLOBALS._nCfgDubbingVolume) * 2;
			if (dwVolume < 0) dwVolume = 0;
		}
	} else {
		if (!GLOBALS._bCfgSFX) dwVolume = 0;
		else {
			dwVolume -= (10 - GLOBALS._nCfgSFXVolume) * 2;
			if (dwVolume < 0) dwVolume = 0;
		}
	}
#ifdef REFACTOR_ME
	if (bFileLoaded)
		lpDSBuffer->SetVolume(dwVolume * (DSBVOLUME_MAX - DSBVOLUME_MIN) / 64 + DSBVOLUME_MIN);
#endif
}



/****************************************************************************\
*
* Function:     void GetVolume(int *lpdwVolume);
*
* Description:  Chiede il volume dello sfx
*
* Input:        int *lpdwVolume        Variabile in cui verra' inserito
*                                       il volume corrente
*
\****************************************************************************/

void FPSFX::GetVolume(int *lpdwVolume) {
#ifdef REFACTOR_ME
	if (bFileLoaded)
		lpDSBuffer->GetVolume((uint32 *)lpdwVolume);

	*lpdwVolume -= (DSBVOLUME_MIN);
	*lpdwVolume *= 64;
	*lpdwVolume /= (DSBVOLUME_MAX - DSBVOLUME_MIN);
#endif
}


/****************************************************************************\
*       Metodi di FPSTREAM
\****************************************************************************/

/****************************************************************************\
*
* Function:     FPSTREAM(LPDIRECTSOUND lpDS, bool bSoundOn);
*
* Description:  Costruttore di default. *NON* bisogna dichiarare direttamente
*               un oggetto, ma crearlo piuttosto tramite FPSOUND::CreateStream()
*
\****************************************************************************/

FPSTREAM::FPSTREAM(LPDIRECTSOUND LPDS, HWND hWnd, bool bSoundOn) {
#ifdef REFACTOR_ME
	//hwnd=hWnd;
	lpDS = LPDS;
	bSoundSupported = bSoundOn;
	bFileLoaded = false;
	bIsPlaying = false;
	bPaused = false;
	bSyncExit = false;
	lpDSBuffer = NULL;
	lpDSNotify = NULL;
	hHot1 = hHot2 = hHot3 = hPlayThread_PlayFast = hPlayThread_PlayNormal = NULL;
#endif
}

bool FPSTREAM::CreateBuffer(int nBufSize) {
#ifdef REFACTOR_ME
	static PCMWAVEFORMAT pcmwf;
	static DSBUFFERDESC dsbdesc;
	static HRESULT err;
	static char errbuf[128];

	if (bSoundSupported == false)
		return true;

	/* Setta le strutture necessarie per la creazione di un secondary buffer
	   per lo stream lungo esattamente 1 secondo di musica. Attiviamo inoltre
	   il controllo del volume, in modo da poter abbassare e alzare il volume
	   della musica indipendentemente da quello generale. Ovviamente si tratta
	   di un buffer in RAM */
	pcmwf.wBitsPerSample = 16;
	pcmwf.wf.wFormatTag = WAVE_FORMAT_PCM;
	pcmwf.wf.nChannels = 2;
	pcmwf.wf.nSamplesPerSec = 44100;
	pcmwf.wf.nBlockAlign = (pcmwf.wBitsPerSample / 8) * pcmwf.wf.nChannels;
	pcmwf.wf.nAvgBytesPerSec = (uint32)pcmwf.wf.nBlockAlign * (uint32)pcmwf.wf.nSamplesPerSec;

	dsbdesc.dwSize = sizeof(dsbdesc);
	dsbdesc.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRLPOSITIONNOTIFY;
	dwBufferSize = dsbdesc.dwBufferBytes = (((uint32)(pcmwf.wf.nAvgBytesPerSec * nBufSize) / 1000 + 31) / 32) * 32;
	dsbdesc.lpwfxFormat = (LPWAVEFORMATEX) & pcmwf;

	if ((err = lpDS->CreateSoundBuffer(&dsbdesc, &lpDSBuffer, NULL)) != DS_OK) {
		wsprintf(errbuf, "Error creating the secondary buffer (%lx)", err);
		MessageBox(hwnd, errbuf, "FPSTREAM::FPSTREAM()", MB_OK);
		bSoundSupported = false;
		return false;
	}
	SetVolume(63);


	/* Crea il notify per avvertire quando vengono raggiunte le posizioni chiave
	   all'interno dello stream. Le posizioni chiave si trovano rispettivamente
	   subito dopo l'inizio e subito dopo la meta' del buffer */
	err = lpDSBuffer->QueryInterface(IID_IDirectSoundNotify, (void **)&lpDSNotify);

	if (FAILED(err)) {
		wsprintf(errbuf, "Error creating notify object! (%lx)", err);
		MessageBox(hwnd, errbuf, "FPSTREAM::FPSTREAM()", MB_OK);
		bSoundSupported = false;
		return false;
	}

	hHot1 = CreateEvent(NULL, false, false, NULL);
	hHot2 = CreateEvent(NULL, false, false, NULL);
	hHot3 = CreateEvent(NULL, false, false, NULL);
	hPlayThread_PlayFast = CreateEvent(NULL, false, false, NULL);

	dspnHot[0].dwOffset = 32;
	dspnHot[0].hEventNotify = hHot1;

	dspnHot[1].dwOffset = dwBufferSize / 2 + 32;
	dspnHot[1].hEventNotify = hHot2;

	dspnHot[2].dwOffset = dwBufferSize - 32;   //DSBPN_OFFSETSTOP;
	dspnHot[2].hEventNotify = hHot3;

	lpDSNotify->SetNotificationPositions(3, dspnHot);
#endif
	return true;
}


/****************************************************************************\
*
* Function:     ~FPSTREAM();
*
* Description:  Distruttore di default. Richiama anche la CloseFile() se ce
*               ne e' bisogno.
*
\****************************************************************************/

FPSTREAM::~FPSTREAM() {
#ifdef REFACTOR_ME

	if (!bSoundSupported)
		return;

	if (bIsPlaying)
		Stop();

	if (bFileLoaded)
		UnloadFile();

	if (hHot1) {
		CloseHandle(hHot1);
		hHot1 = NULL;
	}
	if (hHot2) {
		CloseHandle(hHot2);
		hHot2 = NULL;
	}
	if (hHot3) {
		CloseHandle(hHot3);
		hHot3 = NULL;
	}
	if (hPlayThread_PlayFast) {
		CloseHandle(hPlayThread_PlayFast);
		hPlayThread_PlayFast = NULL;
	}
	if (hPlayThread_PlayNormal) {
		CloseHandle(hPlayThread_PlayNormal);
		hPlayThread_PlayNormal = NULL;
	}

	SyncToPlay = NULL;

	RELEASE(lpDSNotify);
	RELEASE(lpDSBuffer);
#endif
}


/****************************************************************************\
*
* Function:     Release();
*
* Description:  Rilascia la memoria dell'oggetto. Deve essere richiamata quando
*               l'oggetto non serve piu' e **SOLO SE** l'oggetto e' stato
*               creato con la FPSOUND::CreateStream().
*
* Note:         Eventuali puntatori all'oggetto non sono piu' validi dopo
*               questa chiamata.
*
\****************************************************************************/

void FPSTREAM::Release() {
	delete this;
//	return NULL;
}


/****************************************************************************\
*
* Function:     bool LoadFile(LPSTREAM lpszFileName, uint32 dwCodec=FPCODEC_RAW);
*
* Description:  Apre un file di stream.
*
* Input:        LPSTR lpszFile          Nome del file di stream da aprire
*               uint32 dwCodec           CODEC da utilizzare per decomprimere
*                                       i campioni sonori
*
* Return:       true se tutto OK, false in caso di errore
*
\****************************************************************************/

bool FPSTREAM::LoadFile(const char *lpszFileName, uint32 dwCodType, int nBufSize) {
#ifdef REFACTOR_ME
	HRESULT err;
	void *lpBuf;
	uint32 dwHi;

	if (!bSoundSupported)
		return true;

	/* Si salva il tipo di codec */
	dwCodec = dwCodType;

	/* Crea il buffer */
	if (!CreateBuffer(nBufSize))
		return true;

	/* Apre il file di stream in lettura */
	if (!_file.open(lpszFileName))
		//MessageBox(hwnd,"Cannot open stream file!","FPSTREAM::LoadFile()", MB_OK);
		return false;
}

/* Si salva la lunghezza dello stream */
dwSize = _file.size();
_file.seek(0);

/* Tutto a posto, possiamo uscire */
bFileLoaded = true;
bIsPlaying = false;
bPaused = false;
#endif
return true;
}


/****************************************************************************\
*
* Function:     UnloadFile();
*
* Description:  Chiude un file di stream eventualmente aperto. E' necessario
*               richiamare questa funzione per disallocare la memoria
*               occupata dallo stream.
*
* Return:       Il distruttore della classe per sicurezza richiama la
*               UnloadFile() se non e' stata richiamata esplicitamente.
*
\****************************************************************************/

bool FPSTREAM::UnloadFile() {
#ifdef REFACTOR_ME

	if (!bSoundSupported || !bFileLoaded)
		return true;

	/* Chiude gli handle del file di stream */
	_file.close();

	RELEASE(lpDSNotify);
	RELEASE(lpDSBuffer);

	/* Si ricorda che non c'e' piu' nessun file in memoria */
	bFileLoaded = false;
#endif
	return true;
}

/****************************************************************************\
*
* Function:     bool Play();
*
* Description:  Suona lo stream caricato.
*
* Return:       true se tutto OK, false in caso di errore.
*
\****************************************************************************/

void FPSTREAM::Prefetch(void) {
#ifdef REFACTOR_ME
	uint32 dwId;
	void *lpBuf;
	uint32 dwHi;
	HRESULT err;

	if (!bSoundSupported || !bFileLoaded)
		return;

	/* Alloca un buffer temporaneo */
	lpTempBuffer = (byte *)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, dwBufferSize / 2);
	if (lpTempBuffer == NULL)
		return;

	switch (dwCodec) {
	case FPCODEC_RAW:
		lpCodec = new CODECRAW(bLoop);
		break;

	case FPCODEC_ADPCM:
		lpCodec = new CODECADPCMSTEREO(bLoop);
		break;

	default:
		return;
	}

	/* Posiziona lo stream file all'inizio */
	_file.seek(0);

	/* Riempie il buffer per avere i dati gia' pronti */
	if ((err = lpDSBuffer->Lock(0, dwBufferSize / 2, &lpBuf, (uint32 *)&dwHi, NULL, NULL, 0)) != DS_OK) {
		MessageBox(hwnd, "Cannot lock stream buffer!", "soundLoadStream()", MB_OK);
		return;
	}

	/* Decomprime i dati dello stream direttamente dentro il buffer lockato */
	lpCodec->Decompress(hFile, lpBuf, dwBufferSize / 2);

	/* Unlocka il buffer */
	lpDSBuffer->Unlock(lpBuf, dwBufferSize / 2, NULL, NULL);

	/* Crea il thread che fa il play dello stream */
	hThreadEnd = CreateEvent(NULL, false, false, NULL);
	hPlayThread = CreateThread(NULL, 10240, (LPTHREAD_START_ROUTINE)PlayThread, (void *)this, 0, &dwId);
	SetThreadPriority(hPlayThread, THREAD_PRIORITY_HIGHEST);

	/* Start il play del buffer DirectSound */
	lpDSBuffer->SetCurrentPosition(0);
	bIsPlaying = true;

	dspnHot[0].dwOffset = 32;
	dspnHot[0].hEventNotify = hHot1;

	dspnHot[1].dwOffset = dwBufferSize / 2 + 32;
	dspnHot[1].hEventNotify = hHot2;

	dspnHot[2].dwOffset = dwBufferSize - 32;   //DSBPN_OFFSETSTOP;
	dspnHot[2].hEventNotify = hHot3;

	if (FAILED(lpDSNotify->SetNotificationPositions(3, dspnHot))) {
		int a = 1;
	}
#endif
}

void FPSTREAM::PlayFast(void) {
#ifdef REFACTOR_ME
	dspnHot[0].dwOffset = 32;
	dspnHot[0].hEventNotify = hHot1;

	dspnHot[1].dwOffset = dwBufferSize / 2 + 32;
	dspnHot[1].hEventNotify = hHot2;

	dspnHot[2].dwOffset = dwBufferSize - 32; //DSBPN_OFFSETSTOP;
	dspnHot[2].hEventNotify = hHot3;

	lpDSBuffer->Stop();

	if (FAILED(lpDSNotify->SetNotificationPositions(3, dspnHot))) {
		warning("PlayFast SNP failed!");
	}

	if (FAILED(lpDSBuffer->Play(0, 0, DSBPLAY_LOOPING))) {
		warning("PlayFast failed!\n");
	}
#endif
}

bool FPSTREAM::Play() {
#ifdef REFACTOR_ME
	uint32 dwId;
	void *lpBuf;
	uint32 dwHi;
	HRESULT err;

	if (!bSoundSupported || !bFileLoaded)
		return false;

	/* Alloca un buffer temporaneo */
	lpTempBuffer = (byte *)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, dwBufferSize / 2);
	if (lpTempBuffer == NULL)
		return false;

	switch (dwCodec) {
	case FPCODEC_RAW:
		lpCodec = new CODECRAW(bLoop);
		break;

	case FPCODEC_ADPCM:
		lpCodec = new CODECADPCMSTEREO(bLoop);
		break;

	default:
		return false;
	}

	/* Posiziona lo stream file all'inizio */
	_file.seek(0);
	lpDSBuffer->Stop();
	lpDSBuffer->SetCurrentPosition(0);

	/* Riempie il buffer per avere i dati gia' pronti */
	if ((err = lpDSBuffer->Lock(0, dwBufferSize / 2, &lpBuf, (uint32 *)&dwHi, NULL, NULL, 0)) != DS_OK) {
		error("Cannot lock stream buffer!", "soundLoadStream()");
	}

	/* Decomprime i dati dello stream direttamente dentro il buffer lockato */
	lpCodec->Decompress(hFile, lpBuf, dwBufferSize / 2);

	/* Unlocka il buffer */
	lpDSBuffer->Unlock(lpBuf, dwBufferSize / 2, NULL, NULL);

	/* Crea il thread che fa il play dello stream */
	hThreadEnd = CreateEvent(NULL, false, false, NULL);
	hPlayThread = CreateThread(NULL, 10240, (LPTHREAD_START_ROUTINE)PlayThread, (void *)this, 0, &dwId);
	SetThreadPriority(hPlayThread, THREAD_PRIORITY_HIGHEST);

	SetEvent(hPlayThread_PlayFast);

#if 0
	/* Start il play del buffer DirectSound */
	lpDSBuffer->SetCurrentPosition(0);

	dspnHot[0].dwOffset = 32;
	dspnHot[0].hEventNotify = hHot1;

	dspnHot[1].dwOffset = dwBufferSize / 2 + 32;
	dspnHot[1].hEventNotify = hHot2;

	dspnHot[2].dwOffset = dwBufferSize - 32; //DSBPN_OFFSETSTOP;
	dspnHot[2].hEventNotify = hHot3;

	if (FAILED(lpDSNotify->SetNotificationPositions(3, dspnHot))) {
		int a = 1;
	}


	lpDSBuffer->Play(0, 0, DSBPLAY_LOOPING);
#endif

	bIsPlaying = true;
#endif
	return true;
}


/****************************************************************************\
*
* Function:     bool Stop(bool bSync);
*
* Description:  Ferma il play dello stream.
*
* Return:       true se tutto OK, false in caso di errore.
*
\****************************************************************************/

bool FPSTREAM::Stop(bool bSync) {
#ifdef REFACTOR_ME

	if (!bSoundSupported)
		return true;

	if (!bFileLoaded)
		return false;

	if (!bIsPlaying)
		return false;

	if (bSync) {
//	 bSyncExit = true;
//	 lpDSBuffer->Stop();
//	 lpDSBuffer->Play(0, 0, 0);
		return true;
	} else {
		/* Ferma il buffer DirectSound */
		lpDSBuffer->Stop();

		/* Avverte il thread che deve uscire e aspetta che si chiuda */
		SetEvent(hThreadEnd);
		WaitForSingleObject(hPlayThread, CORO_INFINITE);

		/* Chiude l'handle del thread e disalloca la memoria temporanea */
		CloseHandle(hPlayThread);
		CloseHandle(hThreadEnd);
		GlobalFree(lpTempBuffer);

		/* Disalloca e chiude il CODEC */
		delete lpCodec;

		bIsPlaying = false;
		bPaused = false;
	}
#endif
	return true;
}

void FPSTREAM::WaitForSync(FPSTREAM *toplay) {
#ifdef REFACTOR_ME
	if (!bSoundSupported)
		return;

	if (!bFileLoaded)
		return;

	if (!bIsPlaying)
		return;

	SyncToPlay = toplay;
	bSyncExit = true;

	char buf[1024];
	sprintf(buf, "Wait for sync: %x (SyncToPlay: [%x]=%x, SyncExit: [%x]=%d) MyThread: 0x%x\n",
	        this->lpDSBuffer, &this->SyncToPlay, SyncToPlay, &bSyncExit, bSyncExit, GetCurrentThreadId());
	warning(buf);

	WaitForSingleObject(hPlayThread, CORO_INFINITE);

	/* Chiude l'handle del thread e disalloca la memoria temporanea */
	CloseHandle(hPlayThread);
	CloseHandle(hThreadEnd);
	GlobalFree(lpTempBuffer);

	/* Disalloca e chiude il CODEC */
	delete lpCodec;
#endif
	bIsPlaying = false;
}

/****************************************************************************\
*
* Function:     void FPSTREAM::PlayThread();
*
* Description:  Thread che si occupa del play dello stream
*
\****************************************************************************/

void FPSTREAM::PlayThread(FPSTREAM *This) {
#ifdef REFACTOR_ME
	byte *lpLockBuf;
	uint32 dwResult;
	byte *lpLockBuf2;
	uint32 dwResult2;
	bool cicla = true;
	uint32 countEnd;
	bool bPrecache;
	char buf[1024];

	/* Eventi che segnalano quando bisogna eseguire qualcosa */
	HANDLE hList[5] = { This->hThreadEnd, This->hHot1, This->hHot2, This->hHot3, This->hPlayThread_PlayFast };

	bPrecache = true;
	countEnd = 0;
	while (cicla) {
		if (This->lpCodec->EndOfStream() && This->lpCodec->bLoop == false) {
			countEnd++;
			if (countEnd == 3)
				break;
		}

		/* Decomprime i dati che stanno per essere scritti dentro il buffer temporaneo */
		if (This->lastVolume == 0)
			ZeroMemory(This->lpTempBuffer, This->dwBufferSize / 2);
		else if (bPrecache)
			This->lpCodec->Decompress(This->_file, This->lpTempBuffer, This->dwBufferSize / 2);

		bPrecache = false;

		/* Attende il set di un evento. Dato che sono tutti in automatic reset,
		  non c'e' bisogno di resettarlo dopo */

//	 uint32 dwBufStatus;
//	 This->lpDSBuffer->GetStatus(&dwBufStatus);


// sprintf(buf, "WFMO: %x (buf status: %x) MyThread: 0x%x\n", This->lpDSBuffer, dwBufStatus, GetCurrentThreadId());
// warning(buf);
		dwResult = WaitForMultipleObjects(5, hList, false, CORO_INFINITE);

		/*  uint32 dwPlay, dwWrite;
		    This->lpDSBuffer->GetCurrentPosition(&dwPlay, &dwWrite);
		    sprintf(buf, "CP Play: %u, Write: %u\n", dwPlay, dwWrite);
		    warning(buf); */

		/* Fa uno switch per stabilire quale evento e' stato settato */
		switch (dwResult - WAIT_OBJECT_0) {
		case 0:
			/* Bisogna uscire dal thread */
			cicla = false;
			break;

		case 1:
			/* Bisogna riempire la seconda meta' del buffer */
//	   if (dwPlay >= This->dspnHot[0].dwOffset && dwPlay <= This->dspnHot[0].dwOffset+1024 )
		{
//		   sprintf(buf, "Prima metà buffer: %x\n", This->lpDSBuffer);
//		   warning(buf);
			This->lpDSBuffer->Lock(This->dwBufferSize / 2, This->dwBufferSize / 2, (void **)&lpLockBuf, &dwResult, (void **)&lpLockBuf2, &dwResult2, 0);
			//     sprintf(buf, "LockedBuf: dwResult=%x, dwBufferSize/2=%x, lpLockBuf2=%x, dwResult2=%x\n", dwResult, This->dwBufferSize/2, lpLockBuf2, dwResult2);
			//     warning(buf);
			copyMemory(lpLockBuf, This->lpTempBuffer, This->dwBufferSize / 2);
			This->lpDSBuffer->Unlock(lpLockBuf, This->dwBufferSize / 2, lpLockBuf2, 0);
			bPrecache = true;
		}
		break;

		case 2:
			/* Bisogna riempire la prima meta' del buffer */
//		 if (dwPlay >= This->dspnHot[1].dwOffset && dwPlay <= This->dspnHot[1].dwOffset+1024 )
		{
//			 sprintf(buf, "Seconda metà buffer: %x\n", This->lpDSBuffer);
//			 warning(buf);
			This->lpDSBuffer->Lock(0, This->dwBufferSize / 2, (void **)&lpLockBuf, &dwResult, NULL, NULL, 0);
			copyMemory(lpLockBuf, This->lpTempBuffer, This->dwBufferSize / 2);
			This->lpDSBuffer->Unlock(lpLockBuf, This->dwBufferSize / 2, NULL, NULL);
			bPrecache = true;
		}
		break;

		case 3: {
//		 sprintf(buf, "End of buffer %x (SyncToPlay [%x]=%x, SyncExit: [%x]=%d)\n", This->lpDSBuffer, &This->SyncToPlay, This->SyncToPlay, &This->bSyncExit, This->bSyncExit);
//		 warning(buf);
			if (This->bSyncExit) {
//			 sprintf(buf, "Go with sync (Buffer: %x) MyThread: %x!\n", This->SyncToPlay->lpDSBuffer, GetCurrentThreadId());
//			 warning(buf);
				//This->SyncToPlay->PlayFast();
				SetEvent(This->SyncToPlay->hPlayThread_PlayFast);
				// Transfer immediatly control to the other threads
				Sleep(0);
				This->bSyncExit = false;
				cicla = false;
				break;
			}
		}
		break;

		case 4:
			This->PlayFast();
			break;
		}
	}

	/* Ferma il buffer DirectSound */
// sprintf(buf, "Exiting thread. Buffer = %x, MyThread = 0x%x\n", This->lpDSBuffer, GetCurrentThreadId());
// warning(buf);
	This->lpDSBuffer->Stop();

	ExitThread(0);
#endif
}


/****************************************************************************\
*
* Function:     bool SetLoop(bool bLoop);
*
* Description:  Attiva o disattiva il loop dello stream.
*
* Input:        bool bLoop              true per attivare il loop, false per
*                                       disattivarlo
*
* Note:         Il loop deve essere attivato PRIMA di eseguire il play
*               dello stream. Qualsiasi modifica effettuata durante il play
*               non avra' effetto fino a che lo stream non viene fermato,
*               e poi rimesso in play.
*
\****************************************************************************/

void FPSTREAM::SetLoop(bool loop) {
	bLoop = loop;
}


void FPSTREAM::Pause(bool bPause) {
#ifdef REFACTOR_ME

	if (bFileLoaded) {
		if (bPause && bIsPlaying) {
			lpDSBuffer->Stop();
			bIsPlaying = false;
			bPaused = true;
		} else if (!bPause && bPaused) {
			dspnHot[0].dwOffset = 32;
			dspnHot[0].hEventNotify = hHot1;

			dspnHot[1].dwOffset = dwBufferSize / 2 + 32;
			dspnHot[1].hEventNotify = hHot2;

			dspnHot[2].dwOffset = dwBufferSize - 32; //DSBPN_OFFSETSTOP;
			dspnHot[2].hEventNotify = hHot3;

			if (FAILED(lpDSNotify->SetNotificationPositions(3, dspnHot))) {
				int a = 1;
			}

			lpDSBuffer->Play(0, 0, bLoop);
			bIsPlaying = true;
			bPaused = false;

			// Trucchetto per risettare il volume secondo le
			// possibili nuove configurazioni sonore
			SetVolume(lastVolume);
		}
	}
#endif
}


/****************************************************************************\
*
* Function:     void SetVolume(int dwVolume);
*
* Description:  Cambia il volume dello stream
*
* Input:        int dwVolume            Volume da settare (0-63)
*
\****************************************************************************/

void FPSTREAM::SetVolume(int dwVolume) {
#ifdef REFACTOR_ME
	if (dwVolume > 63) dwVolume = 63;
	if (dwVolume < 0) dwVolume = 0;

	lastVolume = dwVolume;

	if (!GLOBALS.bCfgMusic) dwVolume = 0;
	else {
		dwVolume -= (10 - GLOBALS.nCfgMusicVolume) * 2;
		if (dwVolume < 0) dwVolume = 0;
	}

	if (lpDSBuffer)
		lpDSBuffer->SetVolume(dwVolume * (DSBVOLUME_MAX - DSBVOLUME_MIN) / 64 + DSBVOLUME_MIN);
#endif
}



/****************************************************************************\
*
* Function:     void GetVolume(int *lpdwVolume);
*
* Description:  Chiede il volume dello stream
*
* Input:        int *lpdwVolume        Variabile in cui verra' inserito
*                                       il volume corrente
*
\****************************************************************************/

void FPSTREAM::GetVolume(int *lpdwVolume) {
#ifdef REFACTOR_ME
	if (lpDSBuffer)
		lpDSBuffer->GetVolume((uint32 *)lpdwVolume);
	*lpdwVolume -= (DSBVOLUME_MIN);
	*lpdwVolume *= 64;
	*lpdwVolume /= (DSBVOLUME_MAX - DSBVOLUME_MIN);
#endif
}

} // End of namespace Tony
