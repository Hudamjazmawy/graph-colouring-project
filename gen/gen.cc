
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
 * Copyright 2013 Andrew.Ju (andrew@skynet.ie)    
 *
 */

#include <stdio.h>
#include <math.h>
#include <time.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include <iostream>
#include <string>
#include <vector>
#include <fstream>

using std::ofstream;
using std::cout;
using std::endl;
using std::string;
using std::vector;

int main(int argc, char** argv) {
	/*
	 * main function
	 * input format: m num_node probability
	 */
	ofstream file;	
	if(argc == 4){
		// filename not provided 
		file.open("output.col");
		file<<"c FILE: output.col\n";
	}
	else if(argc == 5){
		// filename provided by input
		file.open(argv[4]);
		file<<"c FILE: "<<argv[4]<<"\n";
	}
	else{
		/*
		 * missing parameters 
		 */
		printf("%s num_node probability seed [filename]\n", argv[0]);
		return 1;
	}
	
	int i, j, k, count;
	int num_node = atoi(argv[1]); // num_node value
	float probability = atof(argv[2]); // probability value
	int seed = atoi(argv[3]);
	
	int len = num_node*(num_node-1)/2;
	char edge[256]; // format: e %d %d
	vector<string> edges;
	k = 0;
	srand(seed);

	for(i = 0; i < num_node; i++)
		for(j = i+1; j < num_node; j++){
			if((double)rand()/(double)RAND_MAX <= probability) {
				sprintf(edge, "e %d %d", (i+1), (j+1));
				edges.push_back(edge);
			}
		}
	file<<"c created by function call\n";
	file<<"p edge "<<num_node<<" "<<edges.size()<<"\n";

	for(i = 0; i < edges.size(); i++)
			file<<edges[i]<<"\n";
	file.close();
}	  
