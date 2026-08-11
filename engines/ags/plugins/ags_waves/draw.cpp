/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * of the License, or(at your option) any later version.
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

#include "common/util.h"
#include "ags/plugins/ags_waves/ags_waves.h"
#include "ags/ags.h"

namespace AGS3 {
namespace Plugins {
namespace AGSWaves {

void AGSWaves::DrawScreenEffect(ScriptMethodParams &params) {
	PARAMS4(int, sprite, int, sprite_prev, int, ide, int, n);
	DrawEffect(sprite, sprite_prev, ide, n);
}

void AGSWaves::DrawBlur(ScriptMethodParams &params) {
	PARAMS2(int, spriteD, int, radius);

	int spriteD2 = spriteD;
	BITMAP *src = _engine->GetSpriteGraphic(spriteD);
	BITMAP *src2 = _engine->GetSpriteGraphic(spriteD2);

	uint32 *pixelb = (uint32 *)_engine->GetRawBitmapSurface(src);
	uint32 *pixela = (uint32 *)_engine->GetRawBitmapSurface(src2);
	_engine->ReleaseBitmapSurface(src2);
	int32 src_width = 640;
	int32 src_height = 360;
	int32 src_depth = 32;

	_engine->GetBitmapDimensions(src, &src_width, &src_height, &src_depth);


	int x, y;
	for (y = 0; y < src_height; y++) {
		for (x = 0; x < src_width; x++) {
			int totalRed = 0;
			int totalGreen = 0;
			int totalBlue = 0;

			int vx = -(radius);
			int pixels_parsed = 0;

			int setY = y;
			if (setY < 0) setY = 0;
			if (setY > src_height - 1) setY = src_height - 1;

			while (vx < (radius)+1) {
				int setX = x + vx;
				if (setX < 0) setX = 0;
				if (setX > src_width - 1) setX = src_width - 1;


				int color = pixela[setY * src_width + setX];

				totalRed += getRcolor(color);
				totalGreen += getGcolor(color);
				totalBlue += getBcolor(color);

				pixels_parsed++;
				vx++;
			}

			if (!pixels_parsed)
				pixels_parsed = 1;

			int rN = totalRed / pixels_parsed;
			int gN = totalGreen / pixels_parsed;
			int bN = totalBlue / pixels_parsed;

			int r = CLIP(rN, 0, 255);
			int g = CLIP(gN, 0, 255);
			int b = CLIP(bN, 0, 255);

			pixelb[y * src_width + x] = ((r << 16) | (g << 8) | (b << 0) | (255 << 24));
		}
	}


	_engine->ReleaseBitmapSurface(src);
	src = _engine->GetSpriteGraphic(spriteD);

	x = 0;
	y = 0;
	for (y = 0; y < src_height; y++) {
		for (x = 0; x < src_width; x++) {
			int totalRed = 0;
			int totalGreen = 0;
			int totalBlue = 0;

			int pixels_parsed = 0;
			int setX = x;
			if (setX < 0) setX = 0;
			if (setX > src_width - 1) setX = src_width - 1;

			int vy = -(radius);
			while (vy < (radius)+1) {
				int setY = y + vy;
				if (setY < 0) setY = 0;
				if (setY > src_height - 1) setY = src_height - 1;

				int color = pixela[setY * src_width + setX];

				totalRed += getRcolor(color);
				totalGreen += getGcolor(color);
				totalBlue += getBcolor(color);

				pixels_parsed++;

				vy++;
			}

			if (!pixels_parsed)
				pixels_parsed = 1;

			int rN = totalRed / pixels_parsed;
			int gN = totalGreen / pixels_parsed;
			int bN = totalBlue / pixels_parsed;

			int r = CLIP(rN, 0, 255);
			int g = CLIP(gN, 0, 255);
			int b = CLIP(bN, 0, 255);

			pixelb[y * src_width + x] = ((r << 16) | (g << 8) | (b << 0) | (255 << 24));
		}
	}

	_engine->ReleaseBitmapSurface(src);
}

void AGSWaves::DrawTunnel(ScriptMethodParams &params) {
	PARAMS3(int, spriteD, float, scale, float, speed);

	d_time = PARAM_TO_FLOAT(speed);
	BITMAP *src = _engine->GetSpriteGraphic(spriteD);
	uint32 *pixela = (uint32 *)_engine->GetRawBitmapSurface(src);
	int32 src_width = 640;
	int32 src_height = 360;
	int32 src_depth = 32;
	_engine->GetBitmapDimensions(src, &src_width, &src_height, &src_depth);

	BITMAP *src2 = _engine->GetSpriteGraphic((int32)PARAM_TO_FLOAT(scale));
	uint32 *pixelb = (uint32 *)_engine->GetRawBitmapSurface(src2);
	int h = screenHeight;
	int w = screenWidth;
	if (!generateonce) {
		generateonce = true;
		//generate texture
		for (int y = 0; y < texHeight; y++) {
			for (int x = 0; x < texWidth; x++) {
				texture[y][x] = pixelb[y * texWidth + x];
			}
		}

		//generate non-linear transformation table
		for (int y = 0; y < h; y++) {
			for (int x = 0; x < w; x++) {
				int angle, distance;
				float ratio = 32.0;
				distance = int(ratio * texHeight / sqrt((x - w / 2.0) * (x - w / 2.0) + (y - h / 2.0) * (y - h / 2.0))) % texHeight;
				angle = (uint32)(0.5 * texWidth * atan2(y - h / 2.0, x - w / 2.0) / 3.1416);
				distanceTable[y][x] = distance;///4.0;
				angleTable[y][x] = angle;
			}
		}
	}

	int shiftX = int(texWidth * 0.75 * d_time);
	int shiftY = int(texHeight * 1.0 * d_time);

	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			//get the texel from the texture by using the tables, shifted with the animation values
			int color = texture[(uint32)(distanceTable[y][x] + shiftX) % texWidth][(uint32)(angleTable[y][x] + shiftY) % texHeight];

			pixela[y * w + x] = color;
		}
	}

