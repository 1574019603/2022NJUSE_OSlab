build: my_print.o
	@echo "going to compile main.cpp"
	@g++ -std=c++11 -o fat main.cpp my_print.o
	@echo "finish compile main.cpp"

my_print.o: my_print.asm
	@echo "going to compile myPrint.asm"
	@nasm -f elf -o my_print.o my_print.asm
	@echo "going to compile myPrint.asm"

run: build
	@./fat

clean:
	@rm -f *.o
	@rm -f fat
