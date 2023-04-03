static const char *const kEoB2ChargenStrings1DOSChinese[9] = {
	"\xaa\xba\xab\x5f\xc0\x49\xb6\xa4\xa5\xee\xa4\x77\xab\xd8\xa5\xdf\xa7\xb9\xb2\xa6\x2c\xbd\xd0\xb1\x4e\xab\xfc\xbc\xd0\xb2\xbe\xa8\xec\x5b\x50\x4c\x41\x59\x5d\xab\xf6\xa4\x55\xa5\xaa\xc1\xe4\x2c\xa9\xce\xab\xf6\x5b\x50\x5d\xc1\xe4\x2c\xb6\x7d\xa9\x6c\xb1\x7a\xaa\xba\xab\x5f\xc0\x49\xae\xc8\xb5\x7b", /* "的冒險隊伍已建立完畢,請將指標移到[PLAY]按下左鍵,或按[P]鍵,開始您的冒險旅程"; */
	"          ",
	"\xa8\xbe\x3a\r\xa9\x52\x3a\r\xaf\xc5\x3a", /* "防:\r命:\r級:"; */
	"%s\r%d\r%d\r%d\r%d\r%d",
	"%d\r%d",
	"%d",
	"%d/%d",
	"%d/%d/%d",
	"\xbd\xd0\xa5\xfd\xbf\xef\xbe\xdc\xa4\x40\xad\xd3\xa9\xf1\xb8\x6d\r\xa4\x48\xaa\xab\xb9\xcf\xa4\xf9\xaa\xba\xa4\xe8\xae\xd8\x2e", /* "請先選擇一個放置\r人物圖片的方框."; */
};

static const StringListProvider kEoB2ChargenStrings1DOSChineseProvider = { ARRAYSIZE(kEoB2ChargenStrings1DOSChinese), kEoB2ChargenStrings1DOSChinese };

static const char *const kEoB2ChargenStrings2DOSChinese[12] = {
	"%s",
	"%d",
	"%s",
	"%d",
	"%d",
	"%d",
	"%s",
	"%d",
	"\xbf\xef\xbe\xdc\xba\xd8\xb1\xda\x3a", /* "選擇種族:"; */
	"\xbf\xef\xc2\xbe\xb7\x7e", /* "選職業"; */
	"\xbf\xef\xbe\xdc\xb0\x7d\xc0\xe7\x3a", /* "選擇陣營:"; */
	"\xbf\xe9\xa4\x4a\xa9\x6d\xa6\x57\x3a", /* "輸入姓名:"; */
};

static const StringListProvider kEoB2ChargenStrings2DOSChineseProvider = { ARRAYSIZE(kEoB2ChargenStrings2DOSChinese), kEoB2ChargenStrings2DOSChinese };

static const char *const kEoB2ChargenStatStringsDOSChinese[12] = {
	"\xa4\x4f\x3a", /* "力:"; */
	"\xb4\xbc\x3a", /* "智:"; */
	"\xbe\xc7\x3a", /* "學:"; */
	"\xb1\xd3\x3a", /* "敏:"; */
	"\xc5\xe9\x3a", /* "體:"; */
	"\xbe\x79\x3a", /* "魅:"; */
	"\xa4\x4f\xb6\x71\x3a", /* "力量:"; */
	"\xb4\xbc\xaf\xe0\x3a", /* "智能:"; */
	"\xbe\xc7\xb0\xdd\x3a", /* "學問:"; */
	"\xb1\xd3\xb1\xb6\x3a", /* "敏捷:"; */
	"\xc5\xe9\xbd\xe8\x3a", /* "體質:"; */
	"\xbe\x79\xa4\x4f\x3a", /* "魅力:"; */
};

static const StringListProvider kEoB2ChargenStatStringsDOSChineseProvider = { ARRAYSIZE(kEoB2ChargenStatStringsDOSChinese), kEoB2ChargenStatStringsDOSChinese }; // Likely OK

static const char *const kEoB2ChargenRaceSexStringsDOSChinese[12] = {
	"\xa8\x6b\xa9\xca\xa4\x48\xc3\xfe", /* "男性人類"; */
	"\xa4\x6b\xa9\xca\xa4\x48\xc3\xfe", /* "女性人類"; */
	"\xa8\x6b\xa9\xca\xba\xeb\xc6\x46", /* "男性精靈"; */
	"\xa4\x6b\xa9\xca\xba\xeb\xc6\x46", /* "女性精靈"; */
	"\xa8\x6b\xa5\x62\xba\xeb\xc6\x46", /* "男半精靈"; */
	"\xa4\x6b\xa5\x62\xba\xeb\xc6\x46", /* "女半精靈"; */
	"\xa8\x6b\xa9\xca\xb8\x47\xa4\x48", /* "男性矮人"; */
	"\xa4\x6b\xa9\xca\xb8\x47\xa4\x48", /* "女性矮人"; */
	"\xa8\x6b\xa9\xca\xa8\xdc\xbe\xa7", /* "男性侏儒"; */
	"\xa4\x6b\xa9\xca\xa8\xdc\xbe\xa7", /* "女性侏儒"; */
	"\xa8\x6b\xa5\x62\xa8\xad\xa4\x48", /* "男半身人"; */
	"\xa4\x6b\xa5\x62\xa8\xad\xa4\x48", /* "女半身人"; */
};

static const StringListProvider kEoB2ChargenRaceSexStringsDOSChineseProvider = { ARRAYSIZE(kEoB2ChargenRaceSexStringsDOSChinese), kEoB2ChargenRaceSexStringsDOSChinese };

static const char *const kEoB2ChargenClassStringsDOSChinese[21] = {
	"\xbe\xd4\xa4\x68", /* "戰士"; */
	"\xb9\x43\xab\x4c", /* "遊俠"; */
	"\xaa\x5a\xa4\x68", /* "武士"; */
	"\xa7\xc5\xae\x76", /* "巫師"; */
	"\xaa\xaa\xae\x76", /* "牧師"; */
	"\xa4\x70\xb0\xbd", /* "小偷"; */
	"\xbe\xd4\xa4\x68\x2f\xaa\xaa\xae\x76", /* "戰士/牧師"; */
	"\xbe\xd4\xa4\x68\x2f\xa4\x70\xb0\xbd", /* "戰士/小偷"; */
	"\xbe\xd4\xa4\x68\x2f\xa7\xc5\xae\x76", /* "戰士/巫師"; */
	"\xbe\xd4\xa4\x68\x2f\xa7\xc5\xae\x76\x2f\xa4\x70\xb0\xbd", /* "戰士/巫師/小偷"; */
	"\xa4\x70\xb0\xbd\x2f\xa7\xc5\xae\x76", /* "小偷/巫師"; */
	"\xaa\xaa\xae\x76\x2f\xa4\x70\xb0\xbd", /* "牧師/小偷"; */
	"\xbe\xd4\xa4\x68\x2f\xaa\xaa\xae\x76\x2f\xa7\xc5\xae\x76", /* "戰士/牧師/巫師"; */
	"\xb9\x43\xab\x4c\x2f\xaa\xaa\xae\x76", /* "遊俠/牧師"; */
	"\xaa\xaa\xae\x76\x2f\xa7\xc5\xae\x76", /* "牧師/巫師"; */
	"\xbe\xd4\xa4\x68", /* "戰士"; */
	"\xa7\xc5\xae\x76", /* "巫師"; */
	"\xaa\xaa\xae\x76", /* "牧師"; */
	"\xa4\x70\xb0\xbd", /* "小偷"; */
	"\xaa\x5a\xa4\x68", /* "武士"; */
	"\xb9\x43\xab\x4c", /* "遊俠"; */
};

static const StringListProvider kEoB2ChargenClassStringsDOSChineseProvider = { ARRAYSIZE(kEoB2ChargenClassStringsDOSChinese), kEoB2ChargenClassStringsDOSChinese };

static const char *const kEoB2ChargenAlignmentStringsDOSChinese[9] = {
	"\xb5\xb4\xb9\xef\xb5\xbd\xa8\x7d", /* "絕對善良"; */
	"\xb5\xbd\xa8\x7d\xa5\xbb\xa9\xca", /* "善良本性"; */
	"\xa6\xfd\xa8\x44\xa6\x77\xa4\xdf", /* "但求安心"; */
	"\xc4\x59\xa6\x75\xa4\xa4\xa5\xdf", /* "嚴守中立"; */
	"\xa4\xa4\xa5\xdf\xa5\xbb\xa6\xec", /* "中立本位"; */
	"\xc0\x48\xbe\xf7\xc0\xb3\xc5\xdc", /* "隨機應變"; */
	"\xa7\xb9\xa5\xfe\xa8\xb8\xb4\x63", /* "完全邪惡"; */
	"\xa8\xb8\xb4\x63\xa5\xbb\xa9\xca", /* "邪惡本性"; */
	"\xc0\x48\xbf\xb3\xac\xb0\xb4\x63", /* "隨興為惡"; */
};

static const StringListProvider kEoB2ChargenAlignmentStringsDOSChineseProvider = { ARRAYSIZE(kEoB2ChargenAlignmentStringsDOSChinese), kEoB2ChargenAlignmentStringsDOSChinese };

static const char *const kEoB2ChargenEnterGameStringsDOSChinese[1] = {
	"\xb8\xfc\xa4\x4a\xb9\x43\xc0\xb8\xa4\xa4\x21\xbd\xd0\xb5\x79\xb5\xa5\x2e\x2e\x2e", /* "載入遊戲中!請稍等..."; */
};

static const StringListProvider kEoB2ChargenEnterGameStringsDOSChineseProvider = { ARRAYSIZE(kEoB2ChargenEnterGameStringsDOSChinese), kEoB2ChargenEnterGameStringsDOSChinese };

static const char *const kEoB2PryDoorStringsDOSChinese[8] = {
	"\r",
	"\xb1\x7a\xaa\xba\xa4\x4f\xb6\x71\xa4\xa3\xb0\xf7\xbc\xb2\xb6\x7d\xb3\x6f\xb9\x44\xaa\xf9\x2e\r", /* "您的力量不夠撞開這道門."; */
	"\x06\x04\xb1\x7a\xa5\xce\xa4\x4f\xbc\xb2\xaa\xf9\x2e\r", /* "\x06\x04您用力撞門."; */
	"\x06\x06\xb1\x7a\xb9\xc1\xb8\xd5\xb1\x4e\xaa\xf9\xbc\xb2\xb6\x7d\x2c\xa6\xfd\xac\x4f\xb1\x7a\xa5\xa2\xb1\xd1\xa4\x46\x2e\r", /* "\x06\x06您嘗試將門撞開,但是您失敗了."; */
	"\xb1\x7a\xa4\xa3\xaf\xe0\xb1\x4e\xb3\x6f\xb6\xb5\xaa\xab\xab\x7e\xa9\xf1\xb8\x6d\xa9\xf3\xa6\xb9\xb3\x42\x2e\r", /* "您不能將這項物品放置於此處."; */
	"\xb3\x6f\xaa\xab\xab\x7e\xa4\xd3\xa4\x6a\xa4\x46\x2c\xb1\x7a\xa9\xf1\xa4\xa3\xb6\x69\xa5\x68\x2e\r", /* "這物品太大了,您放不進去."; */
	"\xa8\x53\xa6\xb3\xa4\x48\xaf\xe0\xb0\xf7\xb1\x4e\xb3\x6f\xb9\x44\xaa\xf9\xa5\xb4\xb6\x7d\x2e\r", /* "沒有人能夠將這道門打開."; */
	"\r"
};

static const StringListProvider kEoB2PryDoorStringsDOSChineseProvider = { ARRAYSIZE(kEoB2PryDoorStringsDOSChinese), kEoB2PryDoorStringsDOSChinese };

static const char *const kEoB2WarningStringsDOSChinese[4] = {
	"\xb1\x7a\xb5\x4c\xaa\x6b\xa8\xab\xb9\x4c\xa5\x68\x2e\r", /* "您無法走過去."; */
	"%s\xa4\x77\xb8\x67\xb5\x4c\xaa\x6b\xa6\x59\xaa\x46\xa6\xe8\xa4\x46\x21\r", /* "%s已經無法吃東西了!"; */
	"\xb3\x6f\xad\xb9\xaa\xab\xa4\x77\xb8\x67\xbb\x47\xc3\x61\xa4\x46\x21\xb1\x7a\xa4\xa3\xad\x6e\xa6\x59\x21\r", /* "這食物已經腐壞了!您不要吃!"; */
	"\xb1\x7a\xa5\x75\xaf\xe0\xb0\xf7\xa6\x59\xad\xb9\xaa\xab\x21\r", /* "您只能夠吃食物!"; */
};

static const StringListProvider kEoB2WarningStringsDOSChineseProvider = { ARRAYSIZE(kEoB2WarningStringsDOSChinese), kEoB2WarningStringsDOSChinese };

static const char *const kEoB2ItemSuffixStringsRingsDOSChinese[4] = {
	"\xaf\xc2\xba\xe9\xb8\xcb\xb9\xa2\xac\xfc\xc6\x5b\xa5\xce", /* "純粹裝飾美觀用"; */
	"\xbc\x57\xb1\x6a\xa7\xc5\xae\x76\xb0\x4f\xbe\xd0\xaa\x6b\xb3\x4e", /* "增強巫師記憶法術"; */
	"\xab\x4f\xab\xf9\xa4\xb8\xae\xf0\xa4\xa3\xb7\x7c\xc4\xc8\xb4\xf7", /* "保持元氣不會饑渴"; */
	"\xbc\x59\xb8\xa8\xb3\xb4\xa7\x7c\xa4\xa3\xb7\x7c\xa8\xfc\xb6\xcb", /* "墜落陷坑不會受傷"; */
};

