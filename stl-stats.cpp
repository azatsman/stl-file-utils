#include <boost/program_options.hpp>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <string>
#include <sstream>
#include <iostream>
#include <set>
#include "stlfile.hpp"

static std::string inputFileName;
static bool findMinDist = false;

namespace po = boost::program_options;

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

typedef std::set  <V3> PointSet;
typedef std::pair <V3,V3> Edge;

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

static PointSet pntSet;
static std::set<Edge> edgeSet;

static void processTrig (V3 trig[3]) {
  for (int k=0; k<3; k++) {
    pntSet.insert (trig[k]);
    edgeSet.insert (Edge (trig[k], trig[(k+1)%3]));
  }
}

static std::tuple<float, V3, V3> getMinDistance ()
{
  float minD2 = 1e22;
  V3    p1min, p2min;

  for (PointSet::const_iterator p1=pntSet.begin(); p1 != pntSet.end(); p1++) {
    V3 v1 = *p1;
    PointSet::const_iterator p1next = p1;
    p1next++;
    for (PointSet::const_iterator p2=p1next; p2 != pntSet.end(); p2++) {
      V3    v2 = *p2;
      V3    dv = v1-v2;
      float d2 = dv.p[0]*dv.p[0] +dv.p[1]*dv.p[1] +dv.p[2]*dv.p[2];
      if (d2 < minD2) {
        minD2 = d2;
        p1min = *p1;
        p2min = *p2;
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
#if 0
  printf ("===== Area : %e\n", rslt);
  for (int k=0; k<3; k++)
    printf ("    Vertex %d : %f %f %f\n", k, trig[k][0], trig[k][1], trig[k][2]);
#endif
  return rslt;
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
  StatAcc<float> areaStat;
  StatAcc<float> edgeStat;
  try {
    parseOptions (argc, argv);
    StlInFile stlf (inputFileName.c_str());
    std::cout << "STL file type : " << (stlf.isText ? "text" : "binary") << std::endl;

    std::cout << "STL header or name : " << stlf.getHeader() << std::endl;

    stlf.readTriangle(curTrig, curNormal);

    areaStat.putVal (triangleArea (curTrig));

    pnt0 = curTrig[0];
    processTrig (curTrig);

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
      areaStat.putVal (triangleArea (curTrig));
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
      //.......................................Update volume:
      M3 trigPyramid (curTrig[0] - pnt0, curTrig[1] - pnt0, curTrig[2] - pnt0);
      volume += trigPyramid.det ();
    }
  }
  catch (std::string s) {
    std::cerr << " EXCEPTION : " << s << std::endl;
    std::cerr << " Last triangle number : " << trNum << std::endl;
    return 3;
  }
  //........................................................  Print the stats:
  for (auto edge : edgeSet) {
    float edgeLen = (edge.second - edge.first).norm ();
    edgeStat.putVal (edgeLen);
  }
  edgeStat.finish ();
  areaStat.finish ();

  printf (" Number of triangles : %d\n", trNum);
  areaStat.putVal (triangleArea (curTrig));
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
