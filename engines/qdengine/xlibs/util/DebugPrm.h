#ifndef _DEBUG_PRM_H_
#define _DEBUG_PRM_H_

#include "Serialization\LibraryWrapper.h"
extern bool debugShowEnabled;

struct ShowDebugRigidBody {
	bool boundingBox;
	bool mesh;
	bool radius;
	bool boundRadius;
	bool wayPoints;
	bool downUnit;
	bool realSuspension;
	bool autoPTPoint;
	bool contacts;
	bool localFrame;
	bool velocity;
	bool acceleration;
	float linearScale;
	float angularScale;
	bool rocketTarget;
	bool velocityValue;
	bool ptVelocityValue;
	bool average_movement;
	bool showPathTracking_Lines;
	bool showPathTracking_VehicleRadius;
	bool ground_colliding;
	bool onLowWater;
	bool target;
	bool showDebugMessages;
	int debugMessagesCount;

	ShowDebugRigidBody();
	void serialize(Archive& ar);
};

struct ShowDebugUnitBase {
	bool radius;
	bool libraryKey;
	bool modelName;
	bool abnormalState;
	bool effects;
	bool clan;
	bool visibleUnit;
	bool producedPlacementZone;
	bool lodDistance_;
	bool showGraphicsBound;
	
	ShowDebugUnitBase();
	void serialize(Archive& ar);
};

struct ShowDebugUnitInterface {
	bool debugString;
	ShowDebugUnitInterface();
	void serialize(Archive& ar);
};

struct ShowDebugUnitReal {
	bool target;
	bool fireResponse;
	bool toolzer;
	bool positionValue;
	bool unitID;
	bool producedUnitQueue;
	bool transportSlots;
	bool docking;
	bool chain;
	bool parameters;
	bool parametersMax;
	bool parametersParts;
	bool waterDamage;
	bool iceDamage;
	bool lavaDamage;
	bool earthDamage;
	bool attackModes;
	bool currentChain;
	bool sightSector;
	bool noiseRadius;
	bool noiseTarget;
	bool hearingRadius;
	bool directControlWeapon;
	string modelLogicNode;
	string modelNode;
	
	ShowDebugUnitReal();
	void serialize(Archive& ar);
};

struct ShowDebugLegionary {
	bool invisibility;
	bool level;
	bool transport;
	bool resourcerMode;
	bool resourcerProgress;
	bool resourcerCapacity;
	bool trace;
	bool aimed;
	bool usedByTrigger;
	
	ShowDebugLegionary();
	void serialize(Archive& ar);
};

struct ShowDebugBuilding {
	bool status;
	bool basement;
	bool connectionNode;
	bool usedByTrigger;
	
	ShowDebugBuilding();
	void serialize(Archive& ar);
};


struct ShowDebugSquad {
	bool position;
	bool fire_radius;
	bool sight_radius;
	bool described_radius;
	bool attackAction;
	bool squadToFollow;
	bool unitsNumber;
	bool usedByTrigger;
	
	ShowDebugSquad();
	void serialize(Archive& ar);
};

struct ShowDebugUnitEnvironment {
	bool rigidBody;
	bool modelName;
	bool environmentType;
	bool treeType;
	bool treeMode;

	ShowDebugUnitEnvironment();
	void serialize(Archive& ar);
};

struct ShowDebugPlayer {
	bool placeOp;
	bool scanBound;
	bool resource;
	bool resourceCapacity;
	bool unitNumber;
	bool unitNumberReserved;
	bool showSelectedOnly;
	bool showStatistic;
	bool saveLogStatistic;
	bool showSearchRegion;
	
	ShowDebugPlayer();
	void serialize(Archive& ar);
};

struct ShowDebugSource {
	bool enable;
	bool state;
	bool name;
	bool label;
	bool radius;
	bool axis;
	bool type;
	bool zoneDamage;
	bool zoneStateDamage;
	bool dontShowInfo;
	bool showEnvironmentPoints;
	mutable int sourceCount;
	
	ShowDebugSource();
	void serialize(Archive& ar);
};

