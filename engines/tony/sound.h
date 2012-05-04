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
 *      ^$$.     4$$$$$$$$$F     .$P"     Module:  Sound.CPP............  *
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

#ifndef TONY_SOUND_H
#define TONY_SOUND_H

#include "common/file.h"
#include "tony/gfxcore.h"
#include "tony/loc.h"
#include "tony/utils.h"

namespace Tony {

// Dummy type declarations
typedef void *LPDIRECTSOUND;
typedef void *LPDIRECTSOUNDBUFFER;
typedef uint32 HWND;
struct DSCAPS {
};


class FPSTREAM;
class FPSFX;
class CODEC;

enum CODECS {
	FPCODEC_RAW,
	FPCODEC_ADPCM,
	FPCODEC_WAV
};


/****************************************************************************\
*****************************************************************************
*       class FPSound
*       -------------
* Description: Sound driver per Falling Pumpkins
*****************************************************************************
\****************************************************************************/

class FPSOUND {

private:

	bool bSoundSupported;
	LPDIRECTSOUND lpDS;
	LPDIRECTSOUNDBUFFER lpDSBPrimary;
	DSCAPS dscaps;
	HWND hwnd;

/****************************************************************************\
*       Metodi
\****************************************************************************/

public:

/****************************************************************************\
*
* Function:     FPSOUND::FPSOUND();
*
* Description:  Costruttore di default. Inizializza gli attributi.
*
\****************************************************************************/

	FPSOUND();


/****************************************************************************\
*
* Function:     FPSOUND::~FPSOUND();
*
* Description:  Deinizializza l'oggetto, disallocando la memoria.
*
\****************************************************************************/

	~FPSOUND();


/****************************************************************************\
*
* Function:     bool FPSOUND::Init(HWND hWnd);
*
* Description:  Inizializza l'oggetto, e prepara tutto il necessario per
*               creare stream e effetti sonori.
*
* Input:        HWND hWnd               Handle della finestra principale
*
* Return:       True se tutto OK, FALSE in caso di errore.
*
\****************************************************************************/


	bool Init(/*HWND hWnd*/);


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
* Return:       TRUE se tutto OK, FALSE in caso di errore
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

	bool CreateStream(FPSTREAM **lplpStream);



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
* Return:       TRUE se tutto OK, FALSE in caso di errore
*
* Note:         Vedi le note di CreateStream() 
*
\****************************************************************************/

	bool CreateSfx(FPSFX **lplpSfx);



/****************************************************************************\
*
* Function:     void SetMasterVolume(int dwVolume);
*
* Description:  Setta il volume generale
*
* Input:        int dwVolume          Volume da settare (0-63)
*
\****************************************************************************/

	void SetMasterVolume(int dwVolume);


/****************************************************************************\
*
* Function:     void GetMasterVolume(LPINT lpdwVolume);
*
* Description:  Richiede il volume generale
*
* Input:        LPINT lpdwVolume        Variabile che conterra' il volume (0-63)
*
\****************************************************************************/

	void GetMasterVolume(int *lpdwVolume);
};

class FPSFX {

/****************************************************************************\
*       Attributi
\****************************************************************************/

private:
/*
	HWND hwnd;
	LPDIRECTSOUND lpDS;
  LPDIRECTSOUNDBUFFER lpDSBuffer;       // Buffer DirectSound
*/
	bool bSoundSupported;                 // TRUE se il suono e' attivo
	bool bFileLoaded;                     // TRUE se e' stato aperto un file
	bool bLoop;                           // TRUE se bisogna loopare l'effetto sonoro
	bool bPaused;
	int lastVolume;

	bool bStereo;													// TRUE se ่ stereo
	bool b16bit;													// TRUE se ่ 16 bit
	uint32 dwFreq;													// Frequenza originale di campionamento

//  CODEC* lpCodec;                       // CODEC da utilizzare.
	bool bIsPlaying;                      // TRUE se si sta playando l'effetto sonoro

	bool bIsVoice;

//  LPDIRECTSOUNDNOTIFY lpDSNotify;       // Notify degli hotspot nel buffer
//  DSBPOSITIONNOTIFY dspnHot[2];
  
public:
	HANDLE hEndOfBuffer;

private:


/****************************************************************************\
*       Metodi
\****************************************************************************/

public:

/****************************************************************************\
*
* Function:     FPSFX(LPDIRECTSOUND lpDS, bool bSoundOn);
*
* Description:  Costruttore di default. *NON* bisogna dichiarare direttamente
*               un oggetto, ma crearlo piuttosto tramite FPSOUND::CreateSfx()
*
\****************************************************************************/

