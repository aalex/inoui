#!/bin/bash
pd \
    -jack \
    -nomidi \
    -channels 4 \
    -r 48000 \
    inoui.tests.pd 
