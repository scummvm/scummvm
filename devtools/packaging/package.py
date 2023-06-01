import argparse
import json
import subprocess
import sys


def prepare(ex, game, location):
    if ex["prepare"]["type"] == "python-script":
        # hardcoded for now
        subprocess.run(sys.executable + " scripts/" + ex["prepare"]["location"] + " --packagename scummvm --assetpackname beneath-a-steel-sky --deliverymode on-demand --assetsdir " +
                       location + " --outdir output", shell=True)


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
