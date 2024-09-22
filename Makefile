CC=gcc
ARGS=-Wextra -Wall

cascii: main.c stb_image_impl.c 
	$(CC) -o cascii $^ -lm $(ARGS) 

.PHONY: clean
clean:
	rm -f $(EXECUTABLE)

