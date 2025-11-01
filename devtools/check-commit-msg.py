#!/usr/bin/env python3

import sys
import os
import re
from argparse import ArgumentParser

parser = ArgumentParser(description="SCUMMVM check commit message hook")
parser.add_argument("message")
args = parser.parse_args()

with open(args.message, "rt") as f:
    text = f.read()

MESSAGE_FORMAT = re.compile(r"[A-Z_]+:")

if MESSAGE_FORMAT.match(text):
	sys.exit(0)

print("Please start your commit message with subsystem or engine name written in CAPS", file=sys.stderr)
print("See https://wiki.scummvm.org/index.php/Commit_Guidelines", file=sys.stderr)
print("Your original message follows:\n\n", text)
sys.exit(1)
