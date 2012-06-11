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

#include "audio/audiostream.h"
#include "audio/decoders/adpcm.h"
#include "audio/decoders/raw.h"
#include "audio/decoders/wave.h"
#include "common/textconsole.h"
#include "tony/game.h"
#include "tony/tony.h"

namespace Tony {


/****************************************************************************\
*       Defines
\****************************************************************************/

#define RELEASE(x)             {if ((x) != NULL) { (x)->Release(); x = NULL; }}

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
	bSoundSupported = false;
}


/****************************************************************************\
*
* Function:     bool FPSOUND::Init();
*
* Description:  Inizializza l'oggetto, e prepara tutto il necessario per
*               creare stream e effetti sonori.
*
* Return:       True se tutto OK, false in caso di errore.
*
\****************************************************************************/

bool FPSOUND::Init() {
	bSoundSupported = g_system->getMixer()->isReady();
	return bSoundSupported;
}


/****************************************************************************\
*
* Function:     FPSOUND::~FPSOUND();
*
* Description:  Deinizializza l'oggetto, disallocando la memoria.
*
\****************************************************************************/

FPSOUND::~FPSOUND() {
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
	(*lplpStream) = new FPSTREAM(bSoundSupported);

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
	(*lplpSfx) = new FPSFX(bSoundSupported);

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
	if (!bSoundSupported)
		return;

	g_system->getMixer()->setVolumeForSoundType(Audio::Mixer::kPlainSoundType, CLIP<int>(dwVolume, 0, 63) * Audio::Mixer::kMaxChannelVolume / 63);
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
	if (!bSoundSupported)
		return;

	*lpdwVolume = g_system->getMixer()->getVolumeForSoundType(Audio::Mixer::kPlainSoundType) * 63 / Audio::Mixer::kMaxChannelVolume;
}


/****************************************************************************\
*       Metodi di FPSFX
\****************************************************************************/

/****************************************************************************\
*
* Function:     FPSFX(bool bSoundOn);
*
* Description:  Costruttore di default. *NON* bisogna dichiarare direttamente
*               un oggetto, ma crearlo piuttosto tramite FPSOUND::CreateSfx()
*
\****************************************************************************/

FPSFX::FPSFX(bool bSoundOn) {
	bSoundSupported = bSoundOn;
	bFileLoaded = false;
	lastVolume = 63;
	hEndOfBuffer = CORO_INVALID_PID_VALUE;
	bIsVoice = false;
	_stream = 0;
	_rewindableStream = 0;
	bPaused = false;

	_vm->_activeSfx.push_back(this);
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
	if (!bSoundSupported)
		return;

	g_system->getMixer()->stopHandle(_handle);
	_vm->_activeSfx.remove(this);

	delete _stream;
	// _rewindableStream is deleted by deleting _stream

	// FIXME
	//if (hEndOfBuffer != CORO_INVALID_PID_VALUE)
	//	CloseHandle(hEndOfBuffer);
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
}



/****************************************************************************\
*
* Function:     bool loadWave(Common::SeekableReadStream *stream);
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

bool FPSFX::loadWave(Common::SeekableReadStream *stream) {
	if (!stream)
		return false;

	_rewindableStream = Audio::makeWAVStream(stream, DisposeAfterUse::YES);

	if (!_rewindableStream)
		return false;

	_stream = _rewindableStream;
	bFileLoaded = true;
	SetVolume(lastVolume);
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
	if (!bSoundSupported)
		return true;

	uint32 size = vdbFP.readUint32LE();
	uint32 rate = vdbFP.readUint32LE();
	bIsVoice = true;

	_rewindableStream = Audio::makeADPCMStream(vdbFP.readStream(size), DisposeAfterUse::YES, 0, Audio::kADPCMDVI, rate, 1);
	_stream = _rewindableStream;

	bFileLoaded = true;
	SetVolume(62);
	return true;
}


bool FPSFX::LoadFile(const char *lpszFileName, uint32 dwCodec) {
	if (!bSoundSupported)
		return true;

	Common::File file;
	if (!file.open(lpszFileName)) {
		warning("FPSFX::LoadFile(): Cannot open sfx file!");
		return false;
	}

	if (file.readUint32BE() != MKTAG('A', 'D', 'P', 0x10)) {
		warning("FPSFX::LoadFile(): Invalid ADP header!");
		return false;
	}

	uint32 rate = file.readUint32LE();
	uint32 channels = file.readUint32LE();

	Common::SeekableReadStream *buffer = file.readStream(file.size() - file.pos());

	if (dwCodec == FPCODEC_ADPCM) {
		_rewindableStream = Audio::makeADPCMStream(buffer, DisposeAfterUse::YES, 0, Audio::kADPCMDVI, rate, channels);
	} else {
		byte flags = Audio::FLAG_16BITS | Audio::FLAG_LITTLE_ENDIAN;

		if (channels == 2)
			flags |= Audio::FLAG_STEREO;

		_rewindableStream = Audio::makeRawStream(buffer, rate, flags, DisposeAfterUse::YES);
	}

	if (bLoop)
		_stream = Audio::makeLoopingAudioStream(_rewindableStream, 0);
	else
		_stream = _rewindableStream;

	bFileLoaded = true;
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
	Stop(); // sanity check

	if (bFileLoaded) {
		// FIXME
		//if (hEndOfBuffer != CORO_INVALID_PID_VALUE)
		//	ResetEvent(hEndOfBuffer);

		_rewindableStream->rewind();

		g_system->getMixer()->playStream(Audio::Mixer::kPlainSoundType, &_handle, _stream, -1,
				Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO);

		SetVolume(lastVolume);

		if (bPaused)
			g_system->getMixer()->pauseHandle(_handle, true);
	}

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

bool FPSFX::Stop() {
	if (bFileLoaded) {
		g_system->getMixer()->stopHandle(_handle);
		bPaused = false;
	}

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
	if (bFileLoaded) {
		if (g_system->getMixer()->isSoundHandleActive(_handle) && (bPause ^ bPaused))
			g_system->getMixer()->pauseHandle(_handle, bPause);

		bPaused = bPause;
	}
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

	if (g_system->getMixer()->isSoundHandleActive(_handle))
		g_system->getMixer()->setChannelVolume(_handle, dwVolume * Audio::Mixer::kMaxChannelVolume / 63);
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
	if (g_system->getMixer()->isSoundHandleActive(_handle))
		*lpdwVolume = g_system->getMixer()->getChannelVolume(_handle) * 63 / Audio::Mixer::kMaxChannelVolume;
	else
		*lpdwVolume = 0;
}

/**
 * Returns true if the underlying sound has ended
 */
bool FPSFX::endOfBuffer() const {
	return !g_system->getMixer()->isSoundHandleActive(_handle) && (!_stream || _stream->endOfData());
}

/**
 * Continually checks to see if active sounds have finished playing
 * Sets the event signalling the sound has ended
 */
void FPSFX::soundCheckProcess(CORO_PARAM, const void *param) {
	CORO_BEGIN_CONTEXT;
		Common::List<FPSFX *>::iterator i;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	for (;;) {
		// Check each active sound
		for (_ctx->i = _vm->_activeSfx.begin(); _ctx->i != _vm->_activeSfx.end(); ++_ctx->i) {
			FPSFX *sfx = *_ctx->i;
			if (sfx->endOfBuffer())
				CoroScheduler.setEvent(sfx->hEndOfBuffer);
		}

		// Delay until the next check is done
		CORO_INVOKE_1(CoroScheduler.sleep, 50);
	}

	CORO_END_CODE;
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

FPSTREAM::FPSTREAM(bool bSoundOn) {
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
