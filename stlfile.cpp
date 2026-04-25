#include "stlfile.hpp"
#include <cstdio>
#include <sstream>
#include <algorithm>
#include <bit>

void StlInBaseFile::roundTriangle (V3 trig[3]) {
  if (Epsilon > 0) {
    for (int j=0; j<3; j++) {
      V3& vrtx = trig[j];
      for (int k=0; k<3; k++) 
        vrtx.p[k] = Epsilon * round (static_cast<double> (vrtx.p[k]) / Epsilon);
    }
  }
}

void StlInBinFile::init (FILE * f, float epsilon)
{
  fl_ = f;

  switch (std::endian::native) {
    case std::endian::big:
      throw ("Binary STL files can only be read on little-endian computers");
    case std::endian::little:
      break;
    default:
      std::cerr << "WARNING : Cannot confirm little-endiannes on this computer" << std::endl;
  }

  Epsilon = epsilon;

  if (fread (header_, sizeof(header_), 1, fl_) != 1) 
    throw (std::string("Failed to read the file header "));

  header = header_;

  if (fread (&numTriangles_, sizeof(numTriangles_), 1, fl_) != 1) 
    throw (std::string("Failed to read the number of tirangles"));
  trigNum_ = 0;
};

StlInBinFile::StlInBinFile (FILE * f, float epsilon) {
  init (f, epsilon);
}

StlInBinFile::StlInBinFile (const char* fileName, float epsilon)
{
  fl_ = fopen (fileName, "rb");
  if (fl_ == NULL)
    throw (std::string("Cannot open binary STL file ") + 
	   std::string(fileName) + std::string (" for reading"));
  init (fl_, epsilon);
};

StlInBinFile::~StlInBinFile ()
{
  if (fl_ != NULL)
    fclose(fl_);
};

#if 0
bool StlInBinFile::readTriangle(V3 trig[3], V3& normal)
{
  float buf[12];
  unsigned int  n16;

  if (trigNum_ >= numTriangles_)
    return false;

  if (fread(buf, 4, 12, fl_) != 12)
    return false;
  if (fread(&n16, 2, 1, fl_) != 1)
    throw(std::string("Failed to read the \"Attribute Byte Count\" field"));
  float* bp = buf;
  for   (int j=0; j<3; j++)
    normal.p[j] = *bp++;
  for   (int v=0; v<3; v++) 
    for (int j=0; j<3; j++)
      trig[v].p[j] = *bp++;

  roundTriangle (trig);

  trigNum_ ++;
  return true;
}
#endif


bool StlInBinFile::readTriangle (Triangle & trngl)
{
  float buf[12];
  unsigned short  n16;

  if (trigNum_ >= numTriangles_)
    return false;

  if (fread(buf, 4, 12, fl_) != 12)
    return false;
  if (fread(&n16, 2, 1, fl_) != 1)
    throw(std::string("Failed to read the \"Attribute Byte Count\" field"));
  float* bp = buf;
  for   (int j=0; j<3; j++)
    trngl.normal.p[j] = *bp++;
  for   (int v=0; v<3; v++) 
    for (int j=0; j<3; j++)
      trngl.vertices[v].p[j] = *bp++;

  roundTriangle (trngl.vertices);

  trigNum_ ++;
  return true;
}

int StlInBinFile::numTriangles() const
{
  return numTriangles_;
}

StlOutBinFile::StlOutBinFile (const char* fileName, const char* header)
  : fl_(NULL), numTriangles_(0)
{
  fl_ = fopen (fileName, "wb");
  if (fl_ == NULL)
    throw (std::string("Cannot open binary STL file ") + 
	   std::string(fileName) + std::string (" for reading"));
  header_[sizeof(header_)-2] = '\n';
  header_[sizeof(header_)-1] = 0;
  strncpy(header_, header, sizeof(header_) - 2);
  if (fwrite (header_, sizeof(header_), 1, fl_) != 1) 
    throw (std::string("Failed to write the file header "));
  if (fwrite (&numTriangles_, sizeof(numTriangles_), 1, fl_) != 1) 
    throw (std::string("Failed to write the number of tirangles"));
}

