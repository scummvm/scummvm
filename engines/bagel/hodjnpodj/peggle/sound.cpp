/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "bagel/hodjnpodj/hnplibs/stdafx.h"
#include "bagel/boflib/sound.h"

namespace Bagel {
namespace HodjNPodj {
namespace Peggle {

IMPLEMENT_DYNCREATE(CSound, CObject)

BOOL	CSound::m_bInited = FALSE;		// whether sounds have been inited
CSound	*CSound::m_pSoundChain = NULL;  // pointer to chain of linked Sounds
int		CSound::m_nIndex = 0;			// unique Sound indentifier
int		CSound::m_nCount = 0;			// count of currently active Sounds
int 	CSound::m_nWavCount = 0;		// available wave sound devices
int 	CSound::m_nMidiCount = 0;		// available midi sound devices
BOOL	CSound::m_bSoundAvailable = FALSE;	// whether wave sound is available
BOOL	CSound::m_bMidiAvailable = FALSE;	// whether midi sound is available
CWnd	*CSound::m_pMainWnd = NULL;			// window for message processing


/*************************************************************************
 *
 * CSound()
 *
 * Parameters:		none
 *
 * Return Value:	none
 *
 * Description:		Constructor for Sound class.  Initialize all fields
 *					to logical NULL.  Calls should then be made to the
 *					Initialize and Play routines.
 *
 ************************************************************************/

CSound::CSound()
{
	Initialize(NULL, NULL, 0);          	// initialize to null values

	m_pNext = m_pSoundChain;            	// insert at head of chain
	if (m_pSoundChain != NULL)
		(*m_pSoundChain).m_pPrev = this;
	m_pPrev = NULL;
	m_pSoundChain = this;
}


/*************************************************************************
 *
 * CSound()
 *
 * Parameters:
 *
 *	pWnd			pointer to the parent window
 *  pszPathName		pointer to text string that is the path specification
 *					for the Sound out on disk
 *	wFlags			specialized flags
 *
 * Return Value:	none
 *
 * Description:		Constructor for sound class.  Initialize all fields
 *					to values specifed.  Calls should then be made to the
 *					Play routine.
 *
 ************************************************************************/

CSound::CSound(CWnd *pWnd, char *pszPathName, WORD wFlags)
{
	Initialize(pWnd, pszPathName, wFlags);  // initialize data fields

	m_pNext = m_pSoundChain;            	// insert at head of chain
	if (m_pSoundChain != NULL)
		(*m_pSoundChain).m_pPrev = this;
	m_pPrev = NULL;
	m_pSoundChain = this;
}


/*************************************************************************
 *
 * ~CSound()
 *
 * Parameters:		none
 *
 * Return Value:	none
 *
 * Description:		Destructor for Sound class.  The Sound object is
 *					removed from the chain.  If it is then stopped and unloaded
 *					if it is currently active.
 *
 ************************************************************************/

CSound::~CSound()
{
BOOL	bSuccess;
CSound	*pSound, *pTemp;
	
	if (m_nID == (*m_pSoundChain).m_nID) {				// special case head of chain
		m_pSoundChain = (*m_pSoundChain).m_pNext;
		if (m_pSoundChain != NULL)
			(*m_pSoundChain).m_pPrev = NULL;
	}
	else {												// find it in the chain and
		pSound = m_pSoundChain;                         // ... modify the linkages of
		while((*pSound).m_nID != m_nID) {               // ... its neighbors to point
			pSound = (*pSound).m_pNext;                 // ... around it
		}
		pTemp = (*pSound).m_pPrev;
		(*pTemp).m_pNext = m_pNext;
		pTemp = m_pNext;
		if (pTemp != NULL)
			(*pTemp).m_pPrev = m_pPrev;
	}

	if (m_bPlaying == TRUE) {							// stop it and unload it if
		bSuccess = Stop();                             	// ... it is currently playing
		ASSERT(bSuccess);
	}
}


/*************************************************************************
 *
 * InitializeSound()
 *
 * Parameters:		none
 *
 * Return Value:	none
 *
 * Description:		Do initial sound setup functions.
 *
 ************************************************************************/

void CSound::InitializeSound(CWnd *pWnd)
{
m_nWavCount = ::midiOutGetNumDevs();
if (m_nWavCount > 0)
	m_bSoundAvailable = TRUE;
else
	m_bSoundAvailable = FALSE;

m_nMidiCount = waveOutGetNumDevs();
if (m_nMidiCount > 0)
	m_bMidiAvailable = TRUE;
else
	m_bMidiAvailable = FALSE;

m_pMainWnd = pWnd;
}


/*****************************************************************
 *
 * Initialize
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      Initialize a Sound that is in WAV or MIDI format.
 *   
 * FORMAL PARAMETERS:
 *
 *	pWnd			pointer to the parent window
 *  pszPathName		pointer to text string that is the path specification
 *					for the Sound out on disk
 *	wFlags			specialized flags (SOUND_xyz)
 *   
 * RETURN VALUE:
 *
 *      BOOL		success / failure condition
 *
 ****************************************************************/

void CSound::Initialize(CWnd *pWnd, char *pszPathName, WORD wFlags )
{
	m_pMainWnd = pWnd;						// parent window
	m_pszPathName = pszPathName;        	// path spec for sound file
	m_wFlags = wFlags;              		// flags for playing
	m_lpWaveInst = NULL;                    // no wave data
 	m_lpWaveHdr = NULL;
	m_hWaveOut = NULL;
	m_wDeviceID = 0;						// no device id
	m_nID = ++m_nIndex;						// unique identifier
	m_bPlaying = FALSE;                 	// not yet playing
	m_bPaused = FALSE;						// not suspended
	m_bError = FALSE;						// no errors yet           
	m_bExtensionsUsed=FALSE;		//no extended flags used.

 }


/*****************************************************************
 *
 * Play
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      Dispatch to the appropriate routine to play a sound object.
 *   
 * FORMAL PARAMETERS: (BOTH OPTIONAL)
 *
 *		DWORD dwBeginHere		Begin Playing Sound at this time (fmt as in parameter #2).  
 *		DWORD TimeFormatFlag	Specifies TimeFormat; the following constants are defined :
 																FMT_MILLISEC ---> all time parameters will be in milliseconds.
 *
 *		dwStartOfPlay is defualted to 0L.
 *      TimeFlag is defaulted to FMT_MILLISEC.    
 *   
 * RETURN VALUE:
 *
 *      BOOL		success / failure condition
 *
 ****************************************************************/

BOOL CSound::Play(DWORD dwBeginHere, DWORD TimeFormatFlag)
{
BOOL	bSuccess = FALSE;

	if (m_bPlaying)										// punt if already playing
		return(FALSE);

	m_bError = TRUE;									// assume there is an error
		
	if (m_bInited == FALSE) {							// initialize sound info if not done
		InitializeSound(m_pMainWnd);
		m_bInited = TRUE;
	}

	if (m_wFlags & SOUND_WAVE) {						// if it is a .WAV file	
		if (m_bSoundAvailable == FALSE)					// ... see if we can play it
			return(FALSE);
		if (!(m_wFlags & SOUND_BUFFERED)) {             // First try play it loaded in memory
			if ((m_wFlags & SOUND_NOTIFY) ||	            // ... in memory if so requested
				(m_wFlags & SOUND_AUTODELETE))
				bSuccess = PlayMMIOSound();				// play the wave file
			else
				bSuccess = PlaySndSound();				// try to play it now
			if (bSuccess == TRUE)						// Leave if successful
				return(TRUE);                           // ... otherwise try buffered mode
		}
	}

	if (m_wFlags & SOUND_MIDI)							// if it is a .MID file	
		if (m_bMidiAvailable == FALSE)					// ... see if we can play it
			return(FALSE);
	
	bSuccess = PlayMCISound(dwBeginHere,TimeFormatFlag);							// play an .MID file or buffered .WAV

	return(bSuccess);
}


/*****************************************************************
 *
 * PlaySndSound
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      Initialize, and begin playing, a WAV formated Sound
 *		via sndPlay commands.  Callbacks are not supported.
 *   
 * FORMAL PARAMETERS:
 *
 *      n/a
 *   
 * RETURN VALUE:
 *
 *      BOOL		success / failure condition
 *
 ****************************************************************/

BOOL CSound::PlaySndSound(void)
{
BOOL	bSuccess;

	m_chType = SOUND_TYPE_SND;						// mark it as an SndPlay type
	bSuccess = sndPlaySound(m_pszPathName,          // now play the sound
							((m_wFlags & SOUND_ASYNCH) ? SND_ASYNC : 0) |
							((m_wFlags & SOUND_LOOP) ? (SND_LOOP | SND_ASYNC) : 0));

	m_bError = !bSuccess;
	
	if (bSuccess == TRUE) {
	    if (!(m_wFlags & SOUND_ASYNCH) &&
			(m_wFlags & SOUND_AUTODELETE))
			delete this;
	}

	return(bSuccess);
}


/*****************************************************************
 *
 * PlayMMIOSound
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      Initialize, and begin playing, a WAV formated Sound
 *		via low-level sound commands. 
 *   
 * FORMAL PARAMETERS:
 *
 *      n/a
 *   
 * RETURN VALUE:
 *
 *      BOOL			success / failure condition
 *
 ****************************************************************/

BOOL CSound::PlayMMIOSound(void)
{
LPWAVEHDR   lpWaveHdr = NULL;    
LPWAVEINST  lpWaveInst = NULL;    
HANDLE      hWaveHdr = NULL;
HANDLE      hWaveInst = NULL;
HANDLE      hData = NULL;
LPSTR       lpData = NULL;
HWAVEOUT	hWaveOut = NULL;
HMMIO       hmmio;
MMCKINFO    mmckinfoParent;
MMCKINFO    mmckinfoSubchunk;
DWORD       dwFmtSize;
WORD        wResult;
HANDLE      hFormat = NULL;
WAVEFORMAT  *pFormat = NULL;
DWORD       dwDataSize;
WORD        wBlockSize;

	m_bError = TRUE;								// assume failure
	m_chType = SOUND_TYPE_MMIO;						// mark it as a MMIO type

	if (m_pMainWnd == NULL)                 		// not valid if no owning window
		return(FALSE); 

    /* Open the given file for reading using buffered I/O.
     */
	if(!(hmmio = mmioOpen(m_pszPathName, NULL, MMIO_READ | MMIO_ALLOCBUF)))
	{
		return(FALSE);
    }

    /* Locate a 'RIFF' chunk with a 'WAVE' form type 
     * to make sure it's a WAVE file.
     */
    mmckinfoParent.fccType = mmioFOURCC('W', 'A', 'V', 'E');
    if (mmioDescend(hmmio, (LPMMCKINFO) &mmckinfoParent, NULL, MMIO_FINDRIFF))
    {
		mmioClose(hmmio, 0);
		return(FALSE);
    }
    
    /* Now, find the format chunk (form type 'fmt '). It should be
     * a subchunk of the 'RIFF' parent chunk.
     */
    mmckinfoSubchunk.ckid = mmioFOURCC('f', 'm', 't', ' ');
    if (mmioDescend(hmmio, &mmckinfoSubchunk, &mmckinfoParent, 
        MMIO_FINDCHUNK))
    {
		mmioClose(hmmio, 0);
		return(FALSE);
    }

    /* Get the size of the format chunk, allocate and lock memory for it.
     */
    dwFmtSize = mmckinfoSubchunk.cksize;
    hFormat = LocalAlloc(LMEM_MOVEABLE, LOWORD(dwFmtSize));
    if (!hFormat)
    {
		mmioClose(hmmio, 0);
		return(FALSE);
    }
    pFormat = (WAVEFORMAT *) LocalLock(hFormat);
    if (!pFormat)
    {
		LocalFree( hFormat );
        mmioClose(hmmio, 0);
		return(FALSE);
    }

    /* Read the format chunk.
     */
    if (mmioRead(hmmio, (HPSTR) pFormat, dwFmtSize) != (LONG) dwFmtSize)
    {
		LocalUnlock( hFormat );
        LocalFree( hFormat );
        mmioClose(hmmio, 0);
		return(FALSE);
    }
    
    /* Make sure it's a PCM file.
     */
    if (pFormat->wFormatTag != WAVE_FORMAT_PCM)
    {
        LocalUnlock( hFormat );
        LocalFree( hFormat );
        mmioClose(hmmio, 0);
		return(FALSE);
    }

    /* Make sure a waveform output device supports this format.
     */
    if (waveOutOpen(&hWaveOut, (UINT)WAVE_MAPPER, (LPWAVEFORMAT)pFormat, NULL, 0L,
		    (DWORD)WAVE_FORMAT_QUERY))
    {
        LocalUnlock( hFormat );
        LocalFree( hFormat );
        mmioClose(hmmio, 0);
		return(FALSE);
    }
    
    /* Ascend out of the format subchunk.
     */
    mmioAscend(hmmio, &mmckinfoSubchunk, 0);
    
    /* Find the data subchunk.
     */
    mmckinfoSubchunk.ckid = mmioFOURCC('d', 'a', 't', 'a');
    if (mmioDescend(hmmio, &mmckinfoSubchunk, &mmckinfoParent,
        MMIO_FINDCHUNK))
    {
		LocalUnlock( hFormat );
        LocalFree( hFormat );
        mmioClose(hmmio, 0);
		return(FALSE);
    }

    /* Get the size of the data subchunk.
     */
    dwDataSize = mmckinfoSubchunk.cksize;
    if (dwDataSize == 0L)
    {
		LocalUnlock( hFormat );
        LocalFree( hFormat );
        mmioClose(hmmio, 0);
		return(FALSE);
    }
    
    /* Open a waveform output device.
     */
    if (waveOutOpen((LPHWAVEOUT)&hWaveOut, (UINT)WAVE_MAPPER,
		  (LPWAVEFORMAT)pFormat, (UINT) (*m_pMainWnd).m_hWnd, 0L, (DWORD)CALLBACK_WINDOW))
    {
		LocalUnlock( hFormat );
        LocalFree( hFormat );
        mmioClose(hmmio, 0);
		return(FALSE);
    }

    /* Save block alignment info for later use.
     */
    wBlockSize = pFormat->nBlockAlign;

    /* We're done with the format header, free it.
     */
    LocalUnlock( hFormat );
    LocalFree( hFormat );
    
    /* Allocate and lock memory for the waveform data.
     */
    hData = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, dwDataSize );
    if (!hData)
    {
		mmioClose(hmmio, 0);
		return(FALSE);
    }
    lpData = (char *) GlobalLock(hData);
    if (!lpData)
    {
		GlobalFree( hData );
        mmioClose(hmmio, 0);
		return(FALSE);
    }

