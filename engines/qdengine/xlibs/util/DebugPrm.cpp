#include "stdafx.h"
#include "DebugPrm.h"
#include "Serialization\XPrmArchive.h"
#include "Serialization\RangedWrapper.h"
#include "Terra\vmap.h"

WRAP_LIBRARY(DebugPrm, "DebugPrm", "DebugPrm", "Scripts\\TreeControlSetups\\Debug.dat", 0, 0);

bool debugShowEnabled;
bool debugWireFrame;
ShowDebugRigidBody showDebugRigidBody;
ShowDebugUnitBase showDebugUnitBase;
ShowDebugUnitReal showDebugUnitReal;
ShowDebugUnitInterface showDebugUnitInterface;
ShowDebugLegionary showDebugLegionary;
ShowDebugBuilding showDebugBuilding;
ShowDebugSquad showDebugSquad;
ShowDebugUnitEnvironment showDebugUnitEnvironment;
bool show_environment_type;
ShowDebugPlayer showDebugPlayer;
ShowDebugSource showDebugSource;
ShowDebugWeapon showDebugWeapon;
ShowDebugInterface showDebugInterface;
ShowDebugEffects showDebugEffects;
ShowDebugTerrain showDebugTerrain;
PanoScreenshotSetup panoScreenshotSetup;

bool showDebugAnchors;
bool showDebugWaterHeight;
int show_filth_debug;
float show_vector_zmin;
float show_vector_zmax;
int mt_interface_quant;
int ht_intf_test;
bool disableHideUnderscore;
int terMaxTimeInterval;
int logicTimePeriod = 100;
float logicTimePeriodInv = 1.0f/logicTimePeriod;
float logicPeriodSeconds = logicTimePeriod / 1000.f;
int debug_show_briefing_log;
int debug_show_mouse_position;
int debug_show_energy_consumption;
int debug_allow_mainmenu_gamespy;
int debug_allow_replay;
bool show_pathfinder_map;
bool show_pathtracking_map_for_selected;
bool show_normal_map;
bool show_wind_map;
bool debug_stop_time;
bool cameraGroundColliding;
bool showDebugCrashSystem;
bool showDebugNumSounds;
bool debugDisableFogOfWar;
bool debugDisableSpecialExitProcess;

ShowDebugRigidBody::ShowDebugRigidBody()
{
	boundingBox = 0;
	mesh = 0;
	radius = 0;
	boundRadius = 0;
	realSuspension = false;
	wayPoints = 0;
	downUnit = 0;
	autoPTPoint = 0;
	contacts = 0;
	localFrame = 0;
	acceleration = 0;
	linearScale = 1;
	angularScale = 200;
	rocketTarget = 0;
	velocityValue = 0;
	ptVelocityValue = false;
	average_movement = 0;
	showPathTracking_Lines = false;
	showPathTracking_VehicleRadius = false;
	ground_colliding = false;
	onLowWater = false;
	target = false;
	showDebugMessages = false;
	debugMessagesCount = 5;
	showDebugNumSounds = false;

}

