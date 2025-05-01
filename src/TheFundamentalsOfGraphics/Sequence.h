#ifndef TheFundamentalsOfGraphics_h
#define TheFundamentalsOfGraphics_h

#include <comp/Camera.h>
#include <comp/LineMesh.h>
#include <comp/Relationship.h>
#include <comp/Text.h>
#include <comp/Transform.h>
#include <editor/gizmos/Gizmos.h>
#include <gfx/Material.h>
#include <gfx/Mesh.h>
#include <math/Constants.h>
#include <math/Utility.h>
#include <rsl/Library.h>
#include <world/World.h>

#include "../Sequence.h"

struct Line
{
  float mThickness;
  Vec3 mStart;
  Vec3 mEnd;

  void VInit(const World::Object& owner)
  {
    static Rsl::Asset& lines = Rsl::AddAsset("Lines");
    static int lineCount = 0;
    std::string resName = std::to_string(lineCount++);
    lines.InitRes<Gfx::Material>(resName, "vres/renderer:Color")
      .Add<Vec4>("uColor") = {1, 1, 1, 1};
    owner.Get<Comp::Mesh>().mMaterialId = ResId("Lines", resName);
    owner.Get<Comp::Mesh>().mMeshId = Editor::Gizmos::nCubeMeshId;

    mThickness = 0.2f;
    mStart = {0.0f, 0.0f, 0.0f};
    mEnd = {0.0f, 0.0f, 0.0f};
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
    Quat rotation = Quat::FromTo({1.0f, 0.0f, 0.0f}, direction / directionMag);
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
    owner.Get<Comp::Mesh>().mVisible = visible;
  }
};

struct Bracket
{
  Vec3 mCenter;
  // The extent to the right side of the bracket from the center.
  Vec3 mExtent;
  ResId mMeshId;
  ResId mMaterialId;
  World::MemberId mLeftChild;
  World::MemberId mRightChild;

  void VInit(const World::Object& owner)
  {
    static Rsl::Asset& brackets = Rsl::AddAsset("Brackets");
    static int bracketCount = 0;
    std::string resName = std::to_string(bracketCount++);
    mMeshId = ResId("Brackets", resName);
    mMaterialId = ResId("Brackets", resName);
    brackets.InitRes<Gfx::Mesh>(resName);
    brackets.InitRes<Gfx::Material>(resName, "vres/renderer:Color")
      .Add<Vec4>("uColor", {1, 1, 1, 1});

    World::Object leftChild = owner.CreateChild();
    World::Object rightChild = owner.CreateChild();
    mLeftChild = leftChild.mMemberId;
    mRightChild = rightChild.mMemberId;

    auto& rightMesh = rightChild.Add<Comp::Mesh>();
    rightMesh.mMeshId = mMeshId;
    rightMesh.mMaterialId = mMaterialId;
    auto& leftModel = leftChild.Add<Comp::Mesh>();
    leftModel.mMeshId = mMeshId;
    leftModel.mMaterialId = mMaterialId;

    Quat leftRotation = Quat::AngleAxis(Math::nPi, {0.0f, 1.0f, 0.0f});
    leftChild.Get<Comp::Transform>().SetRotation(leftRotation);

    mCenter = {0.0f, 0.0f, 0.0f};
    mExtent = {1.0f, 0.0f, 0.0f};
    Fill(owner, 0.0f);
  }

  void ChangeColor(const World::Object& owner, const Vec4& color)
  {
    Rsl::GetRes<Gfx::Material>(mMaterialId).Get<Vec4>("uColor") = color;
  }

  void Fill(const World::Object& owner, float fill)
  {
    Ds::Vector<Vec3> points;
    points.Push({0.0f, 0.3f, 0.0f});
    points.Push({0.0f, 0.0f, 0.0f});
    float extentLength = Math::Magnitude(mExtent);
    points.Push({extentLength, 0.0f, 0.0f});
    points.Push({extentLength, -0.4f, 0.0f});

    InitLineMesh(
      mMeshId,
      points,
      fill,
      0.25f,
      TerminalType::CollapsedNormal,
      TerminalType::CollapsedNormal);

    Vec3 normalExtent = Math::Normalize(mExtent);
    Quat extentRotation = Quat::FromTo({1.0f, 0.0f, 0.0f}, normalExtent);
    auto& transform = owner.Get<Comp::Transform>();
    transform.SetRotation(extentRotation);
    transform.SetTranslation(mCenter);
  }