  FPSFX(void * /*LPDIRECTSOUND */lpDS, uint32 /*HWND*/ hwnd, bool bSoundOn);


/****************************************************************************\
*
* Function:     ~FPSFX();
*
* Description:  Distruttore di default. Si preoccupa anche di fermare il sound
*								effect eventualmente in esecuzione, e disallocare la memoria 
*								da esso occupata.
*
\****************************************************************************/

  ~FPSFX();


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

  void Release();


/****************************************************************************\
*
* Function:     bool LoadFile(char *lpszFileName, uint32 dwCodec=FPCODEC_RAW);
*
* Description:  Apre un file di effetto sonoro e lo carica.
*
* Input:        char *lpszFile          Nome del file di sfx da aprire
*               uint32 dwCodec           CODEC da utilizzare per decomprimere
*                                       i campioni sonori
*
* Return:       TRUE se tutto OK, FALSE in caso di errore
*
\****************************************************************************/

	bool LoadFile(const char *lpszFileName, uint32 dwCodec = FPCODEC_RAW);
	bool LoadFile(byte *lpBuf, uint32 dwCodec);
	bool LoadVoiceFromVDB(Common::File &vdbFP);


/****************************************************************************\
*
* Function:     bool Play();
*
* Description:  Suona lo sfx caricato.
*
* Return:       TRUE se tutto OK, FALSE in caso di errore.
*
\****************************************************************************/

  bool Play();


/****************************************************************************\
*
* Function:     bool Stop();
*
* Description:  Ferma il play dello sfx.
*
* Return:       TRUE se tutto OK, FALSE in caso di errore.
*
\****************************************************************************/

  bool Stop();


/****************************************************************************\
*
* Function:     void Pause(bool bPause);
*
* Description:  Pause dell'effetto sonoro
*
\****************************************************************************/

  void Pause(bool bPause);


/****************************************************************************\
*
* Function:     bool SetLoop(bool bLoop);
*
* Description:  Attiva o disattiva il loop dello sfx.
*
* Input:        bool bLoop              TRUE per attivare il loop, FALSE per
*                                       disattivarlo
*
* Note:         Il loop deve essere attivato PRIMA di eseguire il play
*               dello sfx. Qualsiasi modifica effettuata durante il play
*               non avra' effetto fino a che lo sfx non viene fermato,
*               e poi rimesso in play.
*
\****************************************************************************/

  void SetLoop(bool bLoop);



/****************************************************************************\
*
* Function:     void SetVolume(int dwVolume);
*
* Description:  Cambia il volume dello sfx
*
* Input:        int dwVolume            Volume da settare (0-63)
*
\****************************************************************************/

  void SetVolume(int dwVolume);



/****************************************************************************\
*
* Function:     void GetVolume(int * lpdwVolume);
*
* Description:  Chiede il volume dello sfx
*
* Input:        int * lpdwVolume        Variabile in cui verra' inserito
*                                       il volume corrente
*
\****************************************************************************/

  void GetVolume(int * lpdwVolume);
};

class FPSTREAM {

/****************************************************************************\
*       Attributi
\****************************************************************************/

private:

/*
	HWND hwnd;
	LPDIRECTSOUND lpDS;
	LPDIRECTSOUNDBUFFER lpDSBuffer;       // Buffer DirectSound circolare
	LPDIRECTSOUNDNOTIFY lpDSNotify;       // Notify degli hotspot nel buffer
*/
	byte *lpTempBuffer;                  // Buffer temporaneo per decompressione

	uint32 dwBufferSize;                   // Dimensione del buffer in bytes
	uint32 dwSize;                         // Dimensione dello stream in bytes
	uint32 dwCodec;                        // CODEC utilizzato

	HANDLE hThreadEnd;                    // Evento per chiudere il thread
	Common::File _file;                   // Handle del file di stream
	HANDLE hPlayThread;                   // Handle del thread di play
	HANDLE hHot1, hHot2, hHot3;           // Eventi settati da DirectSoundNotify
	HANDLE hPlayThread_PlayFast;
	HANDLE hPlayThread_PlayNormal;

