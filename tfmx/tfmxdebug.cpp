#include "common/scummsys.h"
#include "common/endian.h"
#include "common/debug.h"
#include "common/stream.h"
#include "common/util.h"

#include "sound/mods/tfmx.h"

#include "tfmx/tfmxdebug.h"


const char *pattcmds[]={
	"End --Next track  step--",
	"Loop[count     / step.w]",
	"Cont[patternno./ step.w]",
	"Wait[count 00-FF--------",
	"Stop--Stop this pattern-",
	"Kup^-Set key up/channel]",
	"Vibr[speed     / rate.b]",
	"Enve[speed /endvolume.b]",
	"GsPt[patternno./ step.w]",
	"RoPt-Return old pattern-",
	"Fade[speed /endvolume.b]",
	"PPat[patt./track+transp]",
	"Lock---------ch./time.b]",
	"----------No entry------",
	"Stop-Stop custompattern-",
	"NOP!-no operation-------"
};

const char *macrocmds[]={
	"DMAoff+Resetxx/xx/xx flag/addset/vol   ",
	"DMAon (start sample at selected begin) ",
	"SetBegin    xxxxxx   sample-startadress",
	"SetLen      ..xxxx   sample-length     ",
	"Wait        ..xxxx   count (VBI''s)    ",
	"Loop        xx/xxxx  count/step        ",
	"Cont        xx/xxxx  macro-number/step ",
	"-------------STOP----------------------",
	"AddNote     xx/xxxx  note/detune       ",
	"SetNote     xx/xxxx  note/detune       ",
	"Reset   Vibrato-Portamento-Envelope    ",
	"Portamento  xx/../xx count/speed       ",
	"Vibrato     xx/../xx speed/intensity   ",
	"AddVolume   ....xx   volume 00-3F      ",
	"SetVolume   ....xx   volume 00-3F      ",
	"Envelope    xx/xx/xx speed/count/endvol",
	"Loop key up xx/xxxx  count/step        ",
	"AddBegin    xx/xxxx  count/add to start",
	"AddLen      ..xxxx   add to sample-len ",
	"DMAoff stop sample but no clear        ",
	"Wait key up ....xx   count (VBI''s)    ",
	"Go submacro xx/xxxx  macro-number/step ",
	"--------Return to old macro------------",
	"Setperiod   ..xxxx   DMA period        ",
	"Sampleloop  ..xxxx   relative adress   ",
	"-------Set one shot sample-------------",
	"Wait on DMA ..xxxx   count (Wavecycles)",
	"Random play xx/xx/xx macro/speed/mode  ",
	"Splitkey    xx/xxxx  key/macrostep     ",
	"Splitvolume xx/xxxx  volume/macrostep  ",
	"Addvol+note xx/fe/xx note/CONST./volume",
	"SetPrevNote xx/xxxx  note/detune       ",
	"Signal      xx/xxxx  signalnumber/value",
	"Play macro  xx/.x/xx macro/chan/detune ",
	"SID setbeg  xxxxxx   sample-startadress",
	"SID setlen  xx/xxxx  buflen/sourcelen  ",
	"SID op3 ofs xxxxxx   offset            ",
	"SID op3 frq xx/xxxx  speed/amplitude   ",
	"SID op2 ofs xxxxxx   offset            ",
	"SID op2 frq xx/xxxx  speed/amplitude   ",
	"SID op1     xx/xx/xx speed/amplitude/TC",
	"SID stop    xx....   flag (1=clear all)"
};

const char *const trackstepFmt[] = {
	"---Stop Player----",
	"Loop step/count   ",
	"Tempo tempo/ciaDiv",
	"Timeshare ?/?     ",
	"Fade start/end    "
	"Unknown (cc)      "
};

void displayPatternstep(const void *const vptr) {
	const byte *const patData = (const byte *const)vptr;

	const byte command = patData[0];

	if (command < 0xF0) { // Playnote
		const byte flags = command >> 6; // 0-1 means note+detune, 2 means wait, 3 means portamento?
		char *flagsSt[] = {"Note ", "Note ", "Wait ", "Porta"};
		debug("%s %02X%02X%02X%02X", flagsSt[flags], patData[0], patData[1], patData[2], patData[3]);
	} else {
		debug("%s %02X%02X%02X",pattcmds[command&0xF], patData[1], patData[2], patData[3]);
	}

}

