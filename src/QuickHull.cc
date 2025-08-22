#include <functional>

#include <comp/Camera.h>
#include <comp/Mesh.h>
#include <comp/Transform.h>
#include <ds/HashMap.h>
#include <ds/List.h>
#include <gfx/Material.h>
#include <gfx/Mesh.h>
#include <math/Constants.h>
#include <math/Plane.h>
#include <math/Ray.h>
#include <math/Utility.h>
#include <math/Vector.h>
#include <rsl/Library.h>
#include <world/Object.h>
#include <world/World.h>

#include "QuickHull.h"

struct Hull {
  struct HalfEdge;
  struct Face;
  struct Vertex {
    Vec3 mPosition;
    Ds::List<HalfEdge>::Iter mHalfEdge;
  };
  struct HalfEdge {
    Ds::List<Vertex>::Iter mVertex;
    Ds::List<HalfEdge>::Iter mTwin;
    Ds::List<HalfEdge>::Iter mNext;
    Ds::List<HalfEdge>::Iter mPrev;
    Ds::List<Face>::Iter mFace;
  };
  struct Face {
    Ds::List<HalfEdge>::Iter mHalfEdge;
    Math::Plane Plane() const;
    Vec3 Center() const;
  };
  Ds::List<Vertex> mVertices;
  Ds::List<HalfEdge> mHalfEdges;
  Ds::List<Face> mFaces;

  static Result QuickHull(const Ds::Vector<Vec3>& points, Video* vid);

  static void CreateResources();
  static const Vec4 smVertexColor;
  static const Vec4 smAddedVertexColor;
  static const Vec4 smRemovedVertexColor;
  static const Vec4 smRodColor;
  static const Vec4 smAddedRodColor;
  static const Vec4 smRemovedRodColor;
  static const Vec4 smMergedRodColor;
  static const Vec4 smPulseColor;
  static const Vec4 smVanishColor;
};

const Vec4 Hull::smVertexColor = {1, 1, 1, 1};
const Vec4 Hull::smAddedVertexColor = {2, 9, 2, 1};
const Vec4 Hull::smRemovedVertexColor = {9, 2, 2, 1};
const Vec4 Hull::smRodColor = {1, 1, 1, 1};
const Vec4 Hull::smAddedRodColor = {2, 9, 2, 1};
const Vec4 Hull::smRemovedRodColor = {9, 2, 2, 1};
const Vec4 Hull::smMergedRodColor = {2, 9, 9, 1};
const Vec4 Hull::smPulseColor = {5, 5, 5, 1};
const Vec4 Hull::smVanishColor = {1, 1, 1, 0};

void Hull::CreateResources() {
  static Rsl::Asset& asset = Rsl::RequireAsset("QuickHull/asset");
  asset.InitRes<Gfx::Material>("VertexColor", "vres/renderer:Color")
    .Add<Vec4>("uColor") = smVertexColor;
  asset.InitRes<Gfx::Material>("AddedVertexColor", "vres/renderer:Color")
    .Add<Vec4>("uColor") = smAddedVertexColor;
  asset.InitRes<Gfx::Material>("RemovedVertexColor", "vres/renderer:Color")
    .Add<Vec4>("uColor") = smRemovedVertexColor;
  asset.InitRes<Gfx::Material>("RodColor", "vres/renderer:Color")
    .Add<Vec4>("uColor") = smRodColor;
  asset.InitRes<Gfx::Material>("AddedRodColor", "vres/renderer:Color")
    .Add<Vec4>("uColor") = smAddedRodColor;
  asset.InitRes<Gfx::Material>("RemovedRodColor", "vres/renderer:Color")
    .Add<Vec4>("uColor") = smRemovedRodColor;
  asset.InitRes<Gfx::Material>("MergedRodColor", "vres/renderer:Color")
    .Add<Vec4>("uColor") = smMergedRodColor;
  asset.InitRes<Gfx::Material>("PulseColor", "vres/renderer:Color")
    .Add<Vec4>("uColor") = smPulseColor;
}

template<>
size_t Ds::Hash(const Ds::List<Hull::Face>::Iter& it) {
  return (size_t)it.Current();
}

template<>
size_t Ds::Hash(const Ds::List<Hull::HalfEdge>::CIter& it) {
  return (size_t)it.Current();
}

template<>
size_t Ds::Hash(const Vec3& pos) {
  // These numbers were chosen randomly.
  return (size_t)(pos[0] * 153.04f, pos[1] * 268.22f, pos[2] * 58.6f);
}

Math::Plane Hull::Face::Plane() const {
  Ds::Vector<Vec3> points;
  Ds::List<HalfEdge>::Iter currentEdge = mHalfEdge;
  do {
    points.Push(currentEdge->mVertex->mPosition);
    currentEdge = currentEdge->mNext;
  } while (currentEdge != mHalfEdge);
  return Math::Plane::Newell(points);
}

Vec3 Hull::Face::Center() const {
  Vec3 center = {0, 0, 0};
  int count = 0;
  Ds::List<HalfEdge>::Iter currentEdge = mHalfEdge;
  do {
    center += currentEdge->mVertex->mPosition;
    ++count;
    currentEdge = currentEdge->mNext;
  } while (currentEdge != mHalfEdge);
  return center / (float)count;
}

