CC=gcc
ARGS=-Wextra -Wall

cascii: 
	$(CC) -o cascii main.c stb_image_impl.c $(ARGS) 

