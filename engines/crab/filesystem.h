#pragma once
#include "common_header.h"

#ifdef __GNUC__
int fopen_s(FILE** file, const char * filename, const char * mode);
#endif

bool FileOpen(const char *path, char* &data);

bool PathCompare(const boost::filesystem::path &p1, const boost::filesystem::path &p2);