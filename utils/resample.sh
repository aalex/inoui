#!/bin/bash
for i in {0..9}; do sox 2nd-SR00${i}-quad.wav -r 48000 -b 24 2nd-SR00${i}-quad-48k.wav ; done
