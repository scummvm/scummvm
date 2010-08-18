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
    BS_RenderObject
    ---------------
    Dieses ist die Klasse die sämtliche sichtbaren Objekte beschreibt. Alle anderen sichtbaren Objekte müssen von ihr abgeleitet werden.
    Diese Klasse erledigt Aufgaben wie: minimales Neuzeichnen, Renderreihenfolge, Objekthierachie.
    Alle BS_RenderObject Instanzen werden von einem BS_RenderObjectManager in einem Baum verwaltet.

    Autor: Malte Thiesen
*/

#ifndef SWORD25_RENDEROBJECT_H
#define SWORD25_RENDEROBJECT_H

// Includes
#include "sword25/kernel/common.h"
#include "sword25/kernel/persistable.h"
#include "sword25/math/rect.h"
#include "sword25/gfx/renderobjectptr.h"

#include "common/list.h"

namespace Sword25 {

// -----------------------------------------------------------------------------
// Forward Declarations
// -----------------------------------------------------------------------------

class BS_Kernel;
class RenderObjectManager;
class Bitmap;
class Animation;
class AnimationTemplate;
class Panel;
class Text;

// Klassendefinition
/**
    @brief  Dieses ist die Klasse die sämtliche sichtbaren Objekte beschreibt.

    Alle anderen sichtbaren Objekte müssen von ihr abgeleitet werden.
    Diese Klasse erledigt Aufgaben wie: minimales Neuzeichnen, Renderreihenfolge, Objekthierachie.
    Alle BS_RenderObject Instanzen werden von einem BS_RenderObjektManager in einem Baum verwaltet.
 */
class RenderObject {
public:
	// Konstanten
	// ----------
	enum TYPES {
		/// Das Wurzelobjekt. Siehe BS_RenderObjectManager
		TYPE_ROOT,
		/// Ein Image. Siehe BS_Bitmap.
		TYPE_STATICBITMAP,
		TYPE_DYNAMICBITMAP,
		/// Eine Animation. Siehe BS_Animation.
		TYPE_ANIMATION,
		/// Eine farbige Fläche. Siehe BS_Panel.
		TYPE_PANEL,
		/// Ein Text. Siehe BS_Text.
		TYPE_TEXT,
		/// Ein unbekannter Objekttyp. Diesen Typ sollte kein Renderobjekt annehmen.
		TYPE_UNKNOWN
	};

	// Add-Methoden
	// ------------

	/**
	    @brief Erzeugt ein Bitmap als Kinderobjekt des Renderobjektes.
	    @param FileName der Dateiname der Quellbilddatei
	    @return Gibt einen BS_RenderObjectPtr auf das erzeugte Objekt zurück.<br>
	            Falls ein Fehler aufgetreten ist wird ein ungültiger BS_RenderObjectPtr zurückgegeben.
	*/
	RenderObjectPtr<Bitmap> AddBitmap(const Common::String &FileName);
	/**
	    @brief Erzeugt ein veränderbares Bitmap als Kinderobjekt des Renderobjektes.
	    @param Width die Breite des Bitmaps
	    @param Height die Höhe des Bitmaps
	    @return Gibt einen BS_RenderObjectPtr auf das erzeugte Objekt zurück.<br>
	            Falls ein Fehler aufgetreten ist wird ein ungültiger BS_RenderObjectPtr zurückgegeben.
	*/
	RenderObjectPtr<Bitmap> AddDynamicBitmap(unsigned int Width, unsigned int Height);
	/**
	    @brief Erzeugt eine Animation auf Basis einer Animationsdatei als Kinderobjekt des Renderobjektes.
	    @param FileName der Dateiname der Quelldatei
	    @return Gibt einen BS_RenderObjectPtr auf das erzeugte Objekt zurück.<br>
	            Falls ein Fehler aufgetreten ist wird ein ungültiger BS_RenderObjectPtr zurückgegeben.
	*/
	RenderObjectPtr<Animation> AddAnimation(const Common::String &FileName);
	/**
	    @brief Erzeugt eine Animation auf Basis eines Animationstemplate als Kinderobjekt des Renderobjektes.
	    @param pAnimationTemplate ein Pointer auf das Animationstemplate
	    @return Gibt einen BS_RenderObjectPtr auf das erzeugte Objekt zurück.<br>
	            Falls ein Fehler aufgetreten ist wird ein ungültiger BS_RenderObjectPtr zurückgegeben.
	    @remark Das Renderobjekt übernimmt die Verwaltung des Animationstemplate.
	*/
	RenderObjectPtr<Animation> AddAnimation(const AnimationTemplate &AnimationTemplate);
	/**
	    @brief Erzeugt ein neues Farbpanel als Kinderobjekt des Renderobjektes.
	    @param Width die Breite des Panels
	    @param Height die Höhe des Panels
	    @param Color die Farbe des Panels.<br>
	           Der Standardwert ist Schwarz (BS_RGB(0, 0, 0)).
	    @return Gibt einen BS_RenderObjectPtr auf das erzeugte Objekt zurück.<br>
	            Falls ein Fehler aufgetreten ist wird ein ungültiger BS_RenderObjectPtr zurückgegeben.
	*/

