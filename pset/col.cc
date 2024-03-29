//============================================================================
// Name        : col.cc
// Author      : Andrew Ju
// Version     :
// Copyright   : 2013 - present
// Description : DSATUR-pset in testing stage
//============================================================================

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <cstdlib>
#include <fstream>

#include "pset.h"
#include "graph.h"
#include "colset.h"

using namespace std;
#define MAX_NODE 2500
#define TRUE 1
#define FALSE 0

int flag;
int ColorClass[MAX_NODE];	// would be nice to encapsulate this, change in next time!!!!!!!!!!
int **adj;			// now allocated in read_graph_DIMACS_ascii()

ofstream output;

int BestColoring;
double alpha = 0.4; // additional parameter, random here

int lb; // lower bound(size of clique)
int num_node;
int prob_count, num_prob, max_prob;
int best_clique;

// -- start of 'priority queue' code

class sdQ { // using the sdQ to decide next candidate vertex to color
public:
  int *heap;
  int *ver2loc;
  int mark;

  sdQ(int* _clique, graph& _g, colset& _c) {
    int place = num_node;
    int colInd = -1;
    mark = -1;
    heap = new int[num_node];
    ver2loc = new int[num_node];
    for(int v = 0; v < num_node; v++) {
      if(!_clique[v]) {
				mark++;
				heap[mark] = v;
				ver2loc[v] = mark;
      } 
			else {
				place--;
				heap[place] = v;
				ver2loc[v] = place;
      }
    }
    assert(mark+1 == place);
  }

  void build_heap(graph &_g, colset &_c) {
    for(int i = (mark-1)/2; i >= 0; i--)
      trickleDown(_g, _c, i);
  }

  // is v1 in heap higher priority than v2
  bool prioDSATUR(int v1, int v2, graph &_g, colset &_c)
  {
    if (_c.neighbors[v1].countIns() > _c.neighbors[v2].countIns()) return true;
    if (_c.neighbors[v1].countIns() < _c.neighbors[v2].countIns()) return false;
    // assert(_c.mark[v1] == _c.mark[v2]);
    if (_g.uncolCt(v1) > _g.uncolCt(v2)) return true;
    if (_g.uncolCt(v1) < _g.uncolCt(v2)) return false;
    // assert(_g.uncolCt(v1) < _g.uncolCt(v2));
    return (v1 < v2);
  }

  void trickleDown(graph &_g, colset &_c, int hole) {
    assert(hole <= mark+1);
    int child;
    int v, va;			// vertex and adjacant vertex in heap
    int vh;			// vertex at hole posn of heap

    for(; hole*2+1 <= mark; hole = child)
    {
      child = hole*2+1;
      v = heap[child]; va = heap[child+1];
      if (child != mark &&
	  prioDSATUR(va, v, _g, _c)
	  // ((_c.mark[heap[child]] < _c.mark[heap[child+1]])
	  //  ||((_c.mark[heap[child]] == _c.mark[heap[child+1]])
	  //     && ((_g.uncolCt(v) < _g.uncolCt(va))
	  // 	  || ((_g.uncolCt(v) == _g.uncolCt(va))
	  // 	      && (v > va)))))
	  ) // DSATUR rule
	child++;
      v = heap[child]; vh = heap[hole];
      // if ((_c.mark[heap[child]] > _c.mark[heap[hole]])
      // 	  ||((_c.mark[heap[child]] == _c.mark[heap[hole]])
      // 	     && ((_g.uncolCt(v) > _g.uncolCt(vh))
      // 		 || ((_g.uncolCt(v) == _g.uncolCt(vh))
      // && (v < vh)))))
      if (prioDSATUR(v, vh, _g, _c))
      { // DSATUR rule
	int tmp = heap[hole];
	heap[hole] = heap[child];
	ver2loc[heap[child]] = hole;
	heap[child] = tmp;
	ver2loc[tmp] = child;
      } else
	break;
    }
  }

  void percolateUp(graph &_g, colset &_c, int vertex) {
    // vertex refer to the index in the sdQ
    // assumed that heap[vertex] is the only invalid element
    assert(vertex <= mark);
    int parent;
    int v, vp;			// vertex and parent in heap
    for(; (vertex-1)/2 >= 0; vertex = parent)
    {
      parent = (vertex-1)/2;
      v = heap[vertex]; vp = heap[parent];
      if (prioDSATUR(v, vp, _g, _c))
      // if ((_c.mark[heap[vertex]] > _c.mark[heap[parent]])
      // 	  || ((_c.mark[heap[vertex]] == _c.mark[heap[parent]])
      // 	      && ((_g.uncolCt(v) > _g.uncolCt(vp))
      // 		  || ((_g.uncolCt(v) == _g.uncolCt(vp))
      // 		      && (v < vp)))))
      { // DSATUR rule
	int tmp = heap[vertex];
	heap[vertex] = heap[parent];
	ver2loc[heap[parent]] = vertex;
	heap[parent] = tmp;
	ver2loc[tmp] = parent;
      } else
	break;
    }
  }

