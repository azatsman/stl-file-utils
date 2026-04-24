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
#include "stl-collections.hpp"
 
#include <cmath>
#include <cstdio>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

static double Epsilon  = 0;

static EdgeMap   edgeMap;
static VertexMap vertexMap;

static std::string inputFileName;

template <typename T>
static  std::string type2string (T x)
{
  std::ostringstream oss;
  oss << x;
  return oss.str();
}

void parseOptions (int argc, char* argv[])
{
  po::options_description desc("Options");

  desc.add_options()
    ("help,h",      "print usage message")
    ("input,i",      po::value<std::string> (&inputFileName), "Input STL file")
    ("epsilon,e",    po::value<double> (&Epsilon), "Precision of vertex coordinates");

  
  po::basic_parsed_options<char>  parsedCmdOpts = po::parse_command_line (argc, argv, desc);
  po::variables_map varMap;
  po::store (parsedCmdOpts, varMap);
  po::notify (varMap);

  if (varMap.count("help")) {
    std::cout << desc << "\n";
    exit (0);
  }

  if (inputFileName.empty()) {
    fprintf (stderr, "%s : Must specify a valid input file name\n", argv[0]);
    exit (2);
  }
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

#if 0
static bool isRequestForHelp (const char * s) {
  if (*s == '-') {
    char ch1 = *(s+1);  // First non-dash character of 's'
    if (ch1 == '-')
      ch1 = *(s+2);
    return ((ch1 == 'h') || (ch1 == 'H'));
  }
  else
    return false;
}
#endif

int main (int argc, char *argv[])
{
  int trNum = 0;
  Triangle curTrngl;
  TriangleArray trigArray;

  try {
    parseOptions (argc, argv);
    StlInFile stlf (inputFileName.data(), Epsilon);
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
    }
  }  
  catch (std::string s) {
    int trNum  = trigArray.size();
    std::cerr << " EXCEPTION : " << s << std::endl;
    std::cerr << " Last triangle number : " << trNum << std::endl;
    return 3;
  }
  return 0;
}
