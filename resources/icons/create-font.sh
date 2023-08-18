#!/bin/sh

DIR=$(git rev-parse --show-toplevel)/resources/icons

fontforge -lang=ff -c 'Open($1); Generate($2)' $DIR/source.svg $DIR/icons-font.otf
