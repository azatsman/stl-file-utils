#ifndef ________________________________________included_stl_compute_normals_hpp_34041137
#define ________________________________________included_stl_compute_normals_hpp_34041137

#include "stlfile.hpp"

static inline
void computeNormal (Triangle& trngl) {
  V3
    s10     = trngl.vertices[1] - trngl.vertices[0],
    s20     = trngl.vertices[2] - trngl.vertices[0],
    normal  = s10.cross (s20),
    normal1 = normal * (1.0 / normal.norm());
  trngl.normal = normal1;
}

#endif /*________________________________________included_stl_compute_normals_hpp_34041137*/
