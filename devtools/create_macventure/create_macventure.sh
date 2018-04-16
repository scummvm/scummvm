printf "Creating border file...\n"

zip -r macventure.zip *.bmp
mv macventure.zip macventure.dat

echo done

ls -l macventure.dat
