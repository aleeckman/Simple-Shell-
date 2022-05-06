sshell   	  : sshell.c data_struct.o utils.o
				 gcc -Wall -Wextra -Werror -o sshell sshell.c data_struct.o utils.o

utils.o       : utils.c utils.h
				 gcc -Wall -Wextra -Werror -c utils.c

data_struct.o : utils.c utils.h
				 gcc -Wall -Wextra -Werror -c data_struct.c

clean    	  :
				 rm -f sshell utils.o data_struct.o

debug    	  :
				 # Only if you have install valgrind in the machine
				 valgrind --tool=memcheck --leak-check=yes --leak-check=full --show-leak-kinds=all --track-origins=yes ./sshell
			