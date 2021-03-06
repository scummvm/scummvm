#!/usr/bin/env python3

# This script takes two parameters - the first is the steam game id, the second (optional) is the scummvm
# game id. E.g.
#
# ./steam_achivements.py 212050 resonance
#
# It will extract the steam stats information to create the scummvm achievement tables with the macros found
# in AGS achievements_table.h

import requests
from requests_html import HTMLSession
import sys

if len(sys.argv) < 2:
	steam_game_id = 212050
	sys.stderr.write('missing steam game id as first parameter - assuming {0}\n'.format(steam_game_id))
else:
	steam_game_id = sys.argv[1]


statsurl = "https://steamdb.info/app/{0}/stats/".format(steam_game_id)

try:
	session = HTMLSession()
	response = session.get(statsurl)
	achievements_rows = response.html.xpath("//tr[starts-with(@id, 'achievement-')]/td")
	game = response.html.xpath("//meta[@property='og:title']/@content")
	achievements_columns = 3 # id, text, img
	entries = int(len(achievements_rows) / achievements_columns)
	sys.stderr.write('found {0} achievements\n'.format(entries))

	if len(sys.argv) < 3:
		scummvm_game_id = game[0]
		sys.stderr.write('missing scummvm game id - assuming {0}\n'.format(scummvm_game_id))
	else:
		scummvm_game_id = sys.argv[2]

	print("\t{\n\t\t\"%s\",\n\t\tCommon::STEAM_ACHIEVEMENTS,\n\t\t\"%s\",\n\t\t{" % (scummvm_game_id, steam_game_id))
	for i in range(entries):
		idx       = achievements_columns * i
		ach_id    = achievements_rows[idx + 0].text.strip()
		ach_text  = achievements_rows[idx + 1].text.strip()
		ach_title = ach_text.split('\n')[0].replace('"','\\"')
		ach_desc  = ach_text.split('\n')[1].replace('"','\\"')
		if ach_desc == "Hidden.":
			print("\t\t\tACHIEVEMENT_HIDDEN_ENTRY(\"%s\", \"%s\", \"%s\")," % (ach_id, ach_title, ach_desc))
		else:
			print("\t\t\tACHIEVEMENT_SIMPLE_ENTRY(\"%s\", \"%s\", \"%s\")," % (ach_id, ach_title, ach_desc))

	print("\t\t\tACHIEVEMENTS_LISTEND\n\t\t}\n\t},")
except requests.exceptions.RequestException as e:
	print(e)
