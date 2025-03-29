VERILATOR = verilator
VERILATOR_FLAGS = --cc --exe --build -j 0 -Wall

all:
	@echo "Write this Makefile by your self."

sim:
	$(VERILATOR) $(VERILATOR_FLAGS) ./csrc/main.cpp ./vsrc/example.v
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	@echo "Write this Makefile by your self."

include ../Makefile
