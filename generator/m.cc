/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 * Copyright 2012 Andrew Ju (andrew@skynet.ie)    
 *
 */

#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>

int main(int argc, char** argv) {
	/*
	 * main function
	 * input format: m num_node density
	 */
	FILE *fw;	
	if(argc == 3){
		// filename not provided 
		fw = fopen("output.col","w");
		fputs("c FILE: output.col\n", fw);
	}
	else if(argc == 4){
		// filename provided by input
		fw = fopen(argv[3], "w");
		fputs("c FILE: ", fw);
		fputs(argv[3], fw);
		fputs("\n", fw);
	}
	else{
		/*
		 * missing parameters 
		 */
		printf("%s num_node density [filename]\n", argv[0]);
		return 1;
	}
	
	int i, j, k, count;
	int num_node = atoi(argv[1]); // num_node value
	float density = atof(argv[2]); // density value
	
	int len = num_node*(num_node-1)/2;
	char list[len][10];
	k = 0;
	for(i = 0; i < num_node; i++)
		for(j = i+1; j < num_node; j++){
			sprintf(list[k], "e %d %d\n", (i+1), (j+1));
			k++;
		}
	int *edge = new int[len];
	for(i = 0; i < len; i++)	edge[i] = 0;
	int num_edge = (int)(len*density);
	fprintf(fw, "c created by function call\n");
	fprintf(fw, "p edge %d %d\n", num_node, num_edge);
	
	srand(time(NULL));
	count = 0;
	while(true){
		if(count == num_edge)	break;
		/*
		 * int rand(int n);
		 * generates a random number in the range of 0 to n-1
		 */
		k = len - count;
		// The following doesn't work.
		/*
		i = random()%k; // get a random value
		*/
		i = rand()%k;
		fputs(list[i], fw); // print to file
		edge[i] = len - count;
		count++;
	}
	fclose(fw);
}	  
