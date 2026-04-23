#ifndef ________________________________________included_stl_collections_hpp_72893419
#define ________________________________________included_stl_collections_hpp_72893419


static bool lessV3(V3 u, V3 v)
{
  for (int k=0; k<3; k++) {
    if      (u.p[k] < v.p[k])
      return true;
    else if (u.p[k] > v.p[k])
      return false;
  }
  return false;
}

struct CompV3 {
  bool operator() (V3 u, V3 v) const {return lessV3 (u, v);}
};

//............................................ Ordered edge (ordered pair of vertices)

struct Edge {
  V3 pnt0;
  V3 pnt1;
  Edge (V3 p0, V3 p1) {
    pnt0 = p0;
    pnt1 = p1;
  }
  int reorder () {
    if (lessV3 (pnt0, pnt1))
      return 1;
    else {
      std::swap (pnt0, pnt1);
      return -1;
    }
  }
};

struct lessEdge {
  bool operator() (Edge e1, Edge e2) const {
    if      (lessV3(e1.pnt0, e2.pnt0))
      return true;
    else if (lessV3(e2.pnt0, e1.pnt0))
      return false;
    else if (lessV3(e2.pnt1, e1.pnt1))
      return true;
    else
      return false;
  }
};

typedef std::vector<int> TrindexSet;   // Set/array of triangle indices

struct EdgeDesc {
  TrindexSet trindices;
  int sign;

  EdgeDesc () : trindices(), sign(0) {};

  void addTrig(int trindex, int sgn) {
    trindices.push_back (trindex);
    sign += sgn;
  }
};

typedef std::map <Edge, EdgeDesc, lessEdge> EdgeMap;

typedef std::vector<Triangle> TriangleArray

typedef std::map <Edge, EdgeDesc, lessEdge);

static inline addTriangleEdges (EdgeMap& edgeMap,
                                const Triangle& trngl,
                                int ix) {
  Edge
    edgeKey01(trngl.vertices[0], trngl.vertices[1]),
    edgeKey12(trngl.vertices[1], trngl.vertices[2]),
    edgeKey20(trngl.vertices[2], trngl.vertices[0]);
  int
    sgn01 = edgeKey01.reorder(),
    sgn12 = edgeKey12.reorder(),
    sgn20 = edgeKey20.reorder();
  edgeMap [edgeKey01].addTrig(trngl.ix, sgn01);
  edgeMap [edgeKey12].addTrig(trngl.ix, sgn12);
  edgeMap [edgeKey20].addTrig(trngl.ix, sgn20);
}

#endif /*________________________________________included_stl_collections_hpp_72893419*/
