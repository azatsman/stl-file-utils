#ifndef INCLUDED_stlfile_h_9946361
#define INCLUDED_stlfile_h_9946361

// NOTE : the triangles' vertices must be listed COUNTER-clock-wise when looking from the OUTSIDE.

// TODO : should not require numTriangles in output file constructors -- instead we can count them
// in 'writeTriangle' methods.
// TODO: add types for triangles. With or without normals?

#include <string>
#include "vec3.hpp"
#include <fstream>
#include <vector>

typedef Vec3<float> V3;
typedef Mat3<float> M3;

struct Triangle {
  V3 vertices[3];
  V3 normal;
  static std::vector<Triangle> TriangleArray;
};

//.................................................................. ABSTRACT base input file:

class StlInBaseFile {
public:
  virtual ~StlInBaseFile () {};

  std::string header;  // Wikipedia calls it "name" for text files and "header" for binaries.

  //  bool needRounding;

  // When Epsilon > 0 each coordinate of the triangles' vertices is round to the nearest
  // multiple of Epsilon.
  // When Epsilon == 0 the coordinates are left undisturbed.
  
  double Epsilon;

  // Try reading one triangle, return 'true' if successful:

  virtual bool readTriangle (Triangle& trngl) = 0;

  virtual std::string getHeader () {return header;}

protected:

  int trigNum_;    // the number of triangles read.

  // Conditionally round the triangle's coordinates to the nearest multiple of Epsilon:
  void roundTriangle (V3 trig[3]);
};

//.................................................................. Binary file

class StlInBinFile : public StlInBaseFile {
public:
  StlInBinFile (FILE * f, float epsilon=0);
  StlInBinFile (const char* fileName, float epsilon=0);
  virtual ~StlInBinFile ();
  virtual bool readTriangle (Triangle& trngl);
  int numTriangles() const;
private:
  FILE* fl_;
  void init (FILE * f, float epsilon);
  int numTriangles_;
};

class StlInTextFile : public StlInBaseFile {
public:
  StlInTextFile (const char* fileName, float epsilon=0);
  virtual ~StlInTextFile ();
  virtual bool readTriangle (Triangle& trngl);
  int numTriangles();
private:
  bool readLine_ (std::string& line);
  std::ifstream fl_;
  int           numTriangles_;
  std::string fileName_;
  int lineNumber_;
};

struct StlOutBinFile {
  FILE* fl_;
  int numTriangles_;
  StlOutBinFile (const char* fileName, const char* header);
  StlOutBinFile (const char* fileName, int numTriangles, const char* header);
  ~StlOutBinFile ();
  void writeTriangle (const Triangle& trngl);
};

//---------------------------------------------------------------- Text : 

// Input files only for now. Not much use for output type, except in stl-bin-to-text, but that one
// simply dumps the text to the standard output.

class ParseError : public std::runtime_error {
public:
  std::string theLine;
  int         lineNumber;
  ParseError (std::string line, std::string msg, int lineNum) : std::runtime_error (msg),
                                                                theLine (line),
                                                                lineNumber (lineNum) {};
};

//------------------------------------------------------------------ "Generic" STL file

class StlInFile : virtual StlInBaseFile {

public:

  bool isText;
  StlInFile  (const char * fileName, float epsilon=0);
  virtual ~StlInFile ();
  virtual bool readTriangle (Triangle& trngl);

  virtual std::string getHeader () {return actualStlFile->getHeader ();}
  
private:
  StlInBaseFile * actualStlFile;
};

#endif /*INCLUDED_stlfile_h_9946361*/