	_engine->ReleaseBitmapSurface(src2);
	_engine->ReleaseBitmapSurface(src);
}

void AGSWaves::DrawCylinder(ScriptMethodParams &params) {
	PARAMS2(int, spriteD, int, ogsprite);

	BITMAP *src = _engine->GetSpriteGraphic(spriteD);
	uint32 *pixela = (uint32 *)_engine->GetRawBitmapSurface(src);
	int32 src_width = 640;
	int32 src_height = 640;
	int32 src_depth = 32;
	_engine->GetBitmapDimensions(src, &src_width, &src_height, &src_depth);

	BITMAP *src2 = _engine->GetSpriteGraphic(ogsprite);
	uint32 *pixelb = (uint32 *)_engine->GetRawBitmapSurface(src2);
	_engine->ReleaseBitmapSurface(src2);
	int height = src_height;
	int width = src_width;

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			//convertPoint(x,y,width,height);

			//center the point at 0,0
			float pcx = x - width / 2;
			float pcy = y - height / 2;

			//these are your free parameters
			float f = width / 2;
			float r = width;

			float omega = width / 2;
			float z0 = f - sqrt(r * r - omega * omega);
			float zc = (2 * z0 + sqrt(4 * z0 * z0 - 4 * (pcx * pcx / (f * f) + 1) * (z0 * z0 - r * r))) / (2 * (pcx * pcx / (f * f) + 1));

			float finalpointx = pcx * zc / f;
			float finalpointy = pcy * zc / f;
			finalpointx += width / 2;
			finalpointy += height / 2;


			int cposx = finalpointx;
			int cposy = finalpointy;
			if (cposx < 0 ||
				cposx > width - 1 ||
				cposy < 0 ||
				cposy > height - 1) {
				pixela[y * width + x] = SetColorRGBA(0, 0, 0, 0);
			} else {
				pixela[y * width + x] = pixelb[cposy * src2->w + cposx];
			}
		}
	}

	_engine->ReleaseBitmapSurface(src);
}

void AGSWaves::DrawForceField(ScriptMethodParams &params) {
	PARAMS4(int, spriteD, int, scale, float, speed, int, id);

	if (id < 0 || id > 4) {
		return;
	}
	if (b_time[id] == 0.0) b_time[id] = 1.0;
	if (b_time[id] < 1.0) b_time[id] = 1.0;
	b_time[id] += PARAM_TO_FLOAT(speed);
	BITMAP *src = _engine->GetSpriteGraphic(spriteD);

	uint32 *pixelb = (uint32 *)_engine->GetRawBitmapSurface(src);

	int32 src_width = 640;
	int32 src_height = 360;
	int32 src_depth = 32;

	_engine->GetBitmapDimensions(src, &src_width, &src_height, &src_depth);


	int x, y;
	for (y = 0; y < src_height; y++) {
		for (x = 0; x < src_width; x++) {
			int setY = y;
			if (setY < 0) setY = 0;
			int setX = x;
			if (setX < 0) setX = 0;

			float uvx = float(x) / float(scale);
			float uvy = float(y) / float(scale);

			float jx = uvx;
			float jy = uvy + b_time[id] * 3.14;
			float jz = sin(b_time[id]);
			float jyy = uvy + b_time[id];
			float jzz = cos(b_time[id] + 3.0);

			float af = ABS(noiseField(jx, jy, jz) - noiseField(jx, jyy, jzz));
			float newR = 0.5 - pow(af, float(0.2)) / 2.0;
			float newG = 0.0;
			float newB = 0.4 - pow(af, float(0.4));

			int Rd = int(newR * 255.0);
			int Gd = int(newG * 255.0);
			int Bd = int(newB * 255.0);
			int na = int(1.0 * 255.0);//pixelb[setY * src_width + setX];//int(1.0*255.0);

			int highest = 0;
			if (Rd > Gd) {
				if (Rd > Bd) highest = Rd;
				else highest = Bd;
			} else {
				if (Gd > Bd) highest = Gd;
				else highest = Bd;
			}

			int grabA = getAcolor(pixelb[setY * src_width + setX]);

			if (highest <= 40) {
				na = int((float(highest * 2) / 100.0) * 255.0);
			} else {
				na = grabA;
			}
			pixelb[setY * src_width + setX] = SetColorRGBA(Rd, Gd, Bd, na);//


		}
	}

	_engine->ReleaseBitmapSurface(src);
}

