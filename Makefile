FILES = main.c
EXE = c_baccarat

linux:
	gcc -o $(EXE) $(FILES) -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -Wextra -O2

windows:
	gcc -o $(EXE).exe $(FILES) -lraylib  -lgdi32 -lwinmm -O2 

debug:
	gcc -g -Wextra -O2 -o $(EXE)_debug $(FILES) -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

clean:
	rm ./$(EXE)
