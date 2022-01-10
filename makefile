OBJ = main.o utility.o time.o

all: $(OBJ)
	g++ -pthread $(OBJ)  -O -o prodcon


main:
	g++ -c -pthread main.cpp -o main.o

utility:
	g++ -c -pthread utility.cpp -o utility.o

time:
	g++ -c -pthread time.cpp -o time.o

clean:
	rm *.o prodcon