StlOutBinFile::~StlOutBinFile ()
{
  if (fl_ != NULL) {
    // Write the number of triangles and close the file.
    fseek  (fl_, sizeof (header_), SEEK_SET);
    fwrite (&numTriangles_, sizeof(numTriangles_), 1, fl_);
    fclose (fl_);
  }
}

void StlOutBinFile::writeTriangle (const Triangle & trngl)
{
  float buf[12];
  unsigned int  n16 = 0;
  float* bp = buf;
  for   (int j=0; j<3; j++)
    *bp++ = trngl.normal.p[j];

  for   (int v=0; v<3; v++) 
    for (int j=0; j<3; j++)
      *bp++ = trngl.vertices[v].p[j];
  if (fwrite(buf, 4, 12, fl_) != 12)
    throw(std::string("Failed to write a triangle"));
  if (fwrite(&n16, 2, 1, fl_) != 1)
    throw(std::string("Failed to write an \"Attribute Byte Count\""));
  numTriangles_++;
}

//============================================================================= Text Version

StlInTextFile::StlInTextFile (const char* fileName, float epsilon) :
  fl_           (fileName),
  numTriangles_ (0),
  fileName_     (fileName),
  lineNumber_   (0)
{
  trigNum_ = 0;
  Epsilon = epsilon;
  std::string line, tok1, tok2;
  if (! readLine_ (line))
    throw (ParseError (line, "Failed to read the first line of text STL file", lineNumber_));
  std::istringstream lineStream (line);
  lineStream >> tok1;
  std::transform (tok1.begin(), tok1.end(), tok1.begin(), [](unsigned char c){
    return std::tolower(c); });
  if (tok1 != std::string ("solid"))
    throw (ParseError (line, "Bad first line ", lineNumber_));
  lineStream >> header;
};

StlInTextFile::~StlInTextFile () {};

bool StlInTextFile::readLine_ (std::string& line) {
  bool rslt = ! (! std::getline (fl_, line));
  lineNumber_++;
  return rslt;
}

bool StlInTextFile::readTriangle(Triangle & trngl) 
{
  std::istringstream lineStream;
  std::string line, tok1, tok2;
  //........................................................ Read the start of a facet:
  if (! readLine_ (line))
    throw (ParseError (line, "Expected \"facet\" or \"endsolid\", got empty line", lineNumber_));
  else {
    std::istringstream lineStream (line);

    lineStream >> tok1;
    std::transform (tok1.begin(), tok1.end(), tok1.begin(), [](unsigned char c){
      return std::tolower(c); });

    if (tok1 != std::string ("facet")) {
      if (tok1 == std::string ("endsolid"))
	return false;
      else
	throw (ParseError (line, "Expected \"facet\" or \"endsolid\"", lineNumber_));
    }
    lineStream >> tok1;
    std::transform (tok1.begin(), tok1.end(), tok1.begin(), [](unsigned char c){
      return std::tolower(c); });
    if (tok1 != std::string ("normal"))
      throw (ParseError (line, "Missing normal in the first line of facet", lineNumber_));
    lineStream >> trngl.normal.p[0] >> trngl.normal.p[1] >> trngl.normal.p[2];
  }
  //................................................................... Read the "outer loop":
  if (! readLine_ (line))
    throw (ParseError (line, "Failed to read a line with \"outer loop\" of a facet", lineNumber_));
  else {
    std::istringstream lineStream (line);
    lineStream >> tok1 >> tok2;
    std::transform (tok1.begin(), tok1.end(), tok1.begin(), [](unsigned char c){
      return std::tolower(c); });
    std::transform (tok2.begin(), tok2.end(), tok2.begin(), [](unsigned char c){
      return std::tolower(c); });

    if ((tok1 != std::string ("outer")) ||
	(tok2 != std::string ("loop")))
      throw (ParseError (line, "Failed to read \"outer loop\" in the line", lineNumber_));
  }
  for (int k=0; k<3; k++) {
    if (! readLine_ (line))
      throw (ParseError (line, "Failed to read a vertex line of a facet", lineNumber_));
    else {
      std::istringstream lineStream (line);
      lineStream >> tok1;

    std::transform (tok1.begin(), tok1.end(), tok1.begin(), [](unsigned char c){
      return std::tolower(c); });
      
      if (tok1 != std::string ("vertex"))
	throw (ParseError (line, "Expected \"vertex\" in the line", lineNumber_));
      lineStream >> trngl.vertices[k].p[0] >> trngl.vertices[k].p[1] >> trngl.vertices[k].p[2];
    }
  }  
  if (! readLine_ (line))
    throw (ParseError (line, "Failed to read a line with \"endloop\" of a facet", lineNumber_));
  else {
    std::istringstream lineStream (line);
    lineStream >> tok1;
    std::transform (tok1.begin(), tok1.end(), tok1.begin(), [](unsigned char c){
      return std::tolower(c); });
    if (tok1 != std::string ("endloop"))
      throw (ParseError (line, "Failed to read \"endloop\" in the line", lineNumber_));
  }
  if (! readLine_ (line))
    throw (ParseError (line, "Failed to read a line with \"endfacet\" of a facet", lineNumber_));
  else {
    std::istringstream lineStream (line);
    lineStream >> tok1;
    std::transform (tok1.begin(), tok1.end(), tok1.begin(), [](unsigned char c){
      return std::tolower(c); });
    if (tok1 != std::string ("endfacet"))
      throw (ParseError (line, "Failed to read \"endloop\" in the line", lineNumber_));
  }

  roundTriangle (trngl.vertices);

  trigNum_ ++;
  return true;
}

