all: rshell

rshell: BaseCommand.o main.o
  g++ BaseCommand.o main.o -Wall -Werror -ansi -pedantic -o rshell
BaseCommand.o: BaseCommand.cc
  g++ BaseCommand.cc -c -Wall -Werror -ansi -pedantic
main.o: main.cc
  g++ main.cc -c -Wall -Werror -ansi -pedantic
