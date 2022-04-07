#include <comp/Camera.h>
#include <comp/Model.h>
#include <comp/Transform.h>
#include <math/Constants.h>
#include <math/Utility.h>
#include <world/World.h>

#include "EventSequence.h"

#include <iostream>

struct Line
{
  float mThickness;
  Vec3 mStart;
  Vec3 mEnd;

  void VInit(const World::Object& owner)
  {
    mThickness = 0.1f;
    mStart = {-1.0f, 0.0f, 0.0f};
    mEnd = {1.0f, 0.0f, 0.0f};

    Comp::Model* model = owner.GetComponent<Comp::Model>();
    model->mModelId = AssLib::nCubeModelId;
  }

  void UpdateTransform(const World::Object& owner)
  {
    Comp::Transform* transform = owner.GetComponent<Comp::Transform>();
    Vec3 direction = mEnd - mStart;
    float directionMag = Math::Magnitude(direction);
    Math::Quaternion rotation;
    rotation.FromTo({1.0f, 0.0f, 0.0f}, direction / directionMag);
    transform->SetRotation(rotation);
    transform->SetScale({directionMag / 2.0f, mThickness, mThickness});
    transform->SetTranslation(mStart + direction / 2.0f);
  }

  void SetStart(const Vec3& start, const World::Object& owner)
  {
    mStart = start;
    UpdateTransform(owner);
  }

  void SetEnd(const Vec3& end, const World::Object& owner)
  {
    mEnd = end;
    UpdateTransform(owner);
  }
};

struct VertexDescription
{
  World::SpaceIt mSpaceIt;
  World::MemberId mCameraId;
  World::MemberId mVertexLines[8];
  World::MemberId mVertexSphere;
  World::MemberId mVertexLabel;
  VertexDescription(EventSequence* eventSequence);
};

VertexDescription::VertexDescription(EventSequence* seq):
  mSpaceIt(World::CreateTopSpace())
{
  mCameraId = mSpaceIt->CreateMember();
  Comp::Camera& camera = mSpaceIt->AddComponent<Comp::Camera>(mCameraId);
  camera.mProjectionType = Comp::Camera::ProjectionType::Orthographic;
  camera.mHeight = 10.0f;
  Comp::Transform& cameraTransform =
    *mSpaceIt->GetComponent<Comp::Transform>(mCameraId);
  cameraTransform.SetTranslation({0.0f, 0.0f, 10.0f});
  World::Object cameraObject(&(*mSpaceIt), mCameraId);
  camera.LocalLookAt({0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, cameraObject);
  mSpaceIt->mCameraId = mCameraId;

  seq->AddEvent("CreateVertexLines", 0.0f, [this](float t) {
    for (int i = 0; i < 8; ++i) {
      mVertexLines[i] = mSpaceIt->CreateMember();
      mSpaceIt->AddComponent<Line>(mVertexLines[i]);
    }
  });
  seq->AddEvent(
    "ExpandVertexLines", 0.0f, 1.0f, EaseType::QuadIn, [this](float t) {
      float angleIncrement = Math::nPi / 4.0f;
      float angle = 0.0f;
      for (int i = 0; i < 8; ++i) {
        float cosAngle = std::cosf(angle);
        float sinAngle = std::sinf(angle);
        float endDistance = Interpolate(4.0f, 1.5f, t);
        float startDistance = 4.0f;
        Vec3 start = {cosAngle * startDistance, sinAngle * startDistance, 0.0f};
        Vec3 end = {cosAngle * endDistance, sinAngle * endDistance, 0.0f};

        World::Object owner(&(*mSpaceIt), mVertexLines[i]);
        Line& line = *owner.GetComponent<Line>();
        line.SetStart(start, owner);
        line.SetEnd(end, owner);
        angle += angleIncrement;
      }
    });
  seq->AddEvent("CreateVertexSphere", 1.0f, [this](float t) {
    mVertexSphere = mSpaceIt->CreateMember();
    auto& model = mSpaceIt->AddComponent<Comp::Model>(mVertexSphere);
    model.mModelId = AssLib::nSphereModelId;
  });
  seq->AddEvent(
    "ExpandVertexSphere", 0.0f, 1.0f, EaseType::QuadIn, [this](float t) {
      auto& transform = *mSpaceIt->GetComponent<Comp::Transform>(mVertexSphere);
      transform.SetUniformScale(t);
    });
  seq->AddEvent(
    "ShrinkVertexLines", 0.0f, 1.0f, EaseType::QuadIn, [this](float t) {
      float angleIncrement = Math::nPi / 4.0f;
      float angle = 0.0f;
      for (int i = 0; i < 8; ++i) {
        float cosAngle = std::cosf(angle);
        float sinAngle = std::sinf(angle);
        float endDistance = 1.5f;
        float startDistance = Interpolate(4.0f, 1.5f, t);
        Vec3 start = {cosAngle * startDistance, sinAngle * startDistance, 0.0f};
        Vec3 end = {cosAngle * endDistance, sinAngle * endDistance, 0.0f};

        World::Object owner(&(*mSpaceIt), mVertexLines[i]);
        Line& line = *owner.GetComponent<Line>();
        line.SetStart(start, owner);
        line.SetEnd(end, owner);
        angle += angleIncrement;
      }
    });
  seq->AddEvent("DeleteVertexLines", 1.0f, [this](float t) {
    for (int i = 0; i < 8; ++i) {
      mSpaceIt->DeleteMember(mVertexLines[i]);
    }
  });
  seq->AddEvent(
    "ShrinkVertexSphere", 2.0f, 1.0f, EaseType::QuadOut, [this](float t) {
      auto& transform = *mSpaceIt->GetComponent<Comp::Transform>(mVertexSphere);
      transform.SetUniformScale(1.0f - t);
    });
  seq->AddEvent("DeleteVertexSphere", 1.0f, [this](float t) {
    mSpaceIt->DeleteMember(mVertexSphere);
  });
}

struct TheFundamentalsOfGraphics: EventSequence
{
  // The order of this does matter.
  VertexDescription mVertexDescription;

  TheFundamentalsOfGraphics(): mVertexDescription(this) {};
};
