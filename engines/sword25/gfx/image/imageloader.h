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
 * $URL$
 * $Id$
 *
 */

/*
 * This code is based on Broken Sword 2.5 engine
 *
 * Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdoerfer
 *
 * Licensed under GNU GPL v2
 *
 */

/*
    BS_ImageLoader
    --------------

    Autor: Malte Thiesen
*/

#ifndef SWORD25_IMAGELOADER_H
#define SWORD25_IMAGELOADER_H

// Includes
#include "sword25/kernel/bs_stdint.h"
#include "sword25/kernel/common.h"
#include "sword25/gfx/graphicengine.h"

namespace Sword25 {

/**
    @brief Über die statischen Methoden dieser Klasse werden alle unterstützten Bildformate geladen.

    Zum Laden von Bildern wird die #LoadImage-Methode benutzt.

    Außerdem stellt diese Klasse das Interface da, das alle Klassen implementieren müssen, die Bildformate einlesen.<br>
    Zur Unterstützung eines neuen Bildformates muss folgendermaßen vorgegangen werden:
    - Erzeugen einer neuen von #BS_ImageLoader abgeleiteten Klasse, die die Methoden #IsCorrectImageFormat und #DecodeImage impelementiert.
    - Die Klasse muss eine statische Methode haben, die eine Instanz von ihr erzeugt und einen Pointer darauf zurückgibt.
    - Diese Methode muss in der Liste in der Datei imageloader_ids.h eingetragen werden.
    - Die Klasse muss JEDES Eingabebild seines Bildformates in die folgenden Farbformate konvertieren können:
        - BS_GraphicEngine::CF_RGB16
        - BS_GraphicEngine::CF_RGB15
        - BS_GraphicEngine::CF_RGB16_INTERLEAVED
        - BS_GraphicEngine::CF_RGB15_INTERLEAVED
        - BS_GraphicEngine::CF_ARGB32
        - BS_GraphicEngine::CF_BGRA32
    - Zum Konvertieren der Bilddaten können die Hilfsmethoden dieser Klasse benutzt werden, die ARGB Bilddaten in alle benötigten
      Farbformate konvertieren.
*/
class ImageLoader {
public:

	//@{
	/** @name Lade Methoden */

	/**
	    @brief Lädt eine Bilddatei.

	    Diese Methode kann sämtliche unterstütztem Bildformate lesen. Die Methode erkennt selbstständing um welches Dateiformat es sich
	    bei der vorliegenden Datei handelt.<br>
	    Bisher wird nur PNG unterstützt.

	    @param pFileData ein Pointer auf die Bilddaten.
	    @param FileSize die Größe der Bilddaten in Byte.
	    @param ColorFormat gibt das gewünschte Farbformat an, in das die Bilddaten konvertiert werden sollen.<br>
	                       Folgende Farbformate werden unterstützt:
	                       - BS_GraphicEngine::CF_RGB16
	                       - BS_GraphicEngine::CF_RGB15
	                       - BS_GraphicEngine::CF_RGB16_INTERLEAVED
	                       - BS_GraphicEngine::CF_RGB15_INTERLEAVED
	                       - BS_GraphicEngine::CF_ARGB32
	    @param pUncompressedData nach erfolgreichen Laden zeigt dieser Pointer auf die enpackten und konvertierten Bilddaten.
	    @param Width gibt nach erfolgreichen Laden die Breite des geladenen Bildes an.
	    @param Height gibt nach erfolgreichen Laden die Höhe des geladenen Bildes an.
	    @param Pitch gibt nach erfolgreichen Laden die Länge einer Bildzeile in Byte an.
	    @return Gibt false zurück, falls das Laden fehlgeschlagen ist.
	    @remark Die Größe der Ausgabedaten in Bytes kann wie folgt berechnet werden: Pitch * Height.
	    @remark Es darf nicht vergessen werden, die Ausgabedaten nach erfolgter Benutzung mit delete freizugeben.
	*/
	static bool LoadImage(const byte *pFileData, uint FileSize,
	                      GraphicEngine::COLOR_FORMATS ColorFormat,
	                      byte *&pUncompressedData,
	                      int &Width, int &Height,
	                      int &Pitch);

