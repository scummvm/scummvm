#!/usr/bin/env python3

# This script takes two parameters
#   - the first is the Steam game id,
#   - the second (optional) is the GOG Galaxy game id.
#
# Example:
#   ./steam_achievements.py --steamid 631570 [--galaxyid 1845001352]
#
# It will extract the Steam stats information to create the ScummVM ini-files for achievements.dat
# For GOG versions of the games we assume that they are using same achievements as Steam versions

import os
import sys
import codecs
import argparse
import requests
from requests_html import HTMLSession

# For Stats only English strings exists on Steam side.
#   - stats info is collected from SteamDB, we collect lists of <id, [comment], initial value>

# For Achievements some games have only English strings, and some games have additional translations on Steam side.
# This script is currently using various sources of information for Steam achievements:
#   - achievements info collection starts from SteamDB, we collect lists of <id, title, [comment], hidden flag>
#   - if there are hidden achievements, we look up their English "comment" values on 3rd party site (achievementstats.com)
#   - there is an additional call to SteamDB to collect list of achivements translations
#   - if there are no hidden achievements, we use Global Statistics at steamcommunity.com to collect non-English strings
#   - if there are hidden achievements, we use User Statistics at steamcommunity.com to collect non-English strings

# Hidden achievement descriptions are available only at user accounts for those users who have them completed

STEAM_USERS = {
	"283880"  : "lb_ii",
	"370910"  : "lb_ii",
	"378630"  : "lb_ii",
	"420180"  : "lb_ii",
	"551840"  : "lb_ii",
	"631570"  : "lb_ii",
	"761460"  : "lb_ii",
	"1251910" : "lb_ii",
	"1270590" : "lb_ii",
}

# Format is: <SteamDB language name>: (<Steam API language code>, <unixLocale>)
# For <SteamDB language name>, see verbose output of this tool
# For <Steam API language code>, see https://partner.steamgames.com/doc/store/localization
# For <unixLocale>, see "common/language.cpp"

LANGUAGES = {
	"English":             ("english",    "en"),
	"Dutch":               ("dutch",      "nl_NL"),
	"German":              ("german",     "de_DE"),
	"French":              ("french",     "fr_FR"),
	"Hungarian":           ("hungarian",  "hu_HU"),
	"Italian":             ("italian",    "it_IT"),
	"Polish":              ("polish",     "pl_PL"),
	"Portuguese":          ("portuguese", "pt_PT"),
	"Portuguese - Brazil": ("brazilian",  "pt_BR"),
	"Russian":             ("russian",    "ru_RU"),
	"Spanish - Spain":     ("spanish",    "es_ES"),
	"Simplified Chinese":  ("schinese",   "zh_CN"),
}

parser = argparse.ArgumentParser()
parser.add_argument("--steamid", required=True, default="631570", type=int, help="Steam game id")
parser.add_argument("--saveasgalaxyid", type=int, help="GOG Galaxy game id")
parser.add_argument("-v", "--verbose", action="store_true")
args = parser.parse_args()

def log(msg):
	global args
	if args.verbose:
		sys.stderr.write(msg + "\n")

def err(msg):
	sys.stderr.write(msg + "\n")
	sys.exit(127)

def parse_steamdb_info(url):
	log("query {0}".format(url))
	response = HTMLSession().get(url)

	info_rows = response.html.xpath("//div[@id='info']/table/tbody/tr/td")
	info_columns = 2 # id, text,
	info_entries = int(len(info_rows) / info_columns)
	if info_entries == 0:
		log(repr(response.html.raw_html))
		err("found NO information data")

	FORMAT_CHECKER_STRING = "Store Release Date"
	is_format_ok = False

	langs = "English"
	for i in range(info_entries):
		idx        = info_columns * i
		info_key   = info_rows[idx + 0].text.strip()
		info_value = info_rows[idx + 1].text.strip()
		if info_key == FORMAT_CHECKER_STRING:
			is_format_ok = True
		if info_key == "Achievement Languages":
			langs = info_value

	if not is_format_ok:
		log(repr(response.html.raw_html))
		err("found NO {0}\nEntries: {1}".format(FORMAT_CHECKER_STRING, [i.text for i in info_rows][::2]))

	return langs.split(", ")