Result Hull::QuickHull(const Ds::Vector<Vec3>& points, Video* vid) {
  // The extreme points are organised like so: x, y, z, -x, -y, -z.
  const Vec3* extremePoints[6] = {&points[0]};
  for (int i = 0; i < 6; ++i) {
    extremePoints[i] = &points[0];
  }
  for (int p = 1; p < points.Size(); ++p) {
    const Vec3& point = points[p];
    for (int i = 0; i < 3; ++i) {
      if (point[i] > (*extremePoints[i])[i]) {
        extremePoints[i] = &point;
      }
      if (point[i] < (*extremePoints[i + 3])[i]) {
        extremePoints[i + 3] = &point;
      }
    }
  }

  // Find an epsilon that accounts for the span of the point collection.
  using namespace Math;
  const Vec3** eps = extremePoints;
  Vec3 maxes = {
    Max(Abs((*eps[0])[0]), Abs((*eps[3])[0])),
    Max(Abs((*eps[1])[1]), Abs((*eps[4])[1])),
    Max(Abs((*eps[2])[2]), Abs((*eps[5])[2]))};
  float epsilon = 3.0f * (maxes[0] + maxes[1] + maxes[2]) * nEpsilon;

  // Cases where extreme points collapse or we don't get a polyhedron need to
  // be handled before we construct a polyhedron. We choose the first four
  // extreme points we find to create a polyhedron.
  Hull hull;
  hull.mVertices.PushBack({*extremePoints[0], hull.mHalfEdges.end()});
  Ds::Vector<Ds::List<Vertex>::Iter> verts;
  verts.Push(hull.mVertices.Back());
  for (size_t i = 1; i < 6; ++i) {
    const Vec3& newPoint = *extremePoints[i];
    if (verts.Size() == 1) {
      if (!Math::Near(newPoint, verts[0]->mPosition, epsilon)) {
        hull.mVertices.PushBack({newPoint, hull.mHalfEdges.end()});
        verts.Push(hull.mVertices.Back());
      }
    }
    else if (verts.Size() == 2) {
      Math::Ray edge =
        Math::Ray::Points(verts[0]->mPosition, verts[1]->mPosition);
      if (!Math::Near(edge.DistanceSq(newPoint), 0.0f, epsilon)) {
        hull.mVertices.PushBack({newPoint, hull.mHalfEdges.end()});
        verts.Push(hull.mVertices.Back());
      }
    }
    else if (verts.Size() == 3) {
      Math::Plane plane = Math::Plane::Points(
        verts[0]->mPosition, verts[1]->mPosition, verts[2]->mPosition);
      float pointDist = plane.Distance(newPoint);
      if (!Math::Near(pointDist, 0.0f, epsilon)) {
        hull.mVertices.PushBack({newPoint, hull.mHalfEdges.end()});
        verts.Push(hull.mVertices.Back());
        if (pointDist < 0.0f) {
          verts.Swap(1, 2);
        }
      }
    }
  }
  if (verts.Size() < 4) {
    return Result("The points do not form a hull.");
  }

  // Create the initial half edge structure representing the polyhedron.
  Ds::List<Hull::HalfEdge>& edgeList = hull.mHalfEdges;
  Ds::List<HalfEdge>::Iter end = edgeList.end();
  Ds::List<Hull::Face>::Iter faces[4] = {
    hull.mFaces.PushBack({end}),
    hull.mFaces.PushBack({end}),
    hull.mFaces.PushBack({end}),
    hull.mFaces.PushBack({end}),
  };
  Ds::List<Hull::HalfEdge>::Iter edges[12] = {
    edgeList.PushBack({verts[0], end, end, end, faces[0]}),
    edgeList.PushBack({verts[1], end, end, end, faces[0]}),
    edgeList.PushBack({verts[2], end, end, end, faces[0]}),
    edgeList.PushBack({verts[1], end, end, end, faces[1]}),
    edgeList.PushBack({verts[0], end, end, end, faces[1]}),
    edgeList.PushBack({verts[3], end, end, end, faces[1]}),
    edgeList.PushBack({verts[2], end, end, end, faces[2]}),
    edgeList.PushBack({verts[1], end, end, end, faces[2]}),
    edgeList.PushBack({verts[3], end, end, end, faces[2]}),
    edgeList.PushBack({verts[0], end, end, end, faces[3]}),
    edgeList.PushBack({verts[2], end, end, end, faces[3]}),
    edgeList.PushBack({verts[3], end, end, end, faces[3]})};

  edges[0]->mTwin = edges[3];
  edges[1]->mTwin = edges[6];
  edges[2]->mTwin = edges[9];
  edges[3]->mTwin = edges[0];
  edges[4]->mTwin = edges[11];
  edges[5]->mTwin = edges[7];
  edges[6]->mTwin = edges[1];
  edges[7]->mTwin = edges[5];
  edges[8]->mTwin = edges[10];
  edges[9]->mTwin = edges[2];
  edges[10]->mTwin = edges[8];
  edges[11]->mTwin = edges[4];

  edges[0]->mNext = edges[1];
  edges[1]->mNext = edges[2];
  edges[2]->mNext = edges[0];
  edges[3]->mNext = edges[4];
  edges[4]->mNext = edges[5];
  edges[5]->mNext = edges[3];
  edges[6]->mNext = edges[7];
  edges[7]->mNext = edges[8];
  edges[8]->mNext = edges[6];
  edges[9]->mNext = edges[10];
  edges[10]->mNext = edges[11];
  edges[11]->mNext = edges[9];

  edges[1]->mPrev = edges[0];
  edges[2]->mPrev = edges[1];
  edges[0]->mPrev = edges[2];
  edges[4]->mPrev = edges[3];
  edges[5]->mPrev = edges[4];
  edges[3]->mPrev = edges[5];
  edges[7]->mPrev = edges[6];
  edges[8]->mPrev = edges[7];
  edges[6]->mPrev = edges[8];
  edges[10]->mPrev = edges[9];
  edges[11]->mPrev = edges[10];
  edges[9]->mPrev = edges[11];

  faces[0]->mHalfEdge = edges[0];
  faces[1]->mHalfEdge = edges[3];
  faces[2]->mHalfEdge = edges[6];
  faces[3]->mHalfEdge = edges[9];

  verts[0]->mHalfEdge = edges[0];
  verts[1]->mHalfEdge = edges[3];
  verts[2]->mHalfEdge = edges[6];
  verts[3]->mHalfEdge = edges[11];

  // Create a conflict list for each face. Each conflict list stores a vector of
  // points that do not lie in the hull.
  struct ConflictList {
    Math::Plane mPlane;
    struct Point {
      Vec3 mPosition;
      float mDistance;
    };
    Ds::Vector<Point> mPoints;
    ConflictList(const Math::Plane& plane) {
      mPlane = plane;
    }
  };
  Ds::HashMap<Ds::List<Face>::Iter, ConflictList> faceConflictLists;
  Ds::List<Face>::Iter faceIt = hull.mFaces.begin();
  while (faceIt != hull.mFaces.end()) {
    faceConflictLists.Insert(faceIt, ConflictList(faceIt->Plane()));
    ++faceIt;
  }

  // Find the plane each point is closest to and give the point to that plane's
  // conflict list. Conflict lists that didn't receive a point are removed.
  auto assignConflictPoint =
    [&](
      const Vec3& point,
      Ds::HashMap<Ds::List<Face>::Iter, ConflictList>& faceConflictLists)
    -> bool {
    float minDist = FLT_MAX;
    auto bestFaceConflictListIt = faceConflictLists.end();
    auto faceContlictListIt = faceConflictLists.begin();
    while (faceContlictListIt != faceConflictLists.end()) {
      float dist = faceContlictListIt->mValue.mPlane.Distance(point);
      if (dist > epsilon && dist < minDist) {
        minDist = dist;
        bestFaceConflictListIt = faceContlictListIt;
      }
      ++faceContlictListIt;
    }
    if (bestFaceConflictListIt != faceConflictLists.end()) {
      bestFaceConflictListIt->mValue.mPoints.Push({point, minDist});
      return true;
    }
    return false;
  };

  // We only use unique points to define the hull. Equivalent points can
  // potentially be added to the hull multiple times, resulting in a degenerate
  // face. This is caused by a point lying outside of an average plane defined
  // by a face containing an equivalent point.
  Ds::Vector<Vec3> uniquePoints;
  for (const Vec3& point: points) {
    bool unique = true;
    for (const Vec3& uniquePoint: uniquePoints) {
      if (Near(point, uniquePoint, epsilon)) {
        unique = false;
        break;
      }
    }
    if (unique) {
      uniquePoints.Push(point);
    }
  }
  Ds::Vector<Vec3> removedPoints;
  for (const Vec3& uniquePoint: uniquePoints) {
    if (!assignConflictPoint(uniquePoint, faceConflictLists)) {
      removedPoints.Push(uniquePoint);
    }
  }

  // Animation /////////////////////////////////////////////////////////////////
  World::Space& space = vid->mLayerIt->mSpace;
  Sequence& seq = vid->mSeq;
  constexpr float vertexSphereScale = 1.0f / 20.0f;
  Ds::HashMap<Vec3, World::Object> vertexSpheres;
  World::Object parentObject = space.CreateObject();
  for (const Vec3& uniquePoint: uniquePoints) {
    World::Object vertexSphere =
      vertexSpheres.Insert(uniquePoint, parentObject.CreateChild())->mValue;
    auto& mesh = vertexSphere.Add<Comp::Mesh>();
    mesh.mMeshId = "vres/gizmo:Sphere";
    mesh.mMaterialId = "QuickHull/asset:VertexColor";
    mesh.mVisible = false;
    auto& transform = vertexSphere.Get<Comp::Transform>();
    transform.SetTranslation(uniquePoint);
    transform.SetUniformScale(0.0f);
  }

  seq.Add({
    .mName = "CreateAllPotentialVetices",
    .mDuration = 1.0f,
    .mEase = EaseType::Linear,
    .mBegin =
      [=](Sequence::Cross dir) {
        for (const auto& vsIt: vertexSpheres) {
          auto& mesh = vsIt.mValue.Get<Comp::Mesh>();
          if (dir == Sequence::Cross::In) {
            mesh.mVisible = true;
          }
          else {
            mesh.mVisible = false;
          }
        }
      },
    .mLerp =
      [=](float t) {
        for (const auto& vsIt: vertexSpheres) {
          vsIt.mValue.Get<Comp::Transform>().SetUniformScale(
            t * vertexSphereScale);
        }
      },
  });
  seq.Wait();

  Ds::Vector<Vec3> initialVertexPositions;
  for (auto vertIt = hull.mVertices.cbegin(); vertIt != hull.mVertices.cend();
       ++vertIt) {
    initialVertexPositions.Push(vertIt->mPosition);
  }

  seq.Add({
    .mName = "HighlightInitialVertices",
    .mDuration = 1.0f,
    .mEase = EaseType::Linear,
    .mBegin =
      [=](Sequence::Cross dir) {
        for (const Vec3& pos: initialVertexPositions) {
          World::Object vertexSphere = vertexSpheres.Find(pos)->mValue;
          auto& mesh = vertexSphere.Get<Comp::Mesh>();
          if (dir == Sequence::Cross::In) {
            mesh.mMaterialId = "QuickHull/asset:AddedVertexColor";
          }
          else {
            mesh.mMaterialId = "QuickHull/asset:VertexColor";
          }
        }
      },
    .mLerp =
      [=](float t) {
        Rsl::GetRes<Gfx::Material>("QuickHull/asset:AddedVertexColor")
          .Get<Vec4>("uColor") = Lerp(smVertexColor, smAddedVertexColor, t);
      },
  });
  seq.Wait();

  struct EdgeRodInfo {
    World::Object mObject;
    Vec3 mEdgeCenter;
    Vec3 mVertexPosition;
    Vec3 mRodSpan;
  };
  auto createEdgeRods =
    [&parentObject](
      const Ds::Vector<Ds::List<HalfEdge>::CIter>& newRodEdgeIters,
      Ds::HashMap<Ds::List<HalfEdge>::CIter, EdgeRodInfo>* edgeRodInfos) {
      for (const auto& edgeIt: newRodEdgeIters) {
        Vec3 vertexPosition = edgeIt->mVertex->mPosition;
        Vec3 twinVertexPosition = edgeIt->mTwin->mVertex->mPosition;
        Vec3 edgeCenter = (vertexPosition + twinVertexPosition) / 2.0f;
        Vec3 rodSpan = vertexPosition - edgeCenter;
        EdgeRodInfo newInfo = {
          parentObject.CreateChild(), edgeCenter, vertexPosition, rodSpan};
        edgeRodInfos->Insert(edgeIt, newInfo);

        World::Object& edgeRod = newInfo.mObject;
        auto& mesh = edgeRod.Add<Comp::Mesh>();
        mesh.mMeshId = "QuickHull/asset:Rod";
        mesh.mMaterialId = "QuickHull/asset:RodColor";
        mesh.mVisible = false;
        auto& transform = edgeRod.Get<Comp::Transform>();
        transform.SetTranslation((vertexPosition + twinVertexPosition) / 2.0f);
        transform.SetScale({0, 0, 0});
      }
    };

  Ds::Vector<Ds::List<HalfEdge>::CIter> newRodEdgeIters;
  Ds::List<HalfEdge>::CIter edgeIt = hull.mHalfEdges.cbegin();
  Ds::List<HalfEdge>::CIter edgeItE = hull.mHalfEdges.cend();
  while (edgeIt != edgeItE) {
    newRodEdgeIters.Push(edgeIt);
    ++edgeIt;
  }
  Ds::HashMap<Ds::List<HalfEdge>::CIter, EdgeRodInfo> edgeRodInfos;
  createEdgeRods(newRodEdgeIters, &edgeRodInfos);

  seq.Add({
    .mName = "CreateInitialRods",
    .mDuration = 1.0f,
    .mEase = EaseType::Linear,
    .mBegin =
      [=](Sequence::Cross dir) {
        for (const auto& info: edgeRodInfos) {
          auto& mesh = info.mValue.mObject.Get<Comp::Mesh>();
          if (dir == Sequence::Cross::In) {
            mesh.mVisible = true;
            mesh.mMaterialId = "QuickHull/asset:AddedRodColor";
          }
          else {
            mesh.mVisible = false;
            mesh.mMaterialId = "QuickHull/asset:RodColor";
          }
        }
      },
    .mLerp =
      [=](float t) {
        for (const auto& info: edgeRodInfos) {
          auto& transform = info.mValue.mObject.Get<Comp::Transform>();
          Quat orientation = Quat::FromTo({1, 0, 0}, info.mValue.mRodSpan);
          transform.SetRotation(orientation);
          Vec3 rodEnd = info.mValue.mEdgeCenter + t * info.mValue.mRodSpan;
          Vec3 rodCenter = (info.mValue.mEdgeCenter + rodEnd) / 2.0f;
          transform.SetTranslation(rodCenter);
          Vec3 currentRodSpan = rodEnd - info.mValue.mEdgeCenter;
          transform.SetScale({Math::Magnitude(currentRodSpan), 0.5f, 0.5f});
        }
      },
  });
  seq.Wait();

  seq.Add({
    .mName = "FadeAwayInitialAddedColors",
    .mDuration = 1.0f,
    .mEase = EaseType::Linear,
    .mLerp =
      [=](float t) {
        Rsl::GetRes<Gfx::Material>("QuickHull/asset:AddedRodColor")
          .Get<Vec4>("uColor") = Lerp(smAddedRodColor, smRodColor, t);
        Rsl::GetRes<Gfx::Material>("QuickHull/asset:AddedVertexColor")
          .Get<Vec4>("uColor") = Lerp(smAddedVertexColor, smVertexColor, t);
      },
    .mEnd =
      [=](Sequence::Cross dir) {
        for (const auto& info: edgeRodInfos) {
          auto& mesh = info.mValue.mObject.Get<Comp::Mesh>();
          if (dir == Sequence::Cross::In)
            mesh.mMaterialId = "QuickHull/asset:AddedRodColor";
          else {
            mesh.mMaterialId = "QuickHull/asset:RodColor";
          }
        }
        for (const Vec3& pos: initialVertexPositions) {
          World::Object vertexSphere = vertexSpheres.Find(pos)->mValue;
          auto& mesh = vertexSphere.Get<Comp::Mesh>();
          if (dir == Sequence::Cross::In) {
            mesh.mMaterialId = "QuickHull/asset:AddedVertexColor";
          }
          else {
            mesh.mMaterialId = "QuickHull/asset:VertexColor";
          }
        }
      },
  });
  seq.Wait();

  seq.Add({
    .mName = "HighlightRemovedVertices",
    .mDuration = 1.0f,
    .mEase = EaseType::Linear,
    .mBegin =
      [=](Sequence::Cross dir) {
        for (const Vec3& removedPoint: removedPoints) {
          auto& mesh =
            vertexSpheres.Find(removedPoint)->mValue.Get<Comp::Mesh>();
          if (dir == Sequence::Cross::In) {
            mesh.mMaterialId = "QuickHull/asset:RemovedVertexColor";
          }
          else {
            mesh.mMaterialId = "QuickHull/asset:VertexColor";
          }
        }
      },
    .mLerp =
      [=](float t) {
        Rsl::GetRes<Gfx::Material>("QuickHull/asset:RemovedVertexColor")
          .Get<Vec4>("uColor") = Lerp(smVertexColor, smRemovedVertexColor, t);
      },
  });
  seq.Wait();

  seq.Add({
    .mName = "RemoveRemovedVertexSpheres",
    .mDuration = 1.0f,
    .mEase = EaseType::Linear,
    .mLerp =
      [=](float t) {
        for (const Vec3& removedPoint: removedPoints) {
          auto& transform =
            vertexSpheres.Find(removedPoint)->mValue.Get<Comp::Transform>();
          transform.SetUniformScale((1.0f - t) * vertexSphereScale);
        }
      },
    .mEnd =
      [=](Sequence::Cross dir) {
        for (const Vec3& removedPoint: removedPoints) {
          auto& mesh =
            vertexSpheres.Find(removedPoint)->mValue.Get<Comp::Mesh>();
          if (dir == Sequence::Cross::In) {
            mesh.mVisible = true;
            mesh.mMaterialId = "QuickHull/asset:RemovedVertexColor";
          }
          else {
            mesh.mVisible = false;
            mesh.mMaterialId = "QuickHull/asset:VertexColor";
          }
        }
        Rsl::GetRes<Gfx::Material>("QuickHull/asset:RemovedVertexColor")
          .Get<Vec4>("uColor") = smRemovedVertexColor;
      },
  });
  seq.Wait();
  // !Animation ////////////////////////////////////////////////////////////////

  // Any faces without conflicting points do not need to be considered.
  auto faceConflictListIt = faceConflictLists.begin();
  while (faceConflictListIt != faceConflictLists.end()) {
    if (faceConflictListIt->mValue.mPoints.Size() == 0) {
      faceConflictListIt = faceConflictLists.Remove(faceConflictListIt);
    }
    else {
      ++faceConflictListIt;
    }
  }

  // The convex hull has been obtained once all conflicting points are handled.
  while (faceConflictLists.Size() > 0) {
    // For all points in the conflict lists, find the point with maximum
    // distance from its respective plane. This point will be added next.
    float maxDist = -FLT_MAX;
    auto bestFaceConflictListIt = faceConflictLists.end();
    int bestConflictPointIdx = -1;
    auto it = faceConflictLists.begin();
    while (it != faceConflictLists.end()) {
      for (int p = 0; p < it->mValue.mPoints.Size(); ++p) {
        const ConflictList::Point& conflictPoint = it->mValue.mPoints[p];
        if (conflictPoint.mDistance > maxDist) {
          maxDist = conflictPoint.mDistance;
          bestFaceConflictListIt = it;
          bestConflictPointIdx = p;
        }
      }
      ++it;
    }

    // Treating the best point as an eye looking towards the current hull,
    // find the edges that form the horizon around the hull. The horizon edges
    // are the edges that border the faces to be deleted and they are stored in
    // a ccw order.
    ConflictList& conflictList = bestFaceConflictListIt->mValue;
    Vec3 newPoint = conflictList.mPoints[bestConflictPointIdx].mPosition;
    // The point is being added to the hull and is hence no longer a conflict.
    conflictList.mPoints.LazyRemove(bestConflictPointIdx);
    Ds::Vector<Ds::List<HalfEdge>::Iter> horizon;
    Ds::Vector<Ds::List<Face>::Iter> visitedFaces;
    std::function<void(Ds::List<HalfEdge>::CIter)> visitEdge =
      [&](Ds::List<HalfEdge>::CIter edge) {
        if (visitedFaces.Contains(edge->mFace)) {
          return;
        }
        visitedFaces.Push(edge->mFace);
        Ds::List<HalfEdge>::CIter currentEdge = edge;
        do {
          Ds::List<HalfEdge>::Iter twin = currentEdge->mTwin;
          Math::Plane twinPlane = twin->mFace->Plane();
          if (twinPlane.HalfSpaceContains(newPoint, epsilon)) {
            horizon.Push(twin);
          }
          else {
            visitEdge(twin);
          }
        } while ((currentEdge = currentEdge->mNext) != edge);
      };
    visitEdge(bestFaceConflictListIt->Key()->mHalfEdge);

    removedPoints.Clear();
    removedPoints.Push(newPoint);

    // We create a new vertex for each horizon vertex because it makes deleting
    // no longer needed elements a bit easier.
    Ds::Vector<Ds::List<Vertex>::Iter> newHorizonVerts;
    for (const Ds::List<HalfEdge>::Iter& hEdge: horizon) {
      newHorizonVerts.Push(
        hull.mVertices.PushBack({hEdge->mVertex->mPosition, end}));
    }

    // Animation ///////////////////////////////////////////////////////////////
    // We need the edges bordering the horizon. These edges will be replaced
    // with new edges, and we need to ensure that the edge rods are accessible
    // using the new edge iterators.
    Ds::Vector<Ds::List<HalfEdge>::CIter> oldHorizonBorder;
    for (Ds::List<HalfEdge>::CIter edgeIt: horizon) {
      oldHorizonBorder.Push(edgeIt->mTwin);
    }
    // !Animation //////////////////////////////////////////////////////////////

    // Imagine drawing a line from the best point to each of the vertices that
    // lie on the horizon. The new faces formed by these lines and the horizon
    // edges are created here.
    Ds::List<Vertex>::Iter newVertex = hull.mVertices.PushBack({newPoint, end});
    Ds::HashMap<Ds::List<Face>::Iter, ConflictList> newFaceConflictLists;
    for (int i = 0; i < horizon.Size(); ++i) {
      Ds::List<HalfEdge>::Iter hEdge = horizon[i];
      Ds::List<HalfEdge>::Iter hEdgeNext = horizon[(i + 1) % horizon.Size()];
      Ds::List<Vertex>::Iter nhVert = newHorizonVerts[i];
      Ds::List<Vertex>::Iter nhVertNext =
        newHorizonVerts[(i + 1) % horizon.Size()];

      Ds::List<Face>::Iter newFace = hull.mFaces.PushBack({end});
      Ds::List<HalfEdge>::Iter newEdges[3] = {
        edgeList.PushBack({newVertex, end, end, end, newFace}),
        edgeList.PushBack({nhVert, end, end, end, newFace}),
        edgeList.PushBack({nhVertNext, end, end, end, newFace})};
      newFace->mHalfEdge = newEdges[0];
      newVertex->mHalfEdge = newEdges[0];
      nhVert->mHalfEdge = newEdges[1];

      // Ensure that all edges referencing the old horizon vertex reference the
      // new horizon vertex.
      Ds::List<HalfEdge>::Iter currentOldVertEdge = hEdge;
      do {
        currentOldVertEdge->mVertex = nhVert;
        currentOldVertEdge = currentOldVertEdge->mPrev->mTwin;
      } while (currentOldVertEdge != hEdgeNext->mTwin);

      // Link together all edge edge references and create a conflict list
      // representing the new face.
      hEdgeNext->mTwin = newEdges[1];
      newEdges[1]->mTwin = hEdgeNext;
      for (int e = 0; e < 3; ++e) {
        newEdges[e]->mNext = newEdges[(e + 1) % 3];
        newEdges[(e + 1) % 3]->mPrev = newEdges[e];
      }
      newFaceConflictLists.Insert(newFace, ConflictList(newFace->Plane()));
    }

    // Set the twin references of all edges going to and from the new vertex.
    for (int i = 0; i < horizon.Size(); ++i) {
      Ds::List<HalfEdge>::CIter hEdge = horizon[i];
      Ds::List<HalfEdge>::CIter hEdgeNext = horizon[(i + 1) % horizon.Size()];
      hEdge->mTwin->mNext->mTwin = hEdgeNext->mTwin->mPrev;
      hEdgeNext->mTwin->mPrev->mTwin = hEdge->mTwin->mNext;
    }

    // Animation ///////////////////////////////////////////////////////////////
    // We only create new rods for edges attached to the new vertex.
    Ds::HashMap<Ds::List<HalfEdge>::CIter, EdgeRodInfo> newEdgeRodInfos;
    newRodEdgeIters.Clear();
    for (int i = 0; i < horizon.Size(); ++i) {
      Ds::List<HalfEdge>::CIter hEdge = horizon[i];
      newRodEdgeIters.Push(horizon[i]->mTwin->mNext);
      newRodEdgeIters.Push(horizon[i]->mTwin->mNext->mTwin);
    }
    createEdgeRods(newRodEdgeIters, &newEdgeRodInfos);
    auto newEdgeRodInfosIt = newEdgeRodInfos.cbegin();
    auto newEdgeRodInfosItE = newEdgeRodInfos.cend();
    while (newEdgeRodInfosIt != newEdgeRodInfosItE) {
      edgeRodInfos.Insert(newEdgeRodInfosIt->Key(), newEdgeRodInfosIt->mValue);
      ++newEdgeRodInfosIt;
    }

    // Update the iterators referencing the rod information for rods that lay on
    // the horizon border.
    for (int i = 0; i < horizon.Size(); ++i) {
      auto rodInfoIt = edgeRodInfos.Find(oldHorizonBorder[i]);
      edgeRodInfos.Insert(horizon[i]->mTwin, rodInfoIt->mValue);
      edgeRodInfos.Remove(rodInfoIt);
    }

    seq.Add({
      .mName = "HighlightNewVertex",
      .mDuration = 1.0f,
      .mEase = EaseType::Linear,
      .mBegin =
        [=](Sequence::Cross dir) {
          World::Object vertexSphere = vertexSpheres.Find(newPoint)->mValue;
          auto& mesh = vertexSphere.Get<Comp::Mesh>();
          if (dir == Sequence::Cross::In) {
            mesh.mMaterialId = "QuickHull/asset:AddedVertexColor";
          }
          else {
            mesh.mMaterialId = "QuickHull/asset:VertexColor";
          }
        },
      .mLerp =
        [=](float t) {
          Rsl::GetRes<Gfx::Material>("QuickHull/asset:AddedVertexColor")
            .Get<Vec4>("uColor") = Lerp(smVertexColor, smAddedVertexColor, t);
        },
    });
    seq.Wait();

    seq.Add({
      .mName = "CreateNewEdgeRods",
      .mDuration = 1.0f,
      .mEase = EaseType::Linear,
      .mBegin =
        [=](Sequence::Cross dir) {
          for (const auto& info: newEdgeRodInfos) {
            auto& mesh = info.mValue.mObject.Get<Comp::Mesh>();
            if (dir == Sequence::Cross::In) {
              mesh.mVisible = true;
              mesh.mMaterialId = "QuickHull/asset:AddedRodColor";
            }
            else {
              mesh.mVisible = false;
              mesh.mMaterialId = "QuickHull/asset:RodColor";
            }
          }
        },
      .mLerp =
        [=](float t) {
          for (const auto& info: newEdgeRodInfos) {
            auto& transform = info.mValue.mObject.Get<Comp::Transform>();
            Quat orientation = Quat::FromTo({1, 0, 0}, info.mValue.mRodSpan);
            transform.SetRotation(orientation);
            Vec3 rodEnd =
              info.mValue.mVertexPosition - t * info.mValue.mRodSpan;
            Vec3 rodCenter = (info.mValue.mVertexPosition + rodEnd) / 2.0f;
            transform.SetTranslation(rodCenter);
            Vec3 currentRodSpan = rodEnd - info.mValue.mVertexPosition;
            transform.SetScale({Math::Magnitude(currentRodSpan), 0.5f, 0.5f});
          }
        },
    });
    seq.Wait();

    seq.Add({
      .mName = "FadeAddedColors",
      .mDuration = 1.0f,
      .mEase = EaseType::Linear,
      .mLerp =
        [=](float t) {
          Rsl::GetRes<Gfx::Material>("QuickHull/asset:AddedRodColor")
            .Get<Vec4>("uColor") = Lerp(smAddedRodColor, smRodColor, t);
          Rsl::GetRes<Gfx::Material>("QuickHull/asset:AddedVertexColor")
            .Get<Vec4>("uColor") = Lerp(smAddedVertexColor, smVertexColor, t);
        },
      .mEnd =
        [=](Sequence::Cross dir) {
          for (const auto& info: newEdgeRodInfos) {
            auto& rodMesh = info.mValue.mObject.Get<Comp::Mesh>();
            if (dir == Sequence::Cross::In) {
              rodMesh.mMaterialId = "QuickHull/asset:AddedRodColor";
            }
            else {
              rodMesh.mMaterialId = "QuickHull/asset:RodColor";
            }
          }
          Rsl::GetRes<Gfx::Material>("QuickHull/asset:AddedRodColor")
            .Get<Vec4>("uColor") = smAddedRodColor;

          auto& vertexMesh =
            vertexSpheres.Find(newPoint)->mValue.Get<Comp::Mesh>();
          if (dir == Sequence::Cross::In) {
            vertexMesh.mMaterialId = "QuickHull/asset:AddedVertexColor";
          }
          else {
            vertexMesh.mMaterialId = "QuickHull/asset:VertexColor";
          }
          Rsl::GetRes<Gfx::Material>("QuickHull/asset:AddedVertexColor")
            .Get<Vec4>("uColor") = smAddedVertexColor;
        },
    });
    seq.Wait();
    // !Animation //////////////////////////////////////////////////////////////

    // Any time we delete a face, we need to see if that face has an existing or
    // new conflict list associated with it. If it has an existing conflict
    // list, we save its conflict points in order to reassign them to the new
    // set of conflict lists at the end of the iteration.
    Ds::Vector<Vec3> conflictPoints;
    auto tryRemoveFaceConflictList = [&](Ds::List<Face>::Iter faceIt) {
      auto faceConflictIt = faceConflictLists.Find(faceIt);
      if (faceConflictIt != faceConflictLists.end()) {
        const ConflictList& conflictList = faceConflictIt->mValue;
        for (size_t p = 0; p < conflictList.mPoints.Size(); ++p) {
          conflictPoints.Push(conflictList.mPoints[p].mPosition);
        }
        faceConflictLists.Remove(faceConflictIt);
      }
      auto newFaceConflictIt = newFaceConflictLists.Find(faceIt);
      if (newFaceConflictIt != newFaceConflictLists.end()) {
        newFaceConflictLists.Remove(newFaceConflictIt);
      }
    };

    // Delete dead vertices, edges, faces, and conflict lists that were covered
    // by the new faces.
    Ds::Vector<Ds::List<Vertex>::Iter> deadVerts;
    Ds::Vector<Ds::List<HalfEdge>::Iter> deadEdges;
    for (const Ds::List<Face>::Iter& faceIt: visitedFaces) {
      Ds::List<HalfEdge>::Iter currentEdge = faceIt->mHalfEdge;
      do {
        if (!deadVerts.Contains(currentEdge->mVertex)) {
          deadVerts.Push(currentEdge->mVertex);
        }
        deadEdges.Push(currentEdge);
        currentEdge = currentEdge->mNext;
      } while (currentEdge != faceIt->mHalfEdge);
      tryRemoveFaceConflictList(faceIt);
      hull.mFaces.Erase(faceIt);
    }

    // Animation ///////////////////////////////////////////////////////////////
    Ds::Vector<EdgeRodInfo> removedRodInfos;
    for (auto edgeIt: deadEdges) {
      auto edgeRodInfoIt = edgeRodInfos.Find(edgeIt);
      if (edgeRodInfoIt != edgeRodInfos.end()) {
        removedRodInfos.Push(edgeRodInfoIt->mValue);
        edgeRodInfos.Remove(edgeRodInfoIt);
      }
    }

    if (!removedRodInfos.Empty()) {
      seq.Add({
        .mName = "FadeRemoveColoredRodsToRemovedRodColor",
        .mDuration = 1.0f,
        .mEase = EaseType::Linear,
        .mBegin =
          [=](Sequence::Cross dir) {
            for (const auto& info: removedRodInfos) {
              auto& mesh = info.mObject.Get<Comp::Mesh>();
              if (dir == Sequence::Cross::In) {
                mesh.mMaterialId = "QuickHull/asset:RemovedRodColor";
              }
              else {
                mesh.mMaterialId = "QuickHull/asset:RodColor";
              }
            }
          },
        .mLerp =
          [=](float t) {
            Rsl::GetRes<Gfx::Material>("QuickHull/asset:RemovedRodColor")
              .Get<Vec4>("uColor") = Lerp(smRodColor, smRemovedRodColor, t);
          },
      });
      seq.Wait();

      seq.Add({
        .mName = "RemoveCoveredRods",
        .mDuration = 1.0f,
        .mEase = EaseType::Linear,
        .mLerp =
          [=](float t) {
            for (const auto& info: removedRodInfos) {
              auto& transform = info.mObject.Get<Comp::Transform>();
              Vec3 rodEnd = info.mVertexPosition - (1.0f - t) * info.mRodSpan;
              Vec3 rodCenter = (info.mVertexPosition + rodEnd) / 2.0f;
              transform.SetTranslation(rodCenter);
              Vec3 currentRodSpan = rodEnd - info.mVertexPosition;
              transform.SetScale({Math::Magnitude(currentRodSpan), 0.5f, 0.5f});
            }
          },
        .mEnd =
          [=](Sequence::Cross dir) {
            for (const auto& info: removedRodInfos) {
              auto& mesh = info.mObject.Get<Comp::Mesh>();
              if (dir == Sequence::Cross::In) {
                mesh.mVisible = true;
                mesh.mMaterialId = "QuickHull/asset:RemovedRodColor";
              }
              else {
                mesh.mVisible = false;
                mesh.mMaterialId = "QuickHull/asset:RodColor";
              }
            }
            Rsl::GetRes<Gfx::Material>("QuickHull/asset:RemovedRodColor")
              .Get<Vec4>("uColor") = smRemovedRodColor;
          },
      });
      seq.Wait();
    }
    // !Animation //////////////////////////////////////////////////////////////

    for (const Ds::List<Vertex>::Iter& vertIt: deadVerts) {
      hull.mVertices.Erase(vertIt);
    }
    for (const Ds::List<HalfEdge>::Iter& edgeIt: deadEdges) {
      hull.mHalfEdges.Erase(edgeIt);
    }

    // We now need to merge faces that are coplanar. We only need to check
    // whether faces adjacent across new edges are coplanar. We collect all of
    // those edges here.
    Ds::Vector<Ds::List<HalfEdge>::Iter> possibleMerges;
    Ds::List<HalfEdge>::Iter currentEdge = newVertex->mHalfEdge;
    do {
      possibleMerges.Push(currentEdge->mNext);
      possibleMerges.Push(currentEdge->mNext->mNext);
      currentEdge = currentEdge->mPrev->mTwin;
    } while (currentEdge != newVertex->mHalfEdge);
    auto tryRemovePossibleMerge = [&](Ds::List<HalfEdge>::Iter edge) {
      VResult<size_t> search = possibleMerges.Find(edge);
      if (search.Success()) {
        possibleMerges.LazyRemove(search.mValue);
      }
    };

    // As we merge faces, topological errors can arise. If only two edges emerge
    // from a vertex, we have a topological error. Every vertex needs to have 3
    // edges to make it be a part of the volume.
    Ds::Vector<Ds::List<Vertex>::Iter> mergedVerts;
    Ds::Vector<Ds::List<HalfEdge>::Iter> mergedEdges;
    auto ensureValidVertex = [&](Ds::List<Vertex>::Iter vertex) {
      int vertexEdgeCount = 0;
      Ds::Vector<Ds::List<HalfEdge>::Iter> vertexEdges;
      Ds::List<HalfEdge>::Iter currentVertexEdge = vertex->mHalfEdge;
      do {
        vertexEdges.Push(currentVertexEdge);
        currentVertexEdge = currentVertexEdge->mTwin->mNext;
      } while (currentVertexEdge != vertex->mHalfEdge);
      if (vertexEdges.Size() != 2) {
        return;
      }
      removedPoints.Push(vertex->mPosition);

      // How we deal with this topological error is determined by the number of
      // vertices the two adjacent faces have.
      int faceEdgeCounts[2] = {0, 0};
      for (int ve = 0; ve < 2; ++ve) {
        Ds::List<HalfEdge>::Iter currentFaceEdge = vertexEdges[ve];
        do {
          ++faceEdgeCounts[ve];
          currentFaceEdge = currentFaceEdge->mNext;
        } while (currentFaceEdge != vertexEdges[ve]);
      }

      Ds::List<HalfEdge>::Iter edges[2] = {
        vertex->mHalfEdge->mPrev, vertex->mHalfEdge};
      Ds::List<HalfEdge>::Iter edgeTwins[2] = {
        edges[1]->mTwin, edges[0]->mTwin};
      if (faceEdgeCounts[0] == 3 || faceEdgeCounts[1] == 3) {
        // When one of the faces is a triangle, we must remove the vertex and
        // all edges going to and from it. First we update all references to
        // edges that will be removed.
        edges[0]->mPrev->mNext = edgeTwins[1]->mNext;
        edgeTwins[1]->mNext->mPrev = edges[0]->mPrev;
        edges[1]->mNext->mPrev = edgeTwins[0]->mPrev;
        edgeTwins[0]->mPrev->mNext = edges[1]->mNext;

        // Create the new face used to reprsent the merged faces.
        Ds::List<Face>::Iter newFace = hull.mFaces.PushBack({edges[1]->mNext});
        newFaceConflictLists.Insert(newFace, ConflictList(newFace->Plane()));

        // Ensure all edges within the merged faces reference the new face and
        // that remaining vertices reference existing half edges.
        Ds::List<HalfEdge>::Iter currentEdge = edges[1]->mNext;
        do {
          currentEdge->mFace = newFace;
          currentEdge = currentEdge->mNext;
        } while (currentEdge != edges[1]->mNext);
        edges[0]->mVertex->mHalfEdge = edges[0]->mPrev->mNext;
        edgeTwins[0]->mVertex->mHalfEdge = edgeTwins[0]->mPrev->mNext;

        // Remove no longer necessary elements.
        tryRemoveFaceConflictList(edges[0]->mFace);
        tryRemoveFaceConflictList(edgeTwins[0]->mFace);
        tryRemovePossibleMerge(edges[0]);
        tryRemovePossibleMerge(edges[1]);
        tryRemovePossibleMerge(edgeTwins[0]);
        tryRemovePossibleMerge(edgeTwins[1]);
        mergedVerts.Push(vertex);
        hull.mFaces.Erase(edges[0]->mFace);
        hull.mFaces.Erase(edgeTwins[0]->mFace);
        mergedEdges.Push(edges[0]);
        mergedEdges.Push(edges[1]);
        mergedEdges.Push(edgeTwins[0]);
        mergedEdges.Push(edgeTwins[1]);
      }
      else {
        // When neither of the adjacent faces are triangles, the vertex edges
        // are colinear and must be merged into a single edge. We repurpose one
        // set of half edges to represent the merged edge and update  references
        // to the other two half edges that will be removed.
        edges[0]->mNext = edges[1]->mNext;
        edges[0]->mTwin = edgeTwins[0];
        edges[1]->mNext->mPrev = edges[0];
        edgeTwins[0]->mNext = edgeTwins[1]->mNext;
        edgeTwins[0]->mTwin = edges[0];
        edgeTwins[1]->mNext->mPrev = edgeTwins[0];

        // Ensure that the faces reference existing edges.
        edges[0]->mFace->mHalfEdge = edges[0];
        edgeTwins[0]->mFace->mHalfEdge = edgeTwins[0];

        // Remove no longer necessary elements.
        tryRemovePossibleMerge(edges[1]);
        tryRemovePossibleMerge(edgeTwins[1]);
        mergedVerts.Push(vertex);
        mergedEdges.Push(edges[1]);
        mergedEdges.Push(edgeTwins[1]);

        // Animation ///////////////////////////////////////////////////////////
        // We instantly remove the no longer needed rods and the rods remaining
        // after the colinear merge take up the space of the removed edges.
        Ds::Vector<EdgeRodInfo> disolvedRodInfos;
        disolvedRodInfos.Push(edgeRodInfos.Find(edges[1])->mValue);
        disolvedRodInfos.Push(edgeRodInfos.Find(edgeTwins[1])->mValue);
        edgeRodInfos.Remove(edges[1]);
        edgeRodInfos.Remove(edgeTwins[1]);

        Ds::List<HalfEdge>::CIter expandedEdgeIts[] = {edges[0], edgeTwins[0]};
        Ds::Vector<EdgeRodInfo> expandedRodInfos;
        for (int i = 0; i < 2; ++i) {
          Ds::List<HalfEdge>::CIter expandedEdgeIt = expandedEdgeIts[i];
          EdgeRodInfo& edgeRodInfo = edgeRodInfos.Find(expandedEdgeIt)->mValue;
          Vec3 vertexPosition = expandedEdgeIt->mVertex->mPosition;
          Vec3 twinVertexPosition = expandedEdgeIt->mTwin->mVertex->mPosition;
          Vec3 edgeCenter = (vertexPosition + twinVertexPosition) / 2.0f;
          Vec3 rodSpan = vertexPosition - edgeCenter;
          edgeRodInfo.mEdgeCenter = edgeCenter;
          edgeRodInfo.mRodSpan = rodSpan;
          expandedRodInfos.Push(edgeRodInfo);
        }

        seq.Add({
          .mName = "HandleColinearMerge",
          .mDuration = 0.0f,
          .mBegin =
            [=](Sequence::Cross dir) {
              if (dir == Sequence::Cross::In) {
                for (auto& edgeRodInfo: disolvedRodInfos) {
                  edgeRodInfo.mObject.Get<Comp::Mesh>().mVisible = false;
                }
                for (auto& edgeRodInfo: expandedRodInfos) {
                  auto& transform = edgeRodInfo.mObject.Get<Comp::Transform>();
                  transform.SetTranslation(
                    edgeRodInfo.mEdgeCenter + edgeRodInfo.mRodSpan / 2.0f);
                  transform.SetScale(
                    {Math::Magnitude(edgeRodInfo.mRodSpan), 0.5f, 0.5f});
                }
              }
              if (dir == Sequence::Cross::Out) {
                for (auto& edgeRodInfo: disolvedRodInfos) {
                  edgeRodInfo.mObject.Get<Comp::Mesh>().mVisible = true;
                }
              }
            },
        });
        // !Animation //////////////////////////////////////////////////////////
      }
    };

    // Coplanar faces are merged using one of the edges shared between them.
    auto mergeFaces = [&](Ds::List<HalfEdge>::Iter edge) {
      // Link the edges going away and towards the deleted edge.
      Ds::List<HalfEdge>::Iter edgeTwin = edge->mTwin;
      edge->mPrev->mNext = edgeTwin->mNext;
      edge->mNext->mPrev = edgeTwin->mPrev;
      edgeTwin->mPrev->mNext = edge->mNext;
      edgeTwin->mNext->mPrev = edge->mPrev;

      // Create the new face and ensure vertices reference a remaining half edge
      // and that all remaining edges reference the new face.
      Ds::List<Face>::Iter newFace = hull.mFaces.PushBack({edge->mNext});
      newFaceConflictLists.Insert(newFace, ConflictList(newFace->Plane()));
      edge->mVertex->mHalfEdge = edgeTwin->mNext;
      edgeTwin->mVertex->mHalfEdge = edge->mNext;
      Ds::List<HalfEdge>::Iter currentEdge = edge->mNext;
      do {
        currentEdge->mFace = newFace;
        currentEdge = currentEdge->mNext;
      } while (currentEdge != edge->mNext);

      // Ensure that the two vertices that lost an edge an edge are still valid
      // and erase no long necessary elements.
      ensureValidVertex(edge->mVertex);
      ensureValidVertex(edgeTwin->mVertex);
      tryRemoveFaceConflictList(edge->mFace);
      tryRemoveFaceConflictList(edgeTwin->mFace);
      tryRemovePossibleMerge(edge);
      tryRemovePossibleMerge(edgeTwin);
      hull.mFaces.Erase(edge->mFace);
      hull.mFaces.Erase(edgeTwin->mFace);
      mergedEdges.Push(edge);
      mergedEdges.Push(edgeTwin);
    };

    // Check whether a merge should be performed over all possible merges.
    while (!possibleMerges.Empty()) {
      // If a face's halfspace contains the center of the adjacent face and vice
      // versa, the edge is considered convex.
      Ds::List<HalfEdge>::Iter edge = possibleMerges.Top();
      Ds::List<HalfEdge>::Iter twinEdge = edge->mTwin;
      Plane facePlane = edge->mFace->Plane();
      Vec3 faceCenter = edge->mFace->Center();
      Plane twinFacePlane = twinEdge->mFace->Plane();
      Vec3 twinFaceCenter = twinEdge->mFace->Center();
      bool convex = facePlane.HalfSpaceContains(twinFaceCenter, epsilon) &&
        twinFacePlane.HalfSpaceContains(faceCenter, epsilon);

      // If the edge is convex and the angle between face normals lies within
      // the epsilon, the faces are merged.
      float angle = Math::Angle(facePlane.Normal(), twinFacePlane.Normal());
      const float angleEpsilon = 0.015f;
      if (convex && Near(angle, 0.0f, angleEpsilon)) {
        mergeFaces(edge);
      }
      else {
        possibleMerges.Pop();
      }
    }

    // Animation ///////////////////////////////////////////////////////////////
    Ds::Vector<EdgeRodInfo> mergedRodInfos;
    for (auto edgeIt: mergedEdges) {
      auto edgeRodInfoIt = edgeRodInfos.Find(edgeIt);
      if (edgeRodInfoIt != edgeRodInfos.end()) {
        mergedRodInfos.Push(edgeRodInfoIt->mValue);
        edgeRodInfos.Remove(edgeRodInfoIt);
      }
    }

    if (!mergedRodInfos.Empty()) {
      seq.Add({
        .mName = "FadeMergedColoredRodsToMergedRodColor",
        .mDuration = 1.0f,
        .mEase = EaseType::Linear,
        .mBegin =
          [=](Sequence::Cross dir) {
            for (const auto& info: mergedRodInfos) {
              auto& mesh = info.mObject.Get<Comp::Mesh>();
              if (dir == Sequence::Cross::In) {
                mesh.mMaterialId = "QuickHull/asset:MergedRodColor";
              }
              else {
                mesh.mMaterialId = "QuickHull/asset:RodColor";
              }
            }
          },
        .mLerp =
          [=](float t) {
            Rsl::GetRes<Gfx::Material>("QuickHull/asset:MergedRodColor")
              .Get<Vec4>("uColor") = Lerp(smRodColor, smMergedRodColor, t);
          },
      });
      seq.Wait();

      seq.Add({
        .mName = "RemoveMergedRods",
        .mDuration = 1.0f,
        .mEase = EaseType::Linear,
        .mLerp =
          [=](float t) {
            for (const auto& info: mergedRodInfos) {
              auto& transform = info.mObject.Get<Comp::Transform>();
              Vec3 rodEnd = info.mVertexPosition - (1.0f - t) * info.mRodSpan;
              Vec3 rodCenter = (info.mVertexPosition + rodEnd) / 2.0f;
              transform.SetTranslation(rodCenter);
              Vec3 currentRodSpan = rodEnd - info.mVertexPosition;
              transform.SetScale({Math::Magnitude(currentRodSpan), 0.5f, 0.5f});
            }
          },
        .mEnd =
          [=](Sequence::Cross dir) {
            for (const auto& info: mergedRodInfos) {
              auto& mesh = info.mObject.Get<Comp::Mesh>();
              if (dir == Sequence::Cross::In) {
                mesh.mVisible = true;
                mesh.mMaterialId = "QuickHull/asset:MergedRodColor";
              }
              else {
                mesh.mVisible = false;
                mesh.mMaterialId = "QuickHull/asset:RodColor";
              }
            }
            Rsl::GetRes<Gfx::Material>("QuickHull/asset:MergedRodColor")
              .Get<Vec4>("uColor") = smMergedRodColor;
          },
      });
      seq.Wait();
    }
    // !Animation //////////////////////////////////////////////////////////////

    for (const Ds::List<Vertex>::Iter& vertIt: mergedVerts) {
      hull.mVertices.Erase(vertIt);
    }
    for (const Ds::List<HalfEdge>::Iter& edgeIt: mergedEdges) {
      hull.mHalfEdges.Erase(edgeIt);
    }

    // Distribute orphaned conflict points to the new conflict lists. We ignore
    // any conflict lists that have no conflict points.
    for (const Vec3& point: conflictPoints) {
      if (!assignConflictPoint(point, newFaceConflictLists)) {
        removedPoints.Push(point);
      }
    }
    for (auto& newFaceConflictListIt: newFaceConflictLists) {
      ConflictList& newFaceConflistList = newFaceConflictListIt.mValue;
      if (newFaceConflistList.mPoints.Size() > 0) {
        faceConflictLists.Insert(
          newFaceConflictListIt.Key(), std::move(newFaceConflistList));
      }
    }

    // Animation ///////////////////////////////////////////////////////////////
    seq.Add({
      .mName = "HighlightRemovedVertices",
      .mDuration = 1.0f,
      .mEase = EaseType::Linear,
      .mBegin =
        [=](Sequence::Cross dir) {
          for (const Vec3& removedPoint: removedPoints) {
            auto& mesh =
              vertexSpheres.Find(removedPoint)->mValue.Get<Comp::Mesh>();
            if (dir == Sequence::Cross::In) {
              mesh.mMaterialId = "QuickHull/asset:RemovedVertexColor";
            }
            else {
              mesh.mMaterialId = "QuickHull/asset:VertexColor";
            }
          }
        },
      .mLerp =
        [=](float t) {
          Rsl::GetRes<Gfx::Material>("QuickHull/asset:RemovedVertexColor")
            .Get<Vec4>("uColor") = Lerp(smVertexColor, smRemovedVertexColor, t);
        },
    });
    seq.Wait();

    seq.Add({
      .mName = "RemoveRemovedVertexSpheres",
      .mDuration = 1.0f,
      .mEase = EaseType::Linear,
      .mLerp =
        [=](float t) {
          for (const Vec3& removedPoint: removedPoints) {
            auto& transform =
              vertexSpheres.Find(removedPoint)->mValue.Get<Comp::Transform>();
            transform.SetUniformScale((1.0f - t) * vertexSphereScale);
          }
        },
      .mEnd =
        [=](Sequence::Cross dir) {
          for (const Vec3& removedPoint: removedPoints) {
            auto& mesh =
              vertexSpheres.Find(removedPoint)->mValue.Get<Comp::Mesh>();
            if (dir == Sequence::Cross::In) {
              mesh.mVisible = true;
              mesh.mMaterialId = "QuickHull/asset:RemovedVertexColor";
            }
            else {
              mesh.mVisible = false;
              mesh.mMaterialId = "QuickHull/asset:VertexColor";
            }
          }
          Rsl::GetRes<Gfx::Material>("QuickHull/asset:RemovedVertexColor")
            .Get<Vec4>("uColor") = smRemovedVertexColor;
        },
    });
    seq.Wait();
    // !Animation //////////////////////////////////////////////////////////////
  }

  // Animation /////////////////////////////////////////////////////////////////
  seq.Add({
    .mName = "PulseBloomRemainingElements",
    .mDuration = 1.0f,
    .mEase = EaseType::Linear,
    .mBegin =
      [=](Sequence::Cross dir) {
        for (const auto& edgeRodInfo: edgeRodInfos) {
          auto& mesh = edgeRodInfo.mValue.mObject.Get<Comp::Mesh>();
          if (dir == Sequence::Cross::In) {
            mesh.mMaterialId = "QuickHull/asset:PulseColor";
          }
          else {
            mesh.mMaterialId = "QuickHull/asset:RodColor";
          }
        }
      },
    .mLerp =
      [=](float t) {
        Rsl::GetRes<Gfx::Material>("QuickHull/asset:PulseColor")
          .Get<Vec4>("uColor") = Math::Lerp(smRodColor, smPulseColor, t);
      },
  });
  seq.Wait();

  seq.Add({
    .mName = "VanishRemainingElements",
    .mDuration = 1.0f,
    .mEase = EaseType::Linear,
    .mLerp =
      [=](float t) {
        Rsl::GetRes<Gfx::Material>("QuickHull/asset:PulseColor")
          .Get<Vec4>("uColor") = Math::Lerp(smPulseColor, smVanishColor, t);
      },
    .mEnd =
      [=](Sequence::Cross dir) {
        for (const auto& edgeRodInfo: edgeRodInfos) {
          auto& mesh = edgeRodInfo.mValue.mObject.Get<Comp::Mesh>();
          if (dir == Sequence::Cross::In) {
            mesh.mVisible = true;
            mesh.mMaterialId = "QuickHull/asset:PulseColor";
          }
          else {
            mesh.mVisible = false;
            mesh.mMaterialId = "QuickHull/asset:RodColor";
          }
        }
      },
  });
  seq.Wait();
  // !Animation ////////////////////////////////////////////////////////////////

  return Result();
}