	/**
	    @brief Liest die Bildeigenschaften eines Bildes aus.

	    @param pFileData ein Pointer auf die Bilddaten.
	    @param FileSize die Größe des Bilddaten in Byte.
	    @param ColorFormat enthält nach einem erfolgreichem Aufruf das Farbformat des Bildes.
	    @param Width enthält nach einem erfolgreichem Aufruf die Breite des Bildes in Pixeln.
	    @param Height enthält nach einem erfolgreichem Aufruf die Höhe des Bildes in Pixeln.
	    @return Gibt false zurück, wenn die Bildeigenschaften nicht ausgelesen werden konnten.
	    @remark Es darf nicht vergessen werden, die Ausgabedaten nach erfolgter Benutzung mit delete freizugeben.
	*/
	static bool ExtractImageProperties(const byte *pFileData, uint FileSize,
	                                   GraphicEngine::COLOR_FORMATS &ColorFormat,
	                                   int &Width, int &Height);
	//@}

protected:

	// Protected Konstruktor, damit Instanzen dieser Klasse nur von BS_ImageLoader-Objekten erstellt werden können
	/**
	    @brief Der Standardkonstruktor.

	    Dieser Konstruktor registriert alle Instanzen von #BS_ImageLoader-Klassen in einer Liste.<br>
	    Diese Liste enthält jeweils eine Instanz jedes #BS_ImageLoader und wird benutzt um beliebige Bilddateien einem Loader zuzuordnen.
	    @remark Dieser Konstruktor ist protected damit nur #BS_ImageLoader-Objekte diese Klasse instanziieren können.
	*/
	ImageLoader() {
		// Klasse registrieren
		_ImageLoaderList.push_front(this);
	}

	virtual ~ImageLoader() {}

	//@{
	/** @name Abstrakte Methoden */

	/**
	    @brief Gibt an, ob der #BS_ImageLoader ein Bild lesen kann.
	    @param pFileData ein Pointer auf die kompletten Daten des Bildes.
	    @param FileSize die Größe der Daten in Byte.
	    @return Gibt true zurück, wenn der #BS_ImageLoader das Bild lesen kann, ansonsten false.
	    @remark Diese Methode muss von allen BS_ImageLoader Klassen implementiert werden.
	*/
	virtual bool IsCorrectImageFormat(const byte *pFileData, uint FileSize) = 0;

	/**
	    @brief Lädt eine Bilddatei.
	    @param pFileData ein Pointer auf die Bilddaten.
	    @param FileSize die Größe der Bilddaten in Byte.
	    @param ColorFormat gibt das gewünschte Farbformat an, in das die Bilddaten konvertiert werden sollen.<br>
	                       Folgende Farbformate werden unterstützt:
	                       - BS_GraphicEngine::CF_RGB16
	                       - BS_GraphicEngine::CF_RGB15
	                       - BS_GraphicEngine::CF_RGB16_INTERLEAVED
	                       - BS_GraphicEngine::CF_RGB15_INTERLEAVED
	                       - BS_GraphicEngine::CF_ARGB32
	    @param pUncompressedData nach erfolgreichen Laden zeigt dieser Pointer auf die enpackten und konvertierten Bilddaten.
	    @param Width gibt nach erfolgreichen Laden die Breite des geladenen Bildes an.
	    @param Height gibt nach erfolgreichen Laden die Höhe des geladenen Bildes an.
	    @param Pitch gibt nach erfolgreichen Laden die Länge einer Bildzeile in Byte an.
	    @return Gibt false zurück, falls das Laden fehlgeschlagen ist.
	    @remark Die Größe der Ausgabedaten in Bytes kann wie folgt berechnet werden: Pitch * Height.
	    @remark Es darf nicht vergessen werden, die Ausgabedaten nach erfolgter Benutzung mit delete freizugeben.
	    @remark Diese Methode muss von allen BS_ImageLoader Klassen implementiert werden.
	*/
	virtual bool DecodeImage(const byte *pFileData, uint FileSize,
	                         GraphicEngine::COLOR_FORMATS ColorFormat,
	                         byte *&pUncompressedData,
	                         int &Width, int &Height,
	                         int &Pitch) = 0;

	/**
	    @brief Liest die Bildeigenschaften aus.
	    @param pFileData ein Pointer auf die Bilddaten.
	    @param FileSize die Größe des Bilddaten in Byte.
	    @param ColorFormat enthält nach einem erfolgreichem Aufruf das Farbformat des Bildes.
	    @param Width enthält nach einem erfolgreichem Aufruf die Breite des Bildes in Pixeln.
	    @param Height enthält nach einem erfolgreichem Aufruf die Höhe des Bildes in Pixeln.
	    @return Gibt false zurück, wenn die Bildeigenschaften nicht ausgelesen werden konnten.
	    @remark Es darf nicht vergessen werden, die Ausgabedaten nach erfolgter Benutzung mit delete freizugeben.
	    @remark Diese Methode muss von allen BS_ImageLoader Klassen implementiert werden.
	*/
	virtual bool ImageProperties(const byte *pFileData, uint FileSize,
	                             GraphicEngine::COLOR_FORMATS &ColorFormat,
	                             int &Width, int &Height) = 0;

