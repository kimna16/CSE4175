//Link state algorithm

#include <stdio.h>
#include <stdlib.h>
int** nodeArray, *** routingTable;

struct sptNode {
	int weight;
	int parent;
};

void initialRouteTable(int nodeNum) {
	//initialize route table
	for (int i = 0; i < nodeNum; i++) {
		for (int j = 0; j < nodeNum; j++) {
			if (i == j) {
				routingTable[i][j][0] = j;
				routingTable[i][j][1] = 0;
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
	int nodeIdx = 0;
	struct sptNode* SPT;
	int* selectedArray;

	while (nodeIdx != nodeNum) {

		SPT = (struct sptNode*)malloc(sizeof(struct sptNode) * nodeNum);

		for (int i = 0; i < nodeNum; i++) {
			if (i == nodeIdx) {
				SPT[i].weight = 0;
				SPT[i].parent = i;
			}
			else {
				SPT[i].weight = -999;
				SPT[i].parent = -1;
			}
		}

		for (int i = 0; i < nodeNum; i++) {
			if (nodeArray[nodeIdx][i] != 0) {
				SPT[i].weight = nodeArray[nodeIdx][i];
				SPT[i].parent = nodeIdx;
			}
		}

		int selectedNum = 1, selectedIdx = 999, flag, shortest, tmp;
		selectedArray = (int*)malloc(sizeof(int) * nodeNum);
		selectedArray[0] = nodeIdx;

		while (1) {
			shortest = 999;
			for (int i = 0; i < nodeNum; i++) {
				if (SPT[i].weight > 0 && SPT[i].weight < shortest) {
					flag = 0;
					for (int j = 0; j < selectedNum; j++) {
						if (i == selectedArray[j]) {
							flag = 1;
							break;
						}
					}
					if (flag == 1)
						continue;
					selectedIdx = i;
					shortest = SPT[i].weight;
				}
			}

			if (selectedNum == nodeNum || selectedIdx == 999) {
				free(selectedArray);
				break;
			}

			selectedArray[selectedNum++] = selectedIdx;
			for (int i = 0; i < nodeNum; i++) {
				if (nodeArray[selectedIdx][i] != 0) {
					if (SPT[i].weight == -999 || SPT[selectedIdx].weight + nodeArray[selectedIdx][i] < SPT[i].weight) {
						if (SPT[selectedIdx].weight + nodeArray[selectedIdx][i] < 0)
							continue;
						SPT[i].weight = SPT[selectedIdx].weight + nodeArray[selectedIdx][i];
						SPT[i].parent = selectedIdx;
					}
				}
			}
		}

		for (int i = 0; i < nodeNum; i++) {
			if (i != nodeIdx) {
				if (SPT[i].weight < 0)
					continue;
				tmp = i;
				while (1) {
					if (SPT[tmp].parent == nodeIdx)
						break;
					tmp = SPT[tmp].parent;
				}
				routingTable[nodeIdx][i][0] = tmp;
				routingTable[nodeIdx][i][1] = SPT[i].weight;
			}
		}
		nodeIdx++;

		free(SPT);
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

int main(int argc, char* argv[]) {

	int nodeNum, n1, n2, weight;

	//argument number error print 
	if (argc != 4) {
		printf("usage: distvec topologyfile messagesfile changesfile");
		exit(1);
	}

	//File open, error message print 
	FILE* topology, * change, * outputFile, * messages;
	topology = fopen(argv[1], "r");
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
	outputFile = fopen("output_ls.txt", "w");
	printRouteTable(outputFile, nodeNum);

	//messages file
	printMessages(argv[2], outputFile);

	//change file
	//initialRouteTable(nodeNum);
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
	printf("Complete. Output file written to output_ls.txt\n");

	return 0;
}