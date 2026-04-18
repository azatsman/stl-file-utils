// TODO
//    check normals: orthogonality to the triangles and correct orientations.
//
//    total area
//    XYZ ranges
//     moments and gravity center.

static double Epsilon  = 0.0001;
static double MaxValue = 1e9;
static int    verbosity = 1;

#define DO_EDGES 1

#include <boost/program_options.hpp>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <string>
#include <sstream>
#include <iostream>
#include <set>

static std::string inputFileName;

namespace po = boost::program_options;

#include "stlfile.hpp"

typedef Vec3<float>     V3;
typedef Mat3<float>     M3;

struct AngleLogger {
  FILE* fd;
  AngleLogger() : fd(NULL) {};
  ~AngleLogger()  {
    if (fd != NULL)
      fclose (fd);
  };
  void open (std::string fileName) {fd = fopen (fileName.c_str(), "wt");}
  void operator() (V3 nv) {  // 'nv' is supposed to be a normalized (length=1) 3d vector);
    if (fd != NULL) {
      double
	angRad = asin (nv.p[2]),
	angDeg = (180/M_PI) *  angRad;
      fprintf (fd, "%7.3lf\n", angDeg);
    }
  }
};

static AngleLogger angleLogger;

template <typename T>
static  std::string type2string (T x)
{
  std::ostringstream oss;
  oss << x;
  return oss.str();
}

static void checkTrig (V3 trig[3], V3 normal)
{
  //............................................ Check the vertices: 
  for (int v=0; v<3; v++)
    for (int j=0; j<3; j++) {
      if (std::isnan(trig[v].p[j])) 
	throw(std::string("NaN in a triangle"));
      else if (std::isinf(trig[v].p[j]))
	throw(std::string("Infinity in a triangle"));
      if (! (fabs(trig[v].p[j]) < MaxValue)) {
	throw(std::string("Vertex coordinate too large :  = ") +
	      type2string(trig[v].p[j]));
      }
    }
  //............................................ Check the normal: 
  for (int j=0; j<3; j++) {
    if (std::isnan(normal.p[j]))
      throw(std::string("NaN in a normal"));
    else if (std::isinf(normal.p[j]))
      throw(std::string("Infinity in a normal"));
    if (! (fabs(normal.p[j]) < MaxValue)) 
      throw(std::string("Normal's coordinate too large :  = ") +
	    type2string(normal.p[j]));
  }
  //................................................ Check orthonormality:
  if (false) {
    if (fabs(normal * (trig[1] - trig[0])) > Epsilon)
      throw(std::string("Normal is not orthogonal to its triangle"));
    if (fabs(normal * (trig[2] - trig[0])) > Epsilon)
      throw(std::string("Normal is not orthogonal to its triangle"));
    float normNorm2 = normal.sumSquares();
    if (fabs (1.0 - normNorm2) > Epsilon) {
      throw(std::string("Normal's length is not 1. Squares length = ") +
            type2string(normNorm2));
    }
  }
  //................................................ Check orientation:

  if (false && M3(trig[1]-trig[0], trig[2]-trig[0], normal).det() <= 0)
    throw(std::string("Bad facet orientation"));
}

void parseOptions (int argc, char* argv[])
{
  po::options_description desc("Options");

  std::string angleLogFile;

  std::ostringstream epsString;

  epsString << Epsilon;
  
  desc.add_options()
    ("help,h",      "print usage message")
    ("input,i",      po::value<std::string> (&inputFileName), "Input STL file")
    ("angle-log,a",  po::value<std::string> (&angleLogFile),  "Angle log file")
    ("epsilon,e",    po::value(&Epsilon)->default_value (Epsilon), "Precision")
    ("max-value,m",  po::value(&MaxValue)->default_value (MaxValue), "Max. legal coordinate value");
  po::basic_parsed_options<char>  parsedCmdOpts = po::parse_command_line (argc, argv, desc);
  po::variables_map varMap;
  po::store(parsedCmdOpts, varMap);
  po::notify(varMap);

  if (varMap.count("help")) {
    std::cout << desc << "\n";
    exit (0);
  }
  
  po::variables_map::iterator p  = varMap.find ("angle-log");
  if (p != varMap.end()) {
    angleLogger.open (angleLogFile);
    if (verbosity >= 1)
      printf ("Logging angles to \"%s\"\n", angleLogFile.c_str());
  }
  if  (true) {
    if (angleLogFile.empty())
      printf ("No angle log is specified\n");
    else
      printf ("Angle log : \"%s\"\n", angleLogFile.c_str());
    printf ("Epsilon = %e\n", Epsilon);
    printf ("MaxValue = %e\n", MaxValue);
  }

  if (inputFileName.empty()) {
    fprintf (stderr, "%s : Must specify a valid input file name\n", argv[0]);
    exit (2);
  }
}

typedef std::set  <V3> PointSet;
typedef std::pair <V3,V3> Edge;

// static Edge reverse (const Edge& e) {return Edge (e.second, e.first);}

bool lessFunc (const V3& v1, const V3& v2)  {
  for (int k=0; k<3; k++) {
    if (v1.p[k] < v2.p[k])
      return true;
    if (v1.p[k] > v2.p[k])
      return false;
  }
  return false;
}

template <>
struct std::less<V3> {
  bool operator () (const V3& v1, const V3& v2) const {
    return lessFunc (v1, v2);
  }
};