  void insert_node(graph &_g, colset &_c, int index) {
    // index refer to the actual vertex id
    assert(ver2loc[index] > mark);
    int tmp1 = heap[mark+1];
    int tmp2 = ver2loc[index];

    heap[mark+1] = index;
    ver2loc[index] = mark+1;

    heap[tmp2] = tmp1;
    ver2loc[tmp1] = tmp2;

    mark++;

    // update performed once the node is added
    percolateUp(_g, _c, mark);
  }

  void delete_max(graph &_g, colset &_c) {
    assert(mark >= 0);
    int tmp = heap[0];

    heap[0] = heap[mark];
    ver2loc[heap[mark]] = 0;

    heap[mark] = tmp;
    ver2loc[tmp] = mark;

    mark--;

    // update performed once the node is removed
    trickleDown(_g, _c, 0);
  }

  void updatesdQ(graph &_g, colset &_c, bool flag,
		 int vertex, int vertex1, int color, bool dosdQupdate) {
    // vertex refer to actual vertex, not the index in array

    int tmp1 = _g.uncolCt(vertex1);
    int tmp2 = _c.neighbors[vertex1].countIns();
    bool doReverse = false;
    _g.update(flag, vertex1, vertex);
    _c.update(flag, vertex1, color);
    if(_c.neighbors[vertex1].countIns() == tmp2 &&
       _g.uncolCt(vertex1) != tmp1)
      doReverse = true;
    if(dosdQupdate) {
      assert(ColorClass[vertex1] == -1 && ver2loc[vertex1] <= mark);
      if(flag) {
	if(doReverse)
	  trickleDown(_g, _c, ver2loc[vertex1]);
	else
	  percolateUp(_g, _c, ver2loc[vertex1]);
      }
      else {
	if(doReverse)
	  percolateUp(_g, _c, ver2loc[vertex1]);
	else
	  trickleDown(_g, _c, ver2loc[vertex1]);
      }
    }
  }

  void update(graph &_g, colset &_c, bool flag, int vertex, int color, double alpha) {
    // vertex refer to the actual one that is being colored/uncolored
    // ? whether the SD or D is more important

    if(false) { // in certain condition, do single updates
      if(flag) {
	assert(vertex == heap[0]);
	ColorClass[vertex] = color;
	delete_max(_g, _c);
      }
      else {
	ColorClass[vertex] = -1;
	insert_node(_g, _c, vertex);
      }
      for(int i = 0; i < _g.nbrCount(vertex); i++) {
	if(ColorClass[_g.neighbors[vertex].whatAlls()[i]] == -1)
	  updatesdQ(_g, _c, flag, vertex, _g.neighbors[vertex].whatAlls()[i], color, true);
	else
	  updatesdQ(_g, _c, flag, vertex, _g.neighbors[vertex].whatAlls()[i], color, false);
      }
    }
    else { // re-build the entire heap
      if(flag) {
	assert(vertex == heap[0]);
	ColorClass[vertex] = color;
	delete_max(_g, _c);
      }
      else {
	ColorClass[vertex] = -1;
	insert_node(_g, _c, vertex);
      }
      for(int i = 0; i < _g.nbrCount(vertex); i++) {
	_g.update(flag, _g.neighbors[vertex].whatAlls()[i], vertex);
	_c.update(flag, _g.neighbors[vertex].whatAlls()[i], color);
      }
      build_heap(_g, _c);
    }
  }
};

// -- end of code for nDS

