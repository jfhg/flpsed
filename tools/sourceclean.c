#include <stdio.h>

int main(int argc, char **argv) {
	unsigned char c;
	while (fread(&c, sizeof(c), 1, stdin)) {
		if (c < 128) {
			printf("%c", c);
		} else {
			printf("\\x%2X", c);
		}
	}
}
