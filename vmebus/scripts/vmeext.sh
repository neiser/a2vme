#echo `/home/acqu/VUPROM/vmebus/vmeext $1 $((0x$2)) $3 | cut -f 6 -d " " `
set mytest=`printf '%x' $2`
echo -n "0x"
echo `/home/acqu/VUPROM/vmebus/vmeext $1 $mytest $3 | cut -f 6 -d " " `

