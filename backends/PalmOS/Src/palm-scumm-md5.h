
struct MD5Table {
	const char md5[32 + 1];
	const char target[16 + 1];
	Common::Language language;
	Common::Platform platform;
};

static const MD5Table *md5table;