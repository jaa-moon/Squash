all:
	g++ -o squash main.cpp compress.cpp decompress.cpp stats.cpp

clean:
	rm -f squash
