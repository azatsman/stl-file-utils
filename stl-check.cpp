// Check that:
//    Each edge has exactly 2 adjacent triangles, and
//      the edge is traversed in opposite directions by the two adjacent triangles.
//  Implementation: map edges (or vertices ?) to lists of triangles.
//  Use rounding to make sure the same edge is not listed as two different ones
//    because of floating point error ?  Maybe count unique edges first and see if 
//    rounding is really necessary.
//    Make sure that the edge's representation is unique (lexicographical XYZ order?)
//   -- For rounding: find the minimal side length, and use 1/10 of that for rounding ?
//   -- Build a histogram of values mod rounding ?

#include "stlfile.hpp"
 
#include <cmath>
#include <cstdio>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

// typedef StlInBinFile<double> STLF;


typedef Vec3<float>     V3;

static double Epsilon  = 1e-3;
static double MaxRange = 1e9;

static bool lessV3(V3 u, V3 v)
{
  for (int k=0; k<3; k++) {
    if      (u.p[k] < v.p[k])
      return true;
    else if (u.p[k] > v.p[k])
      return false;
  }
  return false;
}

struct CompV3 {
  bool operator() (V3 u, V3 v) const {
    for (int k=0; k<3; k++) {
      if      (u.p[k] < v.p[k])
	return true;
      else if (u.p[k] > v.p[k])
	return false;
    }
    return false;
  };
};

//.................................. Ordered edge:
struct OrdEdge {
  V3 pnt0;
  V3 pnt1;
  OrdEdge (V3 p0, V3 p1) {
    pnt0 = p0;
    pnt1 = p1;
  }
  int reorder () {
    if (lessV3(pnt0, pnt1))
      return 1;
    else {
      std::swap(pnt0, pnt1);
      return -1;
    }
  }
};

struct lessEdge {
  bool operator() (OrdEdge e1, OrdEdge e2) const
  {
    if      (lessV3(e1.pnt0, e2.pnt0))
      return true;
    else if (lessV3(e2.pnt0, e1.pnt0))
      return false;
    else if (lessV3(e2.pnt1, e1.pnt1))
      return true;
    else
      return false;
  }
};
//...........................  Set of up to two triangles:
struct TrigSet {
  int numTrigs;
  V3 trigs[2][3];
  int sign[2];

  TrigSet() {numTrigs=0;};

  void addTrig(V3 trg[3], int sign) {
    if (numTrigs < 2) {
      trigs[numTrigs][0] = trg[0];
      trigs[numTrigs][1] = trg[1];
      trigs[numTrigs][2] = trg[2];
      this->sign[numTrigs] = sign;
    }
    else
      throw (std::string("More than 2 adjacent triangles"));
    numTrigs++;
  }
  
};

std::map<OrdEdge, TrigSet, lessEdge>  edgeMap;
std::map<V3,      int,     CompV3>    vertexMap;


typedef Vec3<float>     V3;

template <typename T>
static  std::string type2string (T x)
{
  std::ostringstream oss;
  oss << x;
  return oss.str();
}

static void usage (char * progName) {
  std::cout << "Usage : "
            << progName
            << " <input-name> [<epsilon> [<max-range> [<verbosity>]]]"
            << std::endl;

  std::cout << "       where: " << std::endl;

  std::cout << "  <input-name> is the name of an STL file " << std::endl;
  std::cout << "  <epsilon>    is smallest distance between vertices before vertices are considered identical " << std::endl;
  std::cout << "  <max-range>  is the largest allowed value of vertex coordinate " << std::endl;
  std::cout << "  <verbosity>  is verbosity level of the output " << std::endl;
}

