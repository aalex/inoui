#!/bin/bash
#
# Combines two stereo wav files into a single 4-channel wav file.
#

if [ $# -ne 1 ];
then
    echo "Usage: $0 <prefix>"
    exit 1
fi

set -o verbose 
ecasound \
    -f:16,2,48000 \
    -a:1 -i ${1}F.wav \
    -a:2 -i ${1}R.wav \
    -f:16,4,48000 -chmove:1,3 -chmove:2,4 -a:all \
    -o ${1}-quad.wav
