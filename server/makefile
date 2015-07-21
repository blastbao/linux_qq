obj=server.o service.o const.o util.o msg_buffer.o msg_util.o

server:$(obj)
	g++ -o server $(obj)

server.o:server.cpp
	g++ -c server.cpp

service.o:service.cpp
	g++ -c service.cpp

const.o:../const.cpp
	g++ -c ../const.cpp

util.o:../util.cpp
	g++ -c ../util.cpp

msg_buffer.o:../msg_buffer.cpp
	g++ -c ../msg_buffer.cpp

msg_util.o:../msg_util.cpp
	g++ -c ../msg_util.cpp

clean:
	rm server $(obj)
