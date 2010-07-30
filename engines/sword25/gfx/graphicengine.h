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

/*
	BS_GraphicEngine
	----------------
	Dies ist das Graphik-Engine Interface, dass alle Methoden und Klassen enthält, die eine 
	Graphik-Engine implementieren muss.

	Autor: Malte Thiesen
*/

#ifndef SWORD25_GRAPHICENGINE_H
#define SWORD25_GRAPHICENGINE_H

// Includes
#include "sword25/kernel/memlog_off.h"
#include <vector>
#include "sword25/kernel/memlog_on.h"

#include "sword25/kernel/common.h"
#include "sword25/kernel/bs_stdint.h"
#include "sword25/kernel/resservice.h"
#include "sword25/kernel/persistable.h"
#include "sword25/math/rect.h"
#include "sword25/gfx/framecounter.h"
#include "sword25/gfx/renderobjectptr.h"

class BS_Kernel;
class BS_Image;
class BS_Panel;
class BS_Screenshot;

// Typen
typedef unsigned int BS_COLOR;

// Makros
#define BS_RGB(R,G,B)		(0xFF000000 | ((R) << 16) | ((G) << 8) | (B))
#define BS_ARGB(A,R,G,B)	(((A) << 24) | ((R) << 16) | ((G) << 8) | (B))

/**
	@brief Dies ist das Graphik-Engine Interface, dass alle Methoden und Klassen enthält, die eine Graphik-Engine implementieren muss.

	Hier sind nur wenige Rumpffunktionen realisiert, wie z.B. das Abfragen der Parameter des Ausgabepuffers.
	Die Hauptfunktionen muss eine Implementation dieses Inferfaces stellen.<br>
	Die bisher einzige Implementation ist BS_DDrawGfx.
*/

class BS_GraphicEngine : public BS_ResourceService, public BS_Persistable
{
public:
	// Enums
	// -----

	// Farbformate
	/**
		@brief Die von der Engine benutzten Farbformate
	*/
	enum COLOR_FORMATS
	{
		/// Undefiniertes/unbekanntes Farbformat
		CF_UNKNOWN = 0,
		/// 16 Bit Farbformat (5 Bit Rot, 5 Bit Grün, 5 Bit Blau)
		CF_RGB15,
		/// 16 Bit Farbformat (5 Bit Rot, 6 Bit Grün, 5 Bit Blau)
		CF_RGB16,
		/**
			Spezielles Alpha-Farbformat der Engine, welches besonders schnelles Darstellen unter Benutzung von MMX-Befehlen unterstützt.<br>
			Die Pixel sind 16 Bit breit und haben das selbe Format wie #CF_RGB15. Zusätzlich besitzt jeder Pixel noch einen 8 Bit Alphawert.<br>
			Es werden jeweils 4 Pixel und 4 Alphawerte zu einem 12 Byte großen Datenblock zusammengefasst.<br>
			Dabei werden die Daten in folgender Reihenfolge abgelegt:
			Alpha0 Alpha1 Alpha2 Alpha3 Pixel0 Pixel1 Pixel2 Pixel3
			Falls die Pixelanzahl einer Zeile nicht durch 4 teilbar ist, wird der letzte Pixelblock trotzdem komplett abgespeichert, die
			nicht verwendeten Pixel- und Alphawerte können beliebige Werte haben.
		*/
		CF_RGB15_INTERLEAVED,
		/**
			Spezielles Alpha-Farbformat der Engine, welches besonders schnelles Darstellen unter Benutzung von MMX-Befehlen unterstützt.<br>
			Die Pixel sind 16 Bit breit und haben das selbe Format wie #CF_RGB16. Zusätzlich besitzt jeder Pixel noch einen 8 Bit Alphawert.<br>
			Es werden jeweils 4 Pixel und 4 Alphawerte zu einem 12 Byte großen Datenblock zusammengefasst.<br>
			Dabei werden die Daten in folgender Reihenfolge abgelegt:
			Alpha0 Alpha1 Alpha2 Alpha3 Pixel0 Pixel1 Pixel2 Pixel3
			Falls die Pixelanzahl einer Zeile nicht durch 4 teilbar ist, wird der letzte Pixelblock trotzdem komplett abgespeichert, die
			nicht verwendeten Pixel- und Alphawerte können beliebige Werte haben.
		*/
		CF_RGB16_INTERLEAVED,
		/**
			24 Bit Farbformat (8 Bit Rot, 8 Bit Grün, 8 Bit Blau)
		*/
		CF_RGB24,
		/**
			32 Bit Farbformat (8 Bit Alpha, 8 Bit Rot, 8 Bit Grün, 8 Bit Blau) (little endian)
		*/
		CF_ARGB32,
		/**
			32 Bit Farbformat (8 Bit Alpha, 8 Bit Blau, 8 Bit Grün, 8 Bit Rot) (little endian)
		*/
		CF_ABGR32,
	};
	