    /* Read the waveform data subchunk.
     */
    if(mmioRead(hmmio, (HPSTR) lpData, dwDataSize) != (LONG) dwDataSize)
    {
		GlobalUnlock( hData );
        GlobalFree( hData );
        mmioClose(hmmio, 0);
		return(FALSE);
    }

    /* We're done with the file, close it.
     */
    mmioClose(hmmio, 0);

    /* Allocate a waveform data header. The WAVEHDR must be 
     * globally allocated and locked.
     */
    hWaveHdr = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE,
                           (DWORD) sizeof(WAVEHDR));
    if (!hWaveHdr)
    {
        GlobalUnlock( hData );
        GlobalFree( hData );
		return(FALSE);
    }
    lpWaveHdr = (LPWAVEHDR) GlobalLock(hWaveHdr);
    if (!lpWaveHdr)
    {
        GlobalUnlock( hData );
        GlobalFree( hData );
        GlobalFree( hWaveHdr );
		return(FALSE);
    }

    /* Allocate and set up instance data for waveform data block.
     * This information is needed by the routine that frees the
     * data block after it has been played.
     */
    hWaveInst = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE,
                            (DWORD) sizeof(WAVEHDR));
    if (!hWaveInst)
    {
        GlobalUnlock( hData );
        GlobalFree( hData );
        GlobalUnlock( hWaveHdr );
        GlobalFree( hWaveHdr );
		return(FALSE);
    }
    lpWaveInst = (LPWAVEINST) GlobalLock(hWaveInst);
    if (!lpWaveInst)
    {
        GlobalUnlock( hData );
        GlobalFree( hData );
        GlobalUnlock( hWaveHdr );
        GlobalFree( hWaveHdr );
        GlobalFree( hWaveInst );
		return(FALSE);
    }
    lpWaveInst->hWaveInst = hWaveInst;
    lpWaveInst->hWaveHdr = hWaveHdr;
    lpWaveInst->hWaveData = hData;
        
