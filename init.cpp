#include"stdafx.h"
#include"scumm.h"

Scumm::Scumm(void)
{
	int i;
        for (i=0;i<17;i++)
        {
                res.address[i]=0;
                res.flags[i]=0;
                res.mode[i]=0;
                res.name[i]=0;
                res.num[i]=0;
                res.roomno[i]=0;
                res.roomoffs[i]=0;
                res.tags[i]=0;
        }

        _resFilePath=0;
        _resFilePrefix=0;

        for (i=1; i<NUM_SCRIPT_SLOT; i++)
        {
                vm.slot[i].status=0;
                vm.slot[i].cutsceneOverride=0;
                vm.slot[i].delay=0;
                vm.slot[i].didexec=0;
                vm.slot[i].freezeCount=0;
                vm.slot[i].newfield=0;
                vm.slot[i].number=0;
                vm.slot[i].offs=0;
                vm.slot[i].unk1=0;
                vm.slot[i].unk2=0;
                vm.slot[i].unk5=0;
                vm.slot[i].where=0;
        }

        for (i=0; i<5;i++)
        {
                vm.cutScenePtr[i]=0;
                vm.cutSceneScript[i]=0;
                vm.cutSceneData[i]=0;
        }

        vm.cutSceneScriptIndex=0;
        vm.cutSceneStackPointer=0;

        _scummStackPos=0;
        _verbMouseOver=0;

        _palDirtyMax=0;
        _palDirtyMin=0;
        _debugger=0;
        camera._cur.x=0;
        camera._cur.y=0;
        camera._dest.x=0;
        camera._dest.y=0;
        camera._accel.x=0;
        camera._accel.y=0;
        camera._last.x=0;
        camera._last.y=0;
        camera._leftTrigger=0;
        camera._rightTrigger=0;
        camera._follows=0;
        camera._mode=0;
        camera._movingToActor=0;

        mouse.x=0;
        mouse.y=0;

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

        memset(_colorCycle, 0, sizeof(_colorCycle));

        _CLUT_offs = 0;
        _ENCD_offs = 0;
        _EPAL_offs = 0;
        _IM00_offs = 0;
        _PALS_offs = 0;

        _fastMode = 0;

        for (i=0; i<6; i++) {
                string[i].t_center = 0;
                string[i].t_xpos = 0;
                string[i].t_ypos = 0;
                string[i].t_color = 0;
                string[i].t_overhead  =0;
                string[i].t_charset = 0;
                string[i].t_right = 0;
        }

        _charsetColor = 0;

        _insaneFlag = 12;

        _insaneState = 0;
}
