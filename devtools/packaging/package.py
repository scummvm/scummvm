import argparse
import json
import subprocess
import sys


def prepare(ex, game, location):
    if ex["prepare"]["type"] == "python-script":
        cmd = sys.executable + " scripts/" + ex["prepare"]["location"]
        for key in ex["prepare"]["options"].keys():
            cmd += " " + key + " "
            value = ex["prepare"]["options"][key]
            if value == "$location":
                cmd += location
            elif value[0] == '$':
                # check in game's metadata
                cmd += game[value[1:]]
            else:
                cmd += value

        subprocess.run(cmd, shell=True)
        print(ex["prepare"]["successMessage"])


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--export-platform', required=True,
                        help='Export platform name as per export-platforms.json')
    parser.add_argument('--game', required=True,
                        help='Game name as per dlc-games.json')
    parser.add_argument('--location', required=True,
                        help='Location of game\'s datafiles')

    args = parser.parse_args()

    f = open('export-platforms.json')
    export_platforms = json.load(f)

    f = open('dlc-games.json')
    games = json.load(f)

    if args.export_platform in export_platforms:
        if args.game in games:
            prepare(export_platforms[args.export_platform],
                    games[args.game], args.location)

        else:
            print(
                "Game's metadata not found! Make sure the provided game is in dlc-games.json")
    else:
        print("Export platform not found! Make sure the provided export-platform is in export-platforms.json")


if __name__ == '__main__':
    main()