    /* Set up WAVEHDR structure and prepare it to be written to wave device.
     */
    lpWaveHdr->lpData = lpData;
    lpWaveHdr->dwBufferLength = dwDataSize;
    lpWaveHdr->dwFlags = ((m_wFlags & SOUND_LOOP) ? (WHDR_BEGINLOOP | WHDR_ENDLOOP) : 0L);
    lpWaveHdr->dwLoops = ((m_wFlags & SOUND_LOOP) ? 2147483647L : 0L);
    lpWaveHdr->dwUser = m_nID;
    if(waveOutPrepareHeader(hWaveOut, lpWaveHdr, sizeof(WAVEHDR)))
    {
        GlobalUnlock( hData );
        GlobalFree( hData );
        GlobalUnlock( hWaveHdr );
        GlobalFree( hWaveHdr );
        GlobalUnlock( hWaveInst );
        GlobalFree( hWaveInst );
		return(FALSE);
    }
        
    /* Then the data block can be sent to the output device.
     */

    wResult = waveOutWrite(hWaveOut, lpWaveHdr, sizeof(WAVEHDR));
    if (wResult != 0)
    {
        waveOutUnprepareHeader( hWaveOut, lpWaveHdr, sizeof(WAVEHDR));
        GlobalUnlock( hData );
        GlobalFree( hData );
        GlobalUnlock( hWaveHdr );
        GlobalFree( hWaveHdr );
        GlobalUnlock( hWaveInst );
        GlobalFree( hWaveInst );
		return(FALSE);
	}

