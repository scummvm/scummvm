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

#include "common/file.h"

#include "tetraedge/te/te_interpolation.h"

namespace Tetraedge {

TeInterpolation::TeInterpolation() {
}

void TeInterpolation::load(Common::ReadStream &stream) {
	uint32 len = stream.readUint32LE();
	if (len > 1000000)
		error("TeInterpolation: Unexpected interpolation length %d", len);
	_array.resize(len);
	for (uint32 i = 0; i < len && !stream.err(); i++)
		_array[i] = stream.readFloatLE();
}

void TeInterpolation::load(TetraedgeFSNode &node) {
	Common::ScopedPtr<Common::SeekableReadStream> f(node.createReadStream());
	if (!f)
		error("Couldn't open %s", node.toString().c_str());

	load(*f);
}


// Note: this function is not in the original but simplifies
// the code for TeCurveAnim2 a lot.
void TeInterpolation::load(const Common::Array<double> &array) {
	_array = array;
}

void TeInterpolation::load(const Common::Array<float> &array) {
	_array.clear();
	for (auto f : array) {
		_array.push_back(f);
	}
}

double TeInterpolation::interpole(double where, double max) const {
	const uint arrayLen = _array.size();
	if (!arrayLen)
		return 0.0;

	double elemNum = (arrayLen - 1) * where / max;
	int leftElemNum = (int)floor(elemNum);

	if (leftElemNum >= (int)arrayLen - 1)
		return _array[arrayLen - 1];
	else if (leftElemNum < 0)
		return _array[0];

	double left = _array[leftElemNum];
	double right = _array[leftElemNum + 1];

	return left + (right - left) * (elemNum - leftElemNum);
}


} // end namespace Tetraedge
