~vmebus/vmebus/vmeext $1 $2 r > /dev/null
~vmebus/vmebus/vmeext $1 $2 $3 > /dev/null
echo -n `~vmebus/vmebus/vmeext $1 $2 $3 | cut -f 6 -d " " `