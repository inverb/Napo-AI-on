CXX_FLAGS=-O2 -std=c++17 -Wall -Wextra

# -DNDEBUG


all: prepare artifacts/random artifacts/basic artifacts/heuristic1 artifacts/flat_mc artifacts/genetic_new

prepare: artifacts/
	mkdir -p artifacts

artifacts/random: random.cpp
	g++ $(CXX_FLAGS) random.cpp -o artifacts/random

artifacts/basic: basic.cpp
	g++ $(CXX_FLAGS) basic.cpp -o artifacts/basic

artifacts/heuristic1: heuristic1.cpp
	g++ $(CXX_FLAGS) heuristic1.cpp -o artifacts/heuristic1

artifacts/flat_mc: flat_mc.cpp
	g++ $(CXX_FLAGS) flat_mc.cpp -o artifacts/flat_mc

artifacts/genetic: genetic.cpp
	g++ $(CXX_FLAGS) genetic.cpp -o artifacts/genetic