	RenderObjectPtr<Panel> AddPanel(int Width, int Height, unsigned int Color = 0xff000000);
	/**
	    @brief Erzeugt ein Textobjekt als Kinderobjekt des Renderobjektes.
	    @param Font der Dateiname des zu verwendenen Fonts
	    @param Text der anzuzeigende Text.<br>
	                Der Standardwert ist "".
	    @return Gibt einen BS_RenderObjectPtr auf das erzeugte Objekt zurück.<br>
	            Falls ein Fehler aufgetreten ist wird ein ungültiger BS_RenderObjectPtr zurückgegeben.
	*/
	RenderObjectPtr<Text> AddText(const Common::String &Font, const Common::String &text = "");

	// Cast-Methoden
	// -------------
	/**
	    @brief Castet das Objekt zu einem BS_Bitmap-Objekt wenn zulässig.
	    @return Gibt einen BS_RenderObjectPtr auf das Objekt zurück.<br>
	            Falls der Cast nicht zulässig ist, wird ein ungültiger BS_RenderObjectPtr zurückgegeben.
	*/
	RenderObjectPtr<Bitmap> ToBitmap() {
		if (m_Type == TYPE_STATICBITMAP || m_Type == TYPE_DYNAMICBITMAP) return RenderObjectPtr<Bitmap>(this->GetHandle());
		else return RenderObjectPtr<Bitmap>();
	}
	/**
	    @brief Castet das Objekt zu einem BS_Animation-Objekt wenn zulässig.
	    @return Gibt einen BS_RenderObjectPtr auf das Objekt zurück.<br>
	            Falls der Cast nicht zulässig ist, wird ein ungültiger BS_RenderObjectPtr zurückgegeben.
	*/
	RenderObjectPtr<Animation> ToAnimation() {
		if (m_Type == TYPE_ANIMATION) return RenderObjectPtr<Animation>(this->GetHandle());
		else return RenderObjectPtr<Animation>();
	}
	/**
	    @brief Castet das Objekt zu einem BS_Panel-Objekt wenn zulässig.
	    @return Gibt einen BS_RenderObjectPtr auf das Objekt zurück.<br>
	            Falls der Cast nicht zulässig ist, wird ein ungültiger BS_RenderObjectPtr zurückgegeben.
	*/
	RenderObjectPtr<Panel> ToPanel() {
		if (m_Type == TYPE_PANEL) return RenderObjectPtr<Panel>(this->GetHandle());
		else return RenderObjectPtr<Panel>();
	}
	/**
	    @brief Castet das Object zu einem BS_Text-Objekt wenn zulässig.
	    @return Gibt einen BS_RenderObjectPtr auf das Objekt zurück.<br>
	            Falls der Cast nicht zulässig ist, wird ein ungültiger BS_RenderObjectPtr zurückgegeben.
	*/
	RenderObjectPtr<Text> ToText() {
		if (m_Type == TYPE_TEXT) return RenderObjectPtr<Text>(this->GetHandle());
		else return RenderObjectPtr<Text>();
	}

