CC = gcc
CFLAGS = -Wall -W -g 
LDFLAGS = -lX11 -lXext -lm
# pour windows :
# LDFLAGS = -lgdi32 -lmsimg32
SRC = flappy.c ez-draw.c ez-image.c

flappy: $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o flappy $(LDFLAGS)

clean:
	rm -f flappy
