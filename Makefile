
CXXFLAGS = -std=c++17 -W -Wall -O3 -ffast-math -fno-exceptions -fno-rtti -I../code
CXX = clang++ -stdlib=libc++ -march=native
#CXX = g++ -march=native

.PHONY: all

all: testbench

test: testbench
	./testbench

testbench: testbench.cc edwards_curve.hh
	$(CXX) $(CXXFLAGS) $< -o $@

.PHONY: clean

clean:
	rm -f testbench