void ShowDebugRigidBody::serialize(Archive& ar)
{
	ar.serialize(boundingBox, "boundingBox", 0);
	ar.serialize(mesh, "mesh", 0);
	ar.serialize(radius, "radius", 0);
	ar.serialize(boundRadius, "boundRadius", 0);
	ar.serialize(wayPoints, "wayPoints", 0);
	ar.serialize(downUnit, "downUnit", 0);
	ar.serialize(realSuspension, "realSuspension", 0);
	ar.serialize(autoPTPoint, "autoPTPoint", 0);
	ar.serialize(contacts, "contacts", 0);
	ar.serialize(localFrame, "localFrame", 0);
	ar.serialize(velocity, "velocity", 0);
	ar.serialize(acceleration, "acceleration", 0);
	ar.serialize(linearScale, "linearScale", 0);
	ar.serialize(angularScale, "angularScale", 0);
	ar.serialize(rocketTarget, "rocketTarget", 0);
	ar.serialize(velocityValue, "velocityValue", 0);
	ar.serialize(ptVelocityValue, "ptVelocityValue", 0);
	ar.serialize(average_movement, "average_movement", 0);
	ar.serialize(showPathTracking_Lines, "showPathTracking_Lines", 0);
	ar.serialize(showPathTracking_VehicleRadius, "showPathTracking_VehicleRadius", 0);
	ar.serialize(ground_colliding, "ground_colliding", 0);
	ar.serialize(onLowWater, "onLowWater", 0);
	ar.serialize(target, "target", 0);
	ar.serialize(showDebugMessages, "showDebugMessages", 0);
	ar.serialize(debugMessagesCount, "debugMessagesCount", 0);
}

ShowDebugUnitBase::ShowDebugUnitBase()
{
	radius = 0;
	libraryKey = 0;
	modelName = 0;
	abnormalState = 0;
	effects = 0;
	clan = 0;
	producedPlacementZone = 0;
	showGraphicsBound = 0;
	lodDistance_ = 0;
	visibleUnit = false;
}

void ShowDebugUnitBase::serialize(Archive& ar)
{
	ar.serialize(radius, "radius", 0);
	ar.serialize(libraryKey, "libraryKey", 0);
	ar.serialize(modelName, "modelName", 0);
	ar.serialize(abnormalState, "abnormalState", 0);
	ar.serialize(effects, "effects", 0);
	ar.serialize(clan, "clan", 0);
	ar.serialize(producedPlacementZone, "producedPlacementZone", 0);
	ar.serialize(lodDistance_,"lodDistance",0);
	ar.serialize(showGraphicsBound, "showGraphicsBound", "showGraphicsBound");
	ar.serialize(visibleUnit, "visibleUnit", "видимость на экране");
}

ShowDebugUnitInterface::ShowDebugUnitInterface()
{
	debugString = 0;
}

void ShowDebugUnitInterface::serialize(Archive& ar)
{
	ar.serialize(debugString, "debugString", 0);
}

ShowDebugUnitReal::ShowDebugUnitReal()
{
	target = 0;
	fireResponse = 0;
	toolzer = 0;
	positionValue = 0;
	unitID = 0;
	producedUnitQueue = 0;
	transportSlots = 0;
	docking = 0;
	chain = 0;
	parameters = 0;
	parametersMax = 0;
	parametersParts = 0;
	waterDamage = 0;
	lavaDamage = 0;
	iceDamage = 0;
	earthDamage = 0;
	attackModes = 0;
	currentChain = 0;
	sightSector = false;
	noiseRadius = false;
	noiseTarget = false;
	hearingRadius = false;
	directControlWeapon = false;
}

void ShowDebugUnitReal::serialize(Archive& ar)
{
	ar.serialize(target, "target", 0);
	ar.serialize(fireResponse, "fireResponse", 0);
	ar.serialize(toolzer, "toolzer", 0);
	ar.serialize(positionValue, "positionValue", 0);
	ar.serialize(unitID, "unitID", 0);
	ar.serialize(producedUnitQueue, "producedUnitQueue", 0);
	ar.serialize(transportSlots, "transportSlots", 0);
	ar.serialize(docking, "docking", 0);
	ar.serialize(chain, "chain", 0);
	ar.serialize(parameters, "parameters", 0);
	ar.serialize(parametersMax, "parametersMax", 0);
	ar.serialize(parametersParts, "parametersParts", 0);
	ar.serialize(waterDamage, "waterDamage", 0);
	ar.serialize(iceDamage, "iceDamage", 0);
	ar.serialize(lavaDamage, "lavaDamage", 0);
	ar.serialize(earthDamage, "earthDamage", 0);
	ar.serialize(attackModes, "attackModes", 0);
	ar.serialize(currentChain, "currentChain", 0);
	ar.serialize(sightSector, "sightSector", "Сектор обзора");
	ar.serialize(noiseRadius, "noiseRadius", "Радиус создаваемого шума");
	ar.serialize(noiseTarget, "noiseTarget", "Услышанный шум");
	ar.serialize(hearingRadius, "hearingRadius", "Радиус слышимости");
	ar.serialize(directControlWeapon, "directControlWeapon", "Выбранное оружие в прямом управлении");
	ar.serialize(modelLogicNode, "modelLogicNode", "Логический узел");
	ar.serialize(modelNode, "modelNode", "Графический узел");
}


