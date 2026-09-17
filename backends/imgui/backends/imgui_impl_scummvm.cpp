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

#include "backends/imgui/imgui.h"
#ifndef IMGUI_DISABLE
#include "common/system.h"
#include "imgui_impl_scummvm.h"

ImGui_ImplScummVM::ImGui_ImplScummVM(Common::EventDispatcher *eventDispatcher) : _eventDispatcher(eventDispatcher), _time(0)
{
    ImGuiIO& io = ImGui::GetIO();
    IMGUI_CHECKVERSION();
    IM_ASSERT(io.BackendPlatformUserData == nullptr && "Already initialized a platform backend!");

    // Setup backend capabilities flags
    io.BackendPlatformUserData = (void*)this;
    io.BackendPlatformName = "imgui_impl_scummvm";

    _eventDispatcher->registerObserver(this, 10, false);
}


ImGui_ImplScummVM::~ImGui_ImplScummVM()
{
    ImGuiIO& io = ImGui::GetIO();

    io.BackendPlatformName = nullptr;
    io.BackendPlatformUserData = nullptr;

    _eventDispatcher->unregisterObserver(this);
}

static ImGuiKey ImGui_ImplScummVM_KeycodeToImGuiKey(const Common::KeyCode keycode)
{
    switch (keycode)
    {
        case Common::KEYCODE_TAB: return ImGuiKey_Tab;
        case Common::KEYCODE_LEFT: return ImGuiKey_LeftArrow;
        case Common::KEYCODE_RIGHT: return ImGuiKey_RightArrow;
        case Common::KEYCODE_UP: return ImGuiKey_UpArrow;
        case Common::KEYCODE_DOWN: return ImGuiKey_DownArrow;
        case Common::KEYCODE_PAGEUP: return ImGuiKey_PageUp;
        case Common::KEYCODE_PAGEDOWN: return ImGuiKey_PageDown;
        case Common::KEYCODE_HOME: return ImGuiKey_Home;
        case Common::KEYCODE_END: return ImGuiKey_End;
        case Common::KEYCODE_INSERT: return ImGuiKey_Insert;
        case Common::KEYCODE_DELETE: return ImGuiKey_Delete;
        case Common::KEYCODE_BACKSPACE: return ImGuiKey_Backspace;
        case Common::KEYCODE_SPACE: return ImGuiKey_Space;
        case Common::KEYCODE_RETURN: return ImGuiKey_Enter;
        case Common::KEYCODE_ESCAPE: return ImGuiKey_Escape;
        case Common::KEYCODE_QUOTE: return ImGuiKey_Apostrophe;
        case Common::KEYCODE_COMMA: return ImGuiKey_Comma;
        case Common::KEYCODE_MINUS: return ImGuiKey_Minus;
        case Common::KEYCODE_PERIOD: return ImGuiKey_Period;
        case Common::KEYCODE_SLASH: return ImGuiKey_Slash;
        case Common::KEYCODE_SEMICOLON: return ImGuiKey_Semicolon;
        case Common::KEYCODE_EQUALS: return ImGuiKey_Equal;
        case Common::KEYCODE_LEFTBRACKET: return ImGuiKey_LeftBracket;
        case Common::KEYCODE_BACKSLASH: return ImGuiKey_Backslash;
        case Common::KEYCODE_RIGHTBRACKET: return ImGuiKey_RightBracket;
        case Common::KEYCODE_BACKQUOTE: return ImGuiKey_GraveAccent;
        case Common::KEYCODE_CAPSLOCK: return ImGuiKey_CapsLock;
        case Common::KEYCODE_SCROLLOCK: return ImGuiKey_ScrollLock;
        case Common::KEYCODE_NUMLOCK: return ImGuiKey_NumLock;
        case Common::KEYCODE_PRINT: return ImGuiKey_PrintScreen;
        case Common::KEYCODE_PAUSE: return ImGuiKey_Pause;
        case Common::KEYCODE_KP0: return ImGuiKey_Keypad0;
        case Common::KEYCODE_KP1: return ImGuiKey_Keypad1;
        case Common::KEYCODE_KP2: return ImGuiKey_Keypad2;
        case Common::KEYCODE_KP3: return ImGuiKey_Keypad3;
        case Common::KEYCODE_KP4: return ImGuiKey_Keypad4;
        case Common::KEYCODE_KP5: return ImGuiKey_Keypad5;
        case Common::KEYCODE_KP6: return ImGuiKey_Keypad6;
        case Common::KEYCODE_KP7: return ImGuiKey_Keypad7;
        case Common::KEYCODE_KP8: return ImGuiKey_Keypad8;
        case Common::KEYCODE_KP9: return ImGuiKey_Keypad9;
        case Common::KEYCODE_KP_PERIOD: return ImGuiKey_KeypadDecimal;
        case Common::KEYCODE_KP_DIVIDE: return ImGuiKey_KeypadDivide;
        case Common::KEYCODE_KP_MULTIPLY: return ImGuiKey_KeypadMultiply;
        case Common::KEYCODE_KP_MINUS: return ImGuiKey_KeypadSubtract;
        case Common::KEYCODE_KP_PLUS: return ImGuiKey_KeypadAdd;
        case Common::KEYCODE_KP_ENTER: return ImGuiKey_KeypadEnter;
        case Common::KEYCODE_KP_EQUALS: return ImGuiKey_KeypadEqual;
        case Common::KEYCODE_LCTRL: return ImGuiKey_LeftCtrl;
        case Common::KEYCODE_LSHIFT: return ImGuiKey_LeftShift;
        case Common::KEYCODE_LALT: return ImGuiKey_LeftAlt;
        case Common::KEYCODE_LMETA: return ImGuiKey_LeftSuper;
        case Common::KEYCODE_RCTRL: return ImGuiKey_RightCtrl;
        case Common::KEYCODE_RSHIFT: return ImGuiKey_RightShift;
        case Common::KEYCODE_RALT: return ImGuiKey_RightAlt;
        case Common::KEYCODE_RMETA: return ImGuiKey_RightSuper;
        case Common::KEYCODE_COMPOSE: return ImGuiKey_Menu;
        case Common::KEYCODE_0: return ImGuiKey_0;
        case Common::KEYCODE_1: return ImGuiKey_1;
        case Common::KEYCODE_2: return ImGuiKey_2;
        case Common::KEYCODE_3: return ImGuiKey_3;
        case Common::KEYCODE_4: return ImGuiKey_4;
        case Common::KEYCODE_5: return ImGuiKey_5;
        case Common::KEYCODE_6: return ImGuiKey_6;
        case Common::KEYCODE_7: return ImGuiKey_7;
        case Common::KEYCODE_8: return ImGuiKey_8;
        case Common::KEYCODE_9: return ImGuiKey_9;
        case Common::KEYCODE_a: return ImGuiKey_A;
        case Common::KEYCODE_b: return ImGuiKey_B;
        case Common::KEYCODE_c: return ImGuiKey_C;
        case Common::KEYCODE_d: return ImGuiKey_D;
        case Common::KEYCODE_e: return ImGuiKey_E;
        case Common::KEYCODE_f: return ImGuiKey_F;
        case Common::KEYCODE_g: return ImGuiKey_G;
        case Common::KEYCODE_h: return ImGuiKey_H;
        case Common::KEYCODE_i: return ImGuiKey_I;
        case Common::KEYCODE_j: return ImGuiKey_J;
        case Common::KEYCODE_k: return ImGuiKey_K;
        case Common::KEYCODE_l: return ImGuiKey_L;
        case Common::KEYCODE_m: return ImGuiKey_M;
        case Common::KEYCODE_n: return ImGuiKey_N;
        case Common::KEYCODE_o: return ImGuiKey_O;
        case Common::KEYCODE_p: return ImGuiKey_P;
        case Common::KEYCODE_q: return ImGuiKey_Q;
        case Common::KEYCODE_r: return ImGuiKey_R;
        case Common::KEYCODE_s: return ImGuiKey_S;
        case Common::KEYCODE_t: return ImGuiKey_T;
        case Common::KEYCODE_u: return ImGuiKey_U;
        case Common::KEYCODE_v: return ImGuiKey_V;
        case Common::KEYCODE_w: return ImGuiKey_W;
        case Common::KEYCODE_x: return ImGuiKey_X;
        case Common::KEYCODE_y: return ImGuiKey_Y;
        case Common::KEYCODE_z: return ImGuiKey_Z;
        case Common::KEYCODE_F1: return ImGuiKey_F1;
        case Common::KEYCODE_F2: return ImGuiKey_F2;
        case Common::KEYCODE_F3: return ImGuiKey_F3;
        case Common::KEYCODE_F4: return ImGuiKey_F4;
        case Common::KEYCODE_F5: return ImGuiKey_F5;
        case Common::KEYCODE_F6: return ImGuiKey_F6;
        case Common::KEYCODE_F7: return ImGuiKey_F7;
        case Common::KEYCODE_F8: return ImGuiKey_F8;
        case Common::KEYCODE_F9: return ImGuiKey_F9;
        case Common::KEYCODE_F10: return ImGuiKey_F10;
        case Common::KEYCODE_F11: return ImGuiKey_F11;
        case Common::KEYCODE_F12: return ImGuiKey_F12;
        case Common::KEYCODE_F13: return ImGuiKey_F13;
        case Common::KEYCODE_F14: return ImGuiKey_F14;
        case Common::KEYCODE_F15: return ImGuiKey_F15;
        case Common::KEYCODE_F16: return ImGuiKey_F16;
        case Common::KEYCODE_F17: return ImGuiKey_F17;
        case Common::KEYCODE_F18: return ImGuiKey_F18;
#if 0
        case Common::KEYCODE_F19: return ImGuiKey_F19;
        case Common::KEYCODE_F20: return ImGuiKey_F20;
        case Common::KEYCODE_F21: return ImGuiKey_F21;
        case Common::KEYCODE_F22: return ImGuiKey_F22;
        case Common::KEYCODE_F23: return ImGuiKey_F23;
        case Common::KEYCODE_F24: return ImGuiKey_F24;
#endif
        case Common::KEYCODE_AC_BACK: return ImGuiKey_AppBack;
        case Common::KEYCODE_AC_FORWARD: return ImGuiKey_AppForward;
        default: break;
    }
    return ImGuiKey_None;
}

