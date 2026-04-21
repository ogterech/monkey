bin/roguelike: roguelike.c bin
	$(CC) roguelike.c -o bin/roguelike -Wall -Wextra -pedantic -std=c99

bin: 
	mkdir -p ./bin

clean:
	rm -rf ./bin
