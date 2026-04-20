# Sources to include in the main application
SRCS = Ultima.cpp Sema.cpp Sched.cpp IPC.cpp WindowManager.cpp MMU.cpp
OBJS = $(SRCS:%.cpp=build/%.o) # Define a .o file for every .cpp file

# Sources to include for unit testing
TEST_SRCS = UnitTest.cpp Sema.cpp Sched.cpp IPC.cpp MMU.cpp
TEST_OBJS = $(TEST_SRCS:%.cpp=build/%.o)

# Entry point for the `make` command
all: Ultima

# Compile the final program
Ultima: $(OBJS)
	g++ $^ -o $@ -lncurses

# Compile every .cpp file, storing them in a build folder
build/%.o: %.cpp Log.h Queue.h | build
	g++ -c $< -o $@

# Create build folder if it doesn't exist
build:
	mkdir -p build

# Removes all built material when `make clean` is run
clean:
	rm -rf build
	rm -f Ultima UnitTest

# With this, entering `make run` will compile and immediately run the program
run: Ultima
	./Ultima

# Build and run unit tests
test: $(TEST_OBJS)
	g++ $^ -o UnitTest -lncurses
	./UnitTest

