#include <stdlib.h>
#include <stdio.h>
#include <search.h>
#include <assert.h>

#include "ltask.h"
#include "lauxlib.h"
#include "lmem.h"
#include "ldo.h"
#include "ltm.h"
#include "ltable.h"
#include "lvm.h"
#include "lopcodes.h"
#include "lstring.h"

SaveRestoreCallback saveCallback = NULL;

void lua_Save(SaveRestoreFunc saveFunc) {
}
