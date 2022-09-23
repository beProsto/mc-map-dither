ifeq ($(OS), Windows_NT)
RM = del
EXE = dither.exe
else
RM = rm
EXE = ./dither
endif

all:
	gcc ./src/main.c -lm -o $(EXE)

test:
ifneq ($(OS), Windows_NT)
	chmod +x dither
endif
	$(EXE) img/input.png img/output.png

clean:
	$(RM) $(EXE)