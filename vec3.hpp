#ifndef ________________________________________included_vec3_hpp_9899015
#define ________________________________________included_vec3_hpp_9899015

#include <iostream>

/* 3D Linear Algebra */

#include <string.h>

//#include <math.h>

#include <cmath>

template <typename Real> class Vec3 {
 public:
  Real p[3];
  
  Vec3() {p[0] = p[1] = p[2] = 0;};

  //   Vec3 (Real val) {p[0] = p[1] = p[2] = val;};

  Vec3(Real x, Real y, Real z) {
    p[0] = x;
    p[1] = y;
    p[2] = z;
  };
  
  Vec3(Real q[3]) {
    p[0] = q[0];
    p[1] = q[1];
    p[2] = q[2];
  };
  
  Real x() const {return p[0];};
  Real y() const {return p[1];};
  Real z() const {return p[2];};

  Real & operator () (int i) {return p[i];};
  Real & operator [] (int i) {return p[i];};




  // Matrix addition:

  Vec3 operator + (Vec3 x) const {
    Vec3 r (p[0]+x.p[0], p[1]+x.p[1], p[2]+x.p[2]);
    return r;
  };

  void operator += (Vec3 x) {
    p[0] += x.p[0];
    p[1] += x.p[1];
    p[2] += x.p[2];
  };

  Vec3 operator - (Vec3 x) const {
    Vec3 r (p[0]-x.p[0], p[1]-x.p[1], p[2]-x.p[2]);
    return r;
  };

  Vec3 operator / (Vec3 x) const {
    Vec3 r (p[0]/x.p[0], p[1]/x.p[1], p[2]/x.p[2]);
    return r;
  };

  // Scaling by a scalar:

  Vec3 operator * (Real a) const {
    Vec3 r (p[0]*a, p[1]*a, p[2]*a);
    return r;
  };

  Vec3 compProd (Vec3 x) const {
    Vec3 rslt (p[0] * x.p[0],
               p[1] * x.p[1],
               p[2] * x.p[2]);
    return rslt;
  }

  // Dot product:

  Real operator * (Vec3 x) const {
    Real r = 
      p[0] * x.p[0] +
      p[1] * x.p[1] +
      p[2] * x.p[2];
    return r;
  };

  Vec3 operator - () const {
    Vec3 r = Vec3 (-p[0], -p[1], -p[2]);
    return r;
  };

  Vec3 cross(Vec3 x) const {

    // Computed according to Right Hand Rule or Right Hand Screw:
    // If rotation from 'this' to 'x' is counter-clockwise the the resulting cross product
    // is pointing to the the viewer.

    Vec3 r;
    r.p[0] = p[1]*x.p[2] - p[2]*x.p[1];
    r.p[1] = p[2]*x.p[0] - p[0]*x.p[2];
    r.p[2] = p[0]*x.p[1] - p[1]*x.p[0];
    return r;
  };

  Real sumSquares() const {
    Real rslt = p[0]*p[0] + p[1]*p[1] + p[2]*p[2];
    return rslt;
  }
  
  Real norm() const {
    return sqrt(sumSquares());
  }
  
  // Normalize myself:

  void normalize() {
    Real normInv = 1.0 / norm();
    p[0] *= normInv;
    p[1] *= normInv;
    p[2] *= normInv;
  }

  // Return a normalized copy:

  Vec3 normalized() const {
    Real normInv = 1.0 / norm();
    Vec3 r (p[0] * normInv,
	    p[1] * normInv,
	    p[2] * normInv);
    return r;
  }

  static int numRows () {return 3;}
};

//inline static
template <typename Real>
std::ostream&  operator<< (std::ostream& out, Vec3<Real> v)
{
  out << "(" << v.p[0] << ", " << v.p[1] << ", " << v.p[2] << ")";
  return out;
}

template <typename Real>
class Mat3 {
public:
  Real m[3][3];