    m_lpWaveInst = lpWaveInst;					// retain pointers to sound structures
	m_lpWaveHdr = lpWaveHdr;
	m_hWaveOut = hWaveOut;

	m_nCount += 1;                          	// bump count of active Sounds
	m_bPlaying = TRUE;							// mark that it's now playing
	m_bPaused = FALSE;
	m_bError = FALSE;

	return(TRUE);
}


/*****************************************************************
 *
 * PlayMCISound
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      Initialize, and begin playing, a WAV or MIDI formated Sound
 *		via MCI commands.  For asynch sounds, an event is generated
 *		for the main window when the sound terminates; the callback
 *		routine (OnMCIStopped) then releases the resources used to
 *		play that Sound.
 *   
 * FORMAL PARAMETERS:	(BOTH OPTOINAL)
 *
 *      DWORD dwStartOfPlay		Begin Playing Sound at this time (fmt as in parameter #2).  
 *		DWORD TimeFlag			Specifies TimeFormat; the following constants are defined:
 														FMT_MILLISEC ---> all time parameters will be in milliseconds.
 *
 *		dwStartOfPlay is defualted to 0L.
 *      TimeFlag is defaulted to FMT_MILLISEC.
 *   
 * RETURN VALUE:
 *
 *      BOOL			success / failure condition
 *
 ****************************************************************/