int main (int argc, char *argv[])
{
  int trNum = 0;
  int verbosity = 0;
  //  double totalArea = 0;
  V3 curTrig[3];
  V3 curNormal;
  try {
    if (argc < 2) {
      usage (argv[0]);
      return 3;
    }
    
    StlInFile stlf (argv[1]);
    if (argc > 2)
      sscanf (argv[2], "%lf", &Epsilon);
    if (argc > 3)
      sscanf (argv[3], "%lf", &MaxRange);
    if (argc > 4)
      sscanf (argv[4], "%d", &verbosity);

    // int numDclTrngl = stlf.numTriangles();


    for (trNum=0; ; trNum++) {
      if (! stlf.readTriangle(curTrig, curNormal))
        break;

      OrdEdge
	edgeKey01(curTrig[0], curTrig[1]),
	edgeKey12(curTrig[1], curTrig[2]),
	edgeKey20(curTrig[2], curTrig[0]);
      int
	sgn01 = edgeKey01.reorder(),
	sgn12 = edgeKey12.reorder(),
	sgn20 = edgeKey20.reorder();
      edgeMap[edgeKey01].addTrig(curTrig, sgn01);
      edgeMap[edgeKey12].addTrig(curTrig, sgn12);
      edgeMap[edgeKey20].addTrig(curTrig, sgn20);
      vertexMap[curTrig[0]]++;
      vertexMap[curTrig[1]]++;
      vertexMap[curTrig[2]]++;
    }
    for (std::map<OrdEdge, TrigSet>::const_iterator p = edgeMap.begin();
	 p != edgeMap.end();
	 p++) {
      if (p->second.numTrigs != 2) {
	printf("Edge ((%lf,%lf,%lf),(%lf,%lf,%lf)) has %d adjacent triangles\n",
		p->first.pnt0.x(), p->first.pnt0.y(), p->first.pnt0.z(),
		p->first.pnt1.x(), p->first.pnt1.y(), p->first.pnt1.z(), p->second.numTrigs);
      }
      int signSum = p->second.sign[0] + p->second.sign[1];
      if (signSum) {
	printf("Edge ((%lf,%lf,%lf),(%lf,%lf,%lf)) has non-0 multiplicity (%d)\n",
		p->first.pnt0.x(), p->first.pnt0.y(), p->first.pnt0.z(),
		p->first.pnt1.x(), p->first.pnt1.y(), p->first.pnt1.z(), signSum);
      }
      if (verbosity > 0) {
	printf("\n");
	printf("%12.6f %12.6f %12.6f   %12.6f %12.6f %12.6f edge\n",
	       p->first.pnt0.x(), p->first.pnt0.y(), p->first.pnt0.z(),
	       p->first.pnt1.x(), p->first.pnt1.y(), p->first.pnt1.z());
	printf("%12.6f %12.6f %12.6f   %12.6f %12.6f %12.6f %12.6f %12.6f %12.6f trig0\n",
	       p->second.trigs[0][0].x(),p->second.trigs[0][0].y(),p->second.trigs[0][0].z(),
	       p->second.trigs[0][1].x(),p->second.trigs[0][1].y(),p->second.trigs[0][1].z(),
	       p->second.trigs[0][2].x(),p->second.trigs[0][2].y(),p->second.trigs[0][2].z());
	printf("%12.6f %12.6f %12.6f   %12.6f %12.6f %12.6f %12.6f %12.6f %12.6f trig1\n",
	       p->second.trigs[1][0].x(),p->second.trigs[1][0].y(),p->second.trigs[1][0].z(),
	       p->second.trigs[1][1].x(),p->second.trigs[1][1].y(),p->second.trigs[1][1].z(),
	       p->second.trigs[1][2].x(),p->second.trigs[1][2].y(),p->second.trigs[1][2].z());
      }
    }
  }
  catch (std::string s) {
    std::cerr << " EXCEPTION : " << s << std::endl;
    std::cerr << " Last triangle number : " << trNum << std::endl;
    return 3;
  }
  int
    numVertices = vertexMap.size(),
    numEdges    = edgeMap.size(),
    euler       = numVertices - numEdges + trNum;
  std::cout << numVertices << " vertices " 
	    << numEdges   << " edges "
	    << trNum            << " faces " 
	    << euler  << " euler number " 
	    << std::endl;
  return 0;
}