struct ShowDebugWeapon {
	bool enable;
	bool showSelectedOnly;
	bool direction;
	bool horizontalAngle;
	bool verticalAngle;
	bool gripVelocity;
	bool targetingPosition;
	bool ownerTarget;
	bool ownerSightRadius;
	bool autoTarget;
	bool angleValues;
	bool angleLimits;
	bool parameters;
	bool damage;
	bool load;
	bool fireRadius;
	bool showChainLightning;
	bool showLightningUnitChainRadius;

	int showWeaponID;
	int showWeaponSlotID;
	
	ShowDebugWeapon();
	void serialize(Archive& ar);
};

struct ShowDebugInterface {
	bool showDebug;
	int writeLog;
	bool disableTextures;
	bool background;
	bool bgeffects;
	bool screens;
	bool controlBorder;
	bool hoveredControlBorder;
	bool hoveredControlExtInfo;
	bool focusedControlBorder;
	bool marks;
	bool hoverUnitBound;
	bool enableAllNetControls;
	bool showUpMarksCount;
	bool showAimPosition;
	bool showSelectManager;
	bool cursorReason;
	bool logicDispatcher;
	bool showTransformInfo;
	bool showDebugJoystick;
	bool showDebugSpriteScale;
	bool showInventoryItemInfo;

	bool needShow() const { return debugShowEnabled && showDebug; }

	ShowDebugInterface();
	void serialize(Archive& ar);
};

struct ShowDebugEffects {
	bool showName;
	bool axis;

	ShowDebugEffects();
	void serialize(Archive& ar);
};

struct ShowDebugTerrain {
	bool showBuildingInfo;
	//int showTerrainSpecialInfo;
	bool showSurKind;

	ShowDebugTerrain();
	void serialize(Archive& ar);
};

struct PanoScreenshotSetup
{
	float delta_psi;
	float delta_theta;

	float add_theta;

	int count_psi;
	int count_theta;

	float aeroZ;
	float aeroDX;
	float aeroDY;

	PanoScreenshotSetup();
	void serialize(Archive& ar);
};

extern bool debugWireFrame;
extern ShowDebugRigidBody showDebugRigidBody;
extern ShowDebugUnitBase showDebugUnitBase;
extern ShowDebugUnitInterface showDebugUnitInterface;
extern ShowDebugUnitReal showDebugUnitReal;
extern ShowDebugLegionary showDebugLegionary;
extern ShowDebugBuilding showDebugBuilding;
extern ShowDebugSquad showDebugSquad;
extern ShowDebugUnitEnvironment showDebugUnitEnvironment;
extern bool show_environment_type;
extern ShowDebugPlayer showDebugPlayer;
extern ShowDebugSource showDebugSource;
extern ShowDebugWeapon showDebugWeapon;
extern ShowDebugInterface showDebugInterface;
extern ShowDebugEffects showDebugEffects;
extern ShowDebugTerrain showDebugTerrain;
extern PanoScreenshotSetup panoScreenshotSetup;
extern int show_filth_debug;
extern float show_vector_zmin;
extern float show_vector_zmax;
extern int mt_interface_quant;
extern int ht_intf_test;
extern bool disableHideUnderscore;
extern int terMaxTimeInterval;
extern int logicTimePeriod;
extern float logicTimePeriodInv;
extern float logicPeriodSeconds;
extern int debug_show_briefing_log;
extern int debug_show_mouse_position;
extern int debug_show_energy_consumption;
extern int debug_allow_mainmenu_gamespy;
extern int debug_allow_replay;
extern bool show_pathfinder_map;
extern bool show_pathtracking_map_for_selected;
extern bool show_normal_map;
extern bool show_wind_map;
extern bool debug_stop_time;
extern bool showDebugWaterHeight;
extern bool showDebugAnchors;
extern bool cameraGroundColliding;
extern bool showDebugCrashSystem;
extern bool showDebugNumSounds;
extern bool debugDisableFogOfWar;
extern bool debugDisableSpecialExitProcess;

struct DebugPrm : public LibraryWrapper<DebugPrm>
{
	bool enableKeyHandlers;
	bool forceDebugKeys;
	float debugDamage;
	float debugDamageArmor;
	bool debugDisableDamage;
	bool debugClickKillMode;
	bool showNetStat;
	bool showFieldOfViewMap;
	bool showCurrentAI;
	bool debugLoadTime;
	int debugFontSize;
	
	DebugPrm();
	void serialize(Archive& ar);
};

#endif //_DEBUG_PRM_H_
