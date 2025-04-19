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

#include "common/system.h"
#include "awe/graphics.h"
#include "awe/systemstub.h"
#include "awe/util.h"

namespace Awe {

struct SystemStub_SDL : SystemStub {
	static const int kJoystickIndex = 0;
	static const int kJoystickCommitValue = 16384;
	static const float kAspectRatio;

	int _w = 0, _h = 0;
	float _aspectRatio[4] = { 0.0 };
#ifdef TODO
	SDL_Window *_window;
	SDL_Renderer *_renderer;
	SDL_GLContext _glcontext;
	int _texW, _texH;
	SDL_Texture *_texture;
	SDL_Joystick *_joystick;
	SDL_GameController *_controller;
#endif
	int _screenshot = 0;

	SystemStub_SDL();
	virtual ~SystemStub_SDL() {
	}

	virtual void init(const char *title, const DisplayMode *dm);
	virtual void fini();

	virtual void prepareScreen(int &w, int &h, float ar[4]);
	virtual void updateScreen();
	virtual void setScreenPixels555(const uint16_t *data, int w, int h);

	virtual void processEvents();
	virtual void sleep(uint32_t duration);
	virtual uint32_t getTimeStamp();

	void setAspectRatio(int w, int h);
};

const float SystemStub_SDL::kAspectRatio = 16.f / 10.f;

SystemStub_SDL::SystemStub_SDL()
	: _w(0), _h(0)
#ifdef TODO
	, _window(0), _renderer(0), _texW(0), _texH(0), _texture(0)
#endif
{
}

void SystemStub_SDL::init(const char *title, const DisplayMode *dm) {
#ifdef TODO
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER);
	SDL_ShowCursor(SDL_DISABLE);
	// SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

	int windowW = 0;
	int windowH = 0;
	int flags = dm->opengl ? SDL_WINDOW_OPENGL : 0;
	if (dm->mode == DisplayMode::WINDOWED) {
		flags |= SDL_WINDOW_RESIZABLE;
		windowW = dm->width;
		windowH = dm->height;
	} else {
		flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
	}
	_window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowW, windowH, flags);
	SDL_GetWindowSize(_window, &_w, &_h);

	if (dm->opengl) {
		_glcontext = SDL_GL_CreateContext(_window);
	} else {
		_glcontext = 0;
		_renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED);
		SDL_SetRenderDrawColor(_renderer, 0, 0, 0, 255);
		SDL_RenderClear(_renderer);
	}
	_aspectRatio[0] = _aspectRatio[1] = 0.;
	_aspectRatio[2] = _aspectRatio[3] = 1.;
	if (dm->mode == DisplayMode::FULLSCREEN_AR) {
		if (dm->opengl) {
			setAspectRatio(_w, _h);
		} else {
			SDL_RenderSetLogicalSize(_renderer, 320, 200);
		}
	}
	_joystick = 0;
	_controller = 0;
	if (SDL_NumJoysticks() > 0) {

#if SDL_COMPILEDVERSION >= SDL_VERSIONNUM(2,0,2)
		SDL_GameControllerAddMappingsFromFile("gamecontrollerdb.txt");
#endif

		if (SDL_IsGameController(kJoystickIndex)) {
			_controller = SDL_GameControllerOpen(kJoystickIndex);
		}
		if (!_controller) {
			_joystick = SDL_JoystickOpen(kJoystickIndex);
		}
	}
#endif
	_screenshot = 1;
	_dm = *dm;
}

void SystemStub_SDL::fini() {
#ifdef TODO
	if (_texture) {
		SDL_DestroyTexture(_texture);
		_texture = 0;
	}
	if (_joystick) {
		SDL_JoystickClose(_joystick);
		_joystick = 0;
	}
	if (_controller) {
		SDL_GameControllerClose(_controller);
		_controller = 0;
	}
	if (_renderer) {
		SDL_DestroyRenderer(_renderer);
		_renderer = 0;
	}
	if (_glcontext) {
		SDL_GL_DeleteContext(_glcontext);
		_glcontext = 0;
	}
	SDL_DestroyWindow(_window);
	SDL_Quit();
#endif
}

void SystemStub_SDL::prepareScreen(int &w, int &h, float ar[4]) {
	w = _w;
	h = _h;
	ar[0] = _aspectRatio[0];
	ar[1] = _aspectRatio[1];
	ar[2] = _aspectRatio[2];
	ar[3] = _aspectRatio[3];
#ifdef TODO
	if (_renderer) {
		SDL_RenderClear(_renderer);
	}
#endif
}

void SystemStub_SDL::updateScreen() {
#ifdef TODO
	if (_renderer) {
		SDL_RenderPresent(_renderer);
	} else {
		SDL_GL_SwapWindow(_window);
	}
#endif
}

