CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g -D_GNU_SOURCE
LDFLAGS = 
LIBS = 
SRCDIR = src
INCDIR = include
TARGET = a.out

# Source files
SOURCES = main.c \
          $(SRCDIR)/display.c \
          $(SRCDIR)/tokenize.c \
          $(SRCDIR)/hop.c \
          $(SRCDIR)/reveal.c \
          $(SRCDIR)/log.c \
          $(SRCDIR)/system.c \
          $(SRCDIR)/proclore.c \
          $(SRCDIR)/seek.c \
          $(SRCDIR)/redirection.c \
          $(SRCDIR)/pipes.c \
          $(SRCDIR)/activities.c \
          $(SRCDIR)/signals.c \
          $(SRCDIR)/fg_bg.c \
          $(SRCDIR)/neonate.c \
          $(SRCDIR)/iman.c \
          $(SRCDIR)/myshrc.c

# Object files
OBJECTS = $(SOURCES:.c=.o)

# Include directories
INCLUDES = -I$(INCDIR)

# Default target
all: $(TARGET)

# Link the executable
$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $(TARGET) $(LIBS)

# Compile source files
%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Clean build artifacts
clean:
	rm -f $(OBJECTS) $(TARGET)

# Rebuild everything
rebuild: clean all

# Debug build
debug: CFLAGS += -DDEBUG
debug: $(TARGET)

# Install (copy to a system directory)
install: $(TARGET)
	cp $(TARGET) /usr/local/bin/

# Uninstall
uninstall:
	rm -f /usr/local/bin/$(TARGET)

# Run the shell
run: $(TARGET)
	./$(TARGET)

# Show help
help:
	@echo "Available targets:"
	@echo "  all       - Build the shell (default) - generates a.out"
	@echo "  clean     - Remove build artifacts"
	@echo "  rebuild   - Clean and build"
	@echo "  debug     - Build with debug symbols"
	@echo "  run       - Build and run the shell"
	@echo "  install   - Install to /usr/local/bin"
	@echo "  uninstall - Remove from /usr/local/bin"
	@echo "  help      - Show this help message"

.PHONY: all clean rebuild debug install uninstall run help
