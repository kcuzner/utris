# uTris (micro-tris)
By Kevin Cuzner

## Overview

This is a 1KB implementation of the classic falling blocks game using trominos
instead of tetrominos. It is written entirely in C. It can be easily adapted to
use the traditional tetrominos, provided that 140 bytes of extra flash could be
found for the bitmap (or a better bitmap format invented which isn't so
wasteful).

## Schematic

This is probably my least specific schematic I've ever made. Some notes:

 * All output enables are asserted (tie to ground for most chips)
 * All memory reset pins are de-asserted (tie to VCC for most chips)
 * All chips are powered from 5V
 * All chips have their very own decoupling capacitor. Probably a little
   overboard, but don't forget it for the microcontroller.
 * I didn't include the programming connections because that's fairly easy to
   google.

![Schematic](/doc/Schematic.jpg?raw=true "Schematic")

## Breadboard Layout

![Layout](/doc/Layout.jpg?raw=true "Layout")

