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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * aint32 with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#include "saga2/std.h"
#include "saga2/video.h"

namespace Saga2 {

// ------------------------------------------------------------------
// Make sure we're actually using it

#if USE_SMK

/* ===================================================================== *
   definitions
 * ===================================================================== */

#ifndef _WIN32
#define LOCK_MEM TRUE
#else
#define LOCK_MEM FALSE
#endif

/* ===================================================================== *
   declarations
 * ===================================================================== */

extern char tNameBuffer[];          // filename storage for debugging

char errMsg[80];

/* ===================================================================== *
   VideoPlayer object implementation
 * ===================================================================== */

// ------------------------------------------------------------------
// initialization

#ifndef _WIN32
void VideoPlayer::InitSMK(uint32 maxTimer, bool UseBuffer)
#else
void VideoPlayer::InitSMK(uint32, bool)
#endif
{
	vleft = 128;
	vright = 128;
	changePalette = 1;
	restoreChangePalette = 1;
	dumpStats = FALSE;
	smk = NULL;

	warning("STUB: InitSMK");

#if 0
	if (dig)
		//SetSmackAILDigDriver(dig,maxTimer);
#ifndef _WIN32
		SmackSoundUseMSS(dig, maxTimer);
#else
		SmackSoundUseMSS(dig);
#endif
	else
#ifndef _WIN32
		SmackSoundUseMSS(NULL, maxTimer);
#else
		SmackSoundUseMSS(NULL);
#endif
#ifndef _WIN32
	if (!UseBuffer) {
		//SmackInitSVGA();
		SVGADetect(1);
	}
#endif

#endif
}

// ------------------------------------------------------------------
// cleanup

void VideoPlayer::TermSMK(void) {
}

// ------------------------------------------------------------------
// palette twiddling for Windows

#ifdef _WIN32

LRESULT VideoPlayer::OnPaletteChanged(HWND win_handle, WORD wparam, LONG lparam) {
	if ((HWND)wparam == win_handle)
		return (0L);
	return ((smBuf) ? SmackBufferSetPalette(smBuf) : 0);
}

LRESULT VideoPlayer::OnQueryNewPalette(HWND win_handle, WORD wparam, LONG lparam) {
	return ((smBuf) ? SmackBufferSetPalette(smBuf) : 0);
}

#endif

// ------------------------------------------------------------------
// START command

bool VideoPlayer::StartSMK(char *filespec,
                           uint16 xxx, uint16 yyy,
                           uint32 extraBuf,
                           bool noSkip
                          ) {
	warning("STUB: StartSMK");
	return TRUE;
#if 0

#ifdef _WIN32
	toBuffer = FALSE;
	smBuf = SmackBufferOpen(hWnd, SMACKAUTOBLIT, bufferWidth, bufferHeight, 0, 0);
	if (smBuf == NULL) {
		return FALSE;
	}
#endif
	//smk=SmackOpen(filespec,SMACKTRACKS ,0); //| (noSkip ? SMACKNOSKIP : 0), extraBuf);

	smk = SmackOpen(filespec, SMACKTRACKS | (noSkip ? SMACKNOSKIP : 0), extraBuf);

	if (smk) {
#ifndef _WIN32
		if (toBuffer) {
			buffer = videoAlloc(bufferWidth * bufferHeight, "VidWorkBuf");
			if (buffer == NULL) {
				SmackClose(smk);
				smk = NULL;
				return FALSE;
			}
		}
		if (toBuffer)
			SmackToBuffer(smk, 0, 0, bufferWidth, bufferHeight, buffer, FALSE);
		else {
			SetSmackerSVGA();
			SmackToScreen(smk, xxx, yyy, SVGABytesPS(), SVGAWinTbl(), SVGASetBank());
		}
#else
		SmackToBuffer(smk, (smBuf->Width - smk->Width) / 2, (smBuf->Height - smk->Height) / 2, smBuf->Width, smBuf->Height, smBuf->Buffer, (u8)smBuf->Reversed);
		while (SmackToBufferRect(smk, smBuf->SurfaceType)) {
			SmackBufferBlit(smBuf, 0, 0, 0, smk->LastRectx, smk->LastRecty, smk->LastRectw, smk->LastRecth);
		}
#endif
		videoWidth = smk->Width;
		videoHeight = smk->Height;
		StartFrame = TRUE;
		//SmackVolume(smk,SMACKTRACK1,vleft,vright);
		lvleft = vleft;
		lvright = vright;
		i = 1;
		return TRUE;
	}
	return FALSE;

#endif
}

// ------------------------------------------------------------------
// CHECK command

bool VideoPlayer::CheckSMK(void) {
#if 0
	VERIFY(smk);
	if (abort)
		i = smk->Frames + 1;
	if (i > smk->Frames) {
		EndPlay();
		return FALSE;
	}
	if (StartFrame) {
#ifndef _WIN32
		if (smk->NewPalette && changePalette)
			//setPaletteHook((smk->NewPalette==1)?smk->Col1:smk->Col2);
			setPaletteHook(smk->Palette);
#else
		if (smk->NewPalette) {
			SmackBufferNewPalette(smBuf, smk->Palette, 0);
			SmackColorRemap(smk, smBuf->Palette, smBuf->MaxPalColors, smBuf->PalType);
		}
#endif

		SmackDoFrame(smk);

#ifndef _WIN32
		if (toBuffer)
			bltBuffer();
#else
		while (SmackToBufferRect(smk, smBuf->SurfaceType)) {
			if (SmackBufferBlit(smBuf, 0, 0, 0, smk->LastRectx, smk->LastRecty, smk->LastRectw, smk->LastRecth))
				break;
		}
#endif
		StartFrame = FALSE;
		if (i < smk->Frames)
			SmackNextFrame(smk);
		i++;
	}
	if (lvleft != vleft || lvright != vright) {
		//SmackVolume(smk,SMACKTRACK1,vleft,vright);
		lvleft = vleft;
		lvright = vright;
	}
	if (!SmackWait(smk))
		StartFrame = TRUE;

#endif
	warning("STUB: CheckSMK");
	return TRUE;
}


uint8 blackPal[1024];

// ------------------------------------------------------------------
// END command

void VideoPlayer::EndSMK(void) {
#if 0
	SmackSum    ss;
	FILE        *fp;
	char dNameBuffer[260];
	if (smk) {
		if (!abort) {
			while (SmackWait(smk));
		}
		if (dumpStats) {
			SmackSummary(smk, &ss);
			strcpy(dNameBuffer, tNameBuffer);
			strcpy(dNameBuffer + strlen(dNameBuffer) - 3, "DMP");
			if (NULL != (fp = fopen(dNameBuffer, "wt"))) {
				fprintf(fp, "File       : %s\n", tNameBuffer);
				fprintf(fp, "Total time : %d\n", ss.TotalTime);
				fprintf(fp, "ms/100 frms: %d\n", ss.MS100PerFrame);
				fprintf(fp, "Total frms : %d\n", ss.TotalFrames);
				fprintf(fp, "Skippd frms: %d\n", ss.SkippedFrames);
				fprintf(fp, "Blit Time  : %d\n", ss.TotalBlitTime);
				fprintf(fp, "Read Time  : %d\n", ss.TotalReadTime);
				fprintf(fp, "Decomp Time: %d\n", ss.TotalReadTime);
				fclose(fp);
			}
		}
		memset(blackPal, 1024, '\0');
		setPaletteHook((void *)blackPal);
		SmackClose(smk);
		smk = NULL;
		if (toBuffer) {
			if (buffer)
				videoFree(buffer);
		}
#ifndef _WIN32
		else {
			ClearSmackerSVGA();
		}
#endif
		if (restoreChangePalette)
			changePalette = 1;
	}
#ifdef _WIN32
	if (smBuf) {
		SmackBufferClose(smBuf);
		smBuf = NULL;
	}
#endif
#endif
	warning("STUB: EndSMK");
}

#endif

} // end of namespace Saga2
