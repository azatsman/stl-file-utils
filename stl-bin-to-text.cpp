#include <iostream>
#include "stlfile.hpp"

#define VERTEX_FORMAT "   vertex    %12.6f %12.6f %12.6f\n"
#define NORMAL_FORMAT "facet normal %12.6f %12.6f %12.6f\n"

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
  Triangle curTrngl;
  int numDclTrngl = -1;  // Set insidet 'try'
  try {
    StlInBinFile stlf (argv[1]);
    numDclTrngl = stlf.numTriangles();
    printf ("solid Converted-from-binary-%s\n", argv[1]);
    for (trNum=0; trNum<numDclTrngl; trNum++) {
      stlf.readTriangle (curTrngl);
      printf (NORMAL_FORMAT, curTrngl.normal.x(), curTrngl.normal.y(), curTrngl.normal.z());
      printf   ("   outer loop\n");
      for (int v=0; v<3; v++)
	printf (VERTEX_FORMAT, curTrngl.vertices[v].x(), curTrngl.vertices[v].y(),
                curTrngl.vertices[v].z());
      printf ("   endloop\n");
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
