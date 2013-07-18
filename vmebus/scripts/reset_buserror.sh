echo "read Statusregister"
./mem ac000000 0 r
echo "reset controlbit 7 (Reset Buserror)"
./mem ac000000 80 w
echo "read Statusregister"
./mem ac000000 0 r


