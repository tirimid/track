.PHONY: all install uninstall

CPP := g++
CPP_FLAGS := \
	-std=c++20 \
	-pedantic \
	-Wall \
	-Wextra \
	-Wshadow \
	-fno-rtti \
	-fno-exceptions \
	-O3

INSTALL_DIR := /usr/bin

all: track

install: track
	cp $< $(INSTALL_DIR)

uninstall:
	rm $(INSTALL_DIR)/track

track: track.cc
	$(CPP) -o $@ $< $(CPP_FLAGS)
