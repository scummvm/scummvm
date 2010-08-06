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
    BS_RenderObjectManager
    ----------------------
    Diese Klasse ist für die Verwaltung von BS_RenderObjects zuständig.

    Sie sorgt z.B. dafür, dass die BS_RenderObjects in der richtigen Reihenfolge gerendert werden.

    Autor: Malte Thiesen
*/

#ifndef SWORD25_RENDEROBJECTMANAGER_H
#define SWORD25_RENDEROBJECTMANAGER_H

// Includes
#include "sword25/kernel/memlog_off.h"
#include <vector>
#include "sword25/kernel/memlog_on.h"

#include "sword25/kernel/common.h"
#include "sword25/gfx/renderobjectptr.h"
#include "sword25/kernel/persistable.h"

namespace Sword25 {

// Klassendefinition
class BS_Kernel;
class BS_Rect;
class BS_RenderObject;
class BS_TimedRenderObject;

/**
    @brief Diese Klasse ist für die Verwaltung von BS_RenderObjects zuständig.

    Sie sorgt dafür, dass die BS_RenderObjects in der richtigen Reihenfolge gerendert werden und ermöglicht den Zugriff auf die
    BS_RenderObjects über einen String.
*/
class BS_RenderObjectManager : public BS_Persistable {
public:
	/**
	    @brief Erzeugt ein neues BS_RenderObjectManager-Objekt.
	    @param Width die horizontale Bildschirmauflösung in Pixeln
	    @param Height die vertikale Bildschirmauflösung in Pixeln
	    @param Die Anzahl an Framebuffern, die eingesetzt wird (Backbuffer + Primary).
	*/
	BS_RenderObjectManager(int Width, int Height, int FramebufferCount);
	virtual ~BS_RenderObjectManager();

	// Interface
	// ---------
	/**
	    @brief Initialisiert den Manager für einen neuen Frame.
	    @remark Alle Veränderungen an Objekten müssen nach einem Aufruf dieser Methode geschehen, damit sichergestellt ist, dass diese
	            visuell umgesetzt werden.<br>
	            Mit dem Aufruf dieser Methode werden die Rückgabewerte von GetUpdateRects() und GetUpdateRectCount() auf ihre Startwerte
	            zurückgesetzt. Wenn man also mit diesen Werten arbeiten möchten, muss man dies nach einem Aufruf von Render() und vor
	            einem Aufruf von StartFrame() tun.
	 */
	void StartFrame();
	/**
	    @brief Rendert alle Objekte die sich während des letzten Aufrufes von Render() verändert haben.
	    @return Gibt false zurück, falls das Rendern fehlgeschlagen ist.
	 */
	bool Render();
	/**
	    @brief Gibt einen Pointer auf die Wurzel des Objektbaumes zurück.
	 */
	BS_RenderObjectPtr<BS_RenderObject> GetTreeRoot() {
		return m_RootPtr;
	}
	/**
	    @brief Fügt ein BS_TimedRenderObject in die Liste der zeitabhängigen Render-Objekte.

	    Alle Objekte die sich in dieser Liste befinden werden vor jedem Frame über die seit dem letzten Frame
	    vergangene Zeit informiert, so dass sich ihren Zustand zeitabhängig verändern können.

	    @param RenderObject das einzufügende BS_TimedRenderObject
	*/
	void AttatchTimedRenderObject(BS_RenderObjectPtr<BS_TimedRenderObject> pRenderObject);
	/**
	    @brief Entfernt ein BS_TimedRenderObject aus der Liste für zeitabhängige Render-Objekte.
	*/
	void DetatchTimedRenderObject(BS_RenderObjectPtr<BS_TimedRenderObject> pRenderObject);

	virtual bool Persist(BS_OutputPersistenceBlock &Writer);
	virtual bool Unpersist(BS_InputPersistenceBlock &Reader);

private:
	bool m_FrameStarted;
	typedef Common::Array<BS_RenderObjectPtr<BS_TimedRenderObject> > RenderObjectList;
	RenderObjectList m_TimedRenderObjects;

	// RenderObject-Tree Variablen
	// ---------------------------
	// Der Baum legt die hierachische Ordnung der BS_RenderObjects fest.
	// Zu weiteren Informationen siehe: "renderobject.h"
	BS_RenderObjectPtr<BS_RenderObject>     m_RootPtr;      // Die Wurzel der Baumes
};

} // End of namespace Sword25

#endif
