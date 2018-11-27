#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

int main (int argc, char **argv) {

	int i = 0;
	int k = 0;
	FILE *fileptr;
	long filelen = 0;
	int bitCount = 0;
	int total = 0;
	int bitWeight = 0;
	char ascii;
	int aIndex = 0;
	int index = 0;
        char chars[] = {' ', '!', '"', '#', '$', '%', '&', '\'', '(', ')', '*', '+', ',', '-', '.', '/', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '<', '=', '>', '?', '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G','H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '[', '\\', ']', '^', '_', '`', 'a','b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u','v', 'w', 'x', 'y', 'z', '{', '|', '}', '~', '\0'};

	/* Get the length of the bin file */
	fileptr = fopen(argv[1], "rb");
	fseek(fileptr, 0, SEEK_END);
	filelen = ftell(fileptr);
	/* buffer[] holds the contents of the bin file */
	char buffer[filelen];
	/* Append the nul terminator */
	buffer[filelen] = '\0';
	int size = sizeof(buffer);
	rewind(fileptr);
	/* text[] holds the ASCII once we've converted */
	char text[(size/8)];
	text[(size/8)] = '\0';
	/* Don't actually know if this needs to be memset with \0 or just 0 */
	memset(buffer, sizeof(buffer), '\0');
	fread(buffer, filelen, 1, fileptr);
	fclose(fileptr);

	/* Read the file backwards? Depends on endian */
	for(i = size - 1; i >= 0; i--) {
		bitCount++;
		if (buffer[i] == '1') {	
		           total += pow(2, bitWeight);
		}
		bitWeight++;

		/* Slice up the data into bytes */
		if (bitCount == 8) {
			bitCount = 0;
			/* Subtract 32 (first printable ASCII) to get the index */
			aIndex = total - 32;
			/* Construct the array that holds the ASCII */
			text[index] = chars[aIndex];
			index++;
			total = 0;
			bitWeight = 0;
		}
	}
	/* Print the ASCII */
	for(k = strlen(text)-1; k >= 0; k--) {
		printf("%c", text[k]);

	}

return 0;
}
