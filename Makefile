name:=finsum
dest:=/usr/local/bin

compile: 
	gcc -Wall -Wextra -Werror -o $(name) finsum.c

install:
	mkdir -p $(dest)
	cp $(name) $(dest)
	sudo chmod 755 $(dest)/$(name)