void AGSWaves::SpriteSkew(ScriptMethodParams &params) {
	PARAMS5(int, sprite, float, xskewmin, float, yskewmin, float, xskewmax, float, yskewmax);

	BITMAP *src = _engine->GetSpriteGraphic(sprite);
	uint32 *pixel_src = (uint32 *)_engine->GetRawBitmapSurface(src);

	int32 src_width = 640;
	int32 src_height = 360;
	int32 src_depth = 32;
	_engine->GetBitmapDimensions(src, &src_width, &src_height, &src_depth);
	_engine->ReleaseBitmapSurface(src);

	BITMAP *dest = _engine->GetSpriteGraphic(sprite);
	uint32 *pixel_dest = (uint32 *)_engine->GetRawBitmapSurface(dest);

	int x, y;

	float raty = abs(yskewmin - yskewmax) / float(src_height * src_width);
	float ratx = abs(xskewmin - xskewmax) / float(src_height * src_width);
	float yskew = yskewmin;
	float xskew = xskewmin;

	for (y = 0; y < src_height; y++) {
		for (x = 0; x < src_width; x++) {
			int ry = int((float(x) * yskew) + float(y));
			int rx = int(float(x) + (float(y) * xskew));

			if (ry > src_height - 1) ry = src_height - 1;
			if (rx > src_width - 1) rx = src_width - 1;
			if (rx < 0) rx = 0;
			if (ry < 0) ry = 0;

			int getColor = pixel_src[ry * src_width + rx];
			int red = getRcolor(getColor);
			int green = getGcolor(getColor);
			int blue = getBcolor(getColor);
			int alpha = getAcolor(getColor);

			pixel_dest[y * src_width + x] = SetColorRGBA(red, green, blue, alpha);

			if (xskewmin < xskewmax) xskew += ratx;
			else xskew -= ratx;

			if (yskewmin < yskewmax) yskew += raty;
			else yskew -= raty;
		}
	}

	_engine->ReleaseBitmapSurface(dest);
}

void AGSWaves::ReturnWidth(ScriptMethodParams &params) {
	//PARAMS8(int, x1, int, y1, int, x2, int, y2, int, x3, int, y3, int, x4, int, y4);
	int x1 = params[0];
	int x2 = params[2];
	int x3 = params[4];
	int x4 = params[6];

	float ax = float(x1);
	float bx = float(x2);
	float cx = float(x3);
	float dx = float(x4);

	params._result = (int(max4(ax, bx, cx, dx)) + 1);
}

void AGSWaves::ReturnHeight(ScriptMethodParams &params) {
	//PARAMS8(int, x1, int, y1, int, x2, int, y2, int, x3, int, y3, int, x4, int, y4);
	int y1 = params[1];
	int y2 = params[3];
	int y3 = params[5];
	int y4 = params[7];

	float ay = float(y1);
	float by = float(y2);
	float cy = float(y3);
	float dy = float(y4);

	params._result = (int)max4(ay, by, cy, dy) + 1;
}

void AGSWaves::Grayscale(ScriptMethodParams &params) {
	PARAMS1(int, sprite);

	BITMAP *src = _engine->GetSpriteGraphic(sprite);
	uint32 *pixels = (uint32 *)_engine->GetRawBitmapSurface(src);

	int32 src_width = 640;
	int32 src_height = 360;
	int32 src_depth = 32;

	_engine->GetBitmapDimensions(src, &src_width, &src_height, &src_depth);

	int x, y;
	for (y = 0; y < src_height; y++) {
		for (x = 0; x < src_width; x++) {
			int color = ConvertColorToGrayScale(pixels[y * src_width + x]);
			pixels[y * src_width + x] = color;
		}
	}


	_engine->ReleaseBitmapSurface(src);
}

void AGSWaves::BlendTwoSprites(ScriptMethodParams &params) {
	PARAMS2(int, graphic, int, refgraphic);

	BITMAP *src = _engine->GetSpriteGraphic(graphic);
	int32 src_width = 640;
	int32 src_height = 360;
	int32 src_depth = 32;
	_engine->GetBitmapDimensions(src, &src_width, &src_height, &src_depth);
	uint32 *sprite_pixels = (uint32 *)_engine->GetRawBitmapSurface(src);

	BITMAP *refsrc = _engine->GetSpriteGraphic(refgraphic);
	int32 refsrc_width = 640;
	int32 refsrc_height = 360;
	int32 refsrc_depth = 32;
	_engine->GetBitmapDimensions(refsrc, &refsrc_width, &refsrc_height, &refsrc_depth);
	uint32 *refsprite_pixels = (uint32 *)_engine->GetRawBitmapSurface(refsrc);
	_engine->ReleaseBitmapSurface(refsrc);

	int x, y;

	for (y = 0; y < src_height; y++) {
		for (x = 0; x < src_width; x++) {
			int getColor = sprite_pixels[y * src_width + x];
			int rn = getRcolor(getColor);
			int gn = getGcolor(getColor);
			int bn = getBcolor(getColor);
			int an = getAcolor(getColor);

			if (an > 0.0 && rn > 4 && gn > 4 && bn > 4) {
				int getColor2 = refsprite_pixels[y * src_width + x];
				int rj = getRcolor(getColor2);
				int gj = getGcolor(getColor2);
				int bj = getBcolor(getColor2);
				int aj = getAcolor(getColor2);

				if (rj > 100 || gj > 100 || bj > 100) {
					sprite_pixels[y * src_width + x] = SetColorRGBA(rj, gj, bj, aj);
				}
			}
		}
	}

	_engine->ReleaseBitmapSurface(src);
}

