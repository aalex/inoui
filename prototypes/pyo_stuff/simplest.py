#!/usr/bin/env python
"""
Simple - but clean - Pyo example.
"""
import pyo
import math
from twisted.internet import reactor

SAMPLING_RATE = 48000
NUM_CHANNELS = 2

class WxyToMono(object):
    def __init__(self, w, x, y, angle):
        self._mixer = pyo.Mixer(outs=1)
        self._mixer.addInput(0, w)
        self._mixer.addInput(1, x)
        self._mixer.addInput(2, y)
        self._angle = angle
        self._update_amplitudes()

    def set_angle(self, angle):
        self._angle = angle
        self._update_amplitudes()

    def _update_amplitudes(self):
        OUTPUT = 0
        self._mixer.setAmp(0, OUTPUT, 1.0)
        self._mixer.setAmp(1, OUTPUT, math.cos(math.radians(self._angle)))
        self._mixer.setAmp(2, OUTPUT, math.sin(math.radians(self._angle)))

class App(object):
    def __init__(self):
        self._server = pyo.Server(sr=SAMPLING_RATE, nchnls=NUM_CHANNELS).boot()
        self._server.start()
        self._player = pyo.SfPlayer("sounds/snd_1.aif", speed=1.0, loop=True).out()
        reactor.callLater(1.0, self.stop)

    def stop(self):
        if reactor.running:
            reactor.stop()
        self._server.stop()

    def print_infos():
        print(pyo.pa_list_devices())

if __name__ == "__main__":
    app = App()
    reactor.run()

