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

#ifndef SWORD25_DEBUGTOOLS_H
#define SWORD25_DEBUGTOOLS_H

class BS_Debugtools
{
public:
	/**
	    @brief Gibt eine ID zurück, die die benutzte Programmversion eindeutig identifiziert.

		Um die Version zu ermitteln wird der MD5-Hash über die EXE-Datei gebildet.
		Falls die ausführende Datei in einem SVN-Repository liegt wird zusätzlich die Revision des Verzeichnisses gehasht.

		@return Gibt einen String zurück, der die Versions-ID des Programmes angibt.<br>
				Falls die Versions-ID nicht bestimmt werden konnte wird "???" zurückgegeben.
		@remark Diese Methode ist momentan nur für WIN32 implementiert.
	*/
	static const char * GetVersionID();

	/**

		@brief Gibt die Subversion-Revisionsnummer der Engine zurück.

		Diese Funktion versucht die aktuelle Revision aus der SVN entries Datei für das aktuelle Verzeichnis zu extrahieren.
		Dabei werden die SVN entries Formatversionen 7 und größer unterstützt.
		Die neueste Version ist aktuell 9. Für folgende Versionen wird angenommen, dass sich das Format des Headers nicht mehr ändert.
	
		@return Gibt die Revisionsnummer zurück. Falls die ausführende Datei nicht in einem SVN-Repository liegt oder die Revision nicht
				festgestellt werden konnte wird 0 zurückgegeben.	
	*/
	static unsigned int GetSubversionRevision();
};

#endif
