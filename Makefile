CC := gcc
CFLAGS := -Wall -Wextra
LDLIBS := -lz -ludev -lncurses
TARGET := tp

SRCS := $(wildcard *.c)
OBJS := $(SRCS:.c=.o)

VALGRIND := valgrind
VALGRIND_FLAGS := --track-origins=yes --leak-check=full --show-leak-kinds=all --error-exitcode=1

RUN_TARGETS := valgrind run
ifneq ($(filter $(RUN_TARGETS),$(MAKECMDGOALS)),)
ARGS ?= $(filter-out $(RUN_TARGETS),$(MAKECMDGOALS))
$(eval $(ARGS):;@:)
endif
ARGS ?=

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

debug: CFLAGS += -g -O0
debug: clean $(TARGET)

valgrind: debug
	$(VALGRIND) $(VALGRIND_FLAGS) ./$(TARGET) $(ARGS)

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean debug valgrind
