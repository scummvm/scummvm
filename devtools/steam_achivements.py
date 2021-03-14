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
	achievements_rows = response.html.xpath("//tr[starts-with(@id, 'achievement-')]/td")
	game = response.html.xpath("//h1[@itemprop='name']/text()")
	achievements_columns = 3 # id, text, img
	entries = int(len(achievements_rows) / achievements_columns)
	if entries == 0:
		sys.exit(127)

	if args.verbose:
		sys.stderr.write('found {0} achievements\n'.format(entries))

	scummvm_game_id = args.gameid
	if not scummvm_game_id:
		scummvm_game_id = game[0].lower().replace(' ', '').replace('-', '')
		if args.verbose:
			sys.stderr.write('missing scummvm game id - assuming {0}\n'.format(scummvm_game_id))

	print("\t{\n\t\t\"%s\",\n\t\tCommon::STEAM_ACHIEVEMENTS,\n\t\t\"%s\",\n\t\t{" % (scummvm_game_id, args.steamid))
	for i in range(entries):
		idx       = achievements_columns * i
		ach_id    = achievements_rows[idx + 0].text.strip()
		ach_text  = achievements_rows[idx + 1].text.strip()
		ach_title = cleanup_text(ach_text.split('\n')[0])
		ach_desc  = cleanup_text(ach_text.split('\n')[1])
		if ach_desc == "Hidden.":
			print("\t\t\tACHIEVEMENT_HIDDEN_ENTRY(\"%s\", \"%s\")," % (ach_id, ach_title))
		else:
			print("\t\t\tACHIEVEMENT_SIMPLE_ENTRY(\"%s\", \"%s\", \"%s\")," % (ach_id, ach_title, ach_desc))

	print("\t\t\tACHIEVEMENTS_LISTEND\n\t\t}\n\t},")
except requests.exceptions.RequestException as e:
	print(e)
