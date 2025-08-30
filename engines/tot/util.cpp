
#include "common/textconsole.h"
#include "graphics/paletteman.h"

#include "tot/tot.h"
#include "tot/util.h"

namespace Tot {

// Enforces a small delay when text reveal is supposed to happen
const int kEnforcedTextAnimDelay = 0;

void showError(int code) {
	switch (code) {
	case 1:
		error("Error 1: Funci\xA2n no existe.");
		break;
	case 2:
		error("Error 2: Fichero no encontrado.");
		break;
	case 3:
		error("Error 3: Camino no encontrado.");
		break;
	case 4:
		error("Error 4: Demasiados ficheros abiertos.");
		break;
	case 5:
		error("Error 5: Acceso denegado.");
		break;
	case 6:
		error("Error 6: Asignacion de fichero incorrecta.");
		break;
	case 12:
		error("Error 12: C\xA2"
			  "digo de acceso a fichero incorrecto.");
		break;
	case 13:
	case 14:
		error("Error %d: \xA8"
			  "Alguien ha visto por ah\xA1 un bol\xA1grafo?.",
			  code);
		break;
	case 15:
		error("Error 15: Unidad incorrecta.");
		break;
	case 16:
		error("Error 16: No se puede borrar el directorio.");
		break;
	case 17:
		error("Error 17: No se puede renombrar en distintas unidades.");
		break;
	case 18:
		error("Error 18: No hay m\xA0s ficheros.");
		break;
	case 100:
		error("Error 100: No se pudo leer del disco.");
		break;
	case 101:
		error("Error 101: No se pudo escribir en el disco.");
		break;
	case 102:
		error("Error 102: Fichero sin asignar.");
		break;
	case 103:
		error("Error 103: Fichero sin abrir.");
		break;
	case 104:
		error("Error 104: Fichero abierto para salida.");
		break;
	case 105:
		error("Error 105: Fichero abierto para entrada.");
		break;
	case 106:
		error("Error 106: Formato num\x82rico no v\xA0lido.");
		break;
	case 150:
		error("Error 150: Disco protegido contra escritura.");
		break;
	case 151:
		error("Error 151: El driver del disco devuelve una estructura incorrecta.");
		break;
	case 152:
		error("Error 152: Unidad no preparada.");
		break;
	case 153:
		error("Error 153: JOZU!! que Barbari\xA0.");
		break;
	case 154:
		error("Error 154: Comprobaci\xA2n err\xA2nea en el control de redundancia c\xA1"
			  "clica.");
		break;
	case 155:
		error("Error %d: Cerrado por vacaciones.", code);
		break;
	case 156:
		error("Error 156: Posicionamiento err\xA2neo en el fichero.");
		break;
	case 157:
		error("Error 157: Disco de formato desconocido.");
		break;
	case 158:
		error("Error 158: Sector no encontrado en el disco.");
		break;
	case 159:
		error("Error 159: Impresora sin papel.");
		break;
	case 160:
		error("Error 160: No se pudo escribir en el dispositivo.");
		break;
	case 161:
		error("Error 161: No se pudo leer del dispositivo.");
		break;
	case 162:
		error("Error 162: Problema con el dispositivo.");
		break;
	case 200:
		error("Error 200: Division por cero.");
		break;
	case 201:
		error("Error 201: Variable fuera de rango.");
		break;
	case 202:
		error("Error 202: Desbordamiento en la pila.");
		break;
	case 203:
		error("Error 203: Desbordamiento en la memoria din\xA0mica.");
		break;
	case 204:
		error("Error 204: Operaci\xA2n err\xA2nea con un puntero.");
		break;
	case 205:
		error("Error 205: Desbordamiento en coma flotante.");
		break;
	case 206:
		error("Error 206: Desbordamiento negativo en coma flotante.");
		break;
	case 207:
		error("Error 207: Operaci\xA2n err\xA2nea de coma flotante.");
		break;
	case 208:
		error("Error 208: Gestor de solapamiento sin instalar.");
		break;
	case 209:
		error("Error 209: Lectura err\xA2nea del fichero de solapamiento.");
		break;
	case 210:
		error("Error 210: Objeto sin inicializar.");
		break;
	case 211:
		error("Error 211: Llamada a un metodo abstracto.");
		break;
	case 212:
		error("Error 212: Cadena de asignaci\xA2n del objeto err\xA2nea.");
		break;
	case 213:
		error("Error 213: Indice para colecci\xA2n fuera de rango.");
		break;
	case 214:
		error("Error 214: Desbordamiento en la colecci\xA2n.");
		break;
	case 215:
		error("Error 215: Desbordamiento en operaci\xA2n aritm\x82tica.");
		break;
	case 216:
		error("Error 216: Acceso a memoria sin asignar.");
		break;
	case 250:
		error("Error 250: El driver de AdLib no est\xA0 instalado.");
		break;
	case 251:
		error("Error 251: Libre para AdLib.");
		break;
	case 252:
		error("Error 252: No se encontr\xA2 el fichero de efectos para SBlaster.");
		break;
	case 253:
		error("Error 253: No se encontr\xA2 el fichero de m\xA3sica para SBlaster.");
		break;
	case 254:
		error("Error 254: Libre para el fichero de configuraci\xA2n del sonido.");
		break;
	case 255:
		error("Error 255: Detectado Ctrl-Break.");
		break;
	case 256:
		error("Error 256: La tarjeta de video no soporta el modo gr\xA0"
			  "fico requerido.");
		break;
	case 257:
		error("Error 257: El tipo de pantalla no es correcto.");
		break;
	case 258:
		error("Error 258: No se encontr\xA2 el fichero con la imagen del menu.");
		break;
	case 259:
		error("Error 259: Clave de protecci\xA2n erronea.");
		break;
	case 260:
		error("Error 260: No se encontr\xA2 el driver del modo X de v\xA1"
			  "deo.");
		break;
	case 261:
		error("Error 261: No se encontr\xA2 el fichero de datos de los objetos.");
		break;
	case 262:
		error("Error 262: No se encontr\xA2 el fichero de los BITMAP.");
		break;
	case 263:
		error("Error 263: No se encontr\xA2 el fichero de paleta para la imagen girando.");
		break;
	case 264:
		error("Error 264: Se desbord\xA2 la pila de montaje de bitmaps");
		break;
	case 265:
		error("Error 265: No se encontr\xA2 el fichero de la animaci\xA2n.");
		break;
	case 266:
		error("Error 266: No se encontr\xA2 el fichero del efecto de sonido.");
		break;
	case 267:
		error("Error 267: No se encontr\xA2 el fichero de la melodia.");
		break;
	case 268:
		error("Error 268: No se encontr\xA2 el driver de sonido MIDI.");
		break;
	case 269:
		error("Error 269: No se pudo capturar un vector de interrupci\xA2n.");
		break;
	case 270:
		error("Error 270: No se encontr\xA2 el fichero de los cr\x82"
			  "ditos.");
		break;
	case 271:
		error("Error 271: No se encontr\xA2 el fichero del BitMap del sello.");
		break;
	case 272:
		error("Error 272: No se encontr\xA2 el fichero de la animaci\xA2n.");
		break;
	case 273:
		error("Error 273: No se encontraron los \xA1"
			  "ndices de los modos X de acceso a v\xA1"
			  "deo.");
		break;
	case 274:
		error("Error 274: Fichero de datos corrupto, instale de nuevo el programa.");
		break;
	case 275:
		error("Error 275: Posici\xA2n incorrecta del Sprite.");
		break;
	case 276:
		error("Error 276: Alineaci\xA2n incorrecta del driver de sonido.");
		break;
	case 277:
		error("Error 277: No se encontr\xA2 el fichero de la animaci\xA2n.");
		break;
	case 278:
		error("Error 278: No se encontr\xA2 el fichero para la actualizaci\xA2n.");
		break;
	case 279:
		error("Error 279: No se actualiz\xA2 el puntero del rat\xA2n.");
		break;
	case 280:
		error("Error 280: No se encontr\xA2 el puntero de video.");
		break;
	case 281:
		error("Error 281: No se actualiz\xA2 el puntero de video.");
		break;
	case 282:
		error("Error 282: No se actualiz\xA2 el flag del trace.");
		break;
	case 299:
		error("Error 299: No se pudo reubicar los datos en la memoria XMS.");
		break;
	case 300:
		error("Error 300: No hay memoria XMS suficiente.");
		break;
	case 301:
		error("Error 301: No se pudo reservar memoria XMS suficiente.");
		break;
	case 302:
		error("Error 302: No se pudo liberar la memoria XMS.");
		break;
	case 303:
		error("Error 303: No se encuentra disponible un handle para XMS.");
		break;
	case 304:
		error("Error 304: No se encontr\xA2 el fichero de gr\xA0"
			  "ficos.");
		break;
	case 305:
		error("Error 305: Necesita una versi\xA2n 6.0 o posterior del driver del rat\xA2n.");
		break;
	case 306:
		error("Error 306: No se pudo inicializar correctamente el driver del rat\xA2n.");
		break;
	case 307:
		error("Error 307: No hay memoria baja suficiente.");
		break;
	case 308:
		error("Error 308: No se pudo inicializar correctamente el modo gr\xA0"
			  "fico.");
		break;
	case 309:
		error("Error 309: No se pudieron inicializar correctamente los tipos de letra.");
		break;
	case 310:
		error("Error 310: No se encontr\xA2 el fichero de paleta.");
		break;
	case 311:
		error("Error 311: No se encontr\xA2 el fichero de paleta para fundido.");
		break;
	case 312:
		error("Error 312: No se encontr\xA2 el fichero de los BITMAP del inventario.");
		break;
	case 313:
		error("Error 313: No se encontr\xA2 el fichero de hipertexto.");
		break;
	case 314:
		error("Error 314: No se encontr\xA2 el fichero del arbol de conversaciones.");
		break;
	case 315:
		error("Error 315: No se encontr\xA2 el fichero de la pantalla a pintar.");
		break;
	case 316:
		error("Error 316: No se encontr\xA2 el fichero de la pantalla a cargar.");
		break;
	case 317:
		error("Error 317: No se encontr\xA2 el fichero de las mascaras del rat\xA2n.");
		break;
	case 318:
		error("Error 318: No se encontr\xA2 el fichero del diploma.");
		break;
	case 319:
		error("Error 319: No se encontr\xA2 el fichero de los objetos en pantalla.");
		break;
	case 320:
		error("Error 320: No se encontr\xA2 el fichero de la rejilla de pantalla.");
		break;
	case 321:
		error("Error 321: No se encontr\xA2 el fichero del objeto rotando.");
		break;
	case 322:
		error("Error 322: No se encontr\xA2 el fichero de la partida salvada.");
		break;
	case 323:
		error("Error 323: No se encontr\xA2 el fichero de configuraci\xA2n.");
		break;
	default: {
		if (code >= 7 && code <= 11) {
			error("Error %d: Espera...  Espera... La placa madre est\xA0 teniendo gemelos", code);
		} else if (code >= 19 && code <= 99) {
			error("Error %d: Mi, mi, mi, mi, mi....  etc.", code);
		} else if (code >= 107 && code <= 149) {
			error("Error %d: Tr\xA0"
				  "fico intenso en el bus de datos.",
				  code);
		} else if (code >= 163 && code <= 199) {
			error("Error %d: No sabe, no contesta.", code);
		} else if (code >= 217 && code <= 249) {
			error("Error %d: Dios mio... estoy lleno de chips.", code);
		} else if (code >= 283 && code <= 298) {
			error("Error %d: Coffe not found. Operator Halted.", code);
		} else {
			error("Error %d:  La parab\xA2lica est\xA0 mal orientada.", code);
		}
	}
	}
}

void delay(uint16 ms) {
	g_engine->_chrono->delay(ms);
}

/*
Allows you to modify palette entries for the IBM
8514 and the VGA drivers.
*/
void setRGBPalette(int color, int r, int g, int b) {
	// Take current palette
	byte palbuf[768];
	g_system->getPaletteManager()->grabPalette(palbuf, 0, 256);

	palbuf[color * 3 + 0] = r << 2;
	palbuf[color * 3 + 1] = g << 2;
	palbuf[color * 3 + 2] = b << 2;

	g_system->getPaletteManager()->setPalette(palbuf, 0, 256);
}

void littText(int x, int y, const Common::String &text, byte color, Graphics::TextAlign align) {
	littText(x, y, text.c_str(), color, align);
}

void littText(int x, int y, char const *text, byte color, Graphics::TextAlign align) {
	bool yAligned = (align == Graphics::kTextAlignCenter) ? true : false;
	x = (align == Graphics::kTextAlignCenter) ? 0 : x;
	y = y + 2;
	g_engine->_graphics->littText(text, x, y, color, align, yAligned);
}

void euroText(int x, int y, const Common::String &text, byte color, Graphics::TextAlign align) {
	euroText(x, y, text.c_str(), color, align);
}

void euroText(int x, int y, char const *text, byte color, Graphics::TextAlign align) {
	bool yAligned = (align == Graphics::kTextAlignCenter) ? true : false;
	x = (align == Graphics::kTextAlignCenter) ? 0 : x;
	y = y + 2;
	g_engine->_graphics->euroText(text, x, y, color, align, yAligned);
}

void biosText(int x, int y, const Common::String &text, byte color) {
	g_engine->_graphics->biosText(text, x, y, color);
}

unsigned int imagesize(int x, int y, int x2, int y2) {
	int w = x2 - x + 1;
	int h = y2 - y + 1;
	return 4 + (w * h);
}

void rectangle(int x1, int y1, int x2, int y2, byte color) {
	g_engine->_screen->drawLine(x1, y1, x2, y1, color);
	g_engine->_screen->drawLine(x2, y1, x2, y2, color);
	g_engine->_screen->drawLine(x1, y2, x2, y2, color);
	g_engine->_screen->drawLine(x1, y1, x1, y2, color);
	g_engine->_screen->addDirtyRect(Common::Rect(x1, y1, x2, y2));
}

void bar(int x1, int y1, int x2, int y2, byte color) {
	x2 = x2 + 1;
	y2 = y2 + 1;
	for (int i = x1; i < x2; i++) {
		for (int j = y1; j < y2; j++) {
			*((byte *)g_engine->_screen->getBasePtr(i, j)) = color;
		}
	}
	g_engine->_screen->addDirtyRect(Common::Rect(x1, y1, x2, y2));
}

void putpixel(int x, int y, byte color) {
	g_engine->_screen->setPixel(x, y, color);
}

void line(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, byte color) {
	g_engine->_screen->drawLine(x1, y1, x2, y2, color);
}

void changeGameSpeed(Common::Event e) {
	if (e.type == Common::EVENT_KEYUP) {
		if (e.kbd.hasFlags(Common::KBD_CTRL)) {
			if (e.kbd.keycode == Common::KEYCODE_f) {
				g_engine->_chrono->changeSpeed();
			}
		}
	}
}

void emptyLoop() {
	do {
		g_engine->_chrono->updateChrono();
		g_engine->_screen->update();
	} while (!g_engine->_chrono->_gameTick && !g_engine->shouldQuit());
}

void emptyLoop2() {
	do {
		g_engine->_chrono->updateChrono();
		g_engine->_screen->update();
	} while (!g_engine->_chrono->_gameTickHalfSpeed);
}

void waitForKey() {
	bool teclapulsada = false;
	Common::Event e;
	debug("Waiting for key!");
	while (!teclapulsada && !g_engine->shouldQuit()) {
		while (g_system->getEventManager()->pollEvent(e)) {
			if (e.type == Common::EVENT_KEYDOWN) {
				teclapulsada = true;
			}
		}

		g_engine->_screen->update();
		g_system->delayMillis(10);
	}
}

unsigned int Random(unsigned int range) { return g_engine->getRandomNumber(range); }
int Random(int range) { return g_engine->getRandomNumber(range - 1); }

} // End of namespace Tot
