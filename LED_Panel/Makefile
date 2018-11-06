CC = gcc
CFLAGS  = -Wall
INCLUDES = -I/usr/local/include/blib-1.1 `pkg-config --cflags --libs gtk+-2.0`
LIBS = -L/usr/local/lib -lb -lm -lrt -lpthread -lgobject-2.0 -lglib-2.0

blinkdebug: blinkdebug.c
	$(CC) $(CFLAGS) $(INCLUDES) blinkdebug.c -o blinkdebug $(LIBS)

blinkpanel: blinkpanel.c
	$(CC) $(CFLAGS) $(INCLUDES) blinkpanel.c -o blinkpanel librgbmatrix.a $(LIBS) -lstdc++
