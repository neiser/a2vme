cd /home/vmebus/ExperimentSlowControl
nohup ./RunSlowControl.sh > /dev/null &
nohup ./CBSlowControlTransmitter.sh  > /dev/null &
cd MWPC
nohup ./MWPCSlowControlTransmitter.sh  > /dev/null &
