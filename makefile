all:
	g++ -g server.cpp -o my_ftpd

clean:
	rm -f ./my_ftpd
	rm -f *.0
	echo "Hello, World!" > hello.txt
