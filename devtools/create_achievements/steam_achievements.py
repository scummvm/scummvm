#!/usr/bin/env python3

# This script takes two parameters
#   - the first is the Steam game id, 
#   - the second (optional) is the GOG Galaxy game id.
#
# Example:
#   ./steam_achievements.py --steamid 631570 [--galaxyid 1845001352]
#
# It will extract the Steam stats information to create the ScummVM ini-files for achievements.dat file
# For GOG versions of the games we assume that they are using same achievements as Steam versions

import os
import sys
import codecs
import argparse
import requests
from requests_html import HTMLSession

parser = argparse.ArgumentParser()
parser.add_argument("--steamid", required=True, default="631570", type=int, help="The Steam game id")
parser.add_argument("--saveasgalaxyid", type=int, help="The GOG Galaxy game id")
parser.add_argument("-v", "--verbose", action="store_true")
args = parser.parse_args()

statsurl = "https://steamdb.info/app/{0}/stats/".format(args.steamid)
if args.verbose:
	sys.stderr.write("query {0}\n".format(statsurl))

try:
	session = HTMLSession()
	response = session.get(statsurl)
	game = response.html.xpath("//h1[@itemprop='name']/text()")

	achievements_rows = response.html.xpath("//tr[starts-with(@id, 'achievement-')]/td")
	achievements_columns = 3 # id, text, img
	achievements_entries = int(len(achievements_rows) / achievements_columns)
	if achievements_entries == 0:
		sys.exit(127)

	if args.verbose:
		sys.stderr.write("found {0} achievements\n".format(achievements_entries))

	stats_rows = response.html.xpath("//tr[starts-with(@id, 'stat-')]/td")
	stats_columns = 3 # id, text, default value, your value
	stats_entries = int(len(stats_rows) / stats_columns)

	if args.verbose:
		sys.stderr.write("found {0} stats\n".format(stats_entries))

	stats_en = {}
	for i in range(stats_entries):
		idx   = stats_columns * i
		id    = stats_rows[idx + 0].text.strip()
		descr = stats_rows[idx + 1].text.strip()
		start = stats_rows[idx + 2].text.strip()
		if descr == "no name":
			descr = ""
		stats_en[i] = (id, descr, start)

	achievements_en = {}
	for i in range(achievements_entries):
		idx   = achievements_columns * i
		id    = achievements_rows[idx + 0].text.strip()
		title = achievements_rows[idx + 1].text.strip().split("\n")[0]
		descr = achievements_rows[idx + 1].text.strip().split("\n")[1]
		hide  = descr == "Hidden."
		if descr in ["No description.", "Hidden."]:
			descr = ""
		achievements_en[i] = (id, title, descr, hide)

	if args.saveasgalaxyid:
		fname = "galaxy-{0}.ini".format(args.saveasgalaxyid)
	else:
		fname = "steam-{0}.ini".format(args.steamid)
	fname = "gen" + os.sep + fname

	if args.verbose:
		sys.stderr.write("writing: {0}\n".format(fname))
	with codecs.open(fname, "w", encoding="utf-8") as out:
		if stats_en:
			out.write("[stats:en]\n")
			for i, (id, descr, start) in stats_en.items():
				out.write("item_%d_id=%s\n" % (i, id))
				if descr:
					out.write("item_%d_comment=%s\n" % (i, descr))
				out.write("item_%d_start=%s\n" % (i, start))
		if achievements_en:
			out.write("[achievements:en]\n")
			for i, (id, title, descr, hide) in achievements_en.items():
				out.write("item_%d_id=%s\n" % (i, id))
				out.write("item_%d_title=%s\n" % (i, title))
				if descr:
					out.write("item_%d_comment=%s\n" % (i, descr))
				if hide:
					out.write("item_%d_hidden=true\n" % i)

except requests.exceptions.RequestException as e:
	print(e)
