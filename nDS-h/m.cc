//============================================================================
// Name        : m.cc
// Author      : Andrew Ju
// Version     :
// Copyright   : 2013 - present
// Description : nDS in testing / used for testing certain usage only
//============================================================================

#include <time.h>
#include <stdio.h>
#include <assert.h>
#include <cstdlib>
#include <iostream>
#include <fstream>

using namespace std;
#define MAX_NODE 2500
#define TRUE 1
#define FALSE 0

int adj[MAX_NODE][MAX_NODE]; // adjacency matrix
int ColorClass[MAX_NODE];

// adding parameters for testing
double alpha = 0.4; // additional parameter, random here
ofstream output;

int lb; // lower bound(size of clique)
int num_node;
int num_prob, max_prob;
int best_clique;

// -- start of code for nDS
class graph { // represent the graph in _set concept
public:
	int **set;
	int **ver2loc;
	int *mark;
	int *count;

	graph() {
	}

	void doInit(int numNode) {
		this->set = new int *[numNode];
		this->ver2loc = new int *[numNode];
		this->mark = new int[numNode];
		this->count = new int[numNode];

		for(int i = 0; i < numNode; i++) {
			this->set[i] = new int[numNode];
			this->ver2loc[i] = new int[numNode];
			for(int j = 0; j < numNode; j++) {
				this->set[i][j] = -1;
				this->ver2loc[i][j] = -1;
			}
			this->mark[i] = -1;
			this->count[i] = -1;
		}
	}

	void addEdges(int ver1, int ver2) { // add edges to graph
		if((ver1 != ver2) &&
				(this->ver2loc[ver1][ver2] == -1)) { // avoid duplication
			this->mark[ver1]++;
			this->count[ver1]++;
			this->set[ver1][this->mark[ver1]] = ver2;
			this->ver2loc[ver1][ver2] = this->mark[ver1];

			this->mark[ver2]++;
			this->count[ver2]++;
			this->set[ver2][this->mark[ver2]] = ver1;
			this->ver2loc[ver2][ver1] = this->mark[ver2];
		}
	}

	void update(bool flag, int index1, int index2) { // update graph
		// index1 refer to target index
		// index2 refer to actual moving object index(one that is being colored/uncolored)
		if(flag) { // being colored
			assert(this->mark[index1] >= this->ver2loc[index1][index2]);
			if(this->mark[index1] >= this->ver2loc[index1][index2]) {
				this->set[index1][this->ver2loc[index1][index2]] =
						this->set[index1][this->mark[index1]];
				this->ver2loc[index1][this->set[index1][this->mark[index1]]] =
						this->ver2loc[index1][index2];

				this->set[index1][this->mark[index1]] = index2;
				this->ver2loc[index1][index2] = this->mark[index1];

				this->mark[index1]--;
			}
		}
		else { // being uncolored
			assert(this->mark[index1] < this->ver2loc[index1][index2]);
			if(this->mark[index1] < this->ver2loc[index1][index2]) {
				this->set[index1][this->ver2loc[index1][index2]] =
						this->set[index1][this->mark[index1]+1];
				this->ver2loc[index1][this->set[index1][this->mark[index1]+1]] =
						this->ver2loc[index1][index2];

				this->set[index1][this->mark[index1]+1] = index2;
				this->ver2loc[index1][index2] = this->mark[index1]+1;

				this->mark[index1]++;
			}
		}
	}
};

class colset { // represent the adjacent color matrix in _set concept
public:
	int ***set;
	int **col2loc;
	int *mark;

	colset() {
		this->set = new int **[num_node];
		this->col2loc = new int *[num_node];
		this->mark = new int[num_node];
		for(int i = 0; i < num_node; i++) {
			this->set[i] = new int *[num_node];
			this->col2loc[i] = new int[num_node];
			for(int j = 0; j < num_node; j++) {
				this->set[i][j] = new int[2];
				this->set[i][j][0] = -1;
				this->set[i][j][1] = 0;
				this->col2loc[i][j] = -1;
			}
			this->mark[i] = -1;
		}
	}