void read_graph_DIMACS_ascii(char *file, graph &_g)
{
  /*
   * read graph from .col file
   */
  int c, oc;
  int order;
  int i, j, numedges, edgecnt;
  char tmp[80];
  FILE *fp;

  if ((fp = fopen(file, "r")) == NULL){
    output<<"ERROR: Cannot open input file\n";
    exit(10);
  }

  for(oc = '\0'; (c = fgetc(fp)) != EOF && ((oc != '\0' && oc != '\n') || c != 'p'); oc = c)
    ;
  if (!fscanf(fp, "%s %d %d\n", tmp, &order, &numedges)){
    output<<"ERROR: corrupted input file in p\n";
    exit(10);
  }
  num_node = order;
  _g.doInit(num_node); // added for nDS

  ungetc(c, fp);

  adj = (int**) calloc(order, sizeof(int *));
  if (adj == 0)
  {
    fprintf(stderr, "Couldn't calloc adj vector (%d)\n", order);
    exit(-1);
  }
  for (int r = 0; r < order; r++)
  {
    int *row = (int *) calloc(order, sizeof(int));
    if (row == 0)
    {
      fprintf(stderr, "Couldn't calloc adj row (%d).\n", order);
      exit(1);
    }
    adj[r] = row;
  }

  // 18-Mar-2012
  for(i = 0; i < num_node; i++)
    for(j = 0; j < num_node; j++)
      adj[i][j] = FALSE;

  edgecnt = 0;
  while ((c = fgetc(fp)) != EOF){
    switch (c){
    case 'e':
      if (!fscanf(fp, "%d %d", &i, &j)) {
	output<<"ERROR: corrupted input file\n";
	exit(10);
      }
      edgecnt++;
      i--; j--;
      _g.addEdge(i, j); // added for nDS
      adj[i][j] = TRUE;
      adj[j][i] = TRUE;
      break;
    case '\n':
    default:
      break;
    }
  }
  fclose(fp);
  output<<"#\t|V|: "<<order<<"\n#\t|E|: "<<numedges<<endl;
  //  _g.printNeighbors();
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


void print_colors()
{
  /*
   * print out bestColoring
   */
  output<<"#\tResult"<<endl;
  for(int i = 0; i < num_node; i++)
    output<<"#\tv"<<i<<": "<<ColorClass[i]<<endl;
}

int color(graph &_g, colset &_c, sdQ &_s, int current_color)
{
  /*
   * core DSATUR function
   */
  int j, place, new_val;
  prob_count++;
  if(current_color >= BestColoring)
    return current_color;
  if(BestColoring <= lb-1)
    return BestColoring;

  if (_s.mark < 0) {
    print_colors();
    return current_color; // reached leaf node, return
  }

  place = _s.heap[0];
  for(j = 0; j <= current_color; j++) {
		//_c.neighbors[place].printIns();
    if(!_c.neighbors[place].isIn(j))
    {
      assert(_s.heap[0] == place);
      _s.update(_g, _c, true, place, j, alpha);
      new_val = color(_g, _c, _s, current_color);

      if(new_val < BestColoring)
      {
	BestColoring = new_val;
	if(BestColoring <= lb-1)
	  return BestColoring;
      }
      _s.update(_g, _c, false, place, j, alpha);

      if(BestColoring <= current_color)
	return BestColoring;
    }
  }

  if(current_color+1 < BestColoring) {
    /*
     * new color current_color+1 is added
     * refer to Pablo's paper, Definition in pp 1725
     */
    assert(_s.heap[0] == place);
    _s.update(_g, _c, true, place, current_color+1, alpha);
    new_val = color(_g, _c, _s, current_color+1);
    if(new_val < BestColoring) {
      BestColoring = new_val;
      if(BestColoring <= lb-1)
	return BestColoring;
    }
    _s.update(_g, _c, false, place, current_color+1, alpha);
    assert(_s.heap[0] == place);
  }
  return BestColoring;
}

int main(int argc, char** argv)
{
  /*
   * main function
   */
  int i,val;
  int valid[MAX_NODE], clique[MAX_NODE];

  flag = 0; // testing mode on
  if(argc != 2) {
    cerr<<argv[0]<<" <dimacs_file>"<<endl;
    return 0;
  }
  char * outputfile = new char[256];
  strcpy(outputfile, argv[1]);

  strcat(outputfile, ".gol");
  output.open (outputfile);
  // Version infor
  if(flag != 1) {
    // Version infor
    cout<<"This is DSATUR/nDS Application, Version ";
    cout<<3.2;
    cout<<" (GCC/OS X)"<<endl;
    cout<<"#\tgraph: "<<argv[1]<<endl;
  }
  graph _g;
  read_graph_DIMACS_ascii(argv[1], _g); // read from .col file
  colset _c(num_node);

  prob_count = 0;
  for(i = 0; i < num_node; i++) {
    ColorClass[i] = -1;
    valid[i] = TRUE;
  }
  BestColoring = num_node + 1;
  num_prob = 0;
  max_prob = 100000;

  lb = max_w_clique(valid, clique, 0, num_node); // find a clique
  clock_t start = clock();

  int _flag = 1;
  if(num_prob >= max_prob)
    _flag = 0;
  if(flag != 1) {
    cout<<"#\t|l|: "<<lb;
    output<<"#\t|l|: "<<lb;
    if(_flag) {
      cout<<"(C)\n";
      output<<"(C)\n";
    }
    else {
      cout<<"(TBC)";
      output<<"(TBC)\n";
    }

  }
  sdQ _s(clique, _g, _c);

	int place = -1;
	for(i = 0; i < num_node; i++) {
		if(clique[i]) {
			place++;
			ColorClass[i] = place;
			for(int j = 0; j < _g.nbrCount(i); j++) {
				_g.update(true, _g.neighbors[i].whatAlls()[j], i);
				_c.update(true, _g.neighbors[i].whatAlls()[j], place);
			}
		}
	}
  _s.build_heap(_g, _c);

  num_prob = 0;
  _flag = 1;

  val = color(_g, _c, _s, place) + 1; // color function: have used [0,..,lb-1]
  if(prob_count >= max_prob)
    _flag = 0;
  if(val == -1) {
    cout<<"#\ttime out\n";
    output<<"#\ttime out\n"<<endl;
  }
  else {
    if(_flag) {
      printf("#\t|X|: %d(C) %d\n", val, prob_count);
      output<<"#\t|X|: "<<val<<"(C)\n";
    }
    else {
      printf("#\t|X|: %d(TBC) %d\n", val, prob_count);
      output<<"#\t|X|: "<<val<<"(TBC)\n";
    }
  }

  clock_t ends = clock();
  output <<"#\tRunning time: "<<(double) (ends - start) /CLOCKS_PER_SEC<<endl;
  output.close();

}
