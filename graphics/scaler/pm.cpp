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
 */

#include "graphics/scaler/pm.h"
#include "graphics/scaler/intern.h"

//
// Code taken from Pablo Medina (aka "pm") (pjmedina3@yahoo.com)
// You can find it here: https://web.archive.org/web/20051018114450/http://2xpm.freeservers.com/ 
// Thanks for his great work
// Implemented and fixed for ScummVM by Johannes Schickel (aka "LordHoto") (lordhoto [at] gmail [dot] com)
//

#define interpolate_1_1(a,b)         (ColorMask::kBytesPerPixel == 2 ? interpolate16_1_1<ColorMask>(a,b) : interpolate32_1_1<ColorMask>(a,b))
#define interpolate_3_1(a,b)         (ColorMask::kBytesPerPixel == 2 ? interpolate16_3_1<ColorMask>(a,b) : interpolate32_3_1<ColorMask>(a,b))

template<typename ColorMask>
void scaleIntern(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height) {
	typedef typename ColorMask::PixelType Pixel;

	uint32 nextLineSrc = srcPitch / sizeof(Pixel);
	uint32 nextLine = dstPitch / sizeof(Pixel);
	uint32 completeLineSrc = nextLineSrc - width;
	uint32 completeLine = nextLine - (2*width);

	const Pixel *startAddr2 = (const Pixel*)srcPtr;
	const Pixel *startAddr1 = startAddr2 - nextLineSrc;
	const Pixel *startAddr3 = startAddr2 + nextLineSrc;

	Pixel *dstPixel = (Pixel*)dstPtr;
	int y = height;

	Pixel E[4] = { 0, 0, 0, 0 };

	while (y--) {
		int x = width;

		while (x--) {
			//
			// pA pB pC
			// pD pE pF
			// pG pH pI
			//
			Pixel pB = startAddr1[0];
			Pixel pE = startAddr2[0];
			Pixel pH = startAddr3[0];

			Pixel pA = startAddr1[-1];
			Pixel pD = startAddr2[-1];
			Pixel pG = startAddr3[-1];

			Pixel pC = startAddr1[1];
			Pixel pF = startAddr2[1];
			Pixel pI = startAddr3[1];

			bool doNotReblit = false;
			E[0] = E[1] = E[2] = E[3] = pE;

			if (!doNotReblit) {
				if (pD != pF) {
					if ((pE != pD) && (pD == pH) && (pD == pI) && (pE != pG)
						&& ((pD != pG) || (pE != pF) || (pA != pD))
						&& (!((pD == pA) && (pD == pG) && (pE == pB) && (pE == pF)))) {
						E[2] = pH;
						E[3] = interpolate_1_1(E[3], pH);
						doNotReblit = true;
					} else if ((pE != pF) && (pF == pH) && (pF == pG) && (pE != pI)
						&& ((pF != pI) || (pE != pD) || (pC != pF))
						&& (!((pF == pC) && (pF == pI) && (pE == pB) && (pE == pD)))) {
						E[2] = interpolate_1_1(E[2], pH);
						E[3] = pH;
						doNotReblit = true;
					}
				}

				if (pB != pH) {
					if (pE != pB) {
						if ((pA != pB) || (pB != pC) || (pE != pH)) {
							if ((pB == pD) && (pB == pG) && (pE != pA)
								&& (!((pD == pA) && (pD == pC) && (pE == pH) && (pE == pF)))) {
								E[0] = interpolate_3_1( pB,E[0]);
								E[2] = interpolate_3_1(E[2], pB);
								doNotReblit = true;
							} else if ((pB == pF) && (pB == pI) && (pE != pC)
								&& (!((pF == pC) && (pF == pA) && (pE == pH) && (pE == pD)))) {
								E[1] = interpolate_3_1(pB, E[1]);
								E[3] = interpolate_3_1(E[3], pB);
								doNotReblit = true;
							}
						}
					}

					if (pE != pH) {
						if ((pG != pH) || (pE != pB) || (pH != pI)) {
							if ((pH == pD) && (pH == pA) && (pE != pG)
								&& (!((pD == pG) && (pD == pI) && (pE == pB) && (pE == pF)))) {
								E[2] = interpolate_3_1( pH,E[2]);
								E[0] = interpolate_3_1(E[0], pH);
								doNotReblit = true;
							} else if ((pH == pF) && (pH == pC) && (pE != pI)
								&& (!((pF == pI) && (pF == pG) && (pE == pB) && (pE == pD)))) {
								E[3] = interpolate_3_1( pH,E[3]);
								E[1] = interpolate_3_1(E[1], pH);
								doNotReblit = true;
							}
						}
					}
				}
			}

			if (!doNotReblit) {
				if ((pB != pH) && (pD != pF)) {

					if ((pB == pD) && (pE != pD)
						&& (!((pE == pA) && (pB == pC) && (pE == pF))) // Block
						&& (!((pB == pA) && (pB == pG)))
						&& (!((pD == pA) && (pD == pC) && (pE == pF) && (pG != pD) && (pG != pE))))
						E[0] = interpolate_1_1(E[0], pB);

					if ((pB == pF) && (pE != pF)
						&& (!((pE == pC) && (pB == pA) && (pE == pD))) // Block
						&& (!((pB == pC) && (pB == pI)))
						&& (!((pF == pA) && (pF == pC) && (pE == pD) && (pI != pF) && (pI != pE))))
						E[1] = interpolate_1_1(E[1], pB);

					if ((pH == pD) && ((pE != pG) || (pE != pD))
						&& (!((pE == pG) && (pH == pI) && (pE == pF))) // Block
						&& (!((pH == pG) && (pH == pA)))
						&& (!((pD == pG) && (pD == pI) && (pE == pF) && (pA != pD) && (pA != pE))))
						E[2] = interpolate_1_1(E[2], pH);

					if ((pH == pF) && ((pE != pI) || (pE != pF))
						&& (!((pE == pI) && (pH == pG) && (pE == pD))) // Block
						&& (!((pH == pI) && (pH == pC)))
						&& (!((pF == pG) && (pF == pI) && (pE == pD) && (pC != pF) && (pI != pE))))
						E[3] = interpolate_1_1(E[3], pH);

				} else if ((pD == pB) && (pD == pF) && (pD == pH) && (pD != pE)) {
					if ((pD == pG) || (pD == pC)) {
						E[1] = interpolate_1_1(E[1], pD);
						E[2] = E[1];
					}

					if ((pD == pA) || (pD == pI)) {
						E[0] = interpolate_1_1(E[0], pD);
						E[3] = E[0];
					}
				}
			}

			dstPixel[0] = E[0];
			dstPixel[1] = E[1];
			dstPixel[nextLine] = E[2];
			dstPixel[nextLine + 1] = E[3];

			startAddr1++;
			startAddr2++;
			startAddr3++;

			dstPixel += 2;
		}

		startAddr2 += completeLineSrc;
		startAddr1 = startAddr2 - nextLineSrc;
		startAddr3 = startAddr2 + nextLineSrc;
		dstPixel += completeLine + nextLine;
	}

}

