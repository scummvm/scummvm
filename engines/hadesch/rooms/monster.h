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
 * Copyright 2020 Google
 *
 */
namespace Hadesch {

enum {
	// Splits of 15269 as we don't support intermediate anim callbacks
	kCyclopsShootingEyeOpenMidAnim = 1015001,
	kCyclopsShootingEyeClosedMidAnim = 1015002,
	kHitReceived = 1015003
};


enum Monster {
	kCyclops = 1,
	kTyphoon = 2,
	kIllusion = 3
};

struct FlightPosition {
	Common::Point centerPos;
	int scale;
};

class Projectile : Common::NonCopyable {
public:
	Projectile(int id, int level, Monster monster, int startScale, Common::Point startPoint, int xmomentum);
	~Projectile();

	void handleEvent(int ev);
	void stop();
	void makeFlightParams(int xmomentum);
	FlightPosition getFlightPosition(double t);
	void handleAbsoluteClick(Common::SharedPtr <Projectile> backRef, Common::Point p);

	// Event 15051
	bool tick(Common::SharedPtr <Projectile> backRef);

private:
	int getProjectileFlightLength(int level);
	int getProjectileHitChance();

	int _level;
	bool _isMiss;
	int _flightCounterMs;
	int _flightStart;
	int _projectileId;
	int _pending;
	bool _isFlightFinished;
	LayerId _pendingAnim;
	Common::String _flyAnim;
	Common::String _interceptAnim;
	Common::String _hitAnim;
	int _startScale;
	int _flightLengthMs;
	Common::Point _start, _target, _attractor1, _attractor2;
};

class Battleground {
public:
	Battleground();

	int getNumOfProjectiles();
	void launchProjectile(int startScale, Common::Point startPoint, int xmomentum);
	void handleAbsoluteClick(Common::Point p);
	void tick();
	void stopFight();

	int _level;
	int _leavesRemaining;
	Monster _monsterNum;
	bool _isInFight;

private:
	void stopProjectiles();

	Common::Array <Common::SharedPtr<Projectile> > _projectiles;
	int _projectileId;	
};

struct Typhoon {
	Typhoon(Common::SharedPtr<Battleground> battleground);
	void handleEvent(int eventId);
	void enterTyphoon(int level);
	void handleClick(Common::SharedPtr<Typhoon> backRef,
			 const Common::String &name);
	void hideHead(int idx);
	void typhoonA();
	void schedule15154();
	int typhonGetNumAliveHeads();
	void hitTyphoonHead(Common::SharedPtr<Typhoon> backRef, int idx);
	void showHeadNormal(int idx);

	static void stopAnims();
	static void disableHotzones();

	bool _headIsAlive[18];
	bool _playingTyphoonRespawnSound;
	bool _playingTyphoonDieSound;
	bool _isKilled;
	Common::SharedPtr<Battleground> _battleground;
};

class Cyclops {
public:
	Cyclops(Common::SharedPtr<Battleground> battleground);
	void handleEvent(int eventId);
	void handleClick(Common::Point p);
	void enterCyclops(int level);

private:
	bool cyclopsIsHit(Common::Point p, int frame);
	bool cyclopsIsHitBA0(Common::Point p, int frame);
	unsigned getSquareOfPrecision();
	void cyclopsState0();
	void cyclopsState1();
	void cyclopsState2();
	void cyclopsState3();
	void cyclopsState4();
	void cyclopsState5();
	void cyclopsState6();

	Common::SharedPtr<Battleground> _battleground;
	bool _cyclopsIsHiding;
	int _cyclopsProximityCheckCountdown;
	int _currentCyclopsState;
};

class Bird : Common::NonCopyable {
public:
	Bird(int id);

	void launch(int level);

	void stop();

	FlightPosition getFlightPosition(double t);

	void handleAbsoluteClick(Common::Point p);

	// Event 15201
	void tick(Common::SharedPtr <Bird> backRef, Common::SharedPtr<Battleground> battleground);

	void makeFlightParams();

	int _id;
	int _level;
	bool _isActive;
	Common::Point _startPos;
	int _flightLengthMs;
	int _flightStart;
	int _flightCounterMs;
	int _flightShootAnimFrame;
	int _flightShootProjectileFrame;
	int _flightShootEndFrame;
	bool _hasShot;

	Common::Point _targetPos;
	Common::Point _attractor1;
	Common::Point _attractor2;

	int _birdType;
	
	int _field84; // ?
};

class Illusion {
public:
	Illusion(Common::SharedPtr<Battleground> battleground);
	void handleEvent(int eventId);
	void handleAbsoluteClick(Common::Point p);
	void handleClick(const Common::String &name);
	void enterIllusion(int level);
	void tick();
	static void stopAnims();
private:
	void movePhil();
	void launchBird();

	Common::SharedPtr<Bird> _birds[3];
	int _philPosition;
	bool _illusionIsKilled;
	Common::SharedPtr<Battleground> _battleground;
};

}