	//@}

	//@{
	/** @name Konvertierungsmethoden */

	/**
	    @brief Konvertiert eine Bildzeile mit ARGB Pixeldaten in das BS_GraphicEngine::CF_RGB16 Farbformat.
	    @param pSrcData ein Pointer auf die Quelldaten.
	    @param pDestData ein Pointer auf den Zielpuffern.
	    @param Width die Anzahl der Pixel in der Bildzeile.
	    @remark Es gilt zu beachten, dass der Zielpuffer ausreichend groß ist.<br>
	            Es sind mindestens Width * 2 Byte notwendig.
	*/
	static void RowARGB32ToRGB16(byte *pSrcData, byte *pDestData, uint Width) {
		for (uint i = 0; i < Width; i++) {
			((uint16_t *)pDestData)[i] = ((pSrcData[2] >> 3) << 11) | ((pSrcData[1] >> 2) << 5) | (pSrcData[0] >> 3);
			pSrcData += 4;
		}
	}

	/**
	    @brief Konvertiert eine Bildzeile mit ARGB Pixeldaten in das BS_GraphicEngine::CF_RGB15 Farbformat.
	    @param pSrcData ein Pointer auf die Quelldaten.
	    @param pDestData ein Pointer auf den Zielpuffern.
	    @param Width die Anzahl der Pixel in der Bildzeile.
	    @remark Es gilt zu beachten, dass der Zielpuffer ausreichend groß ist.<br>
	            Es sind mindestens Width * 2 Byte notwendig.
	*/
	static void RowARGB32ToRGB15(byte *pSrcData, byte *pDestData, uint Width) {
		for (uint i = 0; i < Width; i++) {
			((uint16_t *)pDestData)[i] = ((pSrcData[2] >> 3) << 10) | ((pSrcData[1] >> 3) << 5) | (pSrcData[0] >> 3);
			pSrcData += 4;
		}
	}

	/**
	    @brief Konvertiert eine Bildzeile mit ARGB Pixeldaten in das BS_GraphicEngine::CF_RGB16_INTERLEAVED Farbformat.
	    @param pSrcData ein Pointer auf die Quelldaten.
	    @param pDestData ein Pointer auf den Zielpuffern.
	    @param Width die Anzahl der Pixel in der Bildzeile.
	    @remark Es gilt zu beachten, dass der Zielpuffer ausreichend groß sein muss.<br>
	            Es sind mindestens ((Width + 3) / 4) * 12 Byte notwendig.
	*/
	static void RowARGB32ToRGB16_INTERLEAVED(byte *pSrcData, byte *pDestData, uint Width) {
		// Die Pixelblöcke erstellen, dabei werden immer jeweils 4 Pixel zu einem Block zusammengefasst
		uint BlockFillCount = 0;
		uint AlphaBlock = 0;
		for (uint i = 0; i < Width; i++) {
			// Alphawert in den Alphablock schreiben
			AlphaBlock = (AlphaBlock >> 8) | (pSrcData[BlockFillCount * 4 + 3] << 24);

			// Füllstand der Pixelblockes aktualisieren
			BlockFillCount++;

			// Sobald 4 Alphawerte gesammelt wurden, oder die Zeile zu Ende ist wird der Pixelblock in den Zielpuffer geschrieben
			if (BlockFillCount == 4 || i == (Width - 1)) {
				// Falls der AlphaBlock nicht ganz gefüllt ist muss geshiftet werden um sicherzustellen, dass die Alphawerte
				// "left aligned" sind.
				AlphaBlock >>= (4 - BlockFillCount) * 8;

				// Alphablock schreiben
				*((uint *)pDestData) = AlphaBlock;
				pDestData += 4;

				// Pixel konvertieren und schreiben
				RowARGB32ToRGB16(pSrcData, pDestData, BlockFillCount);

				// Pointer auf den nächsten Platz im Zielpuffer setzen
				pDestData += 8;

				// Pointer auf die nächsten 4 Pixel im Quellpuffer setzen
				pSrcData += 16;

				// Neuen Pixelblock beginnen
				BlockFillCount = 0;
			}
		}
	}

