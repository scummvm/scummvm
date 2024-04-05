#ifndef _2_PASS_SCALE_H_
#define _2_PASS_SCALE_H_

#include <math.h> 

#include <vector>

#define TRACE(a)
#define ASSERT(a)

#include "Filters.h" 

namespace scl {

typedef struct 
{ 
	double *Weights;  // Normalized weights of neighboring pixels
	int Left,Right;   // Bounds of source pixels window
} ContributionType;	  // Contirbution information for a single pixel

typedef struct 
{ 
	ContributionType *ContribRow; // Row (or column) of contribution weights 
	UINT WindowSize,              // Filter window size (of affecting source pixels) 
        LineLength;		      // Length of line (no. or rows / cols) 
} LineContribType;                    // Contribution information for an entire line (row or column)

typedef BOOL (*ProgressAnbAbortCallBack)(BYTE bPercentComplete);

template <class FilterClass>
class C2PassScale 
{
public:

	C2PassScale() : temp_buffer_(65536,0), weights_buffer_(16384,0.0), contribution_buffer_(500) { }
	virtual ~C2PassScale(){ }

	COLORREF* Scale(COLORREF* pOrigImage,UINT uOrigWidth,UINT uOrigHeight,COLORREF* pDstImage,UINT uNewWidth,UINT uNewHeight);

private:


	std::vector<unsigned> temp_buffer_;

	std::vector<double> weights_buffer_;
	std::vector<ContributionType> contribution_buffer_;

	LineContribType* AllocContributions(UINT uLineLength,UINT uWindowSize);
	LineContribType* CalcContributions (UINT uLineSize,UINT uSrcSize,double dScale);

	void ScaleRow(COLORREF* pSrc,UINT uSrcWidth,COLORREF* pRes,UINT uResWidth,UINT uRow,LineContribType* Contrib);
	void HorizScale(COLORREF* pSrc,UINT uSrcWidth,UINT uSrcHeight,COLORREF* pDst,UINT uResWidth,UINT uResHeight);
	void ScaleCol(COLORREF* pSrc,UINT uSrcWidth,COLORREF* pRes,UINT uResWidth,UINT uResHeight,UINT uCol,LineContribType* Contrib);
	void VertScale(COLORREF* pSrc,UINT uSrcWidth,UINT uSrcHeight,COLORREF* pDst,UINT uResWidth,UINT uResHeight);

	static inline BYTE make_r(COLORREF col){ return reinterpret_cast<BYTE*>(&col)[2]; }
	static inline BYTE make_g(COLORREF col){ return reinterpret_cast<BYTE*>(&col)[1]; }
	static inline BYTE make_b(COLORREF col){ return reinterpret_cast<BYTE*>(&col)[0]; }
	static inline BYTE make_a(COLORREF col){ return reinterpret_cast<BYTE*>(&col)[3]; }

	static inline COLORREF make_rgba(BYTE r,BYTE g,BYTE b,BYTE a){ 
		return (r << 16) | (g << 8) | (b << 0) | (a << 24);
	}

