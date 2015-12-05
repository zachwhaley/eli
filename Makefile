CFLAGS = -g -Wall
LDLIBS = -lncurses

PRGM  = eli
SRCS := $(wildcard *.c)
OBJS := $(SRCS:.c=.o)
DEPS := $(OBJS:.o=.d)

all: $(PRGM)

$(PRGM): $(OBJS)
	$(CC) $(OBJS) $(LDLIBS) -o $(PRGM)

%.o: %.c
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

clean:
	rm -rf $(OBJS) $(DEPS) $(PRGM)

-include $(DEPS)
