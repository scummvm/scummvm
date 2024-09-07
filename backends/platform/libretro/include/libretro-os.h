/* Copyright (C) 2023 Giovanni Cascione <ing.cascione@gmail.com>
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
#ifndef BACKENDS_LIBRETRO_OS_H
#define BACKENDS_LIBRETRO_OS_H

#include "audio/mixer_intern.h"
#include "base/main.h"
#include "backends/base-backend.h"
#include "common/system.h"
#include "common/mutex.h"
#include "common/list.h"
#include "common/events.h"
#include "backends/modular-backend.h"

#define BASE_CURSOR_SPEED 4
#define CURSOR_STATUS_DOING_JOYSTICK  (1 << 0)
#define CURSOR_STATUS_DOING_MOUSE     (1 << 1)
#define CURSOR_STATUS_DOING_X         (1 << 2)
#define CURSOR_STATUS_DOING_Y         (1 << 3)
#define CURSOR_STATUS_DOING_SLOWER    (1 << 4)

#define LIBRETRO_G_SYSTEM dynamic_cast<OSystem_libretro *>(g_system)

/**
 *  Dummy mutex implementation
 */
class LibretroMutexInternal final : public Common::MutexInternal {
public:
	LibretroMutexInternal() {};
	~LibretroMutexInternal() override {};
	bool lock() override {
		return 0;
	}
	bool unlock() override {
		return 0;
	};
};

class OSystem_libretro : public EventsBaseBackend, public ModularGraphicsBackend {
private:
	int _relMouseX;
	int _relMouseY;
	float _mouseXAcc;
	float _mouseYAcc;
	float _dpadXAcc;
	float _dpadYAcc;
	float _dpadXVel;
	float _dpadYVel;
	float _adjusted_cursor_speed;
	float _inverse_acceleration_time;
	uint32 _startTime;
	uint8 _cursorStatus;
	Common::String s_systemDir;
	Common::String s_saveDir;
	Common::String s_playlistDir;
	Common::List<Common::Event> _events;

public:
	Audio::MixerImpl *_mixer;
	bool _mouseButtons[2];
	bool _ptrmouseButton;
	int _mouseX;
	int _mouseY;

	/* Base */
	OSystem_libretro(void);
	~OSystem_libretro(void) override;
	void initBackend(void) override;
	void engineInit(void) override;
	void refreshRetroSettings(void);
	void refreshScreen(void);
	void destroy(void);
	void quit() override {}

	void resetGraphicsManager(void);
	void getScreen(const Graphics::Surface *&screen);
	int16 getScreenWidth(void);
	int16 getScreenHeight(void);
	bool isOverlayInGUI(void);

#ifdef USE_OPENGL
	void resetGraphicsContext(void);
	void *getOpenGLProcAddress(const char *name) const override;
#endif

private:
	bool checkPathSetting(const char *setting, Common::String const &defaultPath, bool isDirectory = true);
	void setLibretroDir(const char *path, Common::String &var);

	/* Events */
public:
	bool pollEvent(Common::Event &event) override;
	uint32 getMillis(bool skipRecord = false) override;
	void delayMillis(uint msecs) override;
	Common::MutexInternal *createMutex(void) override;
	void requestQuit(void);
	void resetQuit(void);

	void setMousePosition(int x, int y);
	Common::Point convertWindowToVirtual(int x, int y) const;

	/* Utils */
	void getTimeAndDate(TimeDate &t, bool skipRecord) const override;
	Audio::Mixer *getMixer(void) override;
	Common::Path getDefaultConfigFileName(void) override;
	void logMessage(LogMessageType::Type type, const char *message) override;
	int testGame(const char *filedata, bool autodetect);
	void addSysArchivesToSearchSet(Common::SearchSet &s, int priority = 0) override;
	const char *const *buildHelpDialogData() override;
	Common::String getSaveDir(void);
	GUI::OptionsContainerWidget *buildBackendOptionsWidget(GUI::GuiObject *boss, const Common::String &name, const Common::String &target) const override;
	void applyBackendSettings() override;
private:
	bool parseGameName(const Common::String &gameName, Common::String &engineId, Common::String &gameId);

	/* Inputs */
public:
	void processInputs(void);
	void processKeyEvent(bool down, unsigned keycode, uint32 character, uint16 key_modifiers);
private:
	void updateMouseXY(float deltaAcc, float *cumulativeXYAcc, int doing_x);
	void getMouseXYFromAnalog(bool is_x, int16 coor);
	void getMouseXYFromButton(bool is_x, int16 sign);
};

#endif