  // A zero matrix:

  Mat3 (void) {
    for   (int i=0; i<3; i++)
      for (int j=0; j<3; j++)
	m[i][j] = 0;
  };

  // A diagonal matrix with different diagonal elements:

  Mat3 (Real d1, Real d2, Real d3) {
    for   (int i=0; i<3; i++)
      for (int j=0; j<3; j++)
	m[i][j] = 0;
    m[0][0] = d1;
    m[1][1] = d2;
    m[2][2] = d3;
  };

  Mat3 (const Real d[3]) {
    for   (int i=0; i<3; i++)
      for (int j=0; j<3; j++)
	m[i][j] = 0;
    for   (int i=0; i<3; i++)
      m[i][i] = d[i];
  };

  // A diagonal matrix with the same diagonal element (a*I) :

  Mat3 (Real a) {
    for   (int i=0; i<3; i++)
      for (int j=0; j<3; j++)
	m[i][j] = 0;
    m[0][0] = m[1][1] = m[2][2] = a;
  };

  // Create matrix from a 3x3 array.

  Mat3 (Real m0[3][3]) {
    memcpy ((void*) m, (void*) m0, sizeof(m));
  };

  // Initialize matrix from columns:
  Mat3 (Vec3<Real> col0, Vec3<Real> col1, Vec3<Real> col2) {
    for (int row=0; row<3; row++) {
      m[row][0] = col0.p[row];
      m[row][1] = col1.p[row];
      m[row][2] = col2.p[row];
    }
  };

  // Initialize matrix from elements in row-first order:
  Mat3 (Real a00, Real a01, Real a02,
        Real a10, Real a11, Real a12,
        Real a20, Real a21, Real a22) {
    m[0][0] = a00;
    m[0][1] = a01;
    m[0][2] = a02;
    m[1][0] = a10;
    m[1][1] = a11;
    m[1][2] = a12;
    m[2][0] = a20;
    m[2][1] = a21;
    m[2][2] = a22;
  };

  void getColumns (Real* tgt) {
    int k=0;
    for   (int j=0; j<3; j++)
      for (int i=0; i<3; i++)
	tgt[k++] = m[i][j];
  }

  // Extract element i,j :

  Real & operator() (int i, int j) {
    return m[i][j];
  }
  
  // Matrix addition

  Mat3 operator + (Mat3 x) const {
    Mat3 r;
    for   (int i=0; i<3; i++)
      for (int j=0; j<3; j++)
	r.m[i][j] = m[i][j] + x.m[i][j];
    return r;
  };
  
  Mat3 operator - (Mat3 x) {
    Mat3 r;
    for   (int i=0; i<3; i++)
      for (int j=0; j<3; j++)
	r.m[i][j] = m[i][j] - x.m[i][j];
    return r;
  };
  
  // Matrix multiplicaiton

  Mat3 operator * (Mat3 x) const {
    Mat3 r;
    for   (int i=0; i<3; i++)
      for (int j=0; j<3; j++)
	r.m[i][j] = 
	  m[i][0] * x.m[0][j] +
	  m[i][1] * x.m[1][j] +
	  m[i][2] * x.m[2][j];
    return r;
  };
  
  Vec3<Real> operator * (Vec3<Real> x) const {
    Vec3<Real> r;
    for   (int i=0; i<3; i++)
      r.p[i] = m[i][0]*x.p[0] + m[i][1]*x.p[1] + m[i][2]*x.p[2];
    return r;
  };
  
  // Extract a column vector;

  Vec3<Real> column (int colnum) {
    Vec3<Real> r (m[0][colnum],
                  m[1][colnum],
                  m[2][colnum]);
    return r;
  };

