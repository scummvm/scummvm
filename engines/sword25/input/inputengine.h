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

/**
	BS_InputEngine
	-------------
	Dies ist das Inputengine Interface, dass alle Methoden enthält, die eine Inputengine implementieren muss.
	Implementationen der Inputengine müssen von dieser Klasse abgeleitet werden.

	Autor: Alex Arnst
**/

#ifndef SWORD25_INPUTENGINE_H
#define SWORD25_INPUTENGINE_H

/// Includes
#include "sword25/kernel/common.h"
#include "sword25/kernel/service.h"
#include "sword25/kernel/persistable.h"

/// Klassendefinition
class BS_InputEngine : public BS_Service, public BS_Persistable
{
public:
	BS_InputEngine(BS_Kernel* pKernel);
	virtual ~BS_InputEngine(){};

	// ACHTUNG: Diese Codes werden in inputengine_script.cpp beim Skript-Service registriert. Bei Änderungen an diesem Enum muss auch diese
	// Datei angepasst werden.
	enum KEY_CODES
	{
		KEY_BACKSPACE	= 0x08,
		KEY_TAB			= 0x09,
		KEY_CLEAR		= 0x0C,
		KEY_RETURN      = 0x0D,
		KEY_PAUSE       = 0x13,
		KEY_CAPSLOCK    = 0x14,
		KEY_ESCAPE      = 0x1B,
		KEY_SPACE       = 0x20,
		KEY_PAGEUP      = 0x21,
		KEY_PAGEDOWN    = 0x22,
		KEY_END         = 0x23,
		KEY_HOME        = 0x24,
		KEY_LEFT        = 0x25,
		KEY_UP          = 0x26,
		KEY_RIGHT       = 0x27,
		KEY_DOWN        = 0x28,
		KEY_PRINTSCREEN = 0x2C,   
		KEY_INSERT      = 0x2D,
		KEY_DELETE      = 0x2E,
		KEY_0			= 0x30,
		KEY_1			= 0x31,
		KEY_2			= 0x32,
		KEY_3			= 0x33,
		KEY_4			= 0x34,
		KEY_5			= 0x35,
		KEY_6			= 0x36,
		KEY_7			= 0x37,
		KEY_8			= 0x38,
		KEY_9			= 0x39,
		KEY_A			= 0x41,
		KEY_B			= 0x42,
		KEY_C			= 0x43,
		KEY_D			= 0x44,
		KEY_E			= 0x45,
		KEY_F			= 0x46,
		KEY_G			= 0x47,
		KEY_H			= 0x48,
		KEY_I			= 0x49,
		KEY_J			= 0x4A,
		KEY_K			= 0x4B,
		KEY_L			= 0x4C,
		KEY_M			= 0x4D,
		KEY_N			= 0x4E,
		KEY_O			= 0x4F,
		KEY_P			= 0x50,
		KEY_Q			= 0x51,
		KEY_R			= 0x52,
		KEY_S			= 0x53,
		KEY_T			= 0x54,
		KEY_U			= 0x55,
		KEY_V			= 0x56,
		KEY_W			= 0x57,
		KEY_X			= 0x58,
		KEY_Y			= 0x59,
		KEY_Z			= 0x5A,
		KEY_NUMPAD0     = 0x60,
		KEY_NUMPAD1     = 0x61,
		KEY_NUMPAD2     = 0x62,
		KEY_NUMPAD3     = 0x63,
		KEY_NUMPAD4     = 0x64,
		KEY_NUMPAD5     = 0x65,
		KEY_NUMPAD6     = 0x66,
		KEY_NUMPAD7     = 0x67,
		KEY_NUMPAD8     = 0x68,
		KEY_NUMPAD9     = 0x69,
		KEY_MULTIPLY    = 0x6A,
		KEY_ADD         = 0x6B,
		KEY_SEPARATOR   = 0x6C,
		KEY_SUBTRACT    = 0x6D,
		KEY_DECIMAL     = 0x6E,
		KEY_DIVIDE      = 0x6F,
		KEY_F1          = 0x70,
		KEY_F2          = 0x71,
		KEY_F3          = 0x72,
		KEY_F4          = 0x73,
		KEY_F5          = 0x74,
		KEY_F6          = 0x75,
		KEY_F7          = 0x76,
		KEY_F8          = 0x77,
		KEY_F9          = 0x78,
		KEY_F10         = 0x79,
		KEY_F11         = 0x7A,
		KEY_F12         = 0x7B,
		KEY_NUMLOCK     = 0x90,
		KEY_SCROLL      = 0x91,
		KEY_LSHIFT      = 0xA0,
		KEY_RSHIFT      = 0xA1,
		KEY_LCONTROL    = 0xA2,
		KEY_RCONTROL    = 0xA3
	};