	void update(bool flag, int target, int color) { // update the colset
		// target refer to actual vertex
		if(flag) { // being colored
			if(this->col2loc[target][color] == -1) {
				this->mark[target]++;
				this->set[target][this->mark[target]][0] = color;
				this->set[target][this->mark[target]][1]++;

				this->col2loc[target][color] = this->mark[target];
			}
			else
				this->set[target][this->col2loc[target][color]][1]++;
		}
		else { // being uncolored
			assert(this->col2loc[target][color] >= -1);
			if(this->set[target][this->col2loc[target][color]][1] > 1)
				this->set[target][this->col2loc[target][color]][1]--;

			else {
				assert(this->set[target][this->col2loc[target][color]][1] == 1);
				int loc = this->col2loc[target][color];
				this->set[target][loc][0] =
						this->set[target][this->mark[target]][0];
				this->set[target][loc][1] =
						this->set[target][this->mark[target]][1];
				this->col2loc[target][this->set[target][this->mark[target]][0]] =
						loc;

				// clear entry
				this->set[target][this->mark[target]][0] = -1;
				this->set[target][this->mark[target]][1] = 0;
				this->col2loc[target][color] = -1;

				this->mark[target]--;
			}
		}
	}
};

class sdQ { // using the sdQ to decide next candidate vertex to color
public:
	int *heap;
	int *ver2loc;
	int mark;

	sdQ(int* _clique) {
		int place = num_node;
		this->mark = -1;
		this->heap = new int[num_node];
		this->ver2loc = new int[num_node];
		for(int i = 0; i < num_node; i++) {
			if(!_clique[i]) {
				this->mark++;
				this->heap[this->mark] = i;
				this->ver2loc[i] = this->mark;
			}
			else {
				place--;
				this->heap[place] = i;
				this->ver2loc[i] = place;
			}
		}
		assert(this->mark+1 == place);
	}

	void build_heap(graph &_g, colset &_c) {
		for(int i = (this->mark-1)/2; i >= 0; i--)
			this->trickleDown(_g, _c, i);
	}

	void trickleDown(graph &_g, colset &_c, int hole) {
		assert(hole <= this->mark+1);
		int child;

		for(; hole*2+1 <= this->mark; hole = child) {
			child = hole*2+1;
			if(child != this->mark && ((_c.mark[this->heap[child]] < _c.mark[this->heap[child+1]])
					||((_c.mark[this->heap[child]] == _c.mark[this->heap[child+1]])
							&& ((_g.mark[this->heap[child]] < _g.mark[this->heap[child+1]])
									|| ((_g.mark[this->heap[child]] == _g.mark[this->heap[child+1]])
											&& (this->heap[child] > this->heap[child+1])))))) // DSATUR rule
				child++;
			if ((_c.mark[this->heap[child]] > _c.mark[this->heap[hole]])
				||((_c.mark[this->heap[child]] == _c.mark[this->heap[hole]])
						&& ((_g.mark[this->heap[child]] > _g.mark[this->heap[hole]])
								|| ((_g.mark[this->heap[child]] == _g.mark[this->heap[hole]])
										&& (this->heap[child] < this->heap[hole]))))) { // DSATUR rule
				int tmp = this->heap[hole];
				this->heap[hole] = this->heap[child];
				this->ver2loc[this->heap[child]] = hole;
				this->heap[child] = tmp;
				this->ver2loc[tmp] = child;
			}
			else break;
		}
	}

	void percolateUp(graph &_g, colset &_c, int vertex) {
		// vertex refer to the index in the sdQ
		// assumed that this->heap[vertex] is the only invalid element
		assert(vertex <= this->mark);
		int parent;
		for(; (vertex-1)/2 >= 0; vertex = parent) {
			parent = (vertex-1)/2;
			if ((_c.mark[this->heap[vertex]] > _c.mark[this->heap[parent]])
					|| ((_c.mark[this->heap[vertex]] == _c.mark[this->heap[parent]])
							&& ((_g.mark[this->heap[vertex]] > _g.mark[this->heap[parent]])
									|| ((_g.mark[this->heap[vertex]] == _g.mark[this->heap[parent]])
											&& (this->heap[vertex] < this->heap[parent]))))) { // DSATUR rule
				int tmp = this->heap[vertex];
				this->heap[vertex] = this->heap[parent];
				this->ver2loc[this->heap[parent]] = vertex;
				this->heap[parent] = tmp;
				this->ver2loc[tmp] = parent;
			}
			else
				break;
		}
	}

