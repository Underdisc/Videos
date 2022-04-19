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

    owner.Get<Comp::Model>().mModelId = AssLib::nCubeModelId;
  }

  void UpdateTransform(const World::Object& owner)
  {
    Comp::Transform& transform = owner.Get<Comp::Transform>();
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

  void Show(const World::Object& owner, bool visible)
  {
    owner.Get<Comp::Model>().mVisible = visible;
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
    mModelId = AssLib::CreateEmpty<Gfx::Model>("Bracket");

    World::Object leftChild = owner.CreateChild();
    World::Object rightChild = owner.CreateChild();
    mLeftChild = leftChild.mMemberId;
    mRightChild = rightChild.mMemberId;

    auto& rightModel = rightChild.Add<Comp::Model>();
    rightModel.mModelId = mModelId;
    rightModel.mShaderId = AssLib::nColorShaderId;
    auto& leftModel = leftChild.Add<Comp::Model>();
    leftModel.mModelId = mModelId;
    leftModel.mShaderId = AssLib::nColorShaderId;

    Math::Quaternion leftRotation;
    leftRotation.AngleAxis(Math::nPi, {0.0f, 1.0f, 0.0f});
    leftChild.Get<Comp::Transform>().SetRotation(leftRotation);

    rightChild.Add<Comp::AlphaColor>().mAlphaColor = {1.0f, 1.0f, 1.0f, 1.0f};
    leftChild.Add<Comp::AlphaColor>().mAlphaColor = {1.0f, 1.0f, 1.0f, 1.0f};

    Fill(owner, 0.0f);
  }

  void ChangeColor(const World::Object& owner, const Vec4& color)
  {
    owner.mSpace->Get<Comp::AlphaColor>(mLeftChild).mAlphaColor = color;
    owner.mSpace->Get<Comp::AlphaColor>(mRightChild).mAlphaColor = color;
  }

  void Fill(const World::Object& owner, float fill)
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
      fill,
      0.25f,
      Gfx::TerminalType::CollapsedNormal,
      Gfx::TerminalType::CollapsedNormal);

    Math::Quaternion extentRotation;
    Vec3 normalExtent = Math::Normalize(mExtent);
    extentRotation.FromTo({1.0f, 0.0f, 0.0f}, normalExtent);
    auto& transform = owner.Get<Comp::Transform>();
    transform.SetRotation(extentRotation);
    transform.SetTranslation(mCenter);
  }

  void Show(const World::Object& owner, bool visible)
  {
    owner.mSpace->Get<Comp::Model>(mLeftChild).mVisible = visible;
    owner.mSpace->Get<Comp::Model>(mRightChild).mVisible = visible;
  }
};

struct Box
{
  Vec3 mCenter;
  float mWidth;
  float mHeight;

  void VInit(const World::Object& owner)
  {
    mCenter = {0.0f, 0.0f, 0.0f};
    mHeight = 2.0f;
    mWidth = 4.0f;
    owner.Get<Comp::Model>().mModelId = AssLib::CreateEmpty<Gfx::Model>("Box");
    Fill(owner, 0.0f);
  }

  void Fill(const World::Object& owner, float fill)
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
      owner.Get<Comp::Model>().mModelId,
      points,
      fill,
      thickness,
      Gfx::TerminalType::Flat,
      Gfx::TerminalType::Flat);

    owner.Get<Comp::Transform>().SetTranslation(mCenter);
  }

  void Show(const World::Object& owner, bool visible)
  {
    owner.Get<Comp::Model>().mVisible = visible;
  }
};

struct Arrow
{
  Vec3 mStart;
  Vec3 mEnd;

  void VInit(const World::Object& owner)
  {
    owner.Get<Comp::Model>().mModelId =
      AssLib::CreateEmpty<Gfx::Model>("Arrow");
    mStart = {0.0f, 0.0f, 0.0f};
    mEnd = {0.0f, 0.0f, 0.0f};
    Fill(owner, 0.0f);
  }

