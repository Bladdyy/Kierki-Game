CC     = g++
CFLAGS = -Wall -Wextra -O2 -std=gnu17

.PHONY: all clean

TARGET1 = kierki-klient
TARGET2 = kierki-serwer

all: $(TARGET1) $(TARGET2)

$(TARGET1): $(TARGET1).o common.o
$(TARGET2): $(TARGET2).o common.o

kierki-klient.o: ../Testy_sik2/kierki-klient.cpp
kierki-serwer.o: kierki-serwer.cpp
common.o: common.cpp

clean:
	rm -f $(TARGET1) $(TARGET2) *.o *~