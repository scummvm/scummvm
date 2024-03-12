#!/usr/bin/env python3

from jsonschema import validate
import json


def main():
    f = open('../dlc-metadata-schema.json')
    schema = json.load(f)

    f = open('../dlc-games.json')
    games = json.load(f)

    for key in games.keys():
        game = games[key]
        validate(game, schema)  # we get error if not valid

    print("All games' metadata in dlc-games.json is valid")


if __name__ == "__main__":
    main()