def parse_steamdb_stats(url):
	log("query {0}".format(url))
	response = HTMLSession().get(url)

	achievements_rows = response.html.xpath("//tr[starts-with(@id, 'achievement-')]/td")
	achievements_columns = 3 # name, text, img
	achievements_entries = int(len(achievements_rows) / achievements_columns)
	if achievements_entries == 0:
		log(repr(response.html.raw_html))
		err("found NO achievements")

	stats_rows = response.html.xpath("//tr[starts-with(@id, 'stat-')]/td")
	stats_columns = 3 # name, text, default value
	stats_entries = int(len(stats_rows) / stats_columns)

	stats_en = {}
	for i in range(stats_entries):
		idx   = stats_columns * i
		name  = stats_rows[idx + 0].text.strip()
		descr = stats_rows[idx + 1].text.strip()
		start = stats_rows[idx + 2].text.strip()
		if descr == "no name":
			descr = ""
		stats_en[i] = (name, descr, start)

	achievements_en = {}
	for i in range(achievements_entries):
		idx   = achievements_columns * i
		name  = achievements_rows[idx + 0].text.strip()
		texts = achievements_rows[idx + 1].text.strip().split("\n")

		if len(texts) != 2:
			log(repr(response.html.raw_html))
			err("Unexpected description format: {0}".format(texts))

		title = texts[0]
		descr = texts[1]
		hide  = descr == "Hidden."
		if descr in ["No description.", "Hidden."]:
			descr = ""
		achievements_en[i] = (name, title, descr, hide)

	return achievements_en, stats_en

def parse_steamcommunity_stats(url):
	log("query {0}".format(url))
	response = HTMLSession().get(url)

	achievements_rows = response.html.xpath("//div[@class='achieveRow']")
	achievements_entries = len(achievements_rows)
	if achievements_entries == 0:
		log(repr(response.html.raw_html))
		err("found NO achievements")

	translation = {}
	for idx in range(achievements_entries):
		imgs  = achievements_rows[idx].xpath("//img/@src")
		titles = achievements_rows[idx].xpath(".//div[@class='achieveTxt']/h3/text()")
		descrs = achievements_rows[idx].xpath(".//div[@class='achieveTxt']/h5/text()")

		if len(imgs) != 1:
			log(repr(response.html.raw_html))
			err("Unexpected xpath result: expected exactly one img tag per achievement")
		if len(titles) != 1:
			log(repr(response.html.raw_html))
			err("Unexpected xpath result: expected exactly one h3 tag per achievement")
		if len(descrs) > 1:
			log(repr(response.html.raw_html))
			err("Unexpected xpath result: expected zero or one h5 tag per achievement")

		translation[imgs[0]] = (titles[0].strip(), descrs[0].strip() if descrs else None)

	return translation

def parse_achievementstats_stats(url):
	log("query {0}".format(url))
	response = HTMLSession().get(url)

	tables = response.html.xpath("//table")
	if len(tables) != 1:
		log(repr(response.html.raw_html))
		err("Unexpected xpath result: expected exactly one table tag on page")

	achievements_rows = response.html.xpath("//tbody/tr/td")
	achievements_columns = 6 # icon, name, text, date, point, report
	achievements_entries = int(len(achievements_rows) / achievements_columns)
	if achievements_entries == 0:
		log(repr(response.html.raw_html))
		err("found NO achievements")

	result = {}
	for i in range(achievements_entries):
		idx   = achievements_columns * i
		name  = achievements_rows[idx + 1].text.strip()
		descr = achievements_rows[idx + 2].text.strip()
		result[name] = descr
	return result

