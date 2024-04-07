//Balmer
#include "qd_precomp.h"
#include <windows.h>
#include <dsound.h>
#include "PlayOgg.h"
#include <vorbis/vorbisfile.h>

#ifdef MPP_STAT
#include <xutil.h>
#endif MPP_STAT

#include <stdio.h>
#include <math.h>

#include <aclapi.h>

#define BLK_SIZE         (36 * 32)
void MpegCreateWindowTable();

static LPDIRECTSOUND g_pDS=NULL;
const maximal_len=BLK_SIZE*2;

static HANDLE hWaitEvent=INVALID_HANDLE_VALUE;
static HANDLE hThread=INVALID_HANDLE_VALUE;
static int b_thread_must_stop=0;
static bool b_pause_if_null_volume=false;
MpegSound* pFirstSound=NULL;

//#define MPEG_PROFILE
#ifdef MPEG_PROFILE
#include <stdio.h>
FILE* mpeg_error=NULL;
void mprintf(char *format, ...)
{
  if(mpeg_error==NULL)return;
  va_list args;
  char    buffer[512];
  va_start(args,format);
  vsprintf(buffer,format,args);
  fprintf(mpeg_error,buffer);
}
#endif MPEG_PROFILE

#ifdef MPP_STAT

static double all_time=0,mpeg_time=0;

static int time_index=0;
static double prev_time=0,sum_mpeg_time=0;

const MPP_BUF_SIZE=8192;
class MppLoad
{
	char buffer[MPP_BUF_SIZE];
    OggVorbis_File vf;
	FILE* file;
	int bitstream;
	int channels;//0-mono,1-stereo
	float time_len;
public:
	enum MppDeferredRet
	{
		MDR_ERROR=0,
		MDR_WAIT=1,
		MDR_OK=2,
	};

	MppLoad()
	{
		file=NULL;
		bitstream=0;
		channels=2;
		time_len=0.0f;
	}
	~MppLoad()
	{
		Close();
	}

	bool Open(const char* fname)
	{
		Close();
		if(!fname || fname[0]==0)
			return false;
		file=fopen(fname,"rb");
		if(file==NULL)
			return false;
		if(ov_open(file, &vf, NULL, 0) < 0)
		{
			fclose(file);
			file=NULL;
			return false;
		}

		vorbis_info* info=ov_info(&vf,-1);
		channels=info->channels;
		time_len=(float)ov_time_total(&vf,bitstream);
		return true;
	}

	void Close()
	{
		if(file)
		{
			ov_clear(&vf);
			fclose(file);
			file=NULL;
			bitstream=0;
		}
	}

	//len - величнна буффера buffer в short
	bool GetNextFrame(short*& buffer_,int& len)
	{
		int ret = ov_read(&vf, buffer, MPP_BUF_SIZE, 0, 2, 1, &bitstream);
		len=ret;
		buffer_=(short*)buffer;
		return ret>0;
	}

	float GetLen()
	{
		return time_len;
	}

	float GetCurPos()
	{
		return (float)ov_time_tell(&vf);
	}

	int GetChannels()
	{
		return channels;
	}
};

double MpegCPUUsing()
{
	if(all_time<0.1)
		return 0;
	return mpeg_time/all_time;
}
#endif MPP_STAT

class EWait
{
public:
	EWait()
	{
		if(hWaitEvent==INVALID_HANDLE_VALUE)return;
		WaitForSingleObject(hWaitEvent,INFINITE);
	}
	~EWait()
	{
		if(hWaitEvent==INVALID_HANDLE_VALUE)return;
		SetEvent(hWaitEvent);
	}
};


#define DB_MIN		-10000
#define DB_MAX		0
#define DB_SIZE 	10000

static int FromDirectVolume(long vol)
{
	double v=exp((exp(((vol-DB_MIN)/(double)DB_SIZE)*log(10.0f))-1.0)*log(2.0)*8/9.0)-1;
	
	return (int)(v+0.5);
}

