#include <boost/program_options.hpp>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <string>
#include <sstream>
#include <iostream>
#include <set>
#include "stlfile.hpp"
#include "stl-collections.hpp"

static std::string inputFileName;
static bool findMinDist = false;

namespace po = boost::program_options;

// Statistics Accumulator:

template <typename T> struct StatAcc {

  StatAcc() : count(0), sum(0) {};

  std::vector<T> valArr;
  int count;
  T sum;
  T mean;
  T median;
  T minVal;
  T maxVal;

  void putVal (T v) {
    if (count <= 0)
      minVal = maxVal = v;
    else {
      minVal = std::min (v, minVal);
      maxVal = std::max (v, maxVal);
    }
    sum   += v;
    count += 1;
    valArr.push_back (v);
  }

  void finish () {
    mean = sum / count;
    std::sort (valArr.begin(), valArr.end());
    if (count % 2 == 0)      // Even size array
      median = 0.5 * (valArr[count/2] + valArr[count/2+1]);
    else                     // Odd  size array
      median = valArr[count/2];
  }
};

void parseOptions (int argc, char* argv[])
{
  po::options_description desc("Options");

  desc.add_options()
    ("help,h",      "print usage message")
    ("input,i",      po::value<std::string> (&inputFileName), "Input STL file")
    ("min-distance,d",      po::bool_switch (&findMinDist),
     "Report minimal distance between vertices"
     "   (quadratic complexity)"
     );
  
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

VertexMap     vertexMap;
EdgeMap       edgeMap;
TriangleArray trigArray;

static void processTrig (const Triangle & trngl) {
  for (V3 v : trngl.vertices) 
    vertexMap[v]++;
  int trigNum = storeTriangle (trigArray, trngl);
  addTriangleEdges (edgeMap, trngl, trigNum);
}

static std::tuple<float, V3, V3> getMinDistance ()
{
  float minD2 = 1e22;
  V3    p1min, p2min;

  for (auto p1 = vertexMap.begin(); p1 != vertexMap.end(); p1++) {
    V3 v1 = p1->first;
    auto p1next = p1;
    p1next++;
    for (auto p2 =p1next; p2 != vertexMap.end(); p2++) {
      V3    v2 = p2->first;
      V3    dv = v1-v2;
      float d2 = dv.p[0]*dv.p[0] +dv.p[1]*dv.p[1] +dv.p[2]*dv.p[2];
      if (d2 < minD2) {
        minD2 = d2;
        p1min = p1->first;
        p2min = p2->first;
        // std::cout << "Smaller dist " << *p1 << "  " << *p2 << "  dist2 " << d2 << std::endl;
      }
    }
  }
  return (std::tuple (sqrt (minD2), p1min, p2min));
}

float triangleArea (V3 trig[3]) {
  V3
    s01 = trig[1] - trig[0],
    s02 = trig[2] - trig[0],
    cross = s01.cross (s02);
  float rslt =  0.5 * cross.norm ();
  return rslt;
}

int main (int argc, char *argv[])
{
  int trNum = 0;
  Triangle curTrngl;
  double xMin, yMin, zMin, xMax, yMax, zMax, lsqMin, lsqMax;
  double volume = 0;
  V3 pnt0;
  StatAcc<float> areaStat;
  StatAcc<float> edgeStat;
  try {
    parseOptions (argc, argv);
    StlInFile stlf (inputFileName.c_str());

    std::cout << "STL file type : " << (stlf.isText ? "text" : "binary") << std::endl;
    std::cout << "STL header or name : " << stlf.getHeader() << std::endl;

    stlf.readTriangle (curTrngl);

    areaStat.putVal (triangleArea (curTrngl.vertices));

    pnt0 = curTrngl.vertices[0];
    processTrig (curTrngl);

    xMin = xMax = curTrngl.vertices[0].x();
    yMin = yMax = curTrngl.vertices[0].y();
    zMin = zMax = curTrngl.vertices[0].z();
    lsqMin = lsqMax = (curTrngl.vertices[0]-curTrngl.vertices[1]).sumSquares();
    for (int k=0; k<3; k++) {
      xMin = std::min<float>(xMin, curTrngl.vertices[k].x());
      yMin = std::min<float>(yMin, curTrngl.vertices[k].y());
      zMin = std::min<float>(zMin, curTrngl.vertices[k].z());
      xMax = std::max<float>(xMax, curTrngl.vertices[k].x());
      yMax = std::max<float>(yMax, curTrngl.vertices[k].y());
      zMax = std::max<float>(zMax, curTrngl.vertices[k].z());
    }
    for (trNum=1; ; trNum++) {
      if (! stlf.readTriangle (curTrngl))
        break;
      areaStat.putVal (triangleArea (curTrngl.vertices));
      processTrig (curTrngl);
      for (int k=0; k<3; k++) {
	xMin = std::min<float>(xMin, curTrngl.vertices[k].x());
	yMin = std::min<float>(yMin, curTrngl.vertices[k].y());
	zMin = std::min<float>(zMin, curTrngl.vertices[k].z());
	xMax = std::max<float>(xMax, curTrngl.vertices[k].x());
	yMax = std::max<float>(yMax, curTrngl.vertices[k].y());
	zMax = std::max<float>(zMax, curTrngl.vertices[k].z());
      }
      V3
	s01 = curTrngl.vertices[1] - curTrngl.vertices[0],
	s12 = curTrngl.vertices[2] - curTrngl.vertices[1],
	s20 = curTrngl.vertices[0] - curTrngl.vertices[2];
      double
	lsq01 = s01.sumSquares(),
	lsq12 = s12.sumSquares(),
	lsq20 = s20.sumSquares();
      lsqMin = std::min<double>(lsq01, lsqMin);
      lsqMin = std::min<double>(lsq12, lsqMin);
      lsqMin = std::min<double>(lsq20, lsqMin);
      lsqMax = std::max<double>(lsq01, lsqMax);
      lsqMax = std::max<double>(lsq12, lsqMax);
      lsqMax = std::max<double>(lsq20, lsqMax);
      //.......................................Update volume:
      M3 trigPyramid (curTrngl.vertices[0] - pnt0,
                      curTrngl.vertices[1] - pnt0,
                      curTrngl.vertices[2] - pnt0);
      volume += trigPyramid.det ();
    }
  }
  catch (std::string s) {
    std::cerr << " EXCEPTION : " << s << std::endl;
    std::cerr << " Last triangle number : " << trNum << std::endl;
    return 3;
  }
  //........................................................  Print the stats:
  for (auto edgePair : edgeMap) {
    auto edge     = edgePair.first;
    auto edgeDesc = edgePair.second;
    float edgeLen = (edge.pnt1 - edge.pnt0).norm ();
    int edgeMultiplicity = edgeDesc.trindices.size();
    for (int k=0; k<edgeMultiplicity; k++)
      edgeStat.putVal (edgeLen);
  }
  edgeStat.finish ();
  areaStat.finish ();

  //  printf (" Number of triangles : %d\n", trNum);


  int
    numVertices = vertexMap.size(),
    numEdges    = edgeMap.size(),
    numTriangles  = trigArray.size(),
    euler       = numVertices - numEdges + numTriangles;
  std::cout << numVertices  << " vertices " << std::endl
	    << numEdges     << " edges "    << std::endl
	    << numTriangles << " faces (triangles)" << std::endl
            << "Euler number is " << euler << " ( = num.Vertices - num.Edges + num.Triangles)"
            << std::endl;
  
  areaStat.putVal (triangleArea (curTrngl.vertices));
  printf (" X range:  %12.6f - %12.6f = %12.6f\n", xMax, xMin, xMax - xMin);
  printf (" Y range:  %12.6f - %12.6f = %12.6f\n", yMax, yMin, yMax - yMin);
  printf (" Z range:  %12.6f - %12.6f = %12.6f\n", zMax, zMin, zMax - zMin);
  printf (" Min edge length: %f\n", sqrt(lsqMin));
  printf (" Max edge length: %f\n", sqrt(lsqMax));

  if (findMinDist) {
    auto [minDist, p1min, p2min] = getMinDistance ();
    printf (" Minimal distance of %e is between points:\n", minDist);
    printf ("    (%.20f %.20f %.20f)  and\n", p1min[0], p1min[1], p1min[2]);
    printf ("    (%.20f %.20f %.20f)\n", p2min[0], p2min[1], p2min[2]);
  }

  printf (" Average edge length : %f\n", edgeStat.mean);
  printf (" Median  edge length : %f\n", edgeStat.median);
  printf (" Minimum edge length : %e\n", edgeStat.minVal);
  printf (" Maximum edge length : %f\n", edgeStat.maxVal);
  printf (" Total   edge length : %f\n", edgeStat.sum);

  printf (" Average facet area : %f\n", areaStat.mean);
  printf (" Median  facet area : %f\n", areaStat.median);
  printf (" Minimum facet area : %e\n", areaStat.minVal);
  printf (" Maximum facet area : %f\n", areaStat.maxVal);
  printf (" Surface area : %f\n",       areaStat.sum);
  volume /= 6;
  printf (" Volume       : %lf\n", volume);

  return 0;
}
