all: pencil

pencil.tab.c pencil.tab.h:	pencil.y
	bison -t -v -d pencil.y

lex.yy.c: pencil.l pencil.tab.h
	flex pencil.l

pencil: lex.yy.c pencil.tab.c pencil.tab.h
	g++ -o pencil pencil.tab.c lex.yy.c -w

clean:
	rm pencil pencil.tab.c lex.yy.c pencil.tab.h pencil.output