static long ToDirectVolume(int vol)
{

	int v = DB_MIN + (int)(0.5+
		log10(
			  9.0*log(double(vol + 1))/(log(2.0)*8) + 1.0
			 )*DB_SIZE
		);
	return v;
}

void MpegSetPauseIfNullVolume(bool set)
{
	b_pause_if_null_volume=set;
}

__int64 tick_per_sec2=0;
__int64 beg_tick2=0;

__declspec (noinline)
__int64 getRDTSC2()
{
	#define RDTSC __asm _emit 0xf __asm _emit 0x31
	__int64 timeRDTS;
	__asm {
		push ebx
		push ecx
		push edx
		RDTSC
		mov dword ptr [timeRDTS],eax
		mov dword ptr [timeRDTS+4],edx
		pop edx
		pop ecx
		pop ebx
	}
	return timeRDTS;
}

double clockf()
{
	return (double)(getRDTSC2()-beg_tick2)/(double)tick_per_sec2;
} 

DWORD WINAPI MpegThreadProc(LPVOID lpParameter)
{
	SetThreadPriority(hThread,THREAD_PRIORITY_TIME_CRITICAL);

	prev_time=clockf();

	while(b_thread_must_stop==0)
	{
#ifdef MPP_STAT
		time_index++;
		if(time_index%400==0)
		{
			all_time=clockf()-prev_time;
			mpeg_time=sum_mpeg_time;
			sum_mpeg_time=0;
			prev_time=clockf();
		}
#endif MPP_STAT
		{
			EWait w;
#ifdef MPP_STAT
			double tbeg=clockf();
#endif MPP_STAT

#ifdef MPEG_PROFILE
			static int cur_quant=0;
			mprintf("%i: ",cur_quant++);
#endif MPEG_PROFILE
			for(MpegSound* cur=pFirstSound;cur;cur=cur->next)
			{
				cur->TimeCallbackTrue();
			}

#ifdef MPEG_PROFILE
			mprintf("\n");
#endif MPEG_PROFILE
#ifdef MPP_STAT
			sum_mpeg_time+=clockf()-tbeg;
#endif MPP_STAT
		}

		Sleep(10);
	}

	b_thread_must_stop=2;
	return 0;
}

bool MpegInitLibrary(void* pDS)
{
	MpegCreateWindowTable();
#ifdef MPEG_PROFILE
	mpeg_error=fopen("mpeg_info.txt","w");
#endif MPEG_PROFILE
	b_thread_must_stop=0;
#ifdef MPP_STAT
//	initclock();
#endif MPP_STAT
	g_pDS=(LPDIRECTSOUND)pDS;
	return true;
}

void MpegDeinitLibrary()
{
	{
		EWait w;
		for(MpegSound* cur=pFirstSound;cur;cur=cur->next)
		{
			cur->InternalMpegStop();
			if(cur->pDSBuffer)cur->pDSBuffer->Release();
			cur->pDSBuffer=NULL;
		}
	}


	if(hThread!=INVALID_HANDLE_VALUE)
	{
		b_thread_must_stop=1;
		while(b_thread_must_stop==1)
			Sleep(10);
	}

	if(hWaitEvent!=INVALID_HANDLE_VALUE)
		CloseHandle(hWaitEvent);
	hWaitEvent=INVALID_HANDLE_VALUE;
	hThread=INVALID_HANDLE_VALUE;

	g_pDS=NULL;
}

//////////////////////MpegSound////////////////////////////////
MpegSound::MpegSound()
{
	EWait w;
	prev=NULL;
	next=pFirstSound;
	pFirstSound=this;
	if(next)next->prev=this;
	////
	//Подходить к изменению sizeDSBuffer очень осторожно
	//увеличение его может сказаться на интерактивности 
	//уменьшение - на заикании звука 
	sizeDSBuffer=128*1024;//256*1024;
	volume=255;
	b_cycled=false;
	pDSBuffer=NULL;

	clear_end_buffer=false;
	mpeg_state=MPEG_STOP;

	dwWriteOffset=0;
	Wraps=OldWraps=0;
	BeginBufferOffset=OldBeginBufferOffset=0;
	OffsetBeginPlayFile=0;
	SeekSkipByte=0;

	pMppLoad=new MppLoad;

	last_signal_is_full=false;
	last_signal_offset=0;
	enable_fade=false;
	fade_begin_time=fade_time=0;
	fade_begin_volume=fade_end_volume=0;
	fname[0]=0;
	memset(&wave_format,0,sizeof(wave_format));
}