static const StringListProvider kEoB2ItemSuffixStringsRingsDOSChineseProvider = { ARRAYSIZE(kEoB2ItemSuffixStringsRingsDOSChinese), kEoB2ItemSuffixStringsRingsDOSChinese };


static const char *const kEoB2ItemSuffixStringsPotionsDOSChinese[8] = {
	"\xbc\x57\xa5\x5b\xa4\x4f\xb6\x71", /* "增加力量"; */
	"\xa4\x40\xaf\xeb\xaa\x76\xc0\xf8", /* "一般治療"; */
	"\xbc\x57\xb1\x6a\xa5\xcd\xa9\x52\xa4\x4f", /* "增強生命力"; */
	"\xa6\xb3\xac\x72", /* "有毒"; */
	"\xab\xec\xb4\x5f\xa4\xb8\xae\xf0", /* "恢復元氣"; */
	"\xbc\x57\xa5\x5b\xac\xa1\xa4\x4f\xb3\x74\xab\xd7", /* "增加活力速度"; */
	"\xc1\xf4\xa7\xce\xa5\xce", /* "隱形用"; */
	"\xb8\xd1\xac\x72\xa5\xce", /* "解毒用"; */
};

static const StringListProvider kEoB2ItemSuffixStringsPotionsDOSChineseProvider = { ARRAYSIZE(kEoB2ItemSuffixStringsPotionsDOSChinese), kEoB2ItemSuffixStringsPotionsDOSChinese };

static const char *const kEoB2ItemSuffixStringsWandsDOSChinese[8] = {
	"\xb7\x7c\xc2\x48\xa4\xe2", /* "會黏手"; */
	"\xac\x49\xae\x69\xb9\x70\xb9\x71\xb3\x4e", /* "施展雷電術"; */
	"\xac\x49\xae\x69\xa7\x4e\xc0\x40\xb3\x4e", /* "施展冷錐術"; */
	"\xaa\x76\xc0\xf8\xad\xab\xb6\xcb", /* "治療重傷"; */
	"\xac\x49\xae\x69\xa4\xf5\xb2\x79\xb3\x4e", /* "施展火球術"; */
	"\xac\x50\xa4\xf5\xc4\x5f\xa7\xfa", /* "星火寶杖"; */
	"\xac\x49\xae\x69\xc5\x5d\xaa\x6b\xad\xb8\xbc\x75", /* "施展魔法飛彈"; */
	"\xac\x49\xae\x69\xb8\xd1\xa9\x47\xb3\x4e", /* "施展解咒術"; */
};

static const StringListProvider kEoB2ItemSuffixStringsWandsDOSChineseProvider = { ARRAYSIZE(kEoB2ItemSuffixStringsWandsDOSChinese), kEoB2ItemSuffixStringsWandsDOSChinese };

static const char *const kEoB2RipItemStringsDOSChinese[3] = {
	"\x25\x73\xbf\xf2\xa5\xa2\xa4\x46\xa6\x6f\xaa\xba", /* "%s遺失了她的"; */
	"\x25\x73\xbf\xf2\xa5\xa2\xa4\x46\xa5\x4c\xaa\xba", /* "%s遺失了他的"; */
	".\r"
};

static const StringListProvider kEoB2RipItemStringsDOSChineseProvider = { ARRAYSIZE(kEoB2RipItemStringsDOSChinese), kEoB2RipItemStringsDOSChinese };

static const char *const kEoB2CursedStringDOSChinese[1] = {
	"\xb3\x51\xb6\x41\xa9\x47\xaa\xba\x25\x64\x25\x73", /* "被詛咒的%d%s"; */
};

static const StringListProvider kEoB2CursedStringDOSChineseProvider = { ARRAYSIZE(kEoB2CursedStringDOSChinese), kEoB2CursedStringDOSChinese };

static const char *const kEoB2MagicObjectStringsDOSChinese[5] = {
	"\xa7\xc5\xae\x76", /* "巫師"; */
	"\xaa\xaa\xae\x76", /* "牧師"; */
	"\xa7\xd9\xab\xfc", /* "戒指"; */
	"\xc3\xc4\xa4\xf4", /* "藥水"; */
	"\xc5\x5d\xa7\xfa", /* "魔杖"; */
};

static const StringListProvider kEoB2MagicObjectStringsDOSChineseProvider = { ARRAYSIZE(kEoB2MagicObjectStringsDOSChinese), kEoB2MagicObjectStringsDOSChinese };

static const char *const kEoB2MagicObjectString5DOSChinese[1] = {
	"Stick"
};

static const StringListProvider kEoB2MagicObjectString5DOSChineseProvider = { ARRAYSIZE(kEoB2MagicObjectString5DOSChinese), kEoB2MagicObjectString5DOSChinese };

static const char *const kEoB2PatternSuffixDOSChinese[1] = {
	"%s of %s"
};

static const StringListProvider kEoB2PatternSuffixDOSChineseProvider = { ARRAYSIZE(kEoB2PatternSuffixDOSChinese), kEoB2PatternSuffixDOSChinese }; 

static const char *const kEoB2PatternGrFix1DOSChinese[1] = {
	"%s of %s"
};

static const StringListProvider kEoB2PatternGrFix1DOSChineseProvider = { ARRAYSIZE(kEoB2PatternGrFix1DOSChinese), kEoB2PatternGrFix1DOSChinese };

static const char *const kEoB2PatternGrFix2DOSChinese[1] = {
	"%s of %s"
};

static const StringListProvider kEoB2PatternGrFix2DOSChineseProvider = { ARRAYSIZE(kEoB2PatternGrFix2DOSChinese), kEoB2PatternGrFix2DOSChinese };

static const char *const kEoB2ValidateArmorStringDOSChinese[1] = {
	"%s can't wear that type of armor.\r"
};

static const StringListProvider kEoB2ValidateArmorStringDOSChineseProvider = { ARRAYSIZE(kEoB2ValidateArmorStringDOSChinese), kEoB2ValidateArmorStringDOSChinese };

static const char *const kEoB2ValidateCursedStringDOSChinese[1] = {
	"%s cannot release the weapon!  It is cursed!\r"
};

static const StringListProvider kEoB2ValidateCursedStringDOSChineseProvider = { ARRAYSIZE(kEoB2ValidateCursedStringDOSChinese), kEoB2ValidateCursedStringDOSChinese };

static const char *const kEoB2ValidateNoDropStringDOSChinese[1] = {
	"\xb1\x7a\xa4\xa3\xaf\xe0\xb1\x4e\xb3\x6f\xb6\xb5\xaa\xab\xab\x7e\xa9\xf1\xb8\x6d\xa6\x62\xa8\xba\xad\xd3\xa6\xec\xb8\x6d\x2e\r", /* "您不能將這項物品放置在那."; */
};

static const StringListProvider kEoB2ValidateNoDropStringDOSChineseProvider = { ARRAYSIZE(kEoB2ValidateNoDropStringDOSChinese), kEoB2ValidateNoDropStringDOSChinese };

static const char *const kEoB2PotionStringsDOSChinese[2] = {
	"\xa4\xa4\xac\x72\xa4\x46", /* "中毒了"; */
	"\x25\x73\xb7\x50\xc4\xb1\x25\x73\x21\x0d", /* "%s感覺%s!\r"; */
};

static const StringListProvider kEoB2PotionStringsDOSChineseProvider = { ARRAYSIZE(kEoB2PotionStringsDOSChinese), kEoB2PotionStringsDOSChinese };

static const char *const kEoB2WandStringsDOSChinese[1] = {
	"\xb3\x6f\xa4\xec\xa7\xfa\xa5\x7e\xc6\x5b\xa4\x57\xac\xdd\xa8\xd3\x2c\xa8\xc3\xa8\x53\xa6\xb3\xa4\xb0\xbb\xf2\xc5\x5d\xa4\x4f\r", /* "這木杖外觀上看來,並沒有什麼魔力"; */
};

static const StringListProvider kEoB2WandStringsDOSChineseProvider = { ARRAYSIZE(kEoB2WandStringsDOSChinese), kEoB2WandStringsDOSChinese };

static const char *const kEoB2ItemMisuseStringsDOSChinese[3] = {
	"%s can not use this item.\r",
	"This item automatically used when worn.\r",
	"This item is not used in this way.\r"
};

static const StringListProvider kEoB2ItemMisuseStringsDOSChineseProvider = { ARRAYSIZE(kEoB2ItemMisuseStringsDOSChinese), kEoB2ItemMisuseStringsDOSChinese };

static const char *const kEoB2TakenStringsDOSChinese[1] = {
	"\xae\xb3\xa8\xfa", /* "拿取"; */
};

static const StringListProvider kEoB2TakenStringsDOSChineseProvider = { ARRAYSIZE(kEoB2TakenStringsDOSChinese), kEoB2TakenStringsDOSChinese };

static const char *const kEoB2PotionEffectStringsDOSChinese[8] = {
	"\xab\x44\xb1\x60\xb1\x6a\xa7\xa7", /* "非常強壯"; */
	"\xa4\xf1\xb8\xfb\xa6\x6e\xa4\x46", /* "比較好了"; */
	"\xab\x44\xb1\x60\xa6\x6e", /* "非常好"; */
	"\xa8\xec\xab\xdc\xa4\xa3\xb5\xce\xaa\x41", /* "到很不舒服"; */
	"\xa8\x7b\xa4\x6c\xab\xdc\xb9\xa1\xba\xa1", /* "肚子很飽滿"; */
	"\xa7\xf3\xa7\xd6\xa9\x4d\xb1\xd3\xb1\xb6", /* "更快和敏捷"; */
	"\xc5\xdc\xa6\xa8\xc1\xf4\xa7\xce\xa4\x46", /* "變成隱形了"; */
	"\xac\x72\xa9\xca\xa4\x77\xb8\x67\xae\xf8\xa5\xa2\xa4\x46", /* "毒性已經消失了"; */
};

static const StringListProvider kEoB2PotionEffectStringsDOSChineseProvider = { ARRAYSIZE(kEoB2PotionEffectStringsDOSChinese), kEoB2PotionEffectStringsDOSChinese };

static const char *const kEoB2YesNoStringsDOSChinese[2] = {
	"yes",
	"no"
};

static const StringListProvider kEoB2YesNoStringsDOSChineseProvider = { ARRAYSIZE(kEoB2YesNoStringsDOSChinese), kEoB2YesNoStringsDOSChinese };

static const char *const kEoB2MoreStringsDOSChinese[1] = {
	"MORE"
};

static const StringListProvider kEoB2MoreStringsDOSChineseProvider = { ARRAYSIZE(kEoB2MoreStringsDOSChinese), kEoB2MoreStringsDOSChinese };

static const char *const kEoB2NpcMaxStringsDOSChinese[1] = {
	"\xb1\x7a\xaa\xba\xb6\xa4\xa5\xee\xa4\xa4\xb3\xcc\xa6\x68\xa5\x75\xaf\xe0\xa6\xb3\xa4\xbb\xa6\xec\xb6\xa4\xad\xfb\x2c\xbd\xd0\xbf\xef\xbe\xdc\xa4\x40\xa6\xec\xb1\x7a\xad\x6e\xb8\xd1\xb9\xb5\xaa\xba\xb6\xa4\xad\xfb", /* "您的隊伍中最多只能有六位隊員,請選擇一位您要解僱的隊員"; */
};

static const StringListProvider kEoB2NpcMaxStringsDOSChineseProvider = { ARRAYSIZE(kEoB2NpcMaxStringsDOSChinese), kEoB2NpcMaxStringsDOSChinese };

static const char *const kEoB2OkStringsDOSChinese[1] = {
	"OK"
};

static const StringListProvider kEoB2OkStringsDOSChineseProvider = { ARRAYSIZE(kEoB2OkStringsDOSChinese), kEoB2OkStringsDOSChinese };

static const char *const kEoB2NpcJoinStringsDOSChinese[1] = {
	"%s\xa5\x5b\xa4\x4a\xab\x5f\xc0\x49\xb6\xa4\xa5\xee\xa4\xba\x2e\r", /* "%s加入冒險隊伍內."; */
};

static const StringListProvider kEoB2NpcJoinStringsDOSChineseProvider = { ARRAYSIZE(kEoB2NpcJoinStringsDOSChinese), kEoB2NpcJoinStringsDOSChinese };

static const char *const kEoB2CancelStringsDOSChinese[1] = {
	"CANCEL"
};

static const StringListProvider kEoB2CancelStringsDOSChineseProvider = { ARRAYSIZE(kEoB2CancelStringsDOSChinese), kEoB2CancelStringsDOSChinese };

static const char *const kEoB2AbortStringsDOSChinese[1] = {
	"ABORT"
};

static const StringListProvider kEoB2AbortStringsDOSChineseProvider = { ARRAYSIZE(kEoB2AbortStringsDOSChinese), kEoB2AbortStringsDOSChinese };

static const char *const kEoB2MenuStringsMainDOSChinese[8] = {
	"\xbd\xd0\x20\xbf\xef\x20\xbe\xdc\x20\xb6\xb5\x20\xa5\xd8\x3a", /* "請 選 擇 項 目:"; */
	"\xc5\xfd\x20\xb6\xa4\x20\xa5\xee\x20\xa5\xf0\x20\xae\xa7", /* "讓 隊 伍 休 息"; */
	"\xb0\x4f\x20\x20\xbe\xd0\x20\x20\xaa\x6b\x20\x20\xb3\x4e", /* "記  憶  法  術"; */
	"\xac\xe8\x20\x20\xa8\x44\x20\x20\xaa\x6b\x20\x20\xb3\x4e", /* "祈  求  法  術"; */
	"\xa7\xdb\x20\x20\xbc\x67\x20\x20\xaa\x6b\x20\x20\xb3\x4e", /* "抄  寫  法  術"; */
	"\xa5\x5c\x20\x20\xaf\xe0\x20\x20\xb3\x5d\x20\x20\xa9\x77", /* "功  能  設  定"; */
	"\xb9\x43\x20\x20\xc0\xb8\x20\x20\xbf\xef\x20\x20\xb6\xb5", /* "遊  戲  選  項"; */
	"\xc2\xf7\xb6\x7d", /* "離開"; */
};

