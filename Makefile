CC = g++

CompileParms = -c -Wall

OBJS = algo.o

Opdr: $(OBJS)
	$(CC) $(OBJS) -o algo
	
algo.o: algo.cc
	$(CC) $(CompileParms) algo.cc