	// Interface
	// ---------

	/**
		@brief Initialisiert die Graphikengine und setzt den Bildschirmmodus.
		@param Width die Breite des Ausgabepuffers in Pixeln.<br>Der Standardwert ist 800.
		@param Height die Höhe des Ausgabepuffers in Pixeln.<br>Der Standardwert ist 600.
		@param BitDepth die Bittiefe des gewünschten Ausgabepuffers in Bit.<br>Der Standardwert ist 16.
		@param BackbufferCount die Anzahl an Backbuffern die erzeugt werden soll.<br>Der Standardwert ist 2.
		@param Windowed gibt an, ob die Engine im Fenstermodus laufen soll. Falls true angegeben wird, wird ein Fenster geöffnet in das
						die Ausgaben gerendert werden. Ansonsten wird ein Vollbildmodus gesetzt der den Parametern das Ausgabepuffers
						entspricht.
		@return Gibt false zurück, falls die Initialisierung fehlgeschlagen ist.
		@remark Der Fenstermodus sollte nur zu Debuggingzwecken benutzt werden und ist nicht dafür gedacht im endgültigen Produkt benutzt
				zu werden.
		@remark Diese Methode sollte direkt nach der Initialisierung aller Services aufgerufen werden.
	*/
	virtual bool		Init(int Width = 800, int Height = 600, int BitDepth = 16, int BackbufferCount = 2, bool Windowed = false) = 0;

	//@{
	/** @name Frame-Methoden */
	/**
		@brief Beginnt das Rendern eines neuen Frames.
		@param UpdateAll gibt an, ob der Renderer im nächsten Frame alles neu zeichnen soll.<br>
						 Diese Funktion kann nützlich sein, wenn der Renderer mit Dirty-Rectangles arbeitet, der Benutzer aber gelegentlich
						 darauf angewiesen ist, dass der gesamte Bildschirminhalt neu gezeichnet werden soll.<br>
						 Der Standardwert ist false.
		Diese Methode muss am Anfang das Main-Loops aufgerufen werden und vor dem Aufruf jeglicher Rendermethoden.
		@return Gibt false zurück, falls ein Fehler aufgetreten ist.
		@remark Implementationen dieser Methode müssen _UpdateLastFrameDuration() aufrufen.
	*/
	virtual bool		StartFrame(bool UpdateAll = false) = 0;

	/**
		@brief Beendet das Rendern des Frames und stellt diesen auf dem Bildschirm dar.
		
		Diese Methode muss am Ende des Main-Loops aufgerufen werden. Nach dem Aufruf dürfen keine weiteren Rendermethoden mehr aufgerufen
		werden. Dafür muss erst wieder ein Aufruf von #StartFrame erfolgen.
		@return Gibt false zurück, falls ein Fehler aufgetreten ist:
	*/
	virtual bool		EndFrame() = 0;

	//@}
	
	//@{
	/** @name Debug-Methoden */

	/**
		@brief Zeichnet eine Line in den Framebuffer.

		Diese Methode muss zwischen StartFrame() und EndFrame() aufgerufen werden und ist nur für Debugzwecke gedacht.
		Die Linie erscheint nur für einen Frame. Wenn die Linie dauerhaft zu sehen sein soll, muss sie jeden Frame neu
		gezeichnet werden.

		@param Start der Startpunkt der Linie
		@param End der Endpunkt der Linie
		@param Color die Farbe der Linie. Der Standardwert ist BS_RGB(255, 255, 255) (Weiß).
	*/
	virtual void		DrawDebugLine(const BS_Vertex & Start, const BS_Vertex & End, unsigned int Color = BS_RGB(255, 255, 255)) = 0;

	/**
	    @brief Erstellt einen Screenshot.

		Erstellt einen Screenshot vom aktuellen Framebuffer und schreibt ihn in eine Grafikdatei.<br>
		Das verwendete Dateiformat ist PNG.

		@param Der Dateiname des Screenshots.
		@return Gibt true zurück, wenn der Screenshot gespeichert werden konnte, ansonsten false.
		@remark Diese Methode darf erst nach einem Aufruf von EndFrame() und vor dem nächsten Aufruf von StartFrame() aufgerufen werden.
	*/
	bool SaveScreenshot(const std::string & Filename);

