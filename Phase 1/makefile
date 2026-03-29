# List of source files to include
SRCS = Ultima.cpp Queue.cpp Sema.cpp Sched.cpp WindowHelper.cpp
OBJS = $(SRCS:%.cpp=build/%.o) # Define a .o file for every .cpp file

# Entry point for the `make` command
all: Ultima

# Compile the final program
Ultima: $(OBJS)
	g++ $^ -o $@ -lncurses

# Compile every .cpp file, storing them in a build folder
build/%.o: %.cpp | build
	g++ -c $< -o $@

# Create build folder if it doesn't exist
build:
	mkdir -p build

# Removes all built material when `make clean` is run
clean:
	rm -rf build
	rm Ultima

# With this, entering `make run` will compile and immediately run the program
run: Ultima
	./Ultima
