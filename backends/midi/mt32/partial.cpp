/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 The ScummVM project
 * Based on Tristan's conversion of Canadacow's code
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

// Implementation of the MT-32 partial class

#include "backends/midi/mt32/synth.h"
#include "backends/midi/mt32/partial.h"

INLINE void CPartialMT32::generateSamples(Bit16s * partialBuf, long length) {
	if (!isActive) return;
	if (alreadyOutputed) return;
	

 	alreadyOutputed = true;

	// Generate samples

	int r;
	int i;
	Bit32s envval, ampval, filtval;
	soundaddr *pOff = &partCache->partialOff;
	int noteval = partCache->keyedval;
	for(i=0;i<length;i++) {
		Bit32s ptemp = 0;

		if(partCache->envs[AMPENV].sustaining) {
			ampval = partCache->ampEnvCache;
		} else {
			if(partCache->envs[AMPENV].count<=0) {
				
				ampval = getAmpEnvelope(partCache,tmppoly);
				isActive = partCache->playPartial;
//TODO: check what is going on here
//				if (ampval < 0) ampval = 0;
				//printf("%d %d\n", (int)ampval, (int)isActive);
				if(!isActive) {
					tmppoly->partActive[timbreNum] = false;
					tmppoly->isActive = tmppoly->partActive[0] || tmppoly->partActive[1] || tmppoly->partActive[2] || tmppoly->partActive[3];
				}
				if(ampval>=128) ampval = 127;

				ampval = amptable[ampval];
				int tmpvel = tmppoly->vel;
				if(tcache->ampenvdir==1) tmpvel = 127-tmpvel;
				ampval = (ampval * ampveltable[tmpvel][(int)tcache->ampEnv.velosens]) >> 8;
			//if(partCache->envs[AMPENV].sustaining)
				partCache->ampEnvCache = ampval;
			} else {
				ampval = partCache->ampEnvCache;
			}
			--partCache->envs[AMPENV].count;
		}
	
		int delta = 0x10707;

		// Calculate Pitch envelope
		int lfoat = 0x1000;
		int pdep;
		if(partCache->pitchsustain) {
			// Calculate LFO position
			// LFO does not kick in completely until pitch envelope sustains
		
			if(tcache->lfodepth>0)  {
				partCache->lfopos++;

				if(partCache->lfopos>=tcache->lfoperiod) partCache->lfopos = 0;
				int lfoatm = (partCache->lfopos << 16) / tcache->lfoperiod;
				
				int lfoatr = sintable[lfoatm];

				lfoat = lfoptable[tcache->lfodepth][lfoatr];
			}
			pdep = partCache->pitchEnvCache;


		} else {
			envval = getPitchEnvelope(partCache,tmppoly);
			int pd=tcache->pitchEnv.depth;
			pdep = penvtable[pd][envval];
			if(partCache->pitchsustain) partCache->pitchEnvCache = pdep;

		}


		// Get waveform - either PCM or synthesized sawtooth or square


		if (tcache->PCMPartial) {
			// PCM partial
			
			if(!partCache->PCMDone) {
				
				int addr,len,tmppcm;
				partialTable *tPCM = &tcache->convPCM;

				if(tPCM->aggSound==-1) {
					delta = wavtabler[tPCM->pcmnum][noteval];
					addr = tPCM->addr;
					len = tPCM->len;

				} else {
					tmppcm = LoopPatterns[tPCM->aggSound][partCache->looppos];
					addr = PCM[tmppcm].addr;
					len = PCM[tmppcm].len;
					delta = looptabler[tPCM->aggSound][partCache->looppos][noteval];
				}

				if(ampval>0) {
					int ra,rb,dist;
					int taddr;
					if(delta<0x10000) {
						// Linear sound interpolation
						
						taddr = addr + pOff->pcmoffs.pcmplace;
						ra = romfile[taddr];
						rb = romfile[taddr+1];

						dist = rb-ra;
						r = (ra + ((dist * (Bit32s)(pOff->pcmoffs.pcmoffset>>8)) >>8));

					} else {
						
						//r = romfile[addr + pOff->pcmoffs.pcmplace];
						// Sound decimation
						// The right way to do it is to use a lowpass filter on the waveform before selecting
						// a point.  This is too slow.  The following appoximates this as fast as possible
						int idelta = delta >> 16;
						int ix;
						taddr = addr + pOff->pcmoffs.pcmplace;
						ra = 0;
						for(ix=0;ix<idelta;ix++) {
							ra += romfile[taddr++];
						}
						r = ra / idelta;
						


					}
				} else r = 0;

				ptemp = r;

				if ((pOff->pcmoffs.pcmplace) >= len) {
					if(tPCM->aggSound==-1) {
						if(tPCM->loop) {
							pOff->pcmabs = 0;
						} else {
							partCache->PCMDone = true;
							partCache->playPartial = false;
						}

					} else {
						partCache->looppos++;
						if(LoopPatterns[tPCM->aggSound][partCache->looppos]==-1) partCache->looppos=0;
						pOff->pcmabs = 0;
					}
					//LOG_MSG("tPCM %d loops %d done %d playPart %d", tPCM->pcmnum, tPCM->loop, partCache->PCMDone, partCache->playPartial);

				}


			}

		} else {
			// Synthesis partial
			int divis, hdivis, ofs, ofs3, toff;
			int minorplace;

			int wf = tcache->waveform ;

			divis = divtable[noteval]>>15;

			if(pOff->pcmoffs.pcmplace>=divis) pOff->pcmoffs.pcmplace = (Bit16u)(pOff->pcmoffs.pcmplace-divis);
			
			toff = pOff->pcmoffs.pcmplace;
			minorplace = pOff->pcmoffs.pcmoffset >> 14;
			
			int pa, pb;

			if(ampval>0) {

				filtval = getFiltEnvelope((Bit16s)ptemp,partCache,tmppoly);

				//LOG_MSG("Filtval: %d", filtval);
				
				if(wf==0) {
					// Square waveform.  Made by combining two pregenerated bandlimited
					// sawtooth waveforms
					// Pulse width is not yet correct
					
					hdivis = divis >> 1;
					int divmark = smalldivtable[noteval];
					//int pw = (tcache->pulsewidth * pulsemod[filtval]) >> 8;
					
					
					ofs = toff % (hdivis);

					ofs3 = toff + ((divmark*pulsetable[partCache->pulsewidth])>>16);
					ofs3 = ofs3 % (hdivis);
					
					pa = waveforms[1][noteval][(ofs<<2)+minorplace];
					pb = waveforms[0][noteval][(ofs3<<2)+minorplace];
					//ptemp = pa+pb+pulseoffset[tcache->pulsewidth];
					ptemp = (pa+pb)*4;
										
					// Non-bandlimited squarewave
					/*
					ofs = (divis*pulsetable[tcache->pulsewidth])>>8;
					if(toff < ofs) {
						ptemp = 1 * WGAMP;
					} else {
						ptemp = -1 * WGAMP;
					}*/


				} else {
					// Sawtooth.  Made by combining the full cosine and half cosine according
					// to how it looks on the MT-32.  What it really does it takes the
					// square wave and multiplies it by a full cosine
					// TODO: This area here crashes DosBox due to read overflow                     
					int offsetpos = (toff<<2)+minorplace;
					//int a = 0;
					if(toff < sawtable[noteval][partCache->pulsewidth]) {
						while(offsetpos>waveformsize[2][noteval]) {
							offsetpos-=waveformsize[2][noteval];
						}
						ptemp = waveforms[2][noteval][offsetpos];
					} else {
						while(offsetpos>waveformsize[3][noteval]) {
							offsetpos-=waveformsize[3][noteval];
						}
						ptemp = waveforms[3][noteval][offsetpos];
					}
					ptemp = ptemp *4;
					
					// ptemp = (int)(sin((double)toff / 100.0) * 100.0);
					//ptemp = pa;

					// This is the correct way
					// Seems slow to me (though bandlimited) -- doesn't seem to
					// sound any better though
					/*
					hdivis = divis >> 1;
					int divmark = smalldivtable[noteval];
					//int pw = (tcache->pulsewidth * pulsemod[filtval]) >> 8;
					
					
					ofs = toff % (hdivis);

					ofs3 = toff + ((divmark*pulsetable[tcache->pulsewidth])>>16);
					ofs3 = ofs3 % (hdivis);
					
					pa = waveforms[0][noteval][ofs];
					pb = waveforms[1][noteval][ofs3];
					ptemp = ((pa+pb) * waveforms[3][noteval][toff]) / WGAMP;
					ptemp = ptemp *4;
					*/

					

				}
				
				//Very exact filter
				//ptemp[t] = (int)iir_filter((float)ptemp[t],&partCache->history[t],filtcoeff[filtval][tcache->filtEnv.resonance]);
				if(filtval>((FILTERGRAN*15)/16)) filtval = ((FILTERGRAN*15)/16);
				ptemp = (Bit32s)(usefilter)((float)ptemp,&partCache->history[0],filtcoeff[filtval][(int)tcache->filtEnv.resonance], tcache->filtEnv.resonance);
			} else ptemp = 0;

			//ptemp[t] = Moog1(ptemp[t],&partCache->history[t],(float)filtval/8192.0,tcache->filtEnv.resonance);
			//ptemp[t] = Moog2(ptemp[t],&partCache->history[t],(float)filtval/8192.0,tcache->filtEnv.resonance);
			//ptemp[t] = simpleLowpass(ptemp[t],&partCache->history[t],(float)filtval/8192.0,tcache->filtEnv.resonance);

			// Use this to mute analogue synthesis
			// ptemp = 0;


		}
		
		// Build delta for position of next sample
                /*
		delta = (delta * tcache->fineshift)>>12;
		delta = (delta * pdep)>>12;
		delta = (delta * lfoat)>>12;
		if(tcache->useBender) delta = (delta * *tmppoly->bendptr)>>12;
                */

		// Fix delta code
		__int64 tdelta = (__int64)delta;
		tdelta = (tdelta * tcache->fineshift)>>12;
		tdelta = (tdelta * pdep)>>12;
		tdelta = (tdelta * lfoat)>>12;
		if(tcache->useBender) tdelta = (tdelta * *tmppoly->bendptr)>>12;

		// Add calculated delta to our waveform offset
		pOff->pcmabs+=(int)tdelta;

		// Put volume envelope over generated sample
		ptemp = (ptemp * ampval) >> 9;
		ptemp = (ptemp * *tmppoly->volumeptr) >> 7;
		
		partCache->envs[AMPENV].envpos++;
		partCache->envs[PITCHENV].envpos++;
		partCache->envs[FILTENV].envpos++;
	
		*partialBuf++ = (Bit16s)ptemp;
	}


}

