#!/usr/bin/env python

'''
This script generates subtitles for Toonstrack cutscenes
from .SBV subtitiles.

Usage:
```
submaker.py INFILE.sbv OUTFILE.tss
````

Subtitles format:
<start frame> <end frame> <subtitle text>
'''
import sys

from datetime import datetime

TIME_FORMAT = '%H:%M:%S.%f'
BASETIME = datetime(1900, 1, 1)
FPS = 15

def time2frame(time, fps=FPS):
    return round(fps * (datetime.strptime('0' + time + '000', TIME_FORMAT) - BASETIME).total_seconds())

def sbv2tss(infile, fps=FPS):
    with open(infile, 'r') as sub_file:
        lines = sub_file.read().split('\n\n')

    # ignore empty lines
    lines = [line for line in lines if line]

    for line in lines:
        time_window, text = line.split('\n')[:2]
        start, end = time_window.split(',')
        start = time2frame(start, fps=fps)
        end = time2frame(end[:-1], fps=fps)

        yield '{start} {end} {line}\n'.format(start=start, end=end, line=text)

if __name__ == '__main__':
    if len(sys.argv) < 3:
        print('Usage: toon_submaker.py INFILE.sbv OUTFILE.tss')
        exit(1)

    infile = sys.argv[1]
    outfile = sys.argv[2]

    with open(outfile, 'w') as sub_file:
        for line in sbv2tss(infile):
            sub_file.write(line)
