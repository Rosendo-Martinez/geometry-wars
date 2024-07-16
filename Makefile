# To build run: make
# To delete all binaries run: make clean
# To build & run progam run: make run

CXX := g++
OUTPUT := sfmlgame

CXX_FLAGS := -o3 -std=c++17
LDFLAGS := -o3 -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio

SRC_FILES := $(wildcard ./*.cpp) # find all .cpp files
OBJ_FILES := $(SRC_FILES:.cpp=.o) # for all SRC_FILES replace .cpp with .o

all:$(OUTPUT)

$(OUTPUT): $(OBJ_FILES)
	$(CXX) $(OBJ_FILES) $(LDFLAGS) -o ./bin/$@

$(OBJ_FILES) : %.o : %.cpp
	$(CXX) $(CXX_FLAGS) -c $^ -o $@

clean:
	rm -f $(OBJ_FILES) ./bin/$(OUTPUT)

run: $(OUTPUT)
	cd ./bin && ./sfmlgame