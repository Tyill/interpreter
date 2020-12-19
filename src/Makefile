CC = g++
CFLAGS = -Wall -I/usr/local/include
LDFLAGS = -lgtest -pthread
SOURCES = test.cpp interpreter.cpp
OBJECTS = $(SOURCES:.cpp=.o)
PREFIX = /usr/local/lib
TARGET = irtest
   
# Debug build settings
DBGDIR = Debug
DBGTRT = $(DBGDIR)/$(TARGET)
DBGOBJS = $(addprefix $(DBGDIR)/, $(OBJECTS))
DBGCFLAGS = -g -O0 -DDEBUG

# Release build settings
RELDIR = Release
RELTRT = $(RELDIR)/$(TARGET)
RELOBJS = $(addprefix $(RELDIR)/, $(OBJECTS))
RELCFLAGS = -O3 -DNDEBUG

.PHONY: all debug release prep remake clean install uninstall

# Default build
all: prep release

# Debug rules
debug: $(DBGTRT)
$(DBGTRT): $(DBGOBJS)
	$(CC) $^ $(LDFLAGS) -o $@
$(DBGDIR)/%.o: %.cpp
	$(CC) -c $(CFLAGS) $(DBGCFLAGS) $< -o $@

# Release rules
release: $(RELTRT)
$(RELTRT): $(RELOBJS)
	$(CC) $^ $(LDFLAGS) -o $@
$(RELDIR)/%.o: %.cpp
	$(CC) -c $(CFLAGS) $(RELCFLAGS) $< -o $@

# Other rules
prep:
	@mkdir -p $(DBGDIR) $(RELDIR)

remake: clean all

clean:
	rm -f $(RELTRT) $(RELOBJS) $(DBGTRT) $(DBGOBJS)
install:
	install ./Release/$(TARGET) $(PREFIX)
uninstall:
	rm -rf  $(PREFIX)/$(TARGET)