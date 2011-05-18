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
 */

#include "system.h"
#include "application.h"

using namespace Osp::Base;
using namespace Osp::Base::Runtime;
using namespace Osp::Graphics;
using namespace Osp::Locales;
using namespace Osp::System;
using namespace Osp::App;
using namespace Osp::System;
using namespace Osp::Ui;
using namespace Osp::Ui::Controls;

Application* BadaScummVM::createInstance() {
  return new BadaScummVM();
}

BadaScummVM::BadaScummVM() {
}

BadaScummVM::~BadaScummVM() {
}

bool BadaScummVM::OnAppInitializing(AppRegistry& appRegistry) {
	result r = E_SUCCESS;
}

bool BadaScummVM::OnAppTerminating(AppRegistry& appRegistry, bool forcedTermination) {
	return true;
}

void BadaScummVM::OnForeground(void) {
}

void BadaScummVM::OnBackground(void) {

}

void BadaScummVM::OnBatteryLevelChanged(BatteryLevel batteryLevel) {

}

void BadaScummVM::OnLowMemory(void) {

}

void BadaScummVM::OnKeyPressed(const Control& source, KeyCode keyCode) {
}

void BadaScummVM::OnKeyReleased(const Control& source, KeyCode keyCode) {	
}

void BadaScummVM::OnKeyLongPressed(const Control& source, KeyCode keyCode) {
}

bool BadaScummVM::Draw() {
	return true;
}

void BadaScummVM::OnScreenOn(void) {

}

void BadaScummVM::OnScreenOff(void) {

}

