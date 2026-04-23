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
#include "stl-collections.hpp"

static double Epsilon  = 1e-6;
static double MaxRange = 1e9;

#if 0   // to be moved to stl-collections

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
    return lessV3 (u, v);
    // return (u < v);
  }
};

//............................................ Ordered edge:

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
#endif


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

// std::map<OrdEdge, TrigSet, lessEdge>  edgeMap;

EdgeMap edgeMap;

std::map<V3,      int,     CompV3>    vertexMap;

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

static void checkTrig (const Triangle trngl)
{
  //............................................ Check the vertices: 
  for (int v=0; v<3; v++)
    for (int j=0; j<3; j++) {
      if (std::isnan(trngl.vertices[v].p[j])) 
	throw(std::string("NaN in a triangle"));
      else if (std::isinf(trngl.vertices[v].p[j]))
	throw(std::string("Infinity in a triangle"));
    }
  //............................................ Check the normal: 
  for (int j=0; j<3; j++) {
    if (std::isnan(trngl.normal.p[j]))
      throw(std::string("NaN in a normal"));
    else if (std::isinf(trngl.normal.p[j]))
      throw(std::string("Infinity in a normal"));
  }
}


int main (int argc, char *argv[])
{
  int trNum = 0;
  int verbosity = 0;
  //  double totalArea = 0;
  //  V3 curTrig[3];
  //  V3 curNormal;

  Triangle curTrngl;

  TriangleArray trigArray;
  
  try {
    if (argc < 2) {
      usage (argv[0]);
      return 3;
    }
    if (argc > 2)
      sscanf (argv[2], "%lf", &Epsilon);
    if (argc > 3)
      sscanf (argv[3], "%lf", &MaxRange);
    if (argc > 4)
      sscanf (argv[4], "%d", &verbosity);

    StlInFile stlf (argv[1], Epsilon);

    // int numDclTrngl = stlf.numTriangles();

    for (trNum=0; ; trNum++) {
      if (! stlf.readTriangle (curTrngl))
        break;
      checkTrig (curTrngl);
      int trigNum = storeTriangle (trigArray, curTrngl);
      addTriangleEdges (edgeMap, curTrngl, trigNum);
      vertexMap[curTrngl.vertices[0]]++;
      vertexMap[curTrngl.vertices[1]]++;
      vertexMap[curTrngl.vertices[2]]++;
    }

    for (auto e : edgeMap) {
      EdgeDesc eDesc = e.second;
      int numTriangles = eDesc.trindices.size();
      if (numTriangles != 2) {
	printf("Edge ((%lf,%lf,%lf),(%lf,%lf,%lf)) has %d adjacent triangles\n",
               e.first.pnt0.x(), e.first.pnt0.y(), e.first.pnt0.z(),
               e.first.pnt1.x(), e.first.pnt1.y(), e.first.pnt1.z(),
               numTriangles);
      }
      if (eDesc.sign != 0) {
	printf("Edge ((%lf,%lf,%lf),(%lf,%lf,%lf)) has non-0 multiplicity (%d)\n",
               e.first.pnt0.x(), e.first.pnt0.y(), e.first.pnt0.z(),
               e.first.pnt1.x(), e.first.pnt1.y(), e.first.pnt1.z(), eDesc.sign);
      }
      if (verbosity > 0) {

	printf("\n");
	printf("%12.6f %12.6f %12.6f   %12.6f %12.6f %12.6f edge\n",
	       e.first.pnt0.x(), e.first.pnt0.y(), e.first.pnt0.z(),
	       e.first.pnt1.x(), e.first.pnt1.y(), e.first.pnt1.z());
        for (auto tix : eDesc.trindices) {
          const Triangle & trngl = trigArray[tix];
          printf("%12.6f %12.6f %12.6f   %12.6f %12.6f %12.6f %12.6f %12.6f %12.6f trig0\n",
                 trngl.vertices[0].x(),trngl.vertices[0].y(),trngl.vertices[0].z(),
                 trngl.vertices[1].x(),trngl.vertices[1].y(),trngl.vertices[1].z(),
                 trngl.vertices[2].x(),trngl.vertices[2].y(),trngl.vertices[2].z());
        }
      }
    }
  }  
  catch (std::string s) {
    int trNum  = trigArray.size();
    std::cerr << " EXCEPTION : " << s << std::endl;
    std::cerr << " Last triangle number : " << trNum << std::endl;
    return 3;
  }
  int
    numVertices = vertexMap.size(),
    numEdges    = edgeMap.size(),
    numTriangles  = trigArray.size(),
    euler       = numVertices - numEdges + numTriangles;
  std::cout << numVertices  << " vertices " << std::endl
	    << numEdges     << " edges "    << std::endl
	    << numTriangles << " faces "    << std::endl
	    << euler        << " euler number ( = num.Vertices - num.Edges + num.Triangles)" 
	    << std::endl;
  return 0;
}
