# This is the default target, which will be built when 
# you invoke make
.PHONY: all
all: hello

# This rule tells make how to build hello from hello.cpp
.PHONY: build
build: 
	g++ -o ./bin/soundstandbyblocker ./src/soundstandbyblocker.c -lpulse-simple -lpulse

# This rule tells make to copy hello to the binaries subdirectory,
# creating it if necessary
.PHONY: install
install:
	echo "copy files ..."

# This rule tells make to delete hello and hello.o
.PHONY: clean 
clean:
	rm ./bin/soundstandbyblocker