bin/roguelike: roguelike.c bin
	$(CC) roguelike.c -o bin/roguelike -Wall -Wextra -pedantic -std=c99 -g

bin: 
	mkdir -p ./bin

clean:
	rm -rf ./bin