	// Konstruktor / Desktruktor
	// -------------------------
	/**
	    @brief Erzeugt ein neues BS_RenderObject.
	    @param pKernel ein Pointer auf den Kernel
	    @param pParent ein Pointer auf das Elternobjekt des neuen Objektes im Objektbaum.<br>
	                   Der Pointer darf nicht NULL sein.
	    @param Type der Objekttyp<br>
	                Der Typ BS_RenderObject::TYPE_ROOT ist nicht zulässig. Wurzelknoten müssen mit dem alternativen Konstruktor erzeugt
	                werden.
	    @param Handle das Handle, welches dem Objekt zugewiesen werden soll.<br>
	                Dieser Parameter erzwingt ein bestimmtes Handle für das neue Objekt, oder wählt automatisch ein Handle, wenn der Parameter 0 ist.
	                Ist das gewünschte Handle bereits vergeben, gibt GetInitSuccess() false zurück.<br>
	                Der Standardwert ist 0.
	    @remark Nach dem Aufruf des Konstruktors kann über die Methode GetInitSuccess() abgefragt werden, ob die Konstruktion erfolgreich war.<br>
	            Es ist nicht notwendig alle BS_RenderObject Instanzen einzeln zu löschen. Dieses geschiet automatisch beim Löschen eines
	            Vorfahren oder beim Löschen des zuständigen BS_RenderObjectManager.
	 */
	RenderObject(RenderObjectPtr<RenderObject> pParent, TYPES Type, unsigned int Handle = 0);
	virtual ~RenderObject();

	// Interface
	// ---------
	/**
	    @brief Rendert des Objekt und alle seine Unterobjekte.
	    @return Gibt false zurück, falls beim Rendern ein Fehler aufgetreten ist.
	    @remark Vor jedem Aufruf dieser Methode muss ein Aufruf von UpdateObjectState() erfolgt sein.
	            Dieses kann entweder direkt geschehen oder durch den Aufruf von UpdateObjectState() an einem Vorfahren-Objekt.<br>
	            Diese Methode darf nur von BS_RenderObjectManager aufgerufen werden.
	*/
	bool                Render();
	/**
	    @brief Bereitet das Objekt und alle seine Unterobjekte auf einen Rendervorgang vor.
	           Hierbei werden alle Dirty-Rectangles berechnet und die Renderreihenfolge aktualisiert.
	    @return Gibt false zurück, falls ein Fehler aufgetreten ist.
	    @remark Diese Methode darf nur von BS_RenderObjectManager aufgerufen werden.
	 */
	bool                UpdateObjectState();
	/**
	    @brief Löscht alle Kinderobjekte.
	*/
	void DeleteAllChildren();

