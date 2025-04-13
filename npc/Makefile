TOPNAME = top

VERILATOR = verilator
VERILATOR_FLAGS = --cc --exe --build -j 0 -Wall --top-module $(TOPNAME)
VERILATOR_FLAGS += -Wno-DECLFILENAME -Wno-UNUSEDSIGNAL
# VERILATOR_FLAGS += --trace

BUILD_DIR = ./build
OBJ_DIR = ./obj_dir

EXE = $(addprefix $(OBJ_DIR)/V,$(TOPNAME))

CSRCS = $(abspath $(wildcard ./csrc/*.cpp))
VSRCS = $(abspath $(wildcard ./vsrc/*.v))

all:
	@echo "Write this Makefile by your self."

sim:
	$(VERILATOR) $(VERILATOR_FLAGS) \
		--Mdir $(OBJ_DIR) \
		$(CSRCS) $(VSRCS)
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	$(EXE)

clean:
	rm -rf $(BUILD_DIR)/
	rm -rf $(OBJ_DIR)/

include ../Makefile
