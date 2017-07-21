#!/bin/sh

for size in 16 32 48 64 96 128 256
do
    inkscape ../resources/images/application-icon.svg \
        -e icon-$size.png -w $size -h $size
done

convert \
    -background transparent \
    *.png \
    ../win/appicon.ico

rm icon-*.png