BOOL CSound::PlayMCISound(DWORD dwStartOfPlay, DWORD TimeFlag)
{
UINT    			wDeviceID;
DWORD   			dwReturn;
MCI_OPEN_PARMS  	mciOpenParams;
MCI_PLAY_PARMS  	mciPlayParams;
MCI_STATUS_PARMS	mciStatusParams;

	m_bError = TRUE;								// assume failure
	m_bPlaying = FALSE;								// not yet playing
	m_bPaused = FALSE;
    m_chType = SOUND_TYPE_MCI;						// using MCI commands     
    m_dwPlayStart=dwStartOfPlay;

	if (m_wFlags & SOUND_WAVE)                  	// set for .WAV or .MID
    	mciOpenParams.lpstrDeviceType = "waveaudio";
    else
    	mciOpenParams.lpstrDeviceType = "sequencer";
    mciOpenParams.lpstrElementName = (LPSTR) m_pszPathName;
    
    dwReturn = mciSendCommand( 0, MCI_OPEN,			// open a sound device 
                               MCI_OPEN_TYPE | MCI_OPEN_ELEMENT, 
                               (DWORD)(LPVOID)&mciOpenParams);
    if (dwReturn != 0)                          	// punt if no device acquired
        return(FALSE);

    wDeviceID = mciOpenParams.wDeviceID;        	// save the device
    if(wDeviceID){
    	 m_wDeviceID=wDeviceID;
	    if (SetMCITimeFormat(TimeFlag))       		//set time format.           
	    	return(FALSE);							//return if error.
	}

    mciPlayParams.dwFrom = dwStartOfPlay;           // will play the entire file if StartOfPlay is 0L.
    mciPlayParams.dwTo = m_dwRePlayEnd;				// will play to m_dwRePlayEnd if SOUND_DONT_LOOP_TO_END is specified.

	if (m_wFlags & SOUND_MIDI) {                	// if its .MID check for MIDI Mapper
		mciStatusParams.dwItem = MCI_SEQ_STATUS_PORT;
		dwReturn = mciSendCommand(wDeviceID,MCI_STATUS,MCI_STATUS_ITEM,
								  (DWORD)(LPVOID) &mciStatusParams);
		if ((dwReturn != 0) ||                  	// problem, so punt
			(LOWORD(mciStatusParams.dwReturn) != MIDI_MAPPER)) {	
	        mciSendCommand( wDeviceID, MCI_CLOSE, MCI_WAIT, NULL);
	        return(FALSE);
	    }
	}
    
    if ((m_wFlags & SOUND_NOTIFY) ||				// play the sound file with notify
		(m_wFlags & SOUND_LOOP) ||
		(m_wFlags & SOUND_ASYNCH & SOUND_AUTODELETE)) {
		if (m_pMainWnd == NULL) {                	// not valid if no owning window
	        mciSendCommand( wDeviceID, MCI_CLOSE, MCI_WAIT, NULL);
	        return(FALSE);
	    }
	    mciPlayParams.dwCallback = MAKELONG( (*m_pMainWnd).m_hWnd,0);
	    dwReturn = mciSendCommand(wDeviceID, MCI_PLAY, MCI_NOTIFY|MCI_FROM|((m_wFlags&SOUND_DONT_LOOP_TO_END)? MCI_TO:0X00),
                                  (DWORD)(LPVOID) &mciPlayParams);
        if (dwReturn == 0) {
			m_nCount += 1;                          // bump count of active Sounds
			m_bPlaying = TRUE;						// mark that it's now playing                
		    m_wDeviceID = wDeviceID;                // save the device id
        }
    }
    else {
    	mciPlayParams.dwCallback = 0;           	// play without notification
	    dwReturn = mciSendCommand(wDeviceID, MCI_PLAY, (m_wFlags & SOUND_ASYNCH) ? 0 : MCI_WAIT,
                                  (DWORD)(LPVOID) &mciPlayParams);
	    if ((dwReturn == 0) &&
	    	(m_wFlags & SOUND_AUTODELETE)) {		// release it if marked for delete
			delete this;				           	// ... and was not an asynch sound
	    	return(TRUE);                            
		}
    }
    
    if (dwReturn != 0) {                        	// abort if there is a problem
        mciSendCommand( wDeviceID, MCI_CLOSE, MCI_WAIT, NULL);
        return(FALSE);
    }

    m_bError = FALSE;                           	// mark for no error occurred

    return(TRUE);
}                                                 


/*****************************************************************
 *
 * RePlayMCISound
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      Replay a WAV or MIDI formated Sound via MCI commands.  An
 *		event is generated for the main window when the sound ends;
 *		the callback routine (OnMCIStopped) is then responsible for
 *		deciding what to do next.
 *   
 * FORMAL PARAMETERS:
 *
 *      DWORD wFlags	flags for MCI_PLAY  
 *		DWORD From 		Time in the Format set in MidiLoopSegment() to begin replay. 
 *		DWORD To 		Time in the Format set in MidiLoopSegment() to end replay.  
 *		
 *      The From and To parameters are defaulted to 0L. They are ignored UNLESS the
 *		the MCI_FROM or the MCI_TO flags, RESPECTIVELY  are specified. 
 *
 * RETURN VALUE:
 *
 *      BOOL			success / failure condition
 *
 ****************************************************************/

BOOL CSound::RePlayMCISound(DWORD dwFlag, DWORD From, DWORD To)		
{                                                                            //From and To are defaulted to 0L.
DWORD   			dwReturn;
MCI_PLAY_PARMS  	mciPlayParams;

	m_bError = TRUE;								// assume failure
	m_bPlaying = FALSE;								// not yet playing

    mciPlayParams.dwCallback = (DWORD)MAKELONG((*m_pMainWnd).m_hWnd,0);
    mciPlayParams.dwFrom = From;                   	
    mciPlayParams.dwTo = To; 
	
	dwReturn = mciSendCommand(m_wDeviceID, MCI_PLAY, dwFlag,
                              (DWORD)(LPVOID) &mciPlayParams);
    
    if (dwReturn != 0)								// had a problem
        return(FALSE);

    m_bError = FALSE;                           	// mark for no error occurred
    m_bPlaying = TRUE;								// now playing again
    return(TRUE);
}                                                 


/*************************************************************************
 *
 * midiLoopPlaySegment()
 *
 * Parameters:		????
 *
 * Return Value:
 *	BOOL			success / failure condition
 *
 * Description:		play a looping midi segment.
 *
 ************************************************************************/

BOOL CSound::midiLoopPlaySegment(DWORD dwLoopFrom, DWORD dwLoopTo, DWORD dwBegin, DWORD TimeFmt){                                 
   	
	m_wFlags |= SOUND_LOOP;
	m_dwRePlayStart=dwLoopFrom;
	m_dwRePlayEnd=dwLoopTo; 
	m_bExtensionsUsed=TRUE;
    return(Play(dwBegin, TimeFmt));
}	


/*************************************************************************
 *
 * PauseSounds()
 *
 * Parameters:		none
 *
 * Return Value:
 *	BOOL			success / failure condition
 *
 * Description:		suspend all playing sounds.
 *
 ************************************************************************/

BOOL CSound::PauseSounds(void)
{
BOOL	bSuccess = TRUE;
BOOL	bStatus;
CSound	*pSound;

	pSound = CSound::m_pSoundChain;					// thumb through all the sounds
	while(pSound != NULL) {
		if (((*pSound).m_bPlaying == TRUE) &&		// if one is playing
			((*pSound).m_bPaused == FALSE)) {       // ... and not paused
			bStatus = (*pSound).Pause();            // ... try to suspend it
			if (bStatus == TRUE)
				(*pSound).m_bPaused = TRUE;         // success
			else
				bSuccess = FALSE;                   // failure
		}
		pSound = (*pSound).m_pNext;
	}

	return(bSuccess);
}