MpegSound::~MpegSound()
{
	EWait w;
	//
	if(prev)prev->next=next;
	else    pFirstSound=next;
	if(next)next->prev=prev;
	//
	if(pDSBuffer)pDSBuffer->Release();
	delete pMppLoad;
}

void MpegSound::InternalMpegSetVolume(int _volume)
{
	volume=_volume;
	if(pDSBuffer)
	{
		HRESULT hr;
		long ddvol=ToDirectVolume(_volume);
		hr=pDSBuffer->SetVolume(ddvol);
		
		if(b_pause_if_null_volume && mpeg_state==MPEG_PLAY)
		{
			DWORD status;
			if(pDSBuffer->GetStatus(&status)==DS_OK)
			{
				bool b_play=(status&DSBSTATUS_PLAYING)?true:false;

				if(volume==0)
				{
					if(b_play)pDSBuffer->Stop();
				}else
				{
					if(!b_play)pDSBuffer->Play(0,0,DSBPLAY_LOOPING);
				}
			}
		}
	}
}

bool MpegSound::DebugRealPlay()
{
	if(pDSBuffer==NULL)return false;
	EWait w;
	DWORD status;
	if(pDSBuffer->GetStatus(&status)==DS_OK)
	{
		return (status&DSBSTATUS_PLAYING)?true:false;
	}

	return false;
}

bool MpegSound::InitSoundBuffer()
{

	if(pDSBuffer)
	{
		if(wave_format.nChannels==pMppLoad->GetChannels())
		{
			return true;
		}
	}

	HRESULT hr;
	/*
		Здесь создавать DirectSoundBuffer
	*/
	WAVEFORMATEX&  wfx=wave_format;

	wfx.wFormatTag=WAVE_FORMAT_PCM;
	wfx.nChannels=pMppLoad->GetChannels();
	wfx.nSamplesPerSec=44100;
    wfx.nAvgBytesPerSec=44100*2*wfx.nChannels;
    wfx.nBlockAlign=2*wfx.nChannels; 
    wfx.wBitsPerSample=16;
    wfx.cbSize=0;

    DSBUFFERDESC dsbd;
    ZeroMemory( &dsbd, sizeof(DSBUFFERDESC) );
    dsbd.dwSize        = sizeof(DSBUFFERDESC);
    dsbd.dwFlags       = //DSBCAPS_CTRLPAN | DSBCAPS_CTRLFREQUENCY | 
						DSBCAPS_CTRLVOLUME |DSBCAPS_GETCURRENTPOSITION2;
    dsbd.dwBufferBytes = sizeDSBuffer;
    dsbd.lpwfxFormat   = &wfx;

    // Create the static DirectSound buffer using the focus set by the UI
    if( FAILED( hr = g_pDS->CreateSoundBuffer( &dsbd, &pDSBuffer, NULL ) ) )
	{
		pDSBuffer=NULL;
        return false;
	}

	InternalMpegSetVolume(volume);

	return true;
}

void MpegSound::InternalMpegStop()
{
	mpeg_state=MPEG_STOP;
	if(g_pDS==NULL)return;
	if(pDSBuffer)
		pDSBuffer->Stop();
}

