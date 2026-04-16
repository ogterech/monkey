bin/roguelike: roguelike.c bin
	gcc roguelike.c -o bin/roguelike

bin: 
	mkdir -p ./bin

clean:
	rm -rf ./bin