static const StringListProvider kEoB2MenuStringsMainDOSChineseProvider = { ARRAYSIZE(kEoB2MenuStringsMainDOSChinese), kEoB2MenuStringsMainDOSChinese }; // Likely OK

static const char *const kEoB2MenuStringsSaveLoadDOSChinese[8] = {
	"\xb8\xfc\xa4\x4a\xb9\x43\xc0\xb8\xb6\x69\xab\xd7", /* "載入遊戲進度"; "Load Game" */
	"\xc0\x78\xa6\x73\xb9\x43\xc0\xb8\xb6\x69\xab\xd7", /* "儲存遊戲進度"; "Save Game" */
	"\xbb\xba\x20\xb4\xb2\x20\xb6\xa4\x20\xad\xfb", /* "遣 散 隊 員"; "Drop Character" */
	"\xb5\xb2\x20\xa7\xf4\x20\xb9\x43\x20\xc0\xb8", /* "結 束 遊 戲"; "Quit Game" */
	"\xb9\x43\x20\x20\xc0\xb8\x20\x20\xbf\xef\x20\x20\xb6\xb5\x3a", /* "遊  戲  選  項:"; "Game Options:" */
	"\x0d\x20\x20\x20\xa6\x73\x20\xc0\xc9\x20\xa7\xb9\x20\xb2\xa6\x2e", /* "\r   存 檔 完 畢."; "\r   Game saved." */
	"\x0d\x20\x20\x20\xa6\x73\x20\xc0\xc9\x20\xa5\xa2\x20\xb1\xd1\x2e", /* "\r   存 檔 失 敗."; "Attempts to save your\rgame have failed!", */
	"\xb5\x4c\xaa\x6b\xb8\xfc\xa4\x4a\xc0\xc9\xae\xd7\x2c\xa5\x69\xaf\xe0\x0d\xc0\xc9\xae\xd7\xa4\x77\xb8\x67\xb7\x6c\xc3\x61", /* "無法載入檔案,可能\r檔案已經損壞"; "Cannot load your\rsave game.  The\rfile may be corrupt!" */
};

static const StringListProvider kEoB2MenuStringsSaveLoadDOSChineseProvider = { ARRAYSIZE(kEoB2MenuStringsSaveLoadDOSChinese), kEoB2MenuStringsSaveLoadDOSChinese }; // Likely ok

static const char *const kEoB2MenuStringsOnOffDOSChinese[2] = {
	"\xb6\x7d", /* "開"; */
	"\xc3\xf6", /* "關"; */
};

static const StringListProvider kEoB2MenuStringsOnOffDOSChineseProvider = { ARRAYSIZE(kEoB2MenuStringsOnOffDOSChinese), kEoB2MenuStringsOnOffDOSChinese };

static const char *const kEoB2MenuStringsSpellsDOSChinese[17] = {
	"\x0d\x0d\xb1\x71\xb1\x7a\xaa\xba\xb6\xa4\xa5\xee\xa4\xa4\xbf\xef\xbe\xdc\xa4\x40\x0d\xad\xd3\xbe\x41\xb7\xed\xaa\xba\xb6\xa4\xad\xfb\xa8\xd3\xbe\xc7\xb2\xdf\x0d\xaa\x6b\xb3\x4e\x2e", /* "\r\r從您的隊伍中選擇一\r個適當的隊員來學習\r法術."; */
	"\x0d\xb1\x7a\xaa\xba\xaa\x5a\xa4\x68\xb5\xa5\xaf\xc5\xa4\xd3\xa7\x43\x2c\x0d\xb5\x4c\xaa\x6b\xa8\xcf\xa5\xce\xaa\x6b\xb3\x4e\x2e", /* "\r您的武士等級太低,\r無法使用法術."; */
	"\x0d\xa8\x53\xa6\xb3\xaa\x6b\xb3\x4e\xb1\xb2\xb6\x62", /* "\r沒有法術捲軸"; */
	"\x0d\x0d\xbd\xd0\xb1\x71\xb1\x7a\xaa\xba\xab\x5f\xc0\x49\xb6\xa4\xa5\xee\xa4\xa4\xbf\xef\x0d\xa5\x58\xa4\x40\xad\xd3\xb6\xa4\xad\xfb\xa8\xd3\xac\xe8\xa8\x44\xaa\x6b\xb3\x4e\x2e", /* "\r\r請從您的冒險隊伍中選\r出一個隊員來祈求法術."; */
	"\x0d\xa8\x53\xa6\xb3\xa5\xf4\xa6\xf3\xa4\x40\xa6\xec\xb6\xa4\xad\xfb\x0d\xa5\x69\xa5\x48\xac\xe8\xa8\x44\xb7\x73\xaa\xba\xaa\x6b\xb3\x4e\x2e", /* "\r沒有任何一位隊員\r可以祈求新的法術."; */
	"\x0d\xa8\x53\xa6\xb3\xa5\xf4\xa6\xf3\xa4\x40\xa6\xec\xb6\xa4\xad\xfb\x0d\xa5\x69\xa5\x48\xbe\xc7\xb2\xdf\xb7\x73\xaa\xba\xaa\x6b\xb3\x4e\x2e", /* "\r沒有任何一位隊員\r可以學習新的法術."; */
	"\xa5\xa2\xa5\x68\xaa\xbe\xc4\xb1\xa9\xce\xa4\x77\xa6\xba\xa4\x60\xaa\xba\x0d\xa7\xc5\xae\x76\xa4\xa3\xaf\xe0\xbe\xc7\xb2\xdf\xaa\x6b\xb3\x4e\x2e", /* "失去知覺或已死亡的\r巫師不能學習法術."; */
	"\xa5\xa2\xa5\x68\xaa\xbe\xc4\xb1\xa9\xce\xa4\x77\xa6\xba\xa4\x60\xaa\xba\x0d\xaa\xaa\xae\x76\xa4\xa3\xaf\xe0\xac\xe8\xa8\x44\xaa\x6b\xb3\x4e\x2e", /* "失去知覺或已死亡的\r牧師不能祈求法術."; */
	"1",
	"2",
	"3",
	"4",
	"5",
	"\xb2\x4d\xb0\xa3", /* "清除"; */
	"\xa5\x69\xa5\xce\xaa\xba\xaa\x6b\xb3\x4e\x3a", /* "可用的法術:"; */
	"\xac\x4f", /* "是"; */
	"\xa4\xa3", /* "不"; */
};

static const StringListProvider kEoB2MenuStringsSpellsDOSChineseProvider = { ARRAYSIZE(kEoB2MenuStringsSpellsDOSChinese), kEoB2MenuStringsSpellsDOSChinese }; // Likely OK

static const char *const kEoB2MenuStringsRestDOSChinese[5] = {
	"\xa7\x41\xaa\xba\xaa\x76\xc0\xf8\xaa\xcc\xad\x6e\xc2\xe5\xaa\x76\x0d\xb6\xa4\xad\xfb\xb6\xdc\x3f\x20\x20\x20\x20\x20\x20\x20\x20", /* "你的治療者要醫治\r隊員嗎?        "; */
	"\xa6\xb9\xb3\x42\xa6\xfc\xa5\x47\xa6\xb3\xa6\x4d\xc0\x49\xa6\x73\xa6\x62\x21\x0d\xb1\x7a\xad\x6e\xa6\x62\xa6\xb9\xa5\xf0\xae\xa7\xc0\xf8\xbe\x69\xb6\xdc\x3f", /* "此處似乎有危險存在!\r您要在此休息療養嗎?"; */
	"\x20\x20\xb6\xa4\xa5\xee\xa5\xf0\xae\xa7\xa4\xa4\x2e", /* "  隊伍休息中."; */
	"\x0d\xa5\xfe\xb3\xa1\xaa\xba\xb6\xa4\xad\xfb\xa4\x77\xb8\x67\xb1\x6f\xa8\xec\x0d\xa4\x46\xa5\x52\xa5\xf7\xaa\xba\xa5\xf0\xae\xa7", /* "\r全部的隊員已經得到\r了充份的休息"; */
	"\xb1\x7a\xaa\xba\xb6\xa4\xa5\xee\xbb\xdd\xad\x6e\xa5\xf0\xae\xa7\x0d\xae\xc9\xb6\xa1\x2c\xa5\x48\xab\x4b\xbe\xc7\xb2\xdf\xaa\x6b\xb3\x4e", /* "您的隊伍需要休息\r時間,以便學習法術"; */
};

static const StringListProvider kEoB2MenuStringsRestDOSChineseProvider = { ARRAYSIZE(kEoB2MenuStringsRestDOSChinese), kEoB2MenuStringsRestDOSChinese }; // Likely OK

static const char *const kEoB2MenuStringsDropDOSChinese[1] = {
	"\xaa\x60\xb7\x4e\x21\xb1\x7a\xaa\xba\xb6\xa4\xad\xfb\xc1\x60\xa6\x40\x0d\xa4\xa3\xb1\x6f\xa4\xd6\xa9\xf3\xa5\x7c\xa4\x48", /* "注意!您的隊員總共\r不得少於四人"; */
};

static const StringListProvider kEoB2MenuStringsDropDOSChineseProvider = { ARRAYSIZE(kEoB2MenuStringsDropDOSChinese), kEoB2MenuStringsDropDOSChinese }; // Likely OK

static const char *const kEoB2MenuStringsExitDOSChinese[1] = {
	"\x0d\xb1\x7a\xbd\x54\xa9\x77\xad\x6e\xb5\xb2\xa7\xf4\xb9\x43\xc0\xb8\x3f", /* "\r您確定要結束遊戲?"; */
};

static const StringListProvider kEoB2MenuStringsExitDOSChineseProvider = { ARRAYSIZE(kEoB2MenuStringsExitDOSChinese), kEoB2MenuStringsExitDOSChinese }; // Likely OK

static const char *const kEoB2MenuStringsStarveDOSChinese[1] = {
	"\xb1\x7a\xaa\xba\xb6\xa4\xad\xfb\xa5\xbf\xb3\x42\xa9\xf3\xc4\xc8\xbe\x6a\x0d\xa4\xa4\x2e\xb1\x7a\xad\x6e\xc4\x7e\xc4\xf2\xa5\xf0\xae\xa7\xb6\xdc\x3f", /* "您的隊員正處於饑餓\r中.您要繼續休息嗎?"; */
};

static const StringListProvider kEoB2MenuStringsStarveDOSChineseProvider = { ARRAYSIZE(kEoB2MenuStringsStarveDOSChinese), kEoB2MenuStringsStarveDOSChinese }; // Likely OK

static const char *const kEoB2MenuStringsScribeDOSChinese[5] = {
	"\xbd\xd0\xbf\xef\xbe\xdc\xb1\x7a\xad\x6e\xa7\xdb\xbc\x67\xaa\xba\xb1\xb2\xb6\x62", /* "請選擇您要抄寫的捲軸"; */
	"\xbd\xd0\xb1\x71\xb6\xa4\xad\xfb\xa4\xa4\xbf\xef\xa5\x58\xa4\x40\xad\xd3\x0d\xa7\xc5\xae\x76\xa8\xd3\xa7\xdb\xbc\x67\xaa\x6b\xb3\x4e\x2e", /* "請從隊員中選出一個\r巫師來抄寫法術."; */
	"\xb1\x7a\xa8\x53\xa6\xb3\xa5\xf4\xa6\xf3\xc5\x5d\xaa\x6b\xb1\xb2\xb6\x62\x0d\xa5\x69\xa5\x48\xa5\xce\xa8\xd3\xa7\xdb\xbc\x67\xaa\x6b\xb3\x4e\x2e", /* "您沒有任何魔法捲軸\r可以用來抄寫法術."; */
	"\xb1\x7a\xa8\xc3\xa8\x53\xa6\xb3\xb3\x6f\xa6\xec\xa7\xc5\xae\x76\x0d\xa5\xbc\xbe\xc7\xb9\x4c\xaa\xba\xc5\x5d\xaa\x6b\xb1\xb2\xb6\x62\x2e", /* "您並沒有這位巫師\r未學過的魔法捲軸."; */
	"\xb1\x7a\xa8\xc3\xa8\x53\xa6\xb3\xa7\xc5\xae\x76\xa5\x69\xa5\x48\x0d\xa7\xdb\xbc\x67\xc5\x5d\xaa\x6b\xb1\xb2\xb6\x62", /* "您並沒有巫師可以\r抄寫魔法捲軸"; */
};

static const StringListProvider kEoB2MenuStringsScribeDOSChineseProvider = { ARRAYSIZE(kEoB2MenuStringsScribeDOSChinese), kEoB2MenuStringsScribeDOSChinese };  // Likely OK

static const char *const kEoB2MenuStringsDrop2DOSChinese[3] = {
	"\x0d\x0d\x0d\xbd\xd0\xbf\xef\xbe\xdc\xb1\x7a\xb7\x51\xbb\xba\xb4\xb2\xaa\xba\xb6\xa4\xad\xfb\x2e", /* "\r\r\r請選擇您想遣散的隊員."; */
	"\x0d\x20\xb1\x7a\xa5\xb2\xb6\xb7\xbf\xe9\xa4\x4a\xc0\xc9\xae\xd7\xa6\x57\xba\xd9", /* "\r 您必須輸入檔案名稱"; */
	"\xb1\x7a\xbd\x54\xa9\x77\xb7\x73\xa6\x73\xa9\xf1\xaa\xba\xc0\xc9\xae\xd7\x0d\xad\x6e\xa8\xfa\xa5\x4e\xb3\x6f\xad\xd3\xc2\xc2\xc0\xc9\xae\xd7\x3f", /* "您確定新存放的檔案\r要取代這個舊檔案?"; */
};