	void insert_node(graph &_g, colset &_c, int index) {
		// index refer to the actual vertex id
		assert(this->ver2loc[index] > this->mark);
		int tmp1 = this->heap[this->mark+1];
		int tmp2 = this->ver2loc[index];

		this->heap[this->mark+1] = index;
		this->ver2loc[index] = this->mark+1;

		this->heap[tmp2] = tmp1;
		this->ver2loc[tmp1] = tmp2;

		this->mark++;

		// update performed once the node is added
		this->percolateUp(_g, _c, this->mark);
	}

	void delete_max(graph &_g, colset &_c) {
		assert(this->mark >= 0);
		int tmp = this->heap[0];

		this->heap[0] = this->heap[this->mark];
		this->ver2loc[this->heap[this->mark]] = 0;

		this->heap[this->mark] = tmp;
		this->ver2loc[tmp] = this->mark;

		this->mark--;

		// update performed once the node is removed
		this->trickleDown(_g, _c, 0);
	}

	void updatesdQ(graph &_g, colset &_c, bool flag,
			int vertex, int vertex1, int color, bool dosdQupdate) {
		// vertex refer to actual vertex, not the index in array

		int tmp1 = _g.mark[vertex1];
		int tmp2 = _c.mark[vertex1];
		bool doReverse = false;
		_g.update(flag, vertex1, vertex);
		_c.update(flag, vertex1, color);
		if(_c.mark[vertex1] == tmp2 && _g.mark[vertex1] != tmp1)
			doReverse = true;
		if(dosdQupdate) {
			assert(ColorClass[vertex1] == -1 && this->ver2loc[vertex1] <= this->mark);
			if(flag) {
				if(doReverse)
					this->trickleDown(_g, _c, this->ver2loc[vertex1]);
				else
					this->percolateUp(_g, _c, this->ver2loc[vertex1]);
			}
			else {
				if(doReverse)
					this->percolateUp(_g, _c, this->ver2loc[vertex1]);
				else
					this->trickleDown(_g, _c, this->ver2loc[vertex1]);
			}
		}
	}

	void update(graph &_g, colset &_c, bool flag, int vertex, int color, double alpha) {
		// vertex refer to the actual one that is being colored/uncolored
		// ? whether the SD or D is more important

		if(true) { // in certain condition, do single updates
			if(flag) {
				assert(vertex == this->heap[0]);
				ColorClass[vertex] = color;
				this->delete_max(_g, _c);
			}
			else {
				ColorClass[vertex] = -1;
				this->insert_node(_g, _c, vertex);
			}
			for(int i = 0; i <= _g.count[vertex]; i++) {
				if(ColorClass[_g.set[vertex][i]] == -1)
					this->updatesdQ(_g, _c, flag, vertex,
							_g.set[vertex][i], color, true);
				else
					this->updatesdQ(_g, _c, flag, vertex,
							_g.set[vertex][i], color, false);
			}
		}
		else { // re-build the entire heap
			if(flag) {
				assert(vertex == this->heap[0]);
				ColorClass[vertex] = color;
				this->delete_max(_g, _c);
			}
			else {
				ColorClass[vertex] = -1;
				this->insert_node(_g, _c, vertex);
			}
			for(int i = 0; i <= _g.count[vertex]; i++) {
				_g.update(flag, _g.set[vertex][i], vertex);
				_c.update(flag, _g.set[vertex][i], color);
			}
			this->build_heap(_g, _c);
		}
	}
};

void updateDS(graph &_g, colset &_c, int vertex, int color) {
	// vertex refer to actual vertex
	for(int i = 0; i <= _g.count[vertex]; i++) {
		_g.update(true, _g.set[vertex][i], vertex);
		_c.update(true, _g.set[vertex][i], color);        
	}
}

// -- end of code for nDS

