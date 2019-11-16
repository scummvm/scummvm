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
			if (ly > 1 && (_curMap == 54 || _curMap == 58))
				o2 = _objectMap[lx][ly - 2];

			// cst
			if ((_objectInfo[o].type == 1 && (o == 0 || o > 4)) || (_objectInfo[o2].type == 0 && o2 == 10)) {
				if (o2 == 10)
					o = 10;

				int oscript = _objectInfo[o].script;
				switch (oscript) {
				case kScriptFlask:
					if (_player.inventory[kInvFlask] < 9) {
						_player.inventory[kInvFlask]++;
						addFloatIcon(6, lx * 16, (ly - 1) * 16);

						_objectMapFull[_curMap][lx][ly - 1] = 1;

						if (config.effects) {
							int snd = playSound(_sfx[kSndPowerUp]);
							setChannelVolume(snd, config.effectsVol);
						}

						if (_objectInfo[o].type == 1)
							_objectMap[lx][ly - 1] = 3;

						eventText("Found Flask!");
					} else {
						if (config.effects) {
							int snd = playSound(_sfx[kSndChest]);
							setChannelVolume(snd, config.effectsVol);
						}

						eventText("Cannot Carry any more Flasks!");
					}
					_itemTicks = _ticks + 215;
					return;
					break;
				case kScriptMasterKey:
					_player.inventory[kInvMasterKey]++;

					addFloatIcon(14, lx * 16, (ly - 1) * 16);

					_itemTicks = _ticks + 215;

					if (_curMap == 34)
						_scriptFlag[kScriptMasterKey][0] = 2;
					else if (_curMap == 62)
						_scriptFlag[kScriptGardenMasterKey][0] = 2;
					else if (_curMap == 81)
						_scriptFlag[kScriptCitadelMasterKey][0] = 2;

					if (config.effects) {
						int snd = playSound(_sfx[kSndPowerUp]);
						setChannelVolume(snd, config.effectsVol);
					}

					if (_objectInfo[o].type == 1)
						_objectMap[lx][ly - 1] = 3;
					eventText("Found the Temple Key!");
					return;
				case kScriptFindCrystal:
					_player.foundSpell[0] = 1;
					_player.spellCharge[0] = 0;

					addFloatIcon(7, lx * 16, (ly - 1) * 16);

					if (config.effects) {
						int snd = playSound(_sfx[kSndPowerUp]);
						setChannelVolume(snd, config.effectsVol);
					}

					if (_objectInfo[o].type == 1)
						_objectMap[lx][ly - 1] = 3;

					eventText("Found the Infinite Crystal!");
					_itemTicks = _ticks + 215;
					return;
				case kScriptFindShield:
					if (_player.shield == 1) {
						_player.shield = 2;

						addFloatIcon(4, lx * 16, (ly - 1) * 16);

						_itemTicks = _ticks + 215;

						if (config.effects) {
							int snd = playSound(_sfx[kSndPowerUp]);
							setChannelVolume(snd, config.effectsVol);
						}

						if (_objectInfo[o].type == 1)
							_objectMap[lx][ly - 1] = 3;

						eventText("Found the Obsidian Shield!");
						_objectMapFull[4][1][2] = 1;
						return;
					}
					break;
				case kScriptFindSword:
					if (_player.sword == 1) {
						_player.sword = 2;

						addFloatIcon(3, lx * 16, (ly - 1) * 16);

						_itemTicks = _ticks + 215;

						if (config.effects) {
							int snd = playSound(_sfx[kSndPowerUp]);
							setChannelVolume(snd, config.effectsVol);
						}

						if (_objectInfo[o].type == 1)
							_objectMap[lx][ly - 1] = 3;
						eventText("Found the Fidelis Sword!");
						return;
					}
					break;
				case kScriptKeyChest:
					if (_player.inventory[kInvNormalKey] < 9) {
						_player.inventory[kInvNormalKey]++;

						for (int s = 20; s <= 23; s++) {
							if (_scriptFlag[s][0] == 1) {
								_scriptFlag[s][0] = 2;
							}
						}

						if (config.effects) {
							int snd = playSound(_sfx[kSndPowerUp]);
							setChannelVolume(snd, config.effectsVol);
						}

						_objectMapFull[_curMap][lx][ly - 1] = 1;

						if (_objectInfo[o].type == 1)
							_objectMap[lx][ly - 1] = 3;

						eventText("Found Key");
						addFloatIcon(16, lx * 16, (ly - 1) * 16);
					} else {
						if (config.effects) {
							int snd = playSound(_sfx[kSndChest]);
							setChannelVolume(snd, config.effectsVol);
						}

						eventText("Cannot Carry Any More Keys");
					}
					break;
				case kScriptBlueFlask:
					if (_player.inventory[kInvDoubleFlask] < 9) {
						_player.inventory[kInvDoubleFlask]++;
						addFloatIcon(12, lx * 16, (ly - 1) * 16);

						_objectMapFull[_curMap][lx][ly - 1] = 1;

						if (config.effects) {
							int snd = playSound(_sfx[kSndPowerUp]);
							setChannelVolume(snd, config.effectsVol);
						}

						if (_objectInfo[o].type == 1)
							_objectMap[lx][ly - 1] = 3;

						eventText("Found Mega Flask!");
					} else {
						if (config.effects) {
							int snd = playSound(_sfx[kSndChest]);
							setChannelVolume(snd, config.effectsVol);
						}

						eventText("Cannot Carry any more Mega Flasks!");
					}

					_itemTicks = _ticks + 215;
					return;
					break;
				case kScriptBlueFlaskChest:
					if (_player.inventory[kInvDoubleFlask] < 9) {
						_player.inventory[kInvDoubleFlask]++;
						addFloatIcon(12, lx * 16, (ly - 1) * 16);

						_objectMapFull[_curMap][lx][ly - 1] = 1;

						if (config.effects) {
							int snd = playSound(_sfx[kSndPowerUp]);
							setChannelVolume(snd, config.effectsVol);
						}

						if (_objectInfo[o].type == 1)
							_objectMap[lx][ly - 1] = 3;

						eventText("Found Mega Flask!");
					} else {
						if (config.effects) {
							int snd = playSound(_sfx[kSndChest]);
							setChannelVolume(snd, config.effectsVol);
						}

						eventText("Cannot Carry any more Mega Flasks!");
					}
					_itemTicks = _ticks + 215;
					return;
					break;
				case kScriptLightningChest:
					if (_player.inventory[kInvShock] < 9) {
						_player.inventory[kInvShock]++;
						addFloatIcon(17, lx * 16, (ly - 1) * 16);

						_objectMapFull[_curMap][lx][ly - 1] = 1;

						if (config.effects) {
							int snd = playSound(_sfx[kSndPowerUp]);
							setChannelVolume(snd, config.effectsVol);
						}

						if (_objectInfo[o].type == 1)
							_objectMap[lx][ly - 1] = 3;

						eventText("Found Lightning Bomb!");
					} else {
						if (config.effects) {
							int snd = playSound(_sfx[kSndChest]);
							setChannelVolume(snd, config.effectsVol);
						}

						eventText("Cannot Carry any more Lightning Bombs!");
					}
					_itemTicks = _ticks + 215;
					return;
					break;
				case kScriptArmourChest:
					if (_player.armour == 1) {
						_player.armour = 2;

						addFloatIcon(5, lx * 16, (ly - 1) * 16);

						if (config.effects) {
							int snd = playSound(_sfx[kSndPowerUp]);
							setChannelVolume(snd, config.effectsVol);
						}

						if (_objectInfo[o].type == 1)
							_objectMap[lx][ly - 1] = 3;

						eventText("Found the Fidelis Mail!");
						_itemTicks = _ticks + 215;
						return;
					}
					break;
				case kScriptLever:
					if (_curMap == 58 && _scriptFlag[kScriptLever][0] == 0) {
						_scriptFlag[kScriptLever][0] = 1;

						if (config.effects) {
							int snd = playSound(_sfx[kSndLever]);
							setChannelVolume(snd, config.effectsVol);
						}
					} else if (_curMap == 58 && _scriptFlag[kScriptLever][0] > 0) {
						if (config.effects) {
							int snd = playSound(_sfx[kSndDoor]);
							setChannelVolume(snd, config.effectsVol);
						}

						eventText("It's stuck!");
					} else if (_curMap == 54 && _scriptFlag[kScriptLever][0] == 1) {
						if (config.effects) {
							int snd = playSound(_sfx[kSndLever]);
							setChannelVolume(snd, config.effectsVol);
						}

						_scriptFlag[kScriptLever][0] = 2;
					} else if (_curMap == 54 && _scriptFlag[kScriptLever][0] > 1) {
						if (config.effects) {
							int snd = playSound(_sfx[kSndDoor]);
							setChannelVolume(snd, config.effectsVol);
						}

						eventText("It's stuck!");
					}
					break;
				case kScriptGetSword3:
					if (_player.sword < 3) {
						_player.sword = 3;

						addFloatIcon(18, lx * 16, (ly - 1) * 16);

						_itemTicks = _ticks + 215;

						if (config.effects) {
							int snd = playSound(_sfx[kSndPowerUp]);
							setChannelVolume(snd, config.effectsVol);
						}

						if (_objectInfo[o].type == 1)
							_objectMap[lx][ly - 1] = 3;
						eventText("Found the Blood Sword!");
						_objectMapFull[4][1][2] = 1;
						return;
					}
					break;
				case kScriptShield3:
					if (_player.shield < 3) {
						_player.shield = 3;
						addFloatIcon(19, lx * 16, (ly - 1) * 16);
						_itemTicks = _ticks + 215;

						if (config.effects) {
							int snd = playSound(_sfx[kSndPowerUp]);
							setChannelVolume(snd, config.effectsVol);
						}

						if (_objectInfo[o].type == 1)
							_objectMap[lx][ly - 1] = 3;
						eventText("Found the Entropy Shield!");
						_objectMapFull[4][1][2] = 1;
						return;
					}
					break;
				case kScriptArmour3:
					if (_player.armour < 3) {
						_player.armour = 3;
						addFloatIcon(20, lx * 16, (ly - 1) * 16);
						_itemTicks = _ticks + 215;

						if (config.effects) {
							int snd = playSound(_sfx[kSndPowerUp]);
							setChannelVolume(snd, config.effectsVol);
						}

						if (_objectInfo[o].type == 1)
							_objectMap[lx][ly - 1] = 3;
						eventText("Found the Rubyscale Armour!");
						_objectMapFull[4][1][2] = 1;
						return;
					}
					break;
				default:
					break;
				}
			}
		}
	}

	_attacking = true;
	_player.attackFrame = 0;
	_movingUp = false;
	_movingDown = false;
	_movingLeft = false;
	_movingRight = false;

	for (int i = 0; i <= 15; i++) {
		for (int a = 0; a <= 3; a++) {
			_playerAttackOfs[a][i].completed = false;
		}
	}
}

