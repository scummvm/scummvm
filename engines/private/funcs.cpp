#include "common/str.h"
#include "common/debug.h"

#include "grammar.h"
#include "private.h"

namespace Private {

void execFunction(char *name, ArgArray args) {
    if (strcmp(name, "ChgMode") == 0) {
	debug("ChgMode(%d, %s)", args[0].val, args[1].str);  
	_mode = args[0].val;
        Common::String *s = new Common::String(args[1].str);
        _nextSetting = s;
    }
    else if (strcmp(name, "SetFlag") == 0) {
	debug("SetFlag(%s, %d)", args[0].sym->name->c_str(), args[1].val);
        args[0].sym->u.val = args[1].val;
	//_mode = args[0].val;
        //Common::String *s = new Common::String(args[1].str);
        //_nextSetting = s;
    }

    else 
        assert(0);

}

}
