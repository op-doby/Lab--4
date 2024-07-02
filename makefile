hexeditplus: task1.o 
	gcc -g -m32 -Wall -o hexeditplus task1.o 
task1.o: task1.c 
	gcc -m32 -g -Wall -c -o task1.o task1.c 
task4: task4.c 
	gcc -m32 -g -Wall -fno-pie -fno-stack-protector -o task4 task4.c
#-fno-pie' and '-fno-stack-protector
.PHONY: clean 
clean: 
	rm -f *.o hexeditplus task4