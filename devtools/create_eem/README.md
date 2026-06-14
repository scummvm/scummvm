EEM engine data
-------------------------------------------------------------------------------

`SCRAPBK_EXTRA.ANI` stores the three floppy-only dialog records displayed
after `SCRAPBK.ANI` and before the scrapbook page. The CD executable omits
those records from its solved recap data. The restored subtitles are kept for
all books, but voice indices are only attached for Book 1 mysteries because
matching unused CD clips could not be identified reliably for Books 2 and 3.

Generate the file from extracted English floppy and CD data:

  ./create_scrapbk_extra.py \
      --floppy-dir ../../eem-full-game/floppy/EAKIDS/EEM \
      --cd-dir ../../eem-full-game/cd

Then rebuild the distributable data archive from this directory:

  cd files
  zip -r9 ../../../dists/engine-data/eem.dat .
