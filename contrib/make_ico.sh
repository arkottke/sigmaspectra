#!/bin/sh

# Source: convert -density 384 -background transparent -fill "#607D8B" -colorize 100 favicon.svg -define icon:auto-resize -colors 256 favicon.ico

convert \
    -density 256x256 \
    -background transparent \
    -fill "#607D8B" \
    -colorize 100 \
    ../resources/images/application-icon.svg \
    -define icon:auto-resize \
    -colors 256 \
    ../win/appicon.ico