/*************************************************************************
 *
 * Pause()
 *
 * Parameters:		none
 *
 * Return Value:
 *	BOOL			success / failure condition
 *
 * Description:		suspend an playing sound.
 *
 ************************************************************************/

BOOL CSound::Pause(void)
{
BOOL				bSuccess = FALSE;
DWORD   			dwReturn;
UINT				nResult;
MCI_GENERIC_PARMS  	mciGenericParams;

	if ((m_bPlaying == TRUE) &&						// must be playing to be paused
		(m_bPaused == FALSE)) {                     // ... and not already paused
		switch(m_chType) {

			case SOUND_TYPE_SND:                    // can't pause this type of sound
				bSuccess = FALSE;
				break;

			case SOUND_TYPE_MCI:                    // will either pause or abort
			    mciGenericParams.dwCallback = NULL; // ... aborts will be treated as pause
			    dwReturn = mciSendCommand(m_wDeviceID, MCI_PAUSE, MCI_WAIT,
			                              (DWORD)(LPVOID) &mciGenericParams);
			    bSuccess = ((dwReturn == 0) ? TRUE : FALSE);
			    break;

			case SOUND_TYPE_MMIO:                   // pauses should be okay
				nResult = waveOutPause((HWAVEOUT) m_hWaveOut);
			    bSuccess = ((nResult == 0) ? TRUE : FALSE);
		}
	}

	if (bSuccess == TRUE)
		m_bPaused = TRUE;

	return(bSuccess);
}

/*************************************************************************
 *
 * SetMCITimeFormat(DWORD)
 *
 * Formal Parameters(Optional):		
 *		DWORD formatFlag	Specifies Time Format. The following constants are defined:
 *														FMT_MILLISEC----> all time formats are now in milliseconds.
 *
 * Return Value:
 *	DWORD			0L if no error in time setting, else returns the error numeral. 
 *
 * Description:		Set Time Format.	Note this call is NOT VALID unless a call to MCI_OPEN is made, 
 *															such as in PlayMCISound(), or elsewhere,
 *															and the m_wDeviceID member is valid.
 *
 ************************************************************************/
DWORD CSound::SetMCITimeFormat(DWORD formatFlag){
	MCI_SET_PARMS mciSetParms;
		
	switch(formatFlag){
		case FMT_MILLISEC:
			mciSetParms.dwTimeFormat=MCI_FORMAT_MILLISECONDS;
			break;
	}
	return(mciSendCommand(m_wDeviceID, MCI_SET, MCI_SET_TIME_FORMAT, (DWORD)(LPVOID)&mciSetParms));
}


/*************************************************************************
 *
 * ResumeSounds()
 *
 * Parameters:		none
 *
 * Return Value:
 *	BOOL			success / failure condition
 *
 * Description:		resume all paused sounds.
 *
 ************************************************************************/

BOOL CSound::ResumeSounds(void)
{
BOOL	bSuccess = TRUE;
BOOL	bStatus;
CSound	*pSound;

	pSound = CSound::m_pSoundChain;					// thumb through all the sounds
	while(pSound != NULL) {
		if ((*pSound).m_bPaused) {                  // if one is paused
			bStatus = (*pSound).Resume();           // ... try to get it going again
			if (bStatus == TRUE)
				(*pSound).m_bPaused = FALSE;        // success
			else
				bSuccess = FALSE;                   // failure
		}
		pSound = (*pSound).m_pNext;
	}

	return(bSuccess);
}


/*************************************************************************
 *
 * Resume()
 *
 * Parameters:		none
 *
 * Return Value:
 *	BOOL			success / failure condition
 *
 * Description:		resume playing a sound.
 *
 ************************************************************************/

BOOL CSound::Resume(void)
{
BOOL				bSuccess = FALSE;
DWORD   			dwReturn;
UINT				nResult;
MCI_GENERIC_PARMS  	mciGenericParams;

	if (m_bPaused == TRUE) {						// must be paused to resume
		switch(m_chType) {

			case SOUND_TYPE_SND:					// not valid for this type
				bSuccess = FALSE;                   // ... and neither are pauses
				break;

			case SOUND_TYPE_MCI:                    // resume may not be valid
			    mciGenericParams.dwCallback = NULL; // ... in which case will use play
			    dwReturn = mciSendCommand(m_wDeviceID, MCI_RESUME, MCI_WAIT,
			                              (DWORD)(LPVOID) &mciGenericParams);
			    bSuccess = ((dwReturn == 0) ? TRUE : FALSE);
			    if (dwReturn == MCIERR_UNSUPPORTED_FUNCTION)	// not supported, so instead
			    	bSuccess = RePlayMCISound(MCI_NOTIFY|((m_wFlags&SOUND_DONT_LOOP_TO_END)? MCI_TO:0X00), 0L, m_dwRePlayEnd);	// ... play from current position to end of flle or to m_dwRePlayEnd.
			    break;

			case SOUND_TYPE_MMIO:					// resume should be okay
				nResult = waveOutRestart((HWAVEOUT) m_hWaveOut);
			    bSuccess = ((nResult == 0) ? TRUE : FALSE);
		}
	}


	if (bSuccess == TRUE)
		m_bPaused = FALSE;

	return(bSuccess);
}


