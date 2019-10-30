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
 *
 *              Originally written by Syn9 in FreeBASIC with SDL
 *              http://syn9.thehideoutgames.com/index_backup.php
 *
 *            Ported to plain C for GCW-Zero handheld by Dmitry Smagin
 *                http://github.com/dmitrysmagin/griffon_legend
 *
 *
 *                 Programming/Graphics: Daniel "Syn9" Kennedy
 *                     Music/Sound effects: David Turner
 *
 *                   Beta testing and gameplay design help:
 *                    Deleter, Cha0s, Aether Fox, and Kiz
 *
 */

#include "griffon/griffon.h"
#include "griffon/config.h"

namespace Griffon {

// memo

/*
 chests
  0 - regular flask
 11 - key chest
 14 - blue flask chest
 15 - lightning chest
 16 - armour chest
 17 - citadel master key
 18 - sword3
 19 - shield3
 20 - armour3

*/

// element tile locations
const int elementmap[15][20] = {
	{  2, 2, 2, 2, -1, -1, -1, 2, 2, 2, 2, 2, 2, -1, -1, -1, -1, -1, -1, -1 },
	{  2, -1, -1, -1, -1, -1, -1, 2, 2, 2, 2, 2, 2, -1, -1, -1, -1, -1, -1, -1 },
	{  2, -1, 2, 2, -1, -1, -1, 2, 2, 2, 2, 2, 2, -1, -1, -1, -1, -1, -1, -1 },
	{  2, -1, 2, -1, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{  2, 2, 2, 2, 2, -1, -1, -1, 2, -1, -1, 2, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ -1, -1, -1, -1, 2, -1, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ -1, -1, -1, -1, -1, 0, 0, 2, 2, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ -1, -1, -1, -1, -1, 2, 2, 2, 2, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ -1, -1, -1, -1, -1, 2, 2, 2, 2, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 }
};


void GriffonEngine::updateAnims() {
	for (int i = 0; i <= _lastObj; i++) {
		int nframes = _objectInfo[i][0];
		int o_animspd = _objectInfo[i][3];
		float frame = _objectFrame[i][0];
		// _objectinfo[i][6] = 0; // ?? out of bounds

		if (nframes > 1) {
			frame += o_animspd / 50 * _fpsr;
			while (frame >= nframes)
				frame -= nframes;

			int cframe = (int)frame; // truncate fractional part
			if (cframe < 0)
				cframe = 0;

			_objectFrame[i][0] = frame;
			_objectFrame[i][1] = cframe;
		}
	}
}

void GriffonEngine::updateY() {
	for (int i = 0; i <= 2400; i++)
		_ysort[i] = -1;

	int ff = (int)(_player.py * 10);
	if (ff < 0) // HACKFIX or _ysort[yy] may go out of bounds
		ff = 0;
	_player.ysort = ff;
	_ysort[ff] = 0;

	_firsty = 2400;
	_lasty = 0;

	for (int i = 1; i <= _lastnpc; i++) {
		int yy = (int)(_npcinfo[i].y * 10);

		do {
			if (_ysort[yy] == -1 || yy == 2400)
				break;
			++yy;
		} while (1);

		_ysort[yy] = i;
		if (yy < _firsty)
			_firsty = yy;
		if (yy > _lasty)
			_lasty = yy;
	}
}

void GriffonEngine::updateNPCs() {
	for (int i = 1; i <= _lastnpc; i++) {
		if (_npcinfo[i].hp > 0) {
			//  is npc walking
			int pass = 0;
			if (!_npcinfo[i].attacking)
				pass = 1;
			if (_npcinfo[i].spriteset == kMonsterFireHydra)
				pass = 1;
			if (pass == 1) {
				bool moveup = false;
				bool movedown = false;
				bool moveleft = false;
				bool moveright = false;

				float npx = _npcinfo[i].x;
				float npy = _npcinfo[i].y;

				float onpx = npx;
				float onpy = npy;

				float wspd = _npcinfo[i].walkspd / 4;

				if (_npcinfo[i].spriteset == kMonsterDragon2)
					wspd = wspd * 2;
				int wdir = _npcinfo[i].walkdir;

				int mode = _npcinfo[i].movementmode;

				float xdif = _player.px - npx;
				float ydif = _player.py - npy;

				if (abs(xdif) < 4 * 16 && abs(ydif) < 4 * 16 && mode < 3)
					mode = 0;
				if (_npcinfo[i].hp < _npcinfo[i].maxhp * 0.25)
					mode = 3;

				if (_npcinfo[i].pause > _ticks)
					mode = -1;
				if (_npcinfo[i].spriteset == kMonsterOneWing && _npcinfo[i].castpause > _ticks)
					mode = -1;

				if (mode == 3) {
					mode = 1;
					if (abs(xdif) < 4 * 16 && abs(ydif) < 4 * 16)
						mode = 3;
				}

				bool checkpass = false;

				// npc  AI CODE
				// --------------

				// *** aggressive
				if (mode == 0) {
					wspd = _npcinfo[i].walkspd / 2;

					xdif = _player.px - npx;
					ydif = _player.py - npy;

					if (abs(xdif) > abs(ydif)) {
						if (xdif < 4)
							wdir = 2;
						if (xdif > -4)
							wdir = 3;
					} else {
						if (ydif < 4)
							wdir = 0;
						if (ydif > -4)
							wdir = 1;
					}

					if (xdif < 4)
						moveleft = true;
					if (xdif > -4)
						moveright = true;
					if (ydif < 4)
						moveup = true;
					if (ydif > -4)
						movedown = true;
				}
				// *******************

				// *** defensive
				if (mode == 1) {

					int movingdir = _npcinfo[i].movingdir;

					if (_npcinfo[i].ticks > _ticks + 100000)
						_npcinfo[i].ticks = _ticks;

					if (_npcinfo[i].ticks < _ticks) {
						_npcinfo[i].ticks = _ticks + 2000;
						movingdir = (int)(RND() * 8);
						_npcinfo[i].movingdir = movingdir;
					}

					if (movingdir == 0) {
						wdir = 2; // left
						moveup = true;
						moveleft = true;
					} else if (movingdir == 1) {
						wdir = 0; // up
						moveup = true;
					} else if (movingdir == 2) {
						wdir = 3; // right
						moveup = true;
						moveright = true;
					} else if (movingdir == 3) {
						wdir = 3; // right
						moveright = true;
					} else if (movingdir == 4) {
						wdir = 3; // right
						moveright = true;
						movedown = true;
					} else if (movingdir == 5) {
						wdir = 1; // down
						movedown = true;
					} else if (movingdir == 6) {
						wdir = 2; // left
						movedown = true;
						moveleft = true;
					} else if (movingdir == 7) {
						wdir = 2; // left
						moveleft = true;
					}

					checkpass = true;
				}
				// *******************

				// *** run away
				if (mode == 3) {
					wspd = _npcinfo[i].walkspd / 2;

					xdif = _player.px - npx;
					ydif = _player.py - npy;

					if (abs(xdif) > abs(ydif)) {
						if (xdif < 4)
							wdir = 3;
						if (xdif > -4)
							wdir = 2;
					} else {
						if (ydif < 4)
							wdir = 1;
						if (ydif > -4)
							wdir = 0;
					}

					if (xdif < 4)
						moveright = true;
					if (xdif > -4)
						moveleft = true;
					if (ydif < 4)
						movedown = true;
					if (ydif > -4)
						moveup = true;
				}
				// *******************

				// -------------- ?? move*** vs movin***
				bool movinup = false;
				bool movindown = false;
				bool movinleft = false;
				bool movinright = false;

				float xp = (npx / 2 + 6);
				float yp = (npy / 2 + 10);

				if (_npcinfo[i].spriteset == kMonsterDragon2)
					wspd = wspd * 2;

				float ii = wspd * _fpsr;
				if (ii < 1)
					ii = 1;

				if (moveup) {
					int sx = xp;
					int sy = yp - ii;
					uint32 *temp = (uint32 *)_clipBg->getBasePtr(sx, sy);
					uint32 dq = *temp;
					if (_npcinfo[i].spriteset == kMonsterFinalBoss)
						dq = 0;

					if (dq == 0)
						movinup = true;
					if (dq > 0) {
						sx = xp - ii;
						sy = yp - ii;
						temp = (uint32 *)_clipBg->getBasePtr(sx, sy);
						dq = *temp;
						if (_npcinfo[i].spriteset == kMonsterFinalBoss)
							dq = 0;
						if (dq == 0) {
							movinup = true;
							movinleft = true;
						}
					}
					if (dq > 0) {
						sx = xp + ii;
						sy = yp - ii;
						temp = (uint32 *)_clipBg->getBasePtr(sx, sy);
						dq = *temp;
						if (_npcinfo[i].spriteset == kMonsterFinalBoss)
							dq = 0;
						if (dq == 0) {
							movinup = true;
							movinright = true;
						}
					}
				}

				if (movedown) {
					int sx = xp;
					int sy = yp + ii;
					uint32 *temp = (uint32 *)_clipBg->getBasePtr(sx, sy);
					uint32 dq = *temp;
					if (_npcinfo[i].spriteset == kMonsterFinalBoss)
						dq = 0;
					if (dq == 0)
						movindown = true;
					if (dq > 0) {
						sx = xp - ii;
						sy = yp + ii;
						temp = (uint32 *)_clipBg->getBasePtr(sx, sy);
						dq = *temp;
						if (_npcinfo[i].spriteset == kMonsterFinalBoss)
							dq = 0;
						if (dq == 0) {
							movindown = true;
							movinleft = true;
						}
					}
					if (dq > 0) {
						sx = xp + ii;
						sy = yp + ii;
						temp = (uint32 *)_clipBg->getBasePtr(sx, sy);
						dq = *temp;
						if (_npcinfo[i].spriteset == kMonsterFinalBoss)
							dq = 0;
						if (dq == 0) {
							movindown = true;
							movinright = true;
						}
					}
				}

				if (moveleft) {
					int sx = xp - ii;
					int sy = yp;
					uint32 *temp = (uint32 *)_clipBg->getBasePtr(sx, sy);
					uint32 dq = *temp;
					if (_npcinfo[i].spriteset == kMonsterFinalBoss)
						dq = 0;
					if (dq == 0)
						movinleft = true;
					if (dq > 0) {
						sx = xp - ii;
						sy = yp - ii;
						temp = (uint32 *)_clipBg->getBasePtr(sx, sy);
						dq = *temp;
						if (_npcinfo[i].spriteset == kMonsterFinalBoss)
							dq = 0;
						if (dq == 0) {
							movinleft = true;
							movinup = true;
						}
					}
					if (dq > 0) {
						sx = xp - ii;
						sy = yp + ii;
						temp = (uint32 *)_clipBg->getBasePtr(sx, sy);
						dq = *temp;
						if (_npcinfo[i].spriteset == kMonsterFinalBoss)
							dq = 0;
						if (dq == 0) {
							movinleft = true;
							movindown = true;
						}
					}
				}

				if (moveright) {
					int sx = xp + ii;
					int sy = yp;
					uint32 *temp = (uint32 *)_clipBg->getBasePtr(sx, sy);
					uint32 dq = *temp;
					if (_npcinfo[i].spriteset == kMonsterFinalBoss)
						dq = 0;
					if (dq == 0)
						movinright = true;
					if (dq > 0) {
						sx = xp + ii;
						sy = yp - ii;
						temp = (uint32 *)_clipBg->getBasePtr(sx, sy);
						dq = *temp;
						if (_npcinfo[i].spriteset == kMonsterFinalBoss)
							dq = 0;
						if (dq == 0) {
							movinright = true;
							movinup = true;
						}
					}
					if (dq > 0) {
						sx = xp + ii;
						sy = yp + ii;
						temp = (uint32 *)_clipBg->getBasePtr(sx, sy);
						dq = *temp;
						if (_npcinfo[i].spriteset == kMonsterFinalBoss)
							dq = 0;
						if (dq == 0) {
							movinright = true;
							movindown = true;
						}
					}
				}

				if (movinup)
					npy -= wspd * _fpsr;
				if (movindown)
					npy += wspd * _fpsr;
				if (movinleft)
					npx -= wspd * _fpsr;
				if (movinright)
					npx += wspd * _fpsr;

				if (checkpass) {
					pass = 0;
					if (npx >= _npcinfo[i].x1 * 16 - 8 && npx <= _npcinfo[i].x2 * 16 + 8 && npy >= _npcinfo[i].y1 * 16 - 8 && npy <= _npcinfo[i].y2 * 16 + 8)
						pass = 1;
					if (pass == 0) {
						npx = onpx;
						npy = onpy;
						_npcinfo[i].ticks = _ticks;
					}
				}

				float aspd = wspd;

				if (_npcinfo[i].spriteset == kMonsterDragon2)
					aspd = wspd / 2;

				xp = (npx / 2 + 6);
				yp = (npy / 2 + 10);

				int sx = xp;
				int sy = yp;
				uint32 *temp = (uint32 *)_clipBg->getBasePtr(sx, sy);
				uint32 bgc = *temp;

				float anpx = npx + 12;
				float anpy = npy + 20;

				int lx = (int)anpx / 16;
				int ly = (int)anpy / 16;

				if (_triggerloc[lx][ly] > -1)
					bgc = 1;
				if (_npcinfo[i].spriteset == kMonsterFinalBoss)
					bgc = 0;

				bool rst = false;

				if (_npcinfo[i].spriteset == kMonsterFinalBoss) {
					if (npx < 40 || npx > 280 || npy < 36 || npy > 204)
						rst = true;
				}

				if (bgc > 0 || rst) {
					npx = onpx;
					npy = onpy;
				}

				_npcinfo[i].x = npx;
				_npcinfo[i].y = npy;

				_npcinfo[i].walkdir = wdir;
				_npcinfo[i].moving = false;

				if (npx != onpx || npy != onpy)
					_npcinfo[i].moving = true;

				if (_npcinfo[i].moving) {
					float frame = _npcinfo[i].frame;

					frame += aspd * _fpsr;
					while (frame >= 16)
						frame -= 16;

					int cframe = (int)(frame);
					if (cframe < 0)
						cframe = 0;

					_npcinfo[i].frame = frame;
					_npcinfo[i].cframe = cframe;
				}

				// spriteset1 specific
				if (_npcinfo[i].spriteset == kMonsterBabyDragon && _npcinfo[i].attackattempt < _ticks) {
					if (_npcinfo[i].attacknext < _ticks && _npcinfo[i].pause < _ticks && !_npcinfo[i].attacking) {
						npx = _npcinfo[i].x;
						npy = _npcinfo[i].y;

						xdif = _player.px - npx;
						ydif = _player.py - npy;

						if (abs(xdif) < 20 && abs(ydif) < 20) {
							_npcinfo[i].attackattempt = _ticks + 100;
							if ((int)(RND() * 2) == 0) {
								if (config.effects) {
									int snd = playSound(_sfx[kSndEnemyHit]);
									setChannelVolume(snd, config.effectsvol);
								}

								_npcinfo[i].attacking = true;
								_npcinfo[i].attackframe = 0;
							}
						}
					}
				}

				bool dospell = false;

				// onewing specific
				if (_npcinfo[i].spriteset == kMonsterOneWing) {
					if (_npcinfo[i].attacknext < _ticks && _npcinfo[i].pause < _ticks && !_npcinfo[i].attacking) {
						npx = _npcinfo[i].x;
						npy = _npcinfo[i].y;

						xdif = _player.px - npx;
						ydif = _player.py - npy;

						if (abs(xdif) < 24 && abs(ydif) < 24) {
							float dist = sqrt(xdif * xdif + ydif * ydif);

							if ((dist) < 24) {
								if (config.effects) {
									int snd = playSound(_sfx[kSndBite]);
									setChannelVolume(snd, config.effectsvol);
								}

								_npcinfo[i].attacking = true;
								_npcinfo[i].attackframe = 0;

								_npcinfo[i].headtargetx[0] = _player.px + 12;
								_npcinfo[i].headtargety[0] = _player.py - 4;
							}
						}

					}

					dospell = false;

					if (!_npcinfo[i].attacking && _npcinfo[i].castpause < _ticks) {
						_npcinfo[i].swayspd = _npcinfo[i].swayspd + _npcinfo[i].swayspd / 200 * _fpsr;
						if (_npcinfo[i].swayspd > 15) {
							dospell = true;
							_npcinfo[i].swayspd = 1;
						}

						// sway code
						_npcinfo[i].swayangle = _npcinfo[i].swayangle + _npcinfo[i].swayspd * _fpsr;
						if (_npcinfo[i].swayangle >= 360)
							_npcinfo[i].swayangle = _npcinfo[i].swayangle - 360;

						_npcinfo[i].headtargetx[0] = _npcinfo[i].x + (24 - _npcinfo[i].swayspd / 2) * sin(3.14159 / 180 * _npcinfo[i].swayangle) + 12;
						_npcinfo[i].headtargety[0] = _npcinfo[i].y - 36 + 16 + 8 * sin(3.14159 * 2 / 180 * _npcinfo[i].swayangle);
					}

					if (dospell) {
						_npcinfo[i].pause = _ticks + 3000;
						_npcinfo[i].attacknext = _ticks + 4500;
						_npcinfo[i].castpause = _ticks + 4500;

						castSpell(3, _npcinfo[i].x, _npcinfo[i].y, _npcinfo[i].x, _npcinfo[i].y, i);

						_npcinfo[i].headtargetx[0] = _npcinfo[i].x;
						_npcinfo[i].headtargety[0] = _npcinfo[i].y - 36 + 16;
					}

					// targethead code
					xdif = _npcinfo[i].bodysection[7].x - _npcinfo[i].headtargetx[0];
					ydif = _npcinfo[i].bodysection[7].y - _npcinfo[i].headtargety[0];


					_npcinfo[i].bodysection[7].x = _npcinfo[i].bodysection[7].x  - xdif * 0.4 * _fpsr;
					_npcinfo[i].bodysection[7].y = _npcinfo[i].bodysection[7].y  - ydif * 0.4 * _fpsr;


					_npcinfo[i].bodysection[0].x = _npcinfo[i].x + 12;
					_npcinfo[i].bodysection[0].y = _npcinfo[i].y + 12;

					for (int f = 6; f >= 1; f--) {
						xdif = _npcinfo[i].bodysection[f + 1].x - _npcinfo[i].bodysection[f - 1].x;
						ydif = _npcinfo[i].bodysection[f + 1].y - _npcinfo[i].bodysection[f - 1].y;

						float tx = _npcinfo[i].bodysection[f - 1].x + xdif / 2;
						float ty = _npcinfo[i].bodysection[f - 1].y + ydif / 2;

						_npcinfo[i].bodysection[f].x = _npcinfo[i].bodysection[f].x - (_npcinfo[i].bodysection[f].x - tx) / 3;
						_npcinfo[i].bodysection[f].y = _npcinfo[i].bodysection[f].y - (_npcinfo[i].bodysection[f].y - ty) / 3;
					}
				}

				// boss1 specific and blackknight
				if (_npcinfo[i].spriteset == kMonsterBoss1 || _npcinfo[i].spriteset == kMonsterBlackKnight) {
					if (_npcinfo[i].attacknext < _ticks && _npcinfo[i].pause < _ticks && !_npcinfo[i].attacking) {
						_npcinfo[i].attacking = true;
						_npcinfo[i].attackframe = 0;

						castSpell(1, _npcinfo[i].x, _npcinfo[i].y, _player.px, _player.py, i);
					}

					if (_npcinfo[i].castpause < _ticks) {
						castSpell(6, _npcinfo[i].x, _npcinfo[i].y, _player.px, _player.py, i);
						_npcinfo[i].castpause = _ticks + 12000;
					}
				}


				// firehydra specific
				if (_npcinfo[i].spriteset == kMonsterFireHydra) {
					_npcinfo[i].swayspd = 4;

					// sway code
					_npcinfo[i].swayangle = _npcinfo[i].swayangle + _npcinfo[i].swayspd * _fpsr;
					if (_npcinfo[i].swayangle >= 360)
						_npcinfo[i].swayangle = _npcinfo[i].swayangle - 360;

					for (int ff = 0; ff <= 2; ff++) {
						if (_npcinfo[i].hp > 10 * ff * 20) {
							if (_npcinfo[i].pause < _ticks && _npcinfo[i].attacking2[ff] == 0 && _npcinfo[i].attacknext2[ff] < _ticks) {
								npx = _npcinfo[i].x;
								npy = _npcinfo[i].y;

								xdif = _player.px - npx;
								ydif = _player.py - npy;

								if (abs(xdif) < 48 && abs(ydif) < 48) {
									float dist = sqrt(xdif * xdif + ydif * ydif);

									if ((dist) < 36) {
										if (config.effects) {
											int snd = playSound(_sfx[kSndBite]);
											setChannelVolume(snd, config.effectsvol);
										}

										_npcinfo[i].attacking = true;
										_npcinfo[i].attacking2[ff] = 1;
										_npcinfo[i].attackframe2[ff] = 0;

										_npcinfo[i].headtargetx[ff] = _player.px + 12;
										_npcinfo[i].headtargety[ff] = _player.py - 4;

										_npcinfo[i].swayangle = 0;
									}
								}

							}

							if (_npcinfo[i].attacking2[ff] == 0) {
								_npcinfo[i].headtargetx[ff] = _npcinfo[i].x + 38 * sin(3.14159 / 180 * (_npcinfo[i].swayangle + 120 * ff)) + 12;
								_npcinfo[i].headtargety[ff] = _npcinfo[i].y - 46 + 16 + 16 * sin(3.14159 * 2 / 180 * (_npcinfo[i].swayangle + 120 * ff));
							}

							// targethead code
							xdif = _npcinfo[i].bodysection[10 * ff + 9].x - _npcinfo[i].headtargetx[ff];
							ydif = _npcinfo[i].bodysection[10 * ff + 9].y - _npcinfo[i].headtargety[ff];

							_npcinfo[i].bodysection[10 * ff + 9].x = _npcinfo[i].bodysection[10 * ff + 9].x  - xdif * 0.4 * _fpsr;
							_npcinfo[i].bodysection[10 * ff + 9].y = _npcinfo[i].bodysection[10 * ff + 9].y  - ydif * 0.4 * _fpsr;

							_npcinfo[i].bodysection[10 * ff].x = _npcinfo[i].x + 12 + 8 * cos(3.141592 * 2 * (_itemyloc / 16 + ff * 120 / 360));
							_npcinfo[i].bodysection[10 * ff].y = _npcinfo[i].y + 12 + 8 * sin(3.141592 * 2 * (_itemyloc / 16 + ff * 120 / 360));

							for (int f = 8; f >= 1; f--) {
								xdif = _npcinfo[i].bodysection[ff * 10 + f + 1].x - _npcinfo[i].bodysection[ff * 10 + f - 1].x;
								ydif = _npcinfo[i].bodysection[ff * 10 + f + 1].y - _npcinfo[i].bodysection[ff * 10 + f - 1].y;

								float tx = _npcinfo[i].bodysection[ff * 10 + f - 1].x + xdif / 2;
								float ty = _npcinfo[i].bodysection[ff * 10 + f - 1].y + ydif / 2;

								_npcinfo[i].bodysection[ff * 10 + f].x = _npcinfo[i].bodysection[ff * 10 + f].x - (_npcinfo[i].bodysection[ff * 10 + f].x - tx) / 3;
								_npcinfo[i].bodysection[ff * 10 + f].y = _npcinfo[i].bodysection[ff * 10 + f].y - (_npcinfo[i].bodysection[ff * 10 + f].y - ty) / 3;
							}
						}
					}
				}

				// spriteset6 specific
				if (_npcinfo[i].spriteset == kMonsterRedDragon && _npcinfo[i].attackattempt < _ticks) {
					if (_npcinfo[i].attacknext < _ticks && _npcinfo[i].pause < _ticks && !_npcinfo[i].attacking) {
						npx = _npcinfo[i].x;
						npy = _npcinfo[i].y;

						xdif = _player.px - npx;
						ydif = _player.py - npy;

						pass = 0;
						if (abs(xdif) < 48 && abs(ydif) < 6)
							pass = 1;
						if (abs(ydif) < 48 && abs(xdif) < 6)
							pass = 2;

						if (pass > 0) {
							_npcinfo[i].attackattempt = _ticks + 100;
							if ((int)(RND() * 2) == 0) {
								_npcinfo[i].attacking = true;
								_npcinfo[i].attackframe = 0;
								float nnxa = 0, nnya = 0, nnxb = 0, nnyb = 0;

								if (pass == 1 && xdif < 0) {
									nnxa = npx - 8;
									nnya = npy + 4;
									nnxb = npx - 48 - 8;
									nnyb = npy + 4;
								} else if (pass == 1 && xdif > 0) {
									nnxa = npx + 16;
									nnya = npy + 4;
									nnxb = npx + 16 + 48;
									nnyb = npy + 4;
								} else if (pass == 2 && ydif < 0) {
									nnya = npy;
									nnxa = npx + 4;
									nnyb = npy - 48;
									nnxb = npx + 4;
								} else if (pass == 2 && ydif > 0) {
									nnya = npy + 20;
									nnxa = npx + 4;
									nnyb = npy + 20 + 48;
									nnxb = npx + 4;
								}

								castSpell(7, nnxa, nnya, nnxb, nnyb, i);
							}
						}
					}
				}

				// wizard1 specific
				if (_npcinfo[i].spriteset == kMonsterPriest) {
					if (_npcinfo[i].attacknext < _ticks && _npcinfo[i].pause < _ticks && !_npcinfo[i].attacking) {
						_npcinfo[i].attacking = true;
						_npcinfo[i].attackframe = 0;

						castSpell(9, _npcinfo[i].x, _npcinfo[i].y, _player.px, _player.py, i);
					}

					if (_npcinfo[i].castpause < _ticks) {
						// castSpell 6, _npcinfo[i].x, _npcinfo[i].y, _player.px, _player.py, i
						// _npcinfo[i].castpause = _ticks + 12000
					}

				}

				// spriteset6 specific
				if (_npcinfo[i].spriteset == kMonsterYellowDragon && _npcinfo[i].attackattempt < _ticks) {
					if (_npcinfo[i].attacknext < _ticks && _npcinfo[i].pause < _ticks && !_npcinfo[i].attacking) {
						npx = _npcinfo[i].x;
						npy = _npcinfo[i].y;

						xdif = _player.px - npx;
						ydif = _player.py - npy;

						pass = 0;
						if (abs(xdif) < 56 && abs(ydif) < 6)
							pass = 1;
						if (abs(ydif) < 56 && abs(xdif) < 6)
							pass = 2;

						if (pass > 0) {
							_npcinfo[i].attackattempt = _ticks + 100;
							if ((int)(RND() * 2) == 0) {
								_npcinfo[i].attacking = true;
								_npcinfo[i].attackframe = 0;

								float nnxa = 0, nnya = 0, nnxb = 0, nnyb = 0;
								if (pass == 1 && xdif < 0) {
									nnxa = npx - 8;
									nnya = npy + 4;
									nnxb = npx - 56 - 8;
									nnyb = npy + 4;
									_npcinfo[i].walkdir = 2;
								} else if (pass == 1 && xdif > 0) {
									nnxa = npx + 16;
									nnya = npy + 4;
									nnxb = npx + 16 + 56;
									nnyb = npy + 4;
									_npcinfo[i].walkdir = 3;
								} else if (pass == 2 && ydif < 0) {
									nnya = npy;
									nnxa = npx + 4;
									nnyb = npy - 56;
									nnxb = npx + 4;
									_npcinfo[i].walkdir = 0;
								} else if (pass == 2 && ydif > 0) {
									nnya = npy + 20;
									nnxa = npx + 4;
									nnyb = npy + 20 + 56;
									nnxb = npx + 4;
									_npcinfo[i].walkdir = 1;
								}

								castSpell(7, nnxa, nnya, nnxb, nnyb, i);
							}
						}
					}
				}

				// twowing specific
				if (_npcinfo[i].spriteset == kMonsterTwoWing) {
					if (_npcinfo[i].attacknext < _ticks && _npcinfo[i].pause < _ticks && !_npcinfo[i].attacking) {
						npx = _npcinfo[i].bodysection[7].x;
						npy = _npcinfo[i].bodysection[7].y;

						xdif = _player.px - npx;
						ydif = _player.py - npy;

						if (abs(xdif) < 24 && abs(ydif) < 24) {
							float dist = sqrt(xdif * xdif + ydif * ydif);

							if ((dist) < 24) {
								if (config.effects) {
									int snd = playSound(_sfx[kSndBite]);
									setChannelVolume(snd, config.effectsvol);
								}

								_npcinfo[i].attacking = true;
								_npcinfo[i].attackframe = 0;

								_npcinfo[i].headtargetx[0] = _player.px + 12;
								_npcinfo[i].headtargety[0] = _player.py - 4;
							}
						}

					}

					if (!_npcinfo[i].attacking && _npcinfo[i].castpause < _ticks) {
						_npcinfo[i].swayspd = 4;

						// sway code
						_npcinfo[i].swayangle = _npcinfo[i].swayangle + _npcinfo[i].swayspd * _fpsr;
						if (_npcinfo[i].swayangle >= 360)
							_npcinfo[i].swayangle = _npcinfo[i].swayangle - 360;

						_npcinfo[i].headtargetx[0] = _npcinfo[i].x + (24 - _npcinfo[i].swayspd / 2) * sin(3.14159 / 180 * _npcinfo[i].swayangle) + 12;
						_npcinfo[i].headtargety[0] = _npcinfo[i].y - 36 + 16 + 8 * sin(3.14159 * 2 / 180 * _npcinfo[i].swayangle);
					}

					if (dospell) {
						_npcinfo[i].pause = _ticks + 3000;
						_npcinfo[i].attacknext = _ticks + 5000;
						_npcinfo[i].castpause = _ticks + 3000;

						castSpell(3, _npcinfo[i].x, _npcinfo[i].y, _npcinfo[i].x, _npcinfo[i].y, i);

						_npcinfo[i].headtargetx[0] = _npcinfo[i].x;
						_npcinfo[i].headtargety[0] = _npcinfo[i].y - 36 + 16;
					}

					// targethead code
					xdif = _npcinfo[i].bodysection[7].x - _npcinfo[i].headtargetx[0];
					ydif = _npcinfo[i].bodysection[7].y - _npcinfo[i].headtargety[0];


					_npcinfo[i].bodysection[7].x = _npcinfo[i].bodysection[7].x  - xdif * 0.4 * _fpsr;
					_npcinfo[i].bodysection[7].y = _npcinfo[i].bodysection[7].y  - ydif * 0.4 * _fpsr;

					_npcinfo[i].bodysection[0].x = _npcinfo[i].x + 12;
					_npcinfo[i].bodysection[0].y = _npcinfo[i].y + 12;

					for (int f = 6; f >= 1; f--) {
						xdif = _npcinfo[i].bodysection[f + 1].x - _npcinfo[i].bodysection[f - 1].x;
						ydif = _npcinfo[i].bodysection[f + 1].y - _npcinfo[i].bodysection[f - 1].y;

						float tx = _npcinfo[i].bodysection[f - 1].x + xdif / 2;
						float ty = _npcinfo[i].bodysection[f - 1].y + ydif / 2;

						_npcinfo[i].bodysection[f].x = _npcinfo[i].bodysection[f].x - (_npcinfo[i].bodysection[f].x - tx) / 3;
						_npcinfo[i].bodysection[f].y = _npcinfo[i].bodysection[f].y - (_npcinfo[i].bodysection[f].y - ty) / 3;
					}

				}

				// dragon2 specific
				if (_npcinfo[i].spriteset == kMonsterDragon2 && _npcinfo[i].attackattempt < _ticks) {
					if (_npcinfo[i].attacknext < _ticks && _npcinfo[i].pause < _ticks && !_npcinfo[i].attacking) {
						npx = _npcinfo[i].x;
						npy = _npcinfo[i].y;

						xdif = _player.px - npx;
						ydif = _player.py - npy;

						if (abs(xdif) < 32 && abs(ydif) < 32) {
							_npcinfo[i].attackattempt = _ticks + 100;
							if ((int)(RND() * 2) == 0) {
								if (config.effects) {
									int snd = playSound(_sfx[kSndEnemyHit]);
									setChannelVolume(snd, config.effectsvol);
								}

								_npcinfo[i].attacking = true;
								_npcinfo[i].attackframe = 0;
							}
						}
					}
				}


				// endboss specific
				if (_npcinfo[i].spriteset == kMonsterFinalBoss && _npcinfo[i].attackattempt < _ticks) {
					if (_npcinfo[i].attacknext < _ticks && _npcinfo[i].pause < _ticks && !_npcinfo[i].attacking) {
						npx = _npcinfo[i].x;
						npy = _npcinfo[i].y;

						xdif = _player.px - npx;
						ydif = _player.py - npy;

						if (abs(xdif) < 38 && abs(ydif) < 38) {
							_npcinfo[i].attackattempt = _ticks + 100;
							if ((int)(RND() * 2) == 0) {
								if (config.effects) {
									int snd = playSound(_sfx[kSndIce]);
									setChannelVolume(snd, config.effectsvol);
								}
								_npcinfo[i].attacking = true;
								_npcinfo[i].attackframe = 0;
							}
						}
					}
				}
			}


			float npx = _npcinfo[i].x;
			float npy = _npcinfo[i].y;

			int xp = (npx / 2 + 6);
			int yp = (npy / 2 + 10);

			rcSrc.left = xp - 1;
			rcSrc.top = yp - 1;
			rcSrc.setWidth(3);
			rcSrc.setHeight(3);

			if (_npcinfo[i].pause < _ticks)
				_clipBg->fillRect(rcSrc, i);


			pass = 0;
			if (_npcinfo[i].attacking)
				pass = 1;
			if (_npcinfo[i].spriteset == kMonsterFireHydra) {
				if (_npcinfo[i].attacking2[0] || _npcinfo[i].attacking2[1] || _npcinfo[i].attacking2[2])
					pass = 1;
			}

			if (pass == 1) {
				int dist;
				float damage;
				// spriteset1 specific
				if (_npcinfo[i].spriteset == kMonsterBabyDragon) {
					_npcinfo[i].attackframe = _npcinfo[i].attackframe + _npcinfo[i].attackspd * _fpsr;
					if (_npcinfo[i].attackframe >= 16) {
						_npcinfo[i].attackframe = 0;
						_npcinfo[i].attacking = false;
						_npcinfo[i].attacknext = _ticks + _npcinfo[i].attackdelay;
					}

					_npcinfo[i].cattackframe = (int)(_npcinfo[i].attackframe);

					npx = _npcinfo[i].x;
					npy = _npcinfo[i].y;

					float xdif = _player.px - npx;
					float ydif = _player.py - npy;

					dist = 10;

					if (abs(xdif) < dist && abs(ydif) < dist && _player.pause < _ticks) {
						_npcinfo[i].attacknext = _ticks + _npcinfo[i].attackdelay;
						// _npcinfo[i].attackframe = 0;
						// _npcinfo[i].attacking = false;

						damage = (float)_npcinfo[i].attackdamage * (0.5 + RND() * 1.0);

						if (_player.hp > 0)
							damagePlayer(damage);
					}
				}

				if (_npcinfo[i].spriteset == kMonsterOneWing) {
					// targethead code
					float xdif = _npcinfo[i].bodysection[7].x - _npcinfo[i].headtargetx[0];
					float ydif = _npcinfo[i].bodysection[7].y - _npcinfo[i].headtargety[0];

					_npcinfo[i].bodysection[7].x = _npcinfo[i].bodysection[7].x  - xdif * 0.4 * _fpsr;
					_npcinfo[i].bodysection[7].y = _npcinfo[i].bodysection[7].y  - ydif * 0.4 * _fpsr;

					_npcinfo[i].bodysection[0].x = _npcinfo[i].x + 12;
					_npcinfo[i].bodysection[0].y = _npcinfo[i].y + 12;

					for (int f = 6; f >= 1; f--) {
						xdif = _npcinfo[i].bodysection[f + 1].x - _npcinfo[i].bodysection[f - 1].x;
						ydif = _npcinfo[i].bodysection[f + 1].y - _npcinfo[i].bodysection[f - 1].y;

						float tx = _npcinfo[i].bodysection[f - 1].x + xdif / 2;
						float ty = _npcinfo[i].bodysection[f - 1].y + ydif / 2;

						_npcinfo[i].bodysection[f].x = _npcinfo[i].bodysection[f].x - (_npcinfo[i].bodysection[f].x - tx);
						_npcinfo[i].bodysection[f].y = _npcinfo[i].bodysection[f].y - (_npcinfo[i].bodysection[f].y - ty);
					}

					_npcinfo[i].attackframe = _npcinfo[i].attackframe + _npcinfo[i].attackspd * _fpsr;
					if (_npcinfo[i].attackframe >= 16) {
						_npcinfo[i].attackframe = 0;
						_npcinfo[i].attacking = false;
						_npcinfo[i].attacknext = _ticks + _npcinfo[i].attackdelay;
					}

					_npcinfo[i].cattackframe = (int)(_npcinfo[i].attackframe);

					npx = _npcinfo[i].bodysection[7].x;
					npy = (_npcinfo[i].bodysection[7].y + 16);

					xdif = (_player.px + 12) - npx;
					ydif = (_player.py + 12) - npy;

					dist = 8;

					if (abs(xdif) < dist && abs(ydif) < dist && _player.pause < _ticks) {
						_npcinfo[i].attacknext = _ticks + _npcinfo[i].attackdelay;
						// _npcinfo[i].attackframe = 0
						// _npcinfo[i].attacking = false
						damage = (float)_npcinfo[i].attackdamage * (1.0 + (RND() * 0.5));
						if (_player.hp > 0)
							damagePlayer(damage);
					}
				}


				// firehydra
				if (_npcinfo[i].spriteset == kMonsterFireHydra) {
					for (int ff = 0; ff <= 2; ff++) {
						if (_npcinfo[i].attacking2[ff]) {
							float xdif = _npcinfo[i].bodysection[10 * ff + 9].x - _npcinfo[i].headtargetx[ff];
							float ydif = _npcinfo[i].bodysection[10 * ff + 9].y - _npcinfo[i].headtargety[ff];

							_npcinfo[i].bodysection[10 * ff + 9].x = _npcinfo[i].bodysection[10 * ff + 9].x  - xdif * .2 * _fpsr;
							_npcinfo[i].bodysection[10 * ff + 9].y = _npcinfo[i].bodysection[10 * ff + 9].y  - ydif * .2 * _fpsr;

							_npcinfo[i].bodysection[10 * ff].x = _npcinfo[i].x + 12 + 8 * cos(3.141592 * 2 * (_itemyloc / 16 + ff * 120 / 360));
							_npcinfo[i].bodysection[10 * ff].y = _npcinfo[i].y + 12 + 8 * sin(3.141592 * 2 * (_itemyloc / 16 + ff * 120 / 360));

							for (int f = 8; f >= 1; f--) {
								xdif = _npcinfo[i].bodysection[ff * 10 + f + 1].x - _npcinfo[i].bodysection[ff * 10 + f - 1].x;
								ydif = _npcinfo[i].bodysection[ff * 10 + f + 1].y - _npcinfo[i].bodysection[ff * 10 + f - 1].y;

								float tx = _npcinfo[i].bodysection[ff * 10 + f - 1].x + xdif / 2;
								float ty = _npcinfo[i].bodysection[ff * 10 + f - 1].y + ydif / 2;

								_npcinfo[i].bodysection[ff * 10 + f].x = _npcinfo[i].bodysection[ff * 10 + f].x - (_npcinfo[i].bodysection[ff * 10 + f].x - tx) / 3;
								_npcinfo[i].bodysection[ff * 10 + f].y = _npcinfo[i].bodysection[ff * 10 + f].y - (_npcinfo[i].bodysection[ff * 10 + f].y - ty) / 3;
							}

							_npcinfo[i].attackframe2[ff] = _npcinfo[i].attackframe2[ff] + _npcinfo[i].attackspd * _fpsr;
							if (_npcinfo[i].attackframe2[ff] >= 16) {
								_npcinfo[i].attackframe2[ff] = 0;
								_npcinfo[i].attacking2[ff] = false;
								_npcinfo[i].attacknext2[ff] = _ticks + _npcinfo[i].attackdelay;
							}

							_npcinfo[i].cattackframe = (int)(_npcinfo[i].attackframe);

							npx = _npcinfo[i].bodysection[10 * ff + 9].x;
							npy = (_npcinfo[i].bodysection[10 * ff + 9].y + 16);

							xdif = (_player.px + 12) - npx;
							ydif = (_player.py + 12) - npy;

							dist = 8;

							if (abs(xdif) < dist && abs(ydif) < dist && _player.pause < _ticks) {
								_npcinfo[i].attacknext2[ff] = _ticks + _npcinfo[i].attackdelay;
								// _npcinfo[i].attackframe2(ff) = 0
								// _npcinfo[i].attacking2(ff) = false
								damage = (float)_npcinfo[i].attackdamage * (1.0 + RND() * 0.5);
								if (_player.hp > 0)
									damagePlayer(damage);
							}
						}
					}

				}

				// twowing specific
				if (_npcinfo[i].spriteset == kMonsterTwoWing) {
					// targethead code
					float xdif = _npcinfo[i].bodysection[7].x - _npcinfo[i].headtargetx[0];
					float ydif = _npcinfo[i].bodysection[7].y - _npcinfo[i].headtargety[0];

					_npcinfo[i].bodysection[7].x = _npcinfo[i].bodysection[7].x - xdif * 0.4 * _fpsr;
					_npcinfo[i].bodysection[7].y = _npcinfo[i].bodysection[7].y - ydif * 0.4 * _fpsr;

					_npcinfo[i].bodysection[0].x = _npcinfo[i].x + 12;
					_npcinfo[i].bodysection[0].y = _npcinfo[i].y + 12;

					for (int f = 6; f >= 1; f--) {
						xdif = _npcinfo[i].bodysection[f + 1].x - _npcinfo[i].bodysection[f - 1].x;
						ydif = _npcinfo[i].bodysection[f + 1].y - _npcinfo[i].bodysection[f - 1].y;

						float tx = _npcinfo[i].bodysection[f - 1].x + xdif / 2;
						float ty = _npcinfo[i].bodysection[f - 1].y + ydif / 2;

						_npcinfo[i].bodysection[f].x = _npcinfo[i].bodysection[f].x - (_npcinfo[i].bodysection[f].x - tx);
						_npcinfo[i].bodysection[f].y = _npcinfo[i].bodysection[f].y - (_npcinfo[i].bodysection[f].y - ty);
					}

					_npcinfo[i].attackframe = _npcinfo[i].attackframe + _npcinfo[i].attackspd * _fpsr;
					if (_npcinfo[i].attackframe >= 16) {
						_npcinfo[i].attackframe = 0;
						_npcinfo[i].attacking = false;
						_npcinfo[i].attacknext = _ticks + _npcinfo[i].attackdelay;
					}

					_npcinfo[i].cattackframe = (int)(_npcinfo[i].attackframe);

					npx = _npcinfo[i].bodysection[7].x;
					npy = (_npcinfo[i].bodysection[7].y + 16);

					xdif = (_player.px + 12) - npx;
					ydif = (_player.py + 12) - npy;

					dist = 8;

					if (abs(xdif) < dist && abs(ydif) < dist && _player.pause < _ticks) {
						_npcinfo[i].attacknext = _ticks + _npcinfo[i].attackdelay;
						// _npcinfo[i].attackframe = 0
						// _npcinfo[i].attacking = false
						damage = (float)_npcinfo[i].attackdamage * (1.0 + RND() * 0.5);
						if (_player.hp > 0)
							damagePlayer(damage);
					}
				}

				// dragon 2 specific
				if (_npcinfo[i].spriteset == kMonsterDragon2) {

					_npcinfo[i].attackframe = _npcinfo[i].attackframe + _npcinfo[i].attackspd * _fpsr;
					if (_npcinfo[i].attackframe >= 16) {
						_npcinfo[i].attackframe = 0;
						_npcinfo[i].attacking = false;
						_npcinfo[i].attacknext = _ticks + _npcinfo[i].attackdelay;
					}

					_npcinfo[i].cattackframe = (int)(_npcinfo[i].attackframe);

					npx = _npcinfo[i].x;
					npy = _npcinfo[i].y;

					float xdif = _player.px - npx;
					float ydif = _player.py - npy;

					dist = 16 + _npcinfo[i].attackframe;

					if (abs(xdif) < dist && abs(ydif) < dist && _player.pause < _ticks) {
						_npcinfo[i].attacknext = _ticks + _npcinfo[i].attackdelay;
						// _npcinfo[i].attackframe = 0
						// _npcinfo[i].attacking = false
						damage = (float)_npcinfo[i].attackdamage * (0.5 + RND() * 1.0);
						if (_player.hp > 0)
							damagePlayer(damage);
					}
				}

				// endboss specific
				if (_npcinfo[i].spriteset == kMonsterFinalBoss) {
					_npcinfo[i].attackframe = _npcinfo[i].attackframe + _npcinfo[i].attackspd * _fpsr;
					if (_npcinfo[i].attackframe >= 16) {
						_npcinfo[i].attackframe = 0;
						_npcinfo[i].attacking = false;
						_npcinfo[i].attacknext = _ticks + _npcinfo[i].attackdelay;
					}

					_npcinfo[i].cattackframe = (int)(_npcinfo[i].attackframe);

					npx = _npcinfo[i].x;
					npy = _npcinfo[i].y;

					float xdif = _player.px - npx;
					float ydif = _player.py - npy;

					dist = 36;

					if (abs(xdif) < dist && abs(ydif) < dist && _player.pause < _ticks) {
						_npcinfo[i].attacknext = _ticks + _npcinfo[i].attackdelay;
						// _npcinfo[i].attackframe = 0
						// _npcinfo[i].attacking = false
						damage = (float)_npcinfo[i].attackdamage * (0.5 + RND() * 1.0);
						if (_player.hp > 0)
							damagePlayer(damage);
					}
				}
			}
			// -------end fight code
		}
	}
}

void GriffonEngine::updateSpells() {
	float xloc = 0, yloc = 0;

	for (int i = 0; i < kMaxSpell; i++) {
		if (spellinfo[i].frame > 0) {
			int spellnum = spellinfo[i].spellnum;

			// water
			if (spellnum == 0 && !_forcePause) {
				float fr = (32 - spellinfo[i].frame);
				int ll[4][2];

				ll[0][0] = -2;
				ll[0][1] = -3;
				ll[1][0] = 2;
				ll[1][1] = -3;
				ll[2][0] = -4;
				ll[2][1] = -2;
				ll[3][0] = 4;
				ll[3][1] = -2;

				for (int f = 0; f <= 3; f++) {
					if (fr > f * 4 && fr < f * 4 + 16) {
						float alf = 255;

						if (fr < f * 4 + 8) {
							int fi = (int)((fr - f * 4) * 3) % 4;
							rcSrc.left = 32 + fi * 16;
							rcSrc.top = 80;
							rcSrc.setWidth(16);
							rcSrc.setHeight(16);

							xloc = spellinfo[i].enemyx + 12 + ll[f][0] * 16;
							yloc = spellinfo[i].enemyy + 16 + ll[f][1] * 16;

							rcDest.left = xloc;
							rcDest.top = yloc;

							alf = 255 * ((fr - f * 4) / 8);
						}

						if (fr >= f * 4 + 8) {
							int fi = 0; // ??

							if (f == 0 || f == 2)
								fi = 0;
							if (f == 1 || f == 3)
								fi = 1;
							rcSrc.left = 32 + fi * 16;
							rcSrc.top = 80;
							rcSrc.setWidth(16);
							rcSrc.setHeight(16);

							float xst = spellinfo[i].enemyx + 12 + ll[f][0] * 16;
							float yst = spellinfo[i].enemyy + 16 + ll[f][1] * 16;

							float xi = (spellinfo[i].enemyx - xst) * 2 / 8;
							float yi = (spellinfo[i].enemyy - yst) * 2 / 8;

							float fl = (fr - f * 4 - 8) / 2;
							xloc = xst + xi * fl * fl;
							yloc = yst + yi * fl * fl;

							rcDest.left = xloc;
							rcDest.top = yloc;

							alf = 255;
						}

						if (xloc > -16 && xloc < 304 && yloc > -16 && yloc < 224) {
							_spellImg->setAlpha(alf, true);
							_spellImg->blit(*_videoBuffer, rcDest.left, rcDest.top, Graphics::FLIP_NONE, &rcSrc);
							_spellImg->setAlpha(255, true);

							if (spellinfo[i].damagewho == 0) {
								for (int e = 1; e <= _lastnpc; e++) {
									float xdif = (xloc + 16) - (_npcinfo[e].x + 12);
									float ydif = (yloc + 16) - (_npcinfo[e].y + 12);

									if ((abs(xdif) < 16 && abs(ydif) < 16)) {
										float damage = (float)_player.spellDamage * (1.0 + RND() * 0.5) * (float)spellinfo[i].strength;

										if (_npcinfo[e].hp > 0 && _npcinfo[e].pause < _ticks) {
											damageNPC(e, damage, 1);
											if (config.effects) {
												int snd = playSound(_sfx[kSndIce]);
												setChannelVolume(snd, config.effectsvol);
											}
										}
									}
								}
							}

							// check for post damage
							if (nposts > 0) {
								for (int e = 0; e <= nposts - 1; e++) {
									float xdif = (xloc + 16) - (_postInfo[e][0] + 8);
									float ydif = (yloc + 16) - (_postInfo[e][1] + 8);

									if ((abs(xdif) < 16 && abs(ydif) < 16)) {
										_objmapf[_curMap][(int)_postInfo[e][0] / 16][(int)_postInfo[e][1] / 16] = 1;
										_objectMap[(int)_postInfo[e][0] / 16][(int)_postInfo[e][1] / 16] = -1;

										rcSrc.left = _postInfo[e][0] / 2;
										rcSrc.top = _postInfo[e][1] / 2;
										rcSrc.setWidth(8);
										rcSrc.setHeight(8);

										_clipBg2->fillRect(rcSrc, 0);

										addFloatIcon(99, _postInfo[e][0], _postInfo[e][1]);

										if (config.effects) {
											int snd = playSound(_sfx[kSndIce]);
											setChannelVolume(snd, config.effectsvol);
										}
									}
								}
							}
						}
					}
				}
			}

			// metal
			if (spellnum == 1 && !_forcePause) {
				int fr = (int)((32 - spellinfo[i].frame) * 4) % 3;

				rcSrc.left = fr * 48;
				rcSrc.top = 0;
				rcSrc.setWidth(48);
				rcSrc.setHeight(48);

				float c1 = (32 - spellinfo[i].frame) / 16;

				float halfx = (spellinfo[i].homex - 12) + ((spellinfo[i].enemyx - 12) - (spellinfo[i].homex - 12)) / 2;
				float halfy = (spellinfo[i].homey - 12) + ((spellinfo[i].enemyy - 12) - (spellinfo[i].homey - 12)) / 2;

				float wdth = (halfx - spellinfo[i].homex) * 1.2;
				float hight = (halfy - spellinfo[i].homey) * 1.2;

				xloc = halfx + wdth * cos(3.14159 + 3.14159 * 2 * c1);
				yloc = halfy + hight * sin(3.14159 + 3.14159 * 2 * c1);

				rcDest.left = xloc;
				rcDest.top = yloc;

				_spellImg->blit(*_videoBuffer, rcDest.left, rcDest.top, Graphics::FLIP_NONE, &rcSrc);

				spellinfo[i].frame = spellinfo[i].frame - 0.2 * _fpsr;
				if (spellinfo[i].frame < 0)
					spellinfo[i].frame = 0;

				if (spellinfo[i].damagewho == 0) {
					for (int e = 1; e <= _lastnpc; e++) {
						float xdif = (xloc + 24) - (_npcinfo[e].x + 12);
						float ydif = (yloc + 24) - (_npcinfo[e].y + 12);

						if ((abs(xdif) < 24 && abs(ydif) < 24)) {
							float damage = (float)_player.spellDamage * (1.0 + RND() * 0.5) * (float)spellinfo[i].strength;

							if (_npcinfo[e].hp > 0 && _npcinfo[e].pause < _ticks) {
								damageNPC(e, damage, 1);
								if (config.effects) {
									int snd = playSound(_sfx[kSndMetalHit]);
									setChannelVolume(snd, config.effectsvol);
								}
							}
						}
					}
				}

				if (spellinfo[i].damagewho == 1) {
					// --------- boss 1 specific
					if (ABS(spellinfo[i].frame) < 0 && _npcinfo[spellinfo[i].npc].spriteset == kMonsterBoss1) {
						int npc = spellinfo[i].npc;
						_npcinfo[npc].attackframe = 0;
						_npcinfo[npc].attacking = false;

						_npcinfo[npc].pause = _ticks + 1000;
						_npcinfo[npc].attacknext = _ticks + 4000;
					}
					// ---------------

					// --------- blackknight specific
					if (ABS(spellinfo[i].frame) < 0 && _npcinfo[spellinfo[i].npc].spriteset == kMonsterBlackKnight) {
						int npc = spellinfo[i].npc;
						_npcinfo[npc].attackframe = 0;
						_npcinfo[npc].attacking = false;

						_npcinfo[npc].pause = _ticks + 1000;
						_npcinfo[npc].attacknext = _ticks + 3500;
					}
					// ---------------

					float xdif = (xloc + 24) - (_player.px + 12);
					float ydif = (yloc + 24) - (_player.py + 12);

					if ((abs(xdif) < 24 && abs(ydif) < 24) && _player.pause < _ticks) {
						float damage = (float)_npcinfo[spellinfo[i].npc].spelldamage * (1.0 + RND() * 0.5);

						if (_player.hp > 0) {
							damagePlayer(damage);
							if (config.effects) {
								int snd = playSound(_sfx[kSndMetalHit]);
								setChannelVolume(snd, config.effectsvol);
							}
						}
					}
				}


				// check for(int post damage
				if (nposts > 0) {
					for (int e = 0; e <= nposts - 1; e++) {
						float xdif = (xloc + 24) - (_postInfo[e][0] + 8);
						float ydif = (yloc + 24) - (_postInfo[e][1] + 8);

						if ((abs(xdif) < 24 && abs(ydif) < 24)) {
							_objmapf[_curMap][(int)_postInfo[e][0] / 16][(int)_postInfo[e][1] / 16] = 1;
							_objectMap[(int)_postInfo[e][0] / 16][(int)_postInfo[e][1] / 16] = -1;

							rcSrc.left = _postInfo[e][0] / 2;
							rcSrc.top = _postInfo[e][1] / 2;
							rcSrc.setWidth(8);
							rcSrc.setHeight(8);

							_clipBg2->fillRect(rcSrc, 0);

							addFloatIcon(99, _postInfo[e][0], _postInfo[e][1]);

							if (config.effects) {
								int snd = playSound(_sfx[kSndMetalHit]);
								setChannelVolume(snd, config.effectsvol);
							}
						}
					}
				}
			}

			// earth
			if (spellnum == 2 && !_forcePause) {
				float hght = 240 - spellinfo[i].enemyy;

				for (int f = 8; f >= 0; f--) {

					float fr = (32 - spellinfo[i].frame);

					if (fr > f && fr < f + 16) {
						rcSrc.left = 32 * spellinfo[i].rockimg[f];
						rcSrc.top = 48;
						rcSrc.setWidth(32);
						rcSrc.setHeight(32);

						bool scatter = false;
						if (fr < 8 + f) {
							xloc = spellinfo[i].enemyx - 4;
							yloc = spellinfo[i].enemyy * (1 - cos(3.14159 / 2 * (fr - f) / 8)); // ^ 2;
							yloc *= yloc;
						} else {
							scatter = true;
							xloc = spellinfo[i].enemyx - 4 - spellinfo[i].rockdeflect[f] * sin(3.14159 / 2 * ((fr - f) - 8) / 8);
							yloc = spellinfo[i].enemyy + hght * (1 - cos(3.14159 / 2 * ((fr - f) - 8) / 8));
						}

						rcDest.left = xloc;
						rcDest.top = yloc;

						if (xloc > -16 && xloc < 304 && yloc > -16 && yloc < 224) {
							_spellImg->blit(*_videoBuffer, rcDest.left, rcDest.top, Graphics::FLIP_NONE, &rcSrc);

							if (scatter) {
								if (spellinfo[i].damagewho == 0) {
									for (int e = 1; e <= _lastnpc; e++) {
										float xdif = (xloc + 16) - (_npcinfo[e].x + 12);
										float ydif = (yloc + 16) - (_npcinfo[e].y + 12);

										if ((abs(xdif) < 16 && abs(ydif) < 16)) {
											float damage = (float)_player.spellDamage * (1.0 + RND() * 0.5) * (float)spellinfo[i].strength;

											if (_npcinfo[e].hp > 0 && _npcinfo[e].pause < _ticks) {
												damageNPC(e, damage, 1);
												if (config.effects) {
													int snd = playSound(_sfx[kSndRocks]);
													setChannelVolume(snd, config.effectsvol);
												}
											}
										}
									}
								}


								// check for(int post damage
								if (nposts > 0) {
									for (int e = 0; e <= nposts - 1; e++) {
										float xdif = (xloc + 16) - (_postInfo[e][0] + 8);
										float ydif = (yloc + 16) - (_postInfo[e][1] + 8);

										if ((abs(xdif) < 16 && abs(ydif) < 16)) {
											_objmapf[_curMap][(int)_postInfo[e][0] / 16][(int)_postInfo[e][1] / 16] = 1;
											_objectMap[(int)_postInfo[e][0] / 16][(int)_postInfo[e][1] / 16] = -1;

											rcSrc.left = _postInfo[e][0] / 2;
											rcSrc.top = _postInfo[e][1] / 2;
											rcSrc.setWidth(8);
											rcSrc.setHeight(8);

											_clipBg2->fillRect(rcSrc, 0);

											addFloatIcon(99, _postInfo[e][0], _postInfo[e][1]);

											if (config.effects) {
												int snd = playSound(_sfx[kSndRocks]);
												setChannelVolume(snd, config.effectsvol);
											}
										}
									}
								}
							}
						}
					}
				}

				spellinfo[i].frame = spellinfo[i].frame - 0.2 * _fpsr;
				if (spellinfo[i].frame < 0)
					spellinfo[i].frame = 0;
			}

			// crystal
			if (spellnum == 5) {

				float fra = (32 - spellinfo[i].frame);
				int fr = (int)((spellinfo[i].frame) * 2) % 8;

				rcSrc.left = fr * 32;
				rcSrc.top = 96 + 48;
				rcSrc.setWidth(32);
				rcSrc.setHeight(64);

				rcDest.left = _player.px - 4;
				rcDest.top = _player.py + 16 - 48;

				int f = 160;
				if (fra < 8)
					f = 192 * fra / 8;
				if (fra > 24)
					f = 192 * (1 - (fra - 24) / 8);

				_spellImg->setAlpha(f, true);
				_spellImg->blit(*_videoBuffer, rcDest.left, rcDest.top, Graphics::FLIP_NONE, &rcSrc);
				_spellImg->setAlpha(255, true);

				spellinfo[i].frame = spellinfo[i].frame - 0.3 * _fpsr;
				if (spellinfo[i].frame < 0) {
					spellinfo[i].frame = 0;
					_forcePause = false;

					float npx = _player.px + 12;
					float npy = _player.py + 20;

					int lx = (int)npx / 16;
					int ly = (int)npy / 16;

					bool foundel[5];
					for (int f1 = 0; f1 < 5; f1++) { // !! f < 5
						foundel[f1] = false;
					}

					for (int xo = -2; xo <= 2; xo++) {
						for (int yo = -2; yo <= 2; yo++) {

							int sx = lx + xo;
							int sy = ly + yo;

							if (sx > -1 && sx < 20 && sy > -1 && sy < 15) {
								for (int l = 0; l <= 2; l++) {
									int curtile = _tileinfo[l][sx][sy][0];
									int curtilel = _tileinfo[l][sx][sy][1];

									if (curtile > 0) {
										curtile = curtile - 1;
										int curtilex = curtile % 20;
										int curtiley = (curtile - curtilex) / 20;

										int element = elementmap[curtiley][curtilex];
										if (element > -1 && curtilel == 0)
											foundel[element + 1] = true;
									}
								}

								int o = _objectMap[sx][sy];
								if (o > -1) {
									if (_objectInfo[o][4] == 1)
										foundel[2] = true;
									if (o == 1 || o == 2) {
										foundel[2] = true;
										foundel[4] = true;
									}
								}
							}
						}
					}

					char line[256];
					strcpy(line, "Found... nothing...");

					for (int f1 = 0; f1 < 5; f1++) {
						if (foundel[f1] && !_player.foundSpell[f1]) {
							_player.foundSpell[f1] = true;
							_player.spellCharge[f1] = 0;
							if (f1 == 1)
								strcpy(line, "Found... Water Essence");
							if (f1 == 2)
								strcpy(line, "Found... Metal Essence");
							if (f1 == 3)
								strcpy(line, "Found... Earth Essence");
							if (f1 == 4)
								strcpy(line, "Found... Fire Essence");
							break;
						}
					}

					eventText(line);
				}
			}

			// room fireballs
			if (spellnum == 6 && !_forcePause) {

				if (spellinfo[i].frame > 16) {
					float fr = (32 - spellinfo[i].frame);

					_spellImg->setAlpha(192 * sin(3.14159 * fr / 4), true);

					rcSrc.left = 16 * (int)(RND() * 2);
					rcSrc.top = 80;
					rcSrc.setWidth(16);
					rcSrc.setHeight(16);

					for (int ff = 0; ff <= spellinfo[i].nfballs - 1; ff++) {

						xloc = spellinfo[i].fireballs[ff][0] + (int)(RND() * 3) - 1;
						yloc = spellinfo[i].fireballs[ff][1] + (int)(RND() * 3) - 1;

						rcDest.left = xloc;
						rcDest.top = yloc;

						_spellImg->blit(*_videoBuffer, rcDest.left, rcDest.top, Graphics::FLIP_NONE, &rcSrc);
					}

					_spellImg->setAlpha(255, true);
				} else {
					_spellImg->setAlpha(192, true);

					rcSrc.left = 16 * (int)(RND() * 2);
					rcSrc.top = 80;
					rcSrc.setWidth(16);
					rcSrc.setHeight(16);

					for (int ff = 0; ff <= spellinfo[i].nfballs - 1; ff++) {
						float ax = spellinfo[i].fireballs[ff][0];
						float ay = spellinfo[i].fireballs[ff][1];
						float bx = _player.px + 4;
						float by = _player.py + 4;
						float d = sqrt((bx - ax) * (bx - ax) + (by - ay) * (by - ay));

						float tx = (bx - ax) / d;
						float ty = (by - ay) / d;

						spellinfo[i].fireballs[ff][2] += tx * 1.2 * _fpsr;
						spellinfo[i].fireballs[ff][3] += ty * 1.2 * _fpsr;

						if (spellinfo[i].ballon[ff] == 1) {
							spellinfo[i].fireballs[ff][0] = ax + spellinfo[i].fireballs[ff][2] * 0.2 * _fpsr;
							spellinfo[i].fireballs[ff][1] = ay + spellinfo[i].fireballs[ff][3] * 0.2 * _fpsr;

							xloc = spellinfo[i].fireballs[ff][0] + (int)(RND() * 3) - 1;
							yloc = spellinfo[i].fireballs[ff][1] + (int)(RND() * 3) - 1;

							rcDest.left = xloc;
							rcDest.top = yloc;

							_spellImg->blit(*_videoBuffer, rcDest.left, rcDest.top, Graphics::FLIP_NONE, &rcSrc);
						}

						if (xloc < -1 || yloc < -1 || xloc > 304 || yloc > 224)
							spellinfo[i].ballon[ff] = 0;
					}

					_spellImg->setAlpha(255, true);
				}

				spellinfo[i].frame = spellinfo[i].frame - 0.2 * _fpsr;
				if (spellinfo[i].frame < 0)
					spellinfo[i].frame = 0;

				if (spellinfo[i].damagewho == 1) {
					for (int ff = 0; ff <= spellinfo[i].nfballs - 1; ff++) {
						if (spellinfo[i].ballon[ff] == 1) {
							xloc = spellinfo[i].fireballs[ff][0] + (int)(RND() * 3) - 1;
							yloc = spellinfo[i].fireballs[ff][1] + (int)(RND() * 3) - 1;

							float xdif = (xloc + 8) - (_player.px + 12);
							float ydif = (yloc + 8) - (_player.py + 12);

							if ((abs(xdif) < 8 && abs(ydif) < 8) && _player.pause < _ticks) {
								float damage = _npcinfo[spellinfo[i].npc].spelldamage * (1 + RND() * 0.5) / 3;

								if (_player.hp > 0)
									damagePlayer(damage);

								if (config.effects) {
									int snd = playSound(_sfx[kSndFire]);
									setChannelVolume(snd, config.effectsvol);
								}
							}
						}
					}
				}
			}

			// lightning bomb
			if (spellnum == 8) {
				long cl1 = _videoBuffer->format.RGBToColor(0, 32, 204);
				long cl2 = _videoBuffer->format.RGBToColor(142, 173, 191);
				long cl3 = _videoBuffer->format.RGBToColor(240, 240, 240);

				float px = _player.px + 12;
				float py = _player.py + 12;

				int apx = px + (int)(RND() * 5 - 2);
				int apy = py + (int)(RND() * 5 - 2);

				for (int f = 0; f <= 0; f++) { // ??
					int y = apy;
					int orn = 0;
					for (int x = apx; x <= 319; x++) {
						int rn = (int)(RND() * 3);

						if (orn == 0)
							--y;
						else if (orn == 2)
							++y;

						drawLine(_videoBuffer, x, y - 1, x, y + 2, cl1);
						drawLine(_videoBuffer, x, y, x, y + 1, cl3);

						if (rn == 0)
							drawLine(_videoBuffer, x, y + 1, x, y + 1, cl2);
						if (rn == 2)
							drawLine(_videoBuffer, x, y, x, y, cl2);

						orn = rn;

						if (spellinfo[i].damagewho == 0) {
							for (int e = 1; e <= _lastnpc; e++) {
								float xdif = (x + 16) - (_npcinfo[e].x + 12);
								float ydif = (y + 16) - (_npcinfo[e].y + 12);

								if ((abs(xdif) < 8 && abs(ydif) < 8)) {
									float damage = 30 * (1 + RND() * 0.5);

									if (_npcinfo[e].hp > 0 && _npcinfo[e].pause < _ticks)
										damageNPC(e, damage, 1);
								}
							}
						}

						// check for post damage
						if (nposts > 0) {
							for (int e = 0; e <= nposts - 1; e++) {
								float xdif = (xloc + 16) - (_postInfo[e][0] + 8);
								float ydif = (yloc + 16) - (_postInfo[e][1] + 8);

								if ((abs(xdif) < 16 && abs(ydif) < 16)) {
									_objmapf[_curMap][(int)_postInfo[e][0] / 16][(int)_postInfo[e][1] / 16] = 1;
									_objectMap[(int)_postInfo[e][0] / 16][(int)_postInfo[e][1] / 16] = -1;

									rcSrc.left = _postInfo[e][0] / 2;
									rcSrc.top = _postInfo[e][1] / 2;
									rcSrc.setWidth(8);
									rcSrc.setHeight(8);

									_clipBg2->fillRect(rcSrc, 0);

									addFloatIcon(99, _postInfo[e][0], _postInfo[e][1]);
								}
							}
						}
					}

					y = apy;
					orn = 0;
					for (int x = apx; x >= 0; x--) {
						int rn = (int)(RND() * 3);

						if (orn == 0)
							--y;
						else if (orn == 2)
							++y;

						drawLine(_videoBuffer, x, y - 1, x, y + 2, cl1);
						drawLine(_videoBuffer, x, y, x, y + 1, cl3);

						if (rn == 0)
							drawLine(_videoBuffer, x, y + 1, x, y + 1, cl2);
						if (rn == 2)
							drawLine(_videoBuffer, x, y, x, y, cl2);

						orn = rn;

						if (spellinfo[i].damagewho == 0) {
							for (int e = 1; e <= _lastnpc; e++) {
								float xdif = (x + 16) - (_npcinfo[e].x + 12);
								float ydif = (y + 16) - (_npcinfo[e].y + 12);

								if ((abs(xdif) < 8 && abs(ydif) < 8)) {
									float damage = 30 * (1 + RND() * 0.5);

									if (_npcinfo[e].hp > 0 && _npcinfo[e].pause < _ticks)
										damageNPC(e, damage, 1);
								}
							}
						}

						// check for post damage
						if (nposts > 0) {
							for (int e = 0; e <= nposts - 1; e++) {
								float xdif = (xloc + 16) - (_postInfo[e][0] + 8);
								float ydif = (yloc + 16) - (_postInfo[e][1] + 8);

								if ((abs(xdif) < 16 && abs(ydif) < 16)) {
									_objmapf[_curMap][(int)_postInfo[e][0] / 16][(int)_postInfo[e][1] / 16] = 1;
									_objectMap[(int)_postInfo[e][0] / 16][(int)_postInfo[e][1] / 16] = -1;

									rcSrc.left = _postInfo[e][0] / 2;
									rcSrc.top = _postInfo[e][1] / 2;
									rcSrc.setWidth(8);
									rcSrc.setHeight(8);

									_clipBg2->fillRect(rcSrc, 0);

									addFloatIcon(99, _postInfo[e][0], _postInfo[e][1]);
								}
							}
						}
					}

					int x = apx;
					orn = 0;
					for (y = apy; y <= 239; y++) {
						int rn = (int)(RND() * 3);

						if (orn == 0)
							x = x - 1;
						if (orn == 2)
							x = x + 1;

						drawLine(_videoBuffer, x - 1, y, x + 2, y, cl1);
						drawLine(_videoBuffer, x, y, x + 1, y, cl3);

						if (rn == 0)
							drawLine(_videoBuffer, x + 1, y, x + 1, y, cl2);
						if (rn == 2)
							drawLine(_videoBuffer, x, y, x, y, cl2);

						orn = rn;

						if (spellinfo[i].damagewho == 0) {
							for (int e = 1; e <= _lastnpc; e++) {
								float xdif = (x + 16) - (_npcinfo[e].x + 12);
								float ydif = (y + 16) - (_npcinfo[e].y + 12);

								if ((abs(xdif) < 8 && abs(ydif) < 8)) {
									float damage = 30 * (1 + RND() * 0.5);

									if (_npcinfo[e].hp > 0 && _npcinfo[e].pause < _ticks)
										damageNPC(e, damage, 1);
								}
							}
						}

						// check for post damage
						if (nposts > 0) {
							for (int e = 0; e <= nposts - 1; e++) {
								float xdif = (xloc + 16) - (_postInfo[e][0] + 8);
								float ydif = (yloc + 16) - (_postInfo[e][1] + 8);

								if ((abs(xdif) < 16 && abs(ydif) < 16)) {
									_objmapf[_curMap][(int)_postInfo[e][0] / 16][(int)_postInfo[e][1] / 16] = 1;
									_objectMap[(int)_postInfo[e][0] / 16][(int)_postInfo[e][1] / 16] = -1;

									rcSrc.left = _postInfo[e][0] / 2;
									rcSrc.top = _postInfo[e][1] / 2;
									rcSrc.setWidth(8);
									rcSrc.setHeight(8);

									_clipBg2->fillRect(rcSrc, 0);

									addFloatIcon(99, _postInfo[e][0], _postInfo[e][1]);
								}
							}
						}
					}

					x = apx;
					orn = 0;
					for (y = apy; y >= 0; y--) {
						int rn = (int)(RND() * 3);

						if (orn == 0)
							x = x - 1;
						if (orn == 2)
							x = x + 1;

						drawLine(_videoBuffer, x - 1, y, x + 2, y, cl1);
						drawLine(_videoBuffer, x, y, x + 1, y, cl3);

						if (rn == 0)
							drawLine(_videoBuffer, x + 1, y, x + 1, y, cl2);
						if (rn == 2)
							drawLine(_videoBuffer, x, y, x, y, cl2);

						orn = rn;

						if (spellinfo[i].damagewho == 0) {
							for (int e = 1; e <= _lastnpc; e++) {
								float xdif = (x + 16) - (_npcinfo[e].x + 12);
								float ydif = (y + 16) - (_npcinfo[e].y + 12);

								if ((abs(xdif) < 8 && abs(ydif) < 8)) {
									float damage = 30 * (1 + RND() * 0.5);

									if (_npcinfo[e].hp > 0 && _npcinfo[e].pause < _ticks)
										damageNPC(e, damage, 1);
								}
							}
						}

						// check for post damage
						if (nposts > 0) {
							for (int e = 0; e <= nposts - 1; e++) {
								float xdif = (xloc + 16) - (_postInfo[e][0] + 8);
								float ydif = (yloc + 16) - (_postInfo[e][1] + 8);

								if ((abs(xdif) < 16 && abs(ydif) < 16)) {
									_objmapf[_curMap][(int)_postInfo[e][0] / 16][(int)_postInfo[e][1] / 16] = 1;
									_objectMap[(int)_postInfo[e][0] / 16][(int)_postInfo[e][1] / 16] = -1;

									rcSrc.left = _postInfo[e][0] / 2;
									rcSrc.top = _postInfo[e][1] / 2;
									rcSrc.setWidth(8);
									rcSrc.setHeight(8);

									_clipBg2->fillRect(rcSrc, 0);

									addFloatIcon(99, _postInfo[e][0], _postInfo[e][1]);
								}
							}
						}
					}
				}

				spellinfo[i].frame -= 0.5 * _fpsr;
				if (spellinfo[i].frame < 0) {
					spellinfo[i].frame = 0;
					_forcePause = false;
				}
			}

			// wizard 1 lightning
			if (spellnum == 9) {
				long cl1 = _videoBuffer->format.RGBToColor(0, 32, 204);
				long cl2 = _videoBuffer->format.RGBToColor(142, 173, 191);
				long cl3 = _videoBuffer->format.RGBToColor(240, 240, 240);

				int px = spellinfo[i].enemyx + 12;
				int py = spellinfo[i].enemyy + 24;

				int apx = px + (int)(RND() * 20 - 10);
				int apy = py + (int)(RND() * 20 - 10);

				int x = apx;
				int orn = 0;
				for (int y = 0; y <= apy; y++) {
					if (y < 240) {
						int rn = (int)(RND() * 3);

						if (orn == 0)
							x = x - 1;
						if (orn == 2)
							x = x + 1;

						drawLine(_videoBuffer, x - 1, y, x + 2, y, cl1);
						drawLine(_videoBuffer, x, y, x + 1, y, cl3);

						if (rn == 0)
							drawLine(_videoBuffer, x + 1, y, x + 1, y, cl2);
						if (rn == 2)
							drawLine(_videoBuffer, x, y, x, y, cl2);

						orn = rn;

						if (spellinfo[i].damagewho == 1) {
							float xdif = (x + 8) - (_player.px + 12);
							float ydif = (y + 8) - (_player.py + 12);

							if ((abs(xdif) < 8 && abs(ydif) < 8) && _player.pause < _ticks) {
								float damage = ((float)_player.hp * 0.75) * (RND() * 0.5 + 0.5);
								if (damage < 5)
									damage = 5;

								if ((_npcinfo[spellinfo[i].npc].spriteset == kMonsterBatKitty) && (damage < 50))
									damage = 40 + (int)(RND() * 40);

								if (_player.hp > 0)
									damagePlayer(damage);
							}
						}
					}
				}

				spellinfo[i].frame -= 0.5 * _fpsr;
				if (spellinfo[i].frame < 0) {
					spellinfo[i].frame = 0;

					_npcinfo[spellinfo[i].npc].attacking = false;
					_npcinfo[spellinfo[i].npc].attacknext = _ticks + _npcinfo[spellinfo[i].npc].attackdelay;
				}
			}
		}
	}
}

void GriffonEngine::updateSpellsUnder() {
	if (_forcePause)
		return;

	for (int i = 0; i < kMaxSpell; i++) {
		if (spellinfo[i].frame > 0) {
			int spellnum = spellinfo[i].spellnum;

			// water
			if (spellnum == 0) {
				int fra = (32 - spellinfo[i].frame);
				int fr = (int)((32 - spellinfo[i].frame) * 2) % 4;

				rcSrc.left = fr * 48;
				rcSrc.top = 96;
				rcSrc.setWidth(48);
				rcSrc.setHeight(48);

				rcDest.left = spellinfo[i].enemyx - 12;
				rcDest.top = spellinfo[i].enemyy - 8;

				int f = 160;
				if (fra < 8)
					f = 160 * fra / 8;
				if (fra > 24)
					f = 160 * (1 - (fra - 24) / 8);

				_spellImg->setAlpha(f, true);
				_spellImg->blit(*_videoBuffer, rcDest.left, rcDest.top, Graphics::FLIP_NONE, &rcSrc);
				_spellImg->setAlpha(255, true);

				spellinfo[i].frame = spellinfo[i].frame - 0.2 * _fpsr;
				if (spellinfo[i].frame < 0)
					spellinfo[i].frame = 0;


				for (f = 1; f <= _lastnpc; f++) {
					int xdif = spellinfo[i].enemyx - _npcinfo[f].x;
					int ydif = spellinfo[i].enemyy - _npcinfo[f].y;

					float dist = sqrt((float)(xdif * xdif + ydif * ydif));

					if (dist > 20)
						dist = 20;

					if (dist > 5) {
						float ratio = (1 - dist / 25);

						float newx = _npcinfo[f].x + ratio * xdif / 3 * _fpsr;
						float newy = _npcinfo[f].y + ratio * ydif / 3 * _fpsr;

						int sx = (newx / 2 + 6);
						int sy = (newy / 2 + 10);

						unsigned int *temp = (uint32 *)_clipBg->getBasePtr(sx, sy);
						unsigned int dq = *temp;

						if (dq == 0) {
							_npcinfo[f].x = newx;
							_npcinfo[f].y = newy;
							// _npcinfo[f].castpause = _ticks + 200;
						} else {
							int xpass = 0;
							int ypass = 0;

							sx = (newx / 2 + 6);
							sy = (_npcinfo[f].y / 2 + 10);
							temp = (uint32 *)_clipBg->getBasePtr(sx, sy);
							dq = *temp;

							if (dq == 0)
								xpass = 1;


							sx = (_npcinfo[f].x / 2 + 6);
							sy = (newy / 2 + 10);
							temp = (uint32 *)_clipBg->getBasePtr(sx, sy);
							dq = *temp;

							if (dq == 0)
								ypass = 1;

							if (ypass == 1) {
								newx = _npcinfo[f].x;
							} else if (xpass == 1) {
								newy = _npcinfo[f].y;
							}

							if (xpass == 1 || ypass == 1) {
								_npcinfo[f].x = newx;
								_npcinfo[f].y = newy;
								// _npcinfo[f].castpause = _ticks + 200;
							}
						}
					}
				}
			}

			// fire
			if (spellnum == 3) {
				float fr = (32 - spellinfo[i].frame);

				fr = fr * fr * (1 - cos(3.14159 / 4 + 3.14159 / 4 * fr / 32));

				if (fr > 32)
					fr = 32;

				float s = 8;
				if (spellinfo[i].frame < 8)
					s = spellinfo[i].frame;

				int fra = (int)fr;

				for (int f = 0; f <= 4; f++) {
					for (int x = 0; x <= fra; x += 2) {
						if (spellinfo[i].legalive[f] >= x) {
							_spellImg->setAlpha(192 * sin(3.14159 * x / 32) * s / 8, true);

							float an = 360 / 5 * f + x / 32 * 180;

							rcSrc.left = 16 * (int)(RND() * 2);
							rcSrc.top = 80;
							rcSrc.setWidth(16);
							rcSrc.setHeight(16);

							float xloc = (float)(spellinfo[i].enemyx + 4 + x * 2 * cos(3.14159 / 180 * an) + (int)(RND() * 3) - 1);
							float yloc = (float)(spellinfo[i].enemyy + 4 + x * 2 * sin(3.14159 / 180 * an) + (int)(RND() * 3) - 1);
							rcDest.left = (int)xloc;
							rcDest.top = (int)yloc;

							if (xloc > -1 && xloc < 304 && yloc > -1 && yloc < 224) {
								_spellImg->blit(*_videoBuffer, rcDest.left, rcDest.top, Graphics::FLIP_NONE, &rcSrc);

								int sx = (xloc / 2 + 4);
								int sy = (yloc / 2 + 8);

								unsigned int *temp = (uint32 *)_clipBg->getBasePtr(sx, sy);
								unsigned int dq = *temp;

								if (dq > 1000 && x > 4)
									spellinfo[i].legalive[f] = x;

								if (spellinfo[i].damagewho == 0) {
									for (int e = 1; e <= _lastnpc; e++) {
										float xdif = (xloc + 8) - (_npcinfo[e].x + 12);
										float ydif = (yloc + 8) - (_npcinfo[e].y + 12);

										if ((abs(xdif) < 8 && abs(ydif) < 8)) {
											float damage = (float)_player.spellDamage * (1.0 + RND() * 0.5) * (float)spellinfo[i].strength;

											if (_npcinfo[e].spriteset == kMonsterFireHydra)
												damage = -damage;
											if (_npcinfo[e].spriteset == kMonsterFinalBoss)
												damage = -damage;
											if (_npcinfo[e].hp > 0 && _npcinfo[e].pause < _ticks) {
												damageNPC(e, damage, 1);
												if (config.effects) {
													int snd = playSound(_sfx[kSndFire]);
													setChannelVolume(snd, config.effectsvol);
												}
											}
										}
									}
								}

								if (spellinfo[i].damagewho == 1) {
									float xdif = (xloc + 8) - (_player.px + 12);
									float ydif = (yloc + 8) - (_player.py + 12);

									if ((abs(xdif) < 8 && abs(ydif) < 8) && _player.pause < _ticks) {
										float damage = (float)_npcinfo[spellinfo[i].npc].spelldamage * (1.0 + RND() * 0.5);

										if (_player.hp > 0) {
											damagePlayer(damage);

											if (config.effects) {
												int snd = playSound(_sfx[kSndFire]);
												setChannelVolume(snd, config.effectsvol);
											}
										}
									}
								}

								// check for post damage
								if (nposts > 0) {
									for (int e = 0; e <= nposts - 1; e++) {
										float xdif = (xloc + 8) - (_postInfo[e][0] + 8);
										float ydif = (yloc + 8) - (_postInfo[e][1] + 8);

										if ((abs(xdif) < 8 && abs(ydif) < 8)) {
											_objmapf[_curMap][(int)_postInfo[e][0] / 16][(int)_postInfo[e][1] / 16] = 1;
											_objectMap[(int)_postInfo[e][0] / 16][(int)_postInfo[e][1] / 16] = -1;

											rcSrc.left = _postInfo[e][0] / 2;
											rcSrc.top = _postInfo[e][1] / 2;
											rcSrc.setWidth(8);
											rcSrc.setHeight(8);

											_clipBg2->fillRect(rcSrc, 0);

											if (config.effects) {
												int snd = playSound(_sfx[kSndFire]);
												setChannelVolume(snd, config.effectsvol);
											}

											addFloatIcon(99, _postInfo[e][0], _postInfo[e][1]);
										}
									}
								}
							}
						}
					}
				}

				_spellImg->setAlpha(255, true);

				spellinfo[i].frame = spellinfo[i].frame - 0.2 * _fpsr;
				if (spellinfo[i].frame < 0)
					spellinfo[i].frame = 0;


			}


			// sprite 6 spitfire
			if (spellnum == 7) {
				float xspan = spellinfo[i].enemyx - spellinfo[i].homex;
				float yspan = spellinfo[i].enemyy - spellinfo[i].homey;
				float fr = (32 - spellinfo[i].frame);

				for (int f = 0; f <= 7; f++) {
					int alpha = 0;
					float xx = 0;
					if (fr > f * 2 && fr < f * 2 + 16)
						xx = fr - f * 2;
					if (xx < 8)
						alpha = 255 * xx / 8;
					if (xx > 8)
						alpha = 255 * (1 - (xx - 8) / 8);
					float yy = 16 * sin(3.141592 / 2 * xx / 16) - 8;

					alpha = CLIP(alpha, 0, 255);
					_spellImg->setAlpha(alpha, true);

					rcSrc.left = 16 * (int)(RND() * 2);
					rcSrc.top = 80;
					rcSrc.setWidth(16);
					rcSrc.setHeight(16);

					float xloc = spellinfo[i].homex + xspan / 7 * f;
					float yloc = spellinfo[i].homey + yspan / 7 * f - yy;

					rcDest.left = xloc;
					rcDest.top = yloc;

					if (xloc > -16 && xloc < 320 && yloc > -16 && yloc < 240) {
						_spellImg->blit(*_videoBuffer, rcDest.left, rcDest.top, Graphics::FLIP_NONE, &rcSrc);

						if (spellinfo[i].damagewho == 1) {
							float xdif = (xloc + 8) - (_player.px + 12);
							float ydif = (yloc + 8) - (_player.py + 12);

							if ((abs(xdif) < 8 && abs(ydif) < 8) && _player.pause < _ticks && alpha > 64) {
								float damage = (float)_npcinfo[spellinfo[i].npc].spelldamage * (1.0 + RND() * 0.5);

								if (_player.hp > 0) {
									damagePlayer(damage);
									if (config.effects) {
										int snd = playSound(_sfx[kSndFire]);
										setChannelVolume(snd, config.effectsvol);
									}
								}

							}
						}

					}

				}

				_spellImg->setAlpha(255, true);
				spellinfo[i].frame = spellinfo[i].frame - 0.5 * _fpsr;
				if (spellinfo[i].frame < 0)
					spellinfo[i].frame = 0;

				if (ABS(spellinfo[i].frame) < kEpsilon) {
					_npcinfo[spellinfo[i].npc].attacking = false;
					_npcinfo[spellinfo[i].npc].attacknext = _ticks + _npcinfo[spellinfo[i].npc].attackdelay;
				}
			}
		}
	}
}


} // end of namespace Griffon
