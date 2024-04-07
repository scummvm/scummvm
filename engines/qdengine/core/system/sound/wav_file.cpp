/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include <mmsystem.h>

#include "wav_file.h"
#include "wav_sound.h"

#include "qd_file_manager.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */

LRESULT PASCAL wav_IO_proc(LPSTR lpmmioinfo,UINT wMsg,LPARAM lParam1,LPARAM lParam2);

/* --------------------------- DEFINITION SECTION --------------------------- */

bool ReadWaveFormat (const HMMIO hmmio,const char * sFileName,WAVEFORMATEX* pwfx,int* iDataSize,MMCKINFO* ckRiff)
{
	// read 'RIFF' chunk
	if(mmioDescend(hmmio,ckRiff,NULL,0) != 0){
//		Log("(winFormatWAVE_c::Import): can't find 'RIFF' chunk, file %s",sFileName);
		return false;
	}

	// check to make sure this is a valid wave file
	if(ckRiff -> ckid != FOURCC_RIFF || ckRiff -> fccType != mmioFOURCC ('W','A','V','E')){
//		Log("(winFormatWAVE_c::Import): invalid 'RIFF' chunk, file %s",sFileName);
		return false;
	}

	// search the input file for for the 'fmt ' chunk.
	MMCKINFO ckInfo;
	ckInfo.ckid = mmioFOURCC('f','m','t',' ');
	if(mmioDescend(hmmio,&ckInfo,ckRiff,MMIO_FINDCHUNK ) != 0){
//		Log("(winFormatWAVE_c::Import): can't find 'fmt ' chunk, file %s",sFileName);
		return false;
	}

	// expect the 'fmt ' chunk to be at least as large as <PCMWAVEFORMAT>.
	// if there are extra parameters at the end, we'll ignore them.
	if(ckInfo.cksize < static_cast <LONG>(sizeof(PCMWAVEFORMAT))){
//		Log("(winFormatWAVE_c::Import): invalid 'fmt ' chunk, file %s",sFileName);
		return false;
	}

	// read the 'fmt ' chunk
	if(mmioRead(hmmio,reinterpret_cast <HPSTR>(pwfx),sizeof(PCMWAVEFORMAT)) != sizeof(PCMWAVEFORMAT)){
//		Log("(winFormatWAVE_c::Import): can't read information from 'fmt ' chunk, file %s",sFileName);
		return false;
	}

	pwfx -> cbSize = 0;      // ignore extra bytes

	// ascend the input file out of the 'fmt ' chunk
	if(mmioAscend(hmmio,&ckInfo,0) != 0){
//		Log("(winFormatWAVE_c::Import): Ascend failed, file %s",sFileName);
		return false;
	}

	// check format tag
	if(pwfx -> wFormatTag != WAVE_FORMAT_PCM){
//		Log("(winFormatWAVE_c::Import): invalid wave format (%i), file %s", static_cast <int> (pwfx->wFormatTag),sFileName);
		return false;
	}

	// check channels
	if(pwfx -> nChannels != 1 && pwfx->nChannels != 2){
//		Log("(winFormatWAVE_c::Import): invalid channels (%i), file %s", static_cast <int> (pwfx->nChannels), sFileName);
		return false;
	}

	// check bits per sample
	if(pwfx -> wBitsPerSample != 8 && pwfx->wBitsPerSample != 16){
//		Log("(winFormatWAVE_c::Import): invalid bits per sample (%i), file %s", static_cast <int> (pwfx->wBitsPerSample), sFileName);
		return false;
	}

	// seek to the data
	if(mmioSeek(hmmio,ckRiff -> dwDataOffset + sizeof(FOURCC),SEEK_SET) == -1){
//		Log("(winFormatWAVE_c::Import): can't seek to the data, file %s", sFileName);
		return false;
	}

	// search the input file for the 'data' chunk.
	MMCKINFO ck;
	ck.ckid = mmioFOURCC('d','a','t','a');
	if(mmioDescend(hmmio,&ck,ckRiff,MMIO_FINDCHUNK) != 0){
//		Log("(winFormatWAVE_c::Import): can't find 'data' chunk, file %s", sFileName);
		return false;
	}

	// check size of sound data
	if(ck.cksize == 0){
//		Log ("(winFormatWAVE_c::Import): invalid data size, file %s", sFileName);
		return false;
	}

	*iDataSize = ck.cksize;

	return true;
}

