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

#include "common/debug.h"
#include "common/events.h"
#include "common/file.h"
#include "common/substream.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "graphics/paletteman.h"

#include "tot/decoder/TotFlicDecoder.h"
#include "tot/anims.h"
#include "tot/graphics.h"
#include "tot/playanim.h"
#include "tot/tot.h"

namespace Tot {

const int sizefrase = 320 * 30 + 4;

int32 posflicfile;
uint numerovuelta, posrelfli;
byte *punterofondofrase = (byte *)malloc(sizefrase);
byte framecontador;
boolean primeravuelta;

palette palpasoflic;

void drawText(uint xfrase, uint yfrase, Common::String str1, Common::String str2, Common::String str3, Common::String str4, Common::String str5, byte colorfrase, byte colorborde) {

	outtextxy(xfrase, (yfrase + 3), str1, colorborde);
	outtextxy(xfrase, (yfrase + 13), str2, colorborde);
	outtextxy(xfrase, (yfrase + 23), str3, colorborde);
	outtextxy(xfrase, (yfrase + 33), str4, colorborde);
	outtextxy(xfrase, (yfrase + 43), str5, colorborde);
	g_engine->_screen->update();
	delay(enforcedTextAnimDelay);
	outtextxy(xfrase, (yfrase + 1), str1, colorborde);
	outtextxy(xfrase, (yfrase + 11), str2, colorborde);
	outtextxy(xfrase, (yfrase + 21), str3, colorborde);
	outtextxy(xfrase, (yfrase + 31), str4, colorborde);
	outtextxy(xfrase, (yfrase + 41), str5, colorborde);
	g_engine->_screen->update();
	delay(enforcedTextAnimDelay);
	outtextxy(xfrase + 1, (yfrase + 2), str1, colorborde);
	outtextxy(xfrase + 1, (yfrase + 12), str2, colorborde);
	outtextxy(xfrase + 1, (yfrase + 22), str3, colorborde);
	outtextxy(xfrase + 1, (yfrase + 32), str4, colorborde);
	outtextxy(xfrase + 1, (yfrase + 42), str5, colorborde);
	g_engine->_screen->update();
	delay(enforcedTextAnimDelay);
	outtextxy(xfrase - 1, (yfrase + 2), str1, colorborde);
	outtextxy(xfrase - 1, (yfrase + 12), str2, colorborde);
	outtextxy(xfrase - 1, (yfrase + 22), str3, colorborde);
	outtextxy(xfrase - 1, (yfrase + 32), str4, colorborde);
	outtextxy(xfrase - 1, (yfrase + 42), str5, colorborde);
	g_engine->_screen->update();
	delay(enforcedTextAnimDelay);
	outtextxy(xfrase, (yfrase + 2), str1, colorfrase);
	outtextxy(xfrase, (yfrase + 12), str2, colorfrase);
	outtextxy(xfrase, (yfrase + 22), str3, colorfrase);
	outtextxy(xfrase, (yfrase + 32), str4, colorfrase);
	outtextxy(xfrase, (yfrase + 42), str5, colorfrase);
	g_engine->_screen->update();
}

void removeText(uint xfrase1, uint yfrase1, uint xfrase2, uint yfrase2, byte colorrelleno) {

	for (int j = yfrase1; j < yfrase2 + 1; j++) {
		for (int i = xfrase1; i < xfrase2 + 1; i++) {
			*((byte *)g_engine->_screen->getBasePtr(i, j)) = 0;
		}
	}
	g_engine->_screen->addDirtyRect(Common::Rect(xfrase1, yfrase1, xfrase2, yfrase2));
}

void handleFlcEvent(byte numero_del_evento) {
	switch (numero_del_evento) {
	case 0:
		if (contadorpc > 103)
			error("handleFlcEvent(): ioresult! (274)");
		break;
	case 1:
		if (framecontador == 3)

			switch (numerovuelta) {
			case 2:
				drawText(80, 0, "Las \xA3ltimas  encuestas  realizadas",
						 "acerca  de  la  siniestralidad  en ",
						 "los transportes arrojan resultados",
						 "alentadores...", "", 253, 0);
				break;
			case 13:
				removeText(80, 0, 319, 53, 0);
				break;
			case 14:
				drawText(80, 0, "El cien por cien  de los encuestados",
						 "declararon no haber sufrido nunca un",
						 "accidente mortal...", "", "", 253, 0);
				break;
			case 25:
				removeText(80, 0, 319, 53, 0);
				break;
			case 26:
				drawText(80, 0, "Es una noticia que nos anima a salir",
						 "de viaje con m\xA0s seguridad.", "", "", "", 253, 0);
				break;
			case 35:
				removeText(80, 0, 319, 53, 0);
				break;
			case 36:
				drawText(2, 100, "\xAD\xADPor un tornillo!!...",
						 "Por un m\xA1sero y simple",
						 "tornillo  del  tr\xA1""fugo",
						 "del trapecio...", "", 255, 0);
				break;
			case 47:
				removeText(2, 100, 134, 199, 0);
				break;
			case 48:
				drawText(2, 100, "Si lo hubiese apretado",
						 "bien, no estar\xA1""a ahora",
						 "sin trabajo...", "", "", 255, 0);
				break;
			case 59:
				removeText(2, 100, 134, 199, 0);
				break;
			case 60:
				drawText(80, 0, "\xAD\xADNoticia de \xA3ltima hora!!", "", "", "", "", 253, 0);
				break;
			case 63:
				drawText(2, 100, "Llevo  sin dormir toda",
						 "la noche, pensando qu\x82",
						 "voy a hacer.", "", "", 255, 0);
				break;
			case 75:
				removeText(2, 100, 135, 199, 0);
				break;
			case 76:
				removeText(80, 0, 319, 53, 0);
				break;
			case 77:
				drawText(2, 100, "Seguramente  no  podr\x82",
						 "encontrar  trabajo  en",
						 "mucho tiempo...",
						 "Aunque  ponga mucha f\x82",
						 "la oficina de empleo.", 255, 0);
				break;
			case 89:
				removeText(2, 100, 135, 199, 0);
				break;
			case 90:
				drawText(2, 100, "Y  para  animarme  m\xA0s",
						 "todav\xAD""a, ma\xA4""ana  noche",
						 "es HALLOWEEN...", "",
						 "Es que tengo la negra.", 255, 0);
				break;
			case 102:
				removeText(2, 100, 135, 199, 0);
				break;
			case 103:
				drawText(80, 0, "Grave  accidente  en  TWELVE MOTORS...",
						 "un empleado olvid\xA2 apretar un tornillo",
						 "y media  cadena  de montaje ha quedado",
						 "para chatarra...",
						 "las p\x82rdidas se  estiman cuantiosas...", 253, 0);
				break;
			case 120:
				removeText(80, 0, 319, 53, 0);
				break;
			case 121:
				drawText(80, 0, "No se descartan las acciones legales",
						 "contra ese inconsciente...", "", "", "", 253, 0);
				break;
			case 125:
				drawText(2, 100, "\xAD""Dios mio!... hasta en",
						 "la televisi\xA2n...",
						 "Seguro  que  hasta  mi",
						 "perro  encuentra curro",
						 "antes que yo...", 255, 0);
				break;
			case 135: {
				removeText(80, 0, 319, 53, 0);
				removeText(2, 100, 135, 199, 0);
			} break;
			}
		break;
	case 2:
		switch (framecontador) {
		case 1: {
			removeText(2, 100, 135, 199, 0);
			removeText(80, 0, 319, 53, 0);
		} break;
		case 7:
			playVoc("MANDO", 142001, 11469);
			break;
		case 20:
			drawText(2, 100, "\xADHORROR!...una factura", "", "", "", "", 255, 0);
			break;
		case 58: {
			removeText(2, 100, 135, 199, 0);
			drawText(2, 100, "\xA8""Oficina de empleo?...",
					 "\xA8""y para m\xA1?...", "",
					 "\xADSe habr\xA0n equivocado!", "", 255, 0);
		} break;
		case 74: {
			delay(1500);
			removeText(2, 100, 135, 199, 0);
			drawText(2, 100, "\xA8Tan  pronto lograron",
					 "encontrarme  trabajo?", "",
					 "Claro... antes de ver",
					 "las noticias...", 255, 0);
			delay(4000);
			removeText(2, 100, 135, 199, 0);
			drawText(80, 0, "Estimado se\xA4or: Nos dirigimos a",
					 "usted  para  comunicarle que su",
					 "solicitud n\xA7:93435 fu\x82 aceptada",
					 "y le hemos encontrado un puesto",
					 "de acuerdo con sus aptitudes...", 253, 0);
			delay(7000);
			removeText(80, 0, 319, 53, 0);
			drawText(80, 0, "Deber\xA0 presentarse  ma\xA4""ana, dia",
					 "31  de  Octubre en la direcci\xA2n",
					 "abajo indicada,  para ocupar el",
					 "puesto de ENCARGADO GENERAL.", "", 253, 0);
			delay(7000);
			removeText(80, 0, 319, 53, 0);
			drawText(80, 0, "Le adjuntamos las  credenciales",
					 "de presentaci\xA2n.",
					 "Deseando no verle  por  aqu\xA1 en",
					 "mucho tiempo se despide:",
					 "    Leonor Scott.", 253, 0);
			delay(7000);
			removeText(80, 0, 319, 53, 0);
			drawText(2, 100, "\xAD\xADQue ilu!! encargado",
					 "general...  si ya  me",
					 "lo decia  mi madre...",
					 "<<nene tienes cara de",
					 "encargado general>>", 255, 0);
			delay(6000);
			removeText(2, 100, 135, 199, 0);
			drawText(2, 100, "Tendr\x82  que conseguir",
					 "ropa  m\xA0s  de acuerdo",
					 "con mi nuevo cargo.", "",
					 "Manos a la obra...", 255, 0);
			delay(6000);
		} break;
		}
		break;
	case 3:
		switch (framecontador) {
		case 15:
			playVoc("FRENAZO", 165322, 15073);
			break;
		case 26:
			delay(1000);
			break;
		case 43:
			playVoc("PORTAZO", 434988, 932);
			break;
		case 60:
			getImg(0, 0, 319, 29, punterofondofrase);
			break;
		}
		break;
	case 4:
		if (framecontador == 3)
			playVoc("TIMBRAZO", 423775, 11213);
		break;
	case 5:
		if ((numerovuelta == 1) && (framecontador == 2)) {

			delay(2000);
			drawText(5, 1, "\xADV\xA0yase!... no queremos vendedores...",
					 "y menos de planes de jubilaci\xA2n.", "", "", "", 255, 249);
			delay(3500);
			putImg(0, 0, punterofondofrase);
			drawText(5, 1, "No soy ning\xA3n vendedor... me envia la oficina",
					 "de empleo... tenga mis credenciales.", "", "", "", 255, 0);
		}
		break;
	case 6:
		if ((numerovuelta == 1) && (framecontador == 3)) {
			putImg(0, 0, punterofondofrase);
			drawText(5, 1, "\xADVaya! eres t\xA3...llegas con retraso. Los ancianos est\xA0n",
					 "esper\xA0ndote en el sal\xA2n. Debes cuidarlos muy bien.",
					 "", "", "", 255, 249);
		}
		break;
	case 7:
		if ((numerovuelta == 1) && (framecontador == 3)) {
			putImg(0, 0, punterofondofrase);
			drawText(5, 1, "Lo siento, no sab\xA1""a a que hora deb\xA1""a presentarme, en",
					 "la oficina de empleo no me lo dijeron.",
					 "", "", "", 255, 0);
		}
		break;
	case 8:
		if ((numerovuelta == 1) && (framecontador == 3)) {
			putImg(0, 0, punterofondofrase);
			drawText(5, 1, "Bueno... no importa... ya est\xA0s aqu\xA1.",
					 "Ese coche es tuyo \xA8no?...", "", "", "", 255, 249);
		}
		break;
	case 9:
		if ((numerovuelta == 1) && (framecontador == 3)) {
			putImg(0, 0, punterofondofrase);
			drawText(5, 1, "Si... es precioso \xA8verdad?...", "", "", "", "", 255, 0);
		}
		break;
	case 10:
		if ((numerovuelta == 1) && (framecontador == 3)) {
			putImg(0, 0, punterofondofrase);
			drawText(5, 1, "Vaya... 200 caballos... elevalunas el\x82""ctricos... ",
					 "inyecci\xA2n electr\xA2nica, llantas de aleaci\xA2n...",
					 "", "", "", 255, 249);
		}
		break;
	case 11:
		if ((numerovuelta == 1) && (framecontador == 3)) {
			putImg(0, 0, punterofondofrase);
			drawText(5, 1, "Pues si...", "", "", "", "", 255, 0);
		}
		break;
	case 12:
		if ((numerovuelta == 1) && (framecontador == 3)) {
			putImg(0, 0, punterofondofrase);
			drawText(5, 1, "Airbag... asientos envolventes... aire acondicionado",
					 "", "", "", "", 255, 249);
		}
		break;
	case 13:
		if ((numerovuelta == 1) && (framecontador == 3)) {
			putImg(0, 0, punterofondofrase);
			drawText(5, 1, "Es el m\xA0s alto de la gama...", "", "", "", "", 255, 0);
		}
		break;
	case 14:
		if ((numerovuelta == 1) && (framecontador == 3)) {
			putImg(0, 0, punterofondofrase);
			drawText(5, 1, "ABS, equipo de m\xA3sica con CD, estructura reforzada",
					 "indeformable, detector de radar...", "", "", "", 255, 249);
		}
		break;
	case 15:
		if ((numerovuelta == 1) && (framecontador == 3)) {
			putImg(0, 0, punterofondofrase);
			drawText(5, 1, "Parece que te gusta, \xA8no?", "", "", "", "", 255, 0);
		}
		break;
	case 16:
		if ((numerovuelta == 1) && (framecontador == 3)) {
			putImg(0, 0, punterofondofrase);
			drawText(5, 1, "Bueno, no est\xA0 mal... pero... gracias de todos modos.",
					 "", "", "", "", 255, 249);
		}
		break;
	case 17:
		switch (framecontador) {
		case 1:
			putImg(0, 0, punterofondofrase);
			break;
		case 17:
			delay(500);
			break;
		case 18:
			playVoc("ACELERON", 30200, 42398);
			break;
		}
		break;
	case 18:
		if ((numerovuelta == 1) && (framecontador == 3)) {
			putImg(0, 0, punterofondofrase);
			drawText(5, 1, "\xA8Gracias?... \xA8qu\x82?... \xAD""espera!... ese es mi coche...",
					 "", "", "", "", 255, 0);
		}
		break;
	case 19:
		if (framecontador == 1)
			drawText(5, 121, "Toma una copita, te la has ganado...",
					 "", "", "", "", 253, 249);
		break;
	case 20:
		switch (numerovuelta) {
		case 1:
			switch (framecontador) {
			case 1:
				getImg(0, 0, 319, 29, punterofondofrase);
				break;
			case 3:
				drawText(15, 1, "\xA8""Donde estoy? ... \xA8Qu\x82 me ha pasado?",
						 "", "", "", "", 255, 0);
				break;
			}
			break;
		case 3:
			switch (framecontador) {
			case 1:
				putImg(0, 0, punterofondofrase);
				break;
			case 3:
				drawText(15, 1, "Tranquilizate, no est\xA0s muerto...",
						 "Est\xA0s tras la puerta de acero...", "", "", "", 230, 249);
				break;
			}
			break;
		case 6:
			switch (framecontador) {
			case 1:
				putImg(0, 0, punterofondofrase);
				break;
			case 3:
				drawText(15, 1, "Te necesitamos para encontrar a Jacob,",
						 "nuestro Gran Maestre.", "", "", "", 230, 249);
				break;
			}
			break;
		case 9:
			switch (framecontador) {
			case 1:
				putImg(0, 0, punterofondofrase);
				break;
			case 3:
				drawText(15, 1, "Hace unos dias se puso a estudiar el libro",
						 "que me diste y desapareci\xA2...", "", "", "", 230, 249);
				break;
			}
			break;
		case 12:
			switch (framecontador) {
			case 1:
				putImg(0, 0, punterofondofrase);
				break;
			case 3:
				drawText(15, 1, Common::String("Creemos que realiz\xA2 un conjuro sin darse") + " cuenta...", "", "", "", "", 230, 249);
				break;
			}
			break;
		case 15:
			switch (framecontador) {
			case 1:
				putImg(0, 0, punterofondofrase);
				break;
			case 3:
				drawText(15, 1, "Por fortuna, pudimos ponernos en contacto",
						 "con \x82l, y nos dijo que est\xA0 en problemas...",
						 "", "", "", 230, 249);
				break;
			}
			break;
		case 18:
			switch (framecontador) {
			case 1:
				putImg(0, 0, punterofondofrase);
				break;
			case 3:
				drawText(15, 1, "Nos pidi\xA2 que envi\xA0semos a alguien en su ayuda,",
						 "y TU eres el elegido...", "", "", "", 230, 249);
				break;
			}
			break;
		case 21:
			switch (framecontador) {
			case 1:
				putImg(0, 0, punterofondofrase);
				break;
			case 3:
				drawText(15, 1, "Para ir a las CAVERNAS ETERNAS debes morir...",
						 "Ning\xA3n cuerpo con alma puede estar all\xA1...", "", "", "", 230, 249);
				break;
			}
			break;
		case 24:
			switch (framecontador) {
			case 1:
				putImg(0, 0, punterofondofrase);
				break;
			case 3:
				drawText(15, 1, Common::String("Haremos un ritual para crear un v\xA1nculo entre") + " tu", "cuerpo y tu alma, s\xA2lo as\xA1 podr\xA0s volver a la vida...",
						 "", "", "", 230, 249);
				break;
			}
			break;
		case 27:
			switch (framecontador) {
			case 1:
				putImg(0, 0, punterofondofrase);
				break;
			case 3:
				drawText(15, 1, Common::String("Si rescatas a Jacob, el v\xA1nculo te traer\xA0") + " de regreso...", "", "", "", "", 230, 249);
				break;
			}
			break;
		case 30:
			switch (framecontador) {
			case 1:
				putImg(0, 0, punterofondofrase);
				break;
			case 3:
				drawText(15, 1, "Ahora... \xADPrep\xA0rate!... Ha llegado tu hora...",
						 "", "", "", "", 230, 249);
				break;
			}
			break;
		case 33:
			if (framecontador == 17)
				putImg(0, 0, punterofondofrase);
			break;
		}
		break;
	case 21:
		switch (framecontador) {
		case 1:
			playVoc("TRIDEN", 409405, 14370);
			break;
		case 5:
			playVoc("PUFF", 191183, 18001);
			break;
		}
		break;
	case 22:
		if (framecontador == 24)
			playVoc("PUFF", 191183, 18001);
		break;
	case 23:
		switch (framecontador) {
		case 8:
			playVoc("AFILAR", 0, 6433);
			break;
		case 18:
			pitavocmem();
			break;
		}
		break;
	case 24:
		if (framecontador == 8)
			playVoc("DECAPITA", 354269, 1509);
		break;
	case 25:
		if (framecontador == 97)
			playVoc("PUFF2", 209184, 14514);
		break;
	case 26:
		switch (numerovuelta) {
		case 1:
			switch (framecontador) {
			case 2:
				getImg(0, 0, 319, 29, punterofondofrase);
				break;
			case 3:
				drawText(65, 1, "\xADGracias ILUSO!...", "", "", "", "", 253, 249);
				break;
			}
			break;
		case 2:
			switch (framecontador) {
			case 1:
				putImg(0, 0, punterofondofrase);
				break;
			case 2:
				drawText(65, 1, "Ahora tendr\x82 un cuerpo nuevo y podr\x82",
						 "volver a la vida...", "", "", "", 253, 249);
				break;
			}
			break;
		case 5:
			switch (framecontador) {
			case 1:
				putImg(0, 0, punterofondofrase);
				break;
			case 2:
				drawText(65, 1, "Tu mientras tanto, puedes seguir en",
						 "las CAVERNAS ETERNAS...", "", "", "", 253, 249);
				break;
			}
			break;
		case 8:
			switch (framecontador) {
			case 1:
				putImg(0, 0, punterofondofrase);
				break;
			case 2:
				drawText(65, 1, "No te preocupes, seguro que nos mandan",
						 "otro encargado...  JA,JA,JA...", "", "", "", 253, 249);
				break;
			}
			break;
		}
		break;
	case 27:
		if (framecontador == 148)
			playVoc("DECAPITA", 354269, 1509);
		break;
	}
}

void drawFlc(uint flicx, uint flicy, int32 posicionfli, uint loop,
			 byte veloc, byte numevento, boolean palcompleta, boolean permitesalida,
			 boolean doscientos, boolean &salidaflis);

static void exitProcedure(boolean &salir_bucle, boolean &permitesalida) {

	salir_bucle = false;
	if (permitesalida) {
		Common::Event e;
		while (g_system->getEventManager()->pollEvent(e)) {
			if (e.type == Common::EVENT_KEYDOWN || (e.type == Common::EVENT_LBUTTONUP)) {
				salir_bucle = true;
				debug("Exiting from exitProcedure!");
			}
		}
	}
}

static fliheader readHeader(Common::File *file) {
	fliheader headerfile;

	headerfile.size = file->readSint32LE();
	headerfile.magic = file->readSint16LE();
	headerfile.frames = file->readSint16LE();
	headerfile.width = file->readSint16LE();
	headerfile.heigth = file->readSint16LE();
	headerfile.depth = file->readSint16LE();
	headerfile.flags = file->readSint16LE();
	headerfile.speed = file->readSint32LE();
	headerfile.reserved1 = file->readSint16LE();
	headerfile.created = file->readSint32LE();
	file->read(headerfile.creator, 4);
	headerfile.updated = file->readSint32LE();
	file->read(headerfile.updator, 4);
	headerfile.aspectx = file->readSint16LE();
	headerfile.aspecty = file->readSint16LE();
	// Note: probably shouldnt just harcode 2 here
	file->read(headerfile.reserved2, 19 * 2);
	headerfile.ofsframe1 = file->readSint32LE();
	headerfile.ofsframe2 = file->readSint32LE();
	// Note: probably shouldnt just harcode 2 here
	file->read(headerfile.reserved2, 20 * 2);
	return headerfile;
}

static void loadFlc(
	uint &loop,
	boolean &permitesalida,
	boolean &salidaflis,
	byte &numevento,
	boolean &palcompleta,
	boolean &doscientos,
	byte &veloc,
	uint &flicx,
	uint &flicy) {

	framecontador = 0;
	numerovuelta = 0;

	Common::File animationsFile;
	Common::String fileName;
	if (loop == 60000)
		fileName = "OBJGIRO.DAT";
	else
		fileName = "FILMS.DAT";

	if (!animationsFile.open(Common::Path(fileName))) {
		error("readFlcXms(): ioresult! (272)");
	}
	animationsFile.seek(posflicfile, SEEK_SET);
	// Need to read header to get the total size of the FLIC file.
	fliheader header = readHeader(&animationsFile);
	Common::SeekableSubReadStream *thisFlic = new Common::SeekableSubReadStream(
		&animationsFile,
		posflicfile,
		posflicfile + header.size);

	TotFlicDecoder flic = TotFlicDecoder();
	flic.loadStream(thisFlic);

	flic.start();

	do {
		g_engine->_chrono->updateChrono();
		numerovuelta++;
		do {
			g_engine->_chrono->updateChrono();
			exitProcedure(salidaflis, permitesalida);
			if (salidaflis) {
				debug("Salidaflis!");
				goto Lsalir_proc;
			}
			if (tocapintar){
				framecontador++;
				handleFlcEvent(numevento);
				const Graphics::Surface *frame = flic.decodeNextFrame();
				if (frame) {
					Common::Rect boundingBox = Common::Rect(flicx, flicy, flicx + flic.getWidth() + 1, flicy + flic.getHeight() + 1);
					g_engine->_graphics->blit(frame, boundingBox);

					if (flic.hasDirtyPalette()) {
						byte *palette = (byte *)flic.getPalette();
						// game fixes background to 0
						palette[0] = 0;
						palette[1] = 0;
						palette[2] = 0;
						if (palcompleta) {
							g_engine->_graphics->changePalette(g_engine->_graphics->getPalette(), palette);
						} else {
							int limit = doscientos ? 200 : 256;
							g_engine->_graphics->setPalette(palette, limit);
						}
					}
					tocapintar = false;
				} else {
					break;
				}
			}
			g_system->delayMillis(10);
		} while (!flic.endOfVideo() && !g_engine->shouldQuit());

		if (flic.endOfVideo()) {
			if (flic.isRewindable()) {
				flic.rewind();
			}
			framecontador = 0;
			primeravuelta = false;
		}
		g_system->delayMillis(10);
	} while (numerovuelta <= loop && !g_engine->shouldQuit());
	flic.stop();
Lsalir_proc:
	animationsFile.close();
}

void drawFlc(
	uint flicx,
	uint flicy,
	int32 posicionfli,
	uint loop,
	byte veloc,
	byte numevento,
	boolean palcompleta,
	boolean permitesalida,
	boolean doscientos,
	boolean &salidaflis) {

	primeravuelta = true;
	posflicfile = posicionfli;
	posrelfli = flicx + flicy * 320;
	// punterofondofrase = malloc(sizefrase);
	loadFlc(loop, permitesalida, salidaflis, numevento, palcompleta, doscientos, veloc, flicx, flicy);
	debug("Salida flis: %d", salidaflis);
	// free(punterofondofrase);
}

} // End of namespace Tot
