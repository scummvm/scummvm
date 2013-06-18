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
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */

/*$I c:\sleep5\DSMI.INC*/ #include "graph.h"
/*#include "Crt.h"*/

namespace Avalanche {

const varying_string<255> song =
    string("Golden slumbers kiss your eyes/Smiles awake you when you rise/") +
    "Sleep, pretty Baron, do not cry/And I will sing a lullaby.%Care you " +
    "know not, therefore sleep/While I o'er you watch do keep;/Sleep now, " +
    "du Lustie, do not cry/And I will leave the castle.*Bye!";

const integer scardcount = 13;

const array < 0, scardcount - 1, integer > soundcards =
{{1, 2, 6, 3, 4, 5, 8, 9, 10, 7, 7, 7, 7}};

const array<1, 5, byte> holding = {
	{
		24, /* 0 : 24 */
		64, /* 1 : 00 */
		128, /* 2 : 00 */
		152, /* 2 : 24 */
		170
	}
};    /* 2 : 42 */

integer gd, gm;
byte fv;
word *skellern;
word s, o;
boolean firstverse;
word nexthangon;

boolean nomusic;

integer getsoundhardware(psoundcard scard) {
	integer sc, i, autosel, select;
	char ch;
	integer e;



	integer getsoundhardware_result;
Lagain:
	sc = detectgus(scard);
	if (sc != 0)  sc = detectpas(scard);
	if (sc != 0)  sc = detectaria(scard);
	if (sc != 0)  sc = detectsb(scard);

	/* if no sound card found, zero scard */
	if (sc != 0)  fillchar(scard, sizeof(tsoundcard), 0);

	autosel = -1;
	/*  if sc=0 then
	    for i:=0 to scardcount-1 do
	      if scard^.ID=soundcards[i].ID then begin
	        { Set auto selection mark }
	        autosel:=i+1;
	        break;
	      end;*/

	/* Print the list of sound cards */

	val(paramstr(13), select, e);

	/* Default entry? */
	if (select == 0)  select = autosel;
	if (select != autosel)  {
		/* clear all assumptions */
		sc = -1;
		fillchar(scard, sizeof(tsoundcard), 0);
		scard->id = soundcards[select - 1]; /* set correct ID */
	}

	/* Query I/O address */
	if (scard->id == id_dac)  scard->ioport = 0x378;

	/* Read user input */
	val(paramstr(15), i, e);

	if (i != 0)  scard->ioport = i;
	if (sc != 1)  /* Not autodetected */
		switch (scard->id) {
		case id_sb16:
		case id_pas16:
		case id_wss:
		case id_aria:
		case id_gus    :
			scard->samplesize = 2;
			break;                            /* 16-bit card */
		case id_sbpro:
		case id_pas:
		case id_pasplus:
			scard->stereo = true;
			break;                            /* enable stereo */
		default: {
			scard->samplesize = 1;
			scard->stereo = false;
		}
		}

	if (scard->id != id_dac)  {
		val(paramstr(17), i, e);

		if (i != 0)  scard->dmairq = i;

		val(paramstr(16), i, e);

		if (i != 0)  scard->dmachannel = i;
	} else {
		/* Select correct DAC */
		scard->maxrate = 44100;
		if (select == 11)  {
			scard->stereo = true;
			scard->dmachannel = 1;  /* Special 'mark' */
			scard->maxrate = 60000;
		} else if (select == 12)  {
			scard->stereo = true;
			scard->dmachannel = 2;
			scard->maxrate = 60000;
			if (scard->ioport == 0)  scard->ioport = 0x378;
		} else if (select == 13)  {
			scard->dmachannel = 0;
			scard->ioport = 0x42;   /* Special 'mark' */
			scard->maxrate = 44100;
		}
	}

	/*    writeln('Your selection: ',select,' at ',scard^.ioPort,
	            ' using IRQ ',scard^.dmaIRQ,' and DMA channel ',scard^.dmaChannel);
	  readln;*/

	getsoundhardware_result = 0;
	return getsoundhardware_result;
}

byte here() {
	byte here_result;
	here_result = (ampgetpattern % 3) * 64 + ampgetrow;
	return here_result;
}

void hold(word amount) {
	*skellern = 0;
	do {
		;
	} while (!(*skellern >= amount));
}

void hangon(word forwhat) {
	if (nomusic)
		hold(40);
	else
		do {
			if (keypressed())  exit(0);
		} while (!(here() >= holding[forwhat]));
}

tsoundcard scard;
tmcpstruct mcpstrc;
tdds dds;
pmodule module;
tsdi_init sdi;
integer e,
        bufsize;
char ch;
boolean v86,
        vdsok;
longint a, rate,
        tempseg;
string answer;
pointer temp;
word flags;
word curch;
byte modulevolume;
array<0, 4, tsampleinfo> sample;
array<0, 31, word> voltable;

int main(int argc, const char *argv[]) {
	pio_initialize(argc, argv);
	for (e = 1; e <= paramcount; e ++) answer = paramstr(e);

	nomusic = paramstr(13) == '0';

	if (! nomusic) {
		/* Read sound card information */
		if (getsoundhardware(&scard) == -1)  exit(1);


		/* Initialize Timer Service */
		tsinit;
		atexit(&tsclose);
		if (scard.id == id_gus)  {
			/* Initialize GUS player */
#ifndef DPMI
			scard.extrafield[2] = 1; /* GUS DMA transfer does not work in V86 */
#endif
			gusinit(&scard);
			atexit(&gusclose);

			/* Initialize GUS heap manager */
			gushminit;

			/* Init CDI */
			cdiinit;

			/* Register GUS into CDI */
			cdiregister(&cdi_gus, 0, 31);

			/* Add GUS event player engine into Timer Service */
			tsaddroutine(&gusinterrupt, gus_timer);
		} else {
			/* Initialize Virtual DMA Specification */
#ifndef DPMI
			vdsok = vdsinit == 0;
#else
			vdsok = false;
#endif

			fillchar(mcpstrc, sizeof(tmcpstruct), 0);

			/* Query for sampling rate */
			val(paramstr(14), a, e);
			if (a > 4000)  rate = a;
			else rate = 21000;

			/* Query for quality */
			mcpstrc.options = mcp_quality;

			switch (scard.id) {
			case id_sb     : {
				sdi = sdi_sb;
				scard.maxrate = 22000;
			}
			break;
			case id_sbpro  : {
				sdi = sdi_sbpro;
				scard.maxrate = 22000;
			}
			break;
			case id_pas:
			case id_pasplus:
			case id_pas16  : {
				sdi = sdi_pas;
				scard.maxrate = 44100;
			}
			break;
			case id_sb16   : {
				sdi = sdi_sb16;
				scard.maxrate = 44100;
			}
			break;
			case id_aria   : {
				sdi = sdi_aria;
				scard.maxrate = 44100;
			}
			break;
			case id_wss    : {
				sdi = sdi_wss;
				scard.maxrate = 48000;
			}
			break;
#ifndef DPMI
			case id_dac    :
				sdi = sdi_dac;
				break; /* Only available in real mode */
#endif
			}

			mcpinitsounddevice(sdi, &scard);
			a = mcp_tablesize;
			mcpstrc.reqsize = 0;

			/* Calculate mixing buffer size */
			bufsize = (longint)(2800 * (integer)(scard.samplesize) << (byte)(scard.stereo)) *
			          (longint)(rate) / (longint)(22000);
			mcpstrc.reqsize = 0;
			if ((mcpstrc.options & mcp_quality) > 0)
				if (scard.samplesize == 1)  a += mcp_qualitysize;
				else
					a = mcp_tablesize16 + mcp_qualitysize16;
			if ((longint)(bufsize) + (longint)(a) > 65500)  bufsize = longint(65500) - a;

#ifdef DPMI
			dpmiversion((byte)(e), (byte)(e), (byte)(e), flags);
			v86 = (flags & 2) == 0;
#endif

			/* Allocate volume table + mixing buffer */
#ifdef DPMI

			/* In the V86 mode, the buffer must be allocated below 1M */
			if (v86)  {
				tempseg = 0;
				dpmiallocdos((a + longint(bufsize)) / longint(16) + longint(1), flags, (word)(tempseg));
			} else {
#endif
				getmem(temp, a + longint(bufsize));
				if (temp == nil)  exit(2);
#ifdef DPMI
				tempseg = seg(temp);
			}
#else
				tempseg = seg(temp) + ofs(temp) / 16 + 1;
#endif
			mcpstrc.bufferseg = tempseg;
			mcpstrc.bufferphysical = -1;

			if (vdsok && (scard.id != id_dac))  {
				dds.size = bufsize;
				dds.segment = tempseg;
				dds.offset = 0;

				/* Lock DMA buffer if VDS present */
				if (vdslockdma(&dds) == 0)  mcpstrc.bufferphysical = dds.address;
			}
			if (mcpstrc.bufferphysical == -1)
#ifdef DPMI
				mcpstrc.bufferphysical = dpmigetlinearaddr(tempseg);
#else
				mcpstrc.bufferphysical = (longint)(tempseg) << 4;
#endif

			mcpstrc.buffersize = bufsize;
			mcpstrc.samplingrate = rate;
			/* Initialize Multi Channel Player */
			if (mcpinit(&mcpstrc) != 0)  exit(3);
			atexit(&mcpclose);

			/* Initialize Channel Distributor */
			cdiinit;

			/* Register MCP into CDI*/
			cdiregister(&cdi_mcp, 0, 31);
		}

		/* Try to initialize AMP */
		if (ampinit(0) != 0)  exit(3);
		atexit(&ampclose);

		/* Hook AMP player routine into Timer Service */
		tsaddroutine(&ampinterrupt, amp_timer);

#ifndef DPMI
		/* If using DAC, then adjust DAC timer */
		if (scard.id == id_dac)  setdactimer(tsgettimerrate);
#endif

		if (scard.id != id_gus)  mcpstartvoice;
		else gusstartvoice;

		/* Load an example AMF */
		module = amploadmod("golden.mod", 0);
		if (module == nil)  exit(4);

		/* Is it MCP, Quality mode and 16-bit card? */
		if ((scard.id != id_gus) && ((mcpstrc.options & mcp_quality) > 0)
		        && (scard.samplesize == 2))  {
			/* Open module+2 channels with amplified volumetable (4.7 gain) */
			for (a = 1; a <= 32; a ++) voltable[a - longint(1)] = a * longint(150) / longint(32);
			cdisetupchannels(0, module->channelcount + 2, &voltable);
		} else {
			/* Open module+2 channels with regular volumetable */
			cdisetupchannels(0, module->channelcount + 2, nil);
		}

		curch = module->channelcount;
		modulevolume = 64;

		/***/ ampplaymodule(module, 0);
	}

	val(paramstr(2), s, e);
	if (e != 0)  exit(0);
	val(paramstr(3), o, e);
	if (e != 0)  exit(0);
	skellern = ptr(s, o + 1);

	gd = 3;
	gm = 0;
	initgraph(gd, gm, "");

	if (! nomusic)  do {
			;
		} while (!(ampgetrow >= 10));

	setcolor(9);
	for (gd = 0; gd <= 320; gd ++) {
		rectangle(320 - gd, 100 - gd / 2, 320 + gd, 100 + gd / 2);
	}


	gd = 50;
	gm = 20;
	firstverse = true;

	hangon(1);
	nexthangon = 2;
	for (fv = 1; fv <= 255; fv ++) {
		switch (song[fv]) {
		case '/': {
			gd = 50;
			gm += 15;
			hangon(nexthangon);
			nexthangon += 1;
		}
		break;

		case '%': {
			gd = 50;
			gm += 35;
			if (nomusic)
				hold(15);
			else
				do {
					;
				} while (!(ampgetpattern > 2));
			nexthangon = 2;
			hangon(1);
		}
		break;

		case '*': {
			gd += 24;
			hangon(5);
		}
		break;

		default: {
			setcolor(1);
			outtextxy(gd + 1, gm + 1, song[fv]);
			setcolor(0);
			outtextxy(gd , gm , song[fv]);
			gd += 12;
		}
		}
		if (song[fv] == ' ')  hold(1);
		if (keypressed())  exit(0);
	}

	if (nomusic)
		hold(25);
	else
		do {
			;
		} while (!(ampgetmodulestatus != md_playing));

	setcolor(0);
	for (gd = 320; gd >= 0; gd --) rectangle(320 - gd, 100 - gd / 2, 320 + gd, 100 + gd / 2);
	return EXIT_SUCCESS;
}

} // End of namespace Avalanche.