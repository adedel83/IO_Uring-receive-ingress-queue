CC := gcc

URING := -I./liburing-master/src/include -L./liburing-master/src -luring -D_GNU_SOURCE


EXE := A E 


all:A E 

A:src/A.c 
	$(CC) -o $@ $^ 

E:src/E.c $(lib)
	$(CC) -o $@ $^  $(URING) 

clean:
	rm A E 

trash:
	rm  $(EXE)
.PHONY: clean