void AGSWaves::Blend(ScriptMethodParams &params) {
	PARAMS4(int, graphic, int, refgraphic, bool, screen, int, perc);

	BITMAP *src = _engine->GetSpriteGraphic(graphic);
	int32 src_width = 640;
	int32 src_height = 360;
	int32 src_depth = 32;
	_engine->GetBitmapDimensions(src, &src_width, &src_height, &src_depth);
	uint32 *sprite_pixels = (uint32 *)_engine->GetRawBitmapSurface(src);

	BITMAP *refsrc = _engine->GetSpriteGraphic(refgraphic);
	int32 refsrc_width = 640;
	int32 refsrc_height = 360;
	int32 refsrc_depth = 32;
	_engine->GetBitmapDimensions(refsrc, &refsrc_width, &refsrc_height, &refsrc_depth);
	uint32 *refsprite_pixels = (uint32 *)_engine->GetRawBitmapSurface(refsrc);
	_engine->ReleaseBitmapSurface(refsrc);

	int x, y;

	for (y = 0; y < src_height; y++) {
		for (x = 0; x < src_width; x++) {
			int getColor = sprite_pixels[y * src_width + x];
			int rn = getRcolor(getColor);
			int gn = getGcolor(getColor);
			int bn = getBcolor(getColor);
			int an = getAcolor(getColor);

			if (an >= 0.0 && rn > 4 && gn > 4 && bn > 4) {
				int getColor2 = refsprite_pixels[y * src_width + x];
				int rj = getRcolor(getColor2);
				int gj = getGcolor(getColor2);
				int bj = getBcolor(getColor2);
				int aj = getAcolor(getColor2);

				if (!screen) {
					rj = BlendColor(rn, rj, perc);
					gj = BlendColor(gn, gj, perc);
					bj = BlendColor(bn, bj, perc);
					aj = BlendColor(an, aj, perc);
				} else {
					rj = BlendColorScreen(rn, rj, perc);
					gj = BlendColorScreen(gn, gj, perc);
					bj = BlendColorScreen(bn, bj, perc);
					aj = BlendColorScreen(an, aj, perc);
				}

				sprite_pixels[y * src_width + x] = SetColorRGBA(rj, gj, bj, aj);
			}
		}
	}

	_engine->ReleaseBitmapSurface(src);
}

void AGSWaves::Dissolve(ScriptMethodParams &params) {
	PARAMS3(int, graphic, int, noisegraphic, int, disvalue);

	BITMAP *src = _engine->GetSpriteGraphic(graphic);
	int32 src_width = 640;
	int32 src_height = 360;
	int32 src_depth = 32;
	_engine->GetBitmapDimensions(src, &src_width, &src_height, &src_depth);
	uint32 *sprite_pixels = (uint32 *)_engine->GetRawBitmapSurface(src);

	BITMAP *noisesrc = _engine->GetSpriteGraphic(noisegraphic);
	int32 noisesrc_width = 640;
	int32 noisesrc_height = 360;
	int32 noisesrc_depth = 32;
	_engine->GetBitmapDimensions(noisesrc, &noisesrc_width, &noisesrc_height, &noisesrc_depth);
	uint32 *noise_pixels = (uint32 *)_engine->GetRawBitmapSurface(noisesrc);
	_engine->ReleaseBitmapSurface(noisesrc);

	int x, y;

	for (y = 0; y < src_height; y++) {
		for (x = 0; x < src_width; x++)//
		{
			int getColor = noise_pixels[y * src_width + x];
			int gn = getRcolor(getColor);

			int getColorx = sprite_pixels[y * src_width + x];
			int rj = getRcolor(getColorx);
			int gj = getGcolor(getColorx);
			int bj = getBcolor(getColorx);
			int originalA = getAcolor(getColorx);
			int aj = 0;

			//disvalue 0-255
			//FOR EACH PIXEL IN THE NOISE GRAPHIC THAT IS < DISVALUE
			if (gn < disvalue) {
				if (gn > disvalue - 2) {
					rj = 193 + Random(20);
					gj = 132 + Random(20);
					bj = 255 + Random(20);
					aj = originalA;
				} else if (gn > disvalue - 3) {
					rj = 128 + Random(20);
					gj = 0 + Random(20);
					bj = 255 + Random(20);
					aj = 150;
				} else {
					aj = 0;
				}
			} else aj = originalA;

			if (originalA > 50) {
				sprite_pixels[y * src_width + x] = SetColorRGBA(rj, gj, bj, aj);
			}
		}
	}

	_engine->ReleaseBitmapSurface(src);
}

