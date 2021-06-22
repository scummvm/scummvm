#!/bin/bash
make bundle
cp dists/engine-data/mouse.lab ResidualVM.app/Contents/Resources/
rm -rf /tmp/pkg
mkdir -p /tmp/pkg
mv ResidualVM.app /tmp/pkg
cp README.ResidualVM /tmp/pkg/Readme_ResidualVM.txt
cp README /tmp/pkg/Readme.txt
hdiutil create /tmp/grim.dmg -ov -volname "Grim Mouse" -fs HFS+ -srcfolder /tmp/pkg
rm ~/Downloads/grim*.dmg
hdiutil convert /tmp/grim.dmg -format UDZO -o ~/Downloads/grim_mouse_0_7.dmg
scp ~/Downloads/grim_*.dmg login.eecs.berkeley.edu:public_html/data/
