
#if ! defined(PSETH)
#define PSETH

#include <vector>
#include <iostream>

using std::vector;

// a set partitioned into two
//  - the in part and the out part; the in part is, in an array
//    implementation, to the left side of the array, and the out part is
//    to the right
//  - given an element 'el' to insert or remove from the partition
//  -  where[el] gives the index in the set where 'el' is stored;
//  -  'el' is in the partition iff 0 <= where[el] <= bar
class pset {
 public:
  pset(int maxsz, bool allIn = false)
  {
    parts = (int *) malloc(sizeof(int) * maxsz); // in general, dict<T,int>
    where = (int *) malloc(sizeof(int) * maxsz); // inverse mapping

    maxSize = maxsz;
    count = 0;  bar = -1;
    for (int i = 0; i < maxSize; i++)
      parts[i] = where[i] = -1;

    if (allIn == true)
    {
      count = maxsz; bar = count-1;
    }
  }

  int elCount() { return count; }

  // is 'el' in left partition
  bool isIn(int el)
  {
    assert(0 <= el && el < maxSize);
    assert(-1 <= where[el] && where[el] < maxSize);
		if(where[el] == -1) return false;
    return (where[el] <= bar);
  }

  // insert an element into left partition; this increases
  //   the total count of elements whereas moveIn() only changes
  //   el from the right side to the left side
  void insert(int el)
  { 
    assert(0 <= el && el < maxSize);

    bar++;    count++;

    parts[bar] = el;
    where[el] = bar;
    //std::cout<< "inserting "<< el<< " at posn="<< bar<< std::endl;
  }

  // move 'el' into partition
  // precond: 'el' is not in left partition already
  void moveIn(int el)
  {
    assert(!isIn(el));
    int ind = where[el];

    // an easy case: if ind is to right of barrier, just move barrier
    if (ind == bar+1) { bar++; return; }
    
    int vind = bar+1;		// the victim, just to right of barrier
    int vdata = parts[vind];
    // now the "swap"
    parts[ind] = vdata;
    where[vdata] = ind;
    parts[vind] = el;
    where[el] = vind;

    bar++;
  }

  // move 'el' out
  // precond: 'el' is in left partition
  void moveOut(int el)
  {
    assert(isIn(el));
    int ind = where[el];

    // an easy case: if ind is to left of barrier, just move barrier
    if (ind == bar) { bar--; return; }
    
    int vind = bar;		// the victim, just to left of barrier
    int vdata = parts[vind];
    // now the "swap"
    parts[ind] = vdata;
    where[vdata] = ind;
    parts[vind] = el;
    where[el] = vind;

    bar--;
  }

  vector<int> whatIns()
  {
    vector<int> ins;		// the elements in the partition
    for (int p = 0; p <= bar; p++)
      ins.push_back(parts[p]);

    return ins;
  }

  vector<int> whatOuts()
  {
    vector<int> outs;		// the elements not in the partition
    for (int p = bar+1; p < count; p++)
      outs.push_back(parts[p]);

    return outs;
  }
	
	vector<int> whatAlls()
	{
		vector<int> alls; // the elements in the list: both Ins and Outs
		for(int p = 0; p < count; p++)
			alls.push_back(parts[p]);

		return alls;
	}

	int loc(int el) 
	{ 
		return where[el];
	}

	int reset(int loc)
	{
		where[parts[loc]] = -1;
		parts[loc] = -1;
	}

  int countIns()
  {
    return bar+1;		// the range of Ins is [0,bar]
  }

  int countOuts()
  {
    return count-1-bar;		// the range of Outs is [bar+1,count)
  }

  void printIns()
  {
    std::cout<< "Ins are: ";
    for (int p = 0; p <= bar; p++)
      std::cout<< parts[p] << " ";
    std::cout<< "\n";
  }

 private:
  int *parts;	     // what element (data) is stored at a location
  int *where;	     // a dictionary giving the location of an element
  int bar;	     	 // separating in from out: in is [0,bar]
  int count;	     // no. of els in set (both sides)
  int maxSize;
};

#endif
