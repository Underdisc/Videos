#include <comp/Camera.h>
#include <comp/Model.h>
#include <comp/Text.h>
#include <comp/Transform.h>
#include <gfx/LineModel.h>
#include <gfx/Model.h>
#include <math/Constants.h>
#include <math/Utility.h>
#include <world/World.h>

#include "Sequence.h"

#include <iostream>

struct Line
{
  float mThickness;
  Vec3 mStart;
  Vec3 mEnd;

  void VInit(const World::Object& owner)
  {
    mThickness = 0.2f;
    mStart = {0.0f, 0.0f, 0.0f};
    mEnd = {0.0f, 0.0f, 0.0f};

    owner.GetComponent<Comp::Model>().mModelId = AssLib::nCubeModelId;
  }

  void UpdateTransform(const World::Object& owner)
  {
    Comp::Transform& transform = owner.GetComponent<Comp::Transform>();
    Vec3 direction = mEnd - mStart;
    float directionMag = Math::Magnitude(direction);
    if (Math::Near(directionMag, 0.0f)) {
      transform.SetUniformScale(0.0f);
      return;
    }
    Math::Quaternion rotation;
    rotation.FromTo({1.0f, 0.0f, 0.0f}, direction / directionMag);
    transform.SetRotation(rotation);
    transform.SetScale(
      {directionMag / 2.0f, mThickness / 2.0f, mThickness / 2.0f});
    transform.SetTranslation(mStart + direction / 2.0f);
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

  void Hide(const World::Object& owner)
  {
    owner.GetComponent<Comp::Model>().mVisible = false;
  }
};

struct Bracket
{
  Vec3 mCenter;
  // The extent to the right side of the bracket from the center.
  Vec3 mExtent;
  float mFill;
  AssetId mModelId;
  World::MemberId mLeftChild;
  World::MemberId mRightChild;

  void VInit(const World::Object& owner)
  {
    mCenter = {0.0f, 0.0f, 0.0f};
    mExtent = {1.0f, 0.0f, 0.0f};
    mFill = 0.0f;
    mModelId = AssLib::CreateEmpty<Gfx::Model>("Bracket");

    World::Object leftChild = owner.CreateChild();
    World::Object rightChild = owner.CreateChild();
    mLeftChild = leftChild.mMemberId;
    mRightChild = rightChild.mMemberId;

    auto& rightModel = rightChild.AddComponent<Comp::Model>();
    rightModel.mModelId = mModelId;
    rightModel.mShaderId = AssLib::nColorShaderId;
    auto& leftModel = leftChild.AddComponent<Comp::Model>();
    leftModel.mModelId = mModelId;
    leftModel.mShaderId = AssLib::nColorShaderId;

    Math::Quaternion leftRotation;
    leftRotation.AngleAxis(Math::nPi, {0.0f, 1.0f, 0.0f});
    leftChild.GetComponent<Comp::Transform>().SetRotation(leftRotation);

    rightChild.AddComponent<Comp::AlphaColor>().mAlphaColor = {
      1.0f, 1.0f, 1.0f, 1.0f};
    leftChild.AddComponent<Comp::AlphaColor>().mAlphaColor = {
      1.0f, 1.0f, 1.0f, 1.0f};
  }

  void ChangeColor(const World::Object& owner, const Vec4& color)
  {
    owner.mSpace->GetComponent<Comp::AlphaColor>(mLeftChild).mAlphaColor =
      color;
    owner.mSpace->GetComponent<Comp::AlphaColor>(mRightChild).mAlphaColor =
      color;
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

    Math::Quaternion extentRotation;
    Vec3 normalExtent = Math::Normalize(mExtent);
    extentRotation.FromTo({1.0f, 0.0f, 0.0f}, normalExtent);
    auto& transform = owner.GetComponent<Comp::Transform>();
    transform.SetRotation(extentRotation);
    transform.SetTranslation(mCenter);
  }

  void Hide(const World::Object& owner)
  {
    owner.mSpace->GetComponent<Comp::Model>(mLeftChild).mVisible = false;
    owner.mSpace->GetComponent<Comp::Model>(mRightChild).mVisible = false;
  }
};

struct Box
{
  Vec3 mCenter;
  float mWidth;
  float mHeight;
  float mFill;