/*************************************************************************
 *
 * StopSounds()
 *
 * Parameters:		none
 *
 * Return Value:	none
 *
 * Description:		Stop all Sounds in the Sound chain.
 *
 ************************************************************************/

BOOL CSound::StopSounds(void)
{
BOOL	bSuccess = TRUE;
BOOL	bStatus;
CSound	*pSound;

	pSound = CSound::m_pSoundChain;					// thumb through all the sounds
	while(pSound != NULL) {
		if ((*pSound).m_bPlaying) {                 // if one is playing
			(*pSound).m_bPaused = FALSE;			// ... its longer paused
			bStatus = (*pSound).Stop();           	// ... try to stop it
			if (bStatus == FALSE)
				bSuccess = FALSE;                   // not the failure
		}
		pSound = (*pSound).m_pNext;
	}

	if (bSuccess == TRUE) {                         // now do general stop calls
		(void) sndPlaySound(NULL,0);				// ... to catch all other sounds
		(void) mciSendCommand(MCI_ALL_DEVICE_ID, MCI_CLOSE, MCI_WAIT, NULL);
	}

	return(bSuccess);
}


/*****************************************************************
 *
 * Stop
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      This procedure is called to terminate a sound in progress.
 *   
 * FORMAL PARAMETERS:
 *
 *      n/a
 *   
 * RETURN VALUE:
 *
 *      TRUE / FALSE
 *
 ****************************************************************/

BOOL CSound::Stop(void)
{
BOOL		bSuccess = FALSE;
DWORD   	dwReturn;
UINT		nResult;
LPWAVEINST	lpWaveInst = NULL;

	if (m_bPlaying == TRUE) {						// only stop if it's playing
		switch(m_chType) {

			case SOUND_TYPE_SND:	             	// stop it as an SndPlay sound
				bSuccess = sndPlaySound(NULL,0);
				break;

			case SOUND_TYPE_MCI:					// stop it as an MCI sound
    			dwReturn = mciSendCommand(m_wDeviceID, MCI_CLOSE, MCI_WAIT, NULL);
				bSuccess = (dwReturn == 0) ? TRUE : FALSE;
				break;

			case SOUND_TYPE_MMIO:					// stop it as an MMIO sound
				nResult = waveOutReset((HWAVEOUT) m_hWaveOut);
				ASSERT(nResult == 0);
				lpWaveInst = (LPWAVEINST) m_lpWaveInst;
				nResult = waveOutUnprepareHeader((HWAVEOUT) m_hWaveOut, (LPWAVEHDR) m_lpWaveHdr, sizeof(WAVEHDR) );
				ASSERT(nResult == 0);
			    GlobalUnlock( lpWaveInst->hWaveData );
			    GlobalFree( lpWaveInst->hWaveData );
			    GlobalUnlock( lpWaveInst->hWaveHdr );
			    GlobalFree( lpWaveInst->hWaveHdr );
			    GlobalUnlock( lpWaveInst->hWaveInst );
			    GlobalFree( lpWaveInst->hWaveInst );
				nResult = waveOutClose((HWAVEOUT) m_hWaveOut);
				ASSERT(nResult == 0);
				bSuccess = TRUE;
		} 
	
		if (bSuccess == TRUE) {						// no longer playing
			m_bPlaying = FALSE;
			m_bPaused = FALSE;
			m_nCount -= 1;
		}
	}

	m_bError = !bSuccess;
	return(bSuccess);
}


/*****************************************************************
 *
 * OnMCIStopped
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      This procedure is called when a termination message is received;
 *		i.e. an MCI sound has finished playing.  If it is a looping sound,
 *		then an attempt is made to play it again.  Otherwise, cleanup is done,
 *		which simply involves closing the sound file and deleting the resource.
 *   
 * FORMAL PARAMETERS:
 *
 *      WPARAM		reason for the termination
 *		LPARAM		the identifier of the Sound that has stopped
 *   
 * RETURN VALUE:
 *
 *      always returns TRUE to signify that we processed the message
 *
 ****************************************************************/

CSound * CSound::OnMCIStopped(WPARAM wParam, LPARAM lParam)
{
BOOL	bSuccess = FALSE;								// success/failure status
CSound	*pSound;
	
	pSound = CSound::m_pSoundChain;						// find this Sound is the queue
	while(pSound != NULL) {
		if ((*pSound).m_wDeviceID == (UINT) lParam)
			break;
		pSound = (*pSound).m_pNext;
	}

	if (pSound != NULL) {
		if (wParam == MCI_NOTIFY_FAILURE)				// note if terminated by error
			(*pSound).m_bError = TRUE;
		else
		if ((wParam == MCI_NOTIFY_ABORTED) &&			// punt if just "paused"
			((*pSound).m_bPaused == TRUE))
			return(NULL);
		if ((*pSound).m_bPlaying == TRUE) {				// verify it's actually playing	
			if (!(*pSound).m_bError &&
				(*pSound).m_wFlags & SOUND_LOOP) {  	// if looping is specified
				if (pSound->m_wFlags & SOUND_DONT_LOOP_TO_END)
					bSuccess = (*pSound).RePlayMCISound(MCI_NOTIFY | MCI_FROM | MCI_TO, pSound->m_bExtensionsUsed ? pSound->m_dwRePlayStart: pSound->m_dwPlayStart, \
				 																																				pSound->m_dwRePlayEnd);	// ... try to play it again upto m_dwRePlayEnd
				else
					bSuccess = (*pSound).RePlayMCISound(MCI_NOTIFY | MCI_FROM, pSound->m_bExtensionsUsed ? pSound->m_dwRePlayStart: pSound->m_dwPlayStart, pSound->m_dwRePlayEnd);	// ... try to play it again to EOF.
				if (bSuccess)
					return(NULL);
			}                                       	// time to close the sound file
	    	mciSendCommand( LOWORD(lParam), MCI_CLOSE, MCI_WAIT, NULL );
			(*pSound).m_bPlaying = FALSE;				// mark sound as no longer playing
			(*pSound).m_bPaused = FALSE;
			m_nCount -= 1;                          	// decrement active sound count
		}
		else
			(*pSound).m_bError = FALSE;
		if ((*pSound).m_wFlags & SOUND_AUTODELETE) {	// scrag it if marked for deletion
			delete pSound;
			pSound = NULL;
		}
		else
		if (!((*pSound).m_wFlags & SOUND_NOTIFY))
			pSound = NULL;
	}

	return(pSound);										// return sound pointer on notify
}