ShowDebugLegionary::ShowDebugLegionary()
{
	invisibility = 0;
	level = false;
	transport = false;
	resourcerMode = 0;
	resourcerProgress = 0;
	resourcerCapacity = 0;
	trace = 0;
	aimed = 0;
	usedByTrigger = 0;
}

void ShowDebugLegionary::serialize(Archive& ar)
{
	ar.serialize(invisibility, "invisibility", 0);
	ar.serialize(level, "level", 0);
	ar.serialize(transport, "transport", 0);
	ar.serialize(resourcerMode, "resourcerMode", 0);
	ar.serialize(resourcerProgress, "resourcerProgress", 0);
	ar.serialize(resourcerCapacity, "resourcerCapacity", 0);
	ar.serialize(trace, "trace", 0);
	ar.serialize(aimed, "aimed", 0);
	ar.serialize(usedByTrigger, "usedByTrigger", "usedByTrigger");
}

ShowDebugBuilding::ShowDebugBuilding()
{
	status = false;
	basement = false;
	connectionNode = false;
	usedByTrigger = false;
}

void ShowDebugBuilding::serialize(Archive& ar)
{
	ar.serialize(status, "status", 0);
	ar.serialize(basement, "basement", 0);
	ar.serialize(connectionNode, "connectionNode", 0);
	ar.serialize(usedByTrigger, "usedByTrigger", "usedByTrigger");
}

ShowDebugSquad::ShowDebugSquad()
{
	position = 0;
	fire_radius = 0;
	sight_radius = 0;
	described_radius = 0;
	attackAction = 0;
	squadToFollow = 0;
	unitsNumber = 0;
	usedByTrigger = 0;
}

void ShowDebugSquad::serialize(Archive& ar)
{
	ar.serialize(position, "position", 0);
	ar.serialize(fire_radius, "fire_radius", 0);
	ar.serialize(sight_radius, "sight_radius", 0);
	ar.serialize(described_radius, "described_radius", 0);
	ar.serialize(attackAction, "attackAction", 0);
	ar.serialize(squadToFollow, "squadToFollow", 0);
	ar.serialize(unitsNumber, "unitsNumber", 0);
	ar.serialize(usedByTrigger, "usedByTrigger", "usedByTrigger");
}

ShowDebugUnitEnvironment::ShowDebugUnitEnvironment()
{
	rigidBody = 0;
	modelName = 0;
	environmentType = 0;
	treeType = 0;
	treeMode = 0;
}

void ShowDebugUnitEnvironment::serialize(Archive& ar)
{
	ar.serialize(rigidBody, "rigidBody", 0);
	ar.serialize(modelName, "modelName", 0);
	ar.serialize(environmentType, "environmentType", 0);
	ar.serialize(treeType, "treeType", 0);
	ar.serialize(treeMode, "treeMode", 0);
}

ShowDebugPlayer::ShowDebugPlayer()
{
	placeOp = 0;
	scanBound = 0;
	resource = 0;
	resourceCapacity = 0;
	unitNumber = 0;
	unitNumberReserved = 0;
	showSelectedOnly = false;
	showStatistic = false;
	saveLogStatistic = false;
	showSearchRegion = false;
}