  void Show(const World::Object& owner, bool visible)
  {
    owner.mSpace->Get<Comp::Mesh>(mLeftChild).mVisible = visible;
    owner.mSpace->Get<Comp::Mesh>(mRightChild).mVisible = visible;
  }

  void Fade(const World::Object& owner, float newAlpha)
  {
    Rsl::GetRes<Gfx::Material>(mMaterialId).Get<Vec4>("uColor")[3] = newAlpha;
  }
};

struct Box
{
  Vec3 mCenter;
  float mWidth;
  float mHeight;

  void VInit(const World::Object& owner)
  {
    static Rsl::Asset& boxes = Rsl::AddAsset("Boxes");
    static int boxCount = 0;
    std::string resName = std::to_string(boxCount++);
    owner.Get<Comp::Mesh>().mMeshId = ResId("Boxes", resName);
    owner.Get<Comp::Mesh>().mMaterialId = ResId("Boxes", resName);
    boxes.InitRes<Gfx::Mesh>(resName);
    boxes.InitRes<Gfx::Material>(resName, "vres/renderer:Color");
    mCenter = {0.0f, 0.0f, 0.0f};
    mHeight = 2.0f;
    mWidth = 4.0f;
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

    InitLineMesh(
      owner.Get<Comp::Mesh>().mMeshId,
      points,
      fill,
      thickness,
      TerminalType::Flat,
      TerminalType::Flat);

    owner.Get<Comp::Transform>().SetTranslation(mCenter);
  }

  void Show(const World::Object& owner, bool visible)
  {
    owner.Get<Comp::Mesh>().mVisible = visible;
  }
};

struct Arrow
{
  Vec3 mStart;
  Vec3 mEnd;

  void VInit(const World::Object& owner)
  {
    static Rsl::Asset& arrows = Rsl::AddAsset("Arrows");
    static int arrowCount = 0;
    std::string resName = std::to_string(arrowCount++);
    owner.Get<Comp::Mesh>().mMeshId = ResId("Arrows", resName);
    owner.Get<Comp::Mesh>().mMaterialId = ResId("Arrows", resName);
    arrows.InitRes<Gfx::Mesh>(resName);
    arrows.InitRes<Gfx::Material>(resName, "vres/renderer:Color");

    mStart = {0.0f, 0.0f, 0.0f};
    mEnd = {0.0f, 0.0f, 0.0f};
    Fill(owner, 0.0f);
  }

  void Fill(const World::Object& owner, float fill)
  {
    Ds::Vector<Vec3> points;
    points.Push(mStart);
    points.Push(mEnd);
    InitLineMesh(
      owner.Get<Comp::Mesh>().mMeshId,
      points,
      fill,
      0.2f,
      TerminalType::CollapsedBinormal,
      TerminalType::Arrow);
  }

  void Show(const World::Object& owner, bool visible)
  {
    owner.Get<Comp::Mesh>().mVisible = visible;
  }
};

struct Table
{
  Vec3 mCenter;
  float mCellWidth;
  float mCellHeight;
  unsigned int mCount;
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

  float Height()
  {
    return mCellHeight * mCount - (mCount - 1) * mThickness;
  }

  float Width()
  {
    return mCellWidth;
  }

