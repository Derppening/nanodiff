CXX := g++
LD := ld
CXXFLAGS := -std=c++23 -Wall -Wextra -Werror=pedantic -pedantic-errors

default:
	g++ ${CXXFLAGS} -O2 -o nanodiff nanodiff.cpp

debug:
	g++ ${CXXFLAGS} -ggdb -o nanodiff nanodiff.cpp

clean:
	rm -f nanodiff
