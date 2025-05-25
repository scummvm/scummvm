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

#include "common/formats/winexe_ne.h"
#include "bagel/mfc/libs/resources.h"

namespace Bagel {
namespace MFC {
namespace Libs {

Resources::~Resources() {
	while (!_resources.empty())
		popResources();
}

void Resources::addResources(const Common::Path &file) {
	Common::NEResources *res = new Common::NEResources();
	if (!res->loadFromEXE(file))
		error("Could not load %s", file.baseName().c_str());

	_resources.push(res);
}

void Resources::popResources() {
	delete _resources.pop();
}

Common::WinResources *Resources::getResources() const {
	if (_resources.empty())
		error("Use CWinApp::addResources to register "
			"an .exe or .dll file containing the resources");
	return _resources.top();
}

} // namespace Libs
} // namespace MFC
} // namespace Bagel
