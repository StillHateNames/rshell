MK_DIR = test -d bin || mkdir bin
all: MK_DIR ./bin/rshell

MK_DIR:
	$(MK_DIR)
./bin/rshell: ./bin/BaseCommand.o ./bin/main.o ./bin/Tests.o
	g++ ./bin/BaseCommand.o ./bin/main.o ./bin/Tests.o -Wall -Werror -ansi -pedantic -o ./bin/rshell
./bin/BaseCommand.o: ./src/BaseCommand.cc
	g++ ./src/BaseCommand.cpp -c -Wall -Werror -ansi -pedantic -o ./bin/BaseCommand.o
./bin/main.o: ./src/main.cpp
	g++ ./src/main.cpp -c -Wall -Werror -ansi -pedantic -o ./bin/main.o
./bin/Tests.o: ./src/Tests.cpp
	g++ ./src/Tests.cpp -c -Wall -Werror -ansi -pedantic -o ./bin/Tests.o