static const StringListProvider kEoB2MenuStringsDrop2DOSChineseProvider = { ARRAYSIZE(kEoB2MenuStringsDrop2DOSChinese), kEoB2MenuStringsDrop2DOSChinese };  // Likely OK

static const char *const kEoB2MenuStringsHeadDOSChinese[3] = {
	"\xb2\xcf\xc0\xe7\x3a", /* "紮營:"; */
	"\xa5\x5c\x20\x20\xaf\xe0\x20\x20\xb3\x5d\x20\x20\xa9\x77\x3a", /* "功  能  設  定:"; */
	"\xb9\x43\x20\x20\xc0\xb8\x20\x20\xbf\xef\x20\x20\xb6\xb5\x3a", /* "遊  戲  選  項:"; */
};

static const StringListProvider kEoB2MenuStringsHeadDOSChineseProvider = { ARRAYSIZE(kEoB2MenuStringsHeadDOSChinese), kEoB2MenuStringsHeadDOSChinese }; // Likely OK

static const char *const kEoB2MenuStringsPoisonDOSChinese[1] = {
	"\xa6\xb3\xb6\xa4\xad\xfb\xa4\xa4\xac\x72\xa4\x46\xc0\x48\xae\xc9\xa5\x69\x0d\xaf\xe0\xb1\xbe\xb1\xbc\x21\xc1\xd9\xad\x6e\xa5\xf0\xae\xa7\xb6\xdc\x3f", /* "有隊員中毒了隨時可\r能掛掉!還要休息嗎?"; */
};

static const StringListProvider kEoB2MenuStringsPoisonDOSChineseProvider = { ARRAYSIZE(kEoB2MenuStringsPoisonDOSChinese), kEoB2MenuStringsPoisonDOSChinese }; // Likely OK

static const char *const kEoB2MenuStringsMgcDOSChinese[2] = {
	"%-8s %1d",
	"\xa5\x69\xa5\xce\xc2\x49\xbc\xc6\x3a\x25\x64", /* "可用點數:%d"; */
};

static const StringListProvider kEoB2MenuStringsMgcDOSChineseProvider = { ARRAYSIZE(kEoB2MenuStringsMgcDOSChinese), kEoB2MenuStringsMgcDOSChinese };

static const char *const kEoB2MenuStringsPrefsDOSChinese[4] = {
	"Tunes are %-3s",
	"\xad\x49\xb4\xba\xad\xb5\xae\xc4\x25\x2d\x31\x73", /* "背景音效%-1s"; */
	"\xb1\xf8\xaa\xac\xb9\xcf\xaa\xed\x25\x2d\x31\x73", /* "條狀圖表%-1s"; */
	"Mouse is %-3s"
};

static const StringListProvider kEoB2MenuStringsPrefsDOSChineseProvider = { ARRAYSIZE(kEoB2MenuStringsPrefsDOSChinese), kEoB2MenuStringsPrefsDOSChinese };

static const char *const kEoB2MenuStringsRest2DOSChinese[5] = {
	"\x25\x73\xbe\xc7\xb7\x7c\xa4\x46\x25\x73\x2e\x0d", /* "%s學會了%s.\r"; */	
	"\x25\x73\xa4\x77\xb0\x4f\xbe\xd0\xa6\xed\x25\x73\xaa\xba\xaa\x6b\xb3\x4e\x2e\x0d", /* "%s已記憶住%s的法術.\r"; */
	"\x25\x73\xac\x49\xae\x69\xaa\x76\xc0\xf8\xb3\x4e\xa8\xd3\xaa\x76\xc0\xf8\x25\x73\xaa\xba\xb6\xcb\xb6\xd5\x2e", /* "%s施展治療術來治療%s的傷勢."; */
	"\xa4\x77\xa5\xf0\xae\xa7\xaa\xba\xae\xc9\xb6\xa1\x3a\x20\x25\x2d\x34\x64", /* "已休息的時間: %-4d"; */
	"\r%s\r"
};

static const StringListProvider kEoB2MenuStringsRest2DOSChineseProvider = { ARRAYSIZE(kEoB2MenuStringsRest2DOSChinese), kEoB2MenuStringsRest2DOSChinese };

static const char *const kEoB2MenuStringsRest3DOSChinese[1] = {
	"\x06\x06\xb1\x7a\xc4\xb1\xb1\x6f\xa6\x62\xb3\x6f\xb8\xcc\xa5\xf0\xae\xa7\x2c\xa8\xc3\xa4\xa3\xac\x4f\xab\xdc\xa6\x77\xa5\xfe\x2e", /* "\x06\x06您覺得在這裡休息,並不是很安全."; */
};

static const StringListProvider kEoB2MenuStringsRest3DOSChineseProvider = { ARRAYSIZE(kEoB2MenuStringsRest3DOSChinese), kEoB2MenuStringsRest3DOSChinese };

static const char *const kEoB2MenuStringsRest4DOSChinese[1] = {
	"\x06\x06\xb1\x7a\xa7\x4f\xa5\xf0\xae\xa7\xa4\x46\x2c\xa9\xc7\xaa\xab\xa8\xd3\xa4\x46\x21", /* "\x06\x06您別休息了,怪物來了!"; */
};

static const StringListProvider kEoB2MenuStringsRest4DOSChineseProvider = { ARRAYSIZE(kEoB2MenuStringsRest4DOSChinese), kEoB2MenuStringsRest4DOSChinese };

static const char *const kEoB2MenuStringsDefeatDOSChinese[1] = {
	"\xb1\x7a\xaa\xba\xbe\xe3\xad\xd3\xab\x5f\xc0\x49\xb6\xa4\xa5\xee\xa4\x77\x0d\xa5\xfe\xad\x78\xb0\x7d\xa4\x60\x21\xb1\x7a\xad\x6e\xb8\xfc\xa4\x4a\xa5\x48\x0d\xab\x65\xa9\xd2\xc0\x78\xa6\x73\xaa\xba\xb6\x69\xab\xd7\xc0\xc9\xb6\xdc\x3f", /* "您的整個冒險隊伍已\r全軍陣亡!您要載入以\r前所儲存的進度檔嗎?"; */
};

static const StringListProvider kEoB2MenuStringsDefeatDOSChineseProvider = { ARRAYSIZE(kEoB2MenuStringsDefeatDOSChinese), kEoB2MenuStringsDefeatDOSChinese }; // Likely OK

static const char *const kEoB2MenuStringsTransferDOSChinese[5] = {
	"\xa6\x62\xb6\x7d\xa9\x6c\xb9\x43\xc0\xb8\xa5\x48\xab\x65\x2c\xb1\x7a\xa5\xb2\xb6\xb7\x0d\xa5\xfd\xb3\x5d\xa9\x77\xa6\x6e\xa5\x7c\xa6\xec\xb6\xa4\xad\xfb\x21", /* "在開始遊戲以前,您必須\r先設定好四位隊員!"; */
	"\xa6\x62\xc1\xf4\xa4\xeb\xb6\xc7\xa9\x5f\xa4\xa4\x2c\xb1\x7a\xb3\xcc\xa6\x68\x0d\xa5\x75\xaf\xe0\xb6\xc7\xb0\x65\xa5\x7c\xa6\xec\xb6\xa4\xad\xfb\x21", /* "在隱月傳奇中,您最多\r只能傳送四位隊員!"; */
	"\xa6\x62\xa5\xbb\xb9\x43\xc0\xb8\xa4\xa4\xb5\x4c\xa5\xce\xa9\xce\xa4\xa3\xaf\xe0\x0d\xa8\xcf\xa5\xce\xaa\xba\xaa\xab\xab\x7e\x2c\xb3\xa3\xb7\x7c\xb3\x51\x0d\xb2\x4d\xb0\xa3\xb1\xbc\x2e", /* "在本遊戲中無用或不能\r使用的物品,都會被\r清除掉."; */
	"\x20\x0d\x20\xb1\x7a\xaa\xba\xab\x5f\xc0\x49\xb6\xa4\xa5\xee\xb8\xcc\x20\x0d\x20\xa8\x53\xa6\xb3\xa7\xc5\xae\x76\x2e", /* " \r 您的冒險隊伍裡 \r 沒有巫師."; */
	"\x20\x0d\xa6\x62\xb1\x7a\xaa\xba\xb6\xa4\xa5\xee\xa4\xa4\xa8\xc3\xa8\x53\xa6\xb3\x0d\xaa\xaa\xae\x76\xa9\xce\xaa\x5a\xa4\x68", /* " \r在您的隊伍中並沒有\r牧師或武士"; */
};

static const StringListProvider kEoB2MenuStringsTransferDOSChineseProvider = { ARRAYSIZE(kEoB2MenuStringsTransferDOSChinese), kEoB2MenuStringsTransferDOSChinese }; // Likely ok

static const char *const kEoB2MenuStringsSpecDOSChinese[2] = {
	"\xb7\xed\xb1\x7a\xa4\x40\xc4\xb1\xbf\xf4\xa8\xd3\x2c\xb1\x7a\xb5\x6f\xb2\x7b\x0d\xad\x5e\xc2\xc4\xba\xb8\xa8\xba\xad\xd3\xa4\x70\xb8\xe9\xa4\x77\xb8\x67\x0d\xc2\xf7\xb6\x7d\xa4\x46\x21", /* "當您一覺醒來,您發現\r英薩爾那個小賊已經\r離開了!"; */
	"\xb1\x7a\xa8\xb3\xb3\x74\xa6\x61\xc0\xcb\xb5\xf8\xa4\x46\xa4\x40\xa4\x55\x0d\xa9\xd2\xa6\xb3\xaa\xab\xab\x7e\x2c\xb5\x6f\xb2\x7b\xb8\xcb\xb3\xc6\xb3\xa3\x0d\xa4\xa3\xa8\xa3\xa4\x46\x21", /* "您迅速地檢視了一下\r所有物品,發現裝備都\r不見了!"; */
};

static const StringListProvider kEoB2MenuStringsSpecDOSChineseProvider = { ARRAYSIZE(kEoB2MenuStringsSpecDOSChinese), kEoB2MenuStringsSpecDOSChinese };

static const char *const kEoB2MenuYesNoStringsDOSChinese[2] = {
	"Yes",
	"No"
};

static const StringListProvider kEoB2MenuYesNoStringsDOSChineseProvider = { ARRAYSIZE(kEoB2MenuYesNoStringsDOSChinese), kEoB2MenuYesNoStringsDOSChinese };

static const char *const kEoB2CharGuiStringsHpDOSChinese[2] = {
	"HP",
	"%3d of %-3d"
};

static const StringListProvider kEoB2CharGuiStringsHpDOSChineseProvider = { ARRAYSIZE(kEoB2CharGuiStringsHpDOSChinese), kEoB2CharGuiStringsHpDOSChinese };

static const char *const kEoB2CharGuiStringsWp2DOSChinese[3] = {
	"\xa7\xf0\xc0\xbb\xa5\xa2\xbb\x7e", /* "攻擊失誤"; */
	"\xad\x4a\xb6\xc3\xac\xe5\xbc\x41", /* "胡亂砍劈"; */
	"\xb2\x72\xaf\x50\xa4\x40\xc0\xbb", /* "猛烈一擊"; */
};

static const StringListProvider kEoB2CharGuiStringsWp2DOSChineseProvider = { ARRAYSIZE(kEoB2CharGuiStringsWp2DOSChinese), kEoB2CharGuiStringsWp2DOSChinese };

static const char *const kEoB2CharGuiStringsWrDOSChinese[4] = {
	"\xb5\x4c\xaa\x6b\xa7\xf0\xc0\xbb", /* "無法攻擊"; */
	"",
	"NO",
	"AMMO"
};

static const StringListProvider kEoB2CharGuiStringsWrDOSChineseProvider = { ARRAYSIZE(kEoB2CharGuiStringsWrDOSChinese), kEoB2CharGuiStringsWrDOSChinese };

static const char *const kEoB2CharGuiStringsSt2DOSChinese[7] = {
	"Swapping",
	"\xa5\xfa\xba\x61\xb0\x7d\xa4\x60", /* "光榮陣亡"; */
	"\xa9\xfc\xb0\x67\xa4\xa3\xbf\xf4", /* "昏迷不醒"; */
	"\xba\x43\xa9\xca\xa4\xa4\xac\x72", /* "慢性中毒"; */
	"\xa8\xad\xac\x56\xbc\x40\xac\x72", /* "身染劇毒"; */
	"\xa5\xfe\xa8\xad\xb7\xf2\xb7\xf4", /* "全身痲痺"; */
	"\xc5\xdc\xa6\xa8\xa4\xc6\xa5\xdb", /* "變成化石"; */
};

static const StringListProvider kEoB2CharGuiStringsSt2DOSChineseProvider = { ARRAYSIZE(kEoB2CharGuiStringsSt2DOSChinese), kEoB2CharGuiStringsSt2DOSChinese };

static const char *const kEoB2CharGuiStringsInDOSChinese[4] = {
	"\xb6\xa4\xad\xfb\xb8\xea\xae\xc6", /* "隊員資料"; */
	"\xa8\xbe\xc5\x40\xa4\x4f\x3a", /* "防護力:"; */
	"\xb8\x67\xc5\xe7", /* "經驗"; */
	"\xb5\xa5\xaf\xc5", /* "等級"; */
};

