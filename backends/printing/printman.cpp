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

#ifdef USE_PRINTING

#include "printman.h"

PrintingManager::~PrintingManager() {}

void PrintingManager::printImage(const Common::String &jobName, const Graphics::ManagedSurface &surf, bool scale) {
	PrintJob *job = createJob(jobName);

	if (scale) {
		Common::Rect printArea=job->getPrintableArea();

		Common::Rational xRatio(printArea.width(), surf.w);
		Common::Rational yRatio(printArea.height(), surf.h);

		Common::Rational scaleFactor = ((xRatio < yRatio) ? xRatio : yRatio);

		Common::Rect bitmapArea(surf.w * scaleFactor.toDouble(), surf.h * scaleFactor.toDouble());

		job->drawBitmap(surf, bitmapArea);
	} else {
		job->drawBitmap(surf, Common::Point());
	}

	job->pageFinished();
	job->endDoc();

	delete job;
}

void PrintingManager::printPlainTextFile(Common::File &file) {
	printPlainTextFile(file.getName(), file);
}

void PrintingManager::printPlainTextFile(const Common::String &jobName, Common::SeekableReadStream &file) {
	PrintJob *job = createJob(jobName);

	Common::Rect printArea = job->getPrintableArea();

	Common::Point textPos;

	TextMetrics metrics = job->getTextMetrics();

	while (!file.eos()) {
		Common::String line = file.readLine();

		Common::Rect bounds(1, metrics.getHeight());

		bounds.moveTo(textPos);

		if (!printArea.contains(bounds)) {
			textPos.y = 0;
			job->pageFinished();
		}

		job->drawText(line, textPos);

		textPos.y += metrics.getLineHeight();
	}
	if (textPos != Common::Point(0, 0)) {
		job->pageFinished();
	}

	job->endDoc();
	delete job;
}



PrintJob::~PrintJob() {}

void PrintJob::drawBitmap(const Graphics::ManagedSurface &surf, Common::Point pos) {
	drawBitmap(surf, Common::Rect(pos.x, pos.y, pos.x + surf.w, pos.y + surf.w));
}

#endif
