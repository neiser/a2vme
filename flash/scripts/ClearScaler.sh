#!/bin/sh
echo ""
echo "************************************"
echo "* Clear Scalers of all modules     *"
echo "* for Pair Spectrometer            *"
echo "* Tagger 1209270f                  *"
echo "************************************"
echo ""

echo -n "Module x01: " ; ./vmeext2x.sh 0x01001800 0x1 w ; echo " "
echo -n "Module x02: " ; ./vmeext2x.sh 0x02001800 0x1 w ; echo " "
echo -n "Module x03: " ; ./vmeext2x.sh 0x03001800 0x1 w ; echo " "
echo -n "Module x04: " ; ./vmeext2x.sh 0x04001800 0x1 w ; echo " "
echo -n "Module x05: " ; ./vmeext2x.sh 0x05001800 0x1 w ; echo " "


echo " "
echo "Clear complete."
echo " "