void SystemStub_SDL::setScreenPixels555(const uint16_t *data, int w, int h) {
#ifdef TODO
	if (_renderer) {
		if (!_texture) {
			_texture = SDL_CreateTexture(_renderer, SDL_PIXELFORMAT_RGB555, SDL_TEXTUREACCESS_STREAMING, w, h);
			if (!_texture) {
				return;
			}
			_texW = w;
			_texH = h;
		}
		assert(w <= _texW && h <= _texH);
		SDL_Rect r;
		r.w = w;
		r.h = h;
		if (w != _texW && h != _texH) {
			r.x = (_texW - w) / 2;
			r.y = (_texH - h) / 2;
		} else {
			r.x = 0;
			r.y = 0;
		}
		SDL_UpdateTexture(_texture, &r, data, w * sizeof(uint16_t));
		SDL_RenderCopy(_renderer, _texture, 0, 0);
	}
#endif
}

void SystemStub_SDL::processEvents() {
#ifdef TODO
	SDL_Event ev;
	while (SDL_PollEvent(&ev)) {
		switch (ev.type) {
		case SDL_QUIT:
			_pi.quit = true;
			break;
		case SDL_WINDOWEVENT:
			if (ev.window.event == SDL_WINDOWEVENT_RESIZED) {
				_w = _dm.width = ev.window.data1;
				_h = _dm.height = ev.window.data2;
			} else if (ev.window.event == SDL_WINDOWEVENT_CLOSE) {
				_pi.quit = true;
			}
			break;
		case SDL_KEYUP:
			switch (ev.key.keysym.sym) {
			case SDLK_LEFT:
				_pi.dirMask &= ~PlayerInput::DIR_LEFT;
				break;
			case SDLK_RIGHT:
				_pi.dirMask &= ~PlayerInput::DIR_RIGHT;
				break;
			case SDLK_UP:
				_pi.dirMask &= ~PlayerInput::DIR_UP;
				break;
			case SDLK_DOWN:
				_pi.dirMask &= ~PlayerInput::DIR_DOWN;
				break;
			case SDLK_SPACE:
			case SDLK_RETURN:
				_pi.action = false;
				break;
			case SDLK_LSHIFT:
			case SDLK_RSHIFT:
				_pi.jump = false;
				break;
			case SDLK_s:
				_pi.screenshot = true;
				break;
			case SDLK_c:
				_pi.code = true;
				break;
			case SDLK_p:
				_pi.pause = true;
				break;
			case SDLK_ESCAPE:
			case SDLK_AC_BACK:
				_pi.back = true;
				break;
			case SDLK_AC_HOME:
				_pi.quit = true;
				break;
			default:
				break;
			}
			break;
		case SDL_KEYDOWN:
			if (ev.key.keysym.mod & KMOD_ALT) {
				if (ev.key.keysym.sym == SDLK_RETURN || ev.key.keysym.sym == SDLK_KP_ENTER) {
				} else if (ev.key.keysym.sym == SDLK_x) {
					_pi.quit = true;
				}
				break;
			} else if (ev.key.keysym.mod & KMOD_CTRL) {
				if (ev.key.keysym.sym == SDLK_f) {
					_pi.fastMode = true;
				}
				break;
			}
			_pi.lastChar = ev.key.keysym.sym;
			switch (ev.key.keysym.sym) {
			case SDLK_LEFT:
				_pi.dirMask |= PlayerInput::DIR_LEFT;
				break;
			case SDLK_RIGHT:
				_pi.dirMask |= PlayerInput::DIR_RIGHT;
				break;
			case SDLK_UP:
				_pi.dirMask |= PlayerInput::DIR_UP;
				break;
			case SDLK_DOWN:
				_pi.dirMask |= PlayerInput::DIR_DOWN;
				break;
			case SDLK_SPACE:
			case SDLK_RETURN:
				_pi.action = true;
				break;
			case SDLK_LSHIFT:
			case SDLK_RSHIFT:
				_pi.jump = true;
				break;
			default:
				break;
			}
			break;
		case SDL_JOYHATMOTION:
			if (_joystick) {
				_pi.dirMask = 0;
				if (ev.jhat.value & SDL_HAT_UP) {
					_pi.dirMask |= PlayerInput::DIR_UP;
				}
				if (ev.jhat.value & SDL_HAT_DOWN) {
					_pi.dirMask |= PlayerInput::DIR_DOWN;
				}
				if (ev.jhat.value & SDL_HAT_LEFT) {
					_pi.dirMask |= PlayerInput::DIR_LEFT;
				}
				if (ev.jhat.value & SDL_HAT_RIGHT) {
					_pi.dirMask |= PlayerInput::DIR_RIGHT;
				}
			}
			break;
		case SDL_JOYAXISMOTION:
			if (_joystick) {
				switch (ev.jaxis.axis) {
				case 0:
					_pi.dirMask &= ~(PlayerInput::DIR_RIGHT | PlayerInput::DIR_LEFT);
					if (ev.jaxis.value > kJoystickCommitValue) {
						_pi.dirMask |= PlayerInput::DIR_RIGHT;
					} else if (ev.jaxis.value < -kJoystickCommitValue) {
						_pi.dirMask |= PlayerInput::DIR_LEFT;
					}
					break;
				case 1:
					_pi.dirMask &= ~(PlayerInput::DIR_UP | PlayerInput::DIR_DOWN);
					if (ev.jaxis.value > kJoystickCommitValue) {
						_pi.dirMask |= PlayerInput::DIR_DOWN;
					} else if (ev.jaxis.value < -kJoystickCommitValue) {
						_pi.dirMask |= PlayerInput::DIR_UP;
					}
					break;
				}
			}
			break;
		case SDL_JOYBUTTONDOWN:
		case SDL_JOYBUTTONUP:
			if (_joystick) {
				_pi.action = (ev.jbutton.state == SDL_PRESSED);
			}
			break;
		case SDL_CONTROLLERAXISMOTION:
			if (_controller) {
				switch (ev.caxis.axis) {
				case SDL_CONTROLLER_AXIS_LEFTX:
				case SDL_CONTROLLER_AXIS_RIGHTX:
					if (ev.caxis.value < -kJoystickCommitValue) {
						_pi.dirMask |= PlayerInput::DIR_LEFT;
					} else {
						_pi.dirMask &= ~PlayerInput::DIR_LEFT;
					}
					if (ev.caxis.value > kJoystickCommitValue) {
						_pi.dirMask |= PlayerInput::DIR_RIGHT;
					} else {
						_pi.dirMask &= ~PlayerInput::DIR_RIGHT;
					}
					break;
				case SDL_CONTROLLER_AXIS_LEFTY:
				case SDL_CONTROLLER_AXIS_RIGHTY:
					if (ev.caxis.value < -kJoystickCommitValue) {
						_pi.dirMask |= PlayerInput::DIR_UP;
					} else {
						_pi.dirMask &= ~PlayerInput::DIR_UP;
					}
					if (ev.caxis.value > kJoystickCommitValue) {
						_pi.dirMask |= PlayerInput::DIR_DOWN;
					} else {
						_pi.dirMask &= ~PlayerInput::DIR_DOWN;
					}
					break;
				}
			}
			break;
		case SDL_CONTROLLERBUTTONDOWN:
		case SDL_CONTROLLERBUTTONUP:
			if (_controller) {
				const bool pressed = (ev.cbutton.state == SDL_PRESSED);
				switch (ev.cbutton.button) {
				case SDL_CONTROLLER_BUTTON_BACK:
					_pi.back = pressed;
					break;
				case SDL_CONTROLLER_BUTTON_GUIDE:
					_pi.code = pressed;
					break;
				case SDL_CONTROLLER_BUTTON_START:
					_pi.pause = pressed;
					break;
				case SDL_CONTROLLER_BUTTON_DPAD_UP:
					if (pressed) {
						_pi.dirMask |= PlayerInput::DIR_UP;
					} else {
						_pi.dirMask &= ~PlayerInput::DIR_UP;
					}
					break;
				case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
					if (pressed) {
						_pi.dirMask |= PlayerInput::DIR_DOWN;
					} else {
						_pi.dirMask &= ~PlayerInput::DIR_DOWN;
					}
					break;
				case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
					if (pressed) {
						_pi.dirMask |= PlayerInput::DIR_LEFT;
					} else {
						_pi.dirMask &= ~PlayerInput::DIR_LEFT;
					}
					break;
				case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
					if (pressed) {
						_pi.dirMask |= PlayerInput::DIR_RIGHT;
					} else {
						_pi.dirMask &= ~PlayerInput::DIR_RIGHT;
					}
					break;
				case SDL_CONTROLLER_BUTTON_A:
					_pi.action = pressed;
					break;
				case SDL_CONTROLLER_BUTTON_B:
					_pi.jump = pressed;
					break;
				}
			}
			break;
		default:
			break;
		}
	}
#endif
}

