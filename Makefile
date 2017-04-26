HEADERS := $(shell find spea2 -name \*.hpp)
SRC := $(shell find test -name \*.cpp) $(HEADERS)

all: test

test:
	@$(MAKE) --no-print-directory -C test
	@echo "Running test suite..."
	@valgrind --error-exitcode=1 --leak-check=full test/test_suite -d yes

format:
	@echo Formatting source...
	@clang-format -i -style=file $(SRC)

tidy:
	@echo Tidying source...
	@clang-tidy $(HEADERS) -fix -fix-errors -- -std=c++1z -I.

clean:
	@$(MAKE) --no-print-directory -C test clean

.PHONY: format test clean tidy
