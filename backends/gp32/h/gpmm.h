
#ifndef	__GPMM_H__
#define	__GPMM_H__

/* MIDI related  */
#define GPC_MIDISTATUS_BUSY	1
#define GPC_MIDISTATUS_READY	0
#define GPC_MIDISTATUS_PAUSED 	2

void GpMidiPlay (unsigned char * midisrc,int repeatcount);
void GpMidiListPlay (unsigned char ** srclist,int listcount);
void GpMidiStop (void);
void GpMidiPause (void);
void GpMidiReplay (void);
int GpMidiStatusGet (int * played);

/* PCM related */
#define GPC_EPCM_OK		0
#define GPC_EPCM_FULL		1
#define GPC_EPCM_NO_INIT	2

typedef enum{
	PCM_M11,
	PCM_S11,
	PCM_M22,
	PCM_S22,
	PCM_M44,
	PCM_S44
} PCM_SR;

typedef enum{
	PCM_8BIT,
	PCM_16BIT
} PCM_BIT;

int GpPcmInit (PCM_SR sr, PCM_BIT bit_count);
int GpPcmPlay (unsigned short * src, int size, int repeatflag);
void GpPcmRemove (unsigned short * src);
void GpPcmStop (void);
int GpPcmEnvGet (PCM_SR * p_sr, PCM_BIT * p_bit_count, int * p_real_sr);

int GpPcmLock (unsigned short * p_src, int * idx_buf, unsigned int * addr_of_playing_buf);
void GpPcmOnlyKill (unsigned short * p_src);

#endif	/*__GENERALMIDI_H__*/
