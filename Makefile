CFLAGS = -std=gnu11 -g -ggdb -Wall -Werror -MMD -MP
LDLIBS = -lncurses

PRGM  = eli
SRCS := $(wildcard *.c)
OBJS := $(SRCS:.c=.o)
DEPS := $(OBJS:.o=.d)

$(PRGM): $(OBJS)

.PHONY: clean
clean:
	$(RM) $(OBJS) $(DEPS) $(PRGM)

-include $(DEPS)
