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

#include "common/textconsole.h"
#include "common/file.h"

#include "tot/sound.h"
#include "tot/tot.h"

namespace Tot {

uint estadovoc, midestatus;

// pointer exitproc = nil;

const char ctmidi[] = "CTMidi.drv";
const int tamdrctmidi = 17000;
const int tambuffmid = 25000;
const uint tambuffvoc = (uint)(65536 - tambuffmid);
// const array<0, 3, byte> regpag8 = {{0x87, 0x83, 0x81, 0x82}};
// const array<0, 7, byte> regpag16 = {{0x87, 0x83, 0x81, 0x82, 0, 0x8b, 0x89, 0x8a}};
// const array<0, 3, byte> dirbase8 = {{0, 2, 4, 6}};
// const array<0, 7, byte> dirbase16 = {{0xc0, 0xc4, 0xc8, 0xcc, 0xc0, 0xc4, 0xc8, 0xcc}};
// const array<0, 3, byte> contpal8 = {{1, 3, 5, 7}};
// const array<0, 7, byte> contpal16 = {{0xc2, 0xc6, 0xca, 0xce, 0xc2, 0xc6, 0xca, 0xce}};
// const array<0, 15, byte> sbirq = {{
// 		0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf,
// 		0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77
// 	}
// };

byte frecvoc,
     codigodsp,
     dmacontpal,
     loopvoc;
uint prefseg,
     segdatosvoc,
     ofsdatosvoc,
     sbdma,
     sbint,
     irqlevel,
     dmaregpag,
     dmadirbase,
     sbtipo,
     sbdirbase,
     sbmixdirbase,
     cuentavueltasvoc,
     musicaactiva;


boolean loopmid,
        existemidadl,
        existevocsb,
        existemidsbdrv;

byte *midmusica;
// pointer sbexitproc,
//         musicavoc,
//         driversbpromid;

int32 contadorvoc,
        dirdatos;



void guarda_oldirq() {
	warning("STUB: guarda_oldirq");
}

void instalairq() {
	warning("STUB: instalairq");
}

void desinstalairq() {
	warning("STUB: desinstalairq");
}

void activacanaldmaauto() {
	warning("STUB: activacanaldmaauto");
}

void activacanaldma() {
	warning("STUB: activacanaldma");
}

void writedsp(byte dato) {
	warning("STUB: writedsp");
}

void readdsp(byte& dato) {
	warning("STUB: readdsp");
}

void resetdsp() {
	warning("STUB: resetdsp");
}

void writemixer(byte funcion, byte dato) {
	warning("STUB: writemixer");
}

void resetmixer() {
	warning("STUB: resetmixer");
}

byte mixerdata3(byte voll, byte volr) {
	warning("STUB: mixerdata3");
}

byte mixerdata2(byte vol) {
	warning("STUB: mixerdata2");
}

uint mid_version() {
	warning("STUB: mid_version");
}

uint mid_setenv(uint segenv, uint ofsenv) {
	warning("STUB: mid_setenv");
}

void mid_mapper(uint wmapper) {
	warning("STUB: mid_mapper");
}

uint mid_init() {
	warning("STUB: mid_init");
}

void mid_finaldrv() {
	warning("STUB: mid_finaldrv");
}

uint mid_setstuint() {
	warning("STUB: mid_setstuint");
}

void mid_reset() {
	warning("STUB: mid_reset");
}

void mid_stopmusic() {
	warning("STUB: mid_stopmusic");
}

void mid_instala1c() {
	warning("STUB: mid_instala1c");
}

void mid_desinstala1c() {
	warning("STUB: mid_desinstala1c");
}

void stopVoc() {
	g_engine->_sound->stopVoc();
	// resetdsp();
	estadovoc = 0;
}


void setSfxVolume(byte voll, byte volr) {
	// if (existevocsb) {
	// 	switch (codigodsp) {
	// 	case 2:
	// 		writemixer(0xa, (mixerdata2((voll + volr))));
	// 		break;
	// 	case 3:
	// 	case 4:
	// 		writemixer(0x4, (mixerdata3(voll, volr)));
	// 		break;
	// 	}
	// }
}

void setMidiVolume(byte voll, byte volr) {
	if (existemidsbdrv)

		switch (codigodsp) {
		case 2:
			writemixer(0x6, mixerdata3(0, (voll + volr) >> 1));
			break;
		case 3:
		case 4:
			writemixer(0x26, mixerdata3(voll, volr));
			break;
		}
}

void lowerMidiVolume(byte voll, byte volr) {

	// byte volpaso = (voll + volr) / 2;
	// for (int ivol = volpaso; ivol >= 0; ivol --) {
	// 	setMidiVolume(ivol, ivol);
	// 	delay(10);
	// }
}

void restoreMidiVolume(byte voll, byte volr) {
	// byte ivol, volpaso;

	// volpaso = (voll + volr) / 2;
	// for (ivol = 0; ivol <= volpaso; ivol ++) {
	// 	setMidiVolume(ivol, ivol);
	// 	delay(10);
	// }
}

void setMasterVolume(byte voll, byte volr) {
	if (existemidsbdrv || existevocsb)

		switch (codigodsp) {
		case 2:
			writemixer(0x2, mixerdata3(0, (voll + volr) >> 1));
			break;
		case 3:
		case 4:
			writemixer(0x22, mixerdata3(voll, volr));
			break;
		}
}

void inicializasb() {
	// varying_string<64> entornoblaster;
	// varying_string<4> cadenapaso;
	// uint indice, errcode;
	// array<1, 2, byte> version;

	// sbtipo = 0;
	// sbmixdirbase = 0;
	// entornoblaster = _stubGetEnv("Blaster");
	// indice = 1;
	// do {
	// 	switch (entornoblaster[indice]) {
	// 	case 'A' : {
	// 		indice += 1;
	// 		cadenapaso = string('$') + entornoblaster[indice]
	// 		             + entornoblaster[indice + 1]
	// 		             + entornoblaster[indice + 2];
	// 		val(cadenapaso, sbdirbase, errcode);
	// 		indice = indice + 4;
	// 	}
	// 	break;
	// 	case 'I' : {
	// 		indice += 1;
	// 		val(entornoblaster[indice], irqlevel, errcode);
	// 		sbint = sbirq[irqlevel];
	// 		indice += 1;
	// 	}
	// 	break;
	// 	case 'D' : {
	// 		existevocsb = true;
	// 		indice += 1;
	// 		val(entornoblaster[indice], sbdma, errcode);
	// 		indice += 1;
	// 		dmaregpag = regpag8[sbdma];
	// 		dmadirbase = dirbase8[sbdma];
	// 		dmacontpal = contpal8[sbdma];
	// 	}
	// 	break;
	// 	case 'T' : {
	// 		indice += 1;
	// 		val(entornoblaster[indice], sbtipo, errcode);
	// 		indice += 1;
	// 	}
	// 	break;
	// 	case 'H' : {
	// 		indice += 1;
	// 	}
	// 	break;
	// 	case 'M' : {
	// 		indice += 5;
	// 	}
	// 	break;
	// 	case 'P' : {
	// 		indice += 1;
	// 	}
	// 	break;
	// 	case 'L' : {
	// 		indice += 1;
	// 		if (entornoblaster[indice] == 'P') {
	// 			indice += 1;
	// 			switch (entornoblaster[indice]) {
	// 			case 'T' : {
	// 				indice = indice + 2;
	// 			}
	// 			break;
	// 			case 'I' : {
	// 				indice = indice + 2;
	// 			}
	// 			break;
	// 			}
	// 		}
	// 	}
	// 	break;
	// 	default:
	// 		indice += 1;
	// 	}
	// } while (!(indice > length(entornoblaster)));
	// if (existevocsb) {
	// 	sbmixdirbase = sbdirbase;
	// 	resetdsp();
	// 	resetmixer();

	// 	writedsp(0xe1);
	// 	readdsp(version[2]);
	// 	readdsp(version[1]);

	// 	if (memw[seg(version) * 1337 + ofs(version)] < 0x201)
	// 		codigodsp = 1;
	// 	else codigodsp = version[2];
	// 	setMasterVolume(0xff, 0xff);
	// 	setSfxVolume(6, 6);
	// }
}

void loadVoc(Common::String vocfile, int32 posinicio, uint tamvocleer) {
	g_engine->_sound->loadVoc(vocfile, posinicio, tamvocleer);
}

void autoPlayVoc(Common::String vocfile, int32 posinicio, uint tamvocleer) {
	g_engine->_sound->loadVoc(vocfile, posinicio, tamvocleer);
	g_engine->_sound->autoPlayVoc();
	// uint indice;
	// byte dato;
	estadovoc = 1;
	// if (existevocsb) {
	// 	loopvoc = 0;
	// 	resetdsp();
	// 	loadVoc(vocfile, posinicio, tamvocleer);
	// 	writedsp(0xd1);
	// 	activacanaldmaauto();
	// 	writedsp(0x40);
	// 	writedsp(frecvoc);
	// 	writedsp(0x48);
	// 	writedsp(lo(contadorvoc));
	// 	writedsp(hi(contadorvoc));
	// 	// estadovoc = 0xffff;
	// 	writedsp(0x1c);
	// }
}

void pitavocmem() {
	g_engine->_sound->playVoc();
	estadovoc = 1;
	// if (existevocsb) {
	// 	loopvoc = 0;
	// 	resetdsp();
	// 	writedsp(0xd1);
	// 	activacanaldma();
	// 	writedsp(0x40);
	// 	writedsp(frecvoc);
	// 	writedsp(0x14);
	// 	writedsp(lo(contadorvoc));
	// 	writedsp(hi(contadorvoc));
	// 	estadovoc = 0xffff;
	// }
}

void playVoc(Common::String vocfile, int32 posinicio, uint tamvocleer) {
	loadVoc(vocfile, posinicio, tamvocleer);
	pitavocmem();
}

void playMidiFile(Common::String nomfich, boolean loop) {
	// Disabled to avoid annoying music

	Common::File ficheromus;
	if(!ficheromus.open(Common::Path(nomfich + ".MUS"))){
		error("pitamidfich(): ioresult! (267)");
	}
	mid_reset();
	midmusica = (byte *)malloc(ficheromus.size());
	ficheromus.read(midmusica, ficheromus.size());

	g_engine->_sound->playMidi((byte *)midmusica, ficheromus.size(), true);
	ficheromus.close();
}

boolean disponiblemid() {
	boolean disponiblemid_result;
	disponiblemid_result = existemidsbdrv;
	return disponiblemid_result;
}

boolean cargasbdrvmid() {
	// uint _error;
	// string entornoblaster;
	// untyped_file ficheromus;

	// boolean cargasbdrvmid_result;
	// cargasbdrvmid_result = false;
	// assign(ficheromus, ctmidi);

	// if (ioresult != 0)  error("cargasbdrvmid(): ioresult! (268)");
	// if (ofs(driversbpromid) == 0) {
	// 	blockread(ficheromus, driversbpromid, tamdrctmidi, _error);
	// 	if (mid_version() >= 0x100) {
	// 		entornoblaster = _stubGetEnv("BLASTER") + chr(0) + chr(0);
	// 		if (mid_setenv(seg(entornoblaster), ofs(entornoblaster)) == 0) {
	// 			mid_reset();
	// 			if (mid_init() == 0)

	// 				if (mid_setstuint() == 0) {
	// 					cargasbdrvmid_result = true;
	// 					existemidsbdrv = true;
	// 					mid_mapper(0);
	// 					setMidiVolume(3, 3);
	// 				}
	// 		}
	// 	}
	// }
	// close(ficheromus);
	// return cargasbdrvmid_result;
	return 0;
}

void initsbmid() {
	warning("STUB! initsbmi");
	// uint alineacion;
	// pointer basurapointer;

	// getmem(driversbpromid, tamdrctmidi);
	// alineacion = ofs(driversbpromid);
	// if (alineacion != 0) {
	// 	freemem(driversbpromid, tamdrctmidi);
	// 	getmem(basurapointer, (16 - alineacion));
	// 	getmem(driversbpromid, tamdrctmidi);
	// 	freemem(basurapointer, (16 - alineacion));
	// 	alineacion = ofs(driversbpromid);
	// }
	// if (alineacion != 0)  error("initsbmid(): alineacion! (276)");

	// midmusica = malloc(tambuffvoc);
	// if (cargasbdrvmid())

	// 	if (existemidsbdrv) {
	// 		// midmusica = ptr(0x9000, tambuffvoc);
	// 		mid_reset();
	// 	} else
	// 		freemem(driversbpromid, tamdrctmidi);
}



void salidasbunit() {
	desinstalairq();
	resetdsp();
	writedsp(0xd3);
	if (existemidsbdrv) {
		mid_stopmusic();
		delay(100);
		mid_reset();
		delay(100);
		mid_finaldrv();
		// free(driversbpromid);
	}
}

void initSound() {
	// existevocsb = false;
	// existemidsbdrv = false;
	// estadovoc = 0;
	// cuentavueltasvoc = 0;
	// loopvoc = 0;
	// inicializasb();
	// if (existevocsb) {
	// 	musicavoc = ptr(0x9000, 0);
	// 	ofsdatosvoc = ofs(musicavoc);
	// 	segdatosvoc = seg(musicavoc);
	// 	// initsbmid();
	// 	guarda_oldirq();
	// 	instalairq();
	// 	sbexitproc = exitproc;

	// 	warning("STUB: exitproc assignment");
	// 	//exitproc=&salidasbunit;
	// 	setMidiVolume(0, 0);
	// 	playMidiFile("Silent", false);
	// }
}

} // End of namespace Totficheromus.size()
;
