/*
 * resource.cpp
 *
 *  Created on: 7-Jun-2009
 *      Author: alex
 */

#include "asylum/resources/resource.h"

namespace Asylum {

Resource::Resource() {
	size   = 0;
	offset = 0;
}

int Resource::save(Common::String filename) {
	FILE *fd;
	fd = fopen(filename.c_str(), "wb+");
	fwrite(data, size, 1, fd);
	fclose(fd);

	return 0;
}

} // end of namespace Asylum
