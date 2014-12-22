To install a program run 'make' command.
This will create two executables 
1)gatorcrypt
	This binary is responsible for encrypting file's content and sending them over the network. It runs in two modes Local and network.
        a) Local Mode:
		Third argument of the command should be -l to use it in local mode
	b) Network Mode:
		Third argument of the command should be <IP:PORT>. Defauld IP and PORT combinations is LOCAL:8080

2)gatordec
	This binary is responsible for decrypting file's content and receiving them over the network by running as deamon. It runs in two modes Local and network.
        a) Local Mode:
		Third argument of the command should be -l to use it in local mode.
	b) Network Mode:
		Third argument of the command should be <IP:PORT>. Defauld IP and PORT combinations is LOCAL:8080
