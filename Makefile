BUILDDIR = build
SRCDIR   = src

SRCS += $(SRCDIR)/main.c

CC      = gcc
CFLAGS += -O0
CFLAGS += -g
CFLAGS += -Wall
CFLAGS += -Wpedantic

default: $(BUILDDIR)/ipv4expand

$(BUILDDIR):
	mkdir -p $@

$(BUILDDIR)/ipv4expand: $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^

.phony: clean
clean:
	rm -rf $(BUILDDIR)
