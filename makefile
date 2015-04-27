all:
	g++ -g server.cpp -o my_ftp

clean:
	rm -f ./my_ftp
	rm -f *.0
	echo "Hello, World!" > hello.txt