  Mat3 transpose() {
    Real q[3][3];
    q[0][0] = m[0][0];
    q[1][1] = m[1][1];
    q[2][2] = m[2][2];
    
    q[0][1] = m[1][0];
    q[0][2] = m[2][0];
    q[1][0] = m[0][1];
    q[1][2] = m[2][1];
    q[2][0] = m[0][2];
    q[2][1] = m[1][2];

    Mat3<Real> rslt(q);
    return rslt;
  }
  
  Real det() const {
    Real rslt = 0
      + m[0][0] * m[1][1] * m[2][2]
      - m[0][0] * m[1][2] * m[2][1]
      - m[0][1] * m[1][0] * m[2][2]
      + m[0][1] * m[1][2] * m[2][0]
      + m[0][2] * m[1][0] * m[2][1]
      - m[0][2] * m[1][1] * m[2][0];
    return rslt;
  }

  Mat3 invert () {
    Mat3 r;
    Real detInv = 1.0 / det();
    r.m[0][0] = detInv * (m[1][1] * m[2][2] - m[1][2] * m[2][1]);
    r.m[1][0] = detInv * (m[1][2] * m[2][0] - m[1][0] * m[2][2]);
    r.m[2][0] = detInv * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
    r.m[0][1] = detInv * (m[2][1] * m[0][2] - m[2][2] * m[0][1]);
    r.m[1][1] = detInv * (m[2][2] * m[0][0] - m[2][0] * m[0][2]);
    r.m[2][1] = detInv * (m[2][0] * m[0][1] - m[2][1] * m[0][0]);
    r.m[0][2] = detInv * (m[0][1] * m[1][2] - m[0][2] * m[1][1]);
    r.m[1][2] = detInv * (m[0][2] * m[1][0] - m[0][0] * m[1][2]);
    r.m[2][2] = detInv * (m[0][0] * m[1][1] - m[0][1] * m[1][0]);
    return r;
  };
  
  Real sumSquares() {
    Real rslt = 0;
    for   (int i=0; i<3; i++)
      for (int j=0; j<3; j++)
	rslt += m[i][j] * m[i][j];
    return rslt;
  }

  static int numRows    () {return 3;}
  static int numColumns () {return 3;}
  
  typedef Real ElementType;

};

// Matrix for rotation around axis number 'axnum' by 'angle'

template <typename Real>
inline Mat3<Real> rotmat (int axnum, Real angle)
{
  Mat3<Real> r((Real)1);
  int axis1 = (axnum+1)%3;
  int axis2 = (axnum+2)%3;
  Real 
    ca = cos(angle),
    sa = sin(angle);
  
  r.m[axis1][axis1] =  ca;
  r.m[axis1][axis2] =  sa;
  r.m[axis2][axis1] = -sa;
  r.m[axis2][axis2] =  ca;
  return r;
}

template <typename Real>
inline Mat3<Real> rotmatdeg (int axnum, Real angle_deg)
{
  return rotmat (axnum, angle_deg * M_PI / 180.0);
}

// latitude and longitude angles are in degrees:

template <typename Real>
inline Mat3<Real> geomat (Real latitude, Real longitude, Real azimuth)
{
  Real deg2rad = M_PI / 180;

  Real coslat = cos(latitude  * deg2rad);
  Real sinlat = sin(latitude  * deg2rad);
  Real coslon = cos(longitude * deg2rad);
  Real sinlon = sin(longitude * deg2rad);
  Real cosazm = cos(azimuth   * deg2rad);
  Real sinazm = sin(azimuth   * deg2rad);

  Vec3<Real> e0 ( coslat*coslon,  coslat*sinlon, sinlat);
  Vec3<Real> en (-sinlat*coslon, -sinlat*sinlon, coslat);
  Vec3<Real> ee (-sinlon, coslon, 0);

  Vec3<Real> e1 ( en *   cosazm  + ee*sinazm);
  Vec3<Real> e2 ( en * (-sinazm) + ee*cosazm);

  Mat3<Real> r (e0, e1, e2);
  return r;
}

#endif /*________________________________________included_vec3_hpp_9899015*/
