#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* inputStream, * dataStream, * outputStream;
//char* modulo2;

void resetFunc(char* nameOfList, int sizeOfList);
int decodeFunc(char* codeword, char* generator, int datawordSize, int codewordSize, int codewordNum, int generatorSize, int arrayIdx);

int main(int argc, char* argv[]) {

	if (argc != 6) {
		printf("usage: ./crc_encoder input_file output_file result_file generator dataword_size");
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

	char resultFile[20];
	FILE* result;

	//open result file 
	memset(resultFile, 0, sizeof(resultFile));
	strcpy(resultFile, argv[3]);
	result = fopen(resultFile, "wb");
	if (result == NULL) {
		printf("output file open error.");
		exit(1);
	}

	char generator[10];
	int generatorSize;

	//store generator, measure length of the generator and make characters in generator list integer type 
	memset(generator, 0, sizeof(generator));
	strcpy(generator, argv[4]);
	generatorSize = strlen(generator);
	for (int i = 0; i < generatorSize; i++) {
		generator[i] -= 48;
		//'0' = 48
	}

	int datawordSize;

	//dataword size error message 
	datawordSize = atoi(argv[5]);
	if (datawordSize != 4 && datawordSize != 8) {
		printf("dataword size must be 4 or 8.");
		exit(1);
	}

	int codewordSize = datawordSize + generatorSize - 1;
	
	char* codeword = (char*)malloc(sizeof(char) * codewordSize);
	memset(codeword, 0, codewordSize);
	//resetFunc(codeword, codewordSize);

	inputStream = (char*)malloc(sizeof(char) * inputFileSize * 8);
	memset(inputStream, 0, inputFileSize * 8);
	//resetFunc(inputStream, inputFileSize * 8);

	char ch;
	int i, paddingbits;
	for (i = 0; i < inputFileSize; i++) {
	//while (ch = fgetc(in) != EOF) {
		ch = fgetc(in);
		//printf("%d", ch); //test
		//if (ch == EOF) 
		//	break;     
		if (i == 0)
			paddingbits = ch;
		int j;
		for (j = 0; j < 8; j++) {
			int tmp = i * 8 + j;
			inputStream[tmp] = (ch >> (7 - j)) & 1;
			if (j == 7)
				break;
		}
		if (i == inputFileSize - 1)
			break;
	}

	int arrayIdx = paddingbits;
	arrayIdx += 8;
	
	int codewordNum = (inputFileSize * 8 - arrayIdx) / codewordSize;

	int datawordNum;
	if (datawordSize == 4)
		datawordNum = codewordNum / 2;
	else if (datawordSize == 8)
		datawordNum = codewordNum;

	dataStream = (char*)malloc(sizeof(char) * codewordNum * datawordSize); 
	memset(dataStream, 0, codewordNum * datawordSize);
	//resetFunc(dataStream, codewordNum * datawordSize);

	outputStream = (char*)malloc(sizeof(char) * datawordNum); 
	memset(outputStream, 0, datawordNum);
	//resetFunc(outputStream, datawordNum);

	int errorNum = decodeFunc(codeword, generator, datawordSize, codewordSize, codewordNum, generatorSize, arrayIdx);
	//free(modulo2);

	fprintf(result, "%d %d", codewordNum, errorNum);

	arrayIdx = 0;

	int tmp = datawordNum * 8;
	for (i = 0; i < tmp; i++) {
		outputStream[arrayIdx] = outputStream[arrayIdx] + dataStream[i];
		if (i % 8 != 7)
			outputStream[arrayIdx] <<= 1;
		else
			arrayIdx++;
		if (i == tmp - 1)
			break;
	}

	fwrite(outputStream, arrayIdx, 1, out);

	free(codeword);
	free(inputStream);
	free(dataStream);
	free(outputStream);

	fclose(in);
	fclose(out);
	fclose(result);

	return 0;
}

void resetFunc(char *nameOfList, int sizeOfList) {
	nameOfList = (char*)malloc(sizeof(char) * sizeOfList);
	memset(nameOfList, 0, sizeOfList);
}

int decodeFunc(char* codeword, char* generator, int datawordSize, int codewordSize, int codewordNum, int generatorSize, int arrayIdx) {
	
	char modulo2[10]; 
	memset(modulo2, 0, 10);
	//resetFunc(modulo2, 10);

	int errorNum = 0, idx = 0;

	int tmp = generatorSize - 1;

	int i;
	for (i = 0; i < codewordNum; i++) {
		memset(codeword, 0, codewordSize);
		memcpy(codeword, inputStream + arrayIdx, codewordSize);
		
		int j; 
		memset(modulo2, 0, 10);
		memcpy(modulo2, codeword, generatorSize);

		arrayIdx = arrayIdx + codewordSize;

		for (j = 0; j <= codewordSize - generatorSize; j++) {
			if (modulo2[0] == 1) {
				int k;
				for (k = 0; k < generatorSize; k++) {
					if (modulo2[k] != generator[k])
						modulo2[k] = 1;
					else
						modulo2[k] = 0;
					if (k == generatorSize - 1)
						break;
				}
			}
			if (j == codewordSize - generatorSize)
				break;
			else {
				memcpy(modulo2, modulo2 + 1, tmp);
				modulo2[tmp] = codeword[j + generatorSize];
			}
		}

		for (j = 0; j < datawordSize; j++) {
			dataStream[idx + j] = codeword[j];
			if (j == datawordSize - 1)
				break;
		}
		idx = idx + datawordSize;

		memcpy(modulo2, modulo2 + 1, tmp);
		for (j = 0; j < tmp; j++) {
			if (modulo2[j] == 0)
				break;
			else {
				errorNum++;
				break;
			}
		}

		if (i == codewordNum - 1)
			break;
	}

	return errorNum;
}