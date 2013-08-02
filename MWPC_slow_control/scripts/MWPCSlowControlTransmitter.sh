# for FreeBSD
# cd /usr/ports/ftp/wget
# make install clean

echo "infinite loop [ hit CTRL+C to stop]"

while :
do
	echo -n "Getting data via ftp... "
	echo "ftp://mwpc:a2messung@magrathea.online.a2.kph//home/mwpc/data/cde_ht/cde_htd_monitor.log_1" > URL.txt

	wget -i URL.txt -o log -O - > Response.txt
	echo -n "done. "
	rm URL.txt

# check, whether file is greater than 0 bytes
	MySize=`ls -la Response.txt | awk -f awkLines2.txt `
	if [ $MySize -gt 0 ]; then 
		echo -n "http://a2onlinedatabase.office.a2.kph/intern/sc/insert.php?InsertNew=-1&NodeID=11&" > ResponseValuesURL.txt

		awk -f awkLines.txt Response.txt >> ResponseValuesURL.txt

#		echo ""
#		cat ResponseValuesURL.txt
#		echo ""

		wget -i ResponseValuesURL.txt -o log -O - > ResponseValues.txt
		echo -n "done. "
		rm ResponseValuesURL.txt

		echo "Got Data."
		sleep 1
	else 
		echo "File Size = 0."; 
		sleep 0.2
	fi


done