	// Accessor-Methoden
	// -----------------
	/**
	    @brief Setzt die Position des Objektes.
	    @param X die neue X-Koordinate des Objektes relativ zum Elternobjekt.
	    @param Y die neue Y-Koordinate des Objektes relativ zum Elternobjekt.
	 */
	virtual void SetPos(int X, int Y);
	/**
	    @brief Setzt die Position des Objektes auf der X-Achse.
	    @param X die neue X-Koordinate des Objektes relativ zum Elternobjekt.
	 */
	virtual void SetX(int X);
	/**
	    @brief Setzt die Position des Objektes auf der Y-Achse.
	    @param Y die neue Y-Koordinate des Objektes relativ zum Elternobjekt.
	 */
	virtual void SetY(int Y);
	/**
	    @brief Setzt den Z-Wert des Objektes.
	    @param Z der neue Z-Wert des Objektes relativ zum Elternobjekt<br>
	             Negative Z-Werte sind nicht zulässig.
	    @remark Der Z-Wert legt die Renderreihenfolge der Objekte fest. Objekte mit niedrigem Z-Wert werden vor Objekten mit höherem
	            Z-Wert gezeichnet. Je höher der Z-Wert desto weiter "vorne" liegt ein Objekt also.<br>
	            Wie alle andere Attribute ist auch dieses relativ zum Elternobjekt, ein Kinderobjekt kann also nie unter seinem
	            Elternobjekt liegen, auch wenn es einen Z-Wert von 0 hat.
	 */
	virtual void SetZ(int Z);
	/**
	    @brief Setzt die Sichtbarkeit eine Objektes.
	    @param Visible der neue Sichtbarkeits-Zustand des Objektes<br>
	                   true entspricht sichtbar, false entspricht unsichtbar.
	 */
	virtual void SetVisible(bool Visible);
	/**
	    @brief Gibt die Position des Objektes auf der X-Achse relativ zum Elternobjekt zurück.
	 */
	virtual int GetX() const {
		return m_X;
	}
	/**
	    @brief Gibt die Position des Objektes auf der Y-Achse relativ zum Elternobjekt zurück.
	 */
	virtual int GetY() const {
		return m_Y;
	}
	/**
	    @brief Gibt die absolute Position des Objektes auf der X-Achse zurück.
	*/
	virtual int GetAbsoluteX() const {
		return m_AbsoluteX;
	}
	/**
	    @brief Gibt die absolute Position des Objektes auf der Y-Achse zurück.
	*/
	virtual int GetAbsoluteY() const {
		return m_AbsoluteY;
	}
	/**
	    @brief Gibt den Z-Wert des Objektes relativ zum Elternobjekt zurück.
	    @remark Der Z-Wert legt die Renderreihenfolge der Objekte fest. Objekte mit niedrigem Z-Wert werden vor Objekten mit höherem
	            Z-Wert gezeichnet. Je höher der Z-Wert desto weiter "vorne" liegt ein Objekt also.<br>
	            Wie alle andere Attribute ist auch dieses relativ zum Elternobjekt, ein Kinderobjekt kann also nie unter seinem
	            Elternobjekt liegen, auch wenn es einen Z-Wert von 0 hat.
	 */
	int         GetZ() const {
		return m_Z;
	}
	/**
	    @brief Gibt die Breite des Objektes zurück.
	 */
	int         GetWidth() const {
		return m_Width;
	}
	/**
	    @brief Gibt die Höhe des Objektes zurück.
	 */
	int         GetHeight() const {
		return m_Height;
	}
	/**
	    @brief Gibt den Sichtbarkeitszustand des Objektes zurück.
	    @return Gibt den Sichtbarkeitszustand des Objektes zurück.<br>
	            true entspricht sichtbar, false entspricht unsichtbar.
	 */
	bool        IsVisible() const {
		return m_Visible;
	}
	/**
	    @brief Gibt den Typ des Objektes zurück.
	 */
	TYPES       GetType() const {
		return m_Type;
	}
	/**
	    @brief Gibt zurück, ob das Objekt erfolgreich initialisiert wurde.
	    @remark Hässlicher Workaround um das Problem, dass Konstruktoren keine Rückgabewerte haben.
	 */
	bool        GetInitSuccess() const {
		return m_InitSuccess;
	}
	/**
	    @brief Gibt die Bounding-Box des Objektes zurück.
	    @remark Diese Angabe erfolgt ausnahmsweise in Bildschirmkoordianten und nicht relativ zum Elternobjekt.
	 */
	const BS_Rect  &GetBBox() const {
		return m_BBox;
	}
	/**
	    @brief Stellt sicher, dass das Objekt im nächsten Frame neu gezeichnet wird.
	*/
	void ForceRefresh() {
		m_RefreshForced = true;
	};
	/**
	    @brief Gibt das Handle des Objekte zurück.
	*/
	unsigned int GetHandle() const {
		return m_Handle;
	}

