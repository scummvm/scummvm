#include "common/file.h"
#include "engines/crab/filesystem.h"

namespace Crab {

bool FileOpen(const Common::Path &path, char* &data)
{
	if (data != NULL)
		delete[] data;

	Common::File file;
	if (!file.open(path)) {
		warning("Unable to open file %s", path.toString().c_str());
		data = NULL;
		return false;
	}

	// allocate data
	int64 len = file.size();
	data = new char[len + 1];
	data[len] = '\0';

	// read the file into data
	file.read(data, len);
	file.close();

	return true;
}

#if 0
bool PathCompare(const path &p1, const path &p2)
{
	return last_write_time(p1) > last_write_time(p2);
}
#endif

} // namespace Crab
