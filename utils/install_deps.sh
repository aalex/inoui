#!/bin/bash
# Note: you also need the ambi abstractions from https://github.com/aalex/ambi
sudo apt-add-repository ppa:sat-metalab/metalab
sudo apt-get update
sudo apt-get install libsndfile-dev gettext help2man  libavc1394-dev libboost-program-options-dev libboost-filesystem-dev libboost-thread-dev libboost-date-time-dev libboost-signals-dev libboost-system-dev liblo-dev libclutter-1.0-dev libglib2.0-dev libglu1-mesa-dev libxml2-dev automake bison build-essential flex libtool libclutter-gst-dev puredata

