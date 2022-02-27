CC = g++
CFLAGS  +=  -g \
			-I/usr/local/include \
			-I/usr/include/Neptune \
			-lPlatinum -lNeptune -laxTLS -lPltMediaRenderer -lpthread -lvlc \
			-DNPT_CONFIG_ENABLE_LOGGING

mymediarenderer: mymediarenderer.cpp mymediarenderer.h main.cpp
	$(CC) mymediarenderer.cpp main.cpp -o mymediarenderer $(CFLAGS)

.PHONY: install clean

install: mymediarenderer
	install -m755 -Dst /usr/bin/ mymediarenderer

clean:
	rm -f mymediarenderer
