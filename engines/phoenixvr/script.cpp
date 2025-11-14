#include "phoenixvr/script.h"
#include "common/debug.h"
#include "common/stream.h"
#include "common/textconsole.h"

namespace PhoenixVR {

Script::Script(Common::SeekableReadStream &s) {
	while (!s.eos()) {
		auto line = s.readLine();
		debug("LINE %s\n", line.c_str());
	}
}

} // namespace PhoenixVR
