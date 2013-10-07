vmeext $1 $2 r > /dev/null
vmeext $1 $2 $3 > /dev/null
echo -n `vmeext $1 $2 $3 | cut -f 6 -d " " `
echo ""