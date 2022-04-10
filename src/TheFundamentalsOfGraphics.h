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

struct Box
{
  Vec3 mCenter;
  float mWidth;
  float mHeight;
  float mFill;
  AssetId mModelId;

  void VInit(const World::Object& owner)
  {
    mCenter = {0.0f, 0.0f, 0.0f};
    mHeight = 2.0f;
    mWidth = 4.0f;
    mModelId = AssLib::CreateEmpty<Gfx::Model>("Box");

    World::Object topChild = owner.CreateChild();
    World::Object bottomChild = owner.CreateChild();

    auto& topModel = topChild.AddComponent<Comp::Model>();
    topModel.mModelId = mModelId;
    auto& bottomModel = bottomChild.AddComponent<Comp::Model>();
    bottomModel.mModelId = mModelId;

    auto* bottomTransform = bottomChild.GetComponent<Comp::Transform>();
    Math::Quaternion bottomRotation;
    bottomRotation.AngleAxis(Math::nPi, {1.0f, 0.0f, 0.0f});
    bottomTransform->SetRotation(bottomRotation);
  }

  void UpdateRepresentation(const World::Object& owner)
  {
    Ds::Vector<Vec3> points;
    float halfHeight = mHeight / 2.0f;
    float halfWidth = mWidth / 2.0f;
    points.Push({-halfWidth, 0.0f, 0.0f});
    points.Push({-halfWidth, halfHeight, 0.0f});
    points.Push({halfWidth, halfHeight, 0.0f});
    points.Push({halfWidth, 0.0f, 0.0f});

    Gfx::InitLineModel(
      mModelId,
      points,
      mFill,
      0.2f,
      Gfx::TerminalType::Flat,
      Gfx::TerminalType::Flat);

    auto* transform = owner.GetComponent<Comp::Transform>();
    transform->SetTranslation(mCenter);
  }
};

struct Arrow
{
  Vec3 mStart;
  Vec3 mEnd;
  float mFill;

  void VInit(const World::Object& owner)
  {
    auto* model = owner.GetComponent<Comp::Model>();
    model->mModelId = AssLib::CreateEmpty<Gfx::Model>("Arrow");
    mStart = {0.0f, 0.0f, 0.0f};
    mEnd = {0.0f, 0.0f, 0.0f};
    mFill = 0.0f;
    UpdateRep(owner);
  }

