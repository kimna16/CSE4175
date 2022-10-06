//Distance Vector routing algorithm

#include <stdio.h>
#include <stdlib.h>
int** nodeArray, *** routingTable;

void initialRouteTable(int nodeNum) { 
	//initialize route table
	for (int i = 0; i < nodeNum; i++) {
		for (int j = 0; j < nodeNum; j++) {
			if (i == j) { 
				routingTable[i][j][0] = j;
				routingTable[i][j][1] = 0; 
			}
			else if (nodeArray[i][j] > 0) { 
				routingTable[i][j][0] = j;              
				routingTable[i][j][1] = nodeArray[i][j]; 
			}
			else { 
				routingTable[i][j][0] = -1;   
				routingTable[i][j][1] = -999; 
			}
		}
	}
}

void convergeFunc(int nodeNum) {
	//update route table
	while (1) {
		int flag = 0;
		for (int i = 0; i < nodeNum; i++) {
			for (int j = 0; j < nodeNum; j++) {
				//if (j == routingTable[i][j][0] && routingTable[i][j][1] != 0) {
				if (j == routingTable[i][j][0] && i != j) {
					for (int k = 0; k < nodeNum; k++) { 
						if (routingTable[i][k][1] == -999 || routingTable[i][j][1] + routingTable[j][k][1] < routingTable[i][k][1]) { 
							//if (routingTable[i][j][1] + routingTable[j][k][1] < 0) 
							if (routingTable[j][k][1] < 0)
								continue;
							routingTable[i][k][0] = j; 
							routingTable[i][k][1] = routingTable[i][j][1] + routingTable[j][k][1]; 
							flag = 1; 
						}
					}
				}
			}
		}
		if (flag != 1) 
			break;
	}
}

void printMessages(char* argv2, FILE* outputFile) {
	FILE* messages = fopen(argv2, "r");
	char ch, message[500];
	int start, end, messageIdx;
	while (1) {
		messageIdx = 0;
		ch = fgetc(messages);
		if (ch == EOF)
			break;
		start = atoi(&ch);
		fgetc(messages);
		ch = fgetc(messages);
		end = atoi(&ch);
		fgetc(messages);
		while (1) {
			ch = fgetc(messages);
			if (ch == '\n' || ch == EOF) {
				message[messageIdx] = '\0';
				break;
			}
			message[messageIdx++] = ch;
		}
		if (routingTable[start][end][1] != -999) {
			fprintf(outputFile, "from %d to %d cost %d hops ", start, end, routingTable[start][end][1]);
			while (1) {
				fprintf(outputFile, "%d ", start);
				start = routingTable[start][end][0];
				if (start == end)
					break;
			}
			fprintf(outputFile, "message %s\n", message);
		}
		else
			fprintf(outputFile, "from %d to %d cost infinite hops unreachable message %s\n", start, end, message);
		if (ch == EOF)
			break;
	}
	fprintf(outputFile, "\n");
	fclose(messages);
}

void printRouteTable(FILE* outputFile, int nodeNum) { 
	for (int i = 0; i < nodeNum; i++) {
		for (int j = 0; j < nodeNum; j++) {
			if (routingTable[i][j][1] == -999) continue;
			fprintf(outputFile, "%d %d %d\n", j, routingTable[i][j][0], routingTable[i][j][1]);
		}
		fprintf(outputFile, "\n"); 
	}
}

int main(int argc, char *argv[]) {

	int nodeNum, n1, n2, weight;

	//argument number error print
	if (argc != 4) {
		printf("usage: distvec topologyfile messagesfile changesfile\n");
		exit(1);
	}

	//File open, error message print 
	FILE* topology, * change, * outputFile, * messages;
	topology = fopen(argv[1],"r");
	if (topology == NULL) {
		printf("Error: open input file.");
		exit(1);
	}
	messages = fopen(argv[2], "r");
	if (messages == NULL) {
		printf("Error: open input file.");
		exit(1);
	}
	fclose(messages);
	change = fopen(argv[3], "r");
	if (change == NULL) {
		printf("Error: open input file.");
		exit(1);
	}

	//topology file
	fscanf(topology, "%d", &nodeNum);
	nodeArray = (int**)malloc(sizeof(int*) * nodeNum);
	routingTable = (int***)malloc(sizeof(int**) * nodeNum);
	for (int i = 0; i < nodeNum; i++) {
		nodeArray[i] = (int*)malloc(sizeof(int) * nodeNum);
		routingTable[i] = (int**)malloc(sizeof(int*) * nodeNum);
		for (int j = 0; j < nodeNum; j++) {
			nodeArray[i][j] = 0;
			routingTable[i][j] = (int*)malloc(sizeof(int) * 2);
			routingTable[i][j][0] = -1; 
			routingTable[i][j][1] = -999;
		}
	}
	while (1) {
		if (fscanf(topology, "%d %d %d", &n1, &n2, &weight) == EOF)
			break;
		//weight <= 100
		nodeArray[n1][n2] = weight;  
		nodeArray[n2][n1] = weight;
	}

	//initial routing table
	initialRouteTable(nodeNum);

	//Converged routing table 
	convergeFunc(nodeNum);

	//output file print 
	outputFile = fopen("output_dv.txt", "w");
	printRouteTable(outputFile, nodeNum);

	//messages file
	printMessages(argv[2], outputFile);

	//change file
	while (1) {
		if (fscanf(change, "%d %d %d", &n1, &n2, &weight) == EOF)
			break;
		if (weight < 0)
			weight = 0;
		nodeArray[n1][n2] = weight;
		nodeArray[n2][n1] = weight;

		//initial routing table
		initialRouteTable(nodeNum);

		//Converged routing table
		convergeFunc(nodeNum);

		//output file print 
		printRouteTable(outputFile, nodeNum);

		//messages file
		printMessages(argv[2], outputFile);
	}

	//free
	for (int i = 0; i < nodeNum; i++) {
		for (int j = 0; j < nodeNum; j++) {
			free(routingTable[i][j]);
		}
		free(nodeArray[i]);
		free(routingTable[i]);
	}
	free(nodeArray);
	free(routingTable);

	//fclose
	fclose(topology);
	fclose(change);
	fclose(outputFile);

	//print completion message
	printf("Complete. Output file written to output_dv.txt\n");

	return 0;
}