#pragma once
#include "common_header.h"

namespace Crab {
    bool FileOpen(const Common::Path &path, char* &data);
#if 0
    bool PathCompare(const boost::filesystem::path &p1, const boost::filesystem::path &p2);
#endif
} // End of namespace Crab
