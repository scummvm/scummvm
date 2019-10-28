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
#include "griffon/console.h"

namespace Griffon {

void GriffonEngine::attack() {
	float npx = _player.px + 12;
	float npy = _player.py + 20;

	int lx = (int)npx / 16;
	int ly = (int)npy / 16;

	// if facing up
	if (_player.walkDir == 0) {
		if (ly > 0) {
			int o2 = 0; // ??
			int o = _objectMap[lx][ly - 1];
			if (ly > 1 && (_curmap == 54 || _curmap == 58))
				o2 = _objectMap[lx][ly - 2];

			// cst
			if ((_objectInfo[o][4] == 1 && (o == 0 || o > 4)) || (_objectInfo[o2][4] == 0 && o2 == 10)) {
				if (o2 == 10)
					o = 10;

				int oscript = _objectInfo[o][5];
				switch (oscript) {
				case kScriptFlask:
					if (_player.inventory[kInvFlask] < 9) {
						_player.inventory[kInvFlask]++;
						addFloatIcon(6, lx * 16, (ly - 1) * 16);

						_objmapf[_curmap][lx][ly - 1] = 1;

						if (config.effects) {
							int snd = playSound(_sfx[kSndPowerUp]);
							setChannelVolume(snd, config.effectsvol);
						}

						if (_objectInfo[o][4] == 1)
							_objectMap[lx][ly - 1] = 3;

						eventText("Found Flask!");
					} else {
						if (config.effects) {
							int snd = playSound(_sfx[kSndChest]);
							setChannelVolume(snd, config.effectsvol);
						}

						eventText("Cannot Carry any more Flasks!");
					}
					_itemticks = _ticks + 215;
					return;
					break;
				case kScriptMasterKey:
					_player.inventory[kInvMasterKey]++;

					addFloatIcon(14, lx * 16, (ly - 1) * 16);

					_itemticks = _ticks + 215;

					if (_curmap == 34)
						_scriptflag[kScriptMasterKey][0] = 2;
					else if (_curmap == 62)
						_scriptflag[kScriptGardenMasterKey][0] = 2;
					else if (_curmap == 81)
						_scriptflag[kScriptCitadelMasterKey][0] = 2;

					if (config.effects) {
						int snd = playSound(_sfx[kSndPowerUp]);
						setChannelVolume(snd, config.effectsvol);
					}

					if (_objectInfo[o][4] == 1)
						_objectMap[lx][ly - 1] = 3;
					eventText("Found the Temple Key!");
					return;
				case kScriptFindCrystal:
					_player.foundSpell[0] = 1;
					_player.spellCharge[0] = 0;

					addFloatIcon(7, lx * 16, (ly - 1) * 16);

					if (config.effects) {
						int snd = playSound(_sfx[kSndPowerUp]);
						setChannelVolume(snd, config.effectsvol);
					}

					if (_objectInfo[o][4] == 1)
						_objectMap[lx][ly - 1] = 3;

					eventText("Found the Infinite Crystal!");
					_itemticks = _ticks + 215;
					return;
				case kScriptFindShield:
					if (_player.shield == 1) {
						_player.shield = 2;

						addFloatIcon(4, lx * 16, (ly - 1) * 16);

						_itemticks = _ticks + 215;

						if (config.effects) {
							int snd = playSound(_sfx[kSndPowerUp]);
							setChannelVolume(snd, config.effectsvol);
						}

						if (_objectInfo[o][4] == 1)
							_objectMap[lx][ly - 1] = 3;

						eventText("Found the Obsidian Shield!");
						_objmapf[4][1][2] = 1;
						return;
					}
					break;
				case kScriptFindSword:
					if (_player.sword == 1) {
						_player.sword = 2;

						addFloatIcon(3, lx * 16, (ly - 1) * 16);

						_itemticks = _ticks + 215;

						if (config.effects) {
							int snd = playSound(_sfx[kSndPowerUp]);
							setChannelVolume(snd, config.effectsvol);
						}

						if (_objectInfo[o][4] == 1)
							_objectMap[lx][ly - 1] = 3;
						eventText("Found the Fidelis Sword!");
						return;
					}
					break;
				case kScriptKeyChest:
					if (_player.inventory[kInvNormalKey] < 9) {
						_player.inventory[kInvNormalKey]++;

						for (int s = 20; s <= 23; s++) {
							if (_scriptflag[s][0] == 1) {
								_scriptflag[s][0] = 2;
							}
						}

						if (config.effects) {
							int snd = playSound(_sfx[kSndPowerUp]);
							setChannelVolume(snd, config.effectsvol);
						}

						_objmapf[_curmap][lx][ly - 1] = 1;

						if (_objectInfo[o][4] == 1)
							_objectMap[lx][ly - 1] = 3;

						eventText("Found Key");
						addFloatIcon(16, lx * 16, (ly - 1) * 16);
					} else {
						if (config.effects) {
							int snd = playSound(_sfx[kSndChest]);
							setChannelVolume(snd, config.effectsvol);
						}

						eventText("Cannot Carry Any More Keys");
					}
					break;
				case kScriptBlueFlask:
					if (_player.inventory[kInvDoubleFlask] < 9) {
						_player.inventory[kInvDoubleFlask]++;
						addFloatIcon(12, lx * 16, (ly - 1) * 16);

						_objmapf[_curmap][lx][ly - 1] = 1;

						if (config.effects) {
							int snd = playSound(_sfx[kSndPowerUp]);
							setChannelVolume(snd, config.effectsvol);
						}

						if (_objectInfo[o][4] == 1)
							_objectMap[lx][ly - 1] = 3;

						eventText("Found Mega Flask!");
					} else {
						if (config.effects) {
							int snd = playSound(_sfx[kSndChest]);
							setChannelVolume(snd, config.effectsvol);
						}

						eventText("Cannot Carry any more Mega Flasks!");
					}

					_itemticks = _ticks + 215;
					return;
					break;
				case kScriptBlueFlaskChest:
					if (_player.inventory[kInvDoubleFlask] < 9) {
						_player.inventory[kInvDoubleFlask]++;
						addFloatIcon(12, lx * 16, (ly - 1) * 16);

						_objmapf[_curmap][lx][ly - 1] = 1;

						if (config.effects) {
							int snd = playSound(_sfx[kSndPowerUp]);
							setChannelVolume(snd, config.effectsvol);
						}

						if (_objectInfo[o][4] == 1)
							_objectMap[lx][ly - 1] = 3;

						eventText("Found Mega Flask!");
					} else {
						if (config.effects) {
							int snd = playSound(_sfx[kSndChest]);
							setChannelVolume(snd, config.effectsvol);
						}

						eventText("Cannot Carry any more Mega Flasks!");
					}
					_itemticks = _ticks + 215;
					return;
					break;
				case kScriptLightningChest:
					if (_player.inventory[kInvShock] < 9) {
						_player.inventory[kInvShock]++;
						addFloatIcon(17, lx * 16, (ly - 1) * 16);

						_objmapf[_curmap][lx][ly - 1] = 1;

						if (config.effects) {
							int snd = playSound(_sfx[kSndPowerUp]);
							setChannelVolume(snd, config.effectsvol);
						}

						if (_objectInfo[o][4] == 1)
							_objectMap[lx][ly - 1] = 3;

						eventText("Found Lightning Bomb!");
					} else {
						if (config.effects) {
							int snd = playSound(_sfx[kSndChest]);
							setChannelVolume(snd, config.effectsvol);
						}

						eventText("Cannot Carry any more Lightning Bombs!");
					}
					_itemticks = _ticks + 215;
					return;
					break;
				case kScriptArmourChest:
					if (_player.armour == 1) {
						_player.armour = 2;

						addFloatIcon(5, lx * 16, (ly - 1) * 16);

						if (config.effects) {
							int snd = playSound(_sfx[kSndPowerUp]);
							setChannelVolume(snd, config.effectsvol);
						}

						if (_objectInfo[o][4] == 1)
							_objectMap[lx][ly - 1] = 3;

						eventText("Found the Fidelis Mail!");
						_itemticks = _ticks + 215;
						return;
					}
					break;
				case kScriptLever:
					if (_curmap == 58 && _scriptflag[kScriptLever][0] == 0) {
						_scriptflag[kScriptLever][0] = 1;

						if (config.effects) {
							int snd = playSound(_sfx[kSndLever]);
							setChannelVolume(snd, config.effectsvol);
						}
					} else if (_curmap == 58 && _scriptflag[kScriptLever][0] > 0) {
						if (config.effects) {
							int snd = playSound(_sfx[kSndDoor]);
							setChannelVolume(snd, config.effectsvol);
						}

						eventText("It's stuck!");
					} else if (_curmap == 54 && _scriptflag[kScriptLever][0] == 1) {
						if (config.effects) {
							int snd = playSound(_sfx[kSndLever]);
							setChannelVolume(snd, config.effectsvol);
						}

						_scriptflag[kScriptLever][0] = 2;
					} else if (_curmap == 54 && _scriptflag[kScriptLever][0] > 1) {
						if (config.effects) {
							int snd = playSound(_sfx[kSndDoor]);
							setChannelVolume(snd, config.effectsvol);
						}

						eventText("It's stuck!");
					}
					break;
				case kScriptGetSword3:
					if (_player.sword < 3) {
						_player.sword = 3;

						addFloatIcon(18, lx * 16, (ly - 1) * 16);

						_itemticks = _ticks + 215;

						if (config.effects) {
							int snd = playSound(_sfx[kSndPowerUp]);
							setChannelVolume(snd, config.effectsvol);
						}

						if (_objectInfo[o][4] == 1)
							_objectMap[lx][ly - 1] = 3;
						eventText("Found the Blood Sword!");
						_objmapf[4][1][2] = 1;
						return;
					}
					break;
				case kScriptShield3:
					if (_player.shield < 3) {
						_player.shield = 3;
						addFloatIcon(19, lx * 16, (ly - 1) * 16);
						_itemticks = _ticks + 215;

						if (config.effects) {
							int snd = playSound(_sfx[kSndPowerUp]);
							setChannelVolume(snd, config.effectsvol);
						}

						if (_objectInfo[o][4] == 1)
							_objectMap[lx][ly - 1] = 3;
						eventText("Found the Entropy Shield!");
						_objmapf[4][1][2] = 1;
						return;
					}
					break;
				case kScriptArmour3:
					if (_player.armour < 3) {
						_player.armour = 3;
						addFloatIcon(20, lx * 16, (ly - 1) * 16);
						_itemticks = _ticks + 215;

						if (config.effects) {
							int snd = playSound(_sfx[kSndPowerUp]);
							setChannelVolume(snd, config.effectsvol);
						}

						if (_objectInfo[o][4] == 1)
							_objectMap[lx][ly - 1] = 3;
						eventText("Found the Rubyscale Armour!");
						_objmapf[4][1][2] = 1;
						return;
					}
					break;
				}
			}
		}
	}

	attacking = true;
	_player.attackFrame = 0;
	movingup = false;
	movingdown = false;
	movingleft = false;
	movingright = false;

	for (int i = 0; i <= 15; i++) {
		for (int a = 0; a <= 3; a++) {
			_playerattackofs[a][i][2] = 0;
		}
	}
}

void GriffonEngine::castSpell(int spellnum, float homex, float homey, float enemyx, float enemyy, int damagewho) {
	// spellnum 7 = sprite 6 spitfire

	for (int i = 0; i < kMaxSpell; i++) {
		if (ABS(spellinfo[i].frame) < kEpsilon) {
			spellinfo[i].homex = homex;
			spellinfo[i].homey = homey;
			spellinfo[i].enemyx = enemyx;
			spellinfo[i].enemyy = enemyy;
			spellinfo[i].spellnum = spellnum;
			int dw = 0;
			int npc = 0;
			if (damagewho > 0) {
				dw = 1;
				npc = damagewho;
			}

			spellinfo[i].damagewho = dw;
			spellinfo[i].npc = npc;

			spellinfo[i].frame = 32.0f;
			if (damagewho == 0) {
				spellinfo[i].strength = _player.spellStrength / 100;
				if (ABS(_player.spellStrength - 100) < kEpsilon)
					spellinfo[i].strength = 1.5f;
			}

			// set earthslide vars
			if (spellnum == 2) {
				for (int f = 0; f <= 8; f++) {
					spellinfo[i].rocky[f] = 0;
					spellinfo[i].rockimg[f] = (int)(RND() * 4);
					spellinfo[i].rockdeflect[f] = ((int)(RND() * 128) - 64) * 1.5;
				}
			}

			// set fire vars
			if (spellnum == 3) {
				for (int f = 0; f <= 4; f++) {
					spellinfo[i].legalive[f] = 32;
				}
			}


			// room fireball vars
			if (spellnum == 6) {
				int nballs = 0;
				for (int x = 0; x <= 19; x++) {
					for (int y = 0; y <= 14; y++) {
						if ((_objectMap[x][y] == 1 || _objectMap[x][y] == 2) && nballs < 5 && (int)(RND() * 4) == 0) {
							int ax = x * 16;
							int ay = y * 16;

							spellinfo[i].fireballs[nballs][0] = ax;
							spellinfo[i].fireballs[nballs][1] = ay;
							spellinfo[i].fireballs[nballs][2] = 0;
							spellinfo[i].fireballs[nballs][3] = 0;

							spellinfo[i].ballon[nballs] = 1;
							++nballs;
						}
					}
				}
				spellinfo[i].nfballs = nballs;
			}

			if (config.effects) {
				if (spellnum == 1) {
					int snd = playSound(_sfx[kSndThrow]);
					setChannelVolume(snd, config.effectsvol);
				} else if (spellnum == 5) {
					int snd = playSound(_sfx[kSndCrystal]);
					setChannelVolume(snd, config.effectsvol);
				} else if (spellnum == 8 || spellnum == 9) {
					int snd = playSound(_sfx[kSndLightning]);
					setChannelVolume(snd, config.effectsvol);
				}
			}

			return;
		}
	}
}

void GriffonEngine::checkHit() {
	if (attacking) {
		for (int i = 1; i <= _lastnpc; i++) {
			if (_npcinfo[i].hp > 0 && _npcinfo[i].pause < _ticks && (int)(RND() * 2) == 0) {
				float npx = _npcinfo[i].x;
				float npy = _npcinfo[i].y;

				float xdif = _player.px - npx;
				float ydif = _player.py - npy;

				float ps = _player.sword;
				if (ps > 1)
					ps *= 0.75;
				float damage = (float)_player.swordDamage * (1.0 + RND() * 1.0) * _player.attackStrength / 100.0 * ps;

				if (_console->_godMode)
					damage = 1000;

				if (ABS(_player.attackStrength - 100) < kEpsilon)
					damage *= 1.5;

				bool hit = false;
				if (_player.walkDir == 0) {
					if (abs(xdif) <= 8 && ydif >= 0 && ydif < 8)
						hit = true;
				} else if (_player.walkDir == 1) {
					if (abs(xdif) <= 8 && ydif <= 0 && ydif > -8)
						hit = true;
				} else if (_player.walkDir == 2) {
					if (abs(ydif) <= 8 && xdif >= -8 && xdif < 8)
						hit = true;
				} else if (_player.walkDir == 3) {
					if (abs(ydif) <= 8 && xdif <= 8 && xdif > -8)
						hit = true;
				}

				if (hit) {
					if (config.effects) {
						int snd = playSound(_sfx[kSndSwordHit]);
						setChannelVolume(snd, config.effectsvol);
					}

					damageNPC(i, damage, 0);
				}
			}
		}
	}
}

void GriffonEngine::damageNPC(int npcnum, int damage, int spell) {
	char line[256];
	int fcol;

	if (damage == 0) {
		strcpy(line, "miss!");
		fcol = 2;
	} else {
		int ratio = 0;
		int heal = 0;
		if (damage < 0)
			heal = 1;
		damage = abs(damage);

		if (heal == 0) {
			if (damage > _npcinfo[npcnum].hp) {
				ratio = (damage - _npcinfo[npcnum].hp) * 100 / damage;
				damage = _npcinfo[npcnum].hp;
			}

			_npcinfo[npcnum].hp -= damage;
			if (_npcinfo[npcnum].hp < 0)
				_npcinfo[npcnum].hp = 0;

			sprintf(line, "-%i", damage);
			fcol = 1;
		} else {
			_npcinfo[npcnum].hp += damage;
			if (_npcinfo[npcnum].hp > _npcinfo[npcnum].maxhp)
				_npcinfo[npcnum].hp = _npcinfo[npcnum].maxhp;

			sprintf(line, "+%i", damage);
			fcol = 5;
		}

		_npcinfo[npcnum].pause = _ticks + 900;

		if (spell == 0)
			_player.attackStrength = ratio;
	}

	addFloatText(line, _npcinfo[npcnum].x + 12 - 4 * strlen(line), _npcinfo[npcnum].y + 16, fcol);

	if (_npcinfo[npcnum].spriteset == kMonsterBatKitty)
		castSpell(9, _npcinfo[npcnum].x, _npcinfo[npcnum].y, _player.px, _player.py, npcnum);

	// if enemy is killed
	if (_npcinfo[npcnum].hp == 0) {
		_player.exp += _npcinfo[npcnum].maxhp;

		if (_npcinfo[npcnum].spriteset == kMonsterBabyDragon || _npcinfo[npcnum].spriteset == kMonsterPriest ||
				_npcinfo[npcnum].spriteset == kMonsterRedDragon) {
			int ff = (int)(RND() * _player.level * 3);
			if (ff == 0) {
				float npx = _npcinfo[npcnum].x + 12;
				float npy = _npcinfo[npcnum].y + 20;

				int lx = (int)npx / 16;
				int ly = (int)npy / 16;

				if (_objectMap[lx][ly] == -1)
					_objectMap[lx][ly] = 4;
			}
		}

		if (_npcinfo[npcnum].spriteset == kMonsterOneWing || _npcinfo[npcnum].spriteset == kMonsterTwoWing || 
					_npcinfo[npcnum].spriteset == kMonsterBlackKnight || _npcinfo[npcnum].spriteset == kMonsterFireHydra) {
			int ff = (int)(RND() * _player.level);
			if (ff == 0) {
				float npx = _npcinfo[npcnum].x + 12;
				float npy = _npcinfo[npcnum].y + 20;

				int lx = (int)npx / 16;
				int ly = (int)npy / 16;

				if (_objectMap[lx][ly] == -1)
					_objectMap[lx][ly] = 12;
			}
		}

		if (_npcinfo[npcnum].spriteset == kMonsterTwoWing || _npcinfo[npcnum].spriteset == kMonsterDragon2 ||
				_npcinfo[npcnum].spriteset == kMonsterFireHydra) {
			int ff = (int)(RND() * _player.level * 2);
			if (ff == 0) {
				float npx = _npcinfo[npcnum].x + 12;
				float npy = _npcinfo[npcnum].y + 20;

				int lx = (int)npx / 16;
				int ly = (int)npy / 16;

				if (_objectMap[lx][ly] == -1)
					_objectMap[lx][ly] = 13;
			}
		}

		// academy master key chest script
		if (_npcinfo[npcnum].script == kScriptMasterKey) {
			bool alive = false;
			for (int i = 1; i <= _lastnpc; i++) {
				if (_npcinfo[i].hp > 0)
					alive = true;
			}

			if (!alive) {
				int cx = 9;
				int cy = 7;

				_objectMap[cx][cy] = 5;

				rcDest.left = cx * 8;
				rcDest.top = cy * 8;
				rcDest.setWidth(8);
				rcDest.setHeight(8);

				float npx = _player.px + 12;
				float npy = _player.py + 20;

				int lx = (int)npx / 16;
				int ly = (int)npy / 16;

				if (lx == cx && ly == cy)
					_player.py += 16;
				_clipbg2->fillRect(rcDest, _clipbg->format.RGBToColor(255, 255, 255));
				_scriptflag[kScriptMasterKey][0] = 1;
			}
		}

		// academy crystal chest script
		if (_npcinfo[npcnum].script == kScriptFindCrystal) {
			bool alive = false;
			for (int i = 1; i <= _lastnpc; i++) {
				if (_npcinfo[i].hp > 0)
					alive = true;
			}

			if (!alive) {
				int cx = 9;
				int cy = 7;

				_objectMap[cx][cy] = 6;

				rcDest.left = cx * 8;
				rcDest.top = cy * 8;
				rcDest.setWidth(8);
				rcDest.setHeight(8);

				float npx = _player.px + 12;
				float npy = _player.py + 20;

				int lx = (int)npx / 16;
				int ly = (int)npy / 16;

				if (lx == cx && ly == cy)
					_player.py += 16;
				_scriptflag[kScriptFindCrystal][0] = 1;
				_clipbg2->fillRect(rcDest, _clipbg->format.RGBToColor(255, 255, 255));
			}
		}

		// tower shield chest script
		if (_npcinfo[npcnum].script == kScriptFindShield && _scriptflag[kScriptFindShield][0] == 0) {
			_triggerloc[9][7] = 5004;

			int curTile = 40;
			int curTileL = 0;
			int curTileX = curTile % 20;
			int curTileY = (curTile - curTileX) / 20;

			int l = 0; // ?? not defined in original code
			_tileinfo[l][9][7][0] = curTile + 1;
			_tileinfo[l][9][7][1] = 0;

			rcSrc.left = curTileX * 16;
			rcSrc.top = curTileY * 16;
			rcSrc.setWidth(16);
			rcSrc.setHeight(16);

			rcDest.left = 9 * 16;
			rcDest.top = 7 * 16;
			rcDest.setWidth(16);
			rcDest.setHeight(16);

			_tiles[curTileL]->blit(*_mapbg, rcDest.left, rcDest.top, Graphics::FLIP_NONE, &rcSrc);
		}

		// firehydra sword chest
		if (_npcinfo[npcnum].script == kScriptFindSword) {
			bool alive = false;
			for (int i = 1; i <= _lastnpc; i++) {
				if (_npcinfo[i].hp > 0)
					alive = true;
			}

			if (!alive) {
				int cx = 9;
				int cy = 6;

				_objectMap[cx][cy] = 9;

				rcDest.left = cx * 8;
				rcDest.top = cy * 8;
				rcDest.setWidth(8);
				rcDest.setHeight(8);

				float npx = _player.px + 12;
				float npy = _player.py + 20;

				int lx = (int)npx / 16;
				int ly = (int)npy / 16;

				if (lx == cx && ly == cy)
					_player.py += 16;
				_scriptflag[kScriptFindSword][0] = 1;
				_clipbg2->fillRect(rcDest, _clipbg->format.RGBToColor(255, 255, 255));
			}

		}

		// gardens master key script
		if (_npcinfo[npcnum].script == kScriptGardenMasterKey && _scriptflag[kScriptKeyChest][0] == 0) {
			bool alive = false;
			for (int i = 1; i <= _lastnpc; i++) {
				if (_npcinfo[i].hp > 0)
					alive = true;
			}

			if (!alive) {
				int cx = 13;
				int cy = 7;

				_objectMap[cx][cy] = 5;

				rcDest.left = cx * 8;
				rcDest.top = cy * 8;
				rcDest.setWidth(8);
				rcDest.setHeight(8);

				float npx = _player.px + 12;
				float npy = _player.py + 20;

				int lx = (int)npx / 16;
				int ly = (int)npy / 16;

				if (lx == cx && ly == cy)
					_player.py += 16;
				_clipbg2->fillRect(rcDest, _clipbg->format.RGBToColor(255, 255, 255));
				_scriptflag[kScriptGardenMasterKey][0] = 1;
			}
		}

		// regular key chest 1
		for (int s = 20; s <= 23; s++) {
			if (_npcinfo[npcnum].script == s && _scriptflag[s][0] < 2) {
				bool alive = false;
				for (int i = 1; i <= _lastnpc; i++) {
					if (_npcinfo[i].hp > 0)
						alive = true;
				}

				if (!alive) {
					int cx = 9;
					int cy = 7;

					_objectMap[cx][cy] = 11;

					rcDest.left = cx * 8;
					rcDest.top = cy * 8;
					rcDest.setWidth(8);
					rcDest.setHeight(8);

					float npx = _player.px + 12;
					float npy = _player.py + 20;

					int lx = (int)npx / 16;
					int ly = (int)npy / 16;

					if (lx == cx && ly == cy)
						_player.py += 16;
					_scriptflag[s][0] = 1;
					_clipbg2->fillRect(rcDest, _clipbg->format.RGBToColor(255, 255, 255));
				}
			}
		}

		// pickup lightning bomb
		if (_npcinfo[npcnum].script == kScriptLightningBomb && (_curmap == 41 && _scriptflag[kScriptLightningBomb][1] == 0)) {
			bool alive = false;
			for (int i = 1; i <= _lastnpc; i++) {
				if (_npcinfo[i].hp > 0)
					alive = true;
			}

			if (!alive) {
				int cx = 9;
				int cy = 7;

				_objectMap[cx][cy] = 13;

				rcDest.left = cx * 8;
				rcDest.top = cy * 8;
				rcDest.setWidth(8);
				rcDest.setHeight(8);

				float npx = _player.px + 12;
				float npy = _player.py + 20;

				int lx = (int)npx / 16;
				int ly = (int)npy / 16;

				if (lx == cx && ly == cy)
					_player.py += 16;
			}
		}

		// citadel armour chest
		if (_npcinfo[npcnum].script == kScriptArmourChest) {
			bool alive = false;
			for (int i = 1; i <= _lastnpc; i++) {
				if (_npcinfo[i].hp > 0)
					alive = true;
			}

			if (!alive) {
				int cx = 8;
				int cy = 7;

				_objectMap[cx][cy] = 16;

				rcDest.left = cx * 8;
				rcDest.top = cy * 8;
				rcDest.setWidth(8);
				rcDest.setHeight(8);

				float npx = _player.px + 12;
				float npy = _player.py + 20;

				int lx = (int)npx / 16;
				int ly = (int)npy / 16;

				if (lx == cx && ly == cy)
					_player.py += 16;
				_scriptflag[kScriptArmourChest][0] = 1;
				_clipbg2->fillRect(rcDest, _clipbg->format.RGBToColor(255, 255, 255));
			}
		}

		// citadel master key script
		if (_npcinfo[npcnum].script == kScriptCitadelMasterKey && _scriptflag[kScriptCitadelMasterKey][0] == 0) {
			bool alive = false;
			for (int i = 1; i <= _lastnpc; i++) {
				if (_npcinfo[i].hp > 0)
					alive = true;
			}

			if (!alive) {
				int cx = 11;
				int cy = 10;

				_objectMap[cx][cy] = 5;

				rcDest.left = cx * 8;
				rcDest.top = cy * 8;
				rcDest.setWidth(8);
				rcDest.setHeight(8);

				float npx = _player.px + 12;
				float npy = _player.py + 20;

				int lx = (int)npx / 16;
				int ly = (int)npy / 16;

				if (lx == cx && ly == cy)
					_player.py += 16;
				_clipbg2->fillRect(rcDest, _clipbg->format.RGBToColor(255, 255, 255));
				_scriptflag[kScriptCitadelMasterKey][0] = 1;
			}
		}

		// max ups
		if (_npcinfo[npcnum].script == kScriptGetSword3 && _scriptflag[kScriptGetSword3][0] == 0) {
			bool alive = false;
			for (int i = 1; i <= _lastnpc; i++) {
				if (_npcinfo[i].hp > 0)
					alive = true;
			}

			if (!alive) {
				int cx = 6;
				int cy = 8;

				_objectMap[cx][cy] = 18;

				rcDest.left = cx * 8;
				rcDest.top = cy * 8;
				rcDest.setWidth(8);
				rcDest.setHeight(8);

				float npx = _player.px + 12;
				float npy = _player.py + 20;

				int lx = (int)npx / 16;
				int ly = (int)npy / 16;

				if (lx == cx && ly == cy)
					_player.py += 16;
				_clipbg2->fillRect(rcDest, _clipbg->format.RGBToColor(255, 255, 255));
				_scriptflag[kScriptGetSword3][0] = 1;

				cx = 9;
				cy = 8;

				_objectMap[cx][cy] = 19;

				rcDest.left = cx * 8;
				rcDest.top = cy * 8;
				rcDest.setWidth(8);
				rcDest.setHeight(8);

				npx = _player.px + 12;
				npy = _player.py + 20;

				lx = (int)npx / 16;
				ly = (int)npy / 16;

				if (lx == cx && ly == cy)
					_player.py += 16;
				_clipbg2->fillRect(rcDest, _clipbg->format.RGBToColor(255, 255, 255));

				_scriptflag[kScriptShield3][0] = 1;

				cx = 12;
				cy = 8;

				_objectMap[cx][cy] = 20;

				rcDest.left = cx * 8;
				rcDest.top = cy * 8;
				rcDest.setWidth(8);
				rcDest.setHeight(8);

				npx = _player.px + 12;
				npy = _player.py + 20;

				lx = (int)npx / 16;
				ly = (int)npy / 16;

				if (lx == cx && ly == cy)
					_player.py += 16;
				_clipbg2->fillRect(rcDest, _clipbg->format.RGBToColor(255, 255, 255));
				_scriptflag[kScriptArmour3][0] = 1;
			}
		}

		if (_npcinfo[npcnum].script == kScriptEndOfGame)
			endOfGame();
	}
}

void GriffonEngine::damagePlayer(int damage) {
	char line[256];

	if (!_console->_godMode)
		_player.hp -= damage;

	if (_player.hp < 0)
		_player.hp = 0;

	sprintf(line, "-%i", damage);
	if (damage == 0)
		strcpy(line, "miss!");

	addFloatText(line, _player.px + 12 - 4 * strlen(line), _player.py + 16, 4);

	_player.pause = _ticks + 1000;
}


} // end of namespace Griffon
