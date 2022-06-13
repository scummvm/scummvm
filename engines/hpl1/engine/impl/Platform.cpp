#include "hpl1/engine/impl/Platform.h"
#include "hpl1/engine/system/LowLevelSystem.h"


namespace hpl {

long Platform::FileLength(const tWString &asFileName) {
	return 0; 
}

long Platform::FileLength(FILE *pFile) {
	return 0;
}

void Platform::FindFileInDir(tWStringList &alstStrings, tWString asDir, tWString asMask) {

}
} // namespace hpl
