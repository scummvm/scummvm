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

#ifndef BACKENDS_PRINTING_PRINTMAN_H
#define BACKENDS_PRINTING_PRINTMAN_H

#include "common/scummsys.h"
#include "common/str.h"
#include "common/str-array.h"
#include "graphics/managed_surface.h"

namespace GUI {
class PrintingDialog;
}

namespace Common {

class PrintingManager {
public:
	friend class GUI::PrintingDialog;

	virtual ~PrintingManager();

	PrintingManager() {
		_jobName = "ScummVM";
	}

	void printImage(const Graphics::ManagedSurface &surf);

	void saveAsImage(const Graphics::ManagedSurface &surf, const Common::String &fileName = "");

	void setJobName(const Common::String &jobName) { _jobName = jobName; }

protected:
	virtual void doPrint(const Graphics::ManagedSurface &surf) = 0;

	virtual Common::StringArray listPrinterNames() const;

	virtual Common::String getDefaultPrinterName() const;

	void setPrinterName(const Common::String &printerName) { _printerName = printerName; }

	Common::String _jobName;
	Common::String _printerName;
};
} // End of namespace Common

#endif
