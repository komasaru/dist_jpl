#gcc_options = -std=c++17 -Wall -O2 --pedantic-errors -fsanitize=address
gcc_options = -std=c++17 -Wall -O2 --pedantic-errors

dist_jpl: dist_jpl.o jpl.o
	g++92 $(gcc_options) -o $@ $^

dist_jpl.o : dist_jpl.cpp
	g++92 $(gcc_options) -c $<

jpl.o : jpl.cpp
	g++92 $(gcc_options) -c $<

run : dist_jpl
	./dist_jpl

clean :
	rm -f ./dist_jpl
	rm -f ./*.o

.PHONY : run clean

