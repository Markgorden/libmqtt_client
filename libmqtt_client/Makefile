GCC=gcc
AR = ar

TARGET = libmqtt_client.a
   
all : MQTTProtocolClient.o Clients.o utf-8.o StackTrace.o MQTTPacket.o MQTTPacketOut.o Messages.o Tree.o Socket.o Log.o MQTTPersistence.o Thread.o MQTTProtocolOut.o MQTTPersistenceDefault.o SocketBuffer.o Heap.o LinkedList.o MQTTClient.o
	$(AR) -r $(TARGET) *.o
	cp $(TARGET) $(MYLIB)

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $< 

clean :
	/bin/rm -f *.o
	/bin/rm -f $(TARGET)
