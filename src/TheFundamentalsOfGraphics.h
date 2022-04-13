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

    auto* leftTransform = leftChild.GetComponent<Comp::Transform>();
    Math::Quaternion leftRotation;
    leftRotation.AngleAxis(Math::nPi, {0.0f, 1.0f, 0.0f});
    leftTransform->SetRotation(leftRotation);

    auto& rightColor = rightChild.AddComponent<Comp::AlphaColor>();
    rightColor.mAlphaColor = {1.0f, 1.0f, 1.0f, 1.0f};
    auto& leftColor = leftChild.AddComponent<Comp::AlphaColor>();
    leftColor.mAlphaColor = {1.0f, 1.0f, 1.0f, 1.0f};
  }

  void ChangeColor(const World::Object& owner, const Vec4& color)
  {
    auto* colorComp = owner.mSpace->GetComponent<Comp::AlphaColor>(mLeftChild);
    colorComp->mAlphaColor = color;
    colorComp = owner.mSpace->GetComponent<Comp::AlphaColor>(mRightChild);
    colorComp->mAlphaColor = color;
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

  void VInit(const World::Object& owner)
  {
    mCenter = {0.0f, 0.0f, 0.0f};
    mHeight = 2.0f;
    mWidth = 4.0f;

    auto* model = owner.GetComponent<Comp::Model>();
    model->mModelId = AssLib::CreateEmpty<Gfx::Model>("Box");
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

    auto* model = owner.GetComponent<Comp::Model>();
    Gfx::InitLineModel(
      model->mModelId,
      points,
      mFill,
      thickness,
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

void VertexDescription(EventSequence* sequence)
{
  EventSequence& seq = *sequence;
  World::SpaceIt spaceIt = World::CreateTopSpace();
  World::MemberId cameraId = spaceIt->CreateMember();
  Comp::Camera& camera = spaceIt->AddComponent<Comp::Camera>(cameraId);
  camera.mProjectionType = Comp::Camera::ProjectionType::Orthographic;
  camera.mHeight = 10.0f;
  Comp::Transform& cameraTransform =
    *spaceIt->GetComponent<Comp::Transform>(cameraId);
  cameraTransform.SetTranslation({0.0f, 0.0f, 10.0f});
  World::Object cameraObject(&(*spaceIt), cameraId);
  camera.LocalLookAt({0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, cameraObject);
  spaceIt->mCameraId = cameraId;

  World::MemberId vertexLines[8];
  for (int i = 0; i < 8; ++i) {
    vertexLines[i] = spaceIt->CreateMember();
    auto& line = spaceIt->AddComponent<Line>(vertexLines[i]);
    line.mEnd = {0.0f, 0.0f, 0.0f};
    line.mStart = {0.0f, 0.0f, 0.0f};
    World::Object lineOwner(&(*spaceIt), vertexLines[i]);
    line.UpdateTransform(lineOwner);
    auto* model = spaceIt->GetComponent<Comp::Model>(vertexLines[i]);
    model->mShaderId = AssLib::nColorShaderId;
    auto& colorComp = spaceIt->AddComponent<Comp::AlphaColor>(vertexLines[i]);
    colorComp.mAlphaColor = {1.0f, 1.0f, 1.0f, 1.0f};
  }
  EventSequence::Options eo;
  eo.mName = "ExpandVertexLines";
  eo.mDuration = 1.0f;
  eo.mEase = EaseType::QuadIn;
  seq.Add(eo, [=](float t) {
    float angleIncrement = Math::nPi / 4.0f;
    float angle = 0.0f;
    for (int i = 0; i < 8; ++i) {
      float cosAngle = std::cosf(angle);
      float sinAngle = std::sinf(angle);
      float endDistance = Interpolate(4.0f, 1.5f, t);
      float startDistance = 4.0f;
      Vec3 start = {cosAngle * startDistance, sinAngle * startDistance, 0.0f};
      Vec3 end = {cosAngle * endDistance, sinAngle * endDistance, 0.0f};

      World::Object owner(&(*spaceIt), vertexLines[i]);
      Line& line = *owner.GetComponent<Line>();
      line.SetStart(start, owner);
      line.SetEnd(end, owner);
      angle += angleIncrement;
    }
  });
  seq.Wait();

  eo.mName = "ShrinkVertexLines";
  eo.mDuration = 1.0f;
  eo.mEase = EaseType::QuadOut;
  seq.Add(eo, [=](float t) {
    float angleIncrement = Math::nPi / 4.0f;
    float angle = 0.0f;
    for (int i = 0; i < 8; ++i) {
      float cosAngle = std::cosf(angle);
      float sinAngle = std::sinf(angle);
      float endDistance = 1.5f;
      float startDistance = Interpolate(4.0f, 1.5f, t);
      Vec3 start = {cosAngle * startDistance, sinAngle * startDistance, 0.0f};
      Vec3 end = {cosAngle * endDistance, sinAngle * endDistance, 0.0f};

      World::Object owner(&(*spaceIt), vertexLines[i]);
      Line& line = *owner.GetComponent<Line>();
      line.SetStart(start, owner);
      line.SetEnd(end, owner);
      angle += angleIncrement;
    }
  });

  World::MemberId vertexSphere = spaceIt->CreateMember();
  {
    auto& model = spaceIt->AddComponent<Comp::Model>(vertexSphere);
    model.mModelId = AssLib::nSphereModelId;
    auto* transform = spaceIt->GetComponent<Comp::Transform>(vertexSphere);
    transform->SetUniformScale(0.0f);
  }
  eo.mName = "ExpandVertexSphere";
  eo.mDuration = 1.0f;
  eo.mEase = EaseType::QuadIn;
  seq.Add(eo, [=](float t) {
    auto& transform = *spaceIt->GetComponent<Comp::Transform>(vertexSphere);
    transform.SetUniformScale(t);
  });
  seq.Wait();
  seq.Gap(0.5f);

  World::MemberId vertexBracket = spaceIt->CreateChildMember(vertexSphere);
  {
    auto& bracket = spaceIt->AddComponent<Bracket>(vertexBracket);
    bracket.mFill = 0.0f;
    bracket.mCenter = {0.0f, 1.5f, 0.0f};
    World::Object bracketOwner(&(*spaceIt), vertexBracket);
    bracket.UpdateRepresentation(bracketOwner);
  }
  eo.mName = "ExpandVertexBracket";
  eo.mDuration = 0.75;
  eo.mEase = EaseType::QuadIn;
  seq.Add(eo, [=](float t) {
    auto& bracket = *spaceIt->GetComponent<Bracket>(vertexBracket);
    bracket.mFill = -t;
    World::Object bracketOwner(&(*spaceIt), vertexBracket);
    bracket.UpdateRepresentation(bracketOwner);
  });
  seq.Wait();

  AssetId fugazId = AssLib::CreateEmpty<Gfx::Font>("FugazOne");
  AssLib::Asset<Gfx::Font>& font = AssLib::GetAsset<Gfx::Font>(fugazId);
  font.mPaths.Push("font/fugazOne/font.ttf");
  font.FullInit();
  World::MemberId vertexLabel = spaceIt->CreateChildMember(vertexSphere);
  {
    auto& text = spaceIt->AddComponent<Comp::Text>(vertexLabel);
    text.mAlign = Comp::Text::Alignment::Center;
    text.mFillAmount = 0.0f;
    text.mText = "Vertex";
    text.mFontId = fugazId;
    auto* transform = spaceIt->GetComponent<Comp::Transform>(vertexLabel);
    transform->SetTranslation({0.0f, 2.0f, 0.0f});
    transform->SetUniformScale(0.7f);
    auto& color = spaceIt->AddComponent<Comp::AlphaColor>(vertexLabel);
    color.mAlphaColor = {1.0f, 1.0f, 1.0f, 1.0f};
  }
  eo.mName = "ShowVertexLabel";
  eo.mDuration = 0.5f;
  eo.mEase = EaseType::QuadIn;
  seq.Add(eo, [=](float t) {
    auto* text = spaceIt->GetComponent<Comp::Text>(vertexLabel);
    text->mFillAmount = t;
  });
  seq.Wait();
  seq.Gap(0.5f);

  Vec3 finalVertexSpherePosition = {-5.5f, 0.0f, 0.0f};
  eo.mName = "MoveVertexSphere";
  eo.mDuration = 1.0f;
  eo.mEase = EaseType::QuadOutIn;
  seq.Add(eo, [=](float t) {
    auto* transform = spaceIt->GetComponent<Comp::Transform>(vertexSphere);
    transform->SetTranslation(
      Interpolate<Vec3>({0.0f, 0.0f, 0.0f}, finalVertexSpherePosition, t));
  });
  seq.Wait();

  World::MemberId attributeBoxes[3];
  Vec3 boxCenters[3];
  boxCenters[0] = {0.0f, 2.0f, 0.0f};
  boxCenters[1] = {0.0f, 0.0f, 0.0f};
  boxCenters[2] = {0.0f, -2.0f, 0.0f};
  for (int i = 0; i < 3; ++i) {
    attributeBoxes[i] = spaceIt->CreateMember();
    auto& box = spaceIt->AddComponent<Box>(attributeBoxes[i]);
    box.mWidth = 5.0f;
    box.mHeight = 1.25f;
    box.mFill = 0.0f;
    box.mCenter = boxCenters[i];
    World::Object boxOwner(&(*spaceIt), attributeBoxes[i]);
    box.UpdateRepresentation(boxOwner);
    auto& color = spaceIt->AddComponent<Comp::AlphaColor>(attributeBoxes[i]);
    color.mAlphaColor = {0.0f, 1.0f, 0.0f, 1.0f};
    auto* model = spaceIt->GetComponent<Comp::Model>(attributeBoxes[i]);
    model->mShaderId = AssLib::nColorShaderId;
    seq.Gap(0.15f);
    eo.mName = "ShowAttributeBox";
    eo.mDuration = 1.0f;
    eo.mEase = EaseType::QuadOutIn;
    seq.Add(eo, [=](float t) {
      auto* box = spaceIt->GetComponent<Box>(attributeBoxes[i]);
      World::Object boxOwner(&(*spaceIt), attributeBoxes[i]);
      box->mFill = t;
      box->UpdateRepresentation(boxOwner);
    });
  }
  seq.Wait();

  World::MemberId attributeConnectors[3];
  Vec3 connectorEnds[3];
  connectorEnds[0] = {-2.5f, 2.0f, -0.1f};
  connectorEnds[1] = {-2.5f, 0.0f, -0.1f};
  connectorEnds[2] = {-2.5f, -2.0f, -0.1f};
  for (int i = 0; i < 3; ++i) {
    attributeConnectors[i] = spaceIt->CreateMember();
    auto& line = spaceIt->AddComponent<Line>(attributeConnectors[i]);
    line.mStart = finalVertexSpherePosition;
    line.mEnd = finalVertexSpherePosition;
    line.mThickness = 0.07f;
    World::Object lineOwner(&(*spaceIt), attributeConnectors[i]);
    line.UpdateTransform(lineOwner);
    auto* lineModel =
      spaceIt->GetComponent<Comp::Model>(attributeConnectors[i]);
    lineModel->mShaderId = AssLib::nColorShaderId;
    auto& colorComp =
      spaceIt->AddComponent<Comp::AlphaColor>(attributeConnectors[i]);
    colorComp.mAlphaColor = {1.0f, 1.0f, 1.0f, 1.0f};
    seq.Gap(0.1f);
    eo.mName = "ExpandConnector";
    eo.mDuration = 1.0f;
    eo.mEase = EaseType::QuadOutIn;
    seq.Add(eo, [=](float t) {
      auto* lineComp = spaceIt->GetComponent<Line>(attributeConnectors[i]);
      World::Object lineOwner(&(*spaceIt), attributeConnectors[i]);
      lineComp->SetEnd(
        Interpolate<Vec3>(finalVertexSpherePosition, connectorEnds[i], t),
        lineOwner);
    });
  }
  seq.Wait();

  World::MemberId attributeLabel = spaceIt->CreateMember();
  {
    auto& text = spaceIt->AddComponent<Comp::Text>(attributeLabel);
    text.mFontId = fugazId;
    text.mFillAmount = 0.0f;
    text.mText = "Attributes";
    text.mAlign = Comp::Text::Alignment::Center;
    auto* transform = spaceIt->GetComponent<Comp::Transform>(attributeLabel);
    transform->SetTranslation({6.5f, -0.3f, 0.0f});
    transform->SetUniformScale(0.7f);
    auto& color = spaceIt->AddComponent<Comp::AlphaColor>(attributeLabel);
    color.mAlphaColor = {1.0f, 1.0f, 1.0f, 1.0f};
  }
  eo.mName = "ShowAttributeLabel";
  eo.mDuration = 1.0f;
  eo.mEase = EaseType::QuadIn;
  seq.Add(eo, [=](float t) {
    auto* text = spaceIt->GetComponent<Comp::Text>(attributeLabel);
    text->mFillAmount = t;
  });
  seq.Wait();

  World::MemberId attributeArrows[3];
  Vec3 arrowStarts[3];
  Vec3 arrowEnds[3];
  arrowStarts[0] = {4.1f, 0.5f, 0.0f};
  arrowStarts[1] = {3.8f, 0.0f, 0.0f};
  arrowStarts[2] = {4.1f, -0.5f, 0.0f};
  arrowEnds[0] = {3.2f, 1.5f, 0.0f};
  arrowEnds[1] = {3.2f, 0.0f, 0.0f};
  arrowEnds[2] = {3.2f, -1.5f, 0.0f};
  for (int i = 0; i < 3; ++i) {
    attributeArrows[i] = spaceIt->CreateMember();
    auto& arrow = spaceIt->AddComponent<Arrow>(attributeArrows[i]);
    arrow.mFill = 0.0f;
    arrow.mStart = arrowStarts[i];
    arrow.mEnd = arrowEnds[i];
    auto& color = spaceIt->AddComponent<Comp::AlphaColor>(attributeArrows[i]);
    color.mAlphaColor = {1.0f, 1.0f, 1.0f, 1.0f};
    auto* model = spaceIt->GetComponent<Comp::Model>(attributeArrows[i]);
    model->mShaderId = AssLib::nColorShaderId;
    seq.Gap(0.1f);
    eo.mName = "ShowAttributeArrow";
    eo.mDuration = 1.0f;
    eo.mEase = EaseType::QuadIn;
    seq.Add(eo, [=](float t) {
      auto* arrow = spaceIt->GetComponent<Arrow>(attributeArrows[i]);
      arrow->mFill = t;
      World::Object arrowOwner(&(*spaceIt), attributeArrows[i]);
      arrow->UpdateRep(arrowOwner);
    });
  }
  seq.Wait();

  World::MemberId attributeFlashes[3];
  for (int i = 0; i < 3; ++i) {
    attributeFlashes[i] = spaceIt->CreateMember();
    auto& transform =
      spaceIt->AddComponent<Comp::Transform>(attributeFlashes[i]);
    Vec3 flashCenter = boxCenters[i];
    flashCenter[2] = -1.0f;
    transform.SetTranslation(flashCenter);
    transform.SetScale({4.75f / 2.0f, 1.0f / 2.0f, 0.1f});
    auto& model = spaceIt->AddComponent<Comp::Model>(attributeFlashes[i]);
    model.mModelId = AssLib::nCubeModelId;
    model.mShaderId = AssLib::nColorShaderId;
    auto& color = spaceIt->AddComponent<Comp::AlphaColor>(attributeFlashes[i]);
    color.mAlphaColor = {1.0f, 1.0f, 1.0f, 0.0f};
    seq.Gap(0.25f);
    eo.mName = "FlashAttribute";
    eo.mDuration = 1.5f;
    eo.mEase = EaseType::QuadOut;
    seq.Add(eo, [=](float t) {
      t = 1.0f - t;
      auto* color =
        spaceIt->GetComponent<Comp::AlphaColor>(attributeFlashes[i]);
      color->mAlphaColor[3] = t;
    });
  }
  seq.Wait();
  seq.Gap(1.0f);

  World::MemberId positionLabel = spaceIt->CreateChildMember(attributeBoxes[0]);
  {
    auto& text = spaceIt->AddComponent<Comp::Text>(positionLabel);
    text.mText = "Position";
    text.mFontId = fugazId;
    text.mFillAmount = 0.0f;
    text.mAlign = Comp::Text::Alignment::Center;
    auto* transform = spaceIt->GetComponent<Comp::Transform>(positionLabel);
    transform->SetTranslation({0.0f, -0.35f, 0.0f});
    transform->SetUniformScale(0.7f);
    auto& color = spaceIt->AddComponent<Comp::AlphaColor>(positionLabel);
    color.mAlphaColor = {1.0f, 1.0f, 1.0f, 1.0f};
  }
  eo.mName = "ShowPosition";
  eo.mDuration = 0.5f;
  eo.mEase = EaseType::QuadIn;
  seq.Add(eo, [=](float t) {
    auto* text = spaceIt->GetComponent<Comp::Text>(positionLabel);
    text->mFillAmount = t;
  });
  seq.Wait();
  seq.Gap(1.0f);

  AssetId heartId = AssLib::CreateEmpty<Gfx::Model>("Heart");
  AssLib::Asset<Gfx::Model>& heartAsset = AssLib::GetAsset<Gfx::Model>(heartId);
  heartAsset.mPaths.Push("model/heart.obj");
  heartAsset.FullInit();
  World::MemberId loveAttributes[2];
  for (int i = 0; i < 2; ++i) {
    loveAttributes[i] = spaceIt->CreateMember();
    auto& model = spaceIt->AddComponent<Comp::Model>(loveAttributes[i]);
    model.mModelId = heartId;
    model.mShaderId = AssLib::nColorShaderId;
    auto& color = spaceIt->AddComponent<Comp::AlphaColor>(loveAttributes[i]);
    color.mAlphaColor = {1.0f, 1.0f, 1.0f, 1.0f};
    auto* transform = spaceIt->GetComponent<Comp::Transform>(loveAttributes[i]);
    Vec3 translation = boxCenters[i + 1];
    translation[2] = 1.0f;
    transform->SetTranslation(translation);
    transform->SetUniformScale(0.0f);
    seq.Gap(0.25f);
    eo.mName = "ExpressLove";
    eo.mDuration = 0.5f;
    eo.mEase = EaseType::QuadIn;
    seq.Add(eo, [=](float t) {
      auto* transform =
        spaceIt->GetComponent<Comp::Transform>(loveAttributes[i]);
      transform->SetUniformScale(t * 0.9f);
    });
  }
  seq.Wait();
  seq.Gap(0.5f);

  eo.mName = "HideAllExceptPosition";
  eo.mDuration = 1.0f;
  eo.mEase = EaseType::QuadIn;
  seq.Add(eo, [=](float t) {
    float newAlpha = 1.0f - t;
    for (int i = 0; i < 2; ++i) {
      auto* color =
        spaceIt->GetComponent<Comp::AlphaColor>(attributeConnectors[i + 1]);
      color->mAlphaColor[3] = newAlpha;
      color = spaceIt->GetComponent<Comp::AlphaColor>(attributeBoxes[i + 1]);
      color->mAlphaColor[3] = newAlpha;
      color = spaceIt->GetComponent<Comp::AlphaColor>(loveAttributes[i]);
      color->mAlphaColor[3] = newAlpha;
    }
    for (int i = 0; i < 3; ++i) {
      auto* color = spaceIt->GetComponent<Comp::AlphaColor>(attributeArrows[i]);
      color->mAlphaColor[3] = newAlpha;
    }
    auto* color = spaceIt->GetComponent<Comp::AlphaColor>(attributeLabel);
    color->mAlphaColor[3] = newAlpha;
    color = spaceIt->GetComponent<Comp::AlphaColor>(vertexLabel);
    color->mAlphaColor[3] = newAlpha;
    auto* bracket = spaceIt->GetComponent<Bracket>(vertexBracket);
    World::Object bracketOwner(&(*spaceIt), vertexBracket);
    bracket->ChangeColor(bracketOwner, {1.0f, 1.0f, 1.0f, newAlpha});
  });
  seq.Wait();

  eo.mName = "MoveVertexAndPosition";
  eo.mDuration = 1.0f;
  eo.mEase = EaseType::QuadOutIn;
  seq.Add(eo, [=](float t) {
    auto* transform = spaceIt->GetComponent<Comp::Transform>(vertexSphere);
    Vec3 sphereTranslation =
      Interpolate<Vec3>(finalVertexSpherePosition, {-5.5f, 3.5f, 0.0f}, t);
    transform->SetTranslation(sphereTranslation);

    transform = spaceIt->GetComponent<Comp::Transform>(attributeBoxes[0]);
    Vec3 boxTranslation =
      Interpolate<Vec3>(boxCenters[0], {0.0f, 3.5f, 0.0f}, t);
    transform->SetTranslation(boxTranslation);

    World::Object lineOwner(&(*spaceIt), attributeConnectors[0]);
    auto* line = lineOwner.GetComponent<Line>();
    line->mStart = sphereTranslation;
    line->mEnd = boxTranslation;
    line->mEnd[0] -= 2.5f;
    line->mEnd[2] = -0.1f;
    line->UpdateTransform(lineOwner);
  });

  // I am really starting to think that all elements should be created at the
  // start of a group. It does not mean you need to create all of the elements
  // for an entire video, but you do need to create all of the elements for
  // one part of the video.
  // seq.Add("DeleteVertexLines", 0.0f, [this](float t) {
  //   for (int i = 0; i < 8; ++i) {
  //     mSpaceIt->DeleteMember(vertexLines[i]);
  //   }
  // });
}

void TheFundamentalsOfGraphics(EventSequence* sequence)
{
  VertexDescription(sequence);
}
