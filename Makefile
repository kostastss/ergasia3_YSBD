sort:
	@echo " Compile sort_main ...";
	gcc -g -I ./include/ -L ./lib/ -Wl,-rpath,./lib/ \
		./examples/sort_main.c \
		./src/record.c ./src/sort.c ./src/merge.c ./src/chunk.c \
		-lhp_file -lbf \
		-o ./build/sort_main

my_main:
	@echo " Compile my_main ...";
	gcc -g -I ./include/ -L ./lib/ -Wl,-rpath,./lib/ \
		./examples/my_main.c \
		./src/record.c ./src/sort.c ./src/merge.c ./src/chunk.c \
		-lhp_file -lbf \
		-o ./build/my_main