static const StringListProvider kEoB2CharGuiStringsInDOSChineseProvider = { ARRAYSIZE(kEoB2CharGuiStringsInDOSChinese), kEoB2CharGuiStringsInDOSChinese };

static const char *const kEoB2CharStatusStrings7DOSChinese[1] = {
	"\x25\x73\xa4\xa3\xa6\x41\xa6\xb3\xa5\xa8\xa4\x6a\xaa\xba\xa4\x4f\xb6\x71\xa4\x46\x2e\r", /* "%s不再有巨大的力量了.\r"; */
};

static const StringListProvider kEoB2CharStatusStrings7DOSChineseProvider = { ARRAYSIZE(kEoB2CharStatusStrings7DOSChinese), kEoB2CharStatusStrings7DOSChinese };

static const char *const kEoB2CharStatusStrings82DOSChinese[1] = {
	"\x06\x06\x25\x73\xb7\x50\xc4\xb1\xac\x72\xa9\xca\xa5\xbf\xb3\x76\xba\xa5\xb5\x6f\xa7\x40\xa4\xa4\x21\x0d", /* "\x06\x06%s感覺毒性正逐漸發作中!\r"; */
};

static const StringListProvider kEoB2CharStatusStrings82DOSChineseProvider = { ARRAYSIZE(kEoB2CharStatusStrings82DOSChinese), kEoB2CharStatusStrings82DOSChinese };

static const char *const kEoB2CharStatusStrings9DOSChinese[1] = {
	"\x06\x04\x25\x73\xa4\x77\xa4\xa3\xa6\x41\xb7\xf2\xb7\xf4\xa4\x46\x21\r", /* "\x06\x04%s已不再痲痺了!\r"; */
};

static const StringListProvider kEoB2CharStatusStrings9DOSChineseProvider = { ARRAYSIZE(kEoB2CharStatusStrings9DOSChinese), kEoB2CharStatusStrings9DOSChinese };

static const char *const kEoB2CharStatusStrings12DOSChinese[1] = {
	"\x25\x73\xaa\xba\xb3\x74\xab\xd7\xba\x43\xa4\x55\xa8\xd3\xa4\x46\x2e\r", /* "%s的速度慢下來了.\r"; */
};

static const StringListProvider kEoB2CharStatusStrings12DOSChineseProvider = { ARRAYSIZE(kEoB2CharStatusStrings12DOSChinese), kEoB2CharStatusStrings12DOSChinese };

static const char *const kEoB2CharStatusStrings132DOSChinese[1] = {
	"\x06\x06""%s is %s!\r"
};

static const StringListProvider kEoB2CharStatusStrings132DOSChineseProvider = { ARRAYSIZE(kEoB2CharStatusStrings132DOSChinese), kEoB2CharStatusStrings132DOSChinese };

static const char *const kEoB2LevelGainStringsDOSChinese[1] = {
	"\x06\x06\x25\x73\xaa\xba\xb5\xa5\xaf\xc5\xb4\xa3\xa4\xc9\xa4\x46\xa4\x40\xaf\xc5\x2e\x06\x0f\r", /* "\x06\x06%s的等級提升了一級.\x06\x0f\r"; */
};

static const StringListProvider kEoB2LevelGainStringsDOSChineseProvider = { ARRAYSIZE(kEoB2LevelGainStringsDOSChinese), kEoB2LevelGainStringsDOSChinese };

static const char *const kEoB2BookNumbersDOSChinese[5] = {
	"\xb2\xc4\xa4\x40\xaf\xc5", /* "第一級"; */
	"\xb2\xc4\xa4\x47\xaf\xc5", /* "第二級"; */
	"\xb2\xc4\xa4\x54\xaf\xc5", /* "第三級"; */
	"\xb2\xc4\xa5\x7c\xaf\xc5", /* "第四級"; */
	"\xb2\xc4\xa4\xad\xaf\xc5", /* "第五級"; */
};

static const StringListProvider kEoB2BookNumbersDOSChineseProvider = { ARRAYSIZE(kEoB2BookNumbersDOSChinese), kEoB2BookNumbersDOSChinese };

static const char *const kEoB2MageSpellsListDOSChinese[33] = {
	"",
	"\xc5\x5d\xab\x60\xb3\x4e", /* "魔冑術"; */
	"\xbf\x55\xbf\x4e\xa4\xa7\xa4\xe2", /* "燃燒之手"; */
	"\xb0\xbb\xb4\xfa\xc5\x5d\xaa\x6b", /* "偵測魔法"; */
	"\xc5\x5d\xaa\x6b\xad\xb8\xbc\x75", /* "魔法飛彈"; */
	"\xc5\x5d\xac\xde\xb3\x4e", /* "魔盾術"; */
	"\xb9\x71\xc0\xbb\xa4\xe2\xa4\x4d", /* "電擊手刀"; */
	"\xc2\x61\xc4\x67\xa4\xc6\xa8\xad", /* "朦朧化身"; */
	"\xb0\xbb\xb4\xfa\xc1\xf4\xa7\xce", /* "偵測隱形"; */
	"\xbf\xeb\xc3\xd1\xaa\xab\xab\x7e", /* "辨識物品"; */
	"\xc1\xf4\xa7\xce\xb3\x4e", /* "隱形術"; */
	"\xbb\xc4\xb2\x47\xa4\xa7\xbd\x62", /* "酸液之箭"; */
	"\xb8\xd1\xa9\x47\xb3\x4e", /* "解咒術"; */
	"\xa4\xf5\xb2\x79\xb3\x4e", /* "火球術"; */
	"\xa8\xb3\xb1\xb6\xb3\x4e", /* "迅捷術"; */
	"\xa9\x77\xa8\xad\xb3\x4e", /* "定身術"; */
	"\xa4\x51\xa7\x60\xc1\xf4\xa7\xce", /* "十呎隱形"; */
	"\xb9\x70\xb9\x71\xb3\x4e", /* "雷電術"; */
	"\xa7\x6c\xa6\xe5\xb0\xad\xa4\xf6", /* "吸血鬼爪"; */
	"\xae\xa3\xc4\xdf\xb3\x4e", /* "恐懼術"; */
	"\xb4\x48\xa6\x42\xad\xb7\xbc\xc9", /* "寒冰風暴"; */
	"\xb1\x6a\xa4\x4f\xc1\xf4\xa7\xce", /* "強力隱形"; */
	"\xb8\xd1\xb0\xa3\xb6\x41\xa9\x47", /* "解除詛咒"; */
	"\xa7\x4e\xc0\x40\xb3\x4e", /* "冷錐術"; */
	"\xa9\xc7\xaa\xab\xa9\x77\xa8\xad", /* "怪物定身"; */
	"\xa4\x4f\xc0\xf0\xb3\x4e", /* "力牆術"; */
	"\xa4\xc0\xb8\xd1\xb3\x4e", /* "分解術"; */
	"\xa5\xdb\xa4\xc6\xb3\x4e", /* "石化術"; */
	"\xb8\xd1\xb0\xa3\xa5\xdb\xa4\xc6", /* "解除石化"; */
	"\xb3\x7a\xb5\xf8\xb3\x4e", /* "透視術"; */
	"\xa6\xba\xa4\x60\xa4\xa7\xab\xfc", /* "死亡之指"; */
	"\xb1\x6a\xa4\x4f\xc5\x5d\xa9\x47", /* "強力魔咒"; */
	"\xb6\x57\xaf\xc5\xae\xb1\xc0\x59", /* "超級拳頭"; */
};

static const StringListProvider kEoB2MageSpellsListDOSChineseProvider = { ARRAYSIZE(kEoB2MageSpellsListDOSChinese), kEoB2MageSpellsListDOSChinese };

static const char *const kEoB2ClericSpellsListDOSChinese[30] = {
	"",
	"\xaf\xac\xba\xd6\xb3\x4e", /* "祝福術"; */
	"\xbb\xb4\xab\xd7\xb6\xcb\xae\x60", /* "輕度傷害"; */
	"\xaa\x76\xc0\xf8\xbb\xb4\xb6\xcb", /* "治療輕傷"; */
	"\xb0\xbb\xb4\xfa\xc5\x5d\xaa\x6b", /* "偵測魔法"; */
	"\xa6\xca\xa8\xb8\xa4\xa3\xab\x49", /* "百邪不侵"; */
	"\xb1\x6a\xa4\xc6\xa5\xcd\xa9\x52", /* "強化生命"; */
	"\xa4\xf5\xb5\x4b\xa4\xa7\xa4\x62", /* "火焰之刃"; */
	"\xa9\x77\xa8\xad\xb3\x4e", /* "定身術"; */
	"\xb4\xee\xbd\x77\xac\x72\xa9\xca", /* "減緩毒性"; */
	"\xbb\x73\xb3\x79\xad\xb9\xaa\xab", /* "製造食物"; */
	"\xb8\xd1\xa9\x47\xb3\x4e", /* "解咒術"; */
	"\xc5\x5d\xaa\x6b\xa4\xa7\xb3\x54", /* "魔法之袍"; */
	"\xac\xe8\xc3\xab\xb3\x4e", /* "祈禱術"; */
	"\xb8\xd1\xb0\xa3\xb7\xf2\xde\xcd", /* "解除痲痹"; */
	"\xad\xab\xab\xd7\xb6\xcb\xae\x60", /* "重度傷害"; */
	"\xaa\x76\xc0\xf8\xad\xab\xb6\xcb", /* "治療重傷"; */
	"\xa4\xa4\xa9\x4d\xac\x72\xaf\xc0", /* "中和毒素"; */
	"\xb9\x40\xa8\xb8\xa4\x51\xa7\x60", /* "辟邪十呎"; */
	"\xad\x50\xa9\x52\xb6\xcb\xae\x60", /* "致命傷害"; */
	"\xaa\x76\xc0\xf8\xad\x50\xa9\x52", /* "治療致命"; */
	"\xa4\xd1\xa4\xf5\xb5\x49\xa8\xad", /* "天火焚身"; */
	"\xb4\x5f\xac\xa1\xb3\x4e", /* "復活術"; */
	"\xb9\xdc\xa9\x52\xb3\x4e", /* "奪命術"; */
	"\xb3\x7a\xb5\xf8\xb3\x4e", /* "透視術"; */
	"\xb6\xcb\xae\x60\xb3\x4e", /* "傷害術"; */
	"\xa7\xb9\xa5\xfe\xc2\xe5\xc0\xf8", /* "完全醫療"; */
	"\xa6\x41\xb3\x79\xa5\xcd\xa9\x52", /* "再造生命"; */
	"\xb6\xc7\xa5\x5c\xa4\x6a\xaa\x6b", /* "傳功大法"; */
	"\xb6\x57\xb4\xe7\xa4\x60\xbb\xee", /* "超渡亡魂"; */
};

static const StringListProvider kEoB2ClericSpellsListDOSChineseProvider = { ARRAYSIZE(kEoB2ClericSpellsListDOSChinese), kEoB2ClericSpellsListDOSChinese };

