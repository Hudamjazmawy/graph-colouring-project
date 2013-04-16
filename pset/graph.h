
#if ! defined(GRAPHH)
#define GRAPHH

#include "pset.h"

class graph { // represent the graph in pset concept
public:
	int num_node;
  std::vector<pset> neighbors;	// partitioned into uncol'd (l) | col'd (r)

  int colCt(int vertex) { return neighbors[vertex].countOuts(); }
  int uncolCt(int vertex) { return neighbors[vertex].countIns(); }
  int nbrCount(int vertex) { return neighbors[vertex].elCount(); }

  graph() {
  }

  void doInit(int numNode) {
    // see http://stackoverflow.com/questions/4754763/
    //    c-object-array-initialization-without-default-constructor
    //    for why we need a vector of them

    //    neighbors = std::vector<pset>(numNode, pset(numNode, false));
		num_node = numNode;
    for (int v = 0; v < numNode; v++)
      neighbors.push_back(pset(numNode, false));
  }

  void printNeighbors()
  {
    for (int v = 0; v < num_node; v++)
    {
      std::cout<< "vertex "<< v<< " ";
      neighbors[v].printIns();
    }
  }

  void addEdge(int ver1, int ver2) { // add edges to graph
    if(ver1 != ver2 && !neighbors[ver1].isIn(ver2))
    { // avoid duplication
      neighbors[ver1].insert(ver2);
      neighbors[ver2].insert(ver1);
    }
  }

  void update(bool flag, int index1, int index2) { // update graph
    // index1 refer to target index
    // index2 refer to actual moving object index(one that is being colored/uncolored)
    if(flag) { // being colored
      assert(neighbors[index1].isIn(index2));
      neighbors[index1].moveOut(index2);
    }  else { // being uncolored
      assert(!neighbors[index1].isIn(index2));
      neighbors[index1].moveIn(index2);
    }
  }
};

#endif
