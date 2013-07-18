#!/bin/sh
echo "read Statusregister bit 7 = buserror bit 2 = AM-Bit 2"
mem ac000000 0 r
