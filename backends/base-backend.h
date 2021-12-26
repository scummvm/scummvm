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

#ifndef BACKENDS_BASE_BACKEND_H
#define BACKENDS_BASE_BACKEND_H

#include "common/system.h"
#include "common/events.h"

/**
 * Subclass of OSystem that contains default implementations of functions that would
 * cause circular dependencies if they were implemented in common/system.cpp
 */
class BaseBackend : public OSystem {
public:
	void initBackend() override;

	using OSystem::setScaler;
	bool setScaler(const char *name, int factor) override final;
	void displayMessageOnOSD(const Common::U32String &msg) override;
	void displayActivityIconOnOSD(const Graphics::Surface *icon) override {}
	void fillScreen(uint32 col) override;
};

class EventsBaseBackend : virtual public BaseBackend, Common::EventSource {
public:
	virtual void initBackend();
};


#endif
