#include "qdengine/xlibs/util/serialization/Serialization.h"
#include "qdengine/xlibs/util/xtl/StaticMap.h"

namespace QDEngine {

bool StaticMap<class K, class T, class Cmp, class A>::serialize(Archive &ar, const char *name, const char *nameAlt) {
	bool nodeExists = ar.serialize(MapVector, name, nameAlt);
	if (ar.isInput()) // &&(ar.isEdit()))
		std::sort(MapVector.begin(), MapVector.end(), value_comp());
	return nodeExists;
}

} // namespace QDEngine
