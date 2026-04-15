#include "stlfile.hpp"
#include <unistd.h>
#include <stdio.h>
#include <getopt.h>
#include <string>
#include <iostream>

struct Param {
  const char* inputName;
  const char* outputName;
  float scales[3];
  Param() {
    inputName  = 0;
    outputName = 0;
    scales[0] = scales[1] = scales[2] = 1.0f;
  };
  void dump () const {
    printf ("Input : %s\n", inputName);
    printf ("Output: %s\n", outputName);
    printf ("Scales: %f, %f, %f\n", scales[0], scales[1], scales[2]);
  };
};

static void readParams (int argc, char* argv[], Param& prm);
static void scaleSTL  (const Param& prm);

void usage (char* pnm)
{
  printf("Usage: %s -i <input> -o <output> -x <xScale> -y <yScale> -z <zScale>\n",
	 pnm);
}


int main (int argc, char *argv[])
{
  Param prm;
  try {
    if (argc != 11) {
      usage(argv[0]);
      return 2;
    }
    readParams(argc, argv, prm);
    prm.dump();
    scaleSTL (prm);
  }
  catch (const char* msg) {printf ("EXCEPTION: %s\n", msg);}
  catch (std::string msg) {std::cerr << "EXCEPTION2: " << msg << std::endl;}
  return 0;
}

static void readParams (int argc, char* argv[], Param& prm)
{
  const char* opstr = "o:i:x:y:z:";
  int ch;
  opterr = 0;
  while ((ch = getopt(argc, argv, opstr)) != -1) {
    switch (ch) {
      case 'i':
	prm.inputName = optarg;
	break;
      case 'o':
	prm.outputName = optarg;
	break;
      case 'x':
	if (sscanf(optarg, "%f", &prm.scales[0]) != 1)
	  throw ("'x' scale is not a number\n");
	break;
      case 'y':
	if (sscanf(optarg, "%f", &prm.scales[1]) != 1)
	  throw ("'x' scale is not a number\n");
	break;
      case 'z':
	if (sscanf(optarg, "%f", &prm.scales[2]) != 1)
	  throw ("'z' scale is not a number\n");
	break;
      default:
	std::string optStr(1, static_cast<char>(optopt));
	throw (std::string("Unknown option '") + optStr + std::string(";"));
	break;
    }
  }
}

static void scaleSTL  (const Param& prm)
{
  StlInBinFile  inf (prm.inputName);
  int numDclTrngl = inf.numTriangles();
  char buf[80];
  sprintf (buf, "Scaled by %.3f %.3f %.3f", prm.scales[0],prm.scales[1],prm.scales[2]);
  StlOutBinFile onf (prm.outputName, buf);

  M3 scMat(prm.scales);
  for (int trNum=0; trNum<numDclTrngl; trNum++) {
    V3 inTrig[3], outTrig[3], curNormal;
    inf.readTriangle(inTrig, curNormal);
    for (int j=0; j<3; j++)
      outTrig[j] = scMat * inTrig[j];
    onf.writeTriangle(outTrig, curNormal);
  }
}
