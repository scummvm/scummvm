bundle_name = ScummVM.app

ios7bundle: all
	mkdir -p $(bundle_name)
	awk 'BEGIN {s=0}\
		/<key>CFBundleIcons<\/key>/ {\
			print $$0;\
			print "\t<dict>";\
			print "\t\t<key>CFBundlePrimaryIcon</key>";\
			print "\t\t<dict>";\
			print "\t\t\t<key>CFBundleIconFiles</key>";\
			print "\t\t\t<array>";\
			print "\t\t\t\t<string>AppIcon29x29</string>";\
			print "\t\t\t\t<string>AppIcon40x40</string>";\
			print "\t\t\t\t<string>AppIcon60x60</string>";\
			print "\t\t\t</array>";\
			print "\t\t</dict>";\
			print "\t</dict>";\
			s=2}\
		/<key>CFBundleIcons~ipad<\/key>/ {\
			print $$0;\
			print "\t<dict>";\
			print "\t\t<key>CFBundlePrimaryIcon</key>";\
			print "\t\t<dict>";\
			print "\t\t\t<key>CFBundleIconFiles</key>";\
			print "\t\t\t<array>";\
			print "\t\t\t\t<string>AppIcon29x29</string>";\
			print "\t\t\t\t<string>AppIcon40x40</string>";\
			print "\t\t\t\t<string>AppIcon60x60</string>";\
			print "\t\t\t\t<string>AppIcon76x76</string>";\
			print "\t\t\t\t<string>AppIcon83.5x83.5</string>";\
			print "\t\t\t</array>";\
			print "\t\t</dict>";\
			print "\t</dict>";\
			s=2}\
		/<key>UILaunchImages<\/key>/ {\
			print $$0;\
			print "\t<array>";\
			print "\t\t<dict>";\
			print "\t\t\t<key>UILaunchImageMinimumOSVersion</key>";\
			print "\t\t\t<string>8.0</string>";\
			print "\t\t\t<key>UILaunchImageName</key>";\
			print "\t\t\t<string>LaunchImage-800-Portrait-736h</string>";\
			print "\t\t\t<key>UILaunchImageOrientation</key>";\
			print "\t\t\t<string>Portrait</string>";\
			print "\t\t\t<key>UILaunchImageSize</key>";\
			print "\t\t\t<string>{414, 736}</string>";\
			print "\t\t\t<key>UILaunchImageMinimumOSVersion</key>";\
			print "\t\t\t<string>8.0</string>";\
			print "\t\t\t<key>UILaunchImageName</key>";\
			print "\t\t\t<string>LaunchImage-800-Landscape-736h</string>";\
			print "\t\t\t<key>UILaunchImageOrientation</key>";\
			print "\t\t\t<string>Landscape</string>";\
			print "\t\t\t<key>UILaunchImageSize</key>";\
			print "\t\t\t<string>{414, 736}</string>";\
			print "\t\t\t<key>UILaunchImageMinimumOSVersion</key>";\
			print "\t\t\t<string>8.0</string>";\
			print "\t\t\t<key>UILaunchImageName</key>";\
			print "\t\t\t<string>LaunchImage-800-667h</string>";\
			print "\t\t\t<key>UILaunchImageOrientation</key>";\
			print "\t\t\t<string>Portrait</string>";\
			print "\t\t\t<key>UILaunchImageSize</key>";\
			print "\t\t\t<string>{375, 667}</string>";\
			print "\t\t\t<key>UILaunchImageMinimumOSVersion</key>";\
			print "\t\t\t<string>7.0</string>";\
			print "\t\t\t<key>UILaunchImageName</key>";\
			print "\t\t\t<string>LaunchImage-700-568h</string>";\
			print "\t\t\t<key>UILaunchImageOrientation</key>";\
			print "\t\t\t<string>Portrait</string>";\
			print "\t\t\t<key>UILaunchImageSize</key>";\
			print "\t\t\t<string>{320, 568}</string>";\
			print "\t\t</dict>";\
			print "\t\t<dict>";\
			print "\t\t\t<key>UILaunchImageMinimumOSVersion</key>";\
			print "\t\t\t<string>7.0</string>";\
			print "\t\t\t<key>UILaunchImageName</key>";\
			print "\t\t\t<string>LaunchImage-700-Portrait</string>";\
			print "\t\t\t<key>UILaunchImageOrientation</key>";\
			print "\t\t\t<string>Portrait</string>";\
			print "\t\t\t<key>UILaunchImageSize</key>";\
			print "\t\t\t<string>{768, 1024}</string>";\
			print "\t\t</dict>";\
			print "\t\t<dict>";\
			print "\t\t\t<key>UILaunchImageMinimumOSVersion</key>";\
			print "\t\t\t<string>7.0</string>";\
			print "\t\t\t<key>UILaunchImageName</key>";\
			print "\t\t\t<string>LaunchImage-700-Landscape</string>";\
			print "\t\t\t<key>UILaunchImageOrientation</key>";\
			print "\t\t\t<string>Landscape</string>";\
			print "\t\t\t<key>UILaunchImageSize</key>";\
			print "\t\t\t<string>{768, 1024}</string>";\
			print "\t\t</dict>";\
			print "\t</array>";\
			s=2}\
		s==0 {print $$0}\
		s > 0 { s-- }' $(srcdir)/dists/ios7/Info.plist >$(bundle_name)/Info.plist
	sed -i'' -e 's/$$(PRODUCT_BUNDLE_IDENTIFIER)/org.scummvm.scummvm/' $(bundle_name)/Info.plist
	cp $(DIST_FILES_DOCS) $(bundle_name)/
	cp $(DIST_FILES_THEMES) $(bundle_name)/