  void Fill(const World::Object& owner, float fill)
  {
    Ds::Vector<Vec3> points;
    points.Push(mStart);
    points.Push(mEnd);
    Gfx::InitLineModel(
      owner.Get<Comp::Model>().mModelId,
      points,
      fill,
      0.2f,
      Gfx::TerminalType::CollapsedBinormal,
      Gfx::TerminalType::Arrow);
  }

  void Show(const World::Object& owner, bool visible)
  {
    owner.Get<Comp::Model>().mVisible = visible;
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
    mThickness = 0.2f;
    Fill(owner, 0.0f);
  }

  void Fill(const World::Object& owner, float fill)
  {
    owner.Get<Comp::Transform>().SetTranslation(mCenter);

    // Make sure the parent has the correct number of children.
    int requiredChildren = 4 + (mCount - 1);
    for (int i = (int)owner.Children().Size(); i < requiredChildren; ++i) {
      World::Object child = owner.CreateChild();
      child.Add<Line>();
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
      auto& line = owner.mSpace->Get<Line>(owner.Children()[i]);
      line.mStart = {fill * halfWidth, currentOffset, 0.0f};
      line.mEnd = {-fill * halfWidth, currentOffset, 0.0f};
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
      auto& line = owner.mSpace->Get<Line>(owner.Children()[index]);
      line.mStart = {currentOffset, fill * halfHeight, 0.0f};
      line.mEnd = {currentOffset, -fill * halfHeight, 0.0f};
      line.mThickness = mThickness;
      World::Object childOwner(owner.mSpace, owner.Children()[index]);
      line.UpdateTransform(childOwner);
      currentOffset -= widthDiff;
    }
  }

  void Show(const World::Object& owner, bool visible)
  {
    for (int i = 0; i < (int)owner.Children().Size(); ++i) {
      owner.mSpace->Get<Comp::Model>(owner.Children()[i]).mVisible = visible;
    }
  }
};

