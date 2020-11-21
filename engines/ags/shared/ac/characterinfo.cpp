//=============================================================================
//
// Adventure Game Studio (AGS)
//
// Copyright (C) 1999-2011 Chris Jones and 2011-20xx others
// The full list of copyright holders can be found in the Copyright.txt
// file, which is part of this source code distribution.
//
// The AGS source code is provided under the Artistic License 2.0.
// A copy of this license can be found in the file License.txt and at
// http://www.opensource.org/licenses/artistic-license-2.0.php
//
//=============================================================================

#include <string.h>
#include "ac/characterinfo.h"
#include "util/stream.h"

using AGS::Common::Stream;


void CharacterInfo::ReadFromFile(Stream *in)
{
    defview = in->ReadInt32();
    talkview = in->ReadInt32();
    view = in->ReadInt32();
    room = in->ReadInt32();
    prevroom = in->ReadInt32();
    x = in->ReadInt32();
    y = in->ReadInt32();
    wait = in->ReadInt32();
    flags = in->ReadInt32();
    following = in->ReadInt16();
    followinfo = in->ReadInt16();
    idleview = in->ReadInt32();
    idletime = in->ReadInt16();
    idleleft = in->ReadInt16();
    transparency = in->ReadInt16();
    baseline = in->ReadInt16();
    activeinv = in->ReadInt32();
    talkcolor = in->ReadInt32();
    thinkview = in->ReadInt32();
    blinkview = in->ReadInt16();
    blinkinterval = in->ReadInt16();
    blinktimer = in->ReadInt16();
    blinkframe = in->ReadInt16();
    walkspeed_y = in->ReadInt16();
    pic_yoffs = in->ReadInt16();
    z = in->ReadInt32();
    walkwait = in->ReadInt32();
    speech_anim_speed = in->ReadInt16();
    reserved1 = in->ReadInt16();
    blocking_width = in->ReadInt16();
    blocking_height = in->ReadInt16();;
    index_id = in->ReadInt32();
    pic_xoffs = in->ReadInt16();
    walkwaitcounter = in->ReadInt16();
    loop = in->ReadInt16();
    frame = in->ReadInt16();
    walking = in->ReadInt16();
    animating = in->ReadInt16();
    walkspeed = in->ReadInt16();
    animspeed = in->ReadInt16();
    in->ReadArrayOfInt16(inv, MAX_INV);
    actx = in->ReadInt16();
    acty = in->ReadInt16();
    in->Read(name, 40);
    in->Read(scrname, MAX_SCRIPT_NAME_LEN);
    on = in->ReadInt8();
}

void CharacterInfo::WriteToFile(Stream *out)
{
    out->WriteInt32(defview);
    out->WriteInt32(talkview);
    out->WriteInt32(view);
    out->WriteInt32(room);
    out->WriteInt32(prevroom);
    out->WriteInt32(x);
    out->WriteInt32(y);
    out->WriteInt32(wait);
    out->WriteInt32(flags);
    out->WriteInt16(following);
    out->WriteInt16(followinfo);
    out->WriteInt32(idleview);
    out->WriteInt16(idletime);
    out->WriteInt16(idleleft);
    out->WriteInt16(transparency);
    out->WriteInt16(baseline);
    out->WriteInt32(activeinv);
    out->WriteInt32(talkcolor);
    out->WriteInt32(thinkview);
    out->WriteInt16(blinkview);
    out->WriteInt16(blinkinterval);
    out->WriteInt16(blinktimer);
    out->WriteInt16(blinkframe);
    out->WriteInt16(walkspeed_y);
    out->WriteInt16(pic_yoffs);
    out->WriteInt32(z);
    out->WriteInt32(walkwait);
    out->WriteInt16(speech_anim_speed);
    out->WriteInt16(reserved1);
    out->WriteInt16(blocking_width);
    out->WriteInt16(blocking_height);;
    out->WriteInt32(index_id);
    out->WriteInt16(pic_xoffs);
    out->WriteInt16(walkwaitcounter);
    out->WriteInt16(loop);
    out->WriteInt16(frame);
    out->WriteInt16(walking);
    out->WriteInt16(animating);
    out->WriteInt16(walkspeed);
    out->WriteInt16(animspeed);
    out->WriteArrayOfInt16(inv, MAX_INV);
    out->WriteInt16(actx);
    out->WriteInt16(acty);
    out->Write(name, 40);
    out->Write(scrname, MAX_SCRIPT_NAME_LEN);
    out->WriteInt8(on);
}

void ConvertOldCharacterToNew (OldCharacterInfo *oci, CharacterInfo *ci) {
    COPY_CHAR_VAR (defview);
    COPY_CHAR_VAR (talkview);
    COPY_CHAR_VAR (view);
    COPY_CHAR_VAR (room);
    COPY_CHAR_VAR (prevroom);
    COPY_CHAR_VAR (x);
    COPY_CHAR_VAR (y);
    COPY_CHAR_VAR (wait);
    COPY_CHAR_VAR (flags);
    COPY_CHAR_VAR (following);
    COPY_CHAR_VAR (followinfo);
    COPY_CHAR_VAR (idleview);
    COPY_CHAR_VAR (idletime);
    COPY_CHAR_VAR (idleleft);
    COPY_CHAR_VAR (transparency);
    COPY_CHAR_VAR (baseline);
    COPY_CHAR_VAR (activeinv);
    COPY_CHAR_VAR (loop);
    COPY_CHAR_VAR (frame);
    COPY_CHAR_VAR (walking);
    COPY_CHAR_VAR (animating);
    COPY_CHAR_VAR (walkspeed);
    COPY_CHAR_VAR (animspeed);
    COPY_CHAR_VAR (actx);
    COPY_CHAR_VAR (acty);
    COPY_CHAR_VAR (on);
    strcpy (ci->name, oci->name);
    strcpy (ci->scrname, oci->scrname);
    memcpy (&ci->inv[0], &oci->inv[0], sizeof(short) * 100);
    // move the talking colour into the struct and remove from flags
    ci->talkcolor = (oci->flags & OCHF_SPEECHCOL) >> OCHF_SPEECHCOLSHIFT;
    ci->flags = ci->flags & (~OCHF_SPEECHCOL);
}