/*****************************************************************
 *
 * OnMMIOStopped
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      This procedure is called when a termination message is received;
 *		i.e. an MMIO sound has finished playing.  If it is a looping sound,
 *		then an attempt is made to play it again.  Otherwise, cleanup is done,
 *		which simply involves closing the sound file and deleting the resource.
 *   
 * FORMAL PARAMETERS:
 *
 *      WPARAM		pointer to WAVEOUT information
 *		LPARAM		pointer to WAVEHDR information
 *   
 * RETURN VALUE:
 *
 *      always returns TRUE to signify that we processed the message
 *
 ****************************************************************/

CSound * CSound::OnMMIOStopped(WPARAM wParam, LPARAM lParam)
{
BOOL		bSuccess = FALSE;							// success/failure status
LPWAVEINST	lpWaveInst = NULL;
CSound		*pSound = NULL;
UINT		nResult;
int			nID;

	nID = (int) ((LPWAVEHDR)lParam)->dwUser;
	
	pSound = CSound::m_pSoundChain;						// find this Sound is the queue
	while(pSound != NULL) {
		if ((*pSound).m_nID == nID)
			break;
		pSound = (*pSound).m_pNext;
	}

	if (pSound != NULL) {                               // release the data structures
		nResult = waveOutUnprepareHeader( (HWAVEOUT) wParam, (LPWAVEHDR) lParam, sizeof(WAVEHDR) );
		ASSERT(nResult == 0);
		lpWaveInst = (*pSound).m_lpWaveInst;
	    GlobalUnlock( lpWaveInst->hWaveData );
	    GlobalFree( lpWaveInst->hWaveData );
	    GlobalUnlock( lpWaveInst->hWaveHdr );
	    GlobalFree( lpWaveInst->hWaveHdr );
	    GlobalUnlock( lpWaveInst->hWaveInst );
	    GlobalFree( lpWaveInst->hWaveInst );
		nResult = waveOutClose( (HWAVEOUT) wParam );
		ASSERT(nResult == 0);

		(*pSound).m_bPlaying = FALSE;					// mark sound as no longer playing
		(*pSound).m_bPaused = FALSE;
		m_nCount -= 1;                          		// decrement active sound count

		(*pSound).m_bError = FALSE;
		if ((*pSound).m_wFlags & SOUND_AUTODELETE) {	// scrag it if marked for deletion
			delete pSound;
			pSound = NULL;
		}
		else
		if (!((*pSound).m_wFlags & SOUND_NOTIFY))		// return sound pointer on notify
			pSound = NULL;
		bSuccess = TRUE;
	}

	return(pSound);
}


/*************************************************************************
 *
 * ClearSounds()
 *
 * Parameters:		none
 *
 * Return Value:	none
 *
 * Description:		Clear all Sounds from the Sound chain, stopping
 *					play of any that are currently active.
 *
 ************************************************************************/

void CSound::clearSounds(void)
{
	while(m_pSoundChain != NULL) {					// delete all sound entries
		delete m_pSoundChain;
	}
}


/*************************************************************************
 *
 * SoundAvailable()
 *
 * Parameters:		none
 *
 * Return Value:	TRUE / FALSE
 *
 * Description:		Determines whether .WAV sound output is available.
 *
 ************************************************************************/

BOOL CSound::SoundAvailable(void)
{
	if (m_bInited == FALSE) {						// do sound initialization if needed
		InitializeSound(m_pMainWnd);
		m_bInited = TRUE;
	}
	
	return(m_bSoundAvailable);						// return requested info
}


/*************************************************************************
 *
 * MidiAvailable()
 *
 * Parameters:		none
 *
 * Return Value:	TRUE / FALSE
 *
 * Description:		Determines whether .MID sound output is available.
 *
 ************************************************************************/

BOOL CSound::MidiAvailable(void)
{
	if (m_bInited == FALSE) {						// do sound initialization if needed
		InitializeSound(m_pMainWnd);
		m_bInited = TRUE;
	}
	
	return(m_bMidiAvailable);						// return requested info
}
	

/////////////////////////////////////////////////////////////////////////////
// CSound diagnostics

#ifdef _DEBUG
void CSound::AssertValid() const
{
	CObject::AssertValid();
}

void CSound::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);
}

#endif //_DEBUG

} // namespace Peggle
} // namespace HodjNPodj
} // namespace Bagel