INLINE void CPartialMT32::mixBuffers(Bit16s * buf1, Bit16s *buf2, int len) {
	// Early exit if no need to mix
	if(tibrePair==NULL) return;

#if USE_MMX == 0
	int i;
	for(i=0;i<len;i++) {
		Bit32s tmp1 = buf1[i];
		Bit32s tmp2 = buf2[i];
		tmp1 += tmp2;
		buf1[i] = (Bit16s)tmp1;
	}
#else
	len = (len>>2)+4;
#ifdef I_ASM	
	__asm {
		mov ecx, len
		mov esi, buf1
		mov edi, buf2

mixloop1:
		movq mm1, [edi]
		movq mm2, [esi]
		paddw mm1,mm2
		movq [esi],mm1
		add edi,8
		add esi,8

		dec ecx
		cmp ecx,0
		jg mixloop1
		emms
	}
#else
	atti386_mixBuffers(buf1, buf2, len);
#endif	
#endif
}

INLINE void CPartialMT32::mixBuffersRingMix(Bit16s * buf1, Bit16s *buf2, int len) {
#if USE_MMX != 2
	int i;
	for(i=0;i<len;i++) {
		float a, b;
		a = ((float)buf1[i]) / 8192.0;
		b = ((float)buf2[i]) / 8192.0;
		a = (a * b) + a;
		if(a>1.0) a = 1.0;
		if(a<-1.0) a = -1.0;
		buf1[i] = (Bit16s)(a * 8192.0);

		//buf1[i] = (Bit16s)(((Bit32s)buf1[i] * (Bit32s)buf2[i]) >> 10) + buf1[i];
        }
#else
	len = (len>>2)+4;
#ifdef I_ASM	
	__asm {
		mov ecx, len
		mov esi, buf1
		mov edi, buf2

mixloop2:
		movq mm1, [esi]
		movq mm2, [edi]
		movq mm3, mm1
		pmulhw mm1, mm2
		paddw mm1,mm3
		movq [esi],mm1
		add edi,8
		add esi,8

		dec ecx
		cmp ecx,0
		jg mixloop2
		emms
	}
#else
	atti386_mixBuffersRingMix(buf1, buf2, len);
#endif	
#endif
}