bool MpegSound::InternalMpegOpenToPlay(const char* _fname,bool cycled)
{
	if(_fname==NULL)return false;
	b_cycled=cycled;
	if(fname!=_fname)
		strcpy(fname,_fname);

//	if(mpeg_state==MPEG_PLAY)return MpegOpen(fname);

	bool is_initialize=pMppLoad->Open(fname);
	InternalMpegStop();
	if(!is_initialize)
		return false;
	if(g_pDS==NULL)return false;

	if(!InitSoundBuffer())return false;
	InternalMpegStop();
	Sleep(20);

	OldWraps=Wraps=0;
	BeginBufferOffset=OldBeginBufferOffset=0;
	dwWriteOffset=0;
	OffsetBeginPlayFile=0;
	SeekSkipByte=0;

	last_signal_is_full=false;
	last_signal_offset=0;

	int n=sizeDSBuffer/2/(wave_format.nChannels*maximal_len);
	for(int i=0;i<n;i++)
	{
		short* pData;
		int len;
		if(!pMppLoad->GetNextFrame(pData,len))
			break;

		BYTE *AudioPtr1,*AudioPtr2;
		DWORD AudioBytes1,AudioBytes2;

		if(FAILED(pDSBuffer->Lock(
			  dwWriteOffset,
			  len,
			  (LPVOID*)&AudioPtr1,
			  &AudioBytes1,
			  (LPVOID*)&AudioPtr2,
			  &AudioBytes2,
			  0
			)))
			return false;

		if(AudioBytes1+AudioBytes2==(DWORD)len)
		{
			memcpy(AudioPtr1,pData,AudioBytes1);
			memcpy(AudioPtr2,pData+AudioBytes1,AudioBytes2);
		}


		pDSBuffer->Unlock(
			(LPVOID)AudioPtr1,
			AudioBytes1,
			(LPVOID)AudioPtr2,
			AudioBytes2);

		dwWriteOffset=(dwWriteOffset+len)%sizeDSBuffer;
	}

	ClearFade();
	if(pDSBuffer && i>0)
	{
		pDSBuffer->SetCurrentPosition(0);
		InternalMpegSetVolume(volume);

		if(b_pause_if_null_volume && volume==0)
			pDSBuffer->Stop();
		else
			pDSBuffer->Play(0,0,DSBPLAY_LOOPING);
	}

	if(is_initialize)
		mpeg_state=MPEG_PLAY;
	return is_initialize;
}

void MpegSound::AddWriteOffset(DWORD offset)
{
	dwWriteOffset=dwWriteOffset+offset;
	if(dwWriteOffset>=sizeDSBuffer)
	{
		DWORD add_wrap=dwWriteOffset/sizeDSBuffer;
		Wraps+=add_wrap;
		OldWraps+=add_wrap;
		dwWriteOffset=dwWriteOffset%sizeDSBuffer;
	}
}

