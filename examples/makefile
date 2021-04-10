PROGRAMNAME=program
SOURCECODE=code.pencil
COMPILER=./../pencil

all: $(PROGRAMNAME)
	-

clean:
	rm -rf $(PROGRAMNAME) program.o program.asm *~

program.asm: $(SOURCECODE)
	($(COMPILER) < $(SOURCECODE)) > program.asm

program.o: program.asm
	nasm -f macho64 -o program.o program.asm

$(PROGRAMNAME): program.o program.asm
	gcc -w -o$(PROGRAMNAME) program.o io.c
