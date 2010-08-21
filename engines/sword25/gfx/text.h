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

#ifndef SWORD25_TEXT_H
#define SWORD25_TEXT_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "sword25/kernel/common.h"
#include "common/rect.h"
#include "sword25/gfx/renderobject.h"

namespace Sword25 {

// -----------------------------------------------------------------------------
// Forward Declarations
// -----------------------------------------------------------------------------

class Kernel;
class FontResource;
class ResourceManager;

// -----------------------------------------------------------------------------
// Klassendefinition
// -----------------------------------------------------------------------------

class Text : public RenderObject {
	friend class RenderObject;

public:
	/**
	    @brief Setzt den Font mit dem der Text dargestellt werden soll.
	    @param Font der Dateiname der Fontdatei.
	    @return Gibt false zurück, wenn der Font nicht gefunden wurde.
	*/
	bool SetFont(const Common::String &Font);

	/**
	    @brief Setzt den darzustellenden Text.
	    @param Text der darzustellende Text
	*/
	void SetText(const Common::String &text);

	/**
	    @brief Setzt den Alphawert des Textes.
	    @param Alpha der neue Alphawert des Textes (0 = keine Deckung, 255 = volle Deckung).
	*/
	void SetAlpha(int Alpha);

	/**
	    @brief Legt fest, ob der Text automatisch umgebrochen werden soll.

	    Wenn dieses Attribut auf true gesetzt ist, wird der Text umgebrochen, sofern er länger als GetAutoWrapThreshold() ist.

	    @param AutoWrap gibt an, ob der automatische Umbruch aktiviert oder deaktiviert werden soll.
	    @remark Dieses Attribut wird mit dem Wert false initialisiert.
	*/
	void SetAutoWrap(bool AutoWrap);

	/**
	    @brief Legt die Längengrenze des Textes in Pixeln fest, ab der ein automatischer Zeilenumbruch vorgenommen wird.
	    @remark Dieses Attribut wird mit dem Wert 300 initialisiert.
	    @remark Eine automatische Formatierung wird nur vorgenommen, wenn diese durch einen Aufruf von SetAutoWrap() aktiviert wurde.
	*/
	void SetAutoWrapThreshold(unsigned int AutoWrapThreshold);

	/**
	    @brief Gibt den dargestellten Text zurück.
	*/
	const Common::String &GetText() {
		return m_Text;
	}

	/**
	    @brief Gibt den Namen das momentan benutzten Fonts zurück.
	*/
	const Common::String &GetFont() {
		return m_Font;
	}

	/**
	    @brief Setzt die Farbe des Textes.
	    @param Color eine 24-Bit RGB Farbe, die die Farbe des Textes festlegt.
	*/
	void SetColor(unsigned int ModulationColor);

	/**
	    @brief Gibt den Alphawert des Textes zurück.
	    @return Der Alphawert des Textes (0 = keine Deckung, 255 = volle Deckung).
	*/
	int GetAlpha() const {
		return m_ModulationColor >> 24;
	}

	/**
	    @brief Gibt die Farbe des Textes zurück.
	    @return Eine 24-Bit RGB Farbe, die die Farbe des Textes angibt.
	*/
	int GetColor() const {
		return m_ModulationColor & 0x00ffffff;
	}

	/**
	    @brief Gibt zurück, ob die automatische Formatierung aktiviert ist.
	*/
	bool IsAutoWrapActive() const {
		return m_AutoWrap;
	}

	/**
	    @brief Gibt die Längengrenze des Textes in Pixeln zurück, ab der eine automatische Formatierung vorgenommen wird.
	*/
	unsigned int GetAutoWrapThreshold() const {
		return m_AutoWrapThreshold;
	}

	virtual bool    Persist(OutputPersistenceBlock &Writer);
	virtual bool    Unpersist(InputPersistenceBlock &Reader);

protected:
	virtual bool DoRender();

private:
	Text(RenderObjectPtr<RenderObject> ParentPtr);
	Text(InputPersistenceBlock &Reader, RenderObjectPtr<RenderObject> ParentPtr, unsigned int Handle);

	unsigned int    m_ModulationColor;
	Common::String      m_Font;
	Common::String      m_Text;
	bool            m_AutoWrap;
	unsigned int    m_AutoWrapThreshold;

	struct LINE {
		Common::Rect     BBox;
		Common::String  Text;
	};

	Common::Array<LINE> m_Lines;

	void UpdateFormat();
	void UpdateMetrics(FontResource &FontResource);
	ResourceManager *GetResourceManager();
	FontResource *LockFontResource();
};

} // End of namespace Sword25

#endif
