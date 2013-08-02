# for FreeBSD
# cd /usr/ports/ftp/wget
# make install clean

echo "infinite loop [ hit CTRL+C to stop]"

while :
do
	echo -n "Getting data via http... "
	echo "http://a2ortegapc.online.a2.kph:8080/slow_control/station/a2" > URL.txt

	wget -i URL.txt -o log -O - > Response.txt
	echo -n "done. "
	rm URL.txt

	echo -n "http://a2onlinedatabase.office.a2.kph/intern/sc/insert.php?InsertNew=-1&NodeID=10&" > ResponseValuesURL.txt

	awk -v MaxLine=8 -f awkLines.txt Response.txt >> ResponseValuesURL.txt

	#echo ""
	#cat ResponseValuesURL.txt
	#echo ""


	wget -i ResponseValuesURL.txt -o log -O - > ResponseValues.txt
	echo -n "done. "
	rm ResponseValuesURL.txt


	echo "Got Data."

	sleep 60

done