INLINE void CPartialMT32::mixBuffersRing(Bit16s * buf1, Bit16s *buf2, int len) {
#if USE_MMX != 2
	int i;
	for(i=0;i<len;i++) {
		float a, b;
		a = ((float)buf1[i]) / 8192.0;
		b = ((float)buf2[i]) / 8192.0;
		a *= b;
		if(a>1.0) a = 1.0;
		if(a<-1.0) a = -1.0;
		buf1[i] = (Bit16s)(a * 8192.0);
		//buf1[i] = (Bit16s)(((Bit32s)buf1[i] * (Bit32s)buf2[i]) >> 10);
	}
#else
	len = (len>>2)+4;
#ifdef I_ASM		
	__asm {
		mov ecx, len
		mov esi, buf1
		mov edi, buf2

mixloop3:
		movq mm1, [esi]
		movq mm2, [edi]
		pmulhw mm1, mm2
		movq [esi],mm1
		add edi,8
		add esi,8

		dec ecx
		cmp ecx,0
		jg mixloop3
		emms
	}
#else
	atti386_mixBuffersRing(buf1, buf2, len);
#endif	
#endif
}

INLINE void CPartialMT32::mixBuffersStereo(Bit16s *buf1, Bit16s *buf2, Bit16s *outBuf, int len) {
	int i,m;
	m=0;
	for(i=0;i<len;i++) {
		*outBuf++ = (*buf1);
		buf1++;
		*outBuf++ = (*buf2);
		buf2++;
	}

}