ifdef DIST_FILES_NETWORKING
	cp $(DIST_FILES_NETWORKING) $(bundle_name)/
endif
ifdef DIST_FILES_ENGINEDATA
	cp $(DIST_FILES_ENGINEDATA) $(bundle_name)/
endif
	$(STRIP) scummvm
	ldid -S scummvm
	chmod 755 scummvm
	cp scummvm $(bundle_name)/ScummVM
	cp $(srcdir)/dists/ios7/Images.xcassets/AppIcon.appiconset/icon4-29@2x.png $(bundle_name)/AppIcon29x29@2x.png
	cp $(srcdir)/dists/ios7/Images.xcassets/AppIcon.appiconset/icon4-29@2x.png $(bundle_name)/AppIcon29x29@2x~ipad.png
	cp $(srcdir)/dists/ios7/Images.xcassets/AppIcon.appiconset/icon4-29@3x.png $(bundle_name)/AppIcon29x29@3x.png
	cp $(srcdir)/dists/ios7/Images.xcassets/AppIcon.appiconset/icon4-29.png $(bundle_name)/AppIcon29x29~ipad.png
	cp $(srcdir)/dists/ios7/Images.xcassets/AppIcon.appiconset/icon4-40@2x.png $(bundle_name)/AppIcon40x40@2x.png
	cp $(srcdir)/dists/ios7/Images.xcassets/AppIcon.appiconset/icon4-40@2x.png $(bundle_name)/AppIcon40x40@2x~ipad.png
	cp $(srcdir)/dists/ios7/Images.xcassets/AppIcon.appiconset/icon4-40@3x.png $(bundle_name)/AppIcon40x40@3x.png
	cp $(srcdir)/dists/ios7/Images.xcassets/AppIcon.appiconset/icon4-40.png $(bundle_name)/AppIcon40x40~ipad.png
	cp $(srcdir)/dists/ios7/Images.xcassets/AppIcon.appiconset/icon4-60@2x.png $(bundle_name)/AppIcon60x60@2x.png
	cp $(srcdir)/dists/ios7/Images.xcassets/AppIcon.appiconset/icon4-60@3x.png $(bundle_name)/AppIcon60x60@3x.png
	cp $(srcdir)/dists/ios7/Images.xcassets/AppIcon.appiconset/icon4-76@2x.png $(bundle_name)/AppIcon76x76@2x~ipad.png
	cp $(srcdir)/dists/ios7/Images.xcassets/AppIcon.appiconset/icon4-76.png $(bundle_name)/AppIcon76x76~ipad.png
	cp $(srcdir)/dists/ios7/Images.xcassets/AppIcon.appiconset/icon4-83.5@2x.png $(bundle_name)/AppIcon83.5x83.5@2x~ipad.png
	cp $(srcdir)/dists/ios7/Images.xcassets/LaunchImage.launchimage/ScummVM-splash-640x1136-1.png $(bundle_name)/LaunchImage-700-568h@2x.png
	cp $(srcdir)/dists/ios7/Images.xcassets/LaunchImage.launchimage/ScummVM-splash-2048x1536.png $(bundle_name)/LaunchImage-700-Landscape@2x~ipad.png
	cp $(srcdir)/dists/ios7/Images.xcassets/LaunchImage.launchimage/ScummVM-splash-1024x768.png $(bundle_name)/LaunchImage-700-Landscape~ipad.png
	cp $(srcdir)/dists/ios7/Images.xcassets/LaunchImage.launchimage/ScummVM-splash-1536x2048.png $(bundle_name)/LaunchImage-700-Portrait@2x~ipad.png
	cp $(srcdir)/dists/ios7/Images.xcassets/LaunchImage.launchimage/ScummVM-splash-768x1024.png $(bundle_name)/LaunchImage-700-Portrait~ipad.png
	cp $(srcdir)/dists/ios7/Images.xcassets/LaunchImage.launchimage/ScummVM-splash-1242x2208.png $(bundle_name)/LaunchImage-800-Portrait-736h@3x.png
	cp $(srcdir)/dists/ios7/Images.xcassets/LaunchImage.launchimage/ScummVM-splash-2208x1242.png $(bundle_name)/LaunchImage-800-Landscape-736h@3x.png
	cp $(srcdir)/dists/ios7/Images.xcassets/LaunchImage.launchimage/ScummVM-splash-750x1334.png $(bundle_name)/LaunchImage-800-667h@2x.png
