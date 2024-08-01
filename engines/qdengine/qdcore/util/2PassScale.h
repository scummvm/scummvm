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

#ifndef QDENGINE_QDCORE_UTIL_2_PASS_SCALE_H
#define QDENGINE_QDCORE_UTIL_2_PASS_SCALE_H

#include "qdengine/qdcore/util/Filters.h"

#define TRACE(a)
#define ASSERT(a)

namespace QDEngine {

namespace scl {

typedef struct {
	double *Weights;  // Normalized weights of neighboring pixels
	int Left, Right;  // Bounds of source pixels window
} ContributionType;   // Contirbution information for a single pixel

typedef struct {
	ContributionType *ContribRow; // Row (or column) of contribution weights
	uint32 WindowSize,              // Filter window size (of affecting source pixels)
	     LineLength;           // Length of line (no. or rows / cols)
} LineContribType;                    // Contribution information for an entire line (row or column)

typedef BOOL (*ProgressAnbAbortCallBack)(byte bPercentComplete);

template <class FilterClass>
class C2PassScale {
public:

	C2PassScale() : temp_buffer_(65536, 0), weights_buffer_(16384, 0.0), contribution_buffer_(500) { }
	virtual ~C2PassScale() { }

	COLORREF *Scale(COLORREF *pOrigImage, uint32 uOrigWidth, uint32 uOrigHeight, COLORREF *pDstImage, uint32 uNewWidth, uint32 uNewHeight);

private:


	std::vector<uint32> temp_buffer_;

	std::vector<double> weights_buffer_;
	std::vector<ContributionType> contribution_buffer_;

	LineContribType *AllocContributions(uint32 uLineLength, uint32 uWindowSize);
	LineContribType *CalcContributions(uint32 uLineSize, uint32 uSrcSize, double dScale);

	void ScaleRow(COLORREF *pSrc, uint32 uSrcWidth, COLORREF *pRes, uint32 uResWidth, uint32 uRow, LineContribType *Contrib);
	void HorizScale(COLORREF *pSrc, uint32 uSrcWidth, uint32 uSrcHeight, COLORREF *pDst, uint32 uResWidth, uint32 uResHeight);
	void ScaleCol(COLORREF *pSrc, uint32 uSrcWidth, COLORREF *pRes, uint32 uResWidth, uint32 uResHeight, uint32 uCol, LineContribType *Contrib);
	void VertScale(COLORREF *pSrc, uint32 uSrcWidth, uint32 uSrcHeight, COLORREF *pDst, uint32 uResWidth, uint32 uResHeight);

	static inline byte make_r(COLORREF col) {
		return reinterpret_cast<byte *>(&col)[2];
	}
	static inline byte make_g(COLORREF col) {
		return reinterpret_cast<byte *>(&col)[1];
	}
	static inline byte make_b(COLORREF col) {
		return reinterpret_cast<byte *>(&col)[0];
	}
	static inline byte make_a(COLORREF col) {
		return reinterpret_cast<byte *>(&col)[3];
	}

