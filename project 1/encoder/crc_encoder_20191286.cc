#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void encodeFunc(char* generator, char* codeword, int datawordSize, int codewordSize, int generatorSize);
void arrangeFunc(char* unarrangedCodeArray, char* arrangedCode, int codewordSize, int codewordNum, int totalByteNum);

int main(int argc, char* argv[]) {

	//argument number error message
	if (argc != 5) {
		printf("usage: ./crc_encoder input_file output_file generator dataword_size");
		exit(1);
	}

	char inputFile[20];
	FILE* in;

	//open input file and measure size of the file
	memset(inputFile, 0, sizeof(inputFile));
	strcpy(inputFile, argv[1]);
	in = fopen(inputFile, "rb");
	if (in == NULL) {
		printf("input file open error.");
		exit(1);
	}
	fseek(in, 0, SEEK_END);
	int inputFileSize = ftell(in);
	fclose(in);
	in = fopen(inputFile, "rb");
	if (in == NULL) {
		printf("input file open error.");
		exit(1);
	}

	char outputFile[20];
	FILE* out;

	//open output file
	memset(outputFile, 0, sizeof(outputFile));
	strcpy(outputFile, argv[2]);
	out = fopen(outputFile, "wb");
	if (out == NULL) {
		printf("output file open error.");
		exit(1);
	}

	char generator[10];
	int generatorSize;

	//store generator, measure length of the generator and make characters in generator list integer type 
	memset(generator, 0, sizeof(generator));
	strcpy(generator, argv[3]);
	generatorSize = strlen(generator);
	for (int i = 0; i < generatorSize; i++) {
		generator[i] = generator[i] - 48;
		//'0' = 48
	}

	int datawordSize;

	//dataword size error message 
	datawordSize = atoi(argv[4]);
	if (datawordSize != 4 && datawordSize != 8) {
		printf("dataword size must be 4 or 8.");
		exit(1);
	}

	//calculate codeword size, codeword number
	int codewordSize = datawordSize + generatorSize - 1;
	int codewordNum = inputFileSize * 8 / datawordSize;

	//In HW pdf example '00000010 00010001 10001101', arrangedCode means this kind of array with padding bits and in this example, totalByteNum is 3
	int totalByteNum = codewordSize * codewordNum / 8;
	//1st byte must indicate how many padding bits are used  
	totalByteNum += 1;
	if (codewordSize * codewordNum % 8 == 0)
		totalByteNum += 0;
	else
		totalByteNum += 1;
	char* arrangedCode = (char*)malloc(sizeof(char) * totalByteNum);
	memset(arrangedCode, 0, totalByteNum);

	//'A' -> codeword '01000110001101'(unarrangedCodeArray) 
	char* unarrangedCodeArray = (char*)malloc(sizeof(char) * codewordSize * codewordNum);
	memset(unarrangedCodeArray, 0, codewordSize * codewordNum);
	//arrayIdx will be used for storing data in unarrangedCodeArray
	int arrayIdx = 0;

	//repeating for loop inputFileSize times
	int c;
	for (c = 0; c < inputFileSize; c++) {

		//read a character from input file 
		char ch = fgetc(in), ascii[8];

		//ascii list will be used for transforming characters into 8-bit binary codes
		memset(ascii, 0, 8);

		//character to binary code transforming process 
		int i;
		for (i = 0; i < 8; i++) {
			ascii[7 - i] = (ch >> i) & 1;
		}

		if (datawordSize == 8) {
			char* codeword = (char*)malloc(sizeof(char) * codewordSize);
			memset(codeword, 0, codewordSize);
			//use full ascii list to encode
			memcpy(codeword, ascii, 8);

			encodeFunc(generator, codeword, datawordSize, codewordSize, generatorSize);

			//concatenate encoded code in unarrangedCodeArray
			memcpy(unarrangedCodeArray + arrayIdx, codeword, codewordSize);
			//update arrayIdx to point next position
			arrayIdx += codewordSize;
			free(codeword);
		}
		else if (datawordSize == 4) {
			//repeat for loop twice
			for (i = 0; i < 2; i++) {
				char* codeword = (char*)malloc(sizeof(char) * codewordSize);
				memset(codeword, 0, codewordSize);
				//use half of ascii list to encode  
				memcpy(codeword, ascii + (i * 4), 4);

				encodeFunc(generator, codeword, datawordSize, codewordSize, generatorSize);

				//concatenate encoded code in unarrangedCodeArray
				memcpy(unarrangedCodeArray + arrayIdx, codeword, codewordSize);
				//update arrayIdx to point next position
				arrayIdx += codewordSize;
				free(codeword);
				if (i == 1)
					break;
			}
		}
	}

	arrangeFunc(unarrangedCodeArray, arrangedCode, codewordSize, codewordNum, totalByteNum);

	fwrite(arrangedCode, totalByteNum, 1, out);

	free(unarrangedCodeArray);
	free(arrangedCode);

	fclose(in);
	fclose(out);

	return 0;
}
void encodeFunc(char* generator, char* codeword, int datawordSize, int codewordSize, int generatorSize) {
	char modulo2[10];
	memset(modulo2, 0, 10);
	memcpy(modulo2, codeword, generatorSize);

	int i;
	for (i = 0; i <= codewordSize - generatorSize; i++) {
		if (modulo2[0] == 1) {
			int j;
			for (j = 0; j < generatorSize; j++) {
				if (modulo2[j] != generator[j])
					modulo2[j] = 1;
				else
					modulo2[j] = 0;
			}
		}
		if (i == codewordSize - generatorSize)
			break;
		else {
			int tmp = generatorSize - 1;
			memcpy(modulo2, modulo2 + 1, tmp);
			modulo2[tmp] = codeword[i + generatorSize];
		}
	}
	memcpy(codeword + datawordSize, modulo2 + 1, generatorSize - 1);
}

void arrangeFunc(char* unarrangedCodeArray, char* arrangedCode, int codewordSize, int codewordNum, int totalByteNum) {
	int arrayIdx = 0;
	unsigned char byteNum = 0;

	//1st byte indicates number of padding bits 
	int paddingbitNum = 8 - codewordSize * codewordNum % 8;
	arrangedCode[0] = paddingbitNum;

	//2nd byte includes padding bits
	int i;
	for (i = paddingbitNum; i < 8; i++) {
		byteNum += unarrangedCodeArray[arrayIdx++];
		if (i == 7)
			break;
		else
			byteNum <<= 1;
	}
	arrangedCode[1] = byteNum;
	byteNum = 0;

	//3rd byte ~ final byte
	for (i = 2; i < totalByteNum; i++) {
		int j;
		for (j = 0; j < 8; j++) {
			byteNum += unarrangedCodeArray[arrayIdx++];
			if (j == 7)
				break;
			else
				byteNum <<= 1;
		}
		arrangedCode[i] = byteNum;
		byteNum = 0;
	}
}