void displayTrackstep(const void *const vptr) {
	const uint16 *const trackData = (const uint16 *const)vptr;

	if (trackData[0] == FROM_BE_16(0xEFFE)) {
		// 16 byte Trackstep Command
		const uint16 command = READ_BE_UINT16(&trackData[1]);
		const uint16 param1 = READ_BE_UINT16(&trackData[2]);
		const uint16 param2 = READ_BE_UINT16(&trackData[3]);


		if (command >= ARRAYSIZE(trackstepFmt))
			debug("Unknown (%04X)    : %04X %04X", command, param1, param2);
		else
			debug("%s: %04X %04X", trackstepFmt[command], param1, param2);
	} else {
		const byte *const ptr = (const byte *const)vptr;
		// 8 commands for Patterns
		debug("%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X",
			ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5], ptr[6], ptr[7], 
			ptr[8], ptr[9], ptr[10], ptr[11], ptr[12], ptr[13], ptr[14], ptr[15]);
	}
}

void displayMacroStep(const void *const vptr, int chan, int index) {
	const byte *const macroData = (const byte *const)vptr;

	if (macroData[0] < ARRAYSIZE(macrocmds))
		debug("%02X %02X %s %02X%02X%02X", chan, index, macrocmds[macroData[0]], macroData[1], macroData[2], macroData[3]);
	else
		debug("%02X %02X Unkown Macro #%02X %02X%02X%02X", chan, index, macroData[0], macroData[1], macroData[2], macroData[3]);
}

void displayMacroStep(const void *const vptr) {
	const byte *const macroData = (const byte *const)vptr;

	if (macroData[0] < ARRAYSIZE(macrocmds))
		debug("%s %02X%02X%02X", macrocmds[macroData[0]], macroData[1], macroData[2], macroData[3]);
	else
		debug("Unkown Macro #%02X %02X%02X%02X", macroData[0], macroData[1], macroData[2], macroData[3]);
}

void dumpTracksteps(Audio::Tfmx &player, uint16 first, uint16 last) {
	for ( ; first <= last; ++first) {
		displayTrackstep(player._resource.getTrackPtr(first));
	}
}

void dumpTrackstepsBySong(Audio::Tfmx &player, int song) {
	dumpTracksteps(player, player._subsong[song].songstart, player._subsong[song].songend);
}

void dumpMacro(Audio::Tfmx &player, uint16 macroIndex, uint16 len, uint16 start) {
	const uint32 * macroPtr = player._resource.getMacroPtr(player._macroOffset[macroIndex]);
	bool untilMacroStop = (len == 0);
	while (len--) {
		displayMacroStep(macroPtr++);
	}
	while (untilMacroStop) {
		untilMacroStop = *(const byte *)macroPtr != 7;
		displayMacroStep(macroPtr++);
	}
}

void dumpPattern(Audio::Tfmx &player, uint16 pattIndex, uint16 len, uint16 start) {
	const uint32 * pattPtr = player._resource.getPatternPtr(player._patternOffset[pattIndex]);
	if (len == 0)
		len = (player._patternOffset[pattIndex+1] - player._patternOffset[pattIndex])/4;
	bool untilMacroStop = (len == 0);
	while (len--) {
		displayPatternstep(pattPtr++);
	}
	while (untilMacroStop) {
		byte cmd = *(const byte *)pattPtr;
		untilMacroStop = cmd != 0 && cmd != 4;
		displayPatternstep(pattPtr++);
	}
}

void countAllMacros1(Audio::Tfmx &player, uint16 macroIndex, int *list) {
	const uint32 * macroPtr = player._resource.getMacroPtr(player._macroOffset[macroIndex]);
	bool untilMacroStop = true;
	while (untilMacroStop) {
		const int type = *(const byte *)macroPtr++;
		untilMacroStop = type != 7;
		list[type]++;
	}
}

void countAllMacros(Audio::Tfmx &player) {
	int list[256] = {0};
	for (int i = 0; i < 128; ++i)
		countAllMacros1(player, i, list);
	byte fakeMacro[4] = {0};
	for (int i = 0; i < 256; ++i) {
		fakeMacro[0] = (byte)i;
		if (list[i] > 0)
			displayMacroStep(fakeMacro);
	}

}