  void Fill(const World::Object& owner, float fill)
  {
    owner.Get<Comp::Transform>().SetTranslation(mCenter);

    // Make sure the parent has the correct number of children.
    int requiredChildren = 4 + (mCount - 1);
    size_t childCount = owner.Get<Comp::Relationship>().mChildren.Size();
    for (size_t i = childCount; i < requiredChildren; ++i) {
      owner.CreateChild().Add<Line>();
    }
    childCount = owner.Get<Comp::Relationship>().mChildren.Size();
    for (int i = childCount - 1; i >= requiredChildren; --i) {
      owner.mSpace->DeleteMember(owner.Get<Comp::Relationship>().mChildren[i]);
    }

    float halfHeight = Height() / 2.0f;
    float halfWidth = Width() / 2.0f;

    // Horizontal Lines.
    float heightDiff = mCellHeight - mThickness;
    float currentOffset = halfHeight - mThickness / 2.0f;
    const auto& relationship = owner.Get<Comp::Relationship>();
    for (int i = 0; i < (int)mCount + 1; ++i) {
      auto& line = owner.mSpace->Get<Line>(relationship.mChildren[i]);
      line.mStart = {fill * halfWidth, currentOffset, 0.0f};
      line.mEnd = {-fill * halfWidth, currentOffset, 0.0f};
      line.mThickness = mThickness;
      World::Object childOwner(owner.mSpace, relationship.mChildren[i]);
      line.UpdateTransform(childOwner);
      currentOffset -= heightDiff;
    }

    // Vertical Lines.
    float widthDiff = mCellWidth - mThickness;
    currentOffset = halfWidth - mThickness / 2.0f;
    for (int i = 0; i < 2; ++i) {
      int index = (int)mCount + 1 + i;
      auto& line = owner.mSpace->Get<Line>(relationship.mChildren[index]);
      line.mStart = {currentOffset, fill * halfHeight, 0.0f};
      line.mEnd = {currentOffset, -fill * halfHeight, 0.0f};
      line.mThickness = mThickness;
      World::Object childOwner(owner.mSpace, relationship.mChildren[index]);
      line.UpdateTransform(childOwner);
      currentOffset -= widthDiff;
    }
  }

  void Show(const World::Object& owner, bool visible)
  {
    const auto& relationship = owner.Get<Comp::Relationship>();
    for (int i = 0; i < (int)relationship.mChildren.Size(); ++i) {
      owner.mSpace->Get<Comp::Mesh>(relationship.mChildren[i]).mVisible =
        visible;
    }
  }

  void Fade(const World::Object& owner, float newAlpha)
  {
    const auto& relationship = owner.Get<Comp::Relationship>();
    for (World::MemberId childId : relationship.mChildren) {
      const World::Object& child = World::Object(owner.mSpace, childId);
      Rsl::GetRes<Gfx::Material>(child.Get<Comp::Mesh>().mMaterialId)
        .Get<Vec4>("uColor")[3] = newAlpha;
    }
  }
};

