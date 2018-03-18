//
// Created by andrei on 3/17/18.
//

#include "walk_mgr.h"
#include "../archive.h"


void Pink::WalkMgr::deserialize(Pink::Archive &archive) {
    // setLeadActorRef;
    archive >> _locations;
}
