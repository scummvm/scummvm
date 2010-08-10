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

#ifndef SWORD25_BITMAP_H
#define SWORD25_BITMAP_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "sword25/kernel/common.h"
#include "sword25/gfx/renderobject.h"

namespace Sword25 {

// -----------------------------------------------------------------------------
// Klassendeklaration
// -----------------------------------------------------------------------------

class BS_Bitmap : public BS_RenderObject {
protected:
	BS_Bitmap(BS_RenderObjectPtr<BS_RenderObject> ParentPtr, TYPES Type, unsigned int Handle = 0);

public:

	virtual ~BS_Bitmap();

	/**
	    @brief Setzt den Alphawert des Bitmaps.
	    @param Alpha der neue Alphawert der Bitmaps (0 = keine Deckung, 255 = volle Deckung).
	    @remark Diese Methode darf nur aufgerufen werden, wenn die Methode IsAlphaAllowed() true zurückgibt.
	*/
	void SetAlpha(int Alpha);

	/**
	    @brief Setzt die Modulationfarbe der Bitmaps.
	    @param Color eine 24-Bit Farbe, die die Modulationsfarbe des Bitmaps festlegt.
	    @remark Diese Methode darf nur aufgerufen werden, wenn die Methode IsColorModulationAllowed() true zurückgibt.
	*/
	void SetModulationColor(unsigned int ModulationColor);

	/**
	    @brief Setzt den Skalierungsfaktor des Bitmaps.
	    @param ScaleFactor der Faktor um den das Bitmap in beide Richtungen gestreckt werden soll.
	    @remark Diese Methode darf nur aufgerufen werden, wenn die Methode IsScalingAllowed() true zurückgibt.
	*/
	void SetScaleFactor(float ScaleFactor);

	/**
	    @brief Setzt den Skalierungsfaktor der Bitmap auf der X-Achse.
	    @param ScaleFactor der Faktor um den die Bitmap in Richtungen der X-Achse gestreckt werden soll. Dieser Wert muss positiv sein.
	    @remark Diese Methode darf nur aufgerufen werden, wenn die Methode IsScalingAllowed() true zurückgibt.
	*/
	void SetScaleFactorX(float ScaleFactorX);

	/**
	    @brief Setzt den Skalierungsfaktor der Bitmap auf der Y-Achse.
	    @param ScaleFactor der Faktor um den die Bitmap in Richtungen der Y-Achse gestreckt werden soll. Dieser Wert muss positiv sein.
	    @remark Diese Methode darf nur aufgerufen werden, wenn die Methode IsScalingAllowed() true zurückgibt.
	*/
	void SetScaleFactorY(float ScaleFactorY);

	/**
	    @brief Legt fest, ob das Bild an der X-Achse gespiegelt werden soll.
	*/
	void SetFlipH(bool FlipH);

	/**
	    @brief Legt fest, ob das Bild an der Y-Achse gespiegelt werden soll.
	*/
	void SetFlipV(bool FlipV);

	/**
	    @brief Gibt den aktuellen Alphawert des Bildes zurück.
	    @remark Diese Methode darf nur aufgerufen werden, wenn die Methode IsAlphaAllowed() true zurückgibt.
	*/
	int GetAlpha() {
		return m_ModulationColor >> 24;
	}

	/**
	    @brief Gibt die aktuelle 24bit RGB Modulationsfarde des Bildes zurück.
	    @remark Diese Methode darf nur aufgerufen werden, wenn die Methode IsColorModulationAllowed() true zurückgibt.
	*/
	int GetModulationColor() {
		return m_ModulationColor & 0x00ffffff;
	}

	/**
	    @brief Gibt den Skalierungsfakter des Bitmaps auf der X-Achse zurück.
	    @remark Diese Methode darf nur aufgerufen werden, wenn die Methode IsScalingAllowed() true zurückgibt.
	*/
	float GetScaleFactorX() const {
		return m_ScaleFactorX;
	}

	/**
	    @brief Gibt den Skalierungsfakter des Bitmaps auf der Y-Achse zurück.
	    @remark Diese Methode darf nur aufgerufen werden, wenn die Methode IsScalingAllowed() true zurückgibt.
	*/
	float GetScaleFactorY() const {
		return m_ScaleFactorY;
	}

	/**
	    @brief Gibt zurück, ob das Bild an der X-Achse gespiegelt angezeigt wird.
	*/
	bool IsFlipH() {
		return m_FlipH;
	}

	/**
	    @brief Gibt zurück, ob das Bild an der Y-Achse gespiegelt angezeigt wird.
	*/
	bool IsFlipV() {
		return m_FlipV;
	}

	// -----------------------------------------------------------------------------
	// Die folgenden Methoden müssen alle BS_Bitmap-Klassen implementieren
	// -----------------------------------------------------------------------------

	/**
	    @brief Liest einen Pixel des Bildes.
	    @param X die X-Koordinate des Pixels.
	    @param Y die Y-Koordinate des Pixels
	    @return Gibt den 32-Bit Farbwert des Pixels an der übergebenen Koordinate zurück.
	    @remark Diese Methode sollte auf keine Fall benutzt werden um größere Teile des Bildes zu lesen, da sie sehr langsam ist. Sie ist
	            eher dafür gedacht einzelne Pixel des Bildes auszulesen.
	*/
	virtual unsigned int GetPixel(int X, int Y) const = 0;

	/**
	    @brief Füllt den Inhalt des Bildes mit Pixeldaten.
	    @param Pixeldata ein Vector der die Pixeldaten enthält. Sie müssen in dem Farbformat des Bildes vorliegen und es müssen genügend Daten
	           vorhanden sein, um das ganze Bild zu füllen.
	    @param Offset der Offset in Byte im Pixeldata-Vector an dem sich der erste zu schreibende Pixel befindet.<br>
	                  Der Standardwert ist 0.
	    @param Stride der Abstand in Byte zwischen dem Zeilenende und dem Beginn einer neuen Zeile im Pixeldata-Vector.<br>
	                  Der Standardwert ist 0.
	    @return Gibt false zurück, falls der Aufruf fehlgeschlagen ist.
	    @remark Ein Aufruf dieser Methode ist nur erlaubt, wenn IsSetContentAllowed() true zurückgibt.
	*/
	virtual bool    SetContent(const byte *Pixeldata, uint size, unsigned int Offset = 0, unsigned int Stride = 0) = 0;

	virtual bool    IsScalingAllowed() const = 0;
	virtual bool    IsAlphaAllowed() const = 0;
	virtual bool    IsColorModulationAllowed() const = 0;
	virtual bool    IsSetContentAllowed() const = 0;

	virtual bool    Persist(BS_OutputPersistenceBlock &Writer);
	virtual bool    Unpersist(BS_InputPersistenceBlock &Reader);

protected:
	bool            m_FlipH;
	bool            m_FlipV;
	float           m_ScaleFactorX;
	float           m_ScaleFactorY;
	unsigned int    m_ModulationColor;
	int             m_OriginalWidth;
	int             m_OriginalHeight;
};

} // End of namespace Sword25

#endif