  void VInit(const World::Object& owner)
  {
    mCenter = {0.0f, 0.0f, 0.0f};
    mHeight = 2.0f;
    mWidth = 4.0f;

    owner.GetComponent<Comp::Model>().mModelId =
      AssLib::CreateEmpty<Gfx::Model>("Box");
  }

  void UpdateRepresentation(const World::Object& owner)
  {
    float halfHeight = mHeight / 2.0f;
    float halfWidth = mWidth / 2.0f;
    float thickness = 0.2f;

    Ds::Vector<Vec3> points;
    points.Push({-halfWidth - thickness / 2.0f, halfHeight, 0.0f});
    points.Push({halfWidth, halfHeight, 0.0f});
    points.Push({halfWidth, -halfHeight, 0.0f});
    points.Push({-halfWidth, -halfHeight, 0.0f});
    points.Push({-halfWidth, halfHeight - thickness / 2.0f, 0.0f});

    Gfx::InitLineModel(
      owner.GetComponent<Comp::Model>().mModelId,
      points,
      mFill,
      thickness,
      Gfx::TerminalType::Flat,
      Gfx::TerminalType::Flat);

    owner.GetComponent<Comp::Transform>().SetTranslation(mCenter);
  }

  void Hide(const World::Object& owner)
  {
    owner.GetComponent<Comp::Model>().mVisible = false;
  }
};

struct Arrow
{
  Vec3 mStart;
  Vec3 mEnd;
  float mFill;

  void VInit(const World::Object& owner)
  {
    owner.GetComponent<Comp::Model>().mModelId =
      AssLib::CreateEmpty<Gfx::Model>("Arrow");
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
    Gfx::InitLineModel(
      owner.GetComponent<Comp::Model>().mModelId,
      points,
      mFill,
      0.2f,
      Gfx::TerminalType::CollapsedBinormal,
      Gfx::TerminalType::Arrow);
  }

  void Hide(const World::Object& owner)
  {
    owner.GetComponent<Comp::Model>().mVisible = false;
  }
};

struct Table
{
  Vec3 mCenter;
  float mCellWidth;
  float mCellHeight;
  unsigned int mCount;
  float mFill;
  float mThickness;

  void VInit(const World::Object& owner)
  {
    mCenter = {0.0f, 0.0f, 0.0f};
    mCellWidth = 2.0f;
    mCellHeight = 1.0f;
    mCount = 1;
    mFill = 0.0f;
    mThickness = 0.2f;

    UpdateRep(owner);
  }

  void UpdateRep(const World::Object& owner)
  {
    owner.GetComponent<Comp::Transform>().SetTranslation(mCenter);

    // Make sure the parent has the correct number of children.
    int requiredChildren = 4 + (mCount - 1);
    for (int i = (int)owner.Children().Size(); i < requiredChildren; ++i) {
      World::Object child = owner.CreateChild();
      child.AddComponent<Line>();
    }
    for (int i = (int)owner.Children().Size() - 1; i >= requiredChildren; ++i) {
      owner.mSpace->DeleteMember(owner.Children()[i]);
    }

    float height = mCellHeight * mCount - (mCount - 1) * mThickness;
    float halfHeight = height / 2.0f;
    float halfWidth = mCellWidth / 2.0f;

    // Horizontal Lines.
    float heightDiff = mCellHeight - mThickness;
    float currentOffset = halfHeight - mThickness / 2.0f;
    for (int i = 0; i < (int)mCount + 1; ++i) {
      auto& line = owner.mSpace->GetComponent<Line>(owner.Children()[i]);
      line.mStart = {mFill * halfWidth, currentOffset, 0.0f};
      line.mEnd = {-mFill * halfWidth, currentOffset, 0.0f};
      line.mThickness = mThickness;
      World::Object childOwner(owner.mSpace, owner.Children()[i]);
      line.UpdateTransform(childOwner);
      currentOffset -= heightDiff;
    }

    // Vertical Lines.
    float widthDiff = mCellWidth - mThickness;
    currentOffset = halfWidth - mThickness / 2.0f;
    for (int i = 0; i < 2; ++i) {
      int index = (int)mCount + 1 + i;
      auto& line = owner.mSpace->GetComponent<Line>(owner.Children()[index]);
      line.mStart = {currentOffset, mFill * halfHeight, 0.0f};
      line.mEnd = {currentOffset, -mFill * halfHeight, 0.0f};
      line.mThickness = mThickness;
      World::Object childOwner(owner.mSpace, owner.Children()[index]);
      line.UpdateTransform(childOwner);
      currentOffset -= widthDiff;
    }
  }

