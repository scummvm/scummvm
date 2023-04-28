/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef BACKEND_EVENTS_SDL_H
#define BACKEND_EVENTS_SDL_H

#include "backends/platform/sdl/sdl-sys.h"
#include "backends/graphics/sdl/sdl-graphics.h"

#include "common/events.h"

// Type names which changed between SDL 1.2 and SDL 2.
#if !SDL_VERSION_ATLEAST(2, 0, 0)
typedef SDLKey     SDL_Keycode;
typedef SDLMod     SDL_Keymod;
typedef SDL_keysym SDL_Keysym;
#endif

/**
 * The SDL event source.
 */
class SdlEventSource : public Common::EventSource {
public:
	SdlEventSource();
	virtual ~SdlEventSource();

	void setGraphicsManager(SdlGraphicsManager *gMan) { _graphicsManager = gMan; }

	/**
	 * Gets and processes SDL events.
	 */
	virtual bool pollEvent(Common::Event &event);

	/**
	 * Emulates a mouse movement that would normally be caused by a mouse warp
	 * of the system mouse.
	 */
	void fakeWarpMouse(const int x, const int y);

	/** Returns whether a joystick is currently connected */
	bool isJoystickConnected() const;

	/** Sets whether a game is currently running */
	void setEngineRunning(bool value);

protected:
	/** Scroll lock state - since SDL doesn't track it */
	bool _scrollLock;

	bool _engineRunning;

	int _mouseX;
	int _mouseY;

	/** Joystick */
	SDL_Joystick *_joystick;

#if SDL_VERSION_ATLEAST(2, 0, 0)
	/** Game controller */
	SDL_GameController *_controller;
#endif

	/** Last screen id for checking if it was modified */
	int _lastScreenID;

	/**
	 * The associated graphics manager.
	 */
	SdlGraphicsManager *_graphicsManager;

	/**
	 * Search for a game controller db file and load it.
	 */
	void loadGameControllerMappingFile();

	/**
	 * Open the SDL joystick with the specified index
	 *
	 * After this function completes successfully, SDL sends events for the device.
	 *
	 * If the joystick is also a SDL game controller, open it as a controller
	 * so an extended button mapping can be used.
	 */
	void openJoystick(int joystickIndex);

	/**
	 * Close the currently open joystick if any
	 */
	void closeJoystick();

	/**
	 * Pre process an event before it is dispatched.
	 */
	virtual void preprocessEvents(SDL_Event *event);

	/**
	 * Dispatchs SDL events for each handler.
	 */
	virtual bool dispatchSDLEvent(SDL_Event &ev, Common::Event &event);


	/** @name Event Handlers
	 * Handlers for specific SDL events, called by SdlEventSource::dispatchSDLEvent().
	 * This way, if a managers inherits fromt this SDL events manager, it can
	 * change the behavior of only a single event, without having to override all
	 * of SdlEventSource::dispatchSDLEvent().
	 */
	//@{

	virtual bool handleKeyDown(SDL_Event &ev, Common::Event &event);
	virtual bool handleKeyUp(SDL_Event &ev, Common::Event &event);
	virtual bool handleMouseMotion(SDL_Event &ev, Common::Event &event);
	virtual bool handleMouseButtonDown(SDL_Event &ev, Common::Event &event);
	virtual bool handleMouseButtonUp(SDL_Event &ev, Common::Event &event);
	virtual bool handleSysWMEvent(SDL_Event &ev, Common::Event &event);
	virtual int mapSDLJoystickButtonToOSystem(Uint8 sdlButton);
	virtual bool handleJoyButtonDown(SDL_Event &ev, Common::Event &event);
	virtual bool handleJoyButtonUp(SDL_Event &ev, Common::Event &event);
	virtual bool handleJoyAxisMotion(SDL_Event &ev, Common::Event &event);
	virtual bool handleJoyHatMotion(SDL_Event &ev, Common::Event &event);

#if SDL_VERSION_ATLEAST(2, 0, 0)
	virtual bool handleJoystickAdded(const SDL_JoyDeviceEvent &device, Common::Event &event);
	virtual bool handleJoystickRemoved(const SDL_JoyDeviceEvent &device, Common::Event &event);
	virtual int mapSDLControllerButtonToOSystem(Uint8 sdlButton);
	virtual bool handleControllerButton(const SDL_Event &ev, Common::Event &event, bool buttonUp);
	virtual bool handleControllerAxisMotion(const SDL_Event &ev, Common::Event &event);
#endif

	//@}

	/**
	 * Assigns the mouse coords to the mouse event. Furthermore notify the
	 * graphics manager about the position change.
	 * The parameters relx and rely for relative mouse movement
	 */
	virtual bool processMouseEvent(Common::Event &event, int x, int y, int relx = 0, int rely = 0);

	/**
	 * Remaps key events. This allows platforms to configure
	 * their custom keys.
	 */
	virtual bool remapKey(SDL_Event &ev, Common::Event &event);

	/**
	 * Maps the ASCII value of key
	 */
	virtual int mapKey(SDL_Keycode key, SDL_Keymod mod, Uint16 unicode);

	/**
	 * Configures the key modifiers flags status
	 */
	virtual void SDLModToOSystemKeyFlags(SDL_Keymod mod, Common::Event &event);

