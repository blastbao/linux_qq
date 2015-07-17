obj=client.o msg_buffer.o

client:$(obj)
	g++ -o client $(obj)

client.o:client.cpp
	g++ -g -c client.cpp

msg_buffer.o:../msg_buffer.cpp
	g++ -g -c ../msg_buffer.cpp

clean:
	rm client $(obj)