	static inline COLORREF make_rgba(byte r, byte g, byte b, byte a) {
		return (r << 16) | (g << 8) | (b << 0) | (a << 24);
	}
};

template<class FilterClass>
LineContribType *C2PassScale<FilterClass>::AllocContributions(uint32 uLineLength, uint32 uWindowSize) {
	static LineContribType line_ct;

	line_ct.WindowSize = uWindowSize;
	line_ct.LineLength = uLineLength;

	if (contribution_buffer_.size() < uLineLength)
		contribution_buffer_.resize(uLineLength);

	line_ct.ContribRow = &*contribution_buffer_.begin();

	if (weights_buffer_.size() < uLineLength * uWindowSize)
		weights_buffer_.resize(uLineLength * uWindowSize);

	double *p = &*weights_buffer_.begin();

	for (uint32 u = 0; u < uLineLength; u++) {
		line_ct.ContribRow[u].Weights = p;
		p += uWindowSize;
	}
	return &line_ct;
}

template <class FilterClass>
LineContribType *C2PassScale<FilterClass>::CalcContributions(uint32 uLineSize, uint32 uSrcSize, double dScale) {
	FilterClass CurFilter;

	double dWidth;
	double dFScale = 1.0;
	double dFilterWidth = CurFilter.GetWidth();

	if (dScale < 1.0) { // Minification
		dWidth = dFilterWidth / dScale;
		dFScale = dScale;
	} else { // Magnification
		dWidth = dFilterWidth;
	}

	// Window size is the number of sampled pixels
	int iWindowSize = 2 * (int)ceil(dWidth) + 1;

	// Allocate a new line contributions strucutre
	LineContribType *res = AllocContributions(uLineSize, iWindowSize);

	for (uint32 u = 0; u < uLineSize; u++) {
		// Scan through line of contributions
		double dCenter = (double)u / dScale;   // Reverse mapping
		// Find the significant edge points that affect the pixel
		int iLeft = MAX(0, (int)floor(dCenter - dWidth));
		int iRight = MIN((int)ceil(dCenter + dWidth), int(uSrcSize) - 1);

		// Cut edge points to fit in filter window in case of spill-off
		if (iRight - iLeft + 1 > iWindowSize) {
			if (iLeft < (int(uSrcSize) - 1 / 2)) {
				iLeft++;
			} else {
				iRight--;
			}
		}

		res->ContribRow[u].Left = iLeft;
		res->ContribRow[u].Right = iRight;

		double dTotalWeight = 0.0;  // Zero sum of weights
		for (int iSrc = iLeft; iSrc <= iRight; iSrc++) {
			// Calculate weights
			dTotalWeight += (res->ContribRow[u].Weights[iSrc - iLeft] = dFScale * CurFilter.Filter(dFScale * (dCenter - (double)iSrc)));
		}
		ASSERT(dTotalWeight >= 0.0);   // An error in the filter function can cause this
		if (dTotalWeight > 0.0) {
			// Normalize weight of neighbouring points
			for (int iSrc = iLeft; iSrc <= iRight; iSrc++) {
				// Normalize point
				res->ContribRow[u].Weights[iSrc - iLeft] /= dTotalWeight;
			}
		}
	}
	return res;
}

template <class FilterClass>
void C2PassScale<FilterClass>::ScaleRow(COLORREF *pSrc, uint32 uSrcWidth, COLORREF *pRes, uint32 uResWidth, uint32 uRow, LineContribType *Contrib) {
	COLORREF *pSrcRow = &(pSrc[uRow * uSrcWidth]);
	COLORREF *pDstRow = &(pRes[uRow * uResWidth]);
	for (uint32 x = 0; x < uResWidth; x++) {
		// Loop through row
		double dr = 0.0;
		double dg = 0.0;
		double db = 0.0;
		double da = 0.0;

		int iLeft = Contrib->ContribRow[x].Left;    // Retrieve left boundries
		int iRight = Contrib->ContribRow[x].Right;  // Retrieve right boundries
		for (int i = iLeft; i <= iRight; i++) {
			// Scan between boundries
			// Accumulate weighted effect of each neighboring pixel
			dr += Contrib->ContribRow[x].Weights[i - iLeft] * (double)(make_r(pSrcRow[i]));
			dg += Contrib->ContribRow[x].Weights[i - iLeft] * (double)(make_g(pSrcRow[i]));
			db += Contrib->ContribRow[x].Weights[i - iLeft] * (double)(make_b(pSrcRow[i]));
			da += Contrib->ContribRow[x].Weights[i - iLeft] * (double)(make_a(pSrcRow[i]));
		}

		uint32 r = round(dr);
		uint32 g = round(dg);
		uint32 b = round(db);
		uint32 a = round(da);

		pDstRow[x] = make_rgba(r, g, b, a); // Place result in destination pixel
	}
}

template <class FilterClass>
void C2PassScale<FilterClass>::HorizScale(COLORREF *pSrc, uint32 uSrcWidth, uint32 uSrcHeight, COLORREF *pDst, uint32 uResWidth, uint32 uResHeight) {
	TRACE("Performing horizontal scaling...\n");
	if (uResWidth == uSrcWidth) {
		// No scaling required, just copy
		memcpy(pDst, pSrc, sizeof(COLORREF) * uSrcHeight * uSrcWidth);
		return;
	}
	// Allocate and calculate the contributions
	LineContribType *Contrib = CalcContributions(uResWidth, uSrcWidth, double(uResWidth) / double(uSrcWidth));
	for (uint32 u = 0; u < uResHeight; u++)
		ScaleRow(pSrc, uSrcWidth, pDst, uResWidth, u, Contrib); // Scale each row
}

template <class FilterClass>
void C2PassScale<FilterClass>::ScaleCol(COLORREF *pSrc, uint32 uSrcWidth, COLORREF *pRes, uint32 uResWidth, uint32 uResHeight, uint32 uCol, LineContribType *Contrib) {
	for (uint32 y = 0; y < uResHeight; y++) {
		// Loop through column
		double dr = 0.0;
		double dg = 0.0;
		double db = 0.0;
		double da = 0.0;

		int iLeft = Contrib->ContribRow[y].Left;    // Retrieve left boundries
		int iRight = Contrib->ContribRow[y].Right;  // Retrieve right boundries
		for (int i = iLeft; i <= iRight; i++) {
			// Scan between boundries
			// Accumulate weighted effect of each neighboring pixel
			COLORREF pCurSrc = pSrc[i * uSrcWidth + uCol];
			dr += Contrib->ContribRow[y].Weights[i - iLeft] * (double)(make_r(pCurSrc));
			dg += Contrib->ContribRow[y].Weights[i - iLeft] * (double)(make_g(pCurSrc));
			db += Contrib->ContribRow[y].Weights[i - iLeft] * (double)(make_b(pCurSrc));
			da += Contrib->ContribRow[y].Weights[i - iLeft] * (double)(make_a(pCurSrc));
		}

		uint32 r = round(dr);
		uint32 g = round(dg);
		uint32 b = round(db);
		uint32 a = round(da);

		pRes[y * uResWidth + uCol] = make_rgba(r, g, b, a); // Place result in destination pixel
	}
}

template <class FilterClass>
void C2PassScale<FilterClass>::VertScale(COLORREF *pSrc, uint32 uSrcWidth, uint32 uSrcHeight, COLORREF *pDst, uint32 uResWidth, uint32 uResHeight) {
	TRACE("Performing vertical scaling...");

	if (uSrcHeight == uResHeight) {
		// No scaling required, just copy
		memcpy(pDst, pSrc, sizeof(COLORREF) * uSrcHeight * uSrcWidth);
		return;
	}
	// Allocate and calculate the contributions
	LineContribType *Contrib = CalcContributions(uResHeight, uSrcHeight, double(uResHeight) / double(uSrcHeight));
	for (uint32 u = 0; u < uResWidth; u++)
		ScaleCol(pSrc, uSrcWidth, pDst, uResWidth, uResHeight, u, Contrib); // Scale each column
}

template <class FilterClass>
COLORREF *C2PassScale<FilterClass>::Scale(COLORREF *pOrigImage, uint32 uOrigWidth, uint32 uOrigHeight, COLORREF *pDstImage, uint32 uNewWidth, uint32 uNewHeight) {
	if (temp_buffer_.size() < uNewWidth * uOrigHeight)
		temp_buffer_.resize(uNewWidth * uOrigHeight);

	COLORREF *pTemp = reinterpret_cast<COLORREF *>(&*temp_buffer_.begin());
	HorizScale(pOrigImage, uOrigWidth, uOrigHeight, pTemp, uNewWidth, uOrigHeight);

	// Scale temporary image vertically into result image
	VertScale(pTemp, uNewWidth, uOrigHeight, pDstImage, uNewWidth, uNewHeight);

	return pDstImage;
}

}; // namespace scl

} // namespace QDEngine

#endif // QDENGINE_QDCORE_UTIL_2_PASS_SCALE_H
