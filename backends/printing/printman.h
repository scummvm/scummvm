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


#include "common/scummsys.h"
#include "common/str.h"
#include "graphics/managed_surface.h"

class PrintJob;

class PrintingManager {
public:
	virtual ~PrintingManager();

	virtual PrintJob *createJob(Common::String jobName) = 0;

	void printImage(Common::String jobName, const Graphics::ManagedSurface &surf);

	void printImage(const Graphics::ManagedSurface& surf) {
		printImage("ScummVM", surf);
	}
};

class PrintJob {
public:
	friend class PrintingManager;
	virtual ~PrintJob();

	virtual void drawBitmap(const Graphics::ManagedSurface &surf, int x, int y) = 0;

	virtual void newPage() = 0;
	virtual void endDoc() = 0;
	virtual void abortJob() = 0;
};