def join_achievements_translation(achievements_en, translations):
	achievements = {"en": achievements_en}

	entitle2img = {}
	for img, (title, descr) in translations["English"].items():
		entitle2img[title] = img

	for l in translations.keys():
		lang_id = LANGUAGES[l][1]
		if lang_id == "en":
			continue

		achievements[lang_id] = {}
		for i, (name, title, descr, hide) in achievements_en.items():
			if  not title in entitle2img:
				err("Can't find '{0}' at {1}".format(title, entitle2img))

			t = translations[l][entitle2img[title]]
			achievements[lang_id][i] = (name, t[0], t[1] if t[1] else descr, hide)

		completely_same = True
		for i, it in achievements_en.items():
			if achievements[lang_id][i] != it:
				completely_same = False
				break
		if completely_same:
			print("WARNING: Translation is available for {0}, but is completely the same as English for game {1}".format(l, args.steamid))
			del achievements[lang_id]

	return achievements

def join_achievements_descr(achievements_en, descs):
	result = {}
	for i, (name, title, descr, hide) in achievements_en.items():
		ext_descr = descrs[title]
		if descr and descr != ext_descr:
			err("Unexpected difference between {0} and {1} for {2}".format(descr, ext_descr, title))
		result[i] = (name, title, descr if descr else ext_descr, hide)
	return result

def write_ini(fname, achievements, stats):
	log("writing: {0}".format(fname))
	with codecs.open(fname, "w", encoding="utf-8") as out:
		for lang, it in stats.items():
			out.write("[stats:{0}]\n".format(lang))
			for i, (name, descr, start) in it.items():
				out.write("item_%d_id=%s\n" % (i, name))
				if descr:
					out.write("item_%d_comment=%s\n" % (i, descr))
				out.write("item_%d_start=%s\n" % (i, start))
		for lang, it in achievements.items():
			out.write("[achievements:{0}]\n".format(lang))
			for i, (name, title, descr, hide) in it.items():
				out.write("item_%d_id=%s\n" % (i, name))
				out.write("item_%d_title=%s\n" % (i, title))
				if descr:
					out.write("item_%d_comment=%s\n" % (i, descr))
				if hide:
					out.write("item_%d_hidden=true\n" % i)

try:
	STATS_URL = "https://steamdb.info/app/{0}/stats/".format(args.steamid)
	achievements_en, stats_en = parse_steamdb_stats(STATS_URL)
	log("found {0} achievements".format(len(achievements_en)))
	log("found {0} stats".format(len(stats_en)))

	hidden_achievements = [it for it in achievements_en.values() if it[3]]
	log("found {0} hidden achievements".format(len(hidden_achievements)))

	if hidden_achievements:
		HIDDEN_STATS_URL = "https://www.achievementstats.com/index.php?action=games&gameId={0}".format(args.steamid)
		descrs = parse_achievementstats_stats(HIDDEN_STATS_URL)
		achievements_en = join_achievements_descr(achievements_en, descrs)

	INFO_URL = "https://steamdb.info/app/{0}/info/".format(args.steamid)
	langs = parse_steamdb_info(INFO_URL)
	log("found langs: {0}".format(langs))

	translations = {"English":{}}
	if len(langs) > 1:
		for l in langs:
			if not hidden_achievements or len(langs) == 1:
				TRANSLATION_URL = "https://steamcommunity.com/stats/{0}/achievements?l={1}".format(args.steamid, LANGUAGES[l][0])
				translations[l] = parse_steamcommunity_stats(TRANSLATION_URL)
			elif str(args.steamid) in STEAM_USERS:
				TRANSLATION_URL = "https://steamcommunity.com/id/{0}/stats/{1}/?l={2}".format(STEAM_USERS[str(args.steamid)], args.steamid, LANGUAGES[l][0])
				translations[l] = parse_steamcommunity_stats(TRANSLATION_URL)
			else:
				print("WARNING: ignoring {0} localization for game {1} since it has hidden achievements not listed at STEAM_USERS, please buy, complete and add this game!".format(l, args.steamid))

	achievements = join_achievements_translation(achievements_en, translations)
	stats = {"en": stats_en} if stats_en else {}

	if args.saveasgalaxyid:
		FNAME = "galaxy-{0}.ini".format(args.saveasgalaxyid)
	else:
		FNAME = "steam-{0}.ini".format(args.steamid)
	write_ini(os.path.join("gen", FNAME), achievements, stats)

except requests.exceptions.RequestException as e:
	print(e)
	sys.exit(127)