void ShowDebugPlayer::serialize(Archive& ar)
{
	ar.serialize(placeOp, "placeOp", 0);
	ar.serialize(scanBound, "scanBound", 0);
	ar.serialize(resource, "resource", 0);
	ar.serialize(resourceCapacity, "resourceCapacity", "resourceCapacity");
	ar.serialize(unitNumber, "unitNumber", "unitNumber");
	ar.serialize(unitNumberReserved, "unitNumberReserved", "unitNumberReserved");
	ar.serialize(showSelectedOnly, "showSelectedOnly", 0);
	ar.serialize(showStatistic, "showStatistic", 0);
	ar.serialize(saveLogStatistic, "saveLogStatistic", 0);
	ar.serialize(showSearchRegion, "showSearchRegion", 0);
}

ShowDebugSource::ShowDebugSource()
{
	enable = 0;
	state = false;
	name = 0;
	label = 0;
	radius = 0;
	axis = false;
	type = 0;
	zoneDamage = 0;
	zoneStateDamage = 0;
	dontShowInfo = false;
	showEnvironmentPoints = false;
	sourceCount = 0;
}

void ShowDebugSource::serialize(Archive& ar)
{
	ar.serialize(enable, "enable", "Включить");
	ar.serialize(name, "name", 0);
	ar.serialize(state, "state", "Состояние");
	ar.serialize(label, "label", 0);
	ar.serialize(radius, "radius", 0);
	ar.serialize(axis, "axis", 0);
	ar.serialize(type, "type", 0);
	ar.serialize(zoneDamage, "zoneDamage", 0);
	ar.serialize(zoneStateDamage, "zoneStateDamage", 0);
	ar.serialize(dontShowInfo, "dontShowInfo", 0);
	ar.serialize(showEnvironmentPoints, "showEnvironmentPoints", 0);
}				   

ShowDebugWeapon::ShowDebugWeapon()
{
	enable = 0;
	showSelectedOnly = false;
	direction = 0;
	horizontalAngle = 0;
	verticalAngle = 0;
	gripVelocity = 0;
	targetingPosition = false;
	ownerTarget = false;
	ownerSightRadius = false;
	autoTarget = false;
	angleValues = false;
	angleLimits = false;
	parameters = false;
	damage = false;
	load = false;
	fireRadius = false;
	showWeaponID = 0;
	showChainLightning = false;
	showLightningUnitChainRadius = false;
	showWeaponSlotID = -1;
}

void ShowDebugWeapon::serialize(Archive& ar)
{
	ar.serialize(enable, "enable", 0);
	ar.serialize(showSelectedOnly, "showSelectedOnly", 0);
	ar.serialize(direction, "direction", 0);
	ar.serialize(horizontalAngle, "horizontalAngle", 0);
	ar.serialize(verticalAngle, "verticalAngle", 0);
	ar.serialize(gripVelocity, "gripVelocity", 0);
	ar.serialize(targetingPosition, "targetingPosition", 0);
	ar.serialize(ownerTarget, "ownerTarget", 0);
	ar.serialize(ownerSightRadius, "ownerSightRadius", 0);
	ar.serialize(autoTarget, "autoTarget", 0);
	ar.serialize(angleValues, "angleValues", 0);
	ar.serialize(angleLimits, "angleLimits", 0);
	ar.serialize(parameters, "parameters", 0);
	ar.serialize(damage, "damage", 0);
	ar.serialize(load, "load", 0);
	ar.serialize(fireRadius, "fireRadius", "Дальность стрельбы");
	ar.serialize(showWeaponID, "showWeaponID", 0);
	ar.serialize(showWeaponSlotID, "showWeaponSlotID", 0);
	ar.serialize(showChainLightning, "showLightning", "Цепной эффект");
	ar.serialize(showLightningUnitChainRadius, "showLightningUnitChainRadius", "Радиус цепного эффекта вторичных эммитеров");
}				   