void MpegSound::TimeCallbackTrue()
{
	if(pDSBuffer==NULL)return;

	FadeQuant();

	DWORD dwPlayCursor,dwWriteCursor;
	short* pData;
	int len;

	int num_get_bytes=0;
Retry:

	if(FAILED(pDSBuffer->GetCurrentPosition(
			&dwPlayCursor,  
			&dwWriteCursor  
		)))
		return;

	if(dwPlayCursor<=dwWriteOffset)
		dwPlayCursor+=sizeDSBuffer;

	if(dwWriteOffset+maximal_len*wave_format.nChannels>=dwPlayCursor ||
		num_get_bytes>2*maximal_len*wave_format.nChannels)
	{
#ifdef MPEG_PROFILE
		mprintf("%i ",num_get_sample);
#endif MPEG_PROFILE
//		OutputDebugString(temp_buf);
		return;
	}

	if(pMppLoad->GetNextFrame(pData,len))
	{
		num_get_bytes+=len;

		HammingCorrect(pData,len/wave_format.nChannels);

		DWORD cur_write_byte=len;
		if(cur_write_byte>SeekSkipByte)
			cur_write_byte-=SeekSkipByte;

		BYTE *AudioPtr1,*AudioPtr2;
		DWORD AudioBytes1,AudioBytes2;

		if(FAILED(pDSBuffer->Lock(
			  dwWriteOffset,
			  cur_write_byte,
			  (LPVOID*)&AudioPtr1,
			  &AudioBytes1,
			  (LPVOID*)&AudioPtr2,
			  &AudioBytes2,
			  0
			)))
		{
//			OutputDebugString("Failed\n");
			SeekSkipByte=0;
			return;
		}

		if(AudioBytes1+AudioBytes2==cur_write_byte)
		{
			if(AudioPtr1)memcpy(AudioPtr1,
						SeekSkipByte+(BYTE*)pData,AudioBytes1);
			if(AudioPtr2)memcpy(AudioPtr2,
						AudioBytes1+SeekSkipByte+(BYTE*)pData,AudioBytes2);
		}

		pDSBuffer->Unlock((LPVOID)AudioPtr1,AudioBytes1,(LPVOID)AudioPtr2,AudioBytes2);

		SeekSkipByte=0;

		AddWriteOffset(cur_write_byte);

		clear_end_buffer=true;
		goto Retry;
	}

#ifdef MPEG_PROFILE
	mprintf("%i ",num_get_sample);
#endif MPEG_PROFILE

	if(clear_end_buffer && !b_cycled)
	{//Очистить конец буфера
		clear_end_buffer=false;

		BYTE *AudioPtr1,*AudioPtr2;
		DWORD AudioBytes1,AudioBytes2;

		if(FAILED(pDSBuffer->Lock(
			  dwWriteOffset,
			  maximal_len*wave_format.nChannels,
			  (LPVOID*)&AudioPtr1,
			  &AudioBytes1,
			  (LPVOID*)&AudioPtr2,
			  &AudioBytes2,
			  0
			)))
		{
//			OutputDebugString("Failed\n");
			return;
		}

		if(AudioBytes1+AudioBytes2==(DWORD)maximal_len*wave_format.nChannels)
		{
			if(AudioPtr1)memset(AudioPtr1,0,AudioBytes1);
			if(AudioPtr2)memset(AudioPtr2,0,AudioBytes2);
		}

		pDSBuffer->Unlock((LPVOID)AudioPtr1,AudioBytes1,(LPVOID)AudioPtr2,AudioBytes2);
	}

	if(FAILED(pDSBuffer->GetCurrentPosition(&dwPlayCursor,&dwWriteCursor)))
		return;

	pMppLoad->Close();
	if(b_cycled)
	{
		if(pMppLoad->Open(fname))
		{
			mpeg_state=MPEG_PLAY;
			OldWraps=Wraps;
			OldBeginBufferOffset=BeginBufferOffset;
			BeginBufferOffset=dwWriteOffset;
			Wraps=0;
			OffsetBeginPlayFile=0;
			SeekSkipByte=0;
		}else
			InternalMpegStop();
	}else
	{
//		if(dwPlayCursor>dwWriteOffset || dwPlayCursor+maximal_len*wave_format.nChannels<dwWriteOffset)
//			return;
		int mi=dwPlayCursor;
		int ma=mi+maximal_len*wave_format.nChannels;
		int cmp=dwWriteOffset;
		if(mi>cmp)
			cmp+=sizeDSBuffer;

		if(mi<=cmp&& ma>=cmp)
			InternalMpegStop();
	}
}

bool MpegSound::OpenToPlay(const char* _fname,bool cycled)
{
	Stop();

	bool is_ok;
	{
		EWait w;
		is_ok=InternalMpegOpenToPlay(_fname,cycled);
	}

	if(is_ok)
	{
		if(hThread==INVALID_HANDLE_VALUE)
		{
			b_thread_must_stop=0;
			DWORD ThreadId;
			hThread=CreateThread(
				NULL,
				0,
				MpegThreadProc,
				NULL,
				0,
				&ThreadId);

			hWaitEvent=CreateEvent(NULL,FALSE,TRUE,NULL);
		}
	}

	return is_ok ;
}

void MpegSound::Stop()
{
	EWait w;
	InternalMpegStop();
}

void MpegSound::SetVolume(int _volume)
{
	EWait w;
	ClearFade();
	InternalMpegSetVolume(_volume);
}

int MpegSound::GetVolume()
{
	EWait w;
	return enable_fade?fade_begin_volume:volume; 
}

void MpegSound::Pause()
{
	EWait w;
	if(pDSBuffer==NULL)return;
	pDSBuffer->Stop();
	mpeg_state=MPEG_PAUSE;
}