Result QuickHull(Video* video) {
  video->mLayerIt = World::nLayers.EmplaceBack("QuickHull");
  World::Space& space = video->mLayerIt->mSpace;
  World::Object camera = space.CreateObject();
  Comp::Camera& cameraComp = camera.Add<Comp::Camera>();
  cameraComp.mProjectionType = Comp::Camera::ProjectionType::Perspective;
  cameraComp.mFov = 95.0f;
  cameraComp.mHeight = 10.0f;
  Comp::Transform& cameraTransform = camera.Get<Comp::Transform>();
  cameraTransform.SetTranslation({0.0f, 0.0f, 10.0f});
  cameraComp.LocalLookAt({0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, camera);
  video->mLayerIt->mCameraId = camera.mMemberId;

  // Collect the point clouds that we'll animate over.
  Ds::Vector<Ds::Vector<Vec3>> pointClouds;
  Ds::Vector<Vec3> points;
  float heights[4] = {-1, -0.5f, 0.5f, 1};
  for (int i = 0; i < 4; ++i) {
    points.Push({1, heights[i], -1});
    points.Push({1, heights[i], -0.5f});
    points.Push({1, heights[i], 0.5f});
    points.Push({1, heights[i], 1});
    points.Push({0.5f, heights[i], 1});
    points.Push({-0.5f, heights[i], 1});
    points.Push({-1, heights[i], 1});
    points.Push({-1, heights[i], 0.5f});
    points.Push({-1, heights[i], -0.5f});
    points.Push({-1, heights[i], -1});
    points.Push({-0.5f, heights[i], -1});
    points.Push({0.5f, heights[i], -1});
  }
  pointClouds.Emplace(points);

  auto addMeshPoints = [&pointClouds](const char* meshFile, float scale) {
    VResult<Gfx::Mesh::Local> result = Gfx::Mesh::Local::Init(
      meshFile, Gfx::Mesh::Attribute::Position, false, scale);
    LogAbortIf(!result.Success(), result.mError.c_str());
    pointClouds.Emplace(result.mValue.Points());
  };
  addMeshPoints("QuickHull/icepick.obj", 1.0f);
  addMeshPoints("QuickHull/suzanne.obj", 1.0f);

  // Create the animation sequences for the collected point clouds.
  Hull::CreateResources();
  for (const Ds::Vector<Vec3>& points: pointClouds) {
    Result result = Hull::QuickHull(points, video);
    if (!result.Success()) {
      return result;
    }
  }
  return Result();
}