bool CPartialMT32::produceOutput(Bit16s * partialBuf, long length) {
	if (!isActive) return false;
	if (alreadyOutputed) return false;
	int i;

	//alreadyOutputed = true;

	memset(&pairBuffer[0],0,length*4);
	memset(&myBuffer[0],0,length*4);
	// Check for dependant partial
	if(tibrePair != NULL) {
		if ((tibrePair->ownerChan == this->ownerChan) && (!tibrePair->alreadyOutputed)) {			
			tibrePair->generateSamples(pairBuffer,length);
		}
	} else {
		if((useMix!=0) && (useMix != 3)){
			// Generate noise for parialless ring mix
			for(i=0;i<length;i++) pairBuffer[i] = smallnoise[i] << 2;
		}
	}

	generateSamples(myBuffer, length);
/*	FILE *fo = fopen("/tmp/samp.raw", "a");
	for(i = 0; i < length; i++)
		fwrite(myBuffer + i, 1, 2, fo);
	fclose(fo);
*/			
	Bit16s * p1buf, * p2buf;

	if((partNum==0) || ((partNum==1) && (tibrePair==NULL))) {
		p1buf = &myBuffer[0];
		p2buf = &pairBuffer[0];
	} else {
		p2buf = &myBuffer[0];
		p1buf = &pairBuffer[0];
	}
	
//	LOG_MSG("useMix: %d", useMix);
	
	switch(useMix) {
		case 0:
			// Standard sound mix
			mixBuffers(p1buf, p2buf, length);
			break;
		case 1:
			// Ring modulation with sound mix
			mixBuffersRingMix(p1buf, p2buf, length);
			break;
		case 2:
			// Ring modulation alone
			mixBuffersRing(p1buf, p2buf, length);
			break;
		case 3:
			// Stereo mixing.  One partial to one channel, one to another.
			mixBuffersStereo(p1buf, p2buf, partialBuf, length);
			return true;
			break;
		default:
			mixBuffers(p1buf, p2buf, length);
			break;
	}
	
	
	int  m;
	m = 0;	
	Bit16s leftvol, rightvol;
	if (!tmppoly->isRy) {
		leftvol = tmppoly->pansetptr->leftvol;
		rightvol = tmppoly->pansetptr->rightvol;
	} else {
		leftvol = (Bit16s)drumPan[tmppoly->pcmnum][0];
		rightvol = (Bit16s)drumPan[tmppoly->pcmnum][1];
	}

#if USE_MMX == 0
	for(i=0;i<length;i++) {
		partialBuf[m] = (Bit16s)(((Bit32s)p1buf[i] * (Bit32s)leftvol) >> 16);
		m++;
		partialBuf[m] = (Bit16s)(((Bit32s)p1buf[i] * (Bit32s)rightvol) >> 16);
		m++;
	}
#else
	long quadlen = (length >> 1)+2;
#ifdef I_ASM
	__asm {
		mov ecx,quadlen
		mov ax, leftvol
		shl eax,16
		mov ax, rightvol
		movd mm1, eax
		movd mm2, eax
		psllq mm1, 32
		por mm1, mm2
		mov edi, partialBuf
		mov esi, p1buf
mmxloop1:
		mov bx, [esi]
		add esi,2
		mov dx, [esi]
		add esi,2

		mov ax, dx
		shl eax, 16
		mov ax, dx
		movd mm2,eax
		psllq mm2, 32
		mov ax, bx
		shl eax, 16
		mov ax, bx
		movd mm3,eax
		por mm2,mm3

		pmulhw mm2, mm1
		movq [edi], mm2
		add edi, 8

		dec ecx
		cmp ecx,0
		jg mmxloop1
		emms
	}
#else
	atti386_PartProductOutput(quadlen, leftvol, rightvol, partialBuf, p1buf);
#endif	
#endif
	
	return true;
}
