#include "stlfile.hpp"
#include <cstdio>
#include <sstream>
#include <algorithm>


StlInBaseFile* openStlFile (const char* fileName)
{
  char buf[8];
  StlInBaseFile* rslt = 0;
  const char* textMarker = "solid";
  FILE *fl_ = fopen (fileName, "rb");

  if (fl_ == NULL)
    throw (std::string("Cannot open STL file ") + 
	   std::string(fileName) + std::string (" for reading"));
  if (fread (buf, sizeof(buf), 1, fl_) != 1) 
    throw (std::string("Failed to read the file header "));
  fclose (fl_);
  if (memcmp (buf, textMarker, strlen (textMarker)) == 0)
    rslt = new StlInTextFile (fileName);
  else
    rslt = new StlInBinFile  (fileName);
  return rslt;
}

void StlInBinFile::init (FILE * f)
{
  fl_ = f;
  if (fread (header_, sizeof(header_), 1, fl_) != 1) 
    throw (std::string("Failed to read the file header "));

  header = header_;

  if (fread (&numTriangles_, sizeof(numTriangles_), 1, fl_) != 1) 
    throw (std::string("Failed to read the number of tirangles"));
  trigNum_ = 0;
};

StlInBinFile::StlInBinFile (FILE * f) {
  init (f);
}

StlInBinFile::StlInBinFile (const char* fileName)
{
  fl_ = fopen (fileName, "rb");
  if (fl_ == NULL)
    throw (std::string("Cannot open binary STL file ") + 
	   std::string(fileName) + std::string (" for reading"));
  if (fread (header_, sizeof(header_), 1, fl_) != 1) 
    throw (std::string("Failed to read the file header "));
  if (fread (&numTriangles_, sizeof(numTriangles_), 1, fl_) != 1) 
    throw (std::string("Failed to read the number of tirangles"));
  trigNum_ = 0;
};

StlInBinFile::~StlInBinFile ()
{
  if (fl_ != NULL)
    fclose(fl_);
};

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
  trigNum_ ++;
  return true;
}

bool StlInBinFile::readTriangle (Triangle & trngl) {
  return readTriangle (trngl.vertices, trngl.normal);
}

int StlInBinFile::numTriangles() const
{
  return numTriangles_;
}

/*
StlOutBinFile::StlOutBinFile (const char* fileName,
			      int numTrigs,
			      const char* header) : fl_(NULL), numTriangles_(numTrigs)
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
*/



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

void StlOutBinFile::writeTriangle(const V3 trig[3], const V3& normal)
{
  float buf[12];
  unsigned int  n16 = 0;
  float* bp = buf;
  for   (int j=0; j<3; j++)
    *bp++ = normal.p[j];

  for   (int v=0; v<3; v++) 
    for (int j=0; j<3; j++)
      *bp++ = trig[v].p[j];
  if (fwrite(buf, 4, 12, fl_) != 12)
    throw(std::string("Failed to write a triangle"));
  if (fwrite(&n16, 2, 1, fl_) != 1)
    throw(std::string("Failed to write an \"Attribute Byte Count\""));
  numTriangles_++;
}

//============================================================================= Text Version

StlInTextFile::StlInTextFile (const char* fileName) : fl_           (fileName),
                                                      numTriangles_ (0),
                                                      fileName_     (fileName),
                                                      lineNumber_   (0)
{
  trigNum_ = 0;
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

StlInTextFile::~StlInTextFile ()
{
};

bool StlInTextFile::readLine_ (std::string& line) {
  bool rslt = ! (! std::getline (fl_, line));
  lineNumber_++;
  return rslt;
}

bool StlInTextFile::readTriangle (V3 trig[3], V3& normal)
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
    lineStream >> normal.p[0] >> normal.p[1] >> normal.p[2];
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
      lineStream >> trig[k].p[0] >> trig[k].p[1] >> trig[k].p[2];
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
  trigNum_ ++;
  return true;
}


bool StlInTextFile::readTriangle(Triangle & trngl) {
  return readTriangle (trngl.vertices, trngl.normal);
}


int StlInTextFile::numTriangles()
{
  if (numTriangles_ < 0) {
    StlInTextFile  tmp (this->fileName_.c_str());
    numTriangles_ = 0;
    V3 trig[3], normal;
    while (tmp.readTriangle (trig, normal))
      numTriangles_++;
  }
  return numTriangles_;
}

StlOutTextFile::StlOutTextFile (const char* fileName,
			      int numTrigs,
			      const char* header) : fl_(NULL), numTriangles_(numTrigs)
{
  fl_ = fopen (fileName, "wt");
  if (fl_ == NULL)
    throw (std::string("Cannot open text STL file ") + 
	   std::string(fileName) + std::string (" for reading"));
  header_[sizeof(header_)-2] = '\n';
  header_[sizeof(header_)-1] = 0;
  strncpy(header_, header, sizeof(header_)-2);
  if (fwrite (header_, sizeof(header_), 1, fl_) != 1) 
    throw (std::string("Failed to write the file header "));
  // if (fwrite (&numTriangles_, sizeof(numTriangles_), 1, fl_) != 1) 
  //   throw (std::string("Failed to write the number of tirangles"));
}

StlOutTextFile::~StlOutTextFile ()
{
  if (fl_ != NULL)
    fclose(fl_);
}

void StlOutTextFile::writeTriangle(const V3 trig[3], const V3& normal)
{
  float buf[12];
  unsigned int  n16;
  float* bp = buf;
  for   (int j=0; j<3; j++)
    *bp++ = normal.p[j];
  for   (int v=0; v<3; v++) 
    for (int j=0; j<3; j++)
      *bp++ = trig[v].p[j];
  if (fwrite(buf, 4, 12, fl_) != 12)
    throw(std::string("Failed to write a triangle"));
  if (fwrite(&n16, 2, 1, fl_) != 1)
    throw(std::string("Failed to write an \"Attribute Byte Count\""));
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

StlInFile::StlInFile  (const char * fileName) {
  FILE *fl_ = fopen (fileName, "rb");
  if (fl_ == NULL)
    throw (std::string("Cannot open STL file ") + 
	   std::string(fileName) + std::string (" for reading"));
  if (isStlText (fl_)) {
    // Cannot reuse 'fl_' because text STL class uses std::ifstream, not 'FILE*':
    fclose (fl_);
    isText = true;
    actualStlFile = static_cast<StlInBaseFile*> (new StlInTextFile (fileName));
  }
  else {
    isText = false;
    actualStlFile = static_cast<StlInBaseFile*> (new StlInBinFile  (fl_));
  }
}

StlInFile::~StlInFile () {
  if (actualStlFile)
    delete (actualStlFile);
}

bool StlInFile::readTriangle (V3 trig[3], V3& normal) {
  return actualStlFile->readTriangle (trig, normal);
}

bool StlInFile::readTriangle (Triangle& trngl) {
  return actualStlFile->readTriangle (trngl);
}