void AGSWaves::ReverseTransparency(ScriptMethodParams &params) {
	PARAMS1(int, graphic);

	BITMAP *noisesrc = _engine->GetSpriteGraphic(graphic);
	int32 noisesrc_width = 640;
	int32 noisesrc_height = 360;
	int32 noisesrc_depth = 32;
	_engine->GetBitmapDimensions(noisesrc, &noisesrc_width, &noisesrc_height, &noisesrc_depth);
	uint32 *noise_pixels = (uint32 *)_engine->GetRawBitmapSurface(noisesrc);
	_engine->ReleaseBitmapSurface(noisesrc);

	BITMAP *src = _engine->GetSpriteGraphic(graphic);
	int32 src_width = 640;
	int32 src_height = 360;
	int32 src_depth = 32;
	_engine->GetBitmapDimensions(src, &src_width, &src_height, &src_depth);
	uint32 *sprite_pixels = (uint32 *)_engine->GetRawBitmapSurface(src);

	int x, y;

	for (y = 0; y < src_height; y++) {
		for (x = 0; x < src_width; x++) {
			int getColors = noise_pixels[y * src_width + x];
			int TranClr = getAcolor(getColors);

			if (TranClr < 254) {
				//PIXEL IS TRANSPARENT
				sprite_pixels[y * src_width + x] = SetColorRGBA(255, 255, 255, 255);
			} else {
				//PIXEL IS VISIBLE
				sprite_pixels[y * src_width + x] = SetColorRGBA(0, 0, 0, 0);
			}
		}
	}

	_engine->ReleaseBitmapSurface(src);
}

void AGSWaves::TintProper(ScriptMethodParams &params) {
	PARAMS7(int, sprite, int, lightx, int, lighty, int, radi, int, rex, int, grx, int, blx);

	BITMAP *src = _engine->GetSpriteGraphic(sprite);
	BITMAP *src2 = _engine->GetSpriteGraphic(lightx);
	(void)lighty; // Unused

	uint32 *pixelb = (uint32 *)_engine->GetRawBitmapSurface(src);
	uint32 *pixela = (uint32 *)_engine->GetRawBitmapSurface(src2);
	_engine->ReleaseBitmapSurface(src2);
	int32 src_width = 640;
	int32 src_height = 360;
	int32 src_depth = 32;

	_engine->GetBitmapDimensions(src, &src_width, &src_height, &src_depth);

	int x, y;
	for (y = 0; y < src_height; y++) {
		for (x = 0; x < src_width; x++) {
			int totalRed = 0;
			int totalGreen = 0;
			int totalBlue = 0;

			int vx = -(radi);
			int pixels_parsed = 0;

			int setY = y;
			if (setY < 0) setY = 0;
			if (setY > src_height - 1) setY = src_height - 1;

			while (vx < (radi)+1) {
				int setX = x + vx;
				if (setX < 0) setX = 0;
				if (setX > src_width - 1) setX = src_width - 1;

				int color = pixela[setY * src_width + setX];

				totalRed += getRcolor(color);
				totalGreen += getGcolor(color);
				totalBlue += getBcolor(color);

				pixels_parsed++;

				vx++;
			}

			if (!pixels_parsed)
				pixels_parsed = 1;

			int rN = totalRed / pixels_parsed;
			int gN = totalGreen / pixels_parsed;
			int bN = totalBlue / pixels_parsed;

			int r = int(CLIP(rN, 0, 255));
			int g = int(CLIP(gN, 0, 255));
			int b = int(CLIP(bN, 0, 255));

			if (r > rex &&g > grx &&b > blx) {
				pixelb[y * src_width + x] = ((r << 16) | (g << 8) | (b << 0) | (255 << 24));
			} else {
				pixelb[y * src_width + x] = SetColorRGBA(rex, grx, blx, 0);
			}
		}
	}


	_engine->ReleaseBitmapSurface(src);
	src = _engine->GetSpriteGraphic(sprite);

	x = 0;
	y = 0;
	for (y = 0; y < src_height; y++) {
		for (x = 0; x < src_width; x++) {
			int totalRed = 0;
			int totalGreen = 0;
			int totalBlue = 0;

			int pixels_parsed = 0;
			int setX = x;
			if (setX < 0) setX = 0;
			if (setX > src_width - 1) setX = src_width - 1;

			int vy = -(radi);
			while (vy < (radi)+1) {
				int setY = y + vy;
				if (setY < 0) setY = 0;
				if (setY > src_height - 1) setY = src_height - 1;

				int color = pixela[setY * src_width + setX];

				totalRed += getRcolor(color);
				totalGreen += getGcolor(color);
				totalBlue += getBcolor(color);

				pixels_parsed++;


				vy++;
			}

			if (!pixels_parsed)
				pixels_parsed = 1;

			int rN = totalRed / pixels_parsed;
			int gN = totalGreen / pixels_parsed;
			int bN = totalBlue / pixels_parsed;

			int r = CLIP(rN, 0, 255);
			int g = CLIP(gN, 0, 255);
			int b = CLIP(bN, 0, 255);

			if (r > rex &&g > grx &&b > blx) {
				pixelb[y * src_width + x] = ((r << 16) | (g << 8) | (b << 0) | (255 << 24));
			} else {
				pixelb[y * src_width + x] = SetColorRGBA(rex, grx, blx, 0);
			}
		}
	}

	_engine->ReleaseBitmapSurface(src);
}

