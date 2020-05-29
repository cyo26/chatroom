.PHONY: all clean
TARGET = client server

CFLAGS += -O2
CFLAGS += -std=gnu99 -w
CFLAGS += -pthread

OBJS = client.o server.o

all: $(TARGET)

%: %.o
	$(CC) -o $@ $^

%.o: %.c
	$(Q)$(CC) -o $@ $(CFLAGS) -c -MMD -MF .$@.d $<

clean:
	$(RM) $(TARGET)