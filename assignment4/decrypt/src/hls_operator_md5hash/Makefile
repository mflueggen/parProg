SOLUTION_NAME ?= hls_operator_md5hash_sln
SOLUTION_DIR ?= hls_operator_md5hash_sln

HLS_CFLAGS += -I$(METAL_ROOT)/src/metal_fpga/hw/hls/include

srcs += hls_operator_md5hash.cpp

WRAPPER ?= hls_operator_md5hash

include $(SNAP_ROOT)/actions/hls.mk
include $(METAL_ROOT)/src/metal_fpga/hw/hls/hls.mk
