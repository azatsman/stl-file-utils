#include <iostream>
#include "stlfile.hpp"

#define VERTEX_FORMAT "        vertex %9.3f %9.3f %9.3f\n"

int main (int argc, char *argv[])
{
  if (argc != 2) {
    std::cerr << "Usage : " << argv[0] << " <input-bin-file>" << std::endl;
    std::cerr << "   (Note : the output is sent to the standard output" << std::endl;
    return 3;
  }
  if (std::string (argv[1]) == std::string ("-h")) {
    std::cout << "Usage : " << argv[0] << " <input-bin-file>" << std::endl;
    std::cout << "   (Note : the output is sent to the standard output" << std::endl;
    return 0;
  }
  
  int trNum = 0;
  V3 curTrig[3];
  V3 curNormal;
  int numDclTrngl = -1;  // Set insidet 'try'
  try {
    StlInBinFile stlf (argv[1]);
    numDclTrngl = stlf.numTriangles();
    printf ("solid Converted-from-binary-%s\n", argv[1]);
    for (trNum=0; trNum<numDclTrngl; trNum++) {
      stlf.readTriangle(curTrig, curNormal);
      printf ("facet normal %9.3f  %9.3f  %9.3f\n",
	      curNormal.x(), curNormal.y(), curNormal.z());
      printf   ("    outer loop\n");
      for (int v=0; v<3; v++)
	printf (VERTEX_FORMAT, curTrig[v].x(), curTrig[v].y(), curTrig[v].z());
      printf ("    endloop\n");
      printf ("endfacet\n");
    }
    printf ("endsolid\n");
  }
  catch (std::string s) {
    std::cerr << " EXCEPTION : " << s << std::endl;
    std::cerr << " Last triangle number : " << trNum << " out of total of "
              << numDclTrngl << " triangles" << std::endl;
    return 3;
  }
  return 0;
}