	/**
	 * Translates SDL key codes to OSystem key codes
	 */
	Common::KeyCode SDLToOSystemKeycode(const SDL_Keycode key);

	/**
	 * Notify graphics manager of a resize request.
	 */
	bool handleResizeEvent(Common::Event &event, int w, int h);

	/**
	 * Extracts unicode information for the specific key sym.
	 * May only be used for key down events.
	 */
	uint32 obtainUnicode(const SDL_Keysym keySym);

	/**
	 * Extracts the keycode for the specified key sym.
	 */
	SDL_Keycode obtainKeycode(const SDL_Keysym keySym);

	/**
	 * Whether _fakeMouseMove contains an event we need to send.
	 */
	bool _queuedFakeMouseMove;

	/**
	 * A fake mouse motion event sent when the graphics manager is told to warp
	 * the mouse but the system mouse is unable to be warped (e.g. because the
	 * window is not focused).
	 */
	Common::Event _fakeMouseMove;

	uint8 _lastHatPosition;

#if SDL_VERSION_ATLEAST(2, 0, 0)
	/**
	 * Whether _fakeKeyUp contains an event we need to send.
	 */
	bool _queuedFakeKeyUp;

	/**
	 * A fake key up event when we receive a TEXTINPUT without any previous
	 * KEYDOWN event.
	 */
	Common::Event _fakeKeyUp;
#endif

#if defined(USE_SDL_TS_VMOUSE)
private:
    /// Perform initialization of touch-related state structures.
    void _touchInitialize();
    /**
     * Run pre-processing on touch-based events, otherwise null operation.
     *
     * @param event Input event to process.
     */
    void _touchPreprocessEvent(SDL_Event *event);

    /// Preprocessing specific to SDL_FINGERDOWN.
    void _touchPreprocessFingerDown(SDL_Event *event);
    /// Preprocessing specific to SDL_FINGERUP.
    void _touchPreprocessFingerUp(SDL_Event *event);
    /// Preprocessing specific to SDL_FINGERMOTION.
    void _touchPreprocessFingerMotion(SDL_Event *event);
    /// Convert panel normalized coordinates to window pixel coordinates.
    void _touchConvertPanelToWindow(
        float panelX, float panelY, 
        int32_t &winX, int32_t &winY);
    /// Run periodic processing updates - e.g. click finalization.
    void _touchPeriodicUpdate();

    /// Structure representing a single tracked touch point.
    struct Touch {
        /// Identifier of the touch point. -1 for no touch.
        int32_t id;
        /// Current pixel position of the touch point on the x-axis.
        int32_t posX;
        /// Current pixel position of the touch point on the y-axis.
        int32_t posY;
        /// Time of last touchdown.
        uint32_t tdTime;
        /// Touchdown pixel position on the x-axis.
        int32_t tdPosX;
        /// Touchdown pixel position on the y-axis.
        int32_t tdPosY;
    }; // struct Touch

    /// Maximum number of supported touch ports.
    static constexpr uint32_t TOUCH_MAX_PORTS = 16u;
    /// Maximum number of supported touch points per port.
    static constexpr uint32_t TOUCH_MAX_POINTS = 16u;

    /// Recognized types of dragging operations.
    enum TouchDragType {
        /// No dragging
        TOUCH_DRAG_NONE = 0, 
        /// Two-finger drag
        TOUCH_DRAG_TWO, 
        /// Three-finger drag
        TOUCH_DRAG_THREE
    }; // enum TouchDragType

    /// Types of simulated mouse buttons.
    enum TouchMouseButton {
        /// Left mouse button
        TOUCH_BUTTON_LEFT = 0, 
        /// Right mouse button
        TOUCH_BUTTON_RIGHT = 1, 
        /// Total number of simulated mouse buttons
        TOUCH_BUTTON_COUNT, 
    }; // enum TouchMouseButton

    /// Width of the touch panel.
    uint32_t _touchPanelWidth;
    /// Height of the touch panel.
    uint32_t _touchPanelHeight;
    /// Width of the touch window.
    uint32_t _touchWindowWidth;
    /// Height of the touch window.
    uint32_t _touchWindowHeight;

    /// Maximum time of contact in milliseconds which counts as a tap.
    uint32_t _touchMaxTapTime;
    /// Maximum distance of motion in squared pixels which counts as a tap.
    uint32_t _touchMaxTapSqDist;
    /// Length of a simulated mouse click in milliseconds.
    uint32_t _touchClickDuration;
    /// Multiplier used for sub-pixel touch precision - higher = smaller movement.
    uint32_t _touchPrecisionMult;

    /// Finger-tracking structure.
    Touch _touchFingers[TOUCH_MAX_PORTS][TOUCH_MAX_POINTS];

    /// Drag type being executed for each port.
    TouchDragType _touchDragType[TOUCH_MAX_PORTS];

    /// Tracking of start time for mouse button clicks.
    uint32_t _touchClickTime[TOUCH_MAX_PORTS][TOUCH_BUTTON_COUNT];

    /// High resolution x-axis delta for the simulated touchpad cursor.
    int32_t _touchHighResDX;
    /// High resolution y-axis delta for the simulated touchpad cursor.
    int32_t _touchHighResDY;
#endif // defined(SDL_TS_VMOUSE)
};

#endif
