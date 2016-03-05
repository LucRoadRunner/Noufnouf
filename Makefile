roomba : roomba.c roomba.h
	gcc -I . -lm -g -o roomba roomba.c
roombad : roomba.c roomba.h
	gcc -I . -lm -g -o roombad roombad.c

roomba_client : roomba_client.c roomba.h
	gcc -I . -lm -g -o roomba_client roomba_client.c

endian: endian_test.c
	gcc -o endian endian_test.c

all    : roomba roomba_client
	cp *.php /var/www/html/bikini
	cp *.html /var/www/html/bikini
	cp roomba.sh /usr/local/bin