  void UpdateRep(const World::Object& owner)
  {
    Ds::Vector<Vec3> points;
    points.Push(mStart);
    points.Push(mEnd);
    auto* model = owner.GetComponent<Comp::Model>();

    Gfx::InitLineModel(
      model->mModelId,
      points,
      mFill,
      0.2f,
      Gfx::TerminalType::CollapsedBinormal,
      Gfx::TerminalType::Arrow);
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
  World::MemberId mAttributeConnectors[3];
  World::MemberId mAttributeBoxes[3];
  World::MemberId mAttributeLabel;
  World::MemberId mAttributeArrows[3];

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
  Vec3 finalVertexSpherePosition = {-7.0f, 0.0f, 0.0f};
  seq->AddEvent(
    "MoveVertexSphere",
    1.0f,
    1.0f,
    EaseType::QuadOutIn,
    [this, finalVertexSpherePosition](float t) {
      auto* transform = mSpaceIt->GetComponent<Comp::Transform>(mVertexSphere);
      transform->SetTranslation(
        Interpolate<Vec3>({0.0f, 0.0f, 0.0f}, finalVertexSpherePosition, t));
    });
  Vec3 boxCenters[3];
  boxCenters[0] = {0.0f, 2.0f, 0.0f};
  boxCenters[1] = {0.0f, 0.0f, 0.0f};
  boxCenters[2] = {0.0f, -2.0f, 0.0f};
  seq->AddEvent(
    "CreateAttributeElements",
    1.0f,
    [this, finalVertexSpherePosition, boxCenters](float t) {
      for (int i = 0; i < 3; ++i) {
        mAttributeConnectors[i] = mSpaceIt->CreateMember();
        auto& line = mSpaceIt->AddComponent<Line>(mAttributeConnectors[i]);
        line.mStart = finalVertexSpherePosition;
        line.mEnd = finalVertexSpherePosition;
        line.mThickness = 0.07f;
        World::Object lineOwner(&(*mSpaceIt), mAttributeConnectors[i]);
        line.UpdateTransform(lineOwner);
        auto* lineModel =
          mSpaceIt->GetComponent<Comp::Model>(mAttributeConnectors[i]);
        lineModel->mShaderId = AssLib::nColorShaderId;
        auto& colorComp =
          mSpaceIt->AddComponent<Comp::AlphaColor>(mAttributeConnectors[i]);
        colorComp.mAlphaColor = {1.0f, 1.0f, 1.0f, 1.0f};

        mAttributeBoxes[i] = mSpaceIt->CreateMember();
        auto& box = mSpaceIt->AddComponent<Box>(mAttributeBoxes[i]);
        box.mWidth = 5.0f;
        box.mHeight = 1.25f;
        box.mFill = 0.0f;
        box.mCenter = boxCenters[i];
        World::Object boxOwner(&(*mSpaceIt), mAttributeBoxes[i]);
        box.UpdateRepresentation(boxOwner);
      }
    });
  for (int i = 0; i < 3; ++i) {
    seq->AddEvent(
      "ExpandBoxes", 0.25f, 1.0f, EaseType::QuadOutIn, [this, i](float t) {
        auto* box = mSpaceIt->GetComponent<Box>(mAttributeBoxes[i]);
        World::Object boxOwner(&(*mSpaceIt), mAttributeBoxes[i]);
        box->mFill = t;
        box->UpdateRepresentation(boxOwner);
      });
  }
  Vec3 connectorEnds[3];
  connectorEnds[0] = {-2.5f, 2.0f, -0.1f};
  connectorEnds[1] = {-2.5f, 0.0f, -0.1f};
  connectorEnds[2] = {-2.5f, -2.0f, -0.1f};
  for (int i = 0; i < 3; ++i) {
    seq->AddEvent(
      "ExpandConnectors",
      0.25f,
      1.0f,
      EaseType::QuadOutIn,
      [this, finalVertexSpherePosition, connectorEnds, i](float t) {
        auto* lineComp = mSpaceIt->GetComponent<Line>(mAttributeConnectors[i]);
        World::Object lineOwner(&(*mSpaceIt), mAttributeConnectors[i]);
        lineComp->SetEnd(
          Interpolate<Vec3>(finalVertexSpherePosition, connectorEnds[i], t),
          lineOwner);
      });
  }

  {
    mAttributeLabel = mSpaceIt->CreateMember();
    auto& text = mSpaceIt->AddComponent<Comp::Text>(mAttributeLabel);
    text.mFontId = mFontId;
    text.mColor = {1.0f, 1.0f, 1.0f};
    text.mFillAmount = 0.0f;
    text.mText = "Attributes";
    text.mAlign = Comp::Text::Alignment::Center;
    auto* transform = mSpaceIt->GetComponent<Comp::Transform>(mAttributeLabel);
    transform->SetTranslation({6.5f, -0.3f, 0.0f});
    transform->SetUniformScale(0.6f);
  }
  seq->AddEvent(
    "ShowAttributeLabel", 1.0f, 1.0f, EaseType::QuadIn, [this](float t) {
      auto* text = mSpaceIt->GetComponent<Comp::Text>(mAttributeLabel);
      text->mFillAmount = t;
    });

  Vec3 arrowStarts[3];
  Vec3 arrowEnds[3];
  arrowStarts[0] = {4.3f, 0.5f, 0.0f};
  arrowStarts[1] = {4.1f, 0.0f, 0.0f};
  arrowStarts[2] = {4.3f, -0.5f, 0.0f};
  arrowEnds[0] = {3.2f, 1.5f, 0.0f};
  arrowEnds[1] = {3.2f, 0.0f, 0.0f};
  arrowEnds[2] = {3.2f, -1.5f, 0.0f};
  for (int i = 0; i < 3; ++i) {
    mAttributeArrows[i] = mSpaceIt->CreateMember();
    auto& arrow = mSpaceIt->AddComponent<Arrow>(mAttributeArrows[i]);
    arrow.mFill = 0.0f;
    arrow.mStart = arrowStarts[i];
    arrow.mEnd = arrowEnds[i];
    auto& color = mSpaceIt->AddComponent<Comp::AlphaColor>(mAttributeArrows[i]);
    color.mAlphaColor = {1.0f, 1.0f, 1.0f, 1.0f};
    auto* model = mSpaceIt->GetComponent<Comp::Model>(mAttributeArrows[i]);
    model->mShaderId = AssLib::nColorShaderId;

    seq->AddEvent(
      "ShowAttributeArrow", 0.25f, 1.0f, EaseType::QuadIn, [this, i](float t) {
        auto* arrow = mSpaceIt->GetComponent<Arrow>(mAttributeArrows[i]);
        arrow->mFill = t;
        World::Object arrowOwner(&(*mSpaceIt), mAttributeArrows[i]);
        arrow->UpdateRep(arrowOwner);
      });
  }

  // I am really starting to think that all elements should be created at the
  // start of a group. It does not mean you need to create all of the elements
  // for an entire video, but you do need to create all of the elements for
  // one part of the video.
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
