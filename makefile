.PHONY: all
.DEFAULT_GOAL: all

CFLAGS = -Wall -pedantic -std=gnu99 -g
AUS = austerity.o lib.o game.o token.o deck.o endAusterity.o board.o comms.o \
card.o
SHEN = shenzi.o player.o comms.o lib.o board.o card.o token.o
BANZ = banzai.o player.o comms.o lib.o board.o card.o token.o
ED = ed.o player.o comms.o lib.o board.o card.o token.o

all: austerity shenzi banzai ed

austerity: ${AUS}
	gcc ${AUS} ${CFLAGS} -o austerity

austerity.o: austerity.c
	gcc ${CFLAGS} -c austerity.c

lib.o: lib.c lib.h
	gcc ${CFLAGS} -c lib.c

game.o: game.c game.h
	gcc ${CFLAGS} -c game.c

token.o: token.c token.h
	gcc ${CFLAGS} -c token.c

deck.o: deck.c deck.h
	gcc ${CFLAGS} -c deck.c

board.o: board.c board.h
	gcc ${CFLAGS} -c board.c

endAusterity.o: endAusterity.c endAusterity.h
	gcc ${CFLAGS} -c endAusterity.c

comms.o: comms.c comms.h
	gcc ${CFLAGS} -c comms.c

card.o: card.c card.h
	gcc ${CFLAGS} -c card.c

shenzi: ${SHEN}
	gcc ${SHEN} ${CFLAGS} -o shenzi

shenzi.o: shenzi.c
	gcc ${CFLAGS} -c shenzi.c

banzai: ${BANZ}
	gcc ${BANZ} ${CFLAGS} -o banzai

banzai.o: banzai.c
	gcc ${CFLAGS} -c banzai.c

ed: ${ED}
	gcc ${ED} ${CFLAGS} -o ed

ed.o: ed.c
	gcc ${CFLAGS} -c ed.c

player.o: player.c player.h
	gcc ${CFLAGS} -c player.c

clean:
	rm *.o austerity shenzi banzai ed