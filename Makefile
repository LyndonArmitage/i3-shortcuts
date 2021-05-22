CFLAGS= -lm -Wall -std=c99 -D_DEFAULT_SOURCE -Wno-missing-braces
BIN=i3-shortcuts

${BIN}: main.c 
	${CC} ${CFLAGS} main.c -o ${BIN}

run: ${BIN}
	./${BIN}

.PHONY: clean

clean:
	${RM} *.o ${BIN}

