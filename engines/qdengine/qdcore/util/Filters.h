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

#ifndef _FILTERS_H_
#define _FILTERS_H_



namespace QDEngine {

namespace scl {

class CGenericFilter {
public:
	CGenericFilter(double dWidth) : m_dWidth(dWidth) {}
	virtual ~CGenericFilter() {}

	double GetWidth() const             {
		return m_dWidth;
	}
	void   SetWidth(double dWidth)      {
		m_dWidth = dWidth;
	}

	virtual double Filter(double dVal) = 0;

protected:

#define FILTER_PI  double (3.1415926535897932384626433832795)
#define FILTER_2PI double (2.0 * 3.1415926535897932384626433832795)
#define FILTER_4PI double (4.0 * 3.1415926535897932384626433832795)

	double m_dWidth;
};

class CBoxFilter : public CGenericFilter {
public:
	CBoxFilter(double dWidth = double(0.5)) : CGenericFilter(dWidth) {}
	~CBoxFilter() {}

	double Filter(double dVal) {
		return (fabs(dVal) <= m_dWidth ? 1.0 : 0.0);
	}
};

class CBilinearFilter : public CGenericFilter {
public:
	CBilinearFilter(double dWidth = double(1.0)) : CGenericFilter(dWidth) {}
	~CBilinearFilter() {}

	double Filter(double dVal) {
		dVal = fabs(dVal);
		return (dVal < m_dWidth ? m_dWidth - dVal : 0.0);
	}
};

class CGaussianFilter : public CGenericFilter {
public:
	CGaussianFilter(double dWidth = double(3.0)) : CGenericFilter(dWidth) {}
	~CGaussianFilter() {}

	double Filter(double dVal) {
		if (fabs(dVal) > m_dWidth) {
			return 0.0;
		}
		return exp(-dVal * dVal / 2.0) / sqrt(FILTER_2PI);
	}
};

class CHammingFilter : public CGenericFilter {
public:
	CHammingFilter(double dWidth = double(0.5)) : CGenericFilter(dWidth) {}
	~CHammingFilter() {}

	double Filter(double dVal) {
		if (fabs(dVal) > m_dWidth)
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

	double Filter(double dVal) {
		if (fabs(dVal) > m_dWidth)
			return 0.0;

		double dN = 2.0 * m_dWidth + 1.0;
		return 0.42 + 0.5 * cos(FILTER_2PI * dVal / (dN - 1.0)) + 0.08 * cos(FILTER_4PI * dVal / (dN - 1.0));
	}
};

}; // namespace scl

} // namespace QDEngine

#endif  // _FILTERS_H_
