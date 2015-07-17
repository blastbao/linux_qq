obj=server.o msg_buffer.o

server:$(obj)
	g++ -o server $(obj)

server.o:server.cpp
	g++ -c server.cpp

msg_buffer.o:../msg_buffer.cpp
	g++ -c ../msg_buffer.cpp

clean:
	rm server $(obj)
