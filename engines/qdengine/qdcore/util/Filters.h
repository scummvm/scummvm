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

#ifndef QDENGINE_QDCORE_UTIL_FILTERS_H
#define QDENGINE_QDCORE_UTIL_FILTERS_H



namespace QDEngine {

namespace scl {

class CGenericFilter {
public:
	CGenericFilter(double dWidth) : _dWidth(dWidth) {}
	virtual ~CGenericFilter() {}

	double getWidth() const             {
		return _dWidth;
	}
	void   setWidth(double dWidth)      {
		_dWidth = dWidth;
	}

	virtual double filter(double dVal) = 0;

protected:

#define FILTER_PI  double (3.1415926535897932384626433832795)
#define FILTER_2PI double (2.0 * 3.1415926535897932384626433832795)
#define FILTER_4PI double (4.0 * 3.1415926535897932384626433832795)

	double _dWidth;
};

class CBoxFilter : public CGenericFilter {
public:
	CBoxFilter(double dWidth = double(0.5)) : CGenericFilter(dWidth) {}
	~CBoxFilter() {}

	double filter(double dVal) {
		return (fabs(dVal) <= _dWidth ? 1.0 : 0.0);
	}
};

class CBilinearFilter : public CGenericFilter {
public:
	CBilinearFilter(double dWidth = double(1.0)) : CGenericFilter(dWidth) {}
	~CBilinearFilter() {}

	double filter(double dVal) {
		dVal = fabs(dVal);
		return (dVal < _dWidth ? _dWidth - dVal : 0.0);
	}
};

class CGaussianFilter : public CGenericFilter {
public:
	CGaussianFilter(double dWidth = double(3.0)) : CGenericFilter(dWidth) {}
	~CGaussianFilter() {}

	double filter(double dVal) {
		if (fabs(dVal) > _dWidth) {
			return 0.0;
		}
		return exp(-dVal * dVal / 2.0) / sqrt(FILTER_2PI);
	}
};

class CHammingFilter : public CGenericFilter {
public:
	CHammingFilter(double dWidth = double(0.5)) : CGenericFilter(dWidth) {}
	~CHammingFilter() {}

	double filter(double dVal) {
		if (fabs(dVal) > _dWidth)
			return 0.0;

		double dWindow = 0.54 + 0.46 * cos(FILTER_2PI * dVal);
		double dSinc = (dVal == 0) ? 1.0 : sin(FILTER_PI * dVal) / (FILTER_PI * dVal);
		return dWindow * dSinc;
	}
};

class CBlackmanFilter : public CGenericFilter {
public:
	CBlackmanFilter(double dWidth = double(0.5)) : CGenericFilter(dWidth) {}
	~CBlackmanFilter() {}

	double filter(double dVal) {
		if (fabs(dVal) > _dWidth)
			return 0.0;

		double dN = 2.0 * _dWidth + 1.0;
		return 0.42 + 0.5 * cos(FILTER_2PI * dVal / (dN - 1.0)) + 0.08 * cos(FILTER_4PI * dVal / (dN - 1.0));
	}
};

} // namespace scl

} // namespace QDEngine

#endif // QDENGINE_QDCORE_UTIL_FILTERS_H
