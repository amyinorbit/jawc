#
#  Makefile - Use this to compile your CompilerKit-based project
#  CompilerKit
#
#  Created by Amy Parent on 25/01/2022.
#

TARGET := jawc
SRC_DIR := ./src
TMP_DIR := build

CFLAGS := -g3 -ggdb -O0 -std=c17 -Wall -Werror
LDFLAGS := -g3 -ggdb -O0 -std=c17

ALL_SRC := $(wildcard $(SRC_DIR)/*.c)
ALL_HDR := $(wildcard $(SRC_DIR)/*.h)

ALL_OBJ := $(patsubst $(SRC_DIR)/%, $(TMP_DIR)/%, $(ALL_SRC:.c=.o))

.PHONY: directories clean

$(TARGET): directories $(ALL_OBJ) $(ALL_HDR)
	$(CXX) -o $(TARGET) $(ALL_OBJ) $(LDFLAGS)

clean:
	rm -rf $(TMP_DIR)
	rm -f $(TARGET)

directories: $(OUT_DIR) $(TMP_DIR)

$(OUT_DIR):
	mkdir -p $(OUT_DIR)
	
$(TMP_DIR):
	mkdir -p $(TMP_DIR)

$(TMP_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<
	