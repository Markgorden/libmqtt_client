include ../../config.mk
include ../../wifi_shd.mk

TARGET = libc_mqtt_client.a

all : mqtt_client.o
	$(AR) -r $(TARGET) *.o
	cp $(TARGET) $(MYLIB)

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

clean :
	/bin/rm -f *.o
	/bin/rm -f $(TARGET)
