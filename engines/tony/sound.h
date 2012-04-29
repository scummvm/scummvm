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

#include "tony/gfxcore.h"
#include "tony/loc.h"
#include "tony/utils.h"

namespace Tony {

class FPSFX;

enum CODECS {
	FPCODEC_RAW,
	FPCODEC_ADPCM,
	FPCODEC_WAV
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
//	HANDLE hEndOfBuffer;

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

  bool LoadFile(char *lpszFileName, uint32 dwCodec = FPCODEC_RAW);
  bool LoadFile(byte *lpBuf, uint32 dwCodec);
  bool LoadVoiceFromVDB(HANDLE hvdb);


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

} // End of namespace Tony

#endif