	bool bSoundSupported;                 // TRUE se il suono e' attivo
	bool bFileLoaded;                     // TRUE se e' stato aperto un file
	bool bLoop;                           // TRUE se bisogna loopare lo stream
	bool bDoFadeOut;                      // TRUE se bisogna fare un fade out
	bool bSyncExit;
	bool bPaused;
	int lastVolume;
	FPSTREAM *SyncToPlay;
//	DSBPOSITIONNOTIFY dspnHot[3];

	CODEC *lpCodec;                       // CODEC da utilizzare.
	bool CreateBuffer(int nBufSize);

public:
	bool bIsPlaying;                      // TRUE se si sta playando lo stream

private:

	static void PlayThread(FPSTREAM *This);

/****************************************************************************\
*       Metodi
\****************************************************************************/

public:

/****************************************************************************\
*
* Function:     FPSTREAM(LPDIRECTSOUND lpDS, bool bSoundOn);
*
* Description:  Costruttore di default. *NON* bisogna dichiarare direttamente
*               un oggetto, ma crearlo piuttosto tramite FPSOUND::CreateStream()
*
\****************************************************************************/

	FPSTREAM(void * /*LPDIRECTSOUND*/ lpDS, uint32 /*HWND hWnd */, bool bSoundOn);


/****************************************************************************\
*
* Function:     ~FPSTREAM();
*
* Description:  Distruttore di default. Si preoccupa anche di fermare stream
*               eventualmente in esecuzione, e disallocare la memoria da
*               essi occupata.
*
\****************************************************************************/

	~FPSTREAM();


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

	void Release();


/****************************************************************************\
*
* Function:     bool LoadFile(char *lpszFileName, uint32 dwCodec=FPCODEC_RAW);
*
* Description:  Apre un file di stream.
*
* Input:        char *lpszFile          Nome del file di stream da aprire
*               uint32 dwCodec           CODEC da utilizzare per decomprimere
*                                       i campioni sonori
*
* Return:       TRUE se tutto OK, FALSE in caso di errore
*
\****************************************************************************/

	bool LoadFile(const char *lpszFileName, uint32 dwCodec = FPCODEC_RAW, int nSync = 2000);



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

	bool UnloadFile();


/****************************************************************************\
*
* Function:     bool Play();
*
* Description:  Suona lo stream caricato.
*
* Return:       TRUE se tutto OK, FALSE in caso di errore.
*
\****************************************************************************/

	bool Play();
	void PlayFast(void);
	void Prefetch(void);


/****************************************************************************\
*
* Function:     bool Stop();
*
* Description:  Ferma il play dello stream.
*
* Return:       TRUE se tutto OK, FALSE in caso di errore.
*
\****************************************************************************/

	bool Stop(bool bSync = false);
	void WaitForSync(FPSTREAM* toplay);


/****************************************************************************\
*
* Function:     void Pause(bool bPause);
*
* Description:  Pause dell'effetto sonoro
*
\****************************************************************************/

	void Pause(bool bPause);


/****************************************************************************\
*
* Function:     bool SetLoop(bool bLoop);
*
* Description:  Attiva o disattiva il loop dello stream.
*
* Input:        bool bLoop              TRUE per attivare il loop, FALSE per
*                                       disattivarlo
*
* Note:         Il loop deve essere attivato PRIMA di eseguire il play
*               dello stream. Qualsiasi modifica effettuata durante il play
*               non avra' effetto fino a che lo stream non viene fermato,
*               e poi rimesso in play.
*
\****************************************************************************/

	void SetLoop(bool bLoop);



/****************************************************************************\
*
* Function:     void SetVolume(int dwVolume);
*
* Description:  Cambia il volume dello stream
*
* Input:        int dwVolume            Volume da settare (0-63)
*
\****************************************************************************/

	void SetVolume(int dwVolume);



/****************************************************************************\
*
* Function:     void GetVolume(LPINT lpdwVolume);
*
* Description:  Chiede il volume dello stream
*
* Input:        LPINT lpdwVolume        Variabile in cui verra' inserito
*                                       il volume corrente
*
\****************************************************************************/

	void GetVolume(int *lpdwVolume);
};


} // End of namespace Tony

#endif
