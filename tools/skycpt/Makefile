# This worked for me
# Sorry if it doesn't work for you, but I don't know how to write Makefiles.
# After all, I'm only a windows user.
# I would've liked to test it on linux, but I couldn't find the Start button.

TARGET=TXT2CPT
OBJS=AsciiCptCompile.o cptcompiler.o cpthelp.o idFinder.o KmpSearch.o stdafx.o TextFile.o

all: $(TARGET)

clean:
	$(QUIET)$(RM) $(TARGET) $(OBJS)

$(TARGET): $(OBJS)
	$(QUIET_CXX)$(CXX) $(OBJS) $(LDFLAGS) -o $(TARGET)

%.o: %.cpp
	$(QUIET_CXX)$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@
