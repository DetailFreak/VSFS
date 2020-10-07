all: c m d
.PHONY: all

clean:
	rm -rf server_*/
	rm *.out
.PHONY: clean

c : client.out

m : m_server.out

d : d_server.out

client.out : client.c
	gcc client.c -o client.out

m_server.out : m_server.c
	gcc m_server.c -o m_server.out

d_server.out : d_server.c
	gcc d_server.c -o d_server.out