	// ACHTUNG: Diese Codes werden in inputengine_script.cpp beim Skript-Service registriert. Bei Änderungen an diesem Enum muss auch diese
	// Datei angepasst werden.
	enum KEY_COMMANDS
	{
		KEY_COMMAND_ENTER = 1,
		KEY_COMMAND_LEFT = 2,
		KEY_COMMAND_RIGHT = 3,
		KEY_COMMAND_HOME = 4,
		KEY_COMMAND_END = 5,
		KEY_COMMAND_BACKSPACE = 6,
		KEY_COMMAND_TAB = 7,
		KEY_COMMAND_INSERT = 8,
		KEY_COMMAND_DELETE = 9
	};

	/// --------------------------------------------------------------
	/// DIESE METHODEN MÜSSEN VON DER INPUTENGINE IMPLEMENTIERT WERDEN
	/// --------------------------------------------------------------
	
	/**
	    @brief Initialisiert die Inputengine
		@return Gibt bei Erfolg true zurück, ansonsten false.
	*/
	virtual bool Init() = 0;

	/**
		@brief Führt einen "Tick" der Input-Engine aus

		Diese Methode sollte mindestens ein mal pro Frame aufgerufen werden. Sie dient dazu Implementationen der
		Input-Engine zu ermöglichen, die nicht in einem eigenen Thread laufen oder zusätzliche Verwaltungsaufgaben
		durchführen müssen.
	*/
	virtual void Update() = 0;

	/**
	    @brief Gibt true zurück, wenn die linke Maustaste gedrückt ist.
	*/
	virtual bool IsLeftMouseDown() = 0;

	/**
	    @brief Gibt true zurück, wenn die rechte Maustaste gedrückt ist.
	*/
	virtual bool IsRightMouseDown() = 0;

	/**
	    @brief Gibt true zurück, wenn die linke Maustaste gedrückt und losgelassen wurde.

		Der Unterschied zu IsLeftMouseDown() besteht darin, dass erst true zurückgegegen wird, wenn der Tastendruck beendet ist, die Taste also
		wieder losgelassen wurde.
	*/
	virtual bool WasLeftMouseDown() = 0;

	/**
		@brief Gibt true zurück, wenn die linke Maustaste gedrückt und losgelassen wurde.

		Der Unterschied zu IsRightMouseDown() besteht darin, dass erst true zurückgegegen wird, wenn der Tastendruck beendet ist, die Taste also
		wieder losgelassen wurde.
	*/
	virtual bool WasRightMouseDown() = 0;

	/**
		@brief Gibt true zurück wenn mit der linken Maustaste ein Doppelklick ausgelöst wurde.
	*/
	virtual bool IsLeftDoubleClick() = 0;

	/**
	    @brief Gibt die Position des Mauszeigers auf der X-Achse in Pixeln zurück.
	*/
	virtual int GetMouseX() = 0;

	/**
		@brief Gibt die Position des Mauszeigers auf der Y-Achse in Pixeln zurück.
	*/
	virtual int GetMouseY() = 0;

	/**
		@brief Setzt die Position des Mauszeigers auf der X-Achse in Pixeln.
	*/
	virtual void SetMouseX(int PosX) = 0;