int StlInTextFile::numTriangles()
{
  if (numTriangles_ < 0) {
    StlInTextFile  tmp (this->fileName_.c_str());
    numTriangles_ = 0;
    Triangle ___;
    while (tmp.readTriangle (___))
      numTriangles_++;
  }
  return numTriangles_;
}

//----------------------------------------------- "Generic" StlInFile:

typedef std::string::size_type Stype;

static Stype const MaxLineLength = 1000*1000;

static bool finishLine (FILE * f) {
  // Return true if found the end of line, otherwise return false.
  for (Stype j=0; j<MaxLineLength; j++) {
    int ch = fgetc (f);
    if (ch == '\n')
      return true;
    if (ch == EOF)
      return false;
  }
  return false;
}

static std::string  readToken (FILE * f, Stype maxLen) {
  std::string rslt;
  for (Stype j=0; j<MaxLineLength; j++) {
    int ch = fgetc (f);
    if (! isblank (ch)) {
      rslt += tolower (ch);
      break;
    }
  }
  while (rslt.size() < maxLen) {
    char ch = tolower (fgetc (f));
    if (isblank (ch))
        return (rslt);
    else
      rslt += ch;
  }
  return rslt;
}

static bool isStlText (FILE * f) {
  static const std::string SolidToken    ("solid");
  static const std::string EndSolidToken ("endsolid");
  static const std::string FacetToken    ("facet");
  bool rslt = false;
  std::string token1 = readToken (f, SolidToken.size());
  bool isSolid = token1 == SolidToken;
  if (isSolid) {
    if (finishLine (f)) {
      std::string token2 = readToken (f,  std::max (FacetToken.size(), EndSolidToken.size()));
      bool
        isFacet    = token2 == FacetToken,
        isEndSolid = token2 == EndSolidToken;
      rslt = isFacet || isEndSolid;
    }
  }
  fseek (f, 0, SEEK_SET);
  return rslt;
}

StlInFile::StlInFile  (const char * fileName, float epsilon) {
  FILE *fl_ = fopen (fileName, "rb");
  if (fl_ == NULL)
    throw (std::string("Cannot open STL file ") + 
	   std::string(fileName) + std::string (" for reading"));
  if (isStlText (fl_)) {
    // Cannot reuse 'fl_' because text STL class uses std::ifstream, not 'FILE*':
    fclose (fl_);
    isText = true;
    actualStlFile = static_cast<StlInBaseFile*> (new StlInTextFile (fileName, epsilon));
  }
  else {
    isText = false;
    actualStlFile = static_cast<StlInBaseFile*> (new StlInBinFile  (fl_, epsilon));
  }
}

StlInFile::~StlInFile () {
  if (actualStlFile)
    delete (actualStlFile);
}

bool StlInFile::readTriangle (Triangle& trngl) {
  return actualStlFile->readTriangle (trngl);
}
