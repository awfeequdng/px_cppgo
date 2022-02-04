
OBJS=$(subst *.cc,*.o, $(wildcard *.cc))


all: $(OBJS)
	/usr/bin/clang++ $^ -o golang