void read_graph_DIMACS_ascii(char *file, graph &_g) {
	/*
	 * read graph from .col file
	 */
	int c, oc;
    	int order;
    	int i, j, numedges, edgecnt;
    	char tmp[80];
    	FILE *fp;

    	if ((fp = fopen(file, "r")) == NULL){
    		cout<<"ERROR: Cannot open input file\n";
    		exit(10);
    	}

    	for(oc = '\0'; (c = fgetc(fp)) != EOF && ((oc != '\0' && oc != '\n') || c != 'p'); oc = c)
        	;
    	if (!fscanf(fp, "%s %d %d\n", tmp, &order, &numedges)){
    		cout<<"ERROR: corrupted input file in p\n";
		exit(10);
    	}
    	num_node = order;
    	_g.doInit(num_node); // added for nDS

    	ungetc(c, fp);

    	// 18-Mar-2012
    	for(i = 0; i < num_node; i++)
        	for(j = 0; j < num_node; j++)
        		adj[i][j] = FALSE;

    	edgecnt = 0;
    	while ((c = fgetc(fp)) != EOF){
        	switch (c){
            		case 'e':
                	if (!fscanf(fp, "%d %d", &i, &j)) {
                    		cout<<"ERROR: corrupted input file\n";
                    		exit(10);
                	}
                	edgecnt++;
                	i--; j--;
                	_g.addEdges(i, j); // added for nDS
                	adj[i][j] = TRUE;
                	adj[j][i] = TRUE;
                	break;
            		case '\n':
            		default:
            		break;
        	}
    	}
    	fclose(fp);
	cout<<order<<" "<<numedges<<" ";
}

// -- start of code for finding a clique

int greedy_clique(int *valid, int *clique) {
	/*
	 * subfunction called by max_w_clique() function for finding a clique
	 */
	int i, j, k;
    	int max;
    	int place,done;
    	int *order;
    	int weight[MAX_NODE];
	
    	for(i = 0; i < num_node; i++) clique[i] = 0;
    	order = (int *)calloc(num_node+1, sizeof(int));
    	place = 0;
    	for(i = 0; i < num_node; i++){
        	if(valid[i]) {
        		order[place] = i;
            		place++;
        	}
    	}
    	for(i = 0; i < num_node; i++)
        	weight[i] = 0;
    	for(i = 0; i < num_node; i++){
        	if(!valid[i]) continue;
        	for(j = 0; j < num_node; j++){
            		if(!valid[j]) continue;
            		if(adj[i][j]) weight[i]++;
		}
    	}

    	done = FALSE;
    	while(!done){
        	done = TRUE;
        	for(i = 0; i < place-1; i++){
            		j = order[i];
            		k = order[i+1];
            		if(weight[j] < weight[k]){
                		order[i] = k;
                		order[i+1] = j;
                		done = FALSE;
            		}
        	}
    	}

    	clique[order[0]] = TRUE;
    	for(i = 1; i < place; i++){
        	j = order[i];
        	for(k = 0; k < i; k++){
            		if (clique[order[k]] && !adj[j][order[k]]) break;
        	}
        	if(k == i){
            		clique[j] = TRUE;
        	}
        	else clique[j] = FALSE;
    	}
    	max = 0;
    	for(i = 0; i < place; i++)
        	if (clique[order[i]]) max ++;

    	free(order);
    	return max;
}