	/**
	    @brief Konvertiert eine Bildzeile mit ARGB Pixeldaten in das BS_GraphicEngine::CF_RGB15_INTERLEAVED Farbformat.
	    @param pSrcData ein Pointer auf die Quelldaten.
	    @param pDestData ein Pointer auf den Zielpuffern.
	    @param Width die Anzahl der Pixel in der Bildzeile.
	    @remark Es gilt zu beachten, dass der Zielpuffer ausreichend groß ist.<br>
	            Es sind mindestens (Width / 4 + Width % 4) * 3 Byte notwendig.
	*/
	static void RowARGB32ToRGB15_INTERLEAVED(byte *pSrcData, byte *pDestData, uint Width) {
		// Die Pixelblöcke erstellen, dabei werden immer jeweils 4 Pixel zu einem Block zusammengefasst
		uint BlockFillCount = 0;
		uint AlphaBlock = 0;
		for (uint i = 0; i < Width; i++) {
			// Alphawert in den Alphablock schreiben
			AlphaBlock = (AlphaBlock >> 8) | (pSrcData[BlockFillCount * 4 + 3] << 24);

			// Füllstand der Pixelblockes aktualisieren
			BlockFillCount++;

			// Sobald 4 Alphawerte gesammelt wurden, oder die Zeile zu Ende ist wird der Pixelblock in den Zielpuffer geschrieben
			if (BlockFillCount == 4 || i == (Width - 1)) {
				// Falls der AlphaBlock nicht ganz gefüllt ist muss geshiftet werden um sicherzustellen, dass die Alphawerte
				// "left aligned" sind.
				AlphaBlock >>= (4 - BlockFillCount) * 8;

				// Alphablock schreiben
				*((uint *)pDestData) = AlphaBlock;
				pDestData += 4;

				// Pixel konvertieren und schreiben
				RowARGB32ToRGB15(pSrcData, pDestData, BlockFillCount);

				// Pointer auf den nächsten Platz im Zielpuffer setzen
				pDestData += 8;

				// Pointer auf die nächsten 4 Pixel im Quellpuffer setzen
				pSrcData += 16;

				// Neuen Pixelblock beginnen
				BlockFillCount = 0;
			}
		}
	}

	/**
	    @brief Konvertiert eine Bildzeile mit ARGB Pixeldaten in das BS_GraphicEngine::CF_BGRA32 Farbformat.
	    @param pSrcData ein Pointer auf die Quelldaten.
	    @param pDestData ein Pointer auf den Zielpuffern.
	    @param Width die Anzahl der Pixel in der Bildzeile.
	*/
	static void RowARGB32ToABGR32(byte *pSrcData, byte *pDestData, uint Width) {
		for (uint i = 0; i < Width; ++i) {
			*pDestData++ = pSrcData[2];
			*pDestData++ = pSrcData[1];
			*pDestData++ = pSrcData[0];
			*pDestData++ = pSrcData[3];

			pSrcData += 4;
		}
	}

private:

	/**
	    @brief Erzeugt je eine Instanz aller BS_ImageLoader Klassen und fügt diese in eine interne Liste ein. Diese werden dann beim
	           Laden von Bildern benutzt.
	    @remark Die Klassen müssen in der Datei imageloader_ids.h eingetragen sein, damit sie an dieser Stelle berücksichtigt werden.
	*/
	static void _InitializeLoaderList();

	/**
	    @brief Zerstört alle Instanzen von BS_ImageLoader Klassen, die in dieser Klasse registriert sind.
	*/
	static void _DeinitializeLoaderList();

	/**
	    @brief Sucht zu Bilddaten ein BS_ImageLoader Objekt, dass die Bilddaten dekodieren kann.
	    @return Gibt einen Pointer auf ein passendes BS_ImageLoader Objekt zurück, oder NULL, wenn kein passendes Objekt gefunden wurde.
	*/
	static ImageLoader *_FindSuitableImageLoader(const byte *pFileData, uint FileSize);

	static Common::List<ImageLoader *>   _ImageLoaderList;              // Die Liste aller BS_ImageLoader-Objekte
	static bool                         _ImageLoaderListInitialized;    // Gibt an, ob die Liste schon intialisiert wurde
};

} // End of namespace Sword25

#endif
