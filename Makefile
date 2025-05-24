CC = gcc
CFLAGS_DEBUG = -g -ggdb -pthread -std=c17 -pedantic -W -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable
CFLAGS_RELEASE = -std=c17 -pthread -pedantic -W -Wall -Wextra -Werror -Wno-unused-parameter -Wno-unused-variable
LDFLAGS = -pthread

SRC_SERVER_DIR = src/server
SRC_CLIENT_DIR = src/client

BUILD_DIR = build

DEBUG_SERVER_DIR = $(BUILD_DIR)/debug/server
RELEASE_SERVER_DIR = $(BUILD_DIR)/release/server

DEBUG_CLIENT_DIR = $(BUILD_DIR)/debug/client
RELEASE_CLIENT_DIR = $(BUILD_DIR)/release/client

SRC_SERVER_FILES = $(SRC_SERVER_DIR)/main.c $(SRC_SERVER_DIR)/server.c $(SRC_SERVER_DIR)/func.c
SRC_CLIENT_FILES = $(SRC_CLIENT_DIR)/main.c $(SRC_CLIENT_DIR)/client.c

OBJ_FILES_SERVER_DEBUG = $(patsubst $(SRC_SERVER_DIR)/%.c, $(DEBUG_SERVER_DIR)/%.o, $(SRC_SERVER_FILES))
OBJ_FILES_SERVER_RELEASE = $(patsubst $(SRC_SERVER_DIR)/%.c, $(RELEASE_SERVER_DIR)/%.o, $(SRC_SERVER_FILES))

OBJ_FILES_CLIENT_DEBUG = $(patsubst $(SRC_CLIENT_DIR)/%.c, $(DEBUG_CLIENT_DIR)/%.o, $(SRC_CLIENT_FILES))
OBJ_FILES_CLIENT_RELEASE = $(patsubst $(SRC_CLIENT_DIR)/%.c, $(RELEASE_CLIENT_DIR)/%.o, $(SRC_CLIENT_FILES))

EXEC_SERVER_NAME = server
EXEC_CLIENT_NAME = client

DEBUG_TARGETS = $(DEBUG_SERVER_DIR)/$(EXEC_SERVER_NAME) \
				$(DEBUG_CLIENT_DIR)/$(EXEC_CLIENT_NAME)

RELEASE_TARGETS = $(RELEASE_SERVER_DIR)/$(EXEC_SERVER_NAME) \
				  $(RELEASE_CLIENT_DIR)/$(EXEC_CLIENT_NAME)

all: debug

debug: $(DEBUG_TARGETS)

release: $(RELEASE_TARGETS)

define compile_rule
$2/%.o: $1/%.c | $2/
	$(CC) $3 -c $$< -o $$@
endef

$(eval $(call compile_rule,$(SRC_SERVER_DIR),$(DEBUG_SERVER_DIR),$(CFLAGS_DEBUG)))
$(eval $(call compile_rule,$(SRC_SERVER_DIR),$(RELEASE_SERVER_DIR),$(CFLAGS_RELEASE)))

$(eval $(call compile_rule,$(SRC_CLIENT_DIR),$(DEBUG_CLIENT_DIR),$(CFLAGS_DEBUG)))
$(eval $(call compile_rule,$(SRC_CLIENT_DIR),$(RELEASE_CLIENT_DIR),$(CFLAGS_RELEASE)))

define link_rule
$2/$3: $1
	$(CC) $(LDFLAGS) $$^ -o $$@
endef

$(eval $(call link_rule,$(OBJ_FILES_SERVER_DEBUG),$(DEBUG_SERVER_DIR),$(EXEC_SERVER_NAME)))
$(eval $(call link_rule,$(OBJ_FILES_SERVER_RELEASE),$(RELEASE_SERVER_DIR),$(EXEC_SERVER_NAME)))

$(eval $(call link_rule,$(OBJ_FILES_CLIENT_DEBUG),$(DEBUG_CLIENT_DIR),$(EXEC_CLIENT_NAME)))
$(eval $(call link_rule,$(OBJ_FILES_CLIENT_RELEASE),$(RELEASE_CLIENT_DIR),$(EXEC_CLIENT_NAME)))

%/:
	mkdir -p $@

clean:
	rm -fr $(BUILD_DIR)

.PHONY: all debug release clean