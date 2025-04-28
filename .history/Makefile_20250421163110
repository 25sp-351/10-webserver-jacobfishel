# Variables
CC = gcc
CFLAGS = -Wall -Wextra -g
TARGET = server
OBJS = main.o functions.o

# Default target
all: $(TARGET)

# How to build the final executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

# How to build object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up compiled files
clean:
	rm -f $(OBJS) $(TARGET)
