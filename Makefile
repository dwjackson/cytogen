CFLAGS = -Wall -g -O0 -I$(HOME)/localapps/ctache/include -L$(HOME)/localapps/ctache/lib

EXE_NAME = cyto
OBJ_FILES = main.o layout.o
SRC_FILES = main.c layout.c

all: $(EXE_NAME)

$(EXE_NAME): $(OBJ_FILES)
	cc $(CFLAGS) -o $(EXE_NAME) $(OBJ_FILES) -lctache

$(OBJ_FILES): $(SRC_FILES)
	cc $(CFLAGS) -c $(SRC_FILES)

clean:
	rm -f $(EXE_NAME)
	rm *.o

.PHONY: all clean
