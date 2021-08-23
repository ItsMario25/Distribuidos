# Distribuidos
- Para compilar servidor

gcc servidor.c -lpthread -o server
./server 2222
- Para compilar cliente

gcc cliente.c -lpthread -o client
./client 127.0.0.1 2222
