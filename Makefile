INCLUDE=-framework CoreServices
OBJECTS+=fswatch.o

all: fswatch 

fswatch: $(OBJECTS)
	gcc $(INCLUDE) -o fswatch $(OBJECTS)

clean:
	rm -f *.o fswatch