void GriffonEngine::castSpell(int spellnum, float homex, float homey, float enemyx, float enemyy, int damagewho) {
	// spellnum 7 = sprite 6 spitfire

	for (int i = 0; i < kMaxSpell; i++) {
		if (ABS(_spellInfo[i].frame) < kEpsilon) {
			_spellInfo[i].homex = homex;
			_spellInfo[i].homey = homey;
			_spellInfo[i].enemyx = enemyx;
			_spellInfo[i].enemyy = enemyy;
			_spellInfo[i].spellnum = spellnum;
			int dw = 0;
			int npc = 0;
			if (damagewho > 0) {
				dw = 1;
				npc = damagewho;
			}

			_spellInfo[i].damagewho = dw;
			_spellInfo[i].npc = npc;

			_spellInfo[i].frame = 32.0f;
			if (damagewho == 0) {
				_spellInfo[i].strength = _player.spellStrength / 100;
				if (ABS(_player.spellStrength - 100) < kEpsilon)
					_spellInfo[i].strength = 1.5f;
			}

			switch(spellnum) {
			case 1:
				if (config.effects) {
					int snd = playSound(_sfx[kSndThrow]);
					setChannelVolume(snd, config.effectsVol);
				}
				break;
			case 2:
				// set earthslide vars
				for (int f = 0; f <= 8; f++) {
					_spellInfo[i].rocky[f] = 0;
					_spellInfo[i].rockimg[f] = (int)(RND() * 4);
					_spellInfo[i].rockdeflect[f] = ((int)(RND() * 128) - 64) * 1.5;
				}
				break;
			case 3:
				// set fire vars
				for (int f = 0; f <= 4; f++)
					_spellInfo[i].legalive[f] = 32;

				break;
			case 5:
				if (config.effects) {
					int snd = playSound(_sfx[kSndCrystal]);
					setChannelVolume(snd, config.effectsVol);
				}
				break;
			case 6: {
				// room fireball vars
				int nballs = 0;
				for (int x = 0; x <= 19; x++) {
					for (int y = 0; y <= 14; y++) {
						if ((_objectMap[x][y] == 1 || _objectMap[x][y] == 2) && nballs < 5 && (int)(RND() * 4) == 0) {
							int ax = x * 16;
							int ay = y * 16;

							_spellInfo[i].fireballs[nballs][0] = ax;
							_spellInfo[i].fireballs[nballs][1] = ay;
							_spellInfo[i].fireballs[nballs][2] = 0;
							_spellInfo[i].fireballs[nballs][3] = 0;

							_spellInfo[i].ballon[nballs] = 1;
							++nballs;
						}
					}
				}
				_spellInfo[i].nfballs = nballs;
				}
				break;
			case 8:
			case 9:
				if (config.effects) {
					int snd = playSound(_sfx[kSndLightning]);
					setChannelVolume(snd, config.effectsVol);
				}
				break;
			default:
				break;
			}

			return;
		}
	}
}

