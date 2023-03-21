#ifndef LEVELPATHS_H
#define LEVELPATHS_H

#include <string>

class LevelPaths
{
	std::string layout, asset;

public:
	LevelPaths()
	{
		layout = "";
		asset = "";
	}

	LevelPaths(const char *Layout, const char *Asset)
	{
		layout = Layout;
		asset = Asset;
	}

	const char *LayoutPath() { return layout.c_str(); }
	void LayoutPath(const char* str) { layout = str; }

	const char *AssetPath() { return asset.c_str(); }
	void AssetPath(const char* str) { asset = str; }
};

#endif //LEVELPATHS_H