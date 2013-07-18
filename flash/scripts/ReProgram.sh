echo ""
echo "************************************"
echo "* Reprogram all modules            *"
echo "* for CB                           *"
echo "************************************"
echo ""

echo "Flashing Module xe1..."
./vuprom -rew e1 vuprom_input_processor_1211210c.rbt
echo "completed."

echo "Flashing Module xe2..."
./vuprom -rew e2 vuprom_input_processor_1211210c.rbt
echo "completed."

echo "Flashing Module xe3..."
./vuprom -rew e3 vuprom_input_processor_1211210c.rbt
echo "completed."

echo "Flashing Module xe4..."
./vuprom -rew e4 vuprom_input_processor_1211210c.rbt
echo "completed."

echo "Flashing Module xe5..."
./vuprom -rew e5 vuprom_input_processor_1211210c.rbt
echo "completed."

echo "Flashing Module xe6..."
./vuprom -rew e6 vuprom_input_processor_1211210c.rbt
echo "completed."


echo "Flashing Module xe7..."
./vuprom -rew e7 vuprom_multiplicity_master_12120364.rbt
echo "completed."

echo "Flashing Module xe8..."
./vuprom -rew e8 vuprom_coplanarity_master_1211213c.rbt
echo "completed."


#echo "Flashing Module xea..."
#./vuprom -rew ea vuprom_cbmaster_1211283f.rbt
#echo "completed."



echo " "
echo "Flashing completed."
echo "Wait 10 seconds to complete restart."
sleep 10
echo "Check Firmware:"
echo -n "Module xe1 (expected: 1211210c): " ; ./vmeext2x.sh 0xe1002f00 0x0 r ; echo " "
echo -n "Module xe2 (expected: 1211210c): " ; ./vmeext2x.sh 0xe2002f00 0x0 r ; echo " "
echo -n "Module xe3 (expected: 1211210c): " ; ./vmeext2x.sh 0xe3002f00 0x0 r ; echo " "
echo -n "Module xe4 (expected: 1211210c): " ; ./vmeext2x.sh 0xe4002f00 0x0 r ; echo " "
echo -n "Module xe5 (expected: 1211210c): " ; ./vmeext2x.sh 0xe5002f00 0x0 r ; echo " "
echo -n "Module xe6 (expected: 1211210c): " ; ./vmeext2x.sh 0xe6002f00 0x0 r ; echo " "
echo -n "Module xe7 (expected: 12120364): " ; ./vmeext2x.sh 0xe7002f00 0x0 r ; echo " "
echo -n "Module xe8 (expected: 1211213c): " ; ./vmeext2x.sh 0xe8002f00 0x0 r ; echo " "
#echo -n "Module xea (expected: 1211283f): " ; ./vmeext2x.sh 0xea002f00 0x0 r ; echo " "

echo " "
