# $VER: READMEconverter.sed 1.05 (25.01.2018) © Eugene "sev" Sandulenko
# Additions and changes by Raziel
#
# Preprocessing the README file and adding some markers for easier parsing
# and later converting it to an AmigaGuide Hypertext file.
#
s/http:\/\/[#?=&a-zA-Z0-9_.\/\-]*/@{"&" System "URLOpen &"}/	# Convert all URLs to AmigaGuide format
s/https:\/\/[#?=&a-zA-Z0-9_.\/\-]*/@{"&" System "URLOpen &"}/	# Convert all secure URLs to AmigaGuide format
s/[0-9][0-9]*\.[0-9][0-9]*/<>&<>/							# Convert all chapter numbers to <>x<>...
s/<>\([0-9][0-9]*\.[0-9][0-9]*\)<>\(\.[0-9]\)/<>\1\2<>/			# ...and all three-digit chapter numbers...
s/<>\([01]\.[0-9][0-9]*\.[0-9][0-9]*\)<>/\1/					# ...and restore mentioned version numbers like 1.0.0 and 0.7.0.
s/of <>0\.0<>/of 0.0/									# "Fluidsynth's gain setting of 0.0" is not a chapter reference.
s/through <>10\.0<>/through 10.0/						# "through 10.0" is not a chapter reference.
s/ttf-<>2\.00.1<>/ttf-2.00.1/								# This part of an url link is not a chapter reference.
s/patch <>1\.2<>/patch 1.2/								# "Zork patch 1.2" is not a chapter reference.
s/Mac OS X <>10\.2.8<>/Mac OS X 10.2.8/					# "Mac OS X 10.2.8" is not a chapter reference.
s/Mac_OS_X_<>10\.2.8<>/Mac_OS_X_10.2.8/					# "Mac_OS_X_10.2.8" is not a chapter reference.
