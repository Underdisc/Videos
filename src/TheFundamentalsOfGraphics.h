#include <comp/Camera.h>
#include <comp/Model.h>
#include <comp/Text.h>
#include <comp/Transform.h>
#include <gfx/LineModel.h>
#include <gfx/Model.h>
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

struct Bracket
{
  Vec3 mCenter;
  // The extent to the right side of the bracket from the center.
  Vec3 mExtent;
  float mFill;
  AssetId mModelId;

  void VInit(const World::Object& owner)
  {
    mCenter = {0.0f, 0.0f, 0.0f};
    mExtent = {1.0f, 0.0f, 0.0f};
    mFill = 0.0f;
    mModelId = AssLib::CreateEmpty<Gfx::Model>("Bracket");

    World::Object leftChild = owner.CreateChild();
    World::Object rightChild = owner.CreateChild();

    auto& rightModel = rightChild.AddComponent<Comp::Model>();
    rightModel.mModelId = mModelId;
    rightModel.mShaderId = AssLib::nColorShaderId;
    auto& leftModel = leftChild.AddComponent<Comp::Model>();
    leftModel.mModelId = mModelId;
    leftModel.mShaderId = AssLib::nColorShaderId;

    auto* leftTransform = leftChild.GetComponent<Comp::Transform>();
    Math::Quaternion leftRotation;
    leftRotation.AngleAxis(Math::nPi, {0.0f, 1.0f, 0.0f});
    leftTransform->SetRotation(leftRotation);

    auto& rightColor = rightChild.AddComponent<Comp::AlphaColor>();
    rightColor.mAlphaColor = {1.0f, 1.0f, 1.0f, 1.0f};
    auto& leftColor = leftChild.AddComponent<Comp::AlphaColor>();
    leftColor.mAlphaColor = {1.0f, 1.0f, 1.0f, 1.0f};
  }

  void UpdateRepresentation(const World::Object& owner)
  {
    Ds::Vector<Vec3> points;
    points.Push({0.0f, 0.3f, 0.0f});
    points.Push({0.0f, 0.0f, 0.0f});
    float extentLength = Math::Magnitude(mExtent);
    points.Push({extentLength, 0.0f, 0.0f});
    points.Push({extentLength, -0.4f, 0.0f});

    Gfx::InitLineModel(
      mModelId,
      points,
      mFill,
      0.25f,
      Gfx::TerminalType::CollapsedNormal,
      Gfx::TerminalType::CollapsedNormal);

    auto* transform = owner.GetComponent<Comp::Transform>();
    Math::Quaternion extentRotation;
    Vec3 normalExtent = Math::Normalize(mExtent);
    extentRotation.FromTo({1.0f, 0.0f, 0.0f}, normalExtent);
    transform->SetRotation(extentRotation);
    transform->SetTranslation(mCenter);
  }
};

struct VertexDescription
{
  World::SpaceIt mSpaceIt;
  World::MemberId mCameraId;
  World::MemberId mVertexLines[8];
  World::MemberId mVertexSphere;
  World::MemberId mVertexBracket;
  World::MemberId mVertexLabel;

  AssetId mFontId;
  VertexDescription(EventSequence* eventSequence);
};

VertexDescription::VertexDescription(EventSequence* seq):
  mSpaceIt(World::CreateTopSpace())
{
  mFontId = AssLib::CreateEmpty<Gfx::Font>("FugazOne");
  AssLib::Asset<Gfx::Font>& font = AssLib::GetAsset<Gfx::Font>(mFontId);
  font.mPaths.Push("font/fugazOne/font.ttf");
  font.FullInit();

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
      auto* model = mSpaceIt->GetComponent<Comp::Model>(mVertexLines[i]);
      model->mShaderId = AssLib::nColorShaderId;
      auto& colorComp =
        mSpaceIt->AddComponent<Comp::AlphaColor>(mVertexLines[i]);
      colorComp.mAlphaColor = {1.0f, 1.0f, 1.0f, 1.0f};
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
    "ShrinkVertexLines", 0.0f, 1.0f, EaseType::QuadOut, [this](float t) {
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
  seq->AddEvent("CreateVertexBracketAndLabel", 1.5f, [this](float t) {
    mVertexBracket = mSpaceIt->CreateChildMember(mVertexSphere);
    auto& bracket = mSpaceIt->AddComponent<Bracket>(mVertexBracket);
    bracket.mFill = 0.0f;
    bracket.mCenter = {0.0f, 1.5f, 0.0f};
    mVertexLabel = mSpaceIt->CreateChildMember(mVertexSphere);
    auto& text = mSpaceIt->AddComponent<Comp::Text>(mVertexLabel);
    text.mAlign = Comp::Text::Alignment::Center;
    text.mFillAmount = 0.0f;
    text.mText = "Vertex";
    text.mFontId = mFontId;
    auto* transform = mSpaceIt->GetComponent<Comp::Transform>(mVertexLabel);
    transform->SetTranslation({0.0f, 2.0f, 0.0f});
    transform->SetUniformScale(0.5f);
  });
  seq->AddEvent(
    "ExpandVertexBracket", 0.0f, 0.75f, EaseType::QuadIn, [this](float t) {
      auto& bracket = *mSpaceIt->GetComponent<Bracket>(mVertexBracket);
      bracket.mFill = -t;
      World::Object bracketOwner(&(*mSpaceIt), mVertexBracket);
      bracket.UpdateRepresentation(bracketOwner);
    });
  seq->AddEvent(
    "ShowVertexLabel", 0.75f, 0.5f, EaseType::QuadIn, [this](float t) {
      auto* text = mSpaceIt->GetComponent<Comp::Text>(mVertexLabel);
      text->mFillAmount = t;
    });
  seq->AddEvent(
    "MoveVertexSphere", 1.0f, 1.0f, EaseType::QuadOutIn, [this](float t) {
      auto* transform = mSpaceIt->GetComponent<Comp::Transform>(mVertexSphere);
      transform->SetTranslation(
        Interpolate<Vec3>({0.0f, 0.0f, 0.0f}, {-7.0f, 0.0f, 0.0}, t));
    });
  seq->AddEvent("DeleteVertexLines", 0.0f, [this](float t) {
    for (int i = 0; i < 8; ++i) {
      mSpaceIt->DeleteMember(mVertexLines[i]);
    }
  });
}

struct TheFundamentalsOfGraphics: EventSequence
{
  // The order of this does matter.
  VertexDescription mVertexDescription;

  TheFundamentalsOfGraphics(): mVertexDescription(this) {};
};
