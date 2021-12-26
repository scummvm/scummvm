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
// You can find it here: http://2xpm.freeservers.com
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
			// PA PB PC
			// PD PE PF
			// PG PH PI
			//
			Pixel PB = startAddr1[0];
			Pixel PE = startAddr2[0];
			Pixel PH = startAddr3[0];

			Pixel PA = startAddr1[-1];
			Pixel PD = startAddr2[-1];
			Pixel PG = startAddr3[-1];

			Pixel PC = startAddr1[1];
			Pixel PF = startAddr2[1];
			Pixel PI = startAddr3[1];

			bool doNotReblit = false;
			E[0] = E[1] = E[2] = E[3] = PE;

			if (!doNotReblit) {
				if (PD != PF) {
					if ((PE != PD) && (PD == PH) && (PD == PI) && (PE != PG)
						&& ((PD != PG) || (PE != PF) || (PA != PD))
						&& (!((PD == PA) && (PD == PG) && (PE == PB) && (PE == PF)))) {
						E[2] = PH;
						E[3] = interpolate_1_1(E[3], PH);
						doNotReblit = true;
					} else if ((PE != PF) && (PF == PH) && (PF == PG) && (PE != PI)
						&& ((PF != PI) || (PE != PD) || (PC != PF))
						&& (!((PF == PC) && (PF == PI) && (PE == PB) && (PE == PD)))) {
						E[2] = interpolate_1_1(E[2], PH);
						E[3] = PH;
						doNotReblit = true;
					}
				}

				if (PB != PH) {
					if (PE != PB) {
						if ((PA != PB) || (PB != PC) || (PE != PH)) {
							if ((PB == PD) && (PB == PG) && (PE != PA)
								&& (!((PD == PA) && (PD == PC) && (PE == PH) && (PE == PF)))) {
								E[0] = interpolate_3_1( PB,E[0]);
								E[2] = interpolate_3_1(E[2], PB);
								doNotReblit = true;
							} else if ((PB == PF) && (PB == PI) && (PE != PC)
								&& (!((PF == PC) && (PF == PA) && (PE == PH) && (PE == PD)))) {
								E[1] = interpolate_3_1(PB, E[1]);
								E[3] = interpolate_3_1(E[3], PB);
								doNotReblit = true;
							}
						}
					}

					if (PE != PH) {
						if ((PG != PH) || (PE != PB) || (PH != PI)) {
							if ((PH == PD) && (PH == PA) && (PE != PG)
								&& (!((PD == PG) && (PD == PI) && (PE == PB) && (PE == PF)))) {
								E[2] = interpolate_3_1( PH,E[2]);
								E[0] = interpolate_3_1(E[0], PH);
								doNotReblit = true;
							} else if ((PH == PF) && (PH == PC) && (PE != PI)
								&& (!((PF == PI) && (PF == PG) && (PE == PB) && (PE == PD)))) {
								E[3] = interpolate_3_1( PH,E[3]);
								E[1] = interpolate_3_1(E[1], PH);
								doNotReblit = true;
							}
						}
					}
				}
			}

			if (!doNotReblit) {
				if ((PB != PH) && (PD != PF)) {

					if ((PB == PD) && (PE != PD)
						&& (!((PE == PA) && (PB == PC) && (PE == PF))) // Block
						&& (!((PB == PA) && (PB == PG)))
						&& (!((PD == PA) && (PD == PC) && (PE == PF) && (PG != PD) && (PG != PE))))
						E[0] = interpolate_1_1(E[0], PB);

					if ((PB == PF) && (PE != PF)
						&& (!((PE == PC) && (PB == PA) && (PE == PD))) // Block
						&& (!((PB == PC) && (PB == PI)))
						&& (!((PF == PA) && (PF == PC) && (PE == PD) && (PI != PF) && (PI != PE))))
						E[1] = interpolate_1_1(E[1], PB);

					if ((PH == PD) && ((PE != PG) || (PE != PD))
						&& (!((PE == PG) && (PH == PI) && (PE == PF))) // Block
						&& (!((PH == PG) && (PH == PA)))
						&& (!((PD == PG) && (PD == PI) && (PE == PF) && (PA != PD) && (PA != PE))))
						E[2] = interpolate_1_1(E[2], PH);

					if ((PH == PF) && ((PE != PI) || (PE != PF))
						&& (!((PE == PI) && (PH == PG) && (PE == PD))) // Block
						&& (!((PH == PI) && (PH == PC)))
						&& (!((PF == PG) && (PF == PI) && (PE == PD) && (PC != PF) && (PI != PE))))
						E[3] = interpolate_1_1(E[3], PH);

				} else if ((PD == PB) && (PD == PF) && (PD == PH) && (PD != PE)) {
					if ((PD == PG) || (PD == PC)) {
						E[1] = interpolate_1_1(E[1], PD);
						E[2] = E[1];
					}

					if ((PD == PA) || (PD == PI)) {
						E[0] = interpolate_1_1(E[0], PD);
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
