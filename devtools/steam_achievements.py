#!/usr/bin/env python3

# This script takes two parameters - the first is the steam game id, the second (optional) is the scummvm
# game id. E.g.
#
# ./steam_achivements.py 212050 resonance
#
# It will extract the steam stats information to create the scummvm achievement tables with the macros found
# in AGS achievements_table.h

import requests
import argparse
from requests_html import HTMLSession
import sys

parser = argparse.ArgumentParser()
parser.add_argument('--steamid', required=True, default='212050', type=int, help="The steam game id")
parser.add_argument('--gameid', help="The scummvm game id string")
parser.add_argument('-v', '--verbose', action='store_true')
args = parser.parse_args()

statsurl = "https://steamdb.info/app/{0}/stats/".format(args.steamid)
if args.verbose:
	sys.stderr.write('query {0}\n'.format(statsurl))

def cleanup_text(text):
	text = text.encode(encoding="ascii", errors="backslashreplace").decode()
	text = text.replace('"','\\"')
	return text

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
		sys.stderr.write('found {0} achievements\n'.format(achievements_entries))

	stats_rows = response.html.xpath("//tr[starts-with(@id, 'stat-')]/td")
	stats_columns = 3 # id, text, default value, your value
	stats_entries = int(len(stats_rows) / stats_columns)

	if args.verbose:
		sys.stderr.write('found {0} stats\n'.format(stats_entries))

	scummvm_game_id = args.gameid
	if not scummvm_game_id:
		scummvm_game_id = game[0].lower().replace(' ', '').replace('-', '')
		if args.verbose:
			sys.stderr.write('missing scummvm game id - assuming {0}\n'.format(scummvm_game_id))

	print("\t{\n\t\t\"%s\",\n\t\tCommon::STEAM_ACHIEVEMENTS,\n\t\t\"%s\"," % (scummvm_game_id, args.steamid))
	if stats_entries:
		print("\t\t{")
		for i in range(stats_entries):
			idx       = stats_columns * i
			stat_id    = stats_rows[idx + 0].text.strip()
			stat_desc  = stats_rows[idx + 1].text.strip()
			stat_default = stats_rows[idx + 2].text.strip()
			if  stat_desc == "no name":
				print("\t\t\tSTATS_NODESC_ENTRY(\"%s\", \"%s\")," % (stat_id, stat_default))
			else:
				print("\t\t\tSTATS_SIMPLE_ENTRY(\"%s\", \"%s\", \"%s\")," % (stat_id, stat_desc, stat_default))
		print("\t\t\tSTATS_LISTEND\n\t\t},")
	else:
		print("\t\tNOSTATS,")
	print("\t\t{")
	for i in range(achievements_entries):
		idx       = achievements_columns * i
		ach_id    = achievements_rows[idx + 0].text.strip()
		ach_text  = achievements_rows[idx + 1].text.strip()
		ach_title = cleanup_text(ach_text.split('\n')[0])
		ach_desc  = cleanup_text(ach_text.split('\n')[1])
		if ach_desc == "Hidden.":
			print("\t\t\tACHIEVEMENT_HIDDEN_ENTRY(\"%s\", \"%s\")," % (ach_id, ach_title))
		elif ach_desc == "No description.":
			print("\t\t\tACHIEVEMENT_NODESC_ENTRY(\"%s\", \"%s\")," % (ach_id, ach_title))
		else:
			print("\t\t\tACHIEVEMENT_SIMPLE_ENTRY(\"%s\", \"%s\", \"%s\")," % (ach_id, ach_title, ach_desc))

	print("\t\t\tACHIEVEMENTS_LISTEND\n\t\t}\n\t},")
except requests.exceptions.RequestException as e:
	print(e)
