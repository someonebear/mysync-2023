PROJECT = mysync
HEADERS = $(PROJECT).h
OBJ = mysync.o readinput.o globals.o hashmap.o files.o glob2regex.o

C11 = cc -std=c11
CFLAGS = -Wall -Werror

$(PROJECT) : $(OBJ)
	$(C11) $(CFLAGS) -g -o $(PROJECT) $(OBJ)

%.o : %.c $(HEADERS)
	$(C11) $(CFLAGS) -g -c $<

clean:
	rm -f $(PROJECT) $(OBJ)