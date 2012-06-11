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

#ifndef TONY_SOUND_H
#define TONY_SOUND_H

#include "audio/mixer.h"
#include "common/file.h"
#include "tony/gfxcore.h"
#include "tony/loc.h"
#include "tony/utils.h"

namespace Audio {
class RewindableAudioStream;	
}

namespace Tony {

class FPSTREAM;
class FPSFX;

enum CODECS {
	FPCODEC_RAW,
	FPCODEC_ADPCM
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
	* Function:     bool FPSOUND::Init();
	*
	* Description:  Inizializza l'oggetto, e prepara tutto il necessario per
	*               creare stream e effetti sonori.
	*
	* Return:       True se tutto OK, FALSE in caso di errore.
	*
	\****************************************************************************/


	bool Init();


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
	bool bSoundSupported;                 // TRUE se il suono e' attivo
	bool bFileLoaded;                     // TRUE se e' stato aperto un file
	bool bLoop;                           // TRUE se bisogna loopare l'effetto sonoro
	int lastVolume;

	bool bIsVoice;
	bool bPaused;

	Audio::AudioStream *_stream;
	Audio::RewindableAudioStream *_rewindableStream;
	Audio::SoundHandle _handle;

public:
	uint32 hEndOfBuffer;

private:


	/****************************************************************************\
	*       Metodi
	\****************************************************************************/

public:

	/****************************************************************************\
	*
	* Function:     FPSFX(bool bSoundOn);
	*
	* Description:  Costruttore di default. *NON* bisogna dichiarare direttamente
	*               un oggetto, ma crearlo piuttosto tramite FPSOUND::CreateSfx()
	*
	\****************************************************************************/

	FPSFX(bool bSoundOn);


	/****************************************************************************\
	*
	* Function:     ~FPSFX();
	*
	* Description:  Distruttore di default. Si preoccupa anche di fermare il sound
	*                               effect eventualmente in esecuzione, e disallocare la memoria
	*                               da esso occupata.
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
	bool loadWave(Common::SeekableReadStream *stream);
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

	void GetVolume(int *lpdwVolume);
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
	* Function:     FPSTREAM(bool bSoundOn);
	*
	* Description:  Costruttore di default. *NON* bisogna dichiarare direttamente
	*               un oggetto, ma crearlo piuttosto tramite FPSOUND::CreateStream()
	*
	\****************************************************************************/

	FPSTREAM(bool bSoundOn);


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
	void WaitForSync(FPSTREAM *toplay);


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