static const char *const kEoB2SpellNamesDOSChinese[68] = {
	"\xc5\x5d\xab\x60\xb3\x4e", /* "魔冑術"; "armor" */
	"\xbf\x55\xbf\x4e\xa4\xa7\xa4\xe2", /* "燃燒之手"; "burning hands" */
	"\xb0\xbb\xb4\xfa\xc5\x5d\xaa\x6b", /* "偵測魔法"; "detect magic" */
	"\xc5\x5d\xaa\x6b\xad\xb8\xbc\x75", /* "魔法飛彈"; "magic missile" */
	"\xc5\x5d\xac\xde\xb3\x4e", /* "魔盾術"; "shield" */
	"\xb9\x71\xc0\xbb\xa4\xe2\xa4\x4d", /* "電擊手刀"; "shocking grasp" */
	"\xc2\x61\xc4\x67\xa4\xc6\xa8\xad", /* "朦朧化身"; "blur" */
	"\xb0\xbb\xb4\xfa\xc1\xf4\xa7\xce", /* "偵測隱形"; "detect invisibility" */
	"\xbf\xeb\xc3\xd1\xaa\xab\xab\x7e", /* "辨識物品"; "improved identify" */
	"\xc1\xf4\xa7\xce\xb3\x4e", /* "隱形術"; "invisibility" */
	"\xbb\xc4\xb2\x47\xa4\xa7\xbd\x62", /* "酸液之箭"; "melf's acid arrow" */
	"\xb8\xd1\xa9\x47\xb3\x4e", /* "解咒術"; "dispel magic" */
	"\xa4\xf5\xb2\x79\xb3\x4e", /* "火球術"; "fireball" */
	"\xa8\xb3\xb1\xb6\xb3\x4e", /* "迅捷術"; "haste" */
	"\xa9\x77\xa8\xad\xb3\x4e", /* "定身術"; "Hold Person" */
	"\x31\x30\xa7\x60\xc1\xf4\xa7\xce", /* "10呎隱形"; "invisibility 10' radius" */
	"\xb9\x70\xb9\x71\xb3\x4e", /* "雷電術"; "lightning bolt" */
	"\xa7\x6c\xa6\xe5\xb0\xad\xa4\xf6", /* "吸血鬼爪"; "vampiric touch" */
	"\xae\xa3\xc4\xdf\xb3\x4e", /* "恐懼術"; "fear" */
	"\xb4\x48\xa6\x42\xad\xb7\xbc\xc9", /* "寒冰風暴"; "ice storm" */
	"\xb1\x6a\xa4\x4f\xc1\xf4\xa7\xce", /* "強力隱形"; "improved invisibility" */
	"\xb8\xd1\xb0\xa3\xb6\x41\xa9\x47", /* "解除詛咒"; "remove curse" */
	"\xa7\x4e\xc0\x40\xb3\x4e", /* "冷錐術"; "cone of cold" */
	"\xa9\xc7\xaa\xab\xa9\x77\xa8\xad", /* "怪物定身"; "hold monster" */
	"\xa4\x4f\xc0\xf0\xb3\x4e", /* "力牆術"; "wall of force" */
	"\xa4\xc0\xb8\xd1\xb3\x4e", /* "分解術"; "disintegrate" */
	"\xa5\xdb\xa4\xc6\xb3\x4e", /* "石化術"; "flesh to stone" */
	"\xb8\xd1\xb0\xa3\xa5\xdb\xa4\xc6", /* "解除石化"; "stone to flesh" */
	"\xb3\x7a\xb5\xf8\xb3\x4e", /* "透視術"; "true seeing" */
	"\xa6\xba\xa4\x60\xa4\xa7\xab\xfc", /* "死亡之指"; "finger of death" */
	"\xb1\x6a\xa4\x4f\xc5\x5d\xa9\x47", /* "強力魔咒"; "power word stun" */
	"\xb6\x57\xaf\xc5\xad\xab\xae\xb1", /* "超級重拳"; "bigby's clenched fist" */
	"\xaf\xac\xba\xd6\xb3\x4e", /* "祝福術";  "bless"*/
	"\xbb\xb4\xab\xd7\xb6\xcb\xae\x60", /* "輕度傷害"; "cause light wounds" */
	"\xaa\x76\xc0\xf8\xbb\xb4\xb6\xcb", /* "治療輕傷"; "cure light wounds" */
	"\xb0\xbb\xb4\xfa\xc5\x5d\xaa\x6b", /* "偵測魔法"; "detect magic" */
	"\xa6\xca\xa8\xb8\xa4\xa3\xab\x49", /* "百邪不侵"; "protection from evil" */
	"\xb1\x6a\xa4\xc6\xa5\xcd\xa9\x52", /* "強化生命"; "aid" */
	"\xa4\xf5\xb5\x4b\xa4\xa7\xa4\x62", /* "火焰之刃"; "flame blade" */
	"\xa9\x77\xa8\xad\xb3\x4e", /* "定身術"; "hold person" */
	"\xb4\xee\xbd\x77\xac\x72\xa9\xca", /* "減緩毒性"; "slow poison" */
	"\xb3\x79\xad\xb9\xaa\xab\xa9\x4d\xa4\xf4", /* "造食物和水"; "create food" */
	"\xb8\xd1\xa9\x47\xb3\x4e", /* "解咒術"; "dispel magic" */
	"\xc5\x5d\xaa\x6b\xa4\xa7\xb3\x54", /* "魔法之袍"; "magical vestment" */
	"\xac\xe8\xc3\xab\xb3\x4e", /* "祈禱術"; "prayer" */
	"\xb8\xd1\xb0\xa3\xb7\xf2\xde\xcd", /* "解除痲痹"; "remove paralysis" */
	"\xad\xab\xab\xd7\xb6\xcb\xae\x60", /* "重度傷害"; "cause serious wounds" */
	"\xaa\x76\xc0\xf8\xad\xab\xb6\xcb", /* "治療重傷"; "cure serious wounds" */
	"\xa4\xa4\xa9\x4d\xac\x72\xaf\xc0", /* "中和毒素"; "neutralize poison" */
	"\xb9\x40\xa8\xb8\xa4\x51\xa7\x60", /* "辟邪十呎"; "protection from evil 10' radius" */
	"\xad\x50\xa9\x52\xb6\xcb\xae\x60", /* "致命傷害"; "cause critical wounds" */
	"\xaa\x76\xc0\xf8\xad\x50\xa9\x52", /* "治療致命"; "cure critical wounds" */
	"\xa4\xd1\xa4\xf5\xb5\x49\xa8\xad", /* "天火焚身"; "flame strike" */
	"\xb4\x5f\xac\xa1\xb3\x4e", /* "復活術"; "raise dead" */
	"\xb9\xdc\xa9\x52\xb3\x4e", /* "奪命術"; "slay living" */
	"\xb3\x7a\xb5\xf8\xb3\x4e", /* "透視術"; "true seeing" */
	"\xb6\xcb\xae\x60\xb3\x4e", /* "傷害術"; "harm" */
	"\xa7\xb9\xa5\xfe\xc2\xe5\xc0\xf8", /* "完全醫療"; "heal" */
	"\xa6\x41\xb3\x79\xa5\xcd\xa9\x52", /* "再造生命"; "ressurection" */
	"\xb6\xc7\xa5\xa4\x6a\xaa\x6b", /* "傳奶j法"; "lay on hands" */
	"\xb6\x57\xb4\xe7\xa4\x60\xbb\xee", /* "超渡亡魂"; "turn undead" */
	"",
	"\xc0\x73\xae\xf0\xc5\x40\xb8\x6e", /* "龍氣護罩"; "mystic defense" */
	"",
	"",
	"",
	"",
	""
};

static const StringListProvider kEoB2SpellNamesDOSChineseProvider = { ARRAYSIZE(kEoB2SpellNamesDOSChinese), kEoB2SpellNamesDOSChinese };

static const char *const kEoB2MagicStrings1DOSChinese[6] = {
	"\xa9\xf1\xb1\xf3", /* "放棄"; */
	"\xa9\xf1\xb1\xf3", /* "放棄"; */
	"\xb1\x7a\xa5\xb2\xb6\xb7\xaa\xc5\xb5\xdb\xa4\x40\xb0\xa6\xa4\xe2\x2c\xa4\x7e\xaf\xe0\xac\x49\xae\x69\xb3\x6f\xb6\xb5\xaa\x6b\xb3\x4e\x2e", /* "您必須空著一隻手,才能施展這項法術."; */
	"\xb1\x7a\xa6\xdb\xa4\x76\xa4\xa3\xaf\xe0\xa6\x50\xae\xc9\xa8\xcf\xa5\xce\xb3\x6f\xaa\x6b\xb3\x4e\xa8\xe2\xa6\xb8\x2e", /* "您自己不能同時使用這法術兩次."; */
	"%s\xac\x49\xae\x69\xa4\x46%s\x2e", /* "%s施展了%s."; */
	"\xac\x49\xaa\x6b\xa7\xb9\xb2\xa6", /* "施法完畢"; */
};

static const StringListProvider kEoB2MagicStrings1DOSChineseProvider = { ARRAYSIZE(kEoB2MagicStrings1DOSChinese), kEoB2MagicStrings1DOSChinese };

static const char *const kEoB2MagicStrings2DOSChinese[5] = {
	"\xac\x49\xaa\x6b\xa5\xa2\xb1\xd1\x21\r", /* "施法失敗!\r"; */
	"\x25\x73\xa4\x77\xb8\x67\xb3\x51\xa4\xc0\xb8\xd1\xa4\x46\x21\r", /* "%s已經被分解了!\r"; */
	"\x06\x06\xb6\xa4\xa5\xee\xa4\x77\xb3\x51\xa6\xba\xa4\x60\xa4\xa7\xb3\x4e\xa9\xd2\xc0\xbb\xa4\xa4\x21\r", /* "\x06\x06隊伍已被死亡之術所擊中!\r"; */
	"\x06\x06\x25\x73\xa4\x77\xb8\x67\xb3\x51\xad\xab\xab\xd7\xb6\xcb\xae\x60\xaa\x6b\xb3\x4e\xa9\xd2\xb6\xcb\x2e\r", /* "\x06\x06%s已經被重度傷害法術所傷.\r"; */
	"\xb3\x51\xa5\xdb\xa4\xc6", /* "被石化"; */
};

static const StringListProvider kEoB2MagicStrings2DOSChineseProvider = { ARRAYSIZE(kEoB2MagicStrings2DOSChinese), kEoB2MagicStrings2DOSChinese };

static const char *const kEoB2MagicStrings3DOSChinese[5] = {
	"\xa6\x56\xa8\xba\xa4\x40\xad\xd3\xb6\xa4\xad\xfb\xac\x49\xae\x69\xa6\xb9\xb6\xb5\xaa\x6b\xb3\x4e\x3f", /* "向那一個隊員施展此項法術?"; */
	"\r\xa9\xf1\xb1\xf3\xac\x49\xae\x69\xaa\x6b\xb3\x4e\x2e\r", /* "\r放棄施展法術.\r"; */
	"\x25\x73\xaa\xba\x25\x73\xae\xc4\xa4\x4f\xae\xc9\xb6\xa1\xa4\x77\xb9\x4c\x21\r", /* "%s的%s效力時間已過!\r"; */
	"\x25\x73\xa8\x53\xa6\xb3\xc0\xbb\xa4\xa4\xb3\x6f\xa9\xc7\xaa\xab\x2e\r", /* "%s沒有擊中這怪物.\r"; */
	"\x25\x73\xbb\xdd\xad\x6e\xa6\xb3\xb8\xfb\xb0\xaa\xaa\xba\xb5\xa5\xaf\xc5\xa4\x7e\xaf\xe0\xc0\xbb\xa4\xa4\xa5\xd8\xbc\xd0\x21\r", /* "%s需要有較高的等級才能擊中目標!\r"; */
};

static const StringListProvider kEoB2MagicStrings3DOSChineseProvider = { ARRAYSIZE(kEoB2MagicStrings3DOSChinese), kEoB2MagicStrings3DOSChinese };

static const char *const kEoB2MagicStrings4DOSChinese[1] = {
	"\xab\x7a\x21\xa8\x53\xa6\xb3\xa5\x5c\xae\xc4\x21\x0d", /* "哇!沒有功效!\r"; */
};

static const StringListProvider kEoB2MagicStrings4DOSChineseProvider = { ARRAYSIZE(kEoB2MagicStrings4DOSChinese), kEoB2MagicStrings4DOSChinese };

static const char *const kEoB2MagicStrings6DOSChinese[1] = {
	"\x25\x73\xa4\x77\xa6\xb3\xa4\x40\xad\xd3\xc5\x40\xa5\xd2\xaa\xba\xa8\xbe\xbf\x6d\xb2\x76\xb6\x57\xb9\x4c\x20\x36\x20\xc2\x49\x2e", /* "%s已有一個護甲的防禦率超過 6 點."; */
};

static const StringListProvider kEoB2MagicStrings6DOSChineseProvider = { ARRAYSIZE(kEoB2MagicStrings6DOSChinese), kEoB2MagicStrings6DOSChinese };

static const char *const kEoB2MagicStrings7DOSChinese[2] = {
	"\x25\x73\xa4\x77\xa6\x62\xb3\x6f\x25\x73\xaa\xba\xae\xc4\xa4\x4f\xa4\xa7\xa4\xba\x2e\r", /* "%s已在這%s的效力之內.\r"; */
	"\xbe\xe3\xa4\xe4\xb6\xa4\xa5\xee\xa4\x77\xb8\x67\xa6\x62\xb3\x6f\x25\x73\xaa\xba\xae\xc4\xa4\x4f\xa4\xa7\xa4\xba\x2e\r", /* "整支隊伍已經在這%s的效力之內.\r"; */
};

static const StringListProvider kEoB2MagicStrings7DOSChineseProvider = { ARRAYSIZE(kEoB2MagicStrings7DOSChinese), kEoB2MagicStrings7DOSChinese };

static const char *const kEoB2MagicStrings8DOSChinese[6] = {
	"\xac\x49\xaa\x6b\xa5\xa2\xb1\xd1\x21\r", /* "施法失敗!\r"; */
	"\xb3\x51\xb7\xf2\xb7\xf4", /* "被痲痺"; */
	"\xac\x49\xaa\x6b\xa5\xa2\xb1\xd1\x21\r", /* "施法失敗!\r"; */
	"\xac\x49\xaa\x6b\xa5\xa2\xb1\xd1\x21\r", /* "施法失敗!\r"; */
	"\xa8\x53\xa6\xb3\xa9\xc7\xaa\xab\xa9\xce\xbc\xc4\xa4\x48\xb1\xb5\xaa\xf1\x21\r", /* "沒有怪物或敵人接近!\r"; */
	"\x25\x73\xa4\x77\xa6\x62\xb1\x6a\xa4\xc6\xa5\xcd\xa9\x52\xb3\x4e\xaa\xba\xae\xc4\xa4\x4f\xbd\x64\xb3\xf2\xa4\xa7\xa4\xba\x2e\r", /* "%s已在強化生命術的效力範圍之內.\r"; */
};

static const StringListProvider kEoB2MagicStrings8DOSChineseProvider = { ARRAYSIZE(kEoB2MagicStrings8DOSChinese), kEoB2MagicStrings8DOSChinese };

