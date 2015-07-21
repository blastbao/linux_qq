obj=client.o user_input_util.o const.o util.o msg_buffer.o msg_util.o

client:$(obj)
	g++ -o client $(obj)

client.o:client.cpp
	g++ -c client.cpp

user_input_util.o:./user_input_util.cpp
	g++ -c ./user_input_util.cpp

const.o:../const.cpp
	g++ -c ../const.cpp

util.o:../util.cpp
	g++ -c ../util.cpp

msg_buffer.o:../msg_buffer.cpp
	g++ -c ../msg_buffer.cpp

msg_util.o:../msg_util.cpp
	g++ -c ../msg_util.cpp

clean:
	rm client $(obj)
