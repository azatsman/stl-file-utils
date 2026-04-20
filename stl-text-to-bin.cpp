// TODO
//    check normals: orthogonality to the triangles and correct orientations.
//
//    total area
//    XYZ ranges
//     moments and gravity center.

#include <cstdio>
#include <cmath>
#include <string>
#include <sstream>
#include <iostream>
#include "stlfile.hpp"

int main (int argc, char *argv[])
{
  int trNum = 0;
  V3 curTrig[3];
  V3 curNormal;
  try {
    StlInTextFile      fIn  (argv[1]);
    // int numDclTrngl = fIn.numTriangles();
    std::string title = std::string ("Converted to binary by ") + std::string (argv[0]) +
      std::string (" from ") + std::string (argv[1]);
    StlOutBinFile  fOut (argv[2], title.c_str());
    while (fIn .readTriangle  (curTrig, curNormal))
      fOut.writeTriangle (curTrig, curNormal);
  }
  catch (std::string s) {
    std::cerr << " EXCEPTION : " << s << std::endl;
    std::cerr << " Last triangle number : " << trNum << std::endl;
    return 3;
  }
  catch (const ParseError& pe) {
    std::cerr << " PARSE ERROR at line   \"" << pe.theLine << "\" : " << pe.what ();
    std::cerr << ". Last triangle number : " << trNum << std::endl;
    std::cerr << ". Line number : " << pe.lineNumber << std::endl;
    return 3;
  }
  return 0;
}