static const byte kEoB2ManDefDOSChinese[200] = {
	0x09, 0x03, 0x04, 0x00, 0x09, 0x0D, 0x02, 0x00,
	0x09, 0x07, 0x07, 0x00, 0x14, 0x06, 0x05, 0x00,
	0x14, 0x10, 0x06, 0x00, 0x14, 0x1C, 0x03, 0x00,
	0x1A, 0x04, 0x06, 0x00, 0x1A, 0x0A, 0x05, 0x00,
	0x1A, 0x0C, 0x05, 0x00, 0x15, 0x03, 0x04, 0x00,
	0x15, 0x05, 0x05, 0x00, 0x15, 0x0F, 0x02, 0x00,
	0x15, 0x14, 0x03, 0x00, 0x15, 0x1B, 0x02, 0x00,
	0x15, 0x1D, 0x03, 0x00, 0x23, 0x01, 0x03, 0x00,
	0x23, 0x03, 0x03, 0x00, 0x23, 0x08, 0x03, 0x00,
	0x23, 0x19, 0x02, 0x00, 0x23, 0x1F, 0x04, 0x00,
	0x23, 0x26, 0x04, 0x00, 0x26, 0x03, 0x02, 0x00,
	0x26, 0x05, 0x06, 0x00, 0x26, 0x12, 0x03, 0x00,
	0x26, 0x18, 0x01, 0x00, 0x26, 0x1E, 0x01, 0x00,
	0x26, 0x21, 0x04, 0x00, 0x17, 0x01, 0x03, 0x00,
	0x17, 0x03, 0x06, 0x00, 0x17, 0x06, 0x02, 0x00,
	0x17, 0x0F, 0x05, 0x00, 0x17, 0x1B, 0x01, 0x00,
	0x17, 0x21, 0x06, 0x00, 0x12, 0x03, 0x02, 0x00,
	0x12, 0x05, 0x04, 0x00, 0x12, 0x09, 0x02, 0x00,
	0x12, 0x0B, 0x04, 0x00, 0x12, 0x0D, 0x06, 0x00,
	0x12, 0x0F, 0x03, 0x00, 0x12, 0x11, 0x05, 0x00,
	0x12, 0x12, 0x02, 0x00, 0x1F, 0x02, 0x04, 0x00,
	0x1F, 0x02, 0x07, 0x00, 0x1F, 0x04, 0x03, 0x00,
	0x1F, 0x06, 0x03, 0x00, 0x1F, 0x09, 0x03, 0x00,
	0x1F, 0x0A, 0x01, 0x00, 0x1C, 0x03, 0x03, 0x00,
	0x1C, 0x04, 0x02, 0x00, 0x1C, 0x05, 0x06, 0x00
};

static const ByteProvider kEoB2ManDefDOSChineseProvider = { ARRAYSIZE(kEoB2ManDefDOSChinese), kEoB2ManDefDOSChinese };

static const char *const kEoB2ManWordDOSChinese[51] = {
	"cursor",
	"majority",
	"right",
	"unusable",
	"greyed",
	"thrown",
	"spellcasters",
	"button",
	"characters",
	"carefree",
	"practical",
	"inherit",
	"while",
	"combinations",
	"charm",
	"individuals",
	"gestures",
	"pummel",
	"paladins",
	"skill",
	"advancement",
	"also",
	"counters",
	"knowledge",
	"greater",
	"assume",
	"several",
	"laying",
	"reach",
	"swiftly",
	"allows",
	"limited",
	"jack",
	"thrown",
	"weapons",
	"note",
	"certain",
	"damage",
	"done",
	"bonus",
	"may",
	"over",
	"box",
	"put",
	"portrait",
	"backpack",
	"inside",
	"causes",
	"until",
	"outright",
	""
};

static const StringListProvider kEoB2ManWordDOSChineseProvider = { ARRAYSIZE(kEoB2ManWordDOSChinese), kEoB2ManWordDOSChinese };

static const char *const kEoB2ManPromptDOSChinese[1] = {
	"\r\r\r\rOn the page with this symbol...\r\rFind line %d\rEnter word %d\r"
};

static const StringListProvider kEoB2ManPromptDOSChineseProvider = { ARRAYSIZE(kEoB2ManPromptDOSChinese), kEoB2ManPromptDOSChinese };

static const char *const kEoB2MainMenuStringsDOSChinese[5] = {
	"\xb8\xfc\x20\xa4\x4a\x20\xc2\xc2\x20\xc0\xc9", /* "載 入 舊 檔"; */
	"\xad\xab\xb7\x73\xab\xd8\xa5\xdf\xb7\x73\xb6\xa4\xad\xfb", /* "重新建立新隊員"; */
	"\xc2\xe0\xb4\xab\xb2\xc4\xa4\x40\xa5\x4e\xaa\xba\xa4\x48\xaa\xab\xb8\xea\xae\xc6", /* "轉換第一代的人物資料"; */
	"\xc6\x5b\xac\xdd\xa4\xf9\xc0\x59\xac\x47\xa8\xc6\xa4\xb6\xb2\xd0", /* "觀看片頭故事介紹"; */
	"\xb5\xb2\x20\xa7\xf4\x20\xb9\x43\x20\xc0\xb8", /* "結 束 遊 戲"; */
};

static const StringListProvider kEoB2MainMenuStringsDOSChineseProvider = { ARRAYSIZE(kEoB2MainMenuStringsDOSChinese), kEoB2MainMenuStringsDOSChinese };

static const char *const kEoB2TransferStrings1DOSChinese[2] = {
	"\xb5\xa5\xaf\xc5\x3a\x25\x64", /* "等級:%d"; */
	" / %d"
};

static const StringListProvider kEoB2TransferStrings1DOSChineseProvider = { ARRAYSIZE(kEoB2TransferStrings1DOSChinese), kEoB2TransferStrings1DOSChinese };

static const char *const kEoB2TransferStrings2DOSChinese[2] = {
	"\xa6\x62\xb1\x7a\xaa\xba\xb6\xa4\xa5\xee\xa4\xa4\x2c\xb1\x7a\xb3\xcc\xa6\x68\xa5\x75\xaf\xe0\xbf\xef\xa5\x7c\xa6\xec\xa4\x48\xaa\xab\xc2\xe0\xb4\xab", /* ",您最多只能選四位人物轉換"; */
	"Select OK when you are finished choosing your party."
};

static const StringListProvider kEoB2TransferStrings2DOSChineseProvider = { ARRAYSIZE(kEoB2TransferStrings2DOSChinese), kEoB2TransferStrings2DOSChinese };

static const char *const kEoB2TransferLabelsDOSChinese[2] = {
	"CANCEL",
	"OK"
};

static const StringListProvider kEoB2TransferLabelsDOSChineseProvider = { ARRAYSIZE(kEoB2TransferLabelsDOSChinese), kEoB2TransferLabelsDOSChinese };

static const char *const kEoB2IntroStringsDOSChinese[20] = {
	"\xa6\x62\xa4\x40\xad\xd3\xad\xb7\xab\x42\xa5\xe6\xa5\x5b\xaa\xba\xb2\x4d\xb1\xe1", /* "在一個風雨交加的清晨"; "In a storm early morning" */
	"\xa7\x41\xa6\xac\xa8\xec\xa4\x46\xa4\x40\xb1\x69\xab\x4b\xa8\xe7", /* "你收到了一張便函"; "You have received a note" */
	"\xa5\xa6\xac\x4f\xa7\x41\xaa\xba\xa6\x6e\xa4\xcd\r\xc4\xb3\xaa\xf8\xab\xb4\xba\xb8\xaf\x5a\xbc\x67\xa8\xd3\xaa\xba.", /* "它是你的好友\r議長契爾班寫來的."; "It's from your friend Chancellor Khelben." */
	"\xab\x4b\xa8\xe7\xa4\x57\xaa\xba\xa4\xba\xae\x65\xa8\xc3\xa4\xa3\xac\x4f\xab\xdc\xb2\x4d\xb7\xa1,\r\xa6\xfd\xac\x4f\xa6\xfc\xa5\x47\xac\xdb\xb7\xed\xba\xf2\xab\xe6.", /* "便函上的內容並不是很清楚,\r但是似乎相當緊急."; "The content on the note is not very clear, but seems fairly urgent." */
	"\xab\xb4\xba\xb8\xaf\x5a\xad\x6e\xb0\xb5\xa4\xb0\xbb\xf2\xa9\x4f\x3f", /* "契爾班要做什麼呢?"; "What is Khelben going to do?" */
	"\xc5\x77\xaa\xef,\xbd\xd0\xb6\x69.", /* "歡迎,請進."; "Welcome, please come in." */
	"\xab\xb4\xba\xb8\xaf\x5a\xa5\x44\xa4\x48\xa4\x77\xb8\x67\xa6\x62\xa5\x4c\xaa\xba\xae\xd1\xa9\xd0\xb5\xa5\xb1\x7a\xa4\x46.", /* "契爾班主人已經在他的書房等您了."; "Master Khelben is already waiting for you in his study." */
	"\xc1\xc2\xc1\xc2\xa7\x41\xb3\x6f\xbb\xf2\xa7\xd6\xaa\xba\xbb\xb0\xa8\xd3.", /* "謝謝你這麼快的趕來."; "Thank you for rushing. Come." */
	"\xa6\x6e\xa5\x53\xa7\xcc,\xa7\xda\xb2\x7b\xa6\x62\xa6\xb3\xa4\x46\xb3\xc2\xb7\xd0.", /* "好兄弟,我現在有了麻煩."; "Dude, I'm in trouble now." */
	"\xbb\xb7\xa5\x6a\xaa\xba\xa7\xaf\xc5\x5d\xa4\x77\xb8\x67\xa4\x4a\xab\x49\xa4\x46\xc1\xf4\xa4\xeb\xaf\xab\xbc\x71.", /* "遠古的妖魔已經入侵了隱月神廟."; "Ancient demons have invaded the Temple of the Hidden Moon." */
	"\xa7\xda\xb1\x6a\xaf\x50\xaa\xba\xb7\x50\xc4\xb1\xa8\xec,\r\xa5\xbb\xab\xb0\xa5\xab\xaa\xba\xa6\x77\xa5\xfe\xa4\x77\xa6\xb3\xab\xdc\xa4\x6a\xaa\xba\xab\xc2\xaf\xd9.", /* "我強烈的感覺到,\r本城市的安全已有很大的威脅."; "I strongly feel that  the security of this city has been seriously threatened."; */
	"\xa7\xda\xbb\xdd\xad\x6e\xa7\x41\xaa\xba\xc0\xb0\xa6\xa3.", /* "我需要你的幫忙."; "I need your help." */
	"\xa4\x54\xa4\xd1\xab\x65,\xa7\xda\xac\xa3\xa4\x46\xa5\x7c\xad\xd3\xa4\x48\xab\x65\xa5\x68\xbd\xd5\xac\x64.", /* "三天前,我派了四個人前去調查."; "Three days ago, I sent four people to investigate." */
	"\xa6\xfd\xac\x4f\xa6\xdc\xa4\xb5\xa6\x6f\xad\xcc\xad\xb5\xab\x48\xa5\xfe\xb5\x4c.", /* "但是至今她們音信全無."; "but so far They haven't heard from them at all." */
	"\xa7\xda\xb7\x50\xc4\xb1\xa8\xec\xa6\x6f\xad\xcc\xa5\xbf\xa8\xad\xb3\xb4\xc0\x49\xb9\xd2\xa4\xa4.", /* "我感覺到她們正身陷險境中."; "I feel they are in danger." */
	"\xae\xb3\xb5\xdb\xb3\x6f\xad\xd3\xaa\xf7\xb9\xf4.", /* "拿著這個金幣."; Hold this coin."; */
	"\xa7\xda\xb1\x4e\xa8\xcf\xa5\xce\xa5\xa6\xbb\x50\xa7\x41\xc1\x70\xb5\xb8.", /* "我將使用它與你聯絡."; "I will use it with you connection."; */
	"\xa8\xc6\xa4\xa3\xa9\x79\xbf\xf0,\xa7\x41\xa5\xb2\xb6\xb7\xbb\xb0\xa7\xd6\xb0\xca\xa8\xad.", /* "事不宜遲,你必須趕快動身."; "Without further delay, you must leave quickly."; */
	"\xa7\xda\xb2\x7b\xa6\x62\xb1\x4e\xa7\x41\xad\xcc\xb6\xc7\xb0\x65\xa8\xec\xb1\xb5\xaa\xf1\xaf\xab\xbc\x71\xaa\xba\xaa\xfe\xaa\xf1.", /* "我現在將你們傳送到接近神廟的附近."; "I will now teleport you closer to the temple." */
	"\xc4\x40\xa6\x6e\xb9\x42\xa6\xf1\xc0\x48\xb5\xdb\xa7\x41\xad\xcc.", /* "願好運伴隨著你們."; "May good luck be with you."; */
};

static const StringListProvider kEoB2IntroStringsDOSChineseProvider = { ARRAYSIZE(kEoB2IntroStringsDOSChinese), kEoB2IntroStringsDOSChinese }; // OK

static const char *const kEoB2IntroCPSFilesDOSChinese[13] = {
	"STREET1.CPS",
	"STREET2.CPS",
	"DOORWAY1.CPS",
	"DOORWAY2.CPS",
	"WESTWOOD.CPS",
	"WINDING.CPS",
	"KHELBAN2.CPS",
	"KHELBAN1.CPS",
	"KHELBAN3.CPS",
	"KHELBAN4.CPS",
	"COIN.CPS",
	"KHELBAN5.CPS",
	"KHELBAN6.CPS"
};

static const StringListProvider kEoB2IntroCPSFilesDOSChineseProvider = { ARRAYSIZE(kEoB2IntroCPSFilesDOSChinese), kEoB2IntroCPSFilesDOSChinese }; // OK

