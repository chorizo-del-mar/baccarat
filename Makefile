FILES = main.c
EXE = c_baccarat

$(EXE):
	gcc -Wextra -O2 -o $(EXE) $(FILES) -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

windows:
	x86_64-w64-mingw32-gcc -O2 -o $(EXE).exe $(FILES) -lraylib -lgdi32 -lwinmm

debug:
	gcc -g -Wextra -O2 -o $(EXE)_debug $(FILES) -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

clean:
	rm ./$(EXE)