	/**
		@brief Setzt die Position des Mauszeigers auf der Y-Achse in Pixeln.
	*/
	virtual void SetMouseY(int PosY) = 0;

	/**
	    @brief Gibt true zurück wenn eine bestimmte Taste gedrückt ist.
		@param KeyCode der Key-Code der zu testenden Taste
		@return Gibt true zurück, wenn die Taste mit dem übergebenen Key-Code gedrückt ist, ansonsten false.
	*/
	virtual bool IsKeyDown(unsigned int KeyCode) = 0;

	/**
	    @brief Gibt true zurück wenn eine bestimmte Taste gerückt und losgelassen wurde.

		Der Unterschied zu IsKeyDown() besteht darin, dass erst true zurückgegegen wird, wenn der Tastendruck beendet ist, die Taste also
		wieder losgelassen wurde. Diese Methode erleichtert das Abfragen von Funktionstasten und das Einlesen von Zeichenketten, die vom
		Benutzer getippt werden.

		@param KeyCode der Key-Code der zu testenden Taste
	*/
	virtual bool WasKeyDown(unsigned int KeyCode) = 0;

	typedef void (*CharacterCallback)(unsigned char Character);

	/**
		@brief Registriert eine Callbackfunktion für die Eingabe von Buchstaben.

		Die Callbacks, die mit dieser Funktion registriert werden, werden immer dann aufgerufen, wenn der Input-Service eine Buchstabeneingabe
		feststellt. Eine Buchstabeneingabe unterscheidet sich von der Abfrage mittels der Methoden IsKeyDown() und WasKeyDown() in der Hinsicht,
		dass statt Scan-Coded tatsächliche Buchstaben behandelt werden. Dabei wurden unter anderem Berücksichtigt:des Tastaturlayout, der Zustand
		der Shift und Caps Lock Tasten und die Wiederholung durch längeres Halten der Taste.<br>
		Die Eingabe von Zeichenketten durch den Benutzer sollte durch Benutzung dieses Callbacks realisiert werden.

		@return Gibt true zurück, wenn die Funktion registriert werden konnte, ansonsten false.
	*/
	virtual bool RegisterCharacterCallback(CharacterCallback Callback) = 0;

	/**
		@brief Deregistriert eine Callbackfunktion für die Eingabe von Buchstaben.

		@return Gibt true zurück, wenn die Funktion deregistriert werden konnte, ansonsten false.
	*/
	virtual bool UnregisterCharacterCallback(CharacterCallback Callback) = 0;

	typedef void (*CommandCallback)(KEY_COMMANDS Command);

	/**
		@brief Registriert eine Callbackfunktion für die Eingabe von Kommandos, die auf die Zeichenketteneingabe Einfluss haben können.

		Die Callbacks, die mit dieser Funktion registriert werden , werden immer dann aufgerufen, wenn der Input-Service einen Tastendruck
		feststellt, der die Zeichenketteneingabe beeinflussen kann. Dies könnten folgende Tasten sein: Enter, Pos 1, Ende, Links, Rechts, ...<br>
		Die Eingabe von Zeichenketten durch den Benutzer sollte durch Benutzung dieses Callbacks realisiert werden.

		@return Gibt true zurück, wenn die Funktion registriert werden konnte, ansonsten false.
	*/
	virtual bool RegisterCommandCallback(CommandCallback Callback) = 0;

	/**
		@brief Deregistriert eine Callbackfunktion für die Eingabe von Kommandos, die auf die Zeichenketteneingabe Einfluss haben können.

		@return Gibt true zurück, wenn die Funktion deregistriert werden konnte, ansonsten false.		
	*/
	virtual bool UnregisterCommandCallback(CommandCallback Callback) = 0;

	virtual void ReportCharacter(unsigned char Character) = 0;
	virtual void ReportCommand(KEY_COMMANDS Command) = 0;

private:
	bool _RegisterScriptBindings();
};

#endif