#if DO_EDGES
template <>
struct std::less<Edge> {
  bool lessFunc2 (const Edge& e1, const Edge& e2) const {
    //    if (e1.first < e2.first)
    //    if (std::less<V3>::lessFunc (e1.first, e2.first))
    //      return true
    if (lessFunc (e1.first,  e2.first))
      return true;
    if (lessFunc (e2.first,  e1.first))
      return false;
    if (lessFunc (e1.second, e2.second))
      return true;
    return false;
  }
  bool operator () (const Edge& e1, const Edge& e2) const {
    return lessFunc2 (e1, e2);
  }
};
#endif


static PointSet pntSet;
static std::set<Edge> edgeSet;

static void processTrig (V3 trig[3]) {
  for (int k=0; k<3; k++) {
    pntSet.insert (trig[k]);
#if DO_EDGES
    edgeSet.insert (Edge (trig[k], trig[(k+1)%3]));
#endif
  }
}


static bool pointLexComp (const V3& x, const V3& y) {
  if (x.p[0] < y.p[0])
    return true;
  else if (x.p[0] > y.p[0])
    return false;
  else if (x.p[0] == y.p[0]) {
    if (x.p[1] < y.p[1])
      return true;
    else if (x.p[1] > y.p[1])
      return false;
    else if (x.p[1] == y.p[1]) 
      return (x.p[2] < y.p[2]);
  }
  return false;
}

static void dumpSmallDistances ()
{
  float minD2 = 1e22;

  float eps2 = Epsilon * Epsilon;

  for (PointSet::const_iterator p1=pntSet.begin(); p1 != pntSet.end(); p1++) {
    V3 v1 = *p1;
    PointSet::const_iterator p1next = p1;
    p1next++;
    for (PointSet::const_iterator p2=p1next; p2 != pntSet.end(); p2++) {
      V3 v2 = *p2;
      V3  dv = v1-v2;
      float d2 = dv.p[0]*dv.p[0] +dv.p[1]*dv.p[1] +dv.p[2]*dv.p[2];
      if (d2 < minD2) {
        minD2 = d2;
        // std::cout << "Smaller dist " << *p1 << "  " << *p2 << "  dist2 " << d2 << std::endl;
      }
      if (d2 < eps2) {
        std::cout << "Small  dist " << *p1 << "  " << *p2 << "  dist " << sqrt(d2) << std::endl;
      }
    }
  }
}

int main (int argc, char *argv[])
{
  int trNum = 0;
  //  double totalArea = 0;
  V3 curTrig[3];
  V3 curNormal;
  double xMin, yMin, zMin, xMax, yMax, zMax, lsqMin, lsqMax;
  double volume = 0;
  V3 pnt0;
  try {
    parseOptions (argc, argv);
    StlInFile stlf (inputFileName.c_str());

    //    int numDclTrngl = stlf.numTriangles();
    //   printf ("Declared %d triangles\n", numDclTrngl);
    stlf.readTriangle(curTrig, curNormal);
    pnt0 = curTrig[0];
    processTrig (curTrig);
    checkTrig(curTrig, curNormal);
    xMin = xMax = curTrig[0].x();
    yMin = yMax = curTrig[0].y();
    zMin = zMax = curTrig[0].z();
    lsqMin = lsqMax = (curTrig[0]-curTrig[1]).sumSquares();
    for (int k=0; k<3; k++) {
      xMin = std::min<float>(xMin, curTrig[k].x());
      yMin = std::min<float>(yMin, curTrig[k].y());
      zMin = std::min<float>(zMin, curTrig[k].z());
      xMax = std::max<float>(xMax, curTrig[k].x());
      yMax = std::max<float>(yMax, curTrig[k].y());
      zMax = std::max<float>(zMax, curTrig[k].z());
    }
    for (trNum=1; ; trNum++) {
      if (! stlf.readTriangle(curTrig, curNormal))
        break;
      processTrig (curTrig);
      for (int k=0; k<3; k++) {
	xMin = std::min<float>(xMin, curTrig[k].x());
	yMin = std::min<float>(yMin, curTrig[k].y());
	zMin = std::min<float>(zMin, curTrig[k].z());
	xMax = std::max<float>(xMax, curTrig[k].x());
	yMax = std::max<float>(yMax, curTrig[k].y());
	zMax = std::max<float>(zMax, curTrig[k].z());
      }
      V3
	s01 = curTrig[1] - curTrig[0],
	s12 = curTrig[2] - curTrig[1],
	s20 = curTrig[0] - curTrig[2];
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

      // Update volume:
      M3 trigPyramid (curTrig[0] - pnt0, curTrig[1] - pnt0, curTrig[2] - pnt0);
      volume += trigPyramid.det ();
    }
  }
  catch (std::string s) {
    std::cerr << " EXCEPTION : " << s << std::endl;
    std::cerr << " Last triangle number : " << trNum << std::endl;
    return 3;
  }
  // ........................  Print the stats:
  printf (" X range:  %12.6f - %12.6f = %12.6f\n", xMax, xMin, xMax - xMin);
  printf (" Y range:  %12.6f - %12.6f = %12.6f\n", yMax, yMin, yMax - yMin);
  printf (" Z range:  %12.6f - %12.6f = %12.6f\n", zMax, zMin, zMax - zMin);
  
  printf (" Min edge length: %f Max edge length: %f\n",
	  sqrt(lsqMin), sqrt(lsqMax));

  dumpSmallDistances();

  volume /= 6;

  printf (" Volume : %lf\n", volume);
  
  return 0;
}
