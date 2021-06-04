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

parser = argparse.ArgumentParser()
parser.add_argument("--steamid", required=True, default="631570", type=int, help="Steam game id")
parser.add_argument("--saveasgalaxyid", type=int, help="GOG Galaxy game id")
parser.add_argument("-v", "--verbose", action="store_true")
args = parser.parse_args()

def parse_steamdb(statsurl):
	response = HTMLSession().get(statsurl)

	achievements_rows = response.html.xpath("//tr[starts-with(@id, 'achievement-')]/td")
	achievements_columns = 3 # name, text, img
	achievements_entries = int(len(achievements_rows) / achievements_columns)
	if achievements_entries == 0:
		sys.stderr.write("found NO achievements\n")
		sys.exit(127)

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
			sys.stderr.write("Unexpected description format: {0}\n".format(repr(texts)))
			sys.exit(127)

		title = texts[0]
		descr = texts[1]
		hide  = descr == "Hidden."
		if descr in ["No description.", "Hidden."]:
			descr = ""
		achievements_en[i] = (name, title, descr, hide)

	return achievements_en, stats_en

def write_ini(fname, achievements, stats):
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
	if args.verbose:
		sys.stderr.write("query {0}\n".format(STATS_URL))

	achievements_en, stats_en = parse_steamdb(STATS_URL)
	if args.verbose:
		sys.stderr.write("found {0} achievements\n".format(len(achievements_en)))
		sys.stderr.write("found {0} stats\n".format(len(stats_en)))

	if args.saveasgalaxyid:
		FNAME = "galaxy-{0}.ini".format(args.saveasgalaxyid)
	else:
		FNAME = "steam-{0}.ini".format(args.steamid)
	if args.verbose:
		sys.stderr.write("writing: {0}\n".format(FNAME))

	achievements = {"en": achievements_en}
	stats = {"en": stats_en} if stats_en else {}
	write_ini(os.path.join("gen", FNAME), achievements, stats)

except requests.exceptions.RequestException as e:
	print(e)
	sys.exit(127)
