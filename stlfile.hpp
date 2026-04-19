#ifndef INCLUDED_stlfile_h_9946361
#define INCLUDED_stlfile_h_9946361

// NOTE : the triangles' vertices must be listed COUNTER-clock-wise when looking from the OUTSIDE.

// TODO : should not require numTriangles in output file constructors -- instead we can count them
// in 'writeTriangle' methods.
// TODO: add types for triangles. With or without normals?

#include <string>
#include "vec3.hpp"
#include <fstream>

typedef Vec3<float> V3;
typedef Mat3<float> M3;

struct Triangle {
  V3 vertices[3];
  V3 normal;
};

//.................................................................. ABSTRACT base input file:

class StlInBaseFile {
public:
  virtual ~StlInBaseFile () {};

  std::string header;  // Wikipedia calls it "name" for text files and "header" for binaries.

  // Try reading one triangle, return 'true' if successful:
  
  virtual bool readTriangle (V3 trig[3], V3& normal) = 0;
  virtual bool readTriangle (Triangle& trngl) = 0;
protected:
  int trigNum_;    // the number of triangles read.
};

// Returns a pointer to a text or a binary STL file depending on the
//  first bytes of the file:

StlInBaseFile* openStlFile (const char* fileName);

//.................................................................. Binary file

class StlInBinFile : public StlInBaseFile {
public:
  StlInBinFile (FILE * f);
  StlInBinFile (const char* fileName);
  virtual ~StlInBinFile ();
  virtual bool readTriangle (V3 trig[3], V3& normal) ;
  virtual bool readTriangle (Triangle& trngl);
  int numTriangles() const;
private:
  FILE* fl_;
  void init (FILE * f);
  char header_[80];
  int numTriangles_;
};

class StlInTextFile : public StlInBaseFile {
public:
  StlInTextFile (const char* fileName);
  virtual ~StlInTextFile ();
  virtual bool readTriangle (V3 trig[3], V3& normal) ;
  virtual bool readTriangle (Triangle& trngl);// {return readTriangle (trngl.vertices, trngl.normal);
  int numTriangles();
private:
  bool readLine_ (std::string& line);
  std::ifstream fl_;
  char          header_[80];
  int           numTriangles_;
  std::string fileName_;
  int lineNumber_;
};

struct StlOutBinFile {
  FILE* fl_;
  char header_[80];
  int numTriangles_;

  StlOutBinFile (const char* fileName, const char* header);
  StlOutBinFile (const char* fileName, int numTriangles, const char* header);

  // StlOutBinFile (const char* fileName, int numTriangles, const char* header);

  ~StlOutBinFile ();
  void writeTriangle (const V3 trig[3], const V3& normal);
  void writeTriangle (const Triangle& trngl) {writeTriangle (trngl.vertices, trngl.normal);};
};

//---------------------------------------------------------------- Text : 

class ParseError : public std::runtime_error {
public:
  std::string theLine;
  int         lineNumber;
  ParseError (std::string line, std::string msg, int lineNum) : std::runtime_error (msg),
                                                                theLine (line),
                                                                lineNumber (lineNum) {};
};

struct StlOutTextFile {
  FILE* fl_;
  char header_[80];
  int numTriangles_;

  StlOutTextFile (const char* fileName,
		 int numTriangles,
		 const char* header);
  ~StlOutTextFile ();
  void writeTriangle(const V3 trig[3], const V3& normal) ;
};

//------------------------------------------------------------------ "Generic" STL file

class StlInFile : virtual StlInBaseFile {

public:

  bool isText;
  StlInFile  (const char * fileName);
  virtual ~StlInFile ();

  virtual bool readTriangle (V3 trig[3], V3& normal);
  virtual bool readTriangle (Triangle& trngl);

  std::string getHeader () {return actualStlFile->header;}
  

private:
  StlInBaseFile * actualStlFile;
};

#endif /*INCLUDED_stlfile_h_9946361*/