void VertexDescription(Sequence* sequence)
{
  Sequence& seq = *sequence;
  World::LayerIt layerIt =
    World::nLayers.EmplaceBack("TheFundamentalsOfGraphics");
  World::Space& space = layerIt->mSpace;
  World::Object camera = space.CreateObject();
  Comp::Camera& cameraComp = camera.Add<Comp::Camera>();
  cameraComp.mProjectionType = Comp::Camera::ProjectionType::Orthographic;
  cameraComp.mHeight = 10.0f;
  Comp::Transform& cameraTransform = camera.Get<Comp::Transform>();
  cameraTransform.SetTranslation({0.0f, 0.0f, 10.0f});
  cameraComp.LocalLookAt({0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, camera);
  layerIt->mCameraId = camera.mMemberId;

  World::Object vertexLines[8];
  for (int i = 0; i < 8; ++i) {
    vertexLines[i] = space.CreateObject();
    auto& line = vertexLines[i].Add<Line>();
    line.mEnd = {0.0f, 0.0f, 0.0f};
    line.mStart = {0.0f, 0.0f, 0.0f};
    line.UpdateTransform(vertexLines[i]);

    Rsl::GetRes<Gfx::Material>(vertexLines[i].Get<Comp::Mesh>().mMaterialId)
      .Add<Vec4>("uColor") = {1, 1, 1, 1};
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

  World::Object vertexSphere = space.CreateObject();
  vertexSphere.Add<Comp::Mesh>().mMeshId = Editor::Gizmos::nSphereMeshId;
  vertexSphere.Get<Comp::Transform>().SetUniformScale(0.0f);

  ao.mName = "ExpandVertexSphere";
  ao.mDuration = 1.0f;
  ao.mEase = EaseType::QuadIn;
  seq.Add(ao, [=](float t) {
    vertexSphere.Get<Comp::Transform>().SetUniformScale(t);
  });
  seq.Wait();
  seq.Gap(0.5f);

  World::Object vertexBracket = space.CreateChildObject(vertexSphere);
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

  Rsl::Asset& resources = Rsl::AddAsset("TheFundamentalsOfGraphics");
  ResId fugazId = ResId(resources.GetName(), "FugazOne");
  resources.InitRes<Gfx::Font>(
    fugazId.GetResourceName(), "font/fugazOne/font.ttf");
  World::Object vertexLabel = vertexSphere.CreateChild();
  {
    auto& text = vertexLabel.Add<Comp::Text>();
    text.mAlign = Comp::Text::Alignment::Center;
    text.mFillAmount = 0.0f;
    text.mColor = {1, 1, 1, 1};
    text.mText = "Vertex";
    text.mFontId = fugazId;
    auto& transform = vertexLabel.Get<Comp::Transform>();
    transform.SetTranslation({0.0f, 2.0f, 0.0f});
    transform.SetUniformScale(0.7f);
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
    attributeBoxes[i] = space.CreateObject();
    auto& box = attributeBoxes[i].Add<Box>();
    box.mWidth = 5.0f;
    box.mHeight = 1.25f;
    box.mCenter = boxCenters[i];
    Rsl::GetRes<Gfx::Material>(attributeBoxes[i].Get<Comp::Mesh>().mMaterialId)
      .Add<Vec4>("uColor") = {0, 1, 0, 1};

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
    attributeConnectors[i] = space.CreateObject();
    auto& line = attributeConnectors[i].Add<Line>();
    line.mStart = finalVertexSpherePosition;
    line.mEnd = finalVertexSpherePosition;
    line.mThickness = 0.14f;
    line.UpdateTransform(attributeConnectors[i]);
    Rsl::GetRes<Gfx::Material>(
      attributeConnectors[i].Get<Comp::Mesh>().mMaterialId)
      .Add<Vec4>("uColor") = {1, 1, 1, 1};
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

  World::Object attributeLabel = space.CreateObject();
  {
    auto& text = attributeLabel.Add<Comp::Text>();
    text.mFontId = fugazId;
    text.mFillAmount = 0.0f;
    text.mColor = {1, 1, 1, 1};
    text.mText = "Attributes";
    text.mAlign = Comp::Text::Alignment::Center;
    auto& transform = attributeLabel.Get<Comp::Transform>();
    transform.SetTranslation({6.5f, -0.3f, 0.0f});
    transform.SetUniformScale(0.7f);
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
    attributeArrows[i] = space.CreateObject();
    auto& arrow = attributeArrows[i].Add<Arrow>();
    arrow.mStart = arrowStarts[i];
    arrow.mEnd = arrowEnds[i];
    Rsl::GetRes<Gfx::Material>(attributeArrows[i].Get<Comp::Mesh>().mMaterialId)
      .Add<Vec4>("uColor") = {1, 1, 1, 1};

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
    attributeFlashes[i] = space.CreateObject();
    Vec3 flashCenter = boxCenters[i];
    flashCenter[2] = -1.0f;
    auto& transform = attributeFlashes[i].Add<Comp::Transform>();
    transform.SetTranslation(flashCenter);
    transform.SetScale({4.75f / 2.0f, 1.0f / 2.0f, 0.1f});
    auto& mesh = attributeFlashes[i].Add<Comp::Mesh>();
    mesh.mMeshId = Editor::Gizmos::nCubeMeshId;
    mesh.mMaterialId = ResId(resources.GetName(), "Flash" + std::to_string(i));
    resources
      .InitRes<Gfx::Material>(
        mesh.mMaterialId.GetResourceName(), "vres/renderer:Color")
      .Add<Vec4>("uColor") = {1, 1, 1, 0};
    seq.Gap(0.25f);
    ao.mName = "FlashAttribute";
    ao.mDuration = 1.5f;
    ao.mEase = EaseType::Flash;
    seq.Add(ao, [=](float t) {
      Rsl::GetRes<Gfx::Material>(
        attributeFlashes[i].Get<Comp::Mesh>().mMaterialId)
        .Get<Vec4>("uColor")[3] = t;
    });
  }
  seq.Wait();
  seq.Gap(1.0f);

  World::Object positionLabel = space.CreateChildObject(attributeBoxes[0]);
  {
    auto& text = positionLabel.Add<Comp::Text>();
    text.mText = "Position";
    text.mFontId = fugazId;
    text.mFillAmount = 0.0f;
    text.mColor = {1, 1, 1, 1};
    text.mAlign = Comp::Text::Alignment::Center;
    auto& transform = positionLabel.Get<Comp::Transform>();
    transform.SetTranslation({0.0f, -0.35f, 0.0f});
    transform.SetUniformScale(0.7f);
  }
  ao.mName = "ShowPosition";
  ao.mDuration = 0.5f;
  ao.mEase = EaseType::QuadIn;
  seq.Add(ao, [=](float t) {
    positionLabel.Get<Comp::Text>().mFillAmount = t;
  });
  seq.Wait();
  seq.Gap(1.0f);

  ResId heartId(resources.GetName(), "Heart");
  resources
    .InitRes<Gfx::Mesh>(heartId.GetResourceName(), "model/heart.obj", false, 1)
    .Finalize();
  World::Object loveAttributes[2];
  for (int i = 0; i < 2; ++i) {
    loveAttributes[i] = space.CreateObject();
    auto& mesh = loveAttributes[i].Add<Comp::Mesh>();
    mesh.mMeshId = heartId;
    mesh.mMaterialId = ResId(resources.GetName(), "Heart" + std::to_string(i));
    resources
      .InitRes<Gfx::Material>(
        mesh.mMaterialId.GetResourceName(), "vres/renderer:Color")
      .Add<Vec4>("uColor") = {1, 1, 1, 1};
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
    Ds::Vector<ResId> fadeMatIds;
    for (int i = 0; i < 2; ++i) {
      fadeMatIds.Push(attributeConnectors[i].Get<Comp::Mesh>().mMaterialId);
      fadeMatIds.Push(attributeBoxes[i].Get<Comp::Mesh>().mMaterialId);
      fadeMatIds.Push(loveAttributes[i].Get<Comp::Mesh>().mMaterialId);
    }
    for (int i = 0; i < 3; ++i) {
      fadeMatIds.Push(attributeArrows[i].Get<Comp::Mesh>().mMaterialId);
    }
    for (const ResId& fadeMatId : fadeMatIds) {
      Rsl::GetRes<Gfx::Material>(fadeMatId).Get<Vec4>("uColor")[3] = newAlpha;
    }
    attributeLabel.Get<Comp::Text>().mColor[3] = newAlpha;
    vertexLabel.Get<Comp::Text>().mColor[3] = newAlpha;
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
      loveAttributes[i].Get<Comp::Mesh>().mVisible = visible;
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

  World::Object positionTable2dParent = space.CreateObject();
  auto* transform = &positionTable2dParent.Add<Comp::Transform>();
  transform->SetTranslation({1.0f, 1.0f, 0.0f});
  World::Object positionTable2d = positionTable2dParent.CreateChild();
  auto* table = &positionTable2d.Add<Table>();
  table->mCenter = {0.0f, 0.0f, 0.0f};
  table->mCount = 2;
  table->mCellWidth = 4.0f;
  table->mCellHeight = 1.25f;

  ao.mName = "Show2dPositionTable";
  ao.mDuration = 1.0f;
  ao.mEase = EaseType::QuadIn;
  seq.Add(ao, [=](float t) {
    positionTable2d.Get<Table>().Fill(positionTable2d, t);
  });
  seq.Wait();

  World::Object table2dBracket = positionTable2dParent.CreateChild();
  auto* bracket = &table2dBracket.Add<Bracket>();
  bracket->mCenter = {-table->Width() / 2.0f - 0.5f, 0.0f, 0.0f};
  bracket->mExtent = {0.0f, table->Height() / 2.0f, 0.0f};

  ao.mName = "Show2dBracket";
  ao.mDuration = 0.5f;
  ao.mEase = EaseType::QuadIn;
  seq.Add(ao, [=](float t) {
    table2dBracket.Get<Bracket>().Fill(table2dBracket, -t);
  });
  seq.Wait();

  World::Object table2dLabel = positionTable2dParent.CreateChild();
  auto* text = &table2dLabel.Add<Comp::Text>();
  text->mWidth = 0.0f;
  text->mAlign = Comp::Text::Alignment::Right;
  text->mFillAmount = 0.0f;
  text->mColor = {1, 1, 1, 1};
  text->mText = "2D";
  text->mFontId = fugazId;
  transform = &table2dLabel.Get<Comp::Transform>();
  transform->SetTranslation({-3.0f, -0.3f, 0.0f});
  transform->SetUniformScale(0.7f);

  ao.mName = "Show2dLabel";
  ao.mDuration = 0.5f;
  ao.mEase = EaseType::QuadIn;
  seq.Add(ao, [=](float t) {
    table2dLabel.Get<Comp::Text>().mFillAmount = t;
  });
  seq.Wait();

  World::Object positionTable3dParent = space.CreateObject();
  transform = &positionTable3dParent.Add<Comp::Transform>();
  Vec3 table3dTranslation = {1.0f, -2.5f, 0.0f};
  transform->SetTranslation(table3dTranslation);
  World::Object positionTable3d = positionTable3dParent.CreateChild();
  table = &positionTable3d.Add<Table>();
  table->mCenter = {0.0f, 0.0f, 0.0f};
  table->mCount = 3;
  table->mCellWidth = 4.0f;
  table->mCellHeight = 1.25f;

  ao.mName = "Show3dPositionTable";
  ao.mDuration = 0.5f;
  ao.mEase = EaseType::QuadIn;
  seq.Add(ao, [=](float t) {
    positionTable3d.Get<Table>().Fill(positionTable3d, t);
  });
  seq.Wait();

  World::Object table3dBracket = positionTable3dParent.CreateChild();
  bracket = &table3dBracket.Add<Bracket>();
  bracket->mCenter = {-table->Width() / 2.0f - 0.5f, 0.0f, 0.0f};
  bracket->mExtent = {0.0f, table->Height() / 2.0f, 0.0f};

  ao.mName = "Show3dBracket";
  ao.mDuration = 0.5f;
  ao.mEase = EaseType::QuadIn;
  seq.Add(ao, [=](float t) {
    table3dBracket.Get<Bracket>().Fill(table3dBracket, -t);
  });
  seq.Wait();

  World::Object table3dLabel = positionTable3dParent.CreateChild();
  text = &table3dLabel.Add<Comp::Text>();
  text->mWidth = 0.0f;
  text->mAlign = Comp::Text::Alignment::Right;
  text->mFillAmount = 0.0f;
  text->mText = "3D";
  text->mColor = {1, 1, 1, 1};
  text->mFontId = fugazId;
  transform = &table3dLabel.Get<Comp::Transform>();
  transform->SetTranslation({-3.0f, -0.3f, 0.0f});
  transform->SetUniformScale(0.7f);

  ao.mName = "Show3DLabel";
  ao.mDuration = 0.5f;
  ao.mEase = EaseType::QuadIn;
  seq.Add(ao, [=](float t) {
    table3dLabel.Get<Comp::Text>().mFillAmount = t;
  });
  seq.Wait();

  ao.mName = "Fade2dTable";
  ao.mDuration = 1.0f;
  ao.mEase = EaseType::QuadOut;
  seq.Add(ao, [=](float t) {
    float newAlpha = 1.0f - t;
    positionTable2d.Get<Table>().Fade(positionTable2d, newAlpha);
    table2dBracket.Get<Bracket>().Fade(table2dBracket, newAlpha);
    table2dLabel.Get<Comp::Text>().mColor[3] = newAlpha;
  });
  seq.Wait();

  ao.mName = "Hide2dTable";
  ao.mDuration = 0.0f;
  seq.Add(ao, [=](float t) {
    bool visible = !(bool)(int)t;
    positionTable2d.Get<Table>().Show(positionTable2d, visible);
    table2dBracket.Get<Bracket>().Show(table2dBracket, visible);
    table2dLabel.Get<Comp::Text>().mVisible = visible;
  });
  seq.Wait();

  ao.mName = "Center3dTable";
  ao.mDuration = 0.5f;
  ao.mEase = EaseType::QuadOutIn;
  seq.Add(ao, [=](float t) {
    Vec3 translation = Interpolate(table3dTranslation, {1.0f, 0.0f, 0.0f}, t);
    positionTable3dParent.Get<Comp::Transform>().SetTranslation(translation);
  });
}

Sequence TheFundamentalsOfGraphics()
{
  Sequence seq;
  VertexDescription(&seq);
  return seq;
}

#endif