void AGSWaves::ReadWalkBehindIntoSprite(ScriptMethodParams &params) {
	PARAMS3(int, sprite, int, bgsprite, int, walkbehindBaseline);

	BITMAP *src = _engine->GetSpriteGraphic(sprite);
	BITMAP *bgsrc = _engine->GetSpriteGraphic(bgsprite);
	int32 src_width = 640;
	int32 src_height = 360;
	int32 src_depth = 32;

	_engine->GetBitmapDimensions(src, &src_width, &src_height, &src_depth);
	BITMAP *wbh = _engine->GetRoomMask(MASK_WALKBEHIND);

	uint32 *sprite_pixels = (uint32 *)_engine->GetRawBitmapSurface(src);
	uint32 *bgsprite_pixels = (uint32 *)_engine->GetRawBitmapSurface(bgsrc);
	byte *walk_pixels = _engine->GetRawBitmapSurface(wbh); //8bit

	_engine->ReleaseBitmapSurface(wbh);
	_engine->ReleaseBitmapSurface(bgsrc);

	// WE GRAB ALL OF THEM INTO A BITMAP and thus we know where they are drawn
	int x, y;
	for (y = 0; y < src_height; y++) {
		for (x = 0; x < src_width; x++) {
			//READ COLOR
			if (walk_pixels[y * src_width + x] > 0) {
				int grabBaseline = _engine->GetWalkbehindBaseline(walk_pixels[y * src_width + x]);

				if (grabBaseline == walkbehindBaseline) {
					sprite_pixels[y * src_width + x] = bgsprite_pixels[y * src_width + x];
				}
			}
		}
	}

	_engine->ReleaseBitmapSurface(src);
}

void AGSWaves::AdjustSpriteFont(ScriptMethodParams &params) {
	PARAMS5(int, sprite, int, rate, int, outlineRed, int, outlineGreen, int, outlineBlue);

	BITMAP *src = _engine->GetSpriteGraphic(sprite);
	uint32 *pixel_src = (uint32 *)_engine->GetRawBitmapSurface(src);

	int32 src_width = 640;
	int32 src_height = 360;
	int32 src_depth = 32;
	_engine->GetBitmapDimensions(src, &src_width, &src_height, &src_depth);

	int x, y;

	int px = 1;
	bool found = false;
	for (y = 0; y < src_height; y++) {
		if (found) {
			px++;
		}
		//if (px >12) px=12;
		bool havefound = false;
		for (x = 0; x < src_width; x++) {
			int getColor = pixel_src[y * src_width + x];
			int red = getRcolor(getColor);
			int green = getGcolor(getColor);
			int blue = getBcolor(getColor);
			int alpha = getAcolor(getColor);

			if (alpha < 255.0 || (red <= 10 && green <= 10 && blue <= 10)) {
				//px=1;
				if (alpha == 255 && (red <= 10 && green <= 10 && blue <= 10)) {
					pixel_src[y * src_width + x] = SetColorRGBA(outlineRed, outlineGreen, outlineBlue, 255);
				}
			} else {
				havefound = true;
				found = true;
				red -= (px * rate);
				green -= (px * rate);
				blue -= (px * rate);

				pixel_src[y * src_width + x] = SetColorRGBA(red, green, blue, 255);
			}
		}

		if (havefound == false) {
			if (found) {
				px = 1;
				found = false;
			}
		}
	}

	_engine->ReleaseBitmapSurface(src);
}

void AGSWaves::SpriteGradient(ScriptMethodParams &params) {
	PARAMS3(int, sprite, int, rate, int, toy);

	BITMAP *src = _engine->GetSpriteGraphic(sprite);
	uint32 *pixel_src = (uint32 *)_engine->GetRawBitmapSurface(src);

	int32 src_width = 640;
	int32 src_height = 360;
	int32 src_depth = 32;
	_engine->GetBitmapDimensions(src, &src_width, &src_height, &src_depth);

	int x, y;
	int setA = 0;

	for (y = toy; y < src_height; y++) {
		for (x = 0; x < src_width; x++) {
			int getColor = pixel_src[y * src_width + x];
			int red = getRcolor(getColor);
			int green = getGcolor(getColor);
			int blue = getBcolor(getColor);
			int alpha = getAcolor(getColor) + setA;
			if (alpha > 250) alpha = 250;

			if (red > 10 && green > 10 && blue > 10) {
				pixel_src[y * src_width + x] = SetColorRGBA(red, green, blue, alpha);
			}

		}
		setA += rate;
	}

	_engine->ReleaseBitmapSurface(src);
}