int max_w_clique(int *valid, int *clique, int lower, int target) {
	/*
	 * to find a clique with size as large as possible
	 */
    	int start, j, k;
    	int incumb, new_weight;
    	int *valid1, *clique1;
    	int *order;
    	int *value;
    	int i, place, finish, done, place1;
    	int total_left;
    	num_prob++;
    	if(num_prob > max_prob) return -1;
    	for(j = 0; j < num_node; j++) clique[j] = 0;
    	total_left = 0;
    	for(i = 0; i < num_node; i++)
        	if(valid[i]) total_left++;
    	if(total_left < lower){
        	return 0;
    	}

    	order = (int *)calloc(num_node+1, sizeof(int));
    	value = (int *)calloc(num_node, sizeof(int));
    	incumb = greedy_clique(valid, clique);
    	if(incumb >= target) return incumb;
    	if(incumb > best_clique){
        	best_clique = incumb;
    	}

    	place = 0;
    	for(i = 0; i < num_node; i++){
      		if(clique[i]){
          		order[place] = i;
          		total_left --;
          		place++;
      		}
    	}
    	start = place;
    	for(i = 0; i < num_node; i++){
        	if(!clique[i] && valid[i]){
            		order[place] = i;
            		place++;
        	}
    	}
    	finish = place;
    	for(place = start; place < finish; place++){
        	i = order[place];
        	value[i] = 0;
        	for(j = 0; j < num_node; j++){
            		if(valid[j] && adj[i][j]) value[i]++;
        	}
    	}

    	done = FALSE;
    	while(!done){
        	done = TRUE;
        	for(place = start; place < finish-1; place++){
            		i = order[place];
            		j = order[place+1];
            		if(value[i] < value[j]){
                		order[place] = j;
                		order[place+1] = i;
                		done = FALSE;
            		}
        	}
    	}

    	free(value);
    	for(place = start; place < finish; place++){
        	if(incumb + total_left < lower){
            		return 0;
        	}

        	j = order[place];
        	total_left --;

        	if (clique[j]) continue;

        	valid1 = (int *)calloc(num_node, sizeof(int));
        	clique1 = (int *)calloc(num_node, sizeof(int));
        	for(place1 = 0;place1 < num_node; place1++) valid1[place1] = FALSE;
        	for(place1 = 0;place1 < place; place1++){
            		k = order[place1];
            		if (valid[k] && (adj[j][k])){
                		valid1[k] = TRUE;
            		}
            		else
                		valid1[k] = FALSE;
        	}
        	new_weight = max_w_clique(valid1, clique1, incumb-1, target-1);
        	if(new_weight+1 > incumb){
            		incumb = new_weight + 1;
            		for(k = 0; k < num_node; k++) clique[k] = clique1[k];
            		clique[j] = TRUE;
            		if(incumb > best_clique){
                		best_clique = incumb;
            		}
        	}

        	free(valid1);
        	free(clique1);
        	if (incumb >= target) break;
	}
    	free(order);
    	return(incumb);
}

// -- end of code for finding a clique

void print_colors() {
	/*
	 * print out bestColoring
	 */
	/*
	output<<"#\tResult"<<endl;
	for(int i = 0; i < num_node; i++)
		output<<"#\tv"<<i<<": "<<ColorClass[i]<<endl;
		*/
}

int color(graph &_g, colset &_c, sdQ &_s, int current_color) {
	/*
	 * core DSATUR function
	 */
	int j, place;
	while(_s.mark >= 0) {
		place = _s.heap[0];
		for(j = 0; j <= current_color; j++) {
			if(_c.col2loc[place][j] == -1) {
				assert(_s.heap[0] == place);
				_s.update(_g, _c, true, place, j, alpha);
				break;
			}
		}
		if(ColorClass[place] == -1) {
			current_color++;
			_s.update(_g, _c, true, place, current_color, alpha);
		}
	}
    return current_color;
}

int main(int argc, char** argv) {
	/*
	 * main function
	 */
	int i,val;
	int valid[MAX_NODE], clique[MAX_NODE];
	int place;

	if(argc != 2) {
		cerr<<argv[0]<<" <dimacs_file>"<<endl;
		return 0;
	}

	//output.open("/home/andrewju/tmp/log1"); // file location atm
	graph _g;
	read_graph_DIMACS_ascii(argv[1], _g); // read from .col file
	colset _c;

	for(i = 0; i < num_node; i++) {
		ColorClass[i] = -1;
		valid[i] = TRUE;
	}
	best_clique = 0;
	num_prob = 0;
	max_prob = 100000;
	clock_t start1 = clock();
	lb = max_w_clique(valid, clique, 0, num_node); // find a clique
	clock_t start2 = clock();
	cout<<"|l|: "<<lb<<"(";
	cout<<((num_prob >= max_prob)? "TBC": "C")<<")";
	sdQ _s(clique);
	place = -1;
	cout<<endl;
	for(i = 0; i < num_node; i++) {
		if(clique[i]) {
			cout<<i<<" ";
			place++;
			ColorClass[i] = place;
			updateDS(_g, _c, i, place); }
	}
	cout<<endl;

	_s.build_heap(_g, _c);

	val = color(_g, _c, _s, place) + 1; // color function
	printf(" |X-h|: %d ", val);
	clock_t ends = clock();
	cout<<"Time: "<<(double)(ends - start2)/CLOCKS_PER_SEC<<" "<<(double)(ends - start1)/CLOCKS_PER_SEC<<" (h-DSATUR-nDS)"<<endl;

}
