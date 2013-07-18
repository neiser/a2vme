#!/bin/sh
#
# Restart all VUPROM modules in Tagger rack
# 

echo ""
echo "************************************"
echo "* Restart all modules              *"
echo "* for Pair Spectrometer            *"
echo "* Tagger 1209270f                  *"
echo "************************************"
echo ""

vuprom3 -r 1
vuprom3 -r 2
vuprom3 -r 3
vuprom3 -r 4
vuprom3 -r 5

echo "Wait 10 seconds to complete restart."
sleep 10
echo "Check Firmware:"
echo -n "Module x01 (expected: 1209270f): " ; ./vmeext2x.sh 0x01002f00 0x0 r ; echo " "
echo -n "Module x02 (expected: 1209270f): " ; ./vmeext2x.sh 0x02002f00 0x0 r ; echo " "
echo -n "Module x03 (expected: 1209270f): " ; ./vmeext2x.sh 0x03002f00 0x0 r ; echo " "
echo -n "Module x04 (expected: 1209270f): " ; ./vmeext2x.sh 0x04002f00 0x0 r ; echo " "
echo -n "Module x05 (expected: 11030303): " ; ./vmeext2x.sh 0x05002f00 0x0 r ; echo " "

echo "Set master module only to let signals from module x02 through..."
echo -n "Response: "; ./vmeext2x.sh 0x05002500 0x2 w; echo " "

echo -n "Check response: "; ./vmeext2x.sh 0x05002500 0x0 r; echo " "

echo " "
echo "Restart complete."
echo " "