	// Persistenz-Methoden
	// -------------------
	virtual bool Persist(BS_OutputPersistenceBlock &Writer);
	virtual bool Unpersist(BS_InputPersistenceBlock &Reader);
	// TODO: Evtl. protected
	bool PersistChildren(BS_OutputPersistenceBlock &Writer);
	bool UnpersistChildren(BS_InputPersistenceBlock &Reader);
	// TODO: Evtl. private
	RenderObjectPtr<RenderObject> RecreatePersistedRenderObject(BS_InputPersistenceBlock &Reader);

protected:
	// Typen
	// -----
	typedef Common::List<RenderObjectPtr<RenderObject> >          RENDEROBJECT_LIST;
	typedef Common::List<RenderObjectPtr<RenderObject> >::iterator    RENDEROBJECT_ITER;

	int         m_X;            ///< Die X-Position des Objektes relativ zum Eltern-Objekt
	int         m_Y;            ///< Die Y-Position des Objektes relativ zum Eltern-Objekt
	int         m_AbsoluteX;    ///< Die absolute X-Position des Objektes
	int         m_AbsoluteY;    ///< Die absolute Y-Position des Objektes
	int         m_Z;            ///< Der Z-Wert des Objektes relativ zum Eltern-Objekt
	int         m_Width;        ///< Die Breite des Objektes
	int         m_Height;       ///< Die Höhe des Objektes
	bool        m_Visible;      ///< Ist true, wenn das Objekt sichtbar ist
	bool        m_ChildChanged; ///< Ist true, wenn sich ein Kinderobjekt verändert hat
	TYPES       m_Type;         ///< Der Objekttyp
	bool        m_InitSuccess;  ///< Ist true, wenn Objekt erfolgreich intialisiert werden konnte
	BS_Rect     m_BBox;         ///< Die Bounding-Box des Objektes in Bildschirmkoordinaten

	// Kopien der Variablen, die für die Errechnung des Dirty-Rects und zur Bestimmung der Objektveränderung notwendig sind
	BS_Rect     m_OldBBox;
	int         m_OldX;
	int         m_OldY;
	int         m_OldZ;
	bool        m_OldVisible;

	/// Ein Pointer auf den BS_RenderObjektManager, der das Objekt verwaltet.
	RenderObjectManager *m_ManagerPtr;

	// Render-Methode
	// --------------
	/**
	    @brief Einschubmethode, die den tatsächlichen Redervorgang durchführt.

	    Diese Methode wird von Render() aufgerufen um das Objekt darzustellen.
	    Diese Methode sollte von allen Klassen implementiert werden, die von BS_RederObject erben, um das Zeichnen umzusetzen.

	    @return Gibt false zurück, falls das Rendern fehlgeschlagen ist.
	    @remark
	 */
	virtual bool DoRender() = 0; // { return true; };

	// RenderObject-Baum Variablen
	// ---------------------------
	// Der Baum legt die hierachische Ordnung der BS_RenderObjects fest.
	// Alle Eigenschaften wie X, Y, Z und Visible eines BS_RenderObjects sind relativ zu denen seines Vaters.
	// Außerdem sind die Objekte von links nach rechts in ihrer Renderreihenfolge sortiert.
	// Das primäre Sortierkriterium ist hierbei der Z-Wert und das sekundäre der Y-Wert (von oben nach unten).
	// Beispiel:
	//                  Screen
	//                 /  |   \.
	//               /    |     \.
	//             /      |       \.
	//           /        |         \.
	//     Background Interface   Maus
	//      /   \      /  |   \.
	//    /      \    /   |    \.
	// George   Tür Icn1 Icn2 Icn3
	//
	// Wenn jetzt das Interface mit SetVisible() ausgeblendet würde, verschwinden auch die Icons, die sich im Interface
	// befinden.
	// Wenn der Hintergrund bewegt wird (Scrolling), bewegen sich auch die darauf befindlichen Gegenstände und Personen.

	/// Ein Pointer auf das Elternobjekt.
	RenderObjectPtr<RenderObject> m_ParentPtr;
	/// Die Liste der Kinderobjekte nach der Renderreihenfolge geordnet
	RENDEROBJECT_LIST           m_Children;