bool ReadWaveData(const HMMIO hmmio,const char* sFileName,char* pBuffer,int iSize,MMCKINFO* ckRiff)
{
	MMIOINFO info;         // current status of hmmio

	if(mmioGetInfo(hmmio,&info,0) != 0){
//		Log("(winFormatWAVE_c::Import): mmioGetInfo failed, file %s", sFileName);
		return false;
	}

	for(int i = 0; i < iSize; i ++){
		// copy the bytes from the io to the buffer.
		if(info.pchNext == info.pchEndRead){
			if(mmioAdvance(hmmio,&info,MMIO_READ) != 0){
//				Log("(winFormatWAVE_c::Import): mmioAdvance failed, file %s", sFileName);
				return false;
			}
			if(info.pchNext == info.pchEndRead){
//				Log("(winFormatWAVE_c::Import): invalid info.pchNext, file %s", sFileName);
				return false;
			}
		}
		// actual copy
		*((BYTE*)pBuffer + i) = *((BYTE*)info.pchNext);
		info.pchNext ++;
	}

	if(mmioSetInfo(hmmio,&info,0) != 0){
//		Log("(winFormatWAVE_c::Import): mmioSetInfo failed, file %s", sFileName);
		return false;
	}

	return true;
}

bool wav_file_load(const char* fname,class wavSound* snd)
{
	if(!fname)
		return false;

	MMIOINFO inf;
	memset(&inf,0,sizeof(MMIOINFO));

	inf.pIOProc = wav_IO_proc;

	HMMIO hmmio = mmioOpen(const_cast<char*>(fname),&inf,MMIO_ALLOCBUF | MMIO_READ);
	if(hmmio == NULL)
		return false;

	// read the wave format
	WAVEFORMATEX wfx;
	MMCKINFO ckRiff;
	int iDataSize = 0;
 
	memset(&wfx,0,sizeof(wfx));
	if(!ReadWaveFormat(hmmio,fname,&wfx,&iDataSize,&ckRiff)){
		mmioClose(hmmio,0);
		return false;
	}

	// create the wave
	snd -> init(iDataSize,wfx.wBitsPerSample,wfx.nChannels,wfx.nSamplesPerSec);

	// read the wave data
	if(!ReadWaveData(hmmio,fname,snd -> data_,iDataSize,&ckRiff)){
		mmioClose(hmmio,0);
		return false;
	}

	mmioClose(hmmio,0);
	return true;
}

LRESULT PASCAL wav_IO_proc(LPSTR lpmmioinfo,UINT wMsg,LPARAM lParam1,LPARAM lParam2)
{
	MMIOINFO* inf = (MMIOINFO*)lpmmioinfo;

	static XZipStream fh;

	switch(wMsg){
	case MMIOM_OPEN:
		if(!qdFileManager::instance().open_file(fh, (const char*)lParam1, false))
			return MMSYSERR_ERROR;
		
		return MMSYSERR_NOERROR;
	case MMIOM_CLOSE:
		fh.close();
		return 0;
	case MMIOM_READ:
		fh.read((void*)lParam1,lParam2);
		inf -> lDiskOffset = fh.tell();
		return lParam2;
	case MMIOM_WRITE:
		return 0;
	case MMIOM_SEEK:
		switch(lParam2){
		case SEEK_CUR:
			fh.seek(lParam1, XS_CUR);
			inf -> lDiskOffset = fh.tell();
			return fh.tell();
		case SEEK_SET:
			fh.seek(lParam1, XS_BEG);
			inf -> lDiskOffset = fh.tell();
			return fh.tell();
		case SEEK_END:
			fh.seek(lParam1, XS_END);
			inf -> lDiskOffset = fh.tell();
			return fh.tell();
		}
		assert(0);
		return -1;
	}

	return 0;
}
