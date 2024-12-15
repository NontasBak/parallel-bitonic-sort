# Compiler
CC = mpicc

# Compiler flags
CFLAGS = -std=c11 -Wall -Iinclude -O3

# Source files
SRCS = main.c bitonicSort.c bitonicSequence.c

# Object files (replace .c with .o)
OBJS = $(SRCS:.c=.o)

# Target executable
TARGET = bitonic

# Default rule
all: $(TARGET)

# Link object files to create the executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET) -lm

# Compile source files to object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up generated files
clean:
	rm -f $(OBJS) $(TARGET)

# Run the program with LD_LIBRARY_PATH set
run: $(TARGET)
	mpirun -np 4 ./$(TARGET)

# Specify phony targets
.PHONY: all clean run