void VertexDescription(Sequence* sequence)
{
  Sequence& seq = *sequence;
  World::SpaceIt spaceIt = World::CreateTopSpace();
  World::Object camera = spaceIt->CreateObject();
  Comp::Camera& cameraComp = camera.Add<Comp::Camera>();
  cameraComp.mProjectionType = Comp::Camera::ProjectionType::Orthographic;
  cameraComp.mHeight = 10.0f;
  Comp::Transform& cameraTransform = camera.Get<Comp::Transform>();
  cameraTransform.SetTranslation({0.0f, 0.0f, 10.0f});
  cameraComp.LocalLookAt({0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, camera);
  spaceIt->mCameraId = camera.mMemberId;

  World::Object vertexLines[8];
  for (int i = 0; i < 8; ++i) {
    vertexLines[i] = spaceIt->CreateObject();
    auto& line = vertexLines[i].Add<Line>();
    line.mEnd = {0.0f, 0.0f, 0.0f};
    line.mStart = {0.0f, 0.0f, 0.0f};
    line.UpdateTransform(vertexLines[i]);

    vertexLines[i].Get<Comp::Model>().mShaderId = AssLib::nColorShaderId;
    vertexLines[i].Add<Comp::AlphaColor>().mAlphaColor = {
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

      Line& line = vertexLines[i].Get<Line>();
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

      Line& line = vertexLines[i].Get<Line>();
      line.SetStart(start, vertexLines[i]);
      line.SetEnd(end, vertexLines[i]);
      angle += angleIncrement;
    }
  });

  World::Object vertexSphere = spaceIt->CreateObject();
  {
    vertexSphere.Add<Comp::Model>().mModelId = AssLib::nSphereModelId;
    vertexSphere.Get<Comp::Transform>().SetUniformScale(0.0f);
  }
  ao.mName = "ExpandVertexSphere";
  ao.mDuration = 1.0f;
  ao.mEase = EaseType::QuadIn;
  seq.Add(ao, [=](float t) {
    vertexSphere.Get<Comp::Transform>().SetUniformScale(t);
  });
  seq.Wait();
  seq.Gap(0.5f);

  World::Object vertexBracket = spaceIt->CreateChildObject(vertexSphere);
  {
    auto& bracket = vertexBracket.Add<Bracket>().mCenter = {0.0f, 1.5f, 0.0f};
  }
  ao.mName = "ExpandVertexBracket";
  ao.mDuration = 0.75;
  ao.mEase = EaseType::QuadIn;
  seq.Add(ao, [=](float t) {
    vertexBracket.Get<Bracket>().Fill(vertexBracket, -t);
  });
  seq.Wait();

  AssetId fugazId = AssLib::CreateEmpty<Gfx::Font>("FugazOne");
  AssLib::Asset<Gfx::Font>& font = AssLib::GetAsset<Gfx::Font>(fugazId);
  font.mPaths.Push("font/fugazOne/font.ttf");
  font.FullInit();

  World::Object vertexLabel = vertexSphere.CreateChild();
  {
    auto& text = vertexLabel.Add<Comp::Text>();
    text.mAlign = Comp::Text::Alignment::Center;
    text.mFillAmount = 0.0f;
    text.mText = "Vertex";
    text.mFontId = fugazId;
    auto& transform = vertexLabel.Get<Comp::Transform>();
    transform.SetTranslation({0.0f, 2.0f, 0.0f});
    transform.SetUniformScale(0.7f);
    vertexLabel.Add<Comp::AlphaColor>().mAlphaColor = {1.0f, 1.0f, 1.0f, 1.0f};
  }
  ao.mName = "ShowVertexLabel";
  ao.mDuration = 0.5f;
  ao.mEase = EaseType::QuadIn;
  seq.Add(ao, [=](float t) {
    vertexLabel.Get<Comp::Text>().mFillAmount = t;
  });
  seq.Wait();
  seq.Gap(0.5f);

  Vec3 finalVertexSpherePosition = {-5.5f, 0.0f, 0.0f};
  ao.mName = "MoveVertexSphere";
  ao.mDuration = 1.0f;
  ao.mEase = EaseType::QuadOutIn;
  seq.Add(ao, [=](float t) {
    vertexSphere.Get<Comp::Transform>().SetTranslation(
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
    auto& box = attributeBoxes[i].Add<Box>();
    box.mWidth = 5.0f;
    box.mHeight = 1.25f;
    box.mCenter = boxCenters[i];
    attributeBoxes[i].Add<Comp::AlphaColor>().mAlphaColor = {
      0.0f, 1.0f, 0.0f, 1.0f};
    attributeBoxes[i].Get<Comp::Model>().mShaderId = AssLib::nColorShaderId;

    seq.Gap(0.15f);
    ao.mName = "ShowAttributeBox";
    ao.mDuration = 1.0f;
    ao.mEase = EaseType::QuadOutIn;
    seq.Add(ao, [=](float t) {
      attributeBoxes[i].Get<Box>().Fill(attributeBoxes[i], t);
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
    auto& line = attributeConnectors[i].Add<Line>();
    line.mStart = finalVertexSpherePosition;
    line.mEnd = finalVertexSpherePosition;
    line.mThickness = 0.14f;
    line.UpdateTransform(attributeConnectors[i]);
    attributeConnectors[i].Get<Comp::Model>().mShaderId =
      AssLib::nColorShaderId;
    attributeConnectors[i].Add<Comp::AlphaColor>().mAlphaColor = {
      1.0f, 1.0f, 1.0f, 1.0f};
    seq.Gap(0.1f);
    ao.mName = "ExpandConnector";
    ao.mDuration = 1.0f;
    ao.mEase = EaseType::QuadOutIn;
    seq.Add(ao, [=](float t) {
      attributeConnectors[i].Get<Line>().SetEnd(
        Interpolate<Vec3>(finalVertexSpherePosition, connectorEnds[i], t),
        attributeConnectors[i]);
    });
  }
  seq.Wait();

  World::Object attributeLabel = spaceIt->CreateObject();
  {
    auto& text = attributeLabel.Add<Comp::Text>();
    text.mFontId = fugazId;
    text.mFillAmount = 0.0f;
    text.mText = "Attributes";
    text.mAlign = Comp::Text::Alignment::Center;
    auto& transform = attributeLabel.Get<Comp::Transform>();
    transform.SetTranslation({6.5f, -0.3f, 0.0f});
    transform.SetUniformScale(0.7f);
    attributeLabel.Add<Comp::AlphaColor>().mAlphaColor = {
      1.0f, 1.0f, 1.0f, 1.0f};
  }
  ao.mName = "ShowAttributeLabel";
  ao.mDuration = 1.0f;
  ao.mEase = EaseType::QuadIn;
  seq.Add(ao, [=](float t) {
    attributeLabel.Get<Comp::Text>().mFillAmount = t;
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
    auto& arrow = attributeArrows[i].Add<Arrow>();
    arrow.mStart = arrowStarts[i];
    arrow.mEnd = arrowEnds[i];
    attributeArrows[i].Add<Comp::AlphaColor>().mAlphaColor = {
      1.0f, 1.0f, 1.0f, 1.0f};
    attributeArrows[i].Get<Comp::Model>().mShaderId = AssLib::nColorShaderId;

    seq.Gap(0.1f);
    ao.mName = "ShowAttributeArrow";
    ao.mDuration = 1.0f;
    ao.mEase = EaseType::QuadIn;
    seq.Add(ao, [=](float t) {
      attributeArrows[i].Get<Arrow>().Fill(attributeArrows[i], t);
    });
  }
  seq.Wait();

  World::Object attributeFlashes[3];
  for (int i = 0; i < 3; ++i) {
    attributeFlashes[i] = spaceIt->CreateObject();
    Vec3 flashCenter = boxCenters[i];
    flashCenter[2] = -1.0f;
    auto& transform = attributeFlashes[i].Add<Comp::Transform>();
    transform.SetTranslation(flashCenter);
    transform.SetScale({4.75f / 2.0f, 1.0f / 2.0f, 0.1f});
    auto& model = attributeFlashes[i].Add<Comp::Model>();
    model.mModelId = AssLib::nCubeModelId;
    model.mShaderId = AssLib::nColorShaderId;
    attributeFlashes[i].Add<Comp::AlphaColor>().mAlphaColor = {
      1.0f, 1.0f, 1.0f, 0.0f};
    seq.Gap(0.25f);
    ao.mName = "FlashAttribute";
    ao.mDuration = 1.5f;
    ao.mEase = EaseType::Flash;
    seq.Add(ao, [=](float t) {
      attributeFlashes[i].Get<Comp::AlphaColor>().mAlphaColor[3] = t;
    });
  }
  seq.Wait();
  seq.Gap(1.0f);

  World::Object positionLabel = spaceIt->CreateChildObject(attributeBoxes[0]);
  {
    auto& text = positionLabel.Add<Comp::Text>();
    text.mText = "Position";
    text.mFontId = fugazId;
    text.mFillAmount = 0.0f;
    text.mAlign = Comp::Text::Alignment::Center;
    auto& transform = positionLabel.Get<Comp::Transform>();
    transform.SetTranslation({0.0f, -0.35f, 0.0f});
    transform.SetUniformScale(0.7f);
    positionLabel.Add<Comp::AlphaColor>().mAlphaColor = {
      1.0f, 1.0f, 1.0f, 1.0f};
  }
  ao.mName = "ShowPosition";
  ao.mDuration = 0.5f;
  ao.mEase = EaseType::QuadIn;
  seq.Add(ao, [=](float t) {
    positionLabel.Get<Comp::Text>().mFillAmount = t;
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
    auto& model = loveAttributes[i].Add<Comp::Model>();
    model.mModelId = heartId;
    model.mShaderId = AssLib::nColorShaderId;
    loveAttributes[i].Add<Comp::AlphaColor>().mAlphaColor = {
      1.0f, 1.0f, 1.0f, 1.0f};
    Vec3 translation = boxCenters[i + 1];
    translation[2] = 1.0f;
    auto& transform = loveAttributes[i].Get<Comp::Transform>();
    transform.SetTranslation(translation);
    transform.SetUniformScale(0.0f);
    seq.Gap(0.25f);
    ao.mName = "ExpressLove";
    ao.mDuration = 0.5f;
    ao.mEase = EaseType::QuadIn;
    seq.Add(ao, [=](float t) {
      loveAttributes[i].Get<Comp::Transform>().SetUniformScale(t * 0.9f);
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
      attributeConnectors[i + 1].Get<Comp::AlphaColor>().mAlphaColor[3] =
        newAlpha;
      attributeBoxes[i + 1].Get<Comp::AlphaColor>().mAlphaColor[3] = newAlpha;
      loveAttributes[i].Get<Comp::AlphaColor>().mAlphaColor[3] = newAlpha;
    }
    for (int i = 0; i < 3; ++i) {
      attributeArrows[i].Get<Comp::AlphaColor>().mAlphaColor[3] = newAlpha;
    }
    attributeLabel.Get<Comp::AlphaColor>().mAlphaColor[3] = newAlpha;
    vertexLabel.Get<Comp::AlphaColor>().mAlphaColor[3] = newAlpha;
    vertexBracket.Get<Bracket>().ChangeColor(
      vertexBracket, {1.0f, 1.0f, 1.0f, newAlpha});
  });
  seq.Wait();

  ao.mName = "HideFadedElements";
  ao.mDuration = 0.0f;
  seq.Add(ao, [=](float t) {
    bool visible = !(bool)(int)t;
    for (int i = 0; i < 2; ++i) {
      attributeConnectors[i + 1].Get<Line>().Show(
        attributeConnectors[i + 1], visible);
      attributeBoxes[i + 1].Get<Box>().Show(attributeBoxes[i + 1], visible);
      loveAttributes[i].Get<Comp::Model>().mVisible = visible;
    }
    for (int i = 0; i < 3; ++i) {
      attributeArrows[i].Get<Arrow>().Show(attributeArrows[i], visible);
    }
    attributeLabel.Get<Comp::Text>().mVisible = visible;
    vertexLabel.Get<Comp::Text>().mVisible = visible;
    vertexBracket.Get<Bracket>().Show(vertexBracket, visible);
  });

  ao.mName = "MoveVertexAndPosition";
  ao.mDuration = 1.0f;
  ao.mEase = EaseType::QuadOutIn;
  seq.Add(ao, [=](float t) {
    Vec3 sphereTranslation =
      Interpolate(finalVertexSpherePosition, {-5.5f, 3.5f, 0.0f}, t);
    vertexSphere.Get<Comp::Transform>().SetTranslation(sphereTranslation);
    Vec3 boxTranslation = Interpolate(boxCenters[0], {0.0f, 3.5f, 0.0f}, t);
    attributeBoxes[0].Get<Comp::Transform>().SetTranslation(boxTranslation);
    auto& line = attributeConnectors[0].Get<Line>();
    line.mStart = sphereTranslation;
    line.mEnd = boxTranslation;
    line.mEnd[0] -= 2.5f;
    line.mEnd[2] = -0.1f;
    line.UpdateTransform(attributeConnectors[0]);
  });
  seq.Wait();

  World::Object positionTable2D = spaceIt->CreateObject();
  {
    auto& table2D = positionTable2D.Add<Table>();
    table2D.mCenter = {1.0f, 1.0f, 0.0f};
    table2D.mCount = 2;
    table2D.mCellWidth = 4.0f;
    table2D.mCellHeight = 1.5f;
  }
  ao.mName = "Show2DPositionTable";
  ao.mDuration = 1.0f;
  ao.mEase = EaseType::QuadIn;
  seq.Add(ao, [=](float t) {
    positionTable2D.Get<Table>().Fill(positionTable2D, t);
  });
  seq.Wait();
}

void TheFundamentalsOfGraphics(Sequence* sequence)
{
  VertexDescription(sequence);
}
