SUFFIXES += .d

SRC = simple.cpp knapsack.cpp

OBJ := $(SRC:.cpp=.o) test_suite.o
DEP := $(SRC:.cpp=.d) test_suite.d

CXXFLAGS += -O2 -std=c++14 -Wall -Wextra -pedantic -I..

NODEPS = clean

all: test_suite

ifeq (0, $(words $(findstring $(MAKECMDGOALS), $(NODEPS))))
-include $(DEP)
endif

test_suite: $(OBJ)
	@echo "Linking test suite..."
	@$(CXX) $(CXXFLAGS) -o $@ $(OBJ) $(LDFLAGS)

%.o: %.cpp
	@echo "Compiling ($<)..."
	@$(CXX) $(CXXFLAGS) -c -o $@ $<

%.d: %.cpp
	@echo "Finding dependencies ($<)..."
	@$(CXX) $(CXXFLAGS) -MM -MT '$(patsubst %.cpp,%.o,$<)' $< -MF $@

clean:
	@echo "Cleaning test..."
	@find . -name '*.[od]' -exec rm {} \;

.PHONY: test clean
