all : a.out main2
a.out: main.cpp
	g++ -pedantic-errors -g main.cpp -lsqlite3 -o a.out
main2: main2.cpp
	g++ -pedantic-errors -g main2.cpp -lsqlite3 -o main2

clean :
	rm a.out
	rm main2