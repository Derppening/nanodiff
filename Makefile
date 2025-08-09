CXX := g++
CXXFLAGS := -std=c++23 -Wall -Wextra -Werror=pedantic -pedantic-errors

default:
	g++ ${CXXFLAGS} -O2 -o nanodiff nanodiff.cpp

debug:
	g++ ${CXXFLAGS} -ggdb -fsanitize=address,leak,undefined -o nanodiff nanodiff.cpp

clean:
	rm -f nanodiff
