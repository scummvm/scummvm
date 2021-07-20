/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or(at your option) any later version.
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

	uint32 **pixelb = (uint32 **)_engine->GetRawBitmapSurface(src);
	uint32 **pixela = (uint32 **)_engine->GetRawBitmapSurface(src2);
	_engine->ReleaseBitmapSurface(src2);
	int src_width = 640;
	int src_height = 360;
	int src_depth = 32;

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


				int color = pixela[setY][setX];

				totalRed += getRcolor(color);
				totalGreen += getGcolor(color);
				totalBlue += getBcolor(color);

				pixels_parsed++;
				vx++;
			}

			int rN = totalRed / pixels_parsed;
			int gN = totalGreen / pixels_parsed;
			int bN = totalBlue / pixels_parsed;

			int r = CLIP(rN, 0, 255);
			int g = CLIP(gN, 0, 255);
			int b = CLIP(bN, 0, 255);

			pixelb[y][x] = ((r << 16) | (g << 8) | (b << 0) | (255 << 24));
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

				int color = pixela[setY][setX];

				totalRed += getRcolor(color);
				totalGreen += getGcolor(color);
				totalBlue += getBcolor(color);

				pixels_parsed++;

				vy++;
			}

			int rN = totalRed / pixels_parsed;
			int gN = totalGreen / pixels_parsed;
			int bN = totalBlue / pixels_parsed;

			int r = CLIP(rN, 0, 255);
			int g = CLIP(gN, 0, 255);
			int b = CLIP(bN, 0, 255);

			pixelb[y][x] = ((r << 16) | (g << 8) | (b << 0) | (255 << 24));
		}
	}

	_engine->ReleaseBitmapSurface(src);
}

void AGSWaves::DrawTunnel(ScriptMethodParams &params) {
	PARAMS3(int, spriteD, float, scale, float, speed);

	d_time = speed;
	BITMAP *src = _engine->GetSpriteGraphic(spriteD);
	uint32 **pixela = (uint32 **)_engine->GetRawBitmapSurface(src);
	int src_width = 640;
	int src_height = 360;
	int src_depth = 32;
	_engine->GetBitmapDimensions(src, &src_width, &src_height, &src_depth);

	BITMAP *src2 = _engine->GetSpriteGraphic(int(scale));
	uint32 **pixelb = (uint32 **)_engine->GetRawBitmapSurface(src2);
	int h = screenHeight;
	int w = screenWidth;
	if (!generateonce) {
		generateonce = true;
		//generate texture
		for (int y = 0; y < texHeight; y++) {
			for (int x = 0; x < texWidth; x++) {
				texture[y][x] = pixelb[y][x];
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

			pixela[y][x] = color;
		}
	}

	_engine->ReleaseBitmapSurface(src2);
	_engine->ReleaseBitmapSurface(src);
}

void AGSWaves::DrawCylinder(ScriptMethodParams &params) {
	PARAMS2(int, spriteD, int, ogsprite);

	BITMAP *src = _engine->GetSpriteGraphic(spriteD);
	uint32 **pixela = (uint32 **)_engine->GetRawBitmapSurface(src);
	int src_width = 640;
	int src_height = 640;
	int src_depth = 32;
	_engine->GetBitmapDimensions(src, &src_width, &src_height, &src_depth);

	BITMAP *src2 = _engine->GetSpriteGraphic(ogsprite);
	uint32 **pixelb = (uint32 **)_engine->GetRawBitmapSurface(src2);
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
				pixela[y][x] = SetColorRGBA(0, 0, 0, 0);
			} else {
				pixela[y][x] = pixelb[cposy][cposx];
			}
		}
	}

	_engine->ReleaseBitmapSurface(src);
}

void AGSWaves::DrawForceField(ScriptMethodParams &params) {
	PARAMS4(int, spriteD, int, scale, float, speed, int, id);

	if (id < 0 || id >4) {
		return;
	}
	if (b_time[id] == NULL) b_time[id] = 1.0;
	if (b_time[id] < 1.0) b_time[id] = 1.0;
	b_time[id] += speed;
	BITMAP *src = _engine->GetSpriteGraphic(spriteD);

	uint32 **pixelb = (uint32 **)_engine->GetRawBitmapSurface(src);

	int src_width = 640;
	int src_height = 360;
	int src_depth = 32;

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
			int na = int(1.0 * 255.0);//pixelb[setY][setX];//int(1.0*255.0);

			int highest = 0;
			if (Rd > Gd) {
				if (Rd > Bd) highest = Rd;
				else highest = Bd;
			} else {
				if (Gd > Bd) highest = Gd;
				else highest = Bd;
			}

			int grabA = getAcolor(pixelb[setY][setX]);

			if (highest <= 40) {
				na = int((float(highest * 2) / 100.0) * 255.0);
			} else {
				na = grabA;
			}
			pixelb[setY][setX] = SetColorRGBA(Rd, Gd, Bd, na);//


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

	uint32 **pixel_a = (uint32 **)_engine->GetRawBitmapSurface(src_a);
	uint32 **pixel_b = (uint32 **)_engine->GetRawBitmapSurface(src_b);

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
			uint32 colorfromB = pixel_b[y][x];
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


			pixel_a[getY][getX] = colorfromB;	  //
		}
	}

	_engine->ReleaseBitmapSurface(src_a);
	_engine->ReleaseBitmapSurface(src_b);
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

} // namespace AGSWaves
} // namespace Plugins
} // namespace AGS3
