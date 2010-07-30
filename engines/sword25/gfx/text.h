// -----------------------------------------------------------------------------
// This file is part of Broken Sword 2.5
// Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdörfer
//
// Broken Sword 2.5 is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Broken Sword 2.5 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Broken Sword 2.5; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
// -----------------------------------------------------------------------------

#ifndef SWORD25_TEXT_H
#define SWORD25_TEXT_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "sword25/kernel/common.h"
#include "sword25/math/rect.h"
#include "sword25/gfx/renderobject.h"

#include "sword25/kernel/memlog_off.h"
#include <vector>
#include "sword25/kernel/memlog_on.h"

// -----------------------------------------------------------------------------
// Forward Declarations
// -----------------------------------------------------------------------------

class BS_Kernel;
class BS_FontResource;
class BS_ResourceManager;

// -----------------------------------------------------------------------------
// Klassendefinition
// -----------------------------------------------------------------------------

class BS_Text : public BS_RenderObject
{
friend class BS_RenderObject;

public:
	/**
		@brief Setzt den Font mit dem der Text dargestellt werden soll.
		@param Font der Dateiname der Fontdatei.
		@return Gibt false zurück, wenn der Font nicht gefunden wurde.
	*/
	bool SetFont(const std::string & Font);

	/**
		@brief Setzt den darzustellenden Text.
		@param Text der darzustellende Text
	*/
	void SetText(const std::string & Text);

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
	const std::string & GetText() { return m_Text; }

	/**
	    @brief Gibt den Namen das momentan benutzten Fonts zurück.
	*/
	const std::string & GetFont() { return m_Font; }

	/**
		@brief Setzt die Farbe des Textes.
		@param Color eine 24-Bit RGB Farbe, die die Farbe des Textes festlegt.
	*/
	void SetColor(unsigned int ModulationColor);

	/**
		@brief Gibt den Alphawert des Textes zurück.
		@return Der Alphawert des Textes (0 = keine Deckung, 255 = volle Deckung).
	*/
	int GetAlpha() const { return m_ModulationColor >> 24; }

	/**
		@brief Gibt die Farbe des Textes zurück.
		@return Eine 24-Bit RGB Farbe, die die Farbe des Textes angibt.
	*/
	int GetColor() const { return m_ModulationColor & 0x00ffffff; }

	/**
		@brief Gibt zurück, ob die automatische Formatierung aktiviert ist.
	*/
	bool IsAutoWrapActive() const { return m_AutoWrap; }

	/**
		@brief Gibt die Längengrenze des Textes in Pixeln zurück, ab der eine automatische Formatierung vorgenommen wird.
	*/
	unsigned int GetAutoWrapThreshold() const { return m_AutoWrapThreshold; }

	virtual bool	Persist(BS_OutputPersistenceBlock & Writer);
	virtual bool	Unpersist(BS_InputPersistenceBlock & Reader);

protected:
	virtual bool DoRender();

private:
	BS_Text(BS_RenderObjectPtr<BS_RenderObject> ParentPtr);
	BS_Text(BS_InputPersistenceBlock & Reader, BS_RenderObjectPtr<BS_RenderObject> ParentPtr, unsigned int Handle);

	unsigned int	m_ModulationColor;
	std::string		m_Font;
	std::string		m_Text;
	bool			m_AutoWrap;
	unsigned int	m_AutoWrapThreshold;

	struct LINE
	{
		BS_Rect		BBox;
		std::string	Text;
	};

	std::vector<LINE>	m_Lines;

	void UpdateFormat();
	void UpdateMetrics(BS_FontResource & FontResource);
	BS_ResourceManager * GetResourceManager();
	BS_FontResource * LockFontResource();
};

#endif