static const char *const kEoB2FinaleStringsDOSChinese[20] = {
	"\xb3\xcc\xab\xe1\xb3\xd3\xa7\x51\xaa\xba\xae\xc9\xa8\xe8\xb2\xd7\xa9\xf3\xa8\xd3\xa8\xec\x2c\xb1\x4d\xae\xa6\xb3\x51\xc0\xbb\xb1\xd1\xa4\x46\x2e", /* "最後勝利的時刻終於來到,專恩被擊敗了."; "The moment of final victory has finally come, Boen is defeated." */
	"\xac\xf0\xb5\x4d\x2c\xb1\x7a\xaa\xba\xa6\x6e\xa4\xcd\xab\xb4\xba\xb8\xaf\x5a\xa5\x58\xb2\x7b\xa4\x46\x2e", /* "突然,您的好友契爾班出現了."; "Suddenly, your friend Khelben appeared." */
	"\xae\xa5\xb3\xdf\xa7\x41\x2c\xa7\xda\xc0\xf2\xb1\x6f\xb3\xd3\xa7\x51\xaa\xba\xa6\xd1\xa4\xcd\x2e", /* "恭喜你,我獲得勝利的老友."; "Congratulations, my victorious old friend." */
	"\xa7\x41\xa4\x77\xb8\x67\xc0\xbb\xb1\xd1\xa4\x46\xc5\x5d\xc0\x73\xb1\x4d\xae\xa6\x21", /* "你已經擊敗了魔龍專恩!"; "You have Defeated the dragon Boen!" */
	"\xa7\xda\xa8\xc3\xa4\xa3\xaa\xbe\xb9\x44\x2c\xb1\x4d\xae\xa6\xad\xec\xa8\xd3\xac\x4f\xa4\x40\xb1\xf8\xb4\x63\xc0\x73\x2e", /* "我並不知道,專恩原來是一條惡龍."; "I don't know, Zhuanen turned out to be a dragon." */
	"\xa8\x65\xa4\x77\xb8\x67\xa5\xcd\xa6\x73\xa6\xb3\xb6\x57\xb9\x4c\x33\x30\x30\xa6\x7e\xa4\x46\x21", /* "牠已經生存有超過300年了!"; "It's been around for over 300 years!" */
	"\xa8\x65\xaa\xba\xa4\x4f\xb6\x71\xa4\x77\xc0\x48\xad\xb7\xa6\xd3\xa5\x68\x2e", /* "牠的力量已隨風而去."; "Its power has gone with the wind."; */
	"\xa6\xfd\xc1\xf4\xa4\xeb\xaf\xab\xbc\x71\xa8\xcc\xc2\xc2\xac\x4f\xa5\xa8\xa4\x6a\xa8\xb8\xb4\x63\xa4\x4f\xb6\x71\xaa\xba\xb7\xbd\xc0\x59\x2e", /* "但隱月神廟依舊是巨大邪惡力量的源頭."; "But the Darkmoon temple is still the source of great evil power." */
	"\xa8\xc3\xa5\x42\xa8\x65\xaa\xba\xb3\x5c\xa6\x68\xa4\xf6\xa4\xfa\xa8\xcc\xb5\x4d\xa6\x73\xa6\x62\x2e", /* "並且牠的許多爪牙依然存在."; "And many of its minions still exist." */
	"\xb2\x7b\xa6\x62\xa7\xda\xad\xcc\xa5\xb2\xb6\xb7\xbb\xb0\xa7\xd6\xc2\xf7\xb6\x7d\xa6\xb9\xb3\x42\x2e", /* "現在我們必須趕快離開此處."; "We must get out of here now." */
	"\xb3\x6f\xbc\xcb\xa7\xda\xa4\x7e\xaf\xe0\xb1\x4e\xa6\xb9\xbc\x71\xba\x52\xb7\xb4\xa6\x69\xac\xb0\xa5\xad\xa6\x61\x2e", /* "這樣我才能將此廟摧毀夷為平地."; "So I can destroy this temple and raze it to the ground." */
	"\xb8\xf2\xa7\xda\xa8\xd3\x2e", /* "跟我來."; "Follow me." */
	"\xb1\x6a\xa4\x6a\xaa\xba\xaa\x6b\xa4\x4f\xa4\x77\xb7\xc7\xb3\xc6\xa6\x6e\xb9\xef\xaf\xab\xbc\x71\xb0\xb5\xb7\xb4\xb7\xc0\xa9\xca\xa7\xf0\xc0\xbb\x2e", /* "強大的法力已準備好對神廟做毀滅性攻擊."; "The powerful mana is ready to do a devastating attack on the temple." */
	"\xaf\xab\xbc\x71\xaa\xba\xc5\x5d\xa4\x4f\xaf\x75\xac\x4f\xab\x44\xb1\x60\xaa\xba\xb1\x6a\xae\xab\x2e", /* "神廟的魔力真是非常的強悍."; "The magic of the temple is really powerful." */
	"\xa5\xa6\xb5\xb4\xa4\xa3\xa5\x69\xa5\x48\xc4\x7e\xc4\xf2\xa6\x73\xa6\x62\x21", /* "它絕不可以繼續存在!"; "It must not continue to exist !"; */
	"\xaf\xab\xbc\x71\xb2\xd7\xa9\xf3\xa4\xc6\xac\xb0\xaf\x51\xa6\xb3\xa4\x46\x2e", /* "神廟終於化為烏有了."; "The temple finally disappeared up." */
	"\xa7\xda\xaa\xba\xa6\x6e\xaa\x42\xa4\xcd\x2c\xa7\xda\xad\xcc\xaa\xba\xa5\xf4\xb0\xc8\xa4\x77\xa7\xb9\xa6\xa8\xa4\x46\x2e", /* "我的好朋友,我們的任務已完成了."; "My good friend, our task has been completed." */
	"\xc1\xc2\xc1\xc2\xa7\x41\x2e", /* "謝謝你."; "Thank you." */
	"\xa7\x41\xb1\x4e\xc0\xf2\xb1\x6f\xa7\xda\xad\xcc\xb3\xcc\xb2\x60\xaa\xba\xb7\x71\xb7\x4e\xa9\x4d\xb4\x4c\xb1\x52\x2e", /* "你將獲得我們最深的敬意和尊崇."; "You will receive our deepest respect and honor." */
	"\xa7\xda\xad\xcc\xb1\x4e\xa5\xc3\xbb\xb7\xb0\x4f\xb1\x6f\xa7\x41\xad\x5e\xab\x69\xa5\xc7\xc3\xf8\xaa\xba\xa8\xc6\xc2\xdd\x2e", /* "我們將永遠記得你英勇犯難的事蹟."; "We will always remember your heroic deeds." */
};

static const StringListProvider kEoB2FinaleStringsDOSChineseProvider = { ARRAYSIZE(kEoB2FinaleStringsDOSChinese), kEoB2FinaleStringsDOSChinese }; // Likely ok, play finale to check

static const char *const kEoB2FinaleCPSFilesDOSChinese[13] = {
	"DRAGON1.CPS",
	"DRAGON2.CPS",
	"HURRY1.CPS",
	"HURRY2.CPS",
	"DESTROY0.CPS",
	"DESTROY1.CPS",
	"DESTROY2.CPS",
	"MAGIC.CPS",
	"DESTROY3.CPS",
	"CREDITS2.CPS",
	"CREDITS3.CPS",
	"HEROES.CPS",
	"THANKS.CPS"
};

static const StringListProvider kEoB2FinaleCPSFilesDOSChineseProvider = { ARRAYSIZE(kEoB2FinaleCPSFilesDOSChinese), kEoB2FinaleCPSFilesDOSChinese };  // Likely ok, play finale to check

static const char *const kEoB2MonsterDistAttStringsDOSChinese[5] = {
	"\xb6\xa4\xa5\xee\xb3\x51\xa4\xdf\xc6\x46\xae\xa3\xc4\xdf\xb3\x4e\xa9\xd2\xc0\xbb\xa4\xa4\x21\r", /* "隊伍被心靈恐懼術所擊中!\r"; */
	"\xb3\x51\xb7\xf2\xb7\xf4\xb0\xca\xbc\x75\xa4\xa3\xb1\x6f", /* "被痲痺動彈不得"; */
	"\xa4\xa4\xac\x72\xa4\x46", /* "中毒了"; */
	"\xb7\xf2\xb7\xf4\xa4\x46", /* "痲痺了"; */
	"\xb3\x51\xa5\xdb\xa4\xc6\xa4\x46", /* "被石化了"; */
};

static const StringListProvider kEoB2MonsterDistAttStringsDOSChineseProvider = { ARRAYSIZE(kEoB2MonsterDistAttStringsDOSChinese), kEoB2MonsterDistAttStringsDOSChinese };

static const char *const kEoB2Npc1StringsDOSChinese[2] = {
	"\xa5\xe6\x20\xbd\xcd", /* "交 談"; */
	"\xc2\xf7\x20\xb6\x7d", /* "離 開"; */
};

static const StringListProvider kEoB2Npc1StringsDOSChineseProvider = { ARRAYSIZE(kEoB2Npc1StringsDOSChinese), kEoB2Npc1StringsDOSChinese };

static const char *const kEoB2Npc2StringsDOSChinese[2] = {
	"\xc4\xc0\xa9\xf1\xa5\x4c", /* "釋放他"; */
	"\xc2\xf7\x20\xb6\x7d", /* "離 開"; */
};

static const StringListProvider kEoB2Npc2StringsDOSChineseProvider = { ARRAYSIZE(kEoB2Npc2StringsDOSChinese), kEoB2Npc2StringsDOSChinese };

static const char *const kEoB2MonsterDustStringsDOSChinese[1] = {
	"\xb3\x6f\xa9\xc7\xaa\xab\xa4\x77\xb3\x51\xc0\xbb\xa6\xa8\xaf\xbb\xa8\xad\xb8\x48\xb0\xa9\xa4\x46\x21\r", /* "這怪物已被擊成粉身碎骨了!\r"; */
};

static const StringListProvider kEoB2MonsterDustStringsDOSChineseProvider = { ARRAYSIZE(kEoB2MonsterDustStringsDOSChinese), kEoB2MonsterDustStringsDOSChinese };

static const char *const kEoB2KheldranStringsDOSChinese[1] = {
	"\x20\x20\xa6\x70\xa6\xb9\xaf\xc2\xaf\x75\xaa\xba\xa4\x70\xa4\x6c\x21", /* "  如此純真的小子!"; */
};

static const StringListProvider kEoB2KheldranStringsDOSChineseProvider = { ARRAYSIZE(kEoB2KheldranStringsDOSChinese), kEoB2KheldranStringsDOSChinese }; 

static const char *const kEoB2HornStringsDOSChinese[4] = {
	"\xb1\x71\xb8\xb9\xa8\xa4\xa4\xa4\xb6\xc7\xa5\x58\xb6\xaf\xa7\x71\xaa\xba\xc1\x6e\xad\xb5\x2e\r", /* "從號角中傳出雄吼的聲音.\r"; */
	"\xb1\x71\xb8\xb9\xa8\xa4\xa4\xa4\xb6\xc7\xa5\x58\xaa\xc5\xac\x7d\xaa\xba\xc1\x6e\xad\xb5\x2e\r", /* "從號角中傳出空洞的聲音.\r"; */
	"\xb1\x71\xb8\xb9\xa8\xa4\xa4\xa4\xb6\xc7\xa5\x58\xae\xae\xa6\xd5\xaa\xba\xc1\x6e\xad\xb5\x2e\r", /* "從號角中傳出悅耳的聲音.\r"; */
	"\xb1\x71\xb8\xb9\xa8\xa4\xa4\xa4\xb6\xc7\xa5\x58\xa9\xc7\xb2\xa7\xaa\xba\xc1\x6e\xad\xb5\x2e\r", /* "從號角中傳出怪異的聲音.\r"; */
};

static const StringListProvider kEoB2HornStringsDOSChineseProvider = { ARRAYSIZE(kEoB2HornStringsDOSChinese), kEoB2HornStringsDOSChinese };

static const char *const kEoB2NpcPresetsNamesDOSChinese[6] = {
	"\xad\x5e\xc2\xc4\xba\xb8", /* "英薩爾"; "Insal" */
	"\xa5\x64\xc4\xf5\xbc\x77\xdb\x69", /* "卡蘭德菈"; "Calandra" */
	"\\xa5\xf0\xae\xa6", /* "休恩"; "Shorn" */
	"\xb8\x74\xb7\xe7\xba\xb8", /* "聖瑞爾"; "San-raal" */
	"\xc3\xd3\xae\xe6\xa6\xd5", /* "譚格耳"; "Tanglor" */
	"\xa6\x77\xbb\x5f\xba\xb8", /* "安蓓爾"; "Amber" */
};

static const StringListProvider kEoB2NpcPresetsNamesDOSChineseProvider = { ARRAYSIZE(kEoB2NpcPresetsNamesDOSChinese), kEoB2NpcPresetsNamesDOSChinese };

static const DarkMoonAnimCommand kEoB2IntroAnimData40DOSChinese[] = {
	{ 101,    0,  21, 20,   1,   1,   0,   0,   0,   0 },
	{ 101,    0,  21, 20,   1,   1,   0,   0,   0,   0 },
	{ 101,    0,  21, 20,   1,   2,   0,   0,   0,   0 },
	{ 101,    0,  21, 20,   1,   3,   0,   0,   0,   0 },
	{ 0x06,  11,   0,  0,   0,   0,   0,   0,   0,   0 },
	{ 101,    0,  21, 20,   1,   4,   0,   0,   0,   0 },
	{ 101,    0,  21, 20,   1,   5,   0,   0,   0,   0 },
	{ 101,    0,  21, 20,  18,   6,   0,   0,   0,   0 },
	{ 101,    0,  21, 20,  90,   6,   0,   0,   0,   0 },
	{ 101,    0,  21, 20,   1,   5,   0,   0,   0,   0 },
	{ 101,    0,  21, 20,   1,   4,   0,   0,   0,   0 },
	{ 101,    0,  21, 20,   1,   3,   0,   0,   0,   0 },
	{ 101,    0,  21, 20,   1,   2,   0,   0,   0,   0 },
	{ 101,    0,  21, 20,   1,   1,   0,   0,   0,   0 },
	{ 101,    0,  21, 20,   1,   1,   0,   0,   0,   0 },
	{ 101,    0,  21, 20,   1,   0,   0,   0,   0,   0 },
	{ 0xFF,   0,   0,  0,   0,   0,   0,   0,   0,   0 }
};

static const DarkMoonAnimCommandProvider kEoB2IntroAnimData40DOSChineseProvider = { ARRAYSIZE(kEoB2IntroAnimData40DOSChinese), kEoB2IntroAnimData40DOSChinese }; // OK