ShowDebugInterface::ShowDebugInterface()
{
	showDebug = false;
	writeLog = 0;
	disableTextures = false;
	background = false;
	bgeffects = false;
	screens = false;
	controlBorder = false;
	hoveredControlBorder = false;
	hoveredControlExtInfo = false;
	focusedControlBorder = false;
	marks = false;
	hoverUnitBound = false;
	enableAllNetControls = false;
	showUpMarksCount = false;
	showSelectManager = false;
	cursorReason = false;
	showTransformInfo = false;
	logicDispatcher = false;
	showDebugJoystick = false;
	showDebugSpriteScale = false;
	showInventoryItemInfo = false;
}

void ShowDebugInterface::serialize(Archive& ar)
{
	ar.serialize(showDebug, "showDebug", "Показывать интерфейсную информацию");
	ar.serialize(writeLog, "writeLogMode", "Писать в лог");
	ar.serialize(disableTextures, "disableTextures", "Не выводить текстуры");
	ar.serialize(background, "background", "Информация о фоновых 3D моделях");
	ar.serialize(bgeffects, "bgeffects", "Фоновые эффекты");
	ar.serialize(screens, "screens", "Информация о текущих экранах");
	ar.serialize(controlBorder, "controlBorder", "Показывать границы контролов");
	ar.serialize(hoveredControlBorder, "hoveredControlBorder", "Показывать границу и имя контрола под мышкой");
	ar.serialize(hoveredControlExtInfo, "hoveredControlExtInfo", "Показывать расширенную информацию по контролу под мышкой");
	ar.serialize(showTransformInfo, "showTransformInfo", "Информация о трансформации кнопки");
	ar.serialize(showInventoryItemInfo, "showInventoryItemInfo", "Информация об элементе инвентаря");
	ar.serialize(focusedControlBorder, "focusedControlBorder", "Показывать границу контрола с фокусом ввода");
	ar.serialize(enableAllNetControls, "enableAllNetControls", "Разрешить менять все настройки миссии");
	ar.serialize(showSelectManager, "selectManager", "Информация о селекте");
	ar.serialize(marks, "marks", "Пометки");
	ar.serialize(hoverUnitBound, "hoverUnitBound", "Показать баунд юнита под мышкой");
	ar.serialize(showUpMarksCount, "showUpMarksCount", "Количество обработчиков взлетающих значений");
	ar.serialize(showAimPosition, "showAimPosition", "Точка прицеливания");
	ar.serialize(cursorReason, "cursorReason", "Причина выбора курсора");
	ar.serialize(logicDispatcher, "logicDispatcher", "Временная инфорация интерфейса");
	ar.serialize(showDebugJoystick, "showDebugJoystick", "Состояние джойстика");
	ar.serialize(showDebugSpriteScale, "showDebugSpriteScale", "Маштаб спрайтиков");
}

ShowDebugEffects::ShowDebugEffects()
{
	showName = false;
	axis = false;
}

void ShowDebugEffects::serialize(Archive& ar)
{
	ar.serialize(showName, "showName", "Показывать библиотечное имя");
	ar.serialize(axis, "axis", "Рисовать оси");
}

ShowDebugTerrain::ShowDebugTerrain()
{
	showBuildingInfo=false;
	//showTerrainSpecialInfo=vrtMap::SSI_NoShow;
	showSurKind=false;
}