void AGSWaves::Outline(ScriptMethodParams &params) {
	PARAMS5(int, sprite, int, red, int, ged, int, bed, int, aed);

	BITMAP *src = _engine->GetSpriteGraphic(sprite);
	uint32 *pixel_src = (uint32 *)_engine->GetRawBitmapSurface(src);

	int32 src_width = 640;
	int32 src_height = 360;
	int32 src_depth = 32;
	_engine->GetBitmapDimensions(src, &src_width, &src_height, &src_depth);

	//OUTLINE
	_engine->ReleaseBitmapSurface(src);


	BITMAP *dst = _engine->GetSpriteGraphic(sprite);
	uint32 *pixel_dst = (uint32 *)_engine->GetRawBitmapSurface(dst);

	int x, y;
	for (x = 0; x < src_width; x++) {
		for (y = 0; y < src_height; y++) {
			if (!IsPixelTransparent(pixel_src[y * src_width + x])) {
			} else {
				int pcount = 0;
				int gy = -1;
				while (gy < 2) {
					int gx = -1;
					while (gx < 2) {
						int sx = x + gx;
						int sy = y + gy;

						if (sx < 0) sx = 0;
						if (sy < 0) sy = 0;
						if (sx > src_width - 1) sx = src_width - 1;
						if (sy > src_height - 1) sy = src_height - 1;

						if (!IsPixelTransparent(pixel_src[sy * src_width + sx])) {
							pcount++;
						}

						gx++;
					}
					gy++;
				}

				if (pcount >= 2) {
					int colorLeft = SetColorRGBA(red, ged, bed, aed);
					pixel_dst[y * src_width + x] = colorLeft;
				}
			}
		}
	}

	// OUTLINE
	_engine->ReleaseBitmapSurface(dst);
}

void AGSWaves::OutlineOnly(ScriptMethodParams &params) {
	PARAMS7(int, sprite, int, refsprite, int, red, int, ged, int, bed, int, aed, int, trans);

	BITMAP *src = _engine->GetSpriteGraphic(refsprite);
	uint32 *pixel_src = (uint32 *)_engine->GetRawBitmapSurface(src);

	int32 src_width = 640;
	int32 src_height = 360;
	int32 src_depth = 32;
	_engine->GetBitmapDimensions(src, &src_width, &src_height, &src_depth);

	// OUTLINE
	_engine->ReleaseBitmapSurface(src);

	BITMAP *dst = _engine->GetSpriteGraphic(sprite);
	uint32 *pixel_dst = (uint32 *)_engine->GetRawBitmapSurface(dst);

	int x, y;
	for (x = 0; x < src_width; x++) {
		for (y = 0; y < src_height; y++) {
			if (!IsPixelTransparent(pixel_src[y * src_width + x])) {
				int colorLeft = SetColorRGBA(red, ged, bed, trans);
				pixel_dst[y * src_width + x] = colorLeft;
			} else {
				int pcount = 0;
				int gy = -1;
				while (gy < 2) {
					int gx = -1;
					while (gx < 2) {
						int sx = x + gx;
						int sy = y + gy;

						if (sx < 0) sx = 0;
						if (sy < 0) sy = 0;
						if (sx > src_width - 1) sx = src_width - 1;
						if (sy > src_height - 1) sy = src_height - 1;

						if (!IsPixelTransparent(pixel_src[sy * src_width + sx])) {
							pcount++;
						}

						gx++;
					}
					gy++;
				}

				if (pcount >= 2) {
					int colorLeft = SetColorRGBA(red, ged, bed, aed);
					pixel_dst[y * src_width + x] = colorLeft;
				}
			}
		}
	}

	// OUTLINE
	_engine->ReleaseBitmapSurface(dst);
}

void AGSWaves::NoiseCreator(ScriptMethodParams &params) {
	PARAMS2(int, graphic, int, setA);

	BITMAP *src = _engine->GetSpriteGraphic(graphic);
	int32 src_width = 640;
	int32 src_height = 360;
	int32 src_depth = 32;
	_engine->GetBitmapDimensions(src, &src_width, &src_height, &src_depth);
	uint32 *sprite_pixels = (uint32 *)_engine->GetRawBitmapSurface(src);

	int x, y;
	for (y = 0; y < src_height; y++) {
		for (x = 0; x < src_width; x++) {
			//int getColor = sprite_pixels[y][x];
			int r = Random(256);
			int g = Random(256);
			int b = Random(256);
			int a = setA;

			sprite_pixels[y * src_width + x] = SetColorRGBA(r, g, b, a);

		}
	}

	_engine->ReleaseBitmapSurface(src);
}

void AGSWaves::CastWave(int delayMax, int PixelsWide, int n) {
	tDy[n]++;
	if (tDy[n] > delayMax) {
		tDy[n] = 0;
		if (direction[n] == 0) dY[n]++;
		if (direction[n] == 1) dY[n]--;
		if ((dY[n] > PixelsWide &&direction[n] == 0) || (dY[n] < (-PixelsWide) && direction[n] == 1)) {
			if (direction[n] == 0) {
				dY[n] = PixelsWide; direction[n] = 1;
			} else {
				dY[n] = -PixelsWide; direction[n] = 0;
			}
		}
	}
}

