#include"stdafx.h"
#include"scumm.h"

Scumm::Scumm(void)
{	
	/* Initilize all our stuff */
	ZeroMemory(&res, sizeof(res));
	ZeroMemory(&vm, sizeof(vm));
	ZeroMemory(&camera, sizeof(camera));
	ZeroMemory(&mouse, sizeof(mouse));
	ZeroMemory(&string, sizeof(string));
	ZeroMemory(&_mixer_channel, sizeof(_mixer_channel));
	ZeroMemory(&charset, sizeof(charset));
	
	//ZeroMemory(_objs, sizeof(_objs));
	ZeroMemory(_colorCycle, sizeof(_colorCycle));        
	ZeroMemory(_mouthSyncTimes, sizeof(_mouthSyncTimes));        

		_resFilePath=0;
        _resFilePrefix=0;
        _scummStackPos=0;
        _verbMouseOver=0;

        _palDirtyMax=0;
        _palDirtyMin=0;
        _debugger=0;

        _xPos = 0;
        _yPos = 0;
        _dir = 0;

        _resultVarNumber = 0;
        delta=0;
        _soundEngine=0;
        _gui=0;

        _verbs=0;
        _objs=0;
        _debugger=0;

        _inventory=0;
        _arrays=0;
        _newNames=0;
        _vars=0;
        _varwatch=0;
        _bitVars=0;

        _talk_sound_mode=0;
        _talk_sound_a = 0;
        _talk_sound_b = 0;

        _curActor = 0;
        _curExecScript = 0;
        _curPalIndex = 0;
        _curVerb = 0;
        _curVerbSlot = 0;
        _currentScript = 0;
        _currentRoom = 0;

        _midi_driver = 0;
        _curSoundPos = 0;
        _soundQuePos = 0;
        _soundQue2Pos = 0;
        _soundParam = 0;
        _soundParam2 = 0;
        _soundParam3 = 0;
		_soundsPaused = 0;
		_soundsPaused2 = 0;
        current_cd_sound = 0;
        num_sound_effects = 0;
        _noSubtitles = 0;

        _screenEffectFlag = 0;
        _switchRoomEffect = 0;
        _switchRoomEffect2 = 0;
        _screenLeft = 0;
        _screenTop = 0;

        _enqueue_b = 0;
        _enqueue_c = 0;
        _enqueue_d = 0;
        _enqueue_e = 0;

        _palManipCounter = 0;
        _palManipStart = 0;
        _palManipEnd = 0;

        

        _CLUT_offs = 0;
        _ENCD_offs = 0;
        _EPAL_offs = 0;
        _IM00_offs = 0;
        _PALS_offs = 0;

        _fastMode = 0;

        
        _charsetColor = 0;

        _insaneFlag = 12;
        _insaneState = 0;


		_haveMsg = 0;
		_talkDelay = 0;
		_defaultTalkDelay = 0;
		_useTalkAnims = 0;
		_endOfMouthSync = 0;
		_mouthSyncMode = 0;
}
