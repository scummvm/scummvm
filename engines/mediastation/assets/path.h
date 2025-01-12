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

#ifndef MEDIASTATION_PATH_H
#define MEDIASTATION_PATH_H

#include "mediastation/assetheader.h"
#include "mediastation/asset.h"
#include "mediastation/mediascript/operand.h"
#include "mediastation/mediascript/builtins.h"

namespace MediaStation {

class Path : public Asset {
public:
	Path(AssetHeader *header) : Asset(header) {};
	virtual ~Path() override;

	virtual void process() override;

	virtual Operand callMethod(BuiltInMethod methodId, Common::Array<Operand> &args) override;

private:
	double _percentComplete = 0.0;

	// Method implementations.
	void timePlay();
	void setDuration(uint durationInMilliseconds);
	double percentComplete();
};

} // End of namespace MediaStation

#endif