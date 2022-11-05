CC=gcc

all: 2_4_6.bin

2_4_6.bin: 2_4_6.c
	$(CC) 2_4_6.c -o 2_4_6.bin

clean:
	rm -f *.bin

tests: 2_4_6.bin
	python3 test.py
	