	static __forceinline int round(double x){
		int a;
		_asm {
			fld x
			fistp dword ptr a
		}
		return a;
	}
	static __forceinline int round(float x){
		int a;
		_asm {
			fld x
			fistp dword ptr a
		}
		return a;
	}
};

template<class FilterClass>
LineContribType* C2PassScale<FilterClass>::AllocContributions(UINT uLineLength, UINT uWindowSize)
{
	static LineContribType line_ct;

	LineContribType *res = new LineContribType; 

	line_ct.WindowSize = uWindowSize; 
	line_ct.LineLength = uLineLength; 

	if(contribution_buffer_.size() < uLineLength)
		contribution_buffer_.resize(uLineLength);

	line_ct.ContribRow = &*contribution_buffer_.begin();

	if(weights_buffer_.size() < uLineLength * uWindowSize)
		weights_buffer_.resize(uLineLength * uWindowSize);

	double* p = &*weights_buffer_.begin();

	for(UINT u = 0; u < uLineLength; u++){
		line_ct.ContribRow[u].Weights = p;
		p += uWindowSize;
	}
	return &line_ct;
} 
 
template <class FilterClass>
LineContribType* C2PassScale<FilterClass>::CalcContributions(UINT uLineSize, UINT uSrcSize, double dScale)
{ 
	FilterClass CurFilter;

	double dWidth;
	double dFScale = 1.0;
	double dFilterWidth = CurFilter.GetWidth();

	if(dScale < 1.0){ // Minification
		dWidth = dFilterWidth / dScale; 
		dFScale = dScale; 
	} 
	else { // Magnification
		dWidth= dFilterWidth; 
	}
 
	// Window size is the number of sampled pixels
	int iWindowSize = 2 * (int)ceil(dWidth) + 1; 
 
	// Allocate a new line contributions strucutre
	LineContribType* res = AllocContributions(uLineSize,iWindowSize); 
 
	for(UINT u = 0; u < uLineSize; u++){   
		// Scan through line of contributions
		double dCenter = (double)u / dScale;   // Reverse mapping
		// Find the significant edge points that affect the pixel
		int iLeft = max(0,(int)floor(dCenter - dWidth)); 
		int iRight = min((int)ceil(dCenter + dWidth),int(uSrcSize) - 1); 

		// Cut edge points to fit in filter window in case of spill-off
		if(iRight - iLeft + 1 > iWindowSize){
			if(iLeft < (int(uSrcSize) - 1 / 2)){
				iLeft++; 
			}
			else {
				iRight--; 
			}
		}

		res->ContribRow[u].Left = iLeft; 
		res->ContribRow[u].Right = iRight;

		double dTotalWeight = 0.0;  // Zero sum of weights
		for(int iSrc = iLeft; iSrc <= iRight; iSrc++){   
			// Calculate weights
			dTotalWeight += (res->ContribRow[u].Weights[iSrc-iLeft] = dFScale * CurFilter.Filter (dFScale * (dCenter - (double)iSrc))); 
		}
		ASSERT(dTotalWeight >= 0.0);   // An error in the filter function can cause this 
		if(dTotalWeight > 0.0){
			// Normalize weight of neighbouring points
			for(int iSrc = iLeft; iSrc <= iRight; iSrc++){   
				// Normalize point
				res->ContribRow[u].Weights[iSrc-iLeft] /= dTotalWeight; 
			}
		}
	} 
	return res; 
} 
 
template <class FilterClass>
void C2PassScale<FilterClass>::ScaleRow(COLORREF* pSrc,UINT uSrcWidth,COLORREF* pRes,UINT uResWidth,UINT uRow,LineContribType* Contrib)
{
	COLORREF *pSrcRow = &(pSrc[uRow * uSrcWidth]);
	COLORREF *pDstRow = &(pRes[uRow * uResWidth]);
	for(UINT x = 0; x < uResWidth; x++){ 
		// Loop through row
		double dr = 0.0;
		double dg = 0.0;
		double db = 0.0;
		double da = 0.0;

		int iLeft = Contrib->ContribRow[x].Left;    // Retrieve left boundries
		int iRight = Contrib->ContribRow[x].Right;  // Retrieve right boundries
		for (int i = iLeft; i <= iRight; i++){   
			// Scan between boundries
			// Accumulate weighted effect of each neighboring pixel
			dr += Contrib->ContribRow[x].Weights[i-iLeft] * (double)(make_r(pSrcRow[i]));
			dg += Contrib->ContribRow[x].Weights[i-iLeft] * (double)(make_g(pSrcRow[i]));
			db += Contrib->ContribRow[x].Weights[i-iLeft] * (double)(make_b(pSrcRow[i]));
			da += Contrib->ContribRow[x].Weights[i-iLeft] * (double)(make_a(pSrcRow[i]));
		}

		unsigned r = round(dr);
		unsigned g = round(dg);
		unsigned b = round(db);
		unsigned a = round(da);

		pDstRow[x] = make_rgba(r,g,b,a); // Place result in destination pixel
	} 
} 

template <class FilterClass>
void C2PassScale<FilterClass>::HorizScale(COLORREF* pSrc,UINT uSrcWidth,UINT uSrcHeight,COLORREF* pDst,UINT uResWidth,UINT uResHeight)
{ 
	TRACE("Performing horizontal scaling...\n"); 
	if(uResWidth == uSrcWidth){
		// No scaling required, just copy
		memcpy(pDst,pSrc,sizeof(COLORREF) * uSrcHeight * uSrcWidth);
		return;
	}
	// Allocate and calculate the contributions
	LineContribType* Contrib = CalcContributions(uResWidth,uSrcWidth,double(uResWidth) / double(uSrcWidth));
	for(UINT u = 0; u < uResHeight; u++)
		ScaleRow(pSrc,uSrcWidth,pDst,uResWidth,u,Contrib);    // Scale each row 
} 
 
template <class FilterClass>
void C2PassScale<FilterClass>::ScaleCol(COLORREF* pSrc,UINT uSrcWidth,COLORREF* pRes,UINT uResWidth,UINT uResHeight,UINT uCol,LineContribType* Contrib)
{ 
	for(UINT y = 0; y < uResHeight; y++){    
		// Loop through column
		double dr = 0.0;
		double dg = 0.0;
		double db = 0.0;
		double da = 0.0;

		int iLeft = Contrib->ContribRow[y].Left;    // Retrieve left boundries
		int iRight = Contrib->ContribRow[y].Right;  // Retrieve right boundries
		for(int i = iLeft; i <= iRight; i++){   
			// Scan between boundries
			// Accumulate weighted effect of each neighboring pixel
			COLORREF pCurSrc = pSrc[i * uSrcWidth + uCol];
			dr += Contrib->ContribRow[y].Weights[i-iLeft] * (double)(make_r(pCurSrc));
			dg += Contrib->ContribRow[y].Weights[i-iLeft] * (double)(make_g(pCurSrc));
			db += Contrib->ContribRow[y].Weights[i-iLeft] * (double)(make_b(pCurSrc));
			da += Contrib->ContribRow[y].Weights[i-iLeft] * (double)(make_a(pCurSrc));
		}

		unsigned r = round(dr);
		unsigned g = round(dg);
		unsigned b = round(db);
		unsigned a = round(da);

		pRes[y * uResWidth + uCol] = make_rgba(r,g,b,a);   // Place result in destination pixel
	}
}

template <class FilterClass>
void C2PassScale<FilterClass>::VertScale(COLORREF* pSrc,UINT uSrcWidth,UINT uSrcHeight,COLORREF* pDst,UINT uResWidth,UINT uResHeight)
{ 
	TRACE ("Performing vertical scaling..."); 

	if(uSrcHeight == uResHeight){
		// No scaling required, just copy
		memcpy(pDst,pSrc,sizeof(COLORREF) * uSrcHeight * uSrcWidth);
		return;
	}
	// Allocate and calculate the contributions
	LineContribType* Contrib = CalcContributions(uResHeight,uSrcHeight,double(uResHeight) / double(uSrcHeight)); 
	for(UINT u = 0; u < uResWidth; u++)
		ScaleCol(pSrc,uSrcWidth,pDst,uResWidth,uResHeight,u,Contrib);	// Scale each column
} 

template <class FilterClass>
COLORREF* C2PassScale<FilterClass>::Scale(COLORREF* pOrigImage,UINT uOrigWidth,UINT uOrigHeight,COLORREF* pDstImage,UINT uNewWidth,UINT uNewHeight)
{ 
	if(temp_buffer_.size() < uNewWidth * uOrigHeight)
		temp_buffer_.resize(uNewWidth * uOrigHeight);

	COLORREF* pTemp = reinterpret_cast<COLORREF*>(&*temp_buffer_.begin());
	HorizScale(pOrigImage,uOrigWidth,uOrigHeight,pTemp,uNewWidth,uOrigHeight);

	// Scale temporary image vertically into result image    
	VertScale(pTemp,uNewWidth,uOrigHeight,pDstImage,uNewWidth,uNewHeight);

	return pDstImage;
} 

}; // namespace scl

#endif //   _2_PASS_SCALE_H_
