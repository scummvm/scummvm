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
#include "lua.h"

SaveRestoreCallback restoreCallback = NULL;

void lua_Restore(SaveRestoreFunc restoreFunc) {
}
