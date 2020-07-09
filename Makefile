.PHONY: run clean debug

CFLAGS := -O2 -Wall 
SRCS := $(shell find . -maxdepth 1 -name "*.c")
INCS := $(shell find . -maxdepth 1 -name "*.h") $(SRCS)
# 这个地方搞了我半天: https://stackoverflow.com/questions/3783149/how-do-i-properly-reference-the-gnu-readline-library-to-scan-terminal-input
LDFLAGS := -L/usr/local/lib -I/usr/local/include -lreadline

run: $(INCS)
	gcc $(CFLAGS) $(SRCS) -o runsh $(LDFLAGS)

debug: $(INCS)
	gcc $(CFLAGS) $(SRCS) -o runsh -D DEBUG $(LDFLAGS)

clean:
	rm -rf *.o runsh