void MpegSound::Resume()
{
	EWait w;
	if(pDSBuffer==NULL)return;
	if(mpeg_state==MPEG_STOP)return;

	if(b_pause_if_null_volume && volume==0)
		pDSBuffer->Stop();
	else
	if(SUCCEEDED(pDSBuffer->Play(0,0,DSBPLAY_LOOPING)))
	{
	}

	mpeg_state=MPEG_PLAY;
}

MpegState MpegSound::IsPlay()
{
	EWait w;
	if(pDSBuffer==NULL)return MPEG_STOP;
/*
	DWORD status;
	if(FAILED(pDSBuffer->GetStatus(&status)))
		return MPEG_STOP;
*/
	return mpeg_state;//(status&DSBSTATUS_PLAYING)?true:false;
}

double MpegGetLen(const char* fname)
{
    OggVorbis_File vf;
	FILE* in=fopen(fname,"rb");
	if(in==NULL)
		return -1;

    if(ov_open(in, &vf, NULL, 0) < 0) {
        fclose(in);
        return -1;
    }

	double time=ov_time_total(&vf,-1);
    ov_clear(&vf);
    fclose(in);

	return time;
}


int window_hamming[BLK_SIZE];//Окно Хэмминга
const h_shift=14;

void MpegCreateWindowTable()
{
	const int mul=1<<h_shift;
	const double PI=3.14159265358979323846;
	for(int i=0;i<BLK_SIZE;i++)
	{
		double t=double(i)/(2*BLK_SIZE);
		double w=0.5+0.5*cos(2*PI*t);//0.54+0.46*cos(2*PI*t);
		window_hamming[i]=(int)(w*mul);
	}
}

void MpegSound::HammingCorrect(short* pData,int len)
{
	if(!last_signal_is_full)return;

	len=len>>1;

	int max_offset=last_signal_offset+len;
	if(max_offset>BLK_SIZE)max_offset=BLK_SIZE;

	short* out=pData;
	short* in=last_signal+last_signal_offset*2;

	for(int i=last_signal_offset;i<max_offset;i++)
	{
		for(int j=0;j<2;j++)
		{
			int o=//window_hamming[i]<<h_shift;
				*out*window_hamming[BLK_SIZE-i-1]
				+ *in *window_hamming[i];

			o=o>>h_shift;
			if(o>32767)
				o=32767;
			if(o<-32768)
				o=32768;

			*out=o;
			in++;out++;
		}
	}
	
	last_signal_offset=max_offset;
	if(last_signal_offset>=BLK_SIZE)
	{
		last_signal_is_full=false;
		last_signal_offset=0;
	}
}


bool MpegSound::FadeVolume(float time,int new_volume)
{
	if(time<=0.05f)
	{
		ClearFade();
		return false;
	}

	enable_fade=true;
	fade_begin_time=clockf();
	fade_time=time*1000.0f;
	fade_begin_volume=volume;
	fade_end_volume=new_volume;

	return true;
}

void MpegSound::FadeQuant()
{
	if(!enable_fade)return;

	long vol=0;
	double cur_time=(clockf()-fade_begin_time)/fade_time;
	if(cur_time<0)
		vol=fade_begin_volume;
	else
	if(cur_time<1)
	{
		vol=round((1-cur_time)*fade_begin_volume+cur_time*fade_end_volume);
	}else
	{
		vol=fade_end_volume;
		enable_fade=false;
	}

	InternalMpegSetVolume(vol);

	if(!enable_fade)
	{
		volume=fade_begin_volume;
	}
}

void MpegSound::ClearFade()
{
	if(enable_fade)
		volume=fade_begin_volume;

	enable_fade=false;
}

const char* MpegSound::GetFileName()
{
	if(fname[0]==NULL)
		return NULL;

	return fname;
}

float MpegSound::GetLen()
{
	return pMppLoad->GetLen();
}

float MpegSound::GetCurPos()
{
	return pMppLoad->GetCurPos();
}
