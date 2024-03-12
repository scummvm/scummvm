#!/usr/bin/env python3

import argparse
import json
import subprocess
import sys


def prepare(ex, game, arg_dict):
    if ex["prepare"]["type"] == "python-script":
        cmd = sys.executable + " scripts/" + ex["prepare"]["location"]
        for key in ex["prepare"]["options"].keys():
            cmd += " " + key + " "
            value = ex["prepare"]["options"][key]
            # $ -> get from here (package.py), $$ -> get from games.json, $$$ -> get from environment variables
            if len(value) >= 3 and value[2] == '$':
                cmd += value[2:]
            elif len(value) >= 2 and value[1] == '$':
                # check in game's metadata
                cmd += game[value[2:]]
            elif len(value) >= 1 and value[0] == '$':
                cmd += arg_dict[value[1:]]
            else:
                cmd += value

        subprocess.run(cmd, shell=True)
        print(ex["prepare"]["successMessage"])


def bundle(ex, game, arg_dict):
    if ex["bundle"]["type"] == "python-script":
        cmd = sys.executable + " scripts/" + ex["bundle"]["location"]
        for key in ex["bundle"]["options"].keys():
            cmd += " " + key + " "
            value = ex["bundle"]["options"][key]
            # $ -> get from here (package.py), $$ -> get from games.json, $$$ -> get from environment variables
            if len(value) >= 3 and value[2] == '$':
                cmd += value[2:]
            elif len(value) >= 2 and value[1] == '$':
                # check in game's metadata
                cmd += game[value[2:]]
                if value == "$$packname":
                    cmd += ".zip"  # temporary hack
            elif len(value) >= 1 and value[0] == '$':
                cmd += arg_dict[value[1:]]
            else:
                cmd += value

        subprocess.run(cmd, shell=True)
        print(ex["bundle"]["successMessage"])


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--export-platform', required=True,
                        help='Export platform name as per export-platforms.json')
    parser.add_argument('--game', required=True,
                        help='Game name as per dlc-games.json')
    parser.add_argument('--game-location', required=True,
                        help='Location of game\'s datafiles')
    parser.add_argument('--binary-location', required=False,
                        help='Location of ScummVM executable (Required for Android)')

    args = parser.parse_args()

    arg_dict = {"game_location": args.game_location,
            "binary_location": args.binary_location}

    f = open('export-platforms.json')
    export_platforms = json.load(f)

    f = open('dlc-games.json')
    games = json.load(f)

    if args.export_platform in export_platforms:
        if args.game in games:
            print()
            prepare(export_platforms[args.export_platform],
                    games[args.game], arg_dict)

            bundle(export_platforms[args.export_platform],
                   games[args.game], arg_dict)

        else:
            print(
                "Game's metadata not found! Make sure the provided game is in dlc-games.json")
    else:
        print("Export platform not found! Make sure the provided export-platform is in export-platforms.json")


if __name__ == '__main__':
    main()