	/**
	    @brief Gibt einen Pointer auf den BS_RenderObjektManager zurück, der das Objekt verwaltet.
	 */
	RenderObjectManager *GetManager() const {
		return m_ManagerPtr;
	}
	/**
	    @brief Fügt dem Objekt ein neues Kinderobjekt hinzu.
	    @param pObject ein Pointer auf das einzufügende Objekt
	    @return Gibt false zurück, falls das Objekt nicht eingefügt werden konnte.
	*/
	bool AddObject(RenderObjectPtr<RenderObject> pObject);

private:
	/// Ist true, wenn das Objekt in nächsten Frame neu gezeichnet werden soll
	bool m_RefreshForced;

	unsigned int m_Handle;

	/**
	    @brief Entfernt ein Objekt aus der Kinderliste.
	    @param pObject ein Pointer auf das zu entfernende Objekt
	    @return Gibt false zurück, falls das zu entfernende Objekt nicht in der Liste gefunden werden konnte.
	 */
	bool    DetatchChildren(RenderObjectPtr<RenderObject> pObject);
	/**
	    @brief Berechnet die Bounding-Box und registriert das Dirty-Rect beim BS_RenderObjectManager.
	 */
	void    UpdateBoxes();
	/**
	    @brief Berechnet die Bounding-Box des Objektes.
	    @return Gibt die Bounding-Box des Objektes in Bildschirmkoordinaten zurück.
	*/
	BS_Rect CalcBoundingBox() const;
	/**
	    @brief Berechnet das Dirty-Rectangle des Objektes.
	    @return Gibt das Dirty-Rectangle des Objektes in Bildschirmkoordinaten zurück.
	*/
	BS_Rect CalcDirtyRect() const;
	/**
	    @brief Berechnet die absolute Position des Objektes.
	*/
	void CalcAbsolutePos(int &X, int &Y) const;
	/**
	    @brief Berechnet die absolute Position des Objektes auf der X-Achse.
	*/
	int CalcAbsoluteX() const;
	/**
	    @brief Berechnet die absolute Position des Objektes.
	*/
	int CalcAbsoluteY() const;
	/**
	    @brief Sortiert alle Kinderobjekte nach ihrem Renderang.
	 */
	void SortRenderObjects();
	/**
	    @brief Validiert den Zustand eines Objektes nachdem die durch die Veränderung verursachten Folgen abgearbeitet wurden.
	 */
	void    ValidateObject();
	/**
	    @brief Berechnet die absolute Position des Objektes und aller seiner Kinderobjekte neu.

	    Diese Methode muss aufgerufen werden, wann immer sich die Position des Objektes verändert. Damit die Kinderobjekte immer die
	    richtige absolute Position haben.
	*/
	void UpdateAbsolutePos();
	/**
	    @brief Teilt dem Objekt mit, dass sich eines seiner Kinderobjekte dahingehend verändert hat, die eine erneute Bestimmung der
	           Rendereihenfolge verlangt.
	*/
	void    SignalChildChange() {
		m_ChildChanged = true;
	}
	/**
	    @brief Berechnet des Schnittrechteck der Bounding-Box des Objektes mit einem anderen Objekt.
	    @param pObjekt ein Pointer auf das Objekt mit dem geschnitten werden soll
	    @param Result das Ergebnisrechteck
	    @return Gibt false zurück, falls sich die Objekte gar nicht schneiden.
	 */
	bool    GetObjectIntersection(RenderObjectPtr<RenderObject> pObject, BS_Rect &Result);
	/**
	    @brief Vergleichsoperator der auf Objektpointern basiert statt auf Objekten.
	    @remark Diese Methode wird fürs Sortieren der Kinderliste nach der Rendereihenfolge benutzt.
	*/
	static bool Greater(const RenderObjectPtr<RenderObject> lhs, const RenderObjectPtr<RenderObject> rhs);
};

} // End of namespace Sword25

#endif