	/**
		@Brief Erstellt einen kleinen Screenshot.

		Erstellt einen Screenshot mit den Maßen 200x125. Hierfür werden am oberen und unteren Bildschirmrand die Interfaceleisten abgeschnitten und
		das Bild auf ein 16tel seiner Ursprungsgröße verkleinert.

		@param Der Dateiname des Screenshots.
		@return Gibt true zurück, wenn der Screenshot gespeichert werden konnte, ansonsten false.
		@remark Diese Methode darf erst nach einem Aufruf von EndFrame() und vor dem nächsten Aufruf von StartFrame() aufgerufen werden.
		@remark Der Framebuffer muss eine Auflösung von 800x600 haben.
	*/
	bool SaveThumbnailScreenshot(const std::string & Filename);

	/**
		@brief Liest den aktuellen Inhalt des Framebuffer aus.
		@param Width enthält nach einem erfolgreichen Aufruf die Breite des Framebuffers.
		@param Height enthält nach einem erfolgreichen Aufruf die Höhe des Framebuffers.
		@param Data enthält nach einem erfolgreichen Aufruf den Inhalt des Framebuffers als 32-Bit Farbwerte.
		@return Gibt true zurück, wenn der Aufruf erfolgreich war, ansonsten false.
		@remark Diese Methode ist für das Erstellen von Screenshots gedacht. Sie muss also nicht sehr effizient sein.
		@remark Diese Methode darf erst nach einem Aufruf von EndFrame() und vor dem nächsten Aufruf von StartFrame() aufgerufen werden.
	*/
	virtual bool GetScreenshot(unsigned int & Width, unsigned int & Height, std::vector<unsigned int> & Data) = 0;
	
	//@}

	virtual BS_RenderObjectPtr<BS_Panel> GetMainPanel() = 0;

	/**
		@brief Gibt die Zeit (in Microsekunden) zurück die seit dem letzten Frame vergangen ist.
	*/
	int GetLastFrameDurationMicro() { if (m_TimerActive) return m_LastFrameDuration; else return 0; }

	/**
		@brief Gibt die Zeit (in Sekunden) zurück die seit dem letzten Frame vergangen ist.
	*/
	float GetLastFrameDuration() { if (m_TimerActive) return static_cast<float>(m_LastFrameDuration) / 1000000.0f; else return 0; }

	void StopMainTimer() { m_TimerActive = false; }
	void ResumeMainTimer() { m_TimerActive = true; }
	float GetSecondaryFrameDuration() { return static_cast<float>(m_LastFrameDuration) / 1000000.0f; }

	//@{
	/** @name Accessor-Methoden */

	/**
		@brief Gibt die Breite des Ausgabepuffers in Pixeln zurück.
	*/
	int			GetDisplayWidth() { return m_Width; }

	/**
		@brief Gibt die Höhe des Ausgabepuffers in Pixeln zurück.
	*/
	int			GetDisplayHeight() { return m_Height; }

	/**
		@brief Gibt die Bounding-Box des Ausgabepuffers zurück. (0, 0, Width, Height)
	*/
	BS_Rect&	GetDisplayRect() { return m_ScreenRect; }

	/**
		@brief Gibt die Bittiefe des Ausgabepuffers zurück.
	*/
	int			GetBitDepth() { return m_BitDepth; }

	/**
		@brief Legt fest ob der Framebufferwechsel mit dem vertikalen Strahlenrücklauf synchronisiert werden soll.<br>
			   Vsync ist standardmäßig eingeschaltet.
		@param Vsync gibt an, ob der Framebufferwechsel mit dem vertikalen Strahlenrücklauf synchronisiert werden soll.
		@remark Im Fenstermodus hat diese Einstellung keine Auswirkung.
	*/
	virtual void	SetVsync(bool Vsync) = 0;

	/**
		@brief Gibt true zurück, wenn V-Sync an ist.
		@remark Im Fenstermodus hat diese Einstellung keine Auswirkung.
	*/
	virtual bool	GetVsync() const = 0;

	/**
		@brief Gibt true zurück, falls die Engine im Fenstermodus läuft.
	*/
	bool	IsWindowed() { return m_Windowed; }