void GriffonEngine::checkHit() {
	if (_attacking) {
		for (int i = 1; i <= _lastNpc; i++) {
			if (_npcInfo[i].hp > 0 && _npcInfo[i].pause < _ticks && (int)(RND() * 2) == 0) {
				float npx = _npcInfo[i].x;
				float npy = _npcInfo[i].y;

				float xdif = _player.px - npx;
				float ydif = _player.py - npy;

				float ps = _player.sword;
				if (ps > 1)
					ps *= 0.75;
				float damage = (float)_player.swordDamage * (1.0 + RND() * 1.0) * _player.attackStrength / 100.0 * ps;

				if (_console->_godMode == kGodModeAll)
					damage = 1000;

				if (ABS(_player.attackStrength - 100) < kEpsilon)
					damage *= 1.5;

				bool hit = false;
				if (_player.walkDir == 0) {
					if (ABS(xdif) <= 8 && ydif >= 0 && ydif < 8)
						hit = true;
				} else if (_player.walkDir == 1) {
					if (ABS(xdif) <= 8 && ydif <= 0 && ydif > -8)
						hit = true;
				} else if (_player.walkDir == 2) {
					if (ABS(ydif) <= 8 && xdif >= -8 && xdif < 8)
						hit = true;
				} else if (_player.walkDir == 3) {
					if (ABS(ydif) <= 8 && xdif <= 8 && xdif > -8)
						hit = true;
				}

				if (hit) {
					if (config.effects) {
						int snd = playSound(_sfx[kSndSwordHit]);
						setChannelVolume(snd, config.effectsVol);
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
		damage = ABS(damage);

		if (heal == 0) {
			if (damage > _npcInfo[npcnum].hp) {
				ratio = (damage - _npcInfo[npcnum].hp) * 100 / damage;
				damage = _npcInfo[npcnum].hp;
			}

			_npcInfo[npcnum].hp -= damage;
			if (_npcInfo[npcnum].hp < 0)
				_npcInfo[npcnum].hp = 0;

			sprintf(line, "-%i", damage);
			fcol = 1;
		} else {
			_npcInfo[npcnum].hp += damage;
			if (_npcInfo[npcnum].hp > _npcInfo[npcnum].maxhp)
				_npcInfo[npcnum].hp = _npcInfo[npcnum].maxhp;

			sprintf(line, "+%i", damage);
			fcol = 5;
		}

		_npcInfo[npcnum].pause = _ticks + 900;

		if (spell == 0)
			_player.attackStrength = ratio;
	}

	addFloatText(line, _npcInfo[npcnum].x + 12 - 4 * strlen(line), _npcInfo[npcnum].y + 16, fcol);

	if (_npcInfo[npcnum].spriteset == kMonsterBatKitty)
		castSpell(9, _npcInfo[npcnum].x, _npcInfo[npcnum].y, _player.px, _player.py, npcnum);

	// if enemy is killed
	if (_npcInfo[npcnum].hp == 0) {
		_player.exp += _npcInfo[npcnum].maxhp;

		if (_npcInfo[npcnum].spriteset == kMonsterBabyDragon || _npcInfo[npcnum].spriteset == kMonsterPriest ||
				_npcInfo[npcnum].spriteset == kMonsterRedDragon) {
			int ff = (int)(RND() * _player.level * 3);
			if (ff == 0) {
				float npx = _npcInfo[npcnum].x + 12;
				float npy = _npcInfo[npcnum].y + 20;

				int lx = (int)npx / 16;
				int ly = (int)npy / 16;

				if (_objectMap[lx][ly] == -1)
					_objectMap[lx][ly] = 4;
			}
		}

		if (_npcInfo[npcnum].spriteset == kMonsterOneWing || _npcInfo[npcnum].spriteset == kMonsterTwoWing ||
					_npcInfo[npcnum].spriteset == kMonsterBlackKnight || _npcInfo[npcnum].spriteset == kMonsterFireHydra) {
			int ff = (int)(RND() * _player.level);
			if (ff == 0) {
				float npx = _npcInfo[npcnum].x + 12;
				float npy = _npcInfo[npcnum].y + 20;

				int lx = (int)npx / 16;
				int ly = (int)npy / 16;

				if (_objectMap[lx][ly] == -1)
					_objectMap[lx][ly] = 12;
			}
		}

		if (_npcInfo[npcnum].spriteset == kMonsterTwoWing || _npcInfo[npcnum].spriteset == kMonsterDragon2 ||
				_npcInfo[npcnum].spriteset == kMonsterFireHydra) {
			int ff = (int)(RND() * _player.level * 2);
			if (ff == 0) {
				float npx = _npcInfo[npcnum].x + 12;
				float npy = _npcInfo[npcnum].y + 20;

				int lx = (int)npx / 16;
				int ly = (int)npy / 16;

				if (_objectMap[lx][ly] == -1)
					_objectMap[lx][ly] = 13;
			}
		}

		// academy master key chest script
		if (_npcInfo[npcnum].script == kScriptMasterKey) {
			bool alive = false;
			for (int i = 1; i <= _lastNpc; i++) {
				if (_npcInfo[i].hp > 0)
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
				_clipBg2->fillRect(rcDest, _clipBg->format.RGBToColor(255, 255, 255));
				_scriptFlag[kScriptMasterKey][0] = 1;
			}
		}

		// academy crystal chest script
		if (_npcInfo[npcnum].script == kScriptFindCrystal) {
			bool alive = false;
			for (int i = 1; i <= _lastNpc; i++) {
				if (_npcInfo[i].hp > 0)
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
				_scriptFlag[kScriptFindCrystal][0] = 1;
				_clipBg2->fillRect(rcDest, _clipBg->format.RGBToColor(255, 255, 255));
			}
		}

		// tower shield chest script
		if (_npcInfo[npcnum].script == kScriptFindShield && _scriptFlag[kScriptFindShield][0] == 0) {
			_triggerLoc[9][7] = 5004;

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

			_tiles[curTileL]->blit(*_mapBg, rcDest.left, rcDest.top, Graphics::FLIP_NONE, &rcSrc);
		}

		// firehydra sword chest
		if (_npcInfo[npcnum].script == kScriptFindSword) {
			bool alive = false;
			for (int i = 1; i <= _lastNpc; i++) {
				if (_npcInfo[i].hp > 0)
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
				_scriptFlag[kScriptFindSword][0] = 1;
				_clipBg2->fillRect(rcDest, _clipBg->format.RGBToColor(255, 255, 255));
			}

		}

		// gardens master key script
		if (_npcInfo[npcnum].script == kScriptGardenMasterKey && _scriptFlag[kScriptKeyChest][0] == 0) {
			bool alive = false;
			for (int i = 1; i <= _lastNpc; i++) {
				if (_npcInfo[i].hp > 0)
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
				_clipBg2->fillRect(rcDest, _clipBg->format.RGBToColor(255, 255, 255));
				_scriptFlag[kScriptGardenMasterKey][0] = 1;
			}
		}

		// regular key chest 1
		for (int s = 20; s <= 23; s++) {
			if (_npcInfo[npcnum].script == s && _scriptFlag[s][0] < 2) {
				bool alive = false;
				for (int i = 1; i <= _lastNpc; i++) {
					if (_npcInfo[i].hp > 0)
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
					_scriptFlag[s][0] = 1;
					_clipBg2->fillRect(rcDest, _clipBg->format.RGBToColor(255, 255, 255));
				}
			}
		}

		// pickup lightning bomb
		if (_npcInfo[npcnum].script == kScriptLightningBomb && (_curMap == 41 && _scriptFlag[kScriptLightningBomb][1] == 0)) {
			bool alive = false;
			for (int i = 1; i <= _lastNpc; i++) {
				if (_npcInfo[i].hp > 0)
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
		if (_npcInfo[npcnum].script == kScriptArmourChest) {
			bool alive = false;
			for (int i = 1; i <= _lastNpc; i++) {
				if (_npcInfo[i].hp > 0)
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
				_scriptFlag[kScriptArmourChest][0] = 1;
				_clipBg2->fillRect(rcDest, _clipBg->format.RGBToColor(255, 255, 255));
			}
		}

		// citadel master key script
		if (_npcInfo[npcnum].script == kScriptCitadelMasterKey && _scriptFlag[kScriptCitadelMasterKey][0] == 0) {
			bool alive = false;
			for (int i = 1; i <= _lastNpc; i++) {
				if (_npcInfo[i].hp > 0)
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
				_clipBg2->fillRect(rcDest, _clipBg->format.RGBToColor(255, 255, 255));
				_scriptFlag[kScriptCitadelMasterKey][0] = 1;
			}
		}

		// max ups
		if (_npcInfo[npcnum].script == kScriptGetSword3 && _scriptFlag[kScriptGetSword3][0] == 0) {
			bool alive = false;
			for (int i = 1; i <= _lastNpc; i++) {
				if (_npcInfo[i].hp > 0)
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
				_clipBg2->fillRect(rcDest, _clipBg->format.RGBToColor(255, 255, 255));
				_scriptFlag[kScriptGetSword3][0] = 1;

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
				_clipBg2->fillRect(rcDest, _clipBg->format.RGBToColor(255, 255, 255));

				_scriptFlag[kScriptShield3][0] = 1;

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
				_clipBg2->fillRect(rcDest, _clipBg->format.RGBToColor(255, 255, 255));
				_scriptFlag[kScriptArmour3][0] = 1;
			}
		}

		if (_npcInfo[npcnum].script == kScriptEndOfGame) {
			endOfGame();
			_gameMode = kGameModeIntro;
		}
	}
}

void GriffonEngine::damagePlayer(int damage) {
	char line[256];

	if (_console->_godMode != kGodModeNone)
		damage = 0;

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