void SystemStub_SDL::sleep(uint32_t duration) {
	g_system->delayMillis(duration);
}

uint32_t SystemStub_SDL::getTimeStamp() {
	return g_system->getMillis();
}

void SystemStub_SDL::setAspectRatio(int w, int h) {
	const float currentAspectRatio = w / (float)h;
	if (int(currentAspectRatio * 100) == int(kAspectRatio * 100)) {
		_aspectRatio[0] = 0.f;
		_aspectRatio[1] = 0.f;
		_aspectRatio[2] = 1.f;
		_aspectRatio[3] = 1.f;
		return;
	}
	// pillar box
	if (currentAspectRatio > kAspectRatio) {
		const float inset = 1.f - kAspectRatio / currentAspectRatio;
		_aspectRatio[0] = inset / 2;
		_aspectRatio[1] = 0.f;
		_aspectRatio[2] = 1.f - inset;
		_aspectRatio[3] = 1.f;
		return;
	}
	// letter box
	if (currentAspectRatio < kAspectRatio) {
		const float inset = 1.f - currentAspectRatio / kAspectRatio;
		_aspectRatio[0] = 0.f;
		_aspectRatio[1] = inset / 2;
		_aspectRatio[2] = 1.f;
		_aspectRatio[3] = 1.f - inset;
		return;
	}
}

SystemStub *SystemStub_SDL_create() {
	return new SystemStub_SDL();
}

} // namespace Awe