	/**
		@brief Füllt einen Rechteckigen Bereich des Framebuffers mit einer Farbe.
		@param FillRectPtr Pointer auf ein BS_Rect, welches den Ausschnitt des Framebuffers spezifiziert, der gefüllt
						   werden soll oder 0, falls das gesamte Bild gefüllt werden soll.<br>
						   Der Standardwert ist 0.
		@param Color der 32 Bit Farbwert mit dem der Bildbereich gefüllt werden soll.<br>
			   Der Standardwert ist BS_RGB(0, 0, 0) (Schwarz).
		@return Gibt true zurück, wenn der Aufruf erfolgreich war, ansonsten false.
		@remark Es ist möglich über die Methode transparente Rechtecke darzustellen, indem man eine Farbe mit einem 
				Alphawert ungleich 255 angibt.
		@remark Falls das Rechteck nicht völlig innerhalb des Bildschirms ist, wird es automatisch zurechtgestutzt.
	*/
	virtual bool Fill(const BS_Rect * FillRectPtr = 0, unsigned int Color = BS_RGB(0, 0, 0)) = 0;

	//@}

	//@{
	/** @name Debugging-Methoden */

	int	GetFPSCount() const { return m_FPSCounter.GetFPS(); }
	int GetRepaintedPixels() const { return m_RepaintedPixels; }

	//@}

	//@{
	/** @name Auskunfts-Methoden */

	/**
		@brief Gibt die Größe eines Pixeleintrages in Byte für ein bestimmtes Farbformat zurück
		@param ColorFormat das gewünschte Farbformat. Der Parameter muss vom Typ COLOR_FORMATS sein.
		@return Gibt die Größe eines Pixeleintrages in Byte des Farbsformates ColorFormat zurück.<br>
				Falls das Farbformat unbekannt ist, wird -1 zurückgegeben.
	*/
	static int GetPixelSize(BS_GraphicEngine::COLOR_FORMATS ColorFormat)
	{
		switch (ColorFormat)
		{
		case BS_GraphicEngine::CF_RGB16:
		case BS_GraphicEngine::CF_RGB15:
			return 2;

		case BS_GraphicEngine::CF_RGB16_INTERLEAVED:
		case BS_GraphicEngine::CF_RGB15_INTERLEAVED:
			return 3;

		case BS_GraphicEngine::CF_ARGB32:
			return 4;
		}

		return -1;
	}
	
	/**
		@brief Berechnet die Länge einer Bildzeile eines Bilder in Byte, abhängig vom Farbformat.
		@param ColorFormat das Farbformat des Bildes.
		@param Width die Länge einer Bildzeile in Pixel.
		@return Gibt die Länge einer Bildzeile in Byte wieder.<br>
				Falls das Farbformat unbekannt ist, wird -1 zurückgegeben.
	*/
	static int CalcPitch(BS_GraphicEngine::COLOR_FORMATS ColorFormat, int Width)
	{
		switch (ColorFormat)
		{
		case BS_GraphicEngine::CF_RGB16:
		case BS_GraphicEngine::CF_RGB15:
			return Width * 2;

		case BS_GraphicEngine::CF_RGB16_INTERLEAVED:
		case BS_GraphicEngine::CF_RGB15_INTERLEAVED:
			return (Width + 3) / 4 * 12;

		case BS_GraphicEngine::CF_ARGB32:
		case BS_GraphicEngine::CF_ABGR32:
			return Width * 4;

		default:
			BS_ASSERT(false);
		}

		return -1;
	}

	//@}
	
	// Persistenz Methoden
	// -------------------
	virtual bool Persist(BS_OutputPersistenceBlock & Writer);
	virtual bool Unpersist(BS_InputPersistenceBlock & Reader);

	static void ARGBColorToLuaColor(lua_State * L, unsigned int Color);
	static unsigned int LuaColorToARGBColor(lua_State * L, int StackIndex);

protected:
	// Konstruktor
	// -----------
	BS_GraphicEngine(BS_Kernel* pKernel);

	// Display Variablen
	// -----------------
	int		m_Width;
	int		m_Height;
	BS_Rect m_ScreenRect;
	int		m_BitDepth;
	bool	m_Windowed;

	// Debugging-Variablen
	// -------------------
	BS_Framecounter m_FPSCounter;

	unsigned int	m_RepaintedPixels;

	/**
		@brief Berechnet die Zeit die seit dem letzten Framebeginn vergangen ist.
	*/
	void UpdateLastFrameDuration();

private:
	bool RegisterScriptBindings();

	// LastFrameDuration-Variablen
	// ---------------------------
	uint64_t					m_LastTimeStamp;
	unsigned int				m_LastFrameDuration;
	bool						m_TimerActive;
	std::vector<unsigned int>	m_FrameTimeSamples;
	unsigned int				m_FrameTimeSampleSlot;
};

#endif
