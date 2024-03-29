
#if ! defined(COLSETH)
#define COLSETH

#include "pset.h"

class colset { // represent the adjacent color matrix in pset concept
public:
  int **colCt; // additional support for colset
	int num_node;
  std::vector<pset> neighbors;	// partitioned into two parts

  int colCount(int vertex) { return neighbors[vertex].countIns(); }

	colset(int numNode) {

		num_node = numNode;
		colCt = new int *[numNode];
    for (int v = 0; v < numNode; v++) {
      neighbors.push_back(pset(numNode, false));
			colCt[v] = new int[numNode];
			for(int j = 0; j < numNode; j++)
				colCt[v][j] = 0;
		}
	}

  void printNeighbors()
  {
    for (int v = 0; v < num_node; v++)
    {
      std::cout<< "vertex "<< v<< "adjacent to ";
      neighbors[v].printIns();
    }
  }

  void addColor(int ver, int color) { // add adjacent color to vertex
    if(! neighbors[ver].isIn(color))
    { // avoid duplication

      neighbors[ver].insert(color);
    }
  }

	void update(bool flag, int target, int color) { // update colset
		// target refers to actual vertex
		if(flag) { // being colored
			if(!neighbors[target].isIn(color)) {
				neighbors[target].insert(color);
				colCt[target][neighbors[target].loc(color)]++;
			}
			else
				colCt[target][neighbors[target].loc(color)]++;
		}
		else { // being uncolored
			assert(neighbors[target].isIn(color));
			if(colCt[target][neighbors[target].loc(color)] > 1)
				colCt[target][neighbors[target].loc(color)]--;
			else {
				assert(colCt[target][neighbors[target].loc(color)] == 1);
				int where = neighbors[target].loc(color);
				colCt[target][where] = colCt[target][neighbors[target].countIns()-1];
				colCt[target][neighbors[target].countIns()-1] = 0;
				neighbors[target].moveOut(color);
				neighbors[target].reset(neighbors[target].countIns());
			}
		}
	}
};

#endif
