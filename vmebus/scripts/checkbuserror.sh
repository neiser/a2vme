./reset_buserror.sh
echo "erzeuge BusError im Standart-Modus an Adresse 0xf10000"
./vmestd32 f10000 0 r
./read_status_register.sh
./read_buserror_register.sh
./reset_buserror.sh
echo "erzeuge BusError im Extended-Modus an Adresse 0xf3210000"
./vmeext f3210000 0 r
./read_status_register.sh
./read_buserror_register.sh
./reset_buserror.sh

