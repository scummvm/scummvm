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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "titanic/star_control/base_stars.h"
#include "titanic/star_control/camera.h"
#include "titanic/star_control/star_closeup.h"
#include "titanic/star_control/star_ref.h"
#include "titanic/support/files_manager.h"
#include "titanic/support/simple_file.h"
#include "titanic/titanic.h"

namespace Titanic {

CBaseStarEntry::CBaseStarEntry() : _red(0), _value(0.0) {
	Common::fill(&_data[0], &_data[5], 0);
}

void CBaseStarEntry::load(Common::SeekableReadStream &s) {
	_red = s.readByte();
	_green = s.readByte();
	_blue = s.readByte();
	_thickness = s.readByte();
	_value = s.readFloatLE();
	_position._x = s.readFloatLE();
	_position._y = s.readFloatLE();
	_position._z = s.readFloatLE();

	for (int idx = 0; idx < 5; ++idx)
		_data[idx] = s.readUint32LE();
}

bool CBaseStarEntry::operator==(const CBaseStarEntry &s) const {
	return _red == s._red && _green == s._green
		&& _blue == s._blue && _thickness == s._thickness
		&& _value == s._value && _position == s._position
		&& _data[0] == s._data[0] && _data[1] == s._data[1]
		&& _data[2] == s._data[2] && _data[3] == s._data[3]
		&& _data[4] == s._data[4];
}

/*------------------------------------------------------------------------*/

CBaseStars::CBaseStars() : _minVal(0.0), _maxVal(1.0), _range(0.0),
		_value1(0.0), _value2(0.0), _value3(0.0), _value4(0.0) {
}

void CBaseStars::clear() {
	_data.clear();
}

void CBaseStars::initialize() {
	_minVal = 9.9999998e10;
	_maxVal = -9.9999998e10;
	_minMax.reset();

	for (uint idx = 0; idx < _data.size(); ++idx) {
		const CBaseStarEntry *entry = getDataPtr(idx);
		_minMax.expand(entry->_position);

		if (entry->_value < _minVal)
			_minVal = entry->_value;
		if (entry->_value > _maxVal)
			_maxVal = entry->_value;
	}

	_range = (_maxVal - _minVal) / 1.0;
}

const CBaseStarEntry *CBaseStars::getDataPtr(int index) const {
	return (index >= 0 && index < (int)_data.size()) ? &_data[index] : nullptr;
}

void CBaseStars::loadData(Common::SeekableReadStream &s) {
	uint headerId = s.readUint32LE();
	uint count = s.readUint32LE();
	if (headerId != 100 || count == 0)
		error("Invalid star data");

	// Initialize the data array
	clear();
	_data.resize(count);

	// Iterate through reading the data for each entry
	for (uint idx = 0; idx < count; ++idx)
		_data[idx].load(s);
}

void CBaseStars::loadData(const CString &resName) {
	// Get a stream to read the data from the DAT file
	Common::SeekableReadStream *stream = g_vm->_filesManager->getResource(resName);
	assert(stream);

	// Load the stream
	loadData(*stream);
	delete stream;
}

void CBaseStars::resetEntry(CBaseStarEntry &entry) {
	entry._red = 0xFF;
	entry._green = 0xFF;
	entry._blue = 0xFF;
	entry._thickness = 0;
	entry._position._x = 0;
	entry._position._y = 0;
	entry._position._z = 0;
	for (int idx = 0; idx < 5; ++idx)
		entry._data[idx] = 0;
}

void CBaseStars::draw(CSurfaceArea *surfaceArea, CCamera *camera, CStarCloseup *closeup) {
	if (!_data.empty()) {
		switch (camera->getStarColor()) {
		case WHITE: // draw white, green, and red stars (mostly white)
			switch (surfaceArea->_bpp) {
			case 1:
				draw1(surfaceArea, camera, closeup);
				break;
			case 2:
				draw2(surfaceArea, camera, closeup);
				break;
			default:
				break;
			}
			break;

		case PINK: // draw pink stars
			switch (surfaceArea->_bpp) {
			case 1:
				draw3(surfaceArea, camera, closeup);
				break;
			case 2:
				draw4(surfaceArea, camera, closeup);
				break;
			default:
				break;
			}
			break;

		default:
			break;
		}
	}
}

void CBaseStars::draw1(CSurfaceArea *surfaceArea, CCamera *camera, CStarCloseup *closeup) {
	FPose pose = camera->getPose();
	camera->getRelativeXCenterPixels(&_value1, &_value2, &_value3, &_value4);

	const double MAX_VAL = 1.0e9 * 1.0e9;
	FPoint centroid = surfaceArea->_centroid + FPoint(0.5, 0.5);
	double threshold = camera->getFrontClip();
	double minVal = threshold - 9216.0;
	int width1 = surfaceArea->_width - 1;
	int height1 = surfaceArea->_height - 1;
	double *v1Ptr = &_value1, *v2Ptr = &_value2;
	double tempX, tempY, tempZ, total2;

	for (uint idx = 0; idx < _data.size(); ++idx) {
		CBaseStarEntry &entry = _data[idx];
		const FVector &vector = entry._position;
		tempZ = vector._x * pose._row1._z + vector._y * pose._row2._z
			+ vector._z * pose._row3._z + pose._vector._z;
		if (tempZ <= minVal)
			continue;

		tempY = vector._x * pose._row1._y + vector._y * pose._row2._y + vector._z * pose._row3._y + pose._vector._y;
		tempX = vector._x * pose._row1._x + vector._y * pose._row2._x + vector._z * pose._row3._x + pose._vector._x;
		total2 = tempY * tempY + tempX * tempX + tempZ * tempZ; 

		if (total2 < 1.0e12) {
			closeup->draw(pose, vector, FVector(centroid._x, centroid._y, total2),
				surfaceArea, camera);
			continue;
		}

		if (tempZ <= threshold || total2 >= MAX_VAL)
			continue;

		int xStart = (int)(*v1Ptr * tempX / tempZ + centroid._x);
		int yStart = (int)(*v2Ptr * tempY / tempZ + centroid._y);
		if (xStart < 0 || xStart >= width1 || yStart < 0 || yStart >= height1)
			continue;

		double sVal = sqrt(total2);
		sVal = (sVal < 100000.0) ? 1.0 : 1.0 - ((sVal - 100000.0) / 1.0e9);
		double red = MIN((double)entry._red * sVal, (double)255.0);
		double green = MIN((double)entry._green * sVal, (double)255.0);
		double blue = MIN((double)entry._green * sVal, (double)255.0);

		int skipCtr = 0;
		if (red < 0.0) {
			red = 0.0;
			++skipCtr;
		}
		if (green < 0.0) {
			green = 0.0;
			++skipCtr;
		}
		if (blue < 0.0) {
			blue = 0.0;
			++skipCtr;
		}
		if (skipCtr == 3)
			continue;

		int r = (int)(red - 0.5) & 0xfff8;
		int g = (int)(green - 0.5) & 0xfff8;
		int b = (int)(blue - 0.5) & 0xfff8;
		int rgb = ((g | (r << 5)) << 2) | ((b >> 3) & 0xfff8);
		uint16 *pixelP = (uint16 *)(surfaceArea->_pixelsPtr + surfaceArea->_pitch * yStart + xStart * 2);

		switch (entry._thickness) {
		case 0:
			*pixelP = rgb;
			break;

		case 1:
			*pixelP = rgb;
			*(pixelP + 1) = rgb;
			*(pixelP + surfaceArea->_pitch / 2) = rgb;
			*(pixelP + surfaceArea->_pitch / 2 + 1) = rgb;
			break;

		default:
			break;
		}
	}
}

void CBaseStars::draw2(CSurfaceArea *surfaceArea, CCamera *camera, CStarCloseup *closeup) {
	FPose pose = camera->getPose();
	camera->getRelativeXCenterPixels(&_value1, &_value2, &_value3, &_value4);

	const double MAX_VAL = 1.0e9 * 1.0e9;
	FPoint centroid = surfaceArea->_centroid + FPoint(0.5, 0.5);
	double threshold = camera->getFrontClip();
	double minVal = threshold - 9216.0;
	int width1 = surfaceArea->_width - 1;
	int height1 = surfaceArea->_height - 1;
	double *v1Ptr = &_value1, *v2Ptr = &_value2;
	double tempX, tempY, tempZ, total2;

	for (uint idx = 0; idx < _data.size(); ++idx) {
		CBaseStarEntry &entry = _data[idx];
		const FVector &vector = entry._position;
		tempZ = vector._x * pose._row1._z + vector._y * pose._row2._z
			+ vector._z * pose._row3._z + pose._vector._z;
		if (tempZ <= minVal)
			continue;

		tempY = vector._x * pose._row1._y + vector._y * pose._row2._y + vector._z * pose._row3._y + pose._vector._y;
		tempX = vector._x * pose._row1._x + vector._y * pose._row2._x + vector._z * pose._row3._x + pose._vector._x;
		total2 = tempY * tempY + tempX * tempX + tempZ * tempZ;

		if (total2 < 1.0e12) {
			closeup->draw(pose, vector, FVector(centroid._x, centroid._y, total2),
				surfaceArea, camera);
			continue;
		}

		if (tempZ <= threshold || total2 >= MAX_VAL)
			continue;

		int xStart = (int)(*v1Ptr * tempX / tempZ + centroid._x);
		int yStart = (int)(*v2Ptr * tempY / tempZ + centroid._y);
		if (xStart < 0 || xStart >= width1 || yStart < 0 || yStart >= height1)
			continue;

		double sVal = sqrt(total2);
		sVal = (sVal < 100000.0) ? 1.0 : 1.0 - ((sVal - 100000.0) / 1.0e9);
		double red = MIN((double)entry._red * sVal, (double)255.0);
		double green = MIN((double)entry._green * sVal, (double)255.0);
		double blue = MIN((double)entry._green * sVal, (double)255.0);

		int skipCtr = 0;
		if (red < 0.0) {
			red = 0.0;
			++skipCtr;
		}
		if (green < 0.0) {
			green = 0.0;
			++skipCtr;
		}
		if (blue < 0.0) {
			blue = 0.0;
			++skipCtr;
		}
		if (skipCtr == 3)
			continue;

		int r = (int)(red - 0.5) & 0xf8;
		int g = (int)(green - 0.5) & 0xfc;
		int b = (int)(blue - 0.5) & 0xfff8;

		int rgb = ((g | (r << 5)) << 3) | (b >> 3);
		uint16 *pixelP = (uint16 *)(surfaceArea->_pixelsPtr + surfaceArea->_pitch * yStart + xStart * 2);

		switch (entry._thickness) {
		case 0:
			*pixelP = rgb;
			break;

		case 1:
			*pixelP = rgb;
			*(pixelP + 1) = rgb;
			*(pixelP + surfaceArea->_pitch / 2) = rgb;
			*(pixelP + surfaceArea->_pitch / 2 + 1) = rgb;
			break;

		default:
			break;
		}
	}
}

void CBaseStars::draw3(CSurfaceArea *surfaceArea, CCamera *camera, CStarCloseup *closeup) {
	FPose pose = camera->getPose();
	camera->getRelativeXCenterPixels(&_value1, &_value2, &_value3, &_value4);

	const double MAX_VAL = 1.0e9 * 1.0e9;
	FPoint centroid = surfaceArea->_centroid + FPoint(0.5, 0.5);
	double threshold = camera->getFrontClip();
	double minVal = threshold - 9216.0;
	int width1 = surfaceArea->_width - 1;
	int height1 = surfaceArea->_height - 1;
	double *v1Ptr = &_value1, *v2Ptr = &_value2;
	double *v3Ptr = &_value3, *v4Ptr = &_value4;
	double tempX, tempY, tempZ, total2, sVal;
	int xStart, yStart, rgb;
	uint16 *pixelP;

	for (uint idx = 0; idx < _data.size(); ++idx) {
		CBaseStarEntry &entry = _data[idx];
		const FVector &vector = entry._position;
		tempZ = vector._x * pose._row1._z + vector._y * pose._row2._z
			+ vector._z * pose._row3._z + pose._vector._z;
		if (tempZ <= minVal)
			continue;

		tempY = vector._x * pose._row1._y + vector._y * pose._row2._y + vector._z * pose._row3._y + pose._vector._y;
		tempX = vector._x * pose._row1._x + vector._y * pose._row2._x + vector._z * pose._row3._x + pose._vector._x;
		total2 = tempY * tempY + tempX * tempX + tempZ * tempZ;

		if (total2 < 1.0e12) {
			closeup->draw(pose, vector, FVector(centroid._x, centroid._y, total2),
				surfaceArea, camera);
			continue;
		}

		if (tempZ <= threshold || total2 >= MAX_VAL)
			continue;

		// First pixel
		xStart = (int)((tempX + *v3Ptr) * *v1Ptr / tempZ + centroid._x);
		yStart = (int)(tempY * *v2Ptr / tempZ + centroid._y);
		if (xStart < 0 || xStart >= width1 || yStart < 0 || yStart >= height1)
			continue;

		sVal = sqrt(total2);
		sVal = (sVal < 100000.0) ? 1.0 : 1.0 - ((sVal - 100000.0) / 1.0e9);
		sVal *= 255.0;

		if (sVal > 255.0)
			sVal = 255.0;

		if (sVal > 2.0) {
			pixelP = (uint16 *)(surfaceArea->_pixelsPtr + surfaceArea->_pitch * yStart + xStart * 2);
			rgb = ((int)(sVal - 0.5) & 0xf8) << 7;

			switch (entry._thickness) {
			case 0:
				*pixelP = rgb;
				break;

			case 1:
				*pixelP = rgb;
				*(pixelP + 1) = rgb;
				*(pixelP + surfaceArea->_pitch / 2) = rgb;
				*(pixelP + surfaceArea->_pitch / 2 + 1) = rgb;
				break;

			default:
				break;
			}
		}

		// Second pixel
		xStart = (int)((tempX + *v4Ptr) * *v1Ptr / tempZ + centroid._x);
		yStart = (int)(tempY * *v2Ptr / tempZ + centroid._y);
		if (xStart < 0 || xStart >= width1 || yStart < 0 || yStart >= height1)
			continue;

		sVal = sqrt(total2);
		sVal = (sVal < 100000.0) ? 1.0 : 1.0 - ((sVal - 100000.0) / 1.0e9);
		sVal *= 255.0;

		if (sVal > 255.0)
			sVal = 255.0;

		if (sVal > 2.0) {
			pixelP = (uint16 *)(surfaceArea->_pixelsPtr + surfaceArea->_pitch * yStart + xStart * 2);
			rgb = ((int)(sVal - 0.5) & 0xf8) << 7;

			switch (entry._thickness) {
			case 0:
				*pixelP |= rgb;
				break;

			case 1:
				*pixelP |= rgb;
				*(pixelP + 1) |= rgb;
				*(pixelP + surfaceArea->_pitch / 2) |= rgb;
				*(pixelP + surfaceArea->_pitch / 2 + 1) |= rgb;
				break;

			default:
				break;
			}
		}
	}
}

void CBaseStars::draw4(CSurfaceArea *surfaceArea, CCamera *camera, CStarCloseup *closeup) {
	FPose pose = camera->getPose();
	camera->getRelativeXCenterPixels(&_value1, &_value2, &_value3, &_value4);

	const double MAX_VAL = 1.0e9 * 1.0e9;
	FPoint centroid = surfaceArea->_centroid + FPoint(0.5, 0.5);
	double threshold = camera->getFrontClip();
	double minVal = threshold - 9216.0;
	int width1 = surfaceArea->_width - 1;
	int height1 = surfaceArea->_height - 1;
	double *v1Ptr = &_value1, *v2Ptr = &_value2, *v3Ptr = &_value3, *v4Ptr = &_value4;
	double tempX, tempY, tempZ, total2, sVal;
	int xStart, yStart, rgb;
	uint16 *pixelP;

	for (uint idx = 0; idx < _data.size(); ++idx) {
		const CBaseStarEntry &entry = _data[idx];
		const FVector &vector = entry._position;

		tempZ = vector._x * pose._row1._z + vector._y * pose._row2._z
			+ vector._z * pose._row3._z + pose._vector._z;
		if (tempZ <= minVal)
			continue;

		tempY = vector._x * pose._row1._y + vector._y * pose._row2._y + vector._z * pose._row3._y + pose._vector._y;
		tempX = vector._x * pose._row1._x + vector._y * pose._row2._x + vector._z * pose._row3._x + pose._vector._x;
		total2 = tempY * tempY + tempX * tempX + tempZ * tempZ;

		if (total2 < 1.0e12) {
			// We're in close proximity to the given star, so draw a closeup of it
			closeup->draw(pose, vector, FVector(centroid._x, centroid._y, total2),
				surfaceArea, camera);
			continue;
		}

		if (tempZ <= threshold || total2 >= MAX_VAL)
			continue;

		// First pixel
		xStart = (int)((tempX + *v3Ptr) * *v1Ptr / tempZ + centroid._x);
		yStart = (int)(tempY * *v2Ptr / tempZ + centroid._y);
		if (xStart < 0 || xStart >= width1 || yStart < 0 || yStart >= height1)
			continue;

		sVal = sqrt(total2);
		sVal = (sVal < 100000.0) ? 1.0 : 1.0 - ((sVal - 100000.0) / 1.0e9);
		sVal *= 255.0;

		if (sVal > 255.0)
			sVal = 255.0;

		if (sVal > 2.0) {
			pixelP = (uint16 *)(surfaceArea->_pixelsPtr + surfaceArea->_pitch * yStart + xStart * 2);
			rgb = ((int)(sVal - 0.5) & 0xf8) << 8;

			switch (entry._thickness) {
			case 0:
				*pixelP = rgb;
				break;

			case 1:
				*pixelP = rgb;
				*(pixelP + 1) = rgb;
				*(pixelP + surfaceArea->_pitch / 2) = rgb;
				*(pixelP + surfaceArea->_pitch / 2 + 1) = rgb;
				break;

			default:
				break;
			}
		}

		// Second pixel
		xStart = (int)((tempX + *v4Ptr) * *v1Ptr / tempZ + centroid._x);
		yStart = (int)((tempY * *v2Ptr) / tempZ + centroid._y);
		if (xStart < 0 || xStart >= width1 || yStart < 0 || yStart >= height1)
			continue;

		sVal = sqrt(total2);
		sVal = (sVal < 100000.0) ? 1.0 : 1.0 - ((sVal - 100000.0) / 1.0e9);
		sVal *= 255.0;

		if (sVal > 255.0)
			sVal = 255.0;

		if (sVal > 2.0) {
			pixelP = (uint16 *)(surfaceArea->_pixelsPtr + surfaceArea->_pitch * yStart + xStart * 2);
			rgb = ((int)(sVal - 0.5) >> 3) & 0xff;

			switch (entry._thickness) {
			case 0:
				*pixelP |= rgb;
				break;

			case 1:
				*pixelP |= rgb;
				*(pixelP + 1) |= rgb;
				*(pixelP + surfaceArea->_pitch / 2) |= rgb;
				*(pixelP + surfaceArea->_pitch / 2 + 1) |= rgb;
				break;

			default:
				break;
			}
		}
	}
}

int CBaseStars::findStar(CSurfaceArea *surfaceArea, CCamera *camera,
		const Common::Point &pt) {
	CStarRef1 ref(this, pt);
	ref.process(surfaceArea, camera);
	return ref._index;
}

int CBaseStars::baseFn2(CSurfaceArea *surfaceArea, CCamera *camera) {
	CStarRef3 ref(this);
	ref.process(surfaceArea, camera);
	return ref._index;
}

} // End of namespace Titanic