static void ImGui_ImplScummVM_UpdateKeyModifiers(byte flags)
{
    ImGuiIO& io = ImGui::GetIO();
    io.AddKeyEvent(ImGuiMod_Ctrl, (flags & Common::KBD_CTRL) != 0);
    io.AddKeyEvent(ImGuiMod_Shift, (flags & Common::KBD_SHIFT) != 0);
    io.AddKeyEvent(ImGuiMod_Alt, (flags & Common::KBD_ALT) != 0);
    io.AddKeyEvent(ImGuiMod_Super, (flags & Common::KBD_META) != 0);
}

bool ImGui_ImplScummVM::notifyEvent(const Common::Event &event)
{
    ImGuiIO& io = ImGui::GetIO();

    switch (event.type)
    {
        case Common::EVENT_KEYDOWN:
        case Common::EVENT_KEYUP:
        {
            if (Common::isPrint(event.kbd.ascii) || event.kbd.ascii > 128) {
                io.AddInputCharacter(event.kbd.ascii);
            } else {
                ImGui_ImplScummVM_UpdateKeyModifiers(event.kbd.flags);
                ImGuiKey key = ImGui_ImplScummVM_KeycodeToImGuiKey(event.kbd.keycode);
                io.AddKeyEvent(key, (event.type == Common::EVENT_KEYDOWN));
                io.SetKeyEventNativeData(key, event.kbd.keycode, event.kbd.keycode);
            }
            return io.WantTextInput;
        }

        case Common::EVENT_MOUSEMOVE:
            io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
            io.AddMousePosEvent((float)event.rawMouse.x, (float)event.rawMouse.y);
            return io.WantCaptureMouse;
        case Common::EVENT_LBUTTONDOWN:
            io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
            io.AddMouseButtonEvent(0, true);
            return io.WantCaptureMouse;
        case Common::EVENT_LBUTTONUP:
            io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
            io.AddMouseButtonEvent(0, false);
            return io.WantCaptureMouse;
        case Common::EVENT_RBUTTONDOWN:
            io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
            io.AddMouseButtonEvent(1, true);
            return io.WantCaptureMouse;
        case Common::EVENT_RBUTTONUP:
            io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
            io.AddMouseButtonEvent(1, false);
            return io.WantCaptureMouse;
        case Common::EVENT_MBUTTONDOWN:
            io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
            io.AddMouseButtonEvent(2, true);
            return io.WantCaptureMouse;
        case Common::EVENT_MBUTTONUP:
            io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
            io.AddMouseButtonEvent(2, false);
            return io.WantCaptureMouse;
        case Common::EVENT_X1BUTTONDOWN:
            io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
            io.AddMouseButtonEvent(3, true);
            return io.WantCaptureMouse;
        case Common::EVENT_X1BUTTONUP:
            io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
            io.AddMouseButtonEvent(3, false);
            return io.WantCaptureMouse;
        case Common::EVENT_X2BUTTONDOWN:
            io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
            io.AddMouseButtonEvent(4, true);
            return io.WantCaptureMouse;
        case Common::EVENT_X2BUTTONUP:
            io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
            io.AddMouseButtonEvent(4, false);
            return io.WantCaptureMouse;
        case Common::EVENT_WHEELUP:
            io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
            io.AddMouseWheelEvent(0, 1);
            return io.WantCaptureMouse;
        case Common::EVENT_WHEELDOWN:
            io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
            io.AddMouseWheelEvent(0, -1);
            return io.WantCaptureMouse;
        case Common::EVENT_FOCUS_GAINED:
            io.AddFocusEvent(true);
            break;
        case Common::EVENT_FOCUS_LOST:
            io.AddFocusEvent(true);
            break;
        default:
            break;
    }
    return false;
}

void ImGui_ImplScummVM::newFrame()
{
    ImGuiIO& io = ImGui::GetIO();

    // Setup display size (every frame to accommodate for window resizing)
    // HACK: We assume that the overlay is the same size as the window,
    // however it may be smaller due to the maximum texture size.
    int w = g_system->getOverlayWidth();
    int h = g_system->getOverlayHeight();
    io.DisplaySize = ImVec2((float)w, (float)h);
    io.DisplayFramebufferScale = ImVec2(1, 1);

    // Setup time step (we don't use SDL_GetTicks() because it is using millisecond resolution)
    // (Accept SDL_GetPerformanceCounter() not returning a monotonically increasing value. Happens in VMs and Emscripten, see #6189, #6114, #3644)
    static const uint64 frequency = 1000;
    uint64 current_time = g_system->getMillis();
    if (current_time <= _time)
        current_time = _time + 1;
    io.DeltaTime = _time > 0 ? (float)((double)(current_time - _time) / frequency) : (float)(1.0f / 60.0f);
    _time = current_time;
}

//-----------------------------------------------------------------------------

#endif // #ifndef IMGUI_DISABLE