void PMScaler::scaleIntern(const uint8 *srcPtr, uint32 srcPitch,
							uint8 *dstPtr, uint32 dstPitch, int width, int height, int x, int y) {
	if (_format.bytesPerPixel == 2) {
		if (_format.gLoss == 2)
			::scaleIntern<Graphics::ColorMasks<565> >(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
		else
			::scaleIntern<Graphics::ColorMasks<555> >(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
	} else {
		if (_format.aLoss == 0)
			::scaleIntern<Graphics::ColorMasks<8888> >(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
		else
			::scaleIntern<Graphics::ColorMasks<888> >(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
	}
}

uint PMScaler::increaseFactor() {
	return _factor;
}

uint PMScaler::decreaseFactor() {
	return _factor;
}


class PMPlugin final : public ScalerPluginObject {
public:
	PMPlugin();

	Scaler *createInstance(const Graphics::PixelFormat &format) const override;

	bool canDrawCursor() const override { return false; }
	uint extraPixels() const override { return 1; }
	const char *getName() const override;
	const char *getPrettyName() const override;
};


PMPlugin::PMPlugin() {
	_factors.push_back(2);
}

Scaler *PMPlugin::createInstance(const Graphics::PixelFormat &format) const {
	return new PMScaler(format);
}

const char *PMPlugin::getName() const {
	return "pm";
}

const char *PMPlugin::getPrettyName() const {
	return "PM";
}

REGISTER_PLUGIN_STATIC(PM, PLUGIN_TYPE_SCALER, PMPlugin);
