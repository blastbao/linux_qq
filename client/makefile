obj=client.o user_input_util.o fsocket.o const.o util.o msg_buffer.o msg_util.o ring_queue.o

client:$(obj)
	g++ -o client $(obj) -lpthread

client.o:client.cpp
	g++ -c client.cpp

user_input_util.o:./user_input_util.cpp
	g++ -c ./user_input_util.cpp

fsocket.o:./fsocket.cpp
	g++ -c ./fsocket.cpp

const.o:../const.cpp
	g++ -c ../const.cpp

util.o:../util.cpp
	g++ -c ../util.cpp

msg_buffer.o:../msg_buffer.cpp
	g++ -c ../msg_buffer.cpp

msg_util.o:../msg_util.cpp
	g++ -c ../msg_util.cpp

ring_queue.o:../ring_queue.cpp
	g++ -c ../ring_queue.cpp

clean:
	rm client $(obj)
