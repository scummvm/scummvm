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

#include "ags/plugins/ags_waves/ags_waves.h"

namespace AGS3 {
namespace Plugins {
namespace AGSWaves {

void AGSWaves::FireUpdate(ScriptMethodParams &params) {
	PARAMS2(int, getDynamicSprite, bool, Fire2Visible);

	BITMAP *src = _engine->GetSpriteGraphic(getDynamicSprite);
	uint32 **pixel_src = (uint32 **)_engine->GetRawBitmapSurface(src);
	int32 src_width = 640;
	int32 src_height = 360;
	int32 src_depth = 32;
	_engine->GetBitmapDimensions(src, &src_width, &src_height, &src_depth);

	//OUTLINE
	creationdelay += int(2.0);
	if (creationdelay > 4 && Fire2Visible) {
		int by = 0;
		while (by < 6) {
			int dnx = 95 + (Random(535 - 95));
			int dny = Random(236);

			PluginMethod sfGetRegionXY = _engine->GetScriptFunctionAddress("GetRegionAt");
			int getID = sfGetRegionXY(dnx, dny);

			while (getID != 10) {
				dnx = 95 + (Random(535 - 95));
				dny = Random(236);
				getID = sfGetRegionXY(dnx, dny);
			}
			CreateDustParticle(dnx, dny);
			by++;
		}

		creationdelay = 0;
	}
	int h = dsizeDust - 1;
	while (h > 0) {
		if (dusts[h].life > 0) {
			dusts[h].life -= int(2.0);

			int setX = dusts[h].x;
			int setY = dusts[h].y;

			if (setX < 0) setX = 0;
			if (setX > src_width) setX = src_width;

			if (setY < 0) setY = 0;
			if (setY > src_height) setY = src_height;

			int Rf = Random(100);
			int rv, gv, bv, av;

			if (Rf < 50) {
				rv = 255; gv = 128; bv = 0;
			} else {
				rv = 231; gv = 71; bv = 24;
			}

			av = int((float(255 * (150 - dusts[h].transp))) / 100.0);


			pixel_src[setY][setX] = SetColorRGBA(rv, gv, bv, av);

			//drawt.DrawImage(dusts[h].x, dusts[h].y, sg, dusts[h].transp);
			dusts[h].timlay += int(8.0);
			if (dusts[h].timlay > dusts[h].mlay) {
				dusts[h].timlay = 0;
				dusts[h].x += dusts[h].dx + Random(1);
				dusts[h].y += dusts[h].dy - (Random(1));
			}
			dusts[h].translay += 2;
			if (dusts[h].translay >= dusts[h].translayHold) {
				if (dusts[h].transp <= 99) dusts[h].transp++;
				else dusts[h].life = 0;
			}
		} else {
			dusts[h].active = false;
		}
		h--;
	}

	_engine->ReleaseBitmapSurface(src);
}

void AGSWaves::WindUpdate(ScriptMethodParams &params) {
	PARAMS4(int, ForceX, int, ForceY, int, Transparency, int, sprite);

	BITMAP *src = _engine->GetSpriteGraphic(sprite);
	int32 src_width = 640;
	int32 src_height = 360;
	int32 src_depth = 32;
	_engine->GetBitmapDimensions(src, &src_width, &src_height, &src_depth);
	uint32 **sprite_pixels = (uint32 **)_engine->GetRawBitmapSurface(src);

	int by = 0;
	while (by < 2) {
		int dnx = Random(ww + 250) - 250;
		int dny = Random(hh);
		CreateParticle(dnx, dny, ForceX, ForceY);
		by++;
	}

	int dnx;
	if (ForceX > 0) dnx = (Random(ww + 250) - 250) - (50 + Random(100));
	else dnx = Random(ww + 250) - 250;
	//
	int dny = Random(hh);
	CreateParticle2(dnx, dny, ForceX, ForceY);


	dnx = -(20 + Random(50));//Random(ww);
	if (dnx < -160) dnx = -160;
	if (dnx > ww + 160) dnx = ww + 160;

	dny = Random(hh);
	CreateParticleF(dnx, dny, ForceX, ForceY);

	int h = dsize - 1;

	if (h < dsizeF - 1) {
		h = dsizeF - 1;
	}

	int setByx = 0;
	if (proom == 3 && prevroom == 14) {
		setByx = 640;
	}
	if (proom == 4 && prevroom == 8) {
		setByx -= 480;
	}
	while (h > 0) {
		if (particles[h].life > 0) {
			particles[h].life -= int(3.0);
			particles[h].doingCircleChance -= 2;
			int df = 100 - particles[h].transp;
			df = 10 - (df / 4);

			int pwidth = particles[h].width + df;
			int pheight = particles[h].height + df;

			int px = particles[h].x - (pwidth / 2);
			int py = particles[h].y - (pheight / 2);
			int tp = particles[h].transp + Transparency;

			if (tp > 100) tp = 100;

			int pgraph = 0;
			int SplitBetweenTwo = Random(100);
			if (SplitBetweenTwo <= 50) pgraph = 813;
			else pgraph = 4466;

			if (tp != 100) {

				BITMAP *src2 = _engine->GetSpriteGraphic(pgraph + particles[h].frame);


				int32 src2_width = 640;
				int32 src2_height = 360;
				int32 src2_depth = 32;
				_engine->GetBitmapDimensions(src2, &src2_width, &src2_height, &src2_depth);
				uint32 **sprite_pixels2 = (uint32 **)_engine->GetRawBitmapSurface(src2);
				_engine->ReleaseBitmapSurface(src2);

				int startx = px + setByx;
				int endx = px + setByx + src2_width;
				int starty = py;
				int endy = py + src2_height;



				int x, y;
				int ny = 0;
				for (y = starty; y < endy; y++) {
					int nx = 0;
					for (x = startx; x < endx; x++) {
						int setX = nx;
						int setY = ny;
						if (setX < 0)setX = 0;
						if (setX > src2_width - 1) setX = src2_width - 1;
						if (setY < 0) setY = 0;
						if (setY > src2_height - 1) setY = src2_height - 1;

						int netX = x;
						int netY = y;


						if (netX < 0) netX = 0;
						if (netX > src_width - 1) netX = src_width - 1;
						if (netY < 0) netY = 0;
						if (netY > src_height - 1) netY = src_height - 1;

						int clr = sprite_pixels2[setY][setX];
						int rv = getRcolor(clr);
						int gv = getGcolor(clr);
						int bv = getBcolor(clr);
						int av = getAcolor(clr);

						av = int(float((av * (100 - tp))) / 100.0);

						sprite_pixels[netY][netX] = SetColorRGBA(rv, gv, bv, av);
						nx++;
					}
					ny++;
				}

			}
			particles[h].timlay += int(6.0);
			if (particles[h].timlay > particles[h].mlay) {
				particles[h].frame++;
				if (particles[h].frame > 6) particles[h].frame = 0;
				particles[h].timlay = 0;
				particles[h].x += particles[h].dx + particles[h].fx;
				particles[h].y += particles[h].dy + particles[h].fy;//Random(1);
			}
			particles[h].translay += 2;
			if (particles[h].translay >= particles[h].translayHold) {
				if (particles[h].transp <= 99) particles[h].transp++;
				else {
					particles[h].life = 0;
				}
			}
			if (particles[h].x >= (ww - 90) + setByx || particles[h].x < 90 + setByx) {
				if (particles[h].transp <= 99)particles[h].transp++;
				else {
					particles[h].life = 0;
				}
			}

			if (!particles[h].doingcircle && particles[h].angle == 0.0
				&& particles[h].doingCircleChance <= 0) {
				particles[h].doingcircle = true;
			}
			if (particles[h].doingcircle) {
				particles[h].angleLay += float(1 + WForceX[h]) * 1.5;
				if (particles[h].angleLay > 12.0) {
					particles[h].angleLay = 0.0;
					particles[h].angle += particles[h].anglespeed;
					int Y = particles[h].y + int((sin(particles[h].angle) * particles[h].radius));
					int X = particles[h].x + int((cos(particles[h].angle) * particles[h].radius));
					particles[h].x = X;
					particles[h].y = Y;
				}
			}
			particles[h].fx = ForceX;
			particles[h].fy = ForceY;

		} else {
			particles[h].active = false;
		}






		if (h <= 5 && particlesF[h].life > 0) {
			int pwidth = particlesF[h].width;
			int pheight = particlesF[h].height;
			int px = particlesF[h].x - (pwidth / 2);
			int py = particlesF[h].y - (pheight / 2);
			int pgraph = 0;
			int SplitBetweenTwo = Random(100);
			if (SplitBetweenTwo <= 50) pgraph = 806;
			else pgraph = 4459;

			int tp = particlesF[h].transp + Transparency;
			if (tp > 100) tp = 100;


			if (tp != 100) {

				BITMAP *src2 = _engine->GetSpriteGraphic(pgraph + particlesF[h].frame);
				int32 src2_width = 640;
				int32 src2_height = 360;
				int32 src2_depth = 32;
				_engine->GetBitmapDimensions(src2, &src2_width, &src2_height, &src2_depth);
				uint32 **sprite_pixels2 = (uint32 **)_engine->GetRawBitmapSurface(src2);
				_engine->ReleaseBitmapSurface(src2);

				int startx = px + setByx;
				int endx = px + setByx + src2_width;
				int starty = py;
				int endy = py + src2_height;


				int x, y;
				int ny = 0;
				for (y = starty; y < endy; y++) {
					int nx = 0;
					for (x = startx; x < endx; x++) {
						int setX = nx;
						int setY = ny;
						if (setX < 0)setX = 0;
						if (setX > src2_width - 1) setX = src2_width - 1;
						if (setY < 0) setY = 0;
						if (setY > src2_height - 1) setY = src2_height - 1;

						int netX = x;
						int netY = y;


						if (netX < 0) netX = 0;
						if (netX > src_width - 1) netX = src_width - 1;
						if (netY < 0) netY = 0;
						if (netY > src_height - 1) netY = src_height - 1;

						int clr = sprite_pixels2[setY][setX];
						int rv = getRcolor(clr);
						int gv = getGcolor(clr);
						int bv = getBcolor(clr);
						int av = getAcolor(clr);

						av = int(float((av * (100 - tp))) / 100.0);

						sprite_pixels[netY][netX] = SetColorRGBA(rv, gv, bv, av);

						nx++;
					}
					ny++;
				}




				// drawt.DrawImage(px+setByx, py, , tp, pwidth, pheight);
			}
			particlesF[h].timlay += int(6.0);
			if (particlesF[h].timlay > particlesF[h].mlay) {
				particlesF[h].frame++;
				if (particlesF[h].frame > 6)  particlesF[h].frame = 0;
				particlesF[h].timlay = 0;
				particlesF[h].x += particlesF[h].dx + ForceX;
				particlesF[h].y += particlesF[h].dy + ForceY;
			}


			if (particlesF[h].x >= ww - 90 || particlesF[h].x < 90) {
				particlesF[h].translay += 2;
				if (particlesF[h].translay >= particlesF[h].translayHold) {
					if (particlesF[h].transp <= 99) particlesF[h].transp++;
					else {
						particlesF[h].life = 0;
					}
				}
			}
		} else {
			if (h <= 9)  particlesF[h].active = false;
		}


		//SECOND PARTICLES
		if (h <= 10) {
			if (particles2[h].life > 0) {
				particles2[h].life -= int(3.0);
				particles2[h].doingCircleChance -= 1;
				int df = 100 - particles2[h].transp;//45-0
				df = 10 - (df / 4);//10-0

				int pwidth = particles2[h].width + df;
				int pheight = particles2[h].height + df;

				int px = particles2[h].x - (pwidth / 2);
				int py = particles2[h].y - (pheight / 2);
				int tp = particles2[h].transp + Transparency;

				if (tp > 100) tp = 100;

				int pgraph = 5224;

				if (tp != 100) {

					BITMAP *src2 = _engine->GetSpriteGraphic(pgraph + particles2[h].frame);


					int32 src2_width = 640;
					int32 src2_height = 360;
					int32 src2_depth = 32;
					_engine->GetBitmapDimensions(src2, &src2_width, &src2_height, &src2_depth);
					uint32 **sprite_pixels2 = (uint32 **)_engine->GetRawBitmapSurface(src2);
					_engine->ReleaseBitmapSurface(src2);

					int startx = px + setByx;
					int endx = px + setByx + src2_width;
					int starty = py;
					int endy = py + src2_height;



					int x, y;
					int ny = 0;
					for (y = starty; y < endy; y++) {
						int nx = 0;
						for (x = startx; x < endx; x++) {
							int setX = nx;
							int setY = ny;
							if (setX < 0)setX = 0;
							if (setX > src2_width - 1) setX = src2_width - 1;
							if (setY < 0) setY = 0;
							if (setY > src2_height - 1) setY = src2_height - 1;

							int netX = x;
							int netY = y;


							if (netX < 0) netX = 0;
							if (netX > src_width - 1) netX = src_width - 1;
							if (netY < 0) netY = 0;
							if (netY > src_height - 1) netY = src_height - 1;

							int clr = sprite_pixels2[setY][setX];
							int rv = getRcolor(clr);
							int gv = getGcolor(clr);
							int bv = getBcolor(clr);
							int av = getAcolor(clr);

							av = int(float((av * (100 - tp))) / 100.0);

							sprite_pixels[netY][netX] = SetColorRGBA(rv, gv, bv, av);
							nx++;
						}
						ny++;
					}


				}
				particles2[h].timlay += int(6.0);
				if (particles2[h].timlay > particles2[h].mlay) {
					particles2[h].frame++;
					if (particles2[h].frame > 7) particles2[h].frame = 0;
					particles2[h].timlay = 0;
					particles2[h].x += particles2[h].dx + particles2[h].fx;
					particles2[h].y += particles2[h].dy + particles2[h].fy;//Random(1);
				}
				particles2[h].translay += 2;
				if (particles2[h].translay >= particles2[h].translayHold) {
					if (particles2[h].transp <= 99) particles2[h].transp++;
					else {
						particles2[h].life = 0;
					}
				}
				if (particles2[h].x >= (ww - 90) + setByx || particles2[h].x < 90 + setByx) {
					if (particles2[h].transp <= 99)particles2[h].transp++;
					else {
						particles2[h].life = 0;
					}
				}

				if (!particles2[h].doingcircle && particles2[h].angle == 0.0
					&& particles2[h].doingCircleChance <= 0) {
					particles2[h].doingcircle = true;
				}
				if (particles2[h].doingcircle) {
					particles2[h].angleLay += float((1 + WForceX[h + 200]));
					if (particles2[h].angleLay > 12.0) {
						particles2[h].angleLay = 0.0;
						particles2[h].angle += particles2[h].anglespeed;
						int Y = particles2[h].y + int((sin(particles2[h].angle) * particles2[h].radius));
						int X = particles2[h].x + int((cos(particles2[h].angle) * particles2[h].radius));
						particles2[h].x = X;
						particles2[h].y = Y;
					}
				}
				particles2[h].fx = int(float(ForceX) * 3.5);
				particles2[h].fy = int(float(ForceY) * 3.5);

			} else {
				particles2[h].active = false;
			}
		}

		// SECOND PARTICLES
		h--;
	}

	_engine->ReleaseBitmapSurface(src);
}

void AGSWaves::SetWindValues(ScriptMethodParams &params) {
	PARAMS4(int, w, int, h, int, pr, int, prev);

	ww = w;
	hh = h;
	proom = pr;
	prevroom = prev;
}

void AGSWaves::RainUpdate(ScriptMethodParams &params) {
	PARAMS7(int, rdensity, int, FX, int, FY, int, RW, int, RH, int, graphic, float, perc);

	bool drawBack = true;
	bool drawMid = true;
	bool drawFore = true;
	int h = 0;

	int cdelay = 0;
	while (cdelay < rdensity) {
		if (drawMid) CreateRainParticleMid(Random(640 * 4) - 640, -(20 + Random(50)), FX, FY, int((400.0 * perc) / 100.0));
		if (drawFore) CreateRainParticleFore(Random(640 * 4) - 640, -(20 + Random(50)), FX, FY, int((40.0 * perc) / 100.0));
		if (drawBack) {
			CreateRainParticleBack(Random(640 * 4) - 640, -(20 + Random(50)), FX, FY, int((800.0 * perc) / 100.0));
			CreateRainParticleBack(Random(640 * 4) - 640, -(20 + Random(50)), FX, FY, int((800.0 * perc) / 100.0));
		}
		cdelay++;
	}

	BITMAP *src = _engine->GetSpriteGraphic(graphic);
	int32 src_width = 640;
	int32 src_height = 360;
	int32 src_depth = 32;
	_engine->GetBitmapDimensions(src, &src_width, &src_height, &src_depth);
//	uint32 **sprite_pixels = (uint32 **)_engine->GetRawBitmapSurface(src);

	int rotAngle = 6;
	int rotTrans = 60 + Random(40 + 60);//Random(103)+122;
	int rotX = -50;
	int rotY = 120;
	int totalTrans = 0;

	int maxPart = 800;
	if (!drawBack) maxPart = 400;
	if (!drawMid) maxPart = 400;

	while (h < maxPart) {
		if (h < 400 && drawMid)RainParticles[h].x = RainParticles[h].x - RW;
		if (h < 400 && drawFore)RainParticlesFore[h].x = RainParticlesFore[h].x - RW;
		RainParticlesBack[h].x = RainParticlesBack[h].x - RW;

		h++;
	}

	h = 0;
	//BACK
	while (h < maxPart) {
		//FORE
		if (h < 400 && drawFore) {
			if (RainParticlesFore[h].life > 0 && RainParticlesFore[h].active) {
				RainParticlesFore[h].life -= 4;
				RainParticlesFore[h].translay += 2;
				if (RainParticlesFore[h].translay > RainParticlesFore[h].transhold) {
					RainParticlesFore[h].translay = 0;
					RainParticlesFore[h].trans += 2;
				}

				int setRainTrans = RainParticlesFore[h].trans + 8 + Random(10) + totalTrans;
				if (setRainTrans > 100) {
					setRainTrans = 100;
				}

				if (RainParticlesFore[h].y > RH + 30
					|| RainParticlesFore[h].trans == 100) {
					RainParticlesFore[h].active = false;
				} else {
					//int thick =3;
					//DRAW LINE
					int alpha = int(float((255 * (100 - setRainTrans))) / 100.0);

					int x1 = RainParticlesFore[h].x;
					int y1 = RainParticlesFore[h].y;
					int x2 = RainParticlesFore[h].x + (RainParticlesFore[h].fx * 2);
					int y2 = RainParticlesFore[h].y + (RainParticlesFore[h].fy * 2);

					DrawLineCustom(x1, y1, x2, y2, graphic, 255 - 120, 255 - 120, 255 - 120, alpha - 80, 6);
					DrawLineCustom(x1 - 1, y1, x2 - 1, y2, graphic, 255 - 120, 255 - 120, 255 - 120, alpha - 80, 6);

					DrawLineCustom((x1 - rotX), y1 - rotY, (x2 - rotX) - rotAngle, y2 - rotY, graphic, 255 - 120, 255 - 120, 255 - 120, (alpha - 80) - rotTrans, 6);
					DrawLineCustom((x1 - 1) - rotX, y1 - rotY, ((x2 - 1) - rotX) - rotAngle, y2 - rotY, graphic, 255 - 120, 255 - 120, 255 - 120, (alpha - 80) - rotTrans, 6);

					RainParticlesFore[h].x += RainParticlesFore[h].fx;
					RainParticlesFore[h].y += RainParticlesFore[h].fy;
				}
			} else {
				RainParticlesFore[h].life = 0;
				RainParticlesFore[h].active = false;
			}
		}
		//FORE

		//MID
		if (!h && drawMid /* h < drawMid */) {
			if (RainParticles[h].life > 0 && RainParticles[h].active) {
				RainParticles[h].life -= 4;

				RainParticles[h].translay += 2;
				if (RainParticles[h].translay > RainParticles[h].transhold) {
					RainParticles[h].translay = 0;
					RainParticles[h].trans += 3;
				}


				int setRainTrans = RainParticles[h].trans + 4 + Random(5) + totalTrans;
				if (setRainTrans > 100) {
					setRainTrans = 100;
				}

				if (RainParticles[h].y > RH + 30
					|| RainParticles[h].trans == 100) {
					RainParticles[h].active = false;
				} else {
					//int thick=2;
					//DRAW LINE
					int alpha = int(float((255 * (100 - setRainTrans))) / 100.0);

					int x1 = RainParticles[h].x;
					int y1 = RainParticles[h].y;
					int x2 = RainParticles[h].x + RainParticles[h].fx;
					int y2 = RainParticles[h].y + RainParticles[h].fy;

					DrawLineCustom(x1, y1, x2, y2, graphic, 255 - 40, 255 - 40, 255 - 40, alpha, 6);
					DrawLineCustom(x1 - 1, y1, x2 - 1, y2, graphic, 255 - 40, 255 - 40, 255 - 40, alpha, 6);

					DrawLineCustom((x1)-rotX, y1 - rotY, (x2 - rotX) - rotAngle, y2 - rotY, graphic, 255 - 40, 255 - 40, 255 - 40, alpha - rotTrans, 6);
					DrawLineCustom((x1 - 1) - rotX, y1 - rotY, ((x2 - 1) - rotX) - rotAngle, y2 - rotY, graphic, 255 - 40, 255 - 40, 255 - 40, alpha - rotTrans, 6);

					RainParticles[h].x += RainParticles[h].fx;
					RainParticles[h].y += RainParticles[h].fy;
				}

			} else {
				RainParticles[h].life = 0;
				RainParticles[h].active = false;
			}
		}
		//MID
		if (h < 800 && drawBack) {
			if (RainParticlesBack[h].life > 0 && RainParticlesBack[h].active) {
				RainParticlesBack[h].life -= 4;
				RainParticlesBack[h].translay += 2;
				if (RainParticlesBack[h].translay > RainParticlesBack[h].transhold) {
					RainParticlesBack[h].translay = 0;
					RainParticlesBack[h].trans++;
				}

				int setRainTrans = RainParticlesBack[h].trans + totalTrans;//+8+Random(10);
				if (setRainTrans > 100) {
					setRainTrans = 100;
				}

				if (RainParticlesBack[h].y > RH + 30
					|| RainParticlesBack[h].trans == 100) {
					RainParticlesBack[h].active = false;
				} else {
					//int thick =1;
					//DRAW LINE
					int x1 = RainParticlesBack[h].x;
					int y1 = RainParticlesBack[h].y;
					int x2 = RainParticlesBack[h].x + RainParticlesBack[h].fx;
					int y2 = RainParticlesBack[h].y + RainParticlesBack[h].fy;

					int alpha = int(float((255 * (100 - setRainTrans))) / 100.0);
					DrawLineCustom(x1, y1, x2, y2, graphic, 255 - 80, 255 - 80, 255 - 80, alpha, 3);
					DrawLineCustom((x1 - rotX), y1 - rotY, (x2 - rotX) - rotAngle, y2 - rotY, graphic, 255 - 80, 255 - 80, 255 - 80, alpha - rotTrans, 3);

					RainParticlesBack[h].x += RainParticlesBack[h].fx;
					RainParticlesBack[h].y += RainParticlesBack[h].fy;
				}
			} else {
				RainParticlesBack[h].life = 0;
				RainParticlesBack[h].active = false;
			}
		}
		h++;
	}

	// BACK
	_engine->ReleaseBitmapSurface(src);
}


void AGSWaves::DrawLineCustom(int x1, int y1, int x2, int y2, int graphic, int setR, int setG, int setB, int setA, int TranDif) {
	int ALine = 0;
	BITMAP *src = _engine->GetSpriteGraphic(graphic);
	int32 src_width = 640;
	int32 src_height = 360;
	int32 src_depth = 32;
	_engine->GetBitmapDimensions(src, &src_width, &src_height, &src_depth);
	uint32 **sprite_pixels = (uint32 **)_engine->GetRawBitmapSurface(src);

	int DiffA = -26;

	int x, y;
	int xe;
	int ye;
	int dx = x2 - x1;
	int dy = y2 - y1;
	int dx1 = abs(dx);
	int dy1 = abs(dy);
	int px = (2 * dy1) - dx1;
	int py = (2 * dx1) - dy1;
	if (dy1 <= dx1) {
		if (dx >= 0) {
			x = x1;
			y = y1;
			xe = x2;
		} else {
			x = x2; y = y2; xe = x1;
		}

		int xx2 = x - 320;
		int yy2 = y;

		if (xx2 < 0 || xx2 > src_width - 1 || yy2 > src_height - 1 || yy2 < 0) {
		} else {
			sprite_pixels[yy2][xx2] = SetColorRGBA(setR, setG, setB, setA + DiffA + (ALine * TranDif));
		}

		int xx3 = x + 320;
		int yy3 = y;

		if (xx3 < 0 || xx3 > src_width - 1 || yy3 > src_height - 1 || yy3 < 0) {
		} else {
			sprite_pixels[yy3][xx3] = SetColorRGBA(setR, setG, setB, setA + DiffA + (ALine * TranDif));
		}

		int xx = x;
		int yy = y;

		if (xx < 0 || xx > src_width - 1 || yy > src_height - 1 || yy < 0) {
		} else {
			sprite_pixels[yy][xx] = SetColorRGBA(setR, setG, setB, setA + (ALine * TranDif));
			ALine++;
		}

		int i = 0;
		while (x < xe) {
			x = x + 1;
			if (px < 0) {
				px = px + 2 * dy1;
			} else {
				if ((dx < 0 && dy < 0) || (dx > 0 && dy > 0)) {
					y = y + 1;
				} else {
					y = y - 1;
				}
				px = px + 2 * (dy1 - dx1);
			}

			xx2 = x - 320;
			yy2 = y;
			if (xx2 < 0 || xx2 > src_width - 1 || yy2 > src_height - 1 || yy2 < 0) {
			} else {
				sprite_pixels[yy2][xx2] = SetColorRGBA(setR, setG, setB, setA + DiffA + (ALine * TranDif));
			}
			xx3 = x + 320;
			yy3 = y;
			if (xx3 < 0 || xx3 > src_width - 1 || yy3 > src_height - 1 || yy3 < 0) {
			} else {
				sprite_pixels[yy3][xx3] = SetColorRGBA(setR, setG, setB, setA + DiffA + (ALine * TranDif));
			}

			xx = x;
			yy = y;
			if (xx < 0 || xx > src_width - 1 || yy > src_height - 1 || yy < 0) {
			} else {
				sprite_pixels[yy][xx] = SetColorRGBA(setR, setG, setB, setA + (ALine * TranDif));
				ALine++;
			}

			i++;
		}
	} else {
		if (dy >= 0) {
			x = x1;
			y = y1;
			ye = y2 - 1;
		} else {
			// Line is drawn top to bottom
			x = x2;
			y = y2;
			ye = y1 - 1;
		}

		int xx2 = x - 320;
		int yy2 = y;

		if (xx2 < 0 || xx2 > src_width - 1 || yy2 > src_height - 1 || yy2 < 0) {
		} else {
			sprite_pixels[yy2][xx2] = SetColorRGBA(setR, setG, setB, setA + DiffA + (ALine * TranDif));
		}

		int xx3 = x + 320;
		int yy3 = y;

		if (xx3 < 0 || xx3 > src_width - 1 || yy3 > src_height - 1 || yy3 < 0) {
		} else {
			sprite_pixels[yy3][xx3] = SetColorRGBA(setR, setG, setB, setA + DiffA + (ALine * TranDif));
		}
		int xx = x;
		int yy = y;

		if (xx < 0 || xx > src_width - 1 || yy > src_height - 1 || yy < 0) {
		} else {
			sprite_pixels[yy][xx] = SetColorRGBA(setR, setG, setB, setA + (ALine * TranDif));
			ALine++;
		}

		int i = 0;
		while (y < ye) {
			y = y + 1;
			if (py <= 0) {
				py = py + (2 * dx1);
			} else {
				if ((dx < 0 && dy < 0) || (dx > 0 && dy > 0)) {
					x = x + 1;
				} else {
					x = x - 1;
				}
				py = py + 2 * (dx1 - dy1);
			}
			xx2 = x - 320;
			yy2 = y;
			if (xx2 < 0 || xx2 > src_width - 1 || yy2 > src_height - 1 || yy2 < 0) {
			} else {
				sprite_pixels[yy2][xx2] = SetColorRGBA(setR, setG, setB, setA + DiffA + (ALine * TranDif));
			}
			xx3 = x + 320;
			yy3 = y;
			if (xx3 < 0 || xx3 > src_width - 1 || yy3 > src_height - 1 || yy3 < 0) {
			} else {
				sprite_pixels[yy3][xx3] = SetColorRGBA(setR, setG, setB, setA + DiffA + (ALine * TranDif));
			}
			xx = x;
			yy = y;
			if (xx < 0 || xx > src_width - 1 || yy > src_height - 1 || yy < 0) {
			} else {
				sprite_pixels[yy][xx] = SetColorRGBA(setR, setG, setB, setA + (ALine * TranDif));
				ALine++;
			}
			i++;
		}
	}

	_engine->ReleaseBitmapSurface(src);
}

void AGSWaves::CreateParticle(int xx, int yy, int ForceX, int ForceY) {
	int h = 0;
	bool foundparticle = false;
	int fid = -1;
	while (h <= dsize && !foundparticle) {
		if (particles[h].active == false) {
			foundparticle = true;
			fid = h;
		}
		h++;
	}

	if (foundparticle) {
		int d = fid;
		particles[d].x = xx;
		particles[d].y = yy;
		particles[d].dx = 0;
		particles[d].dy = 0;
		particles[d].life = 20000;
		particles[d].transp = 55 + Random(10);
		particles[d].active = true;
		particles[d].mlay = 4 + Random(2);
		particles[d].timlay = 0;
		particles[d].translay = 0;
		particles[d].translayHold = 19 + Random(15);
		particles[d].width = 2 + Random(2);
		particles[d].height = particles[d].width;
		particles[d].fx = 0;
		particles[d].fy = 0;
		particles[d].doingcircle = false;
		particles[d].angle = 0.0;
		particles[d].radius = 4.0 + float(Random(6));
		particles[d].doingCircleChance = Random(200);
		particles[d].angleLay = 0.0;
		particles[d].frame = 0;
		particles[d].anglespeed = float(Random(20)) / 100.0;
		WForceX[d] = ForceX;
		WForceY[d] = ForceY;
		if (dsize < (raysize - 1)) dsize++;
	}
}

void AGSWaves::CreateParticle2(int xx, int yy, int ForceX, int ForceY) {
	int h = 0;
	bool foundparticle = false;
	int fid = -1;
	while (h <= dsize2 && !foundparticle) {
		if (particles2[h].active == false) {
			foundparticle = true;
			fid = h;
		}
		h++;
	}

	if (foundparticle) {
		int d = fid;
		particles2[d].x = xx;
		particles2[d].y = yy;
		particles2[d].dx = 0;
		particles2[d].dy = 0;
		particles2[d].life = 20000;
		particles2[d].transp = 65 + Random(15);
		particles2[d].active = true;
		particles2[d].mlay = 4 + Random(2);
		particles2[d].timlay = 0;
		particles2[d].translay = 0;
		particles2[d].translayHold = 19 + Random(15);
		particles2[d].width = 16;
		particles2[d].height = particles[d].width;
		particles2[d].fx = 0;
		particles2[d].fy = 0;
		particles2[d].doingcircle = false;
		particles2[d].angle = 0.0;
		particles2[d].radius = 4.0 + float(Random(6));
		particles2[d].doingCircleChance = Random(200);
		particles2[d].angleLay = 0.0;
		particles2[d].frame = 0;
		particles2[d].anglespeed = float(Random(20)) / 100.0;
		WForceX[d + 200] = ForceX;
		WForceY[d + 200] = ForceY;
		if (dsize2 < (raysize2 - 1)) dsize2++;
	}
}

void AGSWaves::CreateParticleF(int xx, int yy, int ForceX, int ForceY) {
	int h = 0;
	bool foundparticle = false;
	int fid = -1;
	while (h <= dsizeF && !foundparticle) {
		if (particlesF[h].active == false) {
			foundparticle = true;
			fid = h;
		}
		h++;
	}

	if (foundparticle) {
		int d = fid;
		particlesF[d].x = xx;
		particlesF[d].y = yy;
		particlesF[d].dx = (-1) + Random(1);
		particlesF[d].dy = (-1) + Random(1);
		particlesF[d].life = 20000;
		particlesF[d].transp = 45 + Random(10);
		particlesF[d].active = true;
		particlesF[d].mlay = 4 + Random(2);
		particlesF[d].timlay = 0;
		particlesF[d].translay = 0;
		particlesF[d].translayHold = 19 + Random(15);
		particlesF[d].width = 8 + Random(2);
		particlesF[d].height = particlesF[d].width;
		particlesF[d].fx = 0;
		particlesF[d].fy = 0;
		particlesF[d].doingcircle = false;
		particlesF[d].angle = 0.0;
		particlesF[d].radius = 4.0 + float(Random(6));
		particlesF[d].doingCircleChance = Random(200);
		particlesF[d].angleLay = 0.0;
		WForceX[d + 100] = ForceX;
		WForceY[d + 100] = ForceY;
		particlesF[d].frame = 0;
		if (dsizeF < (raysizeF - 1)) dsizeF++;

	}
}

void AGSWaves::CreateDustParticle(int xx, int yy) {
	int h = 0;
	bool founddust = false;
	int fid = -1;
	while (h <= dsizeDust && !founddust) {
		if (dusts[h].active == false) {
			founddust = true;
			fid = h;
		}
		h++;
	}

	if (founddust) {
		int d = fid;
		dusts[d].x = xx;
		dusts[d].y = yy;
		dusts[d].dx = (-1) + Random(1);
		dusts[d].dy = (-1) + Random(1);
		dusts[d].life = 20000;
		dusts[d].transp = 55 + Random(10);
		dusts[d].active = true;
		dusts[d].mlay = 4 + Random(2);
		dusts[d].timlay = 0;
		dusts[d].translay = 0;
		dusts[d].translayHold = 19 + Random(15);
		if (dsizeDust < (raysizeDust - 1)) dsizeDust++;
	}
}

void AGSWaves::CreateRainParticleMid(int x, int y, int fx, int fy, int maxpart) {
	int s = 0;

	while (s < maxpart) {
		if (!RainParticles[s].active) {
			RainParticles[s].active = true;
			RainParticles[s].x = x;
			RainParticles[s].y = y;
			RainParticles[s].fx = fx;
			RainParticles[s].fy = fy;
			RainParticles[s].life = 2000;
			RainParticles[s].trans = 70 + Random(25);
			RainParticles[s].transhold = Random(3);
			RainParticles[s].translay = 0;
			return;
		}
		s++;
	}
}

void AGSWaves::CreateRainParticleFore(int x, int y, int fx, int fy, int maxpart) {
	int s = 0;

	while (s < maxpart) {
		if (!RainParticlesFore[s].active) {
			RainParticlesFore[s].active = true;
			RainParticlesFore[s].x = x;
			RainParticlesFore[s].y = y;
			RainParticlesFore[s].fx = fx;//int(1.5*float(fx));
			RainParticlesFore[s].fy = fy;//int(1.5*float(fy));
			RainParticlesFore[s].life = 2000;
			RainParticlesFore[s].trans = 75 + Random(15);
			RainParticlesFore[s].transhold = Random(3);
			RainParticlesFore[s].translay = 0;
			return;
		}
		s++;
	}
}

void AGSWaves::CreateRainParticleBack(int x, int y, int fx, int fy, int maxpart) {
	int s = 0;

	while (s < maxpart) {
		if (!RainParticlesBack[s].active) {
			RainParticlesBack[s].active = true;
			RainParticlesBack[s].x = x;
			RainParticlesBack[s].y = y;
			if (fx == 0) fx = 1;
			if (fy == 0) fy = 1;
			RainParticlesBack[s].fx = fx / 2;
			RainParticlesBack[s].fy = fy / 2;
			RainParticlesBack[s].life = 2000;
			RainParticlesBack[s].trans = 70 + Random(15);
			RainParticlesBack[s].transhold = 2 + Random(3);
			RainParticlesBack[s].translay = 0;
			return;
		}
		s++;
	}
}

} // namespace AGSWaves
} // namespace Plugins
} // namespace AGS3