void AGSWaves::DrawEffect(int sprite_a, int sprite_b, int id, int n) {
	int32 x, y;

	BITMAP *src_a = _engine->GetSpriteGraphic(sprite_a);
	BITMAP *src_b = _engine->GetSpriteGraphic(sprite_b);

	uint32 *pixel_a = (uint32 *)_engine->GetRawBitmapSurface(src_a);
	uint32 *pixel_b = (uint32 *)_engine->GetRawBitmapSurface(src_b);

	int32 src_width = 640;
	int32 src_height = 360;
	int32 src_depth = 32;
	_engine->GetBitmapDimensions(src_a, &src_width, &src_height, &src_depth);



	for (y = 0; y < src_height; y++) {
		if (id == 1)
			CastWave(15, 1, n);
		if (id == 0 || id == 9 || id == 2 || id == 3 || id == 6 || id == 8)
			CastWave(2, 1, n);
		if (id == 4)
			CastWave(15, 4, n);
		if (id == 5 || id == 7 || id == 10) {
			//x=0;
			CastWave(3, 1, n);
		}
		if (id == 11) {
			//x=0;
			CastWave(3, 2, n);
		}
		if (id == 16) CastWave(4, 1, n);
		if (id == 17) CastWave(6, 1, n);


		for (x = 0; x < src_width; x++) {
			uint32 colorfromB = pixel_b[y * src_width + x];
			int32 getX = x;
			int32 getY = y;

			if (id == 0) {
				getX = x - ::AGS::g_vm->getRandomNumber(1) - 2;
				getY = y + dY[n];
			}
			if (id == 1 || id == 4) {
				getX = x;
				getY = y + dY[n];
			}
			if (id == 2) {
				getX = x + dY[n];
				getY = y - ::AGS::g_vm->getRandomNumber(1) - 2;
			}
			if (id == 3) {
				getX = x;
				getY = y - ::AGS::g_vm->getRandomNumber(1) - 2;
			}
			if (id == 5) {
				getX = x + dY[n];
				getY = y - ::AGS::g_vm->getRandomNumber(1) - 2;
			}
			if (id == 6 || id == 16) {
				getX = x + dY[n];
				getY = y - (0 /* rand() % 1 */) - 1;
			}
			if (id == 7 || id == 17) {
				getX = x + dY[n];
				getY = y - (0 /* rand() % 1 */) - 1;
			}
			if (id == 8) {
				getX = x + dY[n];
				getY = y + ::AGS::g_vm->getRandomNumber(1) - 2;
			}
			if (id == 10 || id == 9 || id == 11) {
				getX = x + dY[n];
				getY = y;
			}

			if (getX < 0) getX = 0;
			if (getX > src_width - 1) getX = src_width - 1;
			if (getY > src_height - 1) getY = src_height - 1;
			if (getY < 0) getY = 0;


			pixel_a[getY * src_width + getX] = colorfromB;	  //
		}
	}

	_engine->ReleaseBitmapSurface(src_a);
	_engine->ReleaseBitmapSurface(src_b);
}

int AGSWaves::Random(int threshold) {
	return ::AGS::g_vm->getRandomNumber(threshold - 1);
}

int AGSWaves::SetColorRGBA(int r, int g, int b, int a) {
	r = CLIP(r, 0, 255);
	g = CLIP(g, 0, 255);
	b = CLIP(b, 0, 255);
	a = CLIP(a, 0, 255);
	return int((r << 16) | (g << 8) | (b << 0) | (a << 24));
}

float AGSWaves::noiseField(float tx, float ty, float tz) {
	float px = floor(tx);
	float fx = fracts(tx);
	float py = floor(ty);
	float fy = fracts(ty);
	float pz = floor(tz);
	float fz = fracts(tz);
	fx = fx * fx * (3.0 - 2.0 * fx);
	fy = fy * fy * (3.0 - 2.0 * fy);
	fz = fz * fz * (3.0 - 2.0 * fz);

	float n = px + py * 157.0 + 113.0 * pz;
	return lerp(lerp(lerp(hasher(n + 0.0), hasher(n + 1.0), fx),
		lerp(hasher(n + 157.0), hasher(n + 158.0), fx), fy),
		lerp(lerp(hasher(n + 113.0), hasher(n + 114.0), fx),
			lerp(hasher(n + 270.0), hasher(n + 271.0), fx), fy), fz);
}

int AGSWaves::ConvertColorToGrayScale(int color) {
	int r = getRcolor(color);
	int g = getGcolor(color);
	int b = getBcolor(color);

	float d = float((r * r + g * g + b * b) / 3);
	int gr = int(sqrt(d));

	return ((gr << 16) | (gr << 8) | (gr << 0) | (255 << 24));
}

bool AGSWaves::IsPixelTransparent(int color) {
	int ad = getAcolor(color);
	return ad < 255;
}

} // namespace AGSWaves
} // namespace Plugins
} // namespace AGS3
