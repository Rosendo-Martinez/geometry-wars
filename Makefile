# To build run: make build
# To delete all binaries run: make clean
# To build & run program run: make run

CXX := g++
CXXFLAGS := -O3 -std=c++17
LDFLAGS := -O3 -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio

# Commands

build : ./bin/Game.exe

clean :
	rm -f ./bin/*.o ./bin/Game.exe

run : build
	./bin/Game.exe

# Executable

./bin/Game.exe : ./bin/main.o ./bin/Entity.o ./bin/EntityManager.o ./bin/Game.o ./bin/Vec2.o
	$(CXX) $(CXXFLAGS) -o ./bin/Game.exe ./bin/main.o ./bin/Entity.o ./bin/EntityManager.o ./bin/Game.o ./bin/Vec2.o $(LDFLAGS)

# Object files (compile from ./src to ./bin)

./bin/main.o : ./src/main.cpp ./src/Game.h ./src/EntityManager.h ./src/Entity.h ./src/Components.h ./src/Vec2.h
	$(CXX) $(CXXFLAGS) -c ./src/main.cpp -o ./bin/main.o

./bin/Entity.o : ./src/Entity.cpp ./src/Entity.h ./src/Components.h ./src/Vec2.h
	$(CXX) $(CXXFLAGS) -c ./src/Entity.cpp -o ./bin/Entity.o

./bin/EntityManager.o : ./src/EntityManager.cpp ./src/EntityManager.h ./src/Entity.h ./src/Components.h ./src/Vec2.h
	$(CXX) $(CXXFLAGS) -c ./src/EntityManager.cpp -o ./bin/EntityManager.o

./bin/Game.o : ./src/Game.cpp ./src/Game.h ./src/EntityManager.h ./src/Entity.h ./src/Components.h ./src/Vec2.h 
	$(CXX) $(CXXFLAGS) -c ./src/Game.cpp -o ./bin/Game.o

./bin/Vec2.o : ./src/Vec2.cpp ./src/Vec2.h
	$(CXX) $(CXXFLAGS) -c ./src/Vec2.cpp -o ./bin/Vec2.o