  void Hide(const World::Object& owner)
  {
    for (int i = 0; i < (int)owner.Children().Size(); ++i) {
      owner.mSpace->GetComponent<Comp::Model>(owner.Children()[i]).mVisible =
        false;
    }
  }
};

void VertexDescription(Sequence* sequence)
{
  Sequence& seq = *sequence;
  World::SpaceIt spaceIt = World::CreateTopSpace();
  World::Object camera = spaceIt->CreateObject();
  Comp::Camera& cameraComp = camera.AddComponent<Comp::Camera>();
  cameraComp.mProjectionType = Comp::Camera::ProjectionType::Orthographic;
  cameraComp.mHeight = 10.0f;
  Comp::Transform& cameraTransform = camera.GetComponent<Comp::Transform>();
  cameraTransform.SetTranslation({0.0f, 0.0f, 10.0f});
  cameraComp.LocalLookAt({0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, camera);
  spaceIt->mCameraId = camera.mMemberId;

  World::Object vertexLines[8];
  for (int i = 0; i < 8; ++i) {
    vertexLines[i] = spaceIt->CreateObject();
    auto& line = vertexLines[i].AddComponent<Line>();
    line.mEnd = {0.0f, 0.0f, 0.0f};
    line.mStart = {0.0f, 0.0f, 0.0f};
    line.UpdateTransform(vertexLines[i]);

    vertexLines[i].GetComponent<Comp::Model>().mShaderId =
      AssLib::nColorShaderId;
    vertexLines[i].AddComponent<Comp::AlphaColor>().mAlphaColor = {
      1.0f, 1.0f, 1.0f, 1.0f};
  }
  Sequence::AddOptions ao;
  ao.mName = "ExpandVertexLines";
  ao.mDuration = 1.0f;
  ao.mEase = EaseType::QuadIn;
  seq.Add(ao, [=](float t) {
    float angleIncrement = Math::nPi / 4.0f;
    float angle = 0.0f;
    for (int i = 0; i < 8; ++i) {
      float cosAngle = std::cosf(angle);
      float sinAngle = std::sinf(angle);
      float endDistance = Interpolate(4.0f, 1.5f, t);
      float startDistance = 4.0f;
      Vec3 start = {cosAngle * startDistance, sinAngle * startDistance, 0.0f};
      Vec3 end = {cosAngle * endDistance, sinAngle * endDistance, 0.0f};

      Line& line = vertexLines[i].GetComponent<Line>();
      line.SetStart(start, vertexLines[i]);
      line.SetEnd(end, vertexLines[i]);
      angle += angleIncrement;
    }
  });
  seq.Wait();

  ao.mName = "ShrinkVertexLines";
  ao.mDuration = 1.0f;
  ao.mEase = EaseType::QuadOut;
  seq.Add(ao, [=](float t) {
    float angleIncrement = Math::nPi / 4.0f;
    float angle = 0.0f;
    for (int i = 0; i < 8; ++i) {
      float cosAngle = std::cosf(angle);
      float sinAngle = std::sinf(angle);
      float endDistance = 1.5f;
      float startDistance = Interpolate(4.0f, 1.5f, t);
      Vec3 start = {cosAngle * startDistance, sinAngle * startDistance, 0.0f};
      Vec3 end = {cosAngle * endDistance, sinAngle * endDistance, 0.0f};

      Line& line = vertexLines[i].GetComponent<Line>();
      line.SetStart(start, vertexLines[i]);
      line.SetEnd(end, vertexLines[i]);
      angle += angleIncrement;
    }
  });

  World::Object vertexSphere = spaceIt->CreateObject();
  {
    vertexSphere.AddComponent<Comp::Model>().mModelId = AssLib::nSphereModelId;
    vertexSphere.GetComponent<Comp::Transform>().SetUniformScale(0.0f);
  }
  ao.mName = "ExpandVertexSphere";
  ao.mDuration = 1.0f;
  ao.mEase = EaseType::QuadIn;
  seq.Add(ao, [=](float t) {
    vertexSphere.GetComponent<Comp::Transform>().SetUniformScale(t);
  });
  seq.Wait();
  seq.Gap(0.5f);

  World::Object vertexBracket = spaceIt->CreateChildObject(vertexSphere);
  {
    auto& bracket = vertexBracket.AddComponent<Bracket>();
    bracket.mFill = 0.0f;
    bracket.mCenter = {0.0f, 1.5f, 0.0f};
    bracket.UpdateRepresentation(vertexBracket);
  }
  ao.mName = "ExpandVertexBracket";
  ao.mDuration = 0.75;
  ao.mEase = EaseType::QuadIn;
  seq.Add(ao, [=](float t) {
    auto& bracket = vertexBracket.GetComponent<Bracket>();
    bracket.mFill = -t;
    bracket.UpdateRepresentation(vertexBracket);
  });
  seq.Wait();

  AssetId fugazId = AssLib::CreateEmpty<Gfx::Font>("FugazOne");
  AssLib::Asset<Gfx::Font>& font = AssLib::GetAsset<Gfx::Font>(fugazId);
  font.mPaths.Push("font/fugazOne/font.ttf");
  font.FullInit();

  World::Object vertexLabel = vertexSphere.CreateChild();
  {
    auto& text = vertexLabel.AddComponent<Comp::Text>();
    text.mAlign = Comp::Text::Alignment::Center;
    text.mFillAmount = 0.0f;
    text.mText = "Vertex";
    text.mFontId = fugazId;
    auto& transform = vertexLabel.GetComponent<Comp::Transform>();
    transform.SetTranslation({0.0f, 2.0f, 0.0f});
    transform.SetUniformScale(0.7f);
    vertexLabel.AddComponent<Comp::AlphaColor>().mAlphaColor = {
      1.0f, 1.0f, 1.0f, 1.0f};
  }
  ao.mName = "ShowVertexLabel";
  ao.mDuration = 0.5f;
  ao.mEase = EaseType::QuadIn;
  seq.Add(ao, [=](float t) {
    vertexLabel.GetComponent<Comp::Text>().mFillAmount = t;
  });
  seq.Wait();
  seq.Gap(0.5f);

  Vec3 finalVertexSpherePosition = {-5.5f, 0.0f, 0.0f};
  ao.mName = "MoveVertexSphere";
  ao.mDuration = 1.0f;
  ao.mEase = EaseType::QuadOutIn;
  seq.Add(ao, [=](float t) {
    vertexSphere.GetComponent<Comp::Transform>().SetTranslation(
      Interpolate<Vec3>({0.0f, 0.0f, 0.0f}, finalVertexSpherePosition, t));
  });
  seq.Wait();

  World::Object attributeBoxes[3];
  Vec3 boxCenters[3];
  boxCenters[0] = {0.0f, 2.0f, 0.0f};
  boxCenters[1] = {0.0f, 0.0f, 0.0f};
  boxCenters[2] = {0.0f, -2.0f, 0.0f};
  for (int i = 0; i < 3; ++i) {
    attributeBoxes[i] = spaceIt->CreateObject();
    auto& box = attributeBoxes[i].AddComponent<Box>();
    box.mWidth = 5.0f;
    box.mHeight = 1.25f;
    box.mFill = 0.0f;
    box.mCenter = boxCenters[i];
    box.UpdateRepresentation(attributeBoxes[i]);
    attributeBoxes[i].AddComponent<Comp::AlphaColor>().mAlphaColor = {
      0.0f, 1.0f, 0.0f, 1.0f};
    attributeBoxes[i].GetComponent<Comp::Model>().mShaderId =
      AssLib::nColorShaderId;
    seq.Gap(0.15f);
    ao.mName = "ShowAttributeBox";
    ao.mDuration = 1.0f;
    ao.mEase = EaseType::QuadOutIn;
    seq.Add(ao, [=](float t) {
      auto& box = attributeBoxes[i].GetComponent<Box>();
      box.mFill = t;
      box.UpdateRepresentation(attributeBoxes[i]);
    });
  }
  seq.Wait();

  World::Object attributeConnectors[3];
  Vec3 connectorEnds[3];
  connectorEnds[0] = {-2.5f, 2.0f, -0.1f};
  connectorEnds[1] = {-2.5f, 0.0f, -0.1f};
  connectorEnds[2] = {-2.5f, -2.0f, -0.1f};
  for (int i = 0; i < 3; ++i) {
    attributeConnectors[i] = spaceIt->CreateObject();
    auto& line = attributeConnectors[i].AddComponent<Line>();
    line.mStart = finalVertexSpherePosition;
    line.mEnd = finalVertexSpherePosition;
    line.mThickness = 0.14f;
    line.UpdateTransform(attributeConnectors[i]);
    attributeConnectors[i].GetComponent<Comp::Model>().mShaderId =
      AssLib::nColorShaderId;
    attributeConnectors[i].AddComponent<Comp::AlphaColor>().mAlphaColor = {
      1.0f, 1.0f, 1.0f, 1.0f};
    seq.Gap(0.1f);
    ao.mName = "ExpandConnector";
    ao.mDuration = 1.0f;
    ao.mEase = EaseType::QuadOutIn;
    seq.Add(ao, [=](float t) {
      attributeConnectors[i].GetComponent<Line>().SetEnd(
        Interpolate<Vec3>(finalVertexSpherePosition, connectorEnds[i], t),
        attributeConnectors[i]);
    });
  }
  seq.Wait();

  World::Object attributeLabel = spaceIt->CreateObject();
  {
    auto& text = attributeLabel.AddComponent<Comp::Text>();
    text.mFontId = fugazId;
    text.mFillAmount = 0.0f;
    text.mText = "Attributes";
    text.mAlign = Comp::Text::Alignment::Center;
    auto& transform = attributeLabel.GetComponent<Comp::Transform>();
    transform.SetTranslation({6.5f, -0.3f, 0.0f});
    transform.SetUniformScale(0.7f);
    attributeLabel.AddComponent<Comp::AlphaColor>().mAlphaColor = {
      1.0f, 1.0f, 1.0f, 1.0f};
  }
  ao.mName = "ShowAttributeLabel";
  ao.mDuration = 1.0f;
  ao.mEase = EaseType::QuadIn;
  seq.Add(ao, [=](float t) {
    attributeLabel.GetComponent<Comp::Text>().mFillAmount = t;
  });
  seq.Wait();

  World::Object attributeArrows[3];
  Vec3 arrowStarts[3];
  Vec3 arrowEnds[3];
  arrowStarts[0] = {4.1f, 0.5f, 0.0f};
  arrowStarts[1] = {3.8f, 0.0f, 0.0f};
  arrowStarts[2] = {4.1f, -0.5f, 0.0f};
  arrowEnds[0] = {3.2f, 1.5f, 0.0f};
  arrowEnds[1] = {3.2f, 0.0f, 0.0f};
  arrowEnds[2] = {3.2f, -1.5f, 0.0f};
  for (int i = 0; i < 3; ++i) {
    attributeArrows[i] = spaceIt->CreateObject();
    auto& arrow = attributeArrows[i].AddComponent<Arrow>();
    arrow.mFill = 0.0f;
    arrow.mStart = arrowStarts[i];
    arrow.mEnd = arrowEnds[i];
    attributeArrows[i].AddComponent<Comp::AlphaColor>().mAlphaColor = {
      1.0f, 1.0f, 1.0f, 1.0f};
    attributeArrows[i].GetComponent<Comp::Model>().mShaderId =
      AssLib::nColorShaderId;
    seq.Gap(0.1f);
    ao.mName = "ShowAttributeArrow";
    ao.mDuration = 1.0f;
    ao.mEase = EaseType::QuadIn;
    seq.Add(ao, [=](float t) {
      auto& arrow = attributeArrows[i].GetComponent<Arrow>();
      arrow.mFill = t;
      arrow.UpdateRep(attributeArrows[i]);
    });
  }
  seq.Wait();

  World::Object attributeFlashes[3];
  for (int i = 0; i < 3; ++i) {
    attributeFlashes[i] = spaceIt->CreateObject();
    Vec3 flashCenter = boxCenters[i];
    flashCenter[2] = -1.0f;
    auto& transform = attributeFlashes[i].AddComponent<Comp::Transform>();
    transform.SetTranslation(flashCenter);
    transform.SetScale({4.75f / 2.0f, 1.0f / 2.0f, 0.1f});
    auto& model = attributeFlashes[i].AddComponent<Comp::Model>();
    model.mModelId = AssLib::nCubeModelId;
    model.mShaderId = AssLib::nColorShaderId;
    attributeFlashes[i].AddComponent<Comp::AlphaColor>().mAlphaColor = {
      1.0f, 1.0f, 1.0f, 0.0f};
    seq.Gap(0.25f);
    ao.mName = "FlashAttribute";
    ao.mDuration = 1.5f;
    ao.mEase = EaseType::QuadOut;
    seq.Add(ao, [=](float t) {
      t = 1.0f - t;
      attributeFlashes[i].GetComponent<Comp::AlphaColor>().mAlphaColor[3] = t;
    });
  }
  seq.Wait();
  seq.Gap(1.0f);

  World::Object positionLabel = spaceIt->CreateChildObject(attributeBoxes[0]);
  {
    auto& text = positionLabel.AddComponent<Comp::Text>();
    text.mText = "Position";
    text.mFontId = fugazId;
    text.mFillAmount = 0.0f;
    text.mAlign = Comp::Text::Alignment::Center;
    auto& transform = positionLabel.GetComponent<Comp::Transform>();
    transform.SetTranslation({0.0f, -0.35f, 0.0f});
    transform.SetUniformScale(0.7f);
    positionLabel.AddComponent<Comp::AlphaColor>().mAlphaColor = {
      1.0f, 1.0f, 1.0f, 1.0f};
  }
  ao.mName = "ShowPosition";
  ao.mDuration = 0.5f;
  ao.mEase = EaseType::QuadIn;
  seq.Add(ao, [=](float t) {
    positionLabel.GetComponent<Comp::Text>().mFillAmount = t;
  });
  seq.Wait();
  seq.Gap(1.0f);

  AssetId heartId = AssLib::CreateEmpty<Gfx::Model>("Heart");
  AssLib::Asset<Gfx::Model>& heartAsset = AssLib::GetAsset<Gfx::Model>(heartId);
  heartAsset.mPaths.Push("model/heart.obj");
  heartAsset.FullInit();

  World::Object loveAttributes[2];
  for (int i = 0; i < 2; ++i) {
    loveAttributes[i] = spaceIt->CreateObject();
    auto& model = loveAttributes[i].AddComponent<Comp::Model>();
    model.mModelId = heartId;
    model.mShaderId = AssLib::nColorShaderId;
    loveAttributes[i].AddComponent<Comp::AlphaColor>().mAlphaColor = {
      1.0f, 1.0f, 1.0f, 1.0f};
    Vec3 translation = boxCenters[i + 1];
    translation[2] = 1.0f;
    auto& transform = loveAttributes[i].GetComponent<Comp::Transform>();
    transform.SetTranslation(translation);
    transform.SetUniformScale(0.0f);
    seq.Gap(0.25f);
    ao.mName = "ExpressLove";
    ao.mDuration = 0.5f;
    ao.mEase = EaseType::QuadIn;
    seq.Add(ao, [=](float t) {
      loveAttributes[i].GetComponent<Comp::Transform>().SetUniformScale(
        t * 0.9f);
    });
  }
  seq.Wait();
  seq.Gap(0.5f);

  ao.mName = "FadeAllExceptPosition";
  ao.mDuration = 1.0f;
  ao.mEase = EaseType::QuadIn;
  seq.Add(ao, [=](float t) {
    float newAlpha = 1.0f - t;
    for (int i = 0; i < 2; ++i) {
      attributeConnectors[i + 1]
        .GetComponent<Comp::AlphaColor>()
        .mAlphaColor[3] = newAlpha;
      attributeBoxes[i + 1].GetComponent<Comp::AlphaColor>().mAlphaColor[3] =
        newAlpha;
      loveAttributes[i].GetComponent<Comp::AlphaColor>().mAlphaColor[3] =
        newAlpha;
    }
    for (int i = 0; i < 3; ++i) {
      attributeArrows[i].GetComponent<Comp::AlphaColor>().mAlphaColor[3] =
        newAlpha;
    }
    attributeLabel.GetComponent<Comp::AlphaColor>().mAlphaColor[3] = newAlpha;
    vertexLabel.GetComponent<Comp::AlphaColor>().mAlphaColor[3] = newAlpha;
    vertexBracket.GetComponent<Bracket>().ChangeColor(
      vertexBracket, {1.0f, 1.0f, 1.0f, newAlpha});
  });
  seq.Wait();

  ao.mName = "HideFadedElements";
  ao.mDuration = 0.0f;
  seq.Add(ao, [=](float t) {
    for (int i = 0; i < 2; ++i) {
      // I will sooon turn this.
      attributeConnectors[i + 1].GetComponent<Line>().Hide(
        attributeConnectors[i + 1]);
      // Into this.
      // attributeConnects[i + 1].Get<Line>.Hide(attributeConnectors[i+1]))
      // Would be cool if I didn't have to pass the object in again.
      // But boom, this would really simplify a lot of code.
      attributeBoxes[i + 1].GetComponent<Box>().Hide(attributeBoxes[i + 1]);
      loveAttributes[i].GetComponent<Comp::Model>().mVisible = false;
    }
    for (int i = 0; i < 3; ++i) {
      attributeArrows[i].GetComponent<Arrow>().Hide(attributeArrows[i]);
    }
    attributeLabel.GetComponent<Comp::Text>().mVisible = false;
    vertexLabel.GetComponent<Comp::Text>().mVisible = false;
    vertexBracket.GetComponent<Bracket>().Hide(vertexBracket);
  });

  ao.mName = "MoveVertexAndPosition";
  ao.mDuration = 1.0f;
  ao.mEase = EaseType::QuadOutIn;
  seq.Add(ao, [=](float t) {
    Vec3 sphereTranslation =
      Interpolate(finalVertexSpherePosition, {-5.5f, 3.5f, 0.0f}, t);
    vertexSphere.GetComponent<Comp::Transform>().SetTranslation(
      sphereTranslation);
    Vec3 boxTranslation = Interpolate(boxCenters[0], {0.0f, 3.5f, 0.0f}, t);
    attributeBoxes[0].GetComponent<Comp::Transform>().SetTranslation(
      boxTranslation);
    auto& line = attributeConnectors[0].GetComponent<Line>();
    line.mStart = sphereTranslation;
    line.mEnd = boxTranslation;
    line.mEnd[0] -= 2.5f;
    line.mEnd[2] = -0.1f;
    line.UpdateTransform(attributeConnectors[0]);
  });
  seq.Wait();

  World::Object positionTable2D = spaceIt->CreateObject();
  {
    auto& table2D = positionTable2D.AddComponent<Table>();
    table2D.mCenter = {1.0f, 1.0f, 0.0f};
    table2D.mCount = 2;
    table2D.mCellWidth = 4.0f;
    table2D.mCellHeight = 1.5f;
  }
  ao.mName = "Show2DPositionTable";
  ao.mDuration = 1.0f;
  ao.mEase = EaseType::QuadIn;
  seq.Add(ao, [=](float t) {
    auto& table2D = positionTable2D.GetComponent<Table>();
    table2D.mFill = t;
    table2D.UpdateRep(positionTable2D);
  });
  seq.Wait();
}

void TheFundamentalsOfGraphics(Sequence* sequence)
{
  VertexDescription(sequence);
}
