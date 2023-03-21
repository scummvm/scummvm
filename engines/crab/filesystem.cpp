#include "stdafx.h"
#include "filesystem.h"

using namespace boost::filesystem;

#ifdef __GNUC__
int fopen_s(FILE** file, const char * filename, const char * mode)
{
	if(file)
	{
		*file = fopen(filename,mode);
		return *file == NULL;
	}

	return 0;
}
#endif

bool FileOpen(const char *path, char* &data)
{
	if (data != NULL)
		delete[] data;

	FILE* file;
	if (fopen_s(&file, path, "rb"))
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Unable to open file", path, NULL);
		data = NULL;
	}
	else
	{
		fseek(file, 0L, SEEK_END);
		long int length = ftell(file);
		data = new char[length + 1];
		data[length] = '\0';

		fseek(file, 0L, SEEK_SET);
		long int nread = fread(data, 1, length, file);
		fclose(file);

		if (nread < length) // An error has occurred
		{
			delete[] data;
			data = NULL;
		}
		else
			return true;
	}

	return false;
}

bool PathCompare(const path &p1, const path &p2)
{
	return last_write_time(p1) > last_write_time(p2);
}