void ShowDebugTerrain::serialize(Archive& ar)
{
	bool oldShowBuildingInfo=showBuildingInfo;
	ar.serialize(showBuildingInfo, "showBuildingInfo", 0);
	//int oldShowTerrainSpecialInfo=showTerrainSpecialInfo;
	//vrtMap::eShowSpecialInfo ssi, oldssi;
	//oldssi=ssi=static_cast<vrtMap::eShowSpecialInfo>(showTerrainSpecialInfo);
	//ar.serialize(ssi, "showTerrainInfo", 0);
	//showTerrainSpecialInfo=ssi;
	bool oldShowSurKind=showSurKind;
	ar.serialize(showSurKind, "showSurKind",0);
	if(oldShowBuildingInfo!=showBuildingInfo || oldShowSurKind!=showSurKind ){ //||oldssi!=ssi
		vMap.toShowDbgInfo(showBuildingInfo);
		//vMap.toShowSpecialInfo(ssi);
		vMap.toShowSurKind(showSurKind);
		vMap.WorldRender();
	}

}

PanoScreenshotSetup::PanoScreenshotSetup()
{
	count_psi = 12;
	count_theta = 3;

	delta_psi = 360.f / float(count_psi);
	delta_theta = 30.f;

	add_theta = 0.f;

	aeroZ = 300.f;
	aeroDX = 300.f;
	aeroDY = 300.f;
}

void PanoScreenshotSetup::serialize(Archive& ar)
{
	ar.serialize(count_psi, "count_psi", "шагов по горизонтали");
	ar.serialize(count_theta, "count_theta", "шагов по вертикали");

	ar.serialize(delta_psi, "delta_psi", "шаг угла по горизонтали");
	ar.serialize(delta_theta, "delta_theta", "шаг угла по вертикали");

	ar.serialize(add_theta, "add_theta", "смещение по вертикали");

	ar.serialize(aeroZ, "aeroZ", "высота камеры для аэропанорам");
	ar.serialize(aeroDX, "aeroDX", "шаг по x для аэропанорам");
	ar.serialize(aeroDY, "aeroDY", "шаг по y для аэропанорам");
}

DebugPrm::DebugPrm()
{	
	debugFontSize = 16;
	debugLoadTime = false;
	debugDamage = 99.5f;
	debugDamageArmor = 500.f;
	debugDisableDamage = false;
	debugClickKillMode = false;
	showNetStat = false;
	showFieldOfViewMap = false;
	showCurrentAI = true;
	enableKeyHandlers = 0;
	forceDebugKeys = 0;

	debugWireFrame = 0;
	debugShowEnabled = 0;
	show_environment_type = 0;
	show_filth_debug = 0;
	show_vector_zmin = 200;
	show_vector_zmax = 4000;
	mt_interface_quant = 1;
	ht_intf_test = 0;
	disableHideUnderscore = true;
	terMaxTimeInterval = 100;
	debug_show_briefing_log = 0;
	debug_show_mouse_position = 0;
	debug_show_energy_consumption = 0;
	debug_allow_mainmenu_gamespy = 0;
	debug_allow_replay = 0;
	show_pathfinder_map = false;
	show_pathtracking_map_for_selected = false;
	show_normal_map = false;
	show_wind_map = false;
	debug_stop_time = false;
	showDebugWaterHeight = false;
	showDebugAnchors = false;
	cameraGroundColliding = true;
	showDebugCrashSystem = false;
	debugDisableFogOfWar = false;
	debugDisableSpecialExitProcess = false;
}

void DebugPrm::serialize(Archive& ar)
{
#ifndef _FINAL_VERSION_
	ar.serialize(debugShowEnabled, "debugShowEnabled", 0);
	ar.serialize(enableKeyHandlers, "enableKeyHandlers", 0);
	ar.serialize(forceDebugKeys, "forceDebugKeys", 0);
	ar.serialize(debugDisableFogOfWar, "disableFogOfWar", 0);
#endif
	
	ar.serialize(showDebugRigidBody, "showDebugRigidBody", 0);
	ar.serialize(showDebugUnitBase, "showDebugUnitBase", 0);
	ar.serialize(showDebugUnitInterface, "showDebugUnitInterface", 0);
	ar.serialize(showDebugUnitReal, "showDebugUnitReal", 0);
	ar.serialize(showDebugLegionary, "showDebugLegionary", 0);
	ar.serialize(showDebugBuilding, "showDebugBuilding", 0);
	ar.serialize(showDebugSquad, "showDebugSquad", 0);
	ar.serialize(showDebugUnitEnvironment, "showDebugUnitEnvironment", 0);
	ar.serialize(showDebugPlayer, "showDebugPlayer", 0);
	ar.serialize(showDebugSource, "showDebugSource", 0);
	ar.serialize(showDebugWeapon, "showDebugWeapon", 0);
	ar.serialize(showDebugInterface, "showDebugInterface", 0);
	ar.serialize(showDebugEffects, "showDebugEffects", 0);
	ar.serialize(showDebugTerrain, "showDebugTerrain", 0);

	ar.serialize(panoScreenshotSetup, "panoScreenshotSetup", "Настройки съемки панорам");
	
	ar.serialize(debugWireFrame, "debugWireFrame", 0);
	ar.serialize(showCurrentAI, "showCurrentAI", 0);
	ar.serialize(show_environment_type, "show_environment_type", 0);
	ar.serialize(show_filth_debug, "show_filth_debug", 0);
	ar.serialize(show_vector_zmin, "show_vector_zmin", 0);
	ar.serialize(show_vector_zmax, "show_vector_zmax", 0);
	ar.serialize(mt_interface_quant, "mt_interface_quant", 0);
	ar.serialize(ht_intf_test, "ht_intf_test", 0);
	ar.serialize(disableHideUnderscore, "disableHideUnderscore", 0);
	ar.serialize(debug_show_briefing_log, "debug_show_briefing_log", 0);
	ar.serialize(debug_show_mouse_position, "debug_show_mouse_position", 0);
	ar.serialize(debug_show_energy_consumption, "debug_show_energy_consumption", 0);
	ar.serialize(debug_allow_mainmenu_gamespy, "debug_allow_mainmenu_gamespy", 0);
	ar.serialize(debug_allow_replay, "debug_allow_replay", 0);
	ar.serialize(show_pathfinder_map, "show_pathfinder_map", 0);
	ar.serialize(show_pathtracking_map_for_selected, "show_pathtracking_map_for_selected", 0);
	ar.serialize(show_normal_map, "show_normal_map", 0);
	ar.serialize(show_wind_map, "show_wind_map", 0);
	ar.serialize(showDebugWaterHeight, "showDebugWaterHeight", 0);
	ar.serialize(showDebugCrashSystem, "showDebugCrashSystem", 0);
	ar.serialize(showDebugAnchors, "showDebugAnchors", 0);
	ar.serialize(debug_stop_time, "debug_stop_time", 0);
	ar.serialize(cameraGroundColliding, "cameraGroundColliding", 0);

	ar.serialize(terMaxTimeInterval, "terMaxTimeInterval", 0);
	ar.serialize(logicTimePeriod, "logicTimePeriod", 0);
	
	ar.serialize(debugDamage, "debugDamage", 0);
	ar.serialize(debugDamageArmor, "debugDamageArmor", 0);
	ar.serialize(debugDisableDamage, "debugDisableDamage", "запретить повреждения");
	ar.serialize(debugClickKillMode, "debugClickKillMode", "убивать юнитов по клику");
	ar.serialize(showNetStat, "showNetStat", 0);
	ar.serialize(showFieldOfViewMap, "showFieldOfViewMap", "showFieldOfViewMap");
	ar.serialize(showDebugNumSounds,"showDebugNumSounds",0);
	ar.serialize(debugLoadTime, "debugLoadTime", 0);
	ar.serialize(debugFontSize, "debugFontSize", "Размер шрифта по умолчанию");

	ar.serialize(debugDisableSpecialExitProcess, "disableTriggeredExit", 0);
		
	logicTimePeriodInv = 1.0f/logicTimePeriod;
	logicPeriodSeconds = logicTimePeriod / 1000.f;
}


