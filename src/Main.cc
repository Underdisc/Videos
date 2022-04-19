// Varkor commit hash:

#include <iostream>
#include <math.h>

#include <AssetLibrary.h>
#include <Input.h>
#include <Registrar.h>
#include <Result.h>
#include <Temporal.h>
#include <VarkorMain.h>
#include <comp/AlphaColor.h>
#include <comp/Camera.h>
#include <comp/Model.h>
#include <comp/Text.h>
#include <comp/Transform.h>
#include <debug/Draw.h>
#include <ds/Vector.h>
#include <editor/Editor.h>
#include <gfx/Font.h>
#include <math/Constants.h>
#include <math/Utility.h>
#include <util/Utility.h>
#include <world/World.h>

#include "TheFundamentalsOfGraphics.h"

/*
using namespace Comp;

namespace Video {

namespace Assets {

AssetId nRod;
AssetId nWireframeCube;
AssetId nPyramid;
AssetId nFugazOne;

void Init()
{
  nRod = AssLib::CreateEmpty<Gfx::Model>("Rod", "model/rod.obj");
  nWireframeCube =
    AssLib::CreateEmpty<Gfx::Model>("WireframeCube", "model/wireframeCube.obj");
  nPyramid = AssLib::CreateEmpty<Gfx::Model>("Pyramid", "model/pyramid.obj");
  nFugazOne =
    AssLib::AssetBin<Gfx::Font>::Require("FugazOne", "font/fugazOne/font.ttf");
}

} // namespace Assets


struct VertexPosition: public Timeline
{
  World::MemberId mOriginSphereId;
  World::MemberId mRodXId;
  World::MemberId mSphereXId;
  World::MemberId mRodYId;
  World::MemberId mSphereYId;
  World::MemberId mRodZId;
  World::MemberId mSphereZId;

  void AddEvents(World::Space& space);
};

void VertexPosition::AddEvents(World::Space& space)
{
  //// Grow a sphere at the origin.
  // Event originSphereEvent;
  // originSphereEvent.mTimeUntil = 0.0f;
  // originSphereEvent.mDuration = 1.0f;
  // originSphereEvent.mBegin = [this, &space]() {
  //  mOriginSphereId = space.CreateMember();
  //  Model& model = space.AddComponent<Model>(mOriginSphereId);
  //  model.mModelId = AssLib::nSphereModelId;
  //  model.mShaderId = AssLib::nColorShaderId;
  //  AlphaColor &alphaColorComp =
  //      space.AddComponent<AlphaColor>(mOriginSphereId);
  //  alphaColorComp.mAlphaColor = {1.0f, 1.0f, 1.0f, 1.0f};
  //};
  // originSphereEvent.mPerform = [this, &space](float t) {
  //  t = Ease(t, EaseType::QuadIn);
  //  float size = Interpolate<float>(0.0f, 0.1f, t);
  //  Transform &transform = *space.GetComponent<Transform>(mOriginSphereId);
  //  transform.SetUniformScale(size);
  //};
  // mEvents.Push(Util::Move(originSphereEvent));

  //// Grow a rod on the x axis.
  // Event xAxisEvent;
  // xAxisEvent.mTimeUntil = 1.0f;
  // xAxisEvent.mDuration = 1.0f;
  // xAxisEvent.mBegin = [this, &space]() {
  //  mRodXId = space.CreateMember();
  //  Model &rodModel = space.AddComponent<Model>(mRodXId);
  //  rodModel.mModelId = Assets::nRod;

  //  mSphereXId = space.CreateMember();
  //  Transform& sphereTransform = space.AddComponent<Transform>(mSphereXId);
  //  sphereTransform.SetUniformScale(0.1f);
  //  Model &sphereModel = space.AddComponent<Model>(mSphereXId);
  //  sphereModel.mModelId = AssLib::nSphereModelId;
  //  sphereModel.mShaderId = AssLib::nColorShaderId;
  //  AlphaColor& alphaColorComp = space.AddComponent<AlphaColor>(mSphereXId);
  //  alphaColorComp.mAlphaColor = {1.0f, 1.0f, 1.0f, 1.0f};
  //};
  // xAxisEvent.mPerform = [this, &space](float t) {
  //  t = Ease(t, EaseType::QuadIn);
  //  Vec3 rodScale =
  //      Interpolate<Vec3>({0.0f, 0.8f, 0.8f}, {1.0f, 0.8f, 0.8f}, t);
  //  Vec3 rodPos = Interpolate<Vec3>({0.0f, 0.0f, 0.0f}, {0.5f, 0.0f, 0.0f},
  //  t); Vec3 spherePos =
  //      Interpolate<Vec3>({0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, t);

  //  Transform &rodTransform = *space.GetComponent<Transform>(mRodXId);
  //  rodTransform.SetScale(rodScale);
  //  rodTransform.SetTranslation(rodPos);
  //  Transform &sphereTransform = *space.GetComponent<Transform>(mSphereXId);
  //  sphereTransform.SetTranslation(spherePos);
  //};
  // mEvents.Push(Util::Move(xAxisEvent));

  //// Grow a rod on the y axis.
  // Event yAxisEvent;
  // yAxisEvent.mTimeUntil = 1.0f;
  // yAxisEvent.mDuration = 1.0f;
  // yAxisEvent.mBegin = [this, &space]() {
  //  mRodYId = space.CreateMember();
  //  Transform &rodTransform = space.AddComponent<Transform>(mRodYId);
  //  Math::Quaternion rodRotation;
  //  rodRotation.FromTo({1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f});
  //  rodTransform.SetRotation(rodRotation);
  //  Model &rodModel = space.AddComponent<Model>(mRodYId);
  //  rodModel.mModelId = Assets::nRod;

  //  mSphereYId = space.CreateMember();
  //  Transform& sphereTransform = space.AddComponent<Transform>(mSphereYId);
  //  sphereTransform.SetUniformScale(0.1f);
  //  Model &sphereModel = space.AddComponent<Model>(mSphereYId);
  //  sphereModel.mModelId = AssLib::nSphereModelId;
  //  sphereModel.mShaderId = AssLib::nColorShaderId;
  //  AlphaColor& alphaColorComp = space.AddComponent<AlphaColor>(mSphereYId);
  //  alphaColorComp.mAlphaColor = {1.0f, 1.0f, 1.0f, 1.0f};
  //};
  // yAxisEvent.mPerform = [this, &space](float t) {
  //  t = Ease(t, EaseType::QuadIn);
  //  Vec3 rodScale =
  //      Interpolate<Vec3>({0.0f, 0.8f, 0.8f}, {1.0f, 0.8f, 0.8f}, t);
  //  Vec3 rodPos = Interpolate<Vec3>({1.0f, 0.0f, 0.0f}, {1.0f, 0.5f, 0.0f},
  //  t); Vec3 spherePos =
  //      Interpolate<Vec3>({1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 0.0f}, t);

  //  Transform &rodTransform = *space.GetComponent<Transform>(mRodYId);
  //  rodTransform.SetScale(rodScale);
  //  rodTransform.SetTranslation(rodPos);
  //  Transform &sphereTransform = *space.GetComponent<Transform>(mSphereYId);
  //  sphereTransform.SetTranslation(spherePos);
  //};
  // mEvents.Push(Util::Move(yAxisEvent));

  //// Grow a rod on the z axis.
  // Event zAxisEvent;
  // zAxisEvent.mTimeUntil = 1.0f;
  // zAxisEvent.mDuration = 1.0f;
  // zAxisEvent.mBegin = [this, &space]() {
  //  mRodZId = space.CreateMember();
  //  Transform &rodTransform = space.AddComponent<Transform>(mRodZId);
  //  Math::Quaternion rodRotation;
  //  rodRotation.FromTo({1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f});
  //  rodTransform.SetRotation(rodRotation);
  //  Model &rodModel = space.AddComponent<Model>(mRodZId);
  //  rodModel.mModelId = Assets::nRod;

  //  mSphereZId = space.CreateMember();
  //  Transform& sphereTransform = space.AddComponent<Transform>(mSphereZId);
  //  sphereTransform.SetUniformScale(0.1f);
  //  Model &sphereModel = space.AddComponent<Model>(mSphereZId);
  //  sphereModel.mModelId = AssLib::nSphereModelId;
  //  sphereModel.mShaderId = AssLib::nColorShaderId;
  //  AlphaColor& alphaColorComp = space.AddComponent<AlphaColor>(mSphereZId);
  //  alphaColorComp.mAlphaColor = {1.0f, 1.0f, 1.0f, 1.0f};
  //};
  // zAxisEvent.mPerform = [this, &space](float t) {
  //  t = Ease(t, EaseType::QuadIn);
  //  Vec3 rodScale =
  //      Interpolate<Vec3>({0.0f, 0.8f, 0.8f}, {1.0f, 0.8f, 0.8f}, t);
  //  Vec3 rodPos = Interpolate<Vec3>({1.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 0.5f},
  //  t); Vec3 spherePos =
  //      Interpolate<Vec3>({1.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, t);

  //  Transform &rodTransform = *space.GetComponent<Transform>(mRodZId);
  //  rodTransform.SetScale(rodScale);
  //  rodTransform.SetTranslation(rodPos);
  //  Transform &sphereTransform = *space.GetComponent<Transform>(mSphereZId);
  //  sphereTransform.SetTranslation(spherePos);
  //};
  // mEvents.Push(Util::Move(zAxisEvent));

  // Event textLabelEvent;
  // textLabelEvent.mTimeUntil = 1.0f;
  // textLabelEvent.mDuration = 1.0f;
}
VertexPosition vertexPositionAnimation;

struct Rotator
{
  Math::Quaternion mMoment;
  void VInit(const World::Object& owner)
  {
    mMoment.AngleAxis(Math::nPi, {0.0f, 1.0f, 0.0f});
  }

  void VUpdate(const World::Object& owner)
  {
    float totalTime = Temporal::TotalTime() / 4.0f;
    Vec3 axisAddendA = {std::cosf(totalTime), 0.0f, std::sinf(totalTime)};
    Vec3 axisAddendB = {0.0f, std::cosf(totalTime), std::sinf(totalTime)};
    Vec3 momentAxis = axisAddendA + axisAddendB;
    mMoment.AngleAxis(Math::nPi / 4.0f, momentAxis);

    // Apply the moment.
    Transform& transform = *owner.GetComponent<Transform>();
    Math::Quaternion rotation = transform.GetRotation();
    rotation *= mMoment.Interpolate(Temporal::DeltaTime());
    transform.SetRotation(rotation);
  }
};

struct CameraOrbiter
{
  Vec3 mPosition;
  float mDistance;
  void VInit(const World::Object& owner)
  {
    mPosition = {0.0f, 0.0f, 0.0f};
    mDistance = 5.0f;
  }

  void VUpdate(const World::Object& owner)
  {
    Transform& transform = *owner.GetComponent<Transform>();
    Camera& camera = *owner.GetComponent<Camera>();
    float tt = Temporal::TotalTime();
    Vec3 newTranslation = {0.0f, 0.0f, 5.0f};
    // Vec3 newTranslation = {
    //  mDistance * std::cosf(tt), 2.0f, mDistance * std::sinf(tt)};
    transform.SetTranslation(newTranslation);
    camera.WorldLookAt(mPosition, {0.0f, 1.0f, 0.0f}, owner);
  }
};

struct Introduction: public Timeline
{
  static World::MemberId smCameraId;
  static World::MemberId smCubeId;
  static World::MemberId smTimelineId;
  static World::MemberId smDates[3];
  static World::MemberId smSphereIds[5];
  static World::MemberId smRodIds[4];

  void Init(World::Space& space);
};
World::MemberId Introduction::smCameraId;
World::MemberId Introduction::smCubeId;
World::MemberId Introduction::smTimelineId;
World::MemberId Introduction::smDates[3];
World::MemberId Introduction::smSphereIds[5];
World::MemberId Introduction::smRodIds[4];
void Introduction::Init(World::Space& space)
{
  // Event rotatingCube;
  // rotatingCube.mTimeUntil = 0.0f;
  // rotatingCube.mDuration = 3.0f;
  // rotatingCube.mEase = EaseType::QuadIn;
  // rotatingCube.AddSplit(
  //  [&space]() {
  //    smCubeId = space.CreateMember();
  //    Model& model = space.AddComponent<Model>(smCubeId);
  //    model.mModelId = Assets::nWireframeCube;
  //    space.AddComponent<Rotator>(smCubeId);

  //    smCameraId = space.CreateMember();
  //    space.AddComponent<CameraOrbiter>(smCameraId);
  //    space.mCameraId = smCameraId;
  //    Editor::nEditorMode = false;
  //
  //  },
  //  [&space](float t) {
  //    t = Ease(t, EaseType::QuadIn);
  //    Transform& transform = *space.GetComponent<Transform>(smCubeId);
  //    float scale = Interpolate<float>(0.0f, 0.8f, t);
  //    transform.SetUniformScale(scale);
  //  });
  // mEvents.Emplace(rotatingCube);

  // Event firstTimelineEvent;
  // firstTimelineEvent.mTimeUntil = 4.0f;
  // firstTimelineEvent.mDuration = 3.0f;
  // firstTimelineEvent.mEase = EaseType::QuadIn;
  //// Create the timeline object and the first sphere.
  // firstTimelineEvent.AddSplit(
  //  [&space]() {
  //    smTimelineId = space.CreateMember();
  //    Transform& timelineTransform =
  //      space.AddComponent<Transform>(smTimelineId);
  //    timelineTransform.SetTranslation({0.0f, -2.0f, 0.0f});
  //    smSphereIds[0] = space.CreateChildMember(smTimelineId);
  //    Transform& sphereTransform =
  //      space.AddComponent<Transform>(smSphereIds[0]);
  //    sphereTransform.SetTranslation({-3.0f, 0.0f, 0.0f});
  //    Model& sphereModel = space.AddComponent<Model>(smSphereIds[0]);
  //    sphereModel.mModelId = AssLib::nSphereModelId;
  //  },
  //  [&space](float t) {
  //    Transform& sphereTransform =
  //      *space.GetComponent<Transform>(smSphereIds[0]);
  //    float scale = Interpolate(0.0f, 0.1f, t);
  //    sphereTransform.SetUniformScale(scale);
  //  });
  //// Create the rod extending of the first sphere.
  // firstTimelineEvent.AddSplit(
  //  [&space]() {
  //    smSphereIds[1] = space.CreateChildMember(smTimelineId);
  //    Transform& sphereTransform =
  //      space.AddComponent<Transform>(smSphereIds[1]);
  //    sphereTransform.SetUniformScale(0.1f);
  //    Model& sphereModel = space.AddComponent<Model>(smSphereIds[1]);
  //    sphereModel.mModelId = AssLib::nSphereModelId;
  //    smRodIds[0] = space.CreateChildMember(smTimelineId);
  //    Model& rodModel = space.AddComponent<Model>(smRodIds[0]);
  //    rodModel.mModelId = Assets::nRod;
  //  },
  //  [&space](float t) {
  //    Transform& rodTransform = *space.GetComponent<Transform>(smRodIds[0]);
  //    Vec3 rodScale =
  //      Interpolate<Vec3>({0.0f, 0.8f, 0.8f}, {1.0f, 0.8f, 0.8f}, t);
  //    Vec3 rodTranslation =
  //      Interpolate<Vec3>({-3.0f, 0.0f, 0.0f}, {-2.5f, 0.0f, 0.0f}, t);
  //    rodTransform.SetScale(rodScale);
  //    rodTransform.SetTranslation(rodTranslation);
  //    Transform& sphereTransform =
  //      *space.GetComponent<Transform>(smSphereIds[1]);
  //    Vec3 sphereTranslation =
  //      Interpolate<Vec3>({-3.0f, 0.0f, 0.0f}, {-2.0f, 0.0f, 0.0f}, t);
  //    sphereTransform.SetTranslation(sphereTranslation);
  //  });
  //// Create the rod that extends to the date.
  // firstTimelineEvent.AddSplit(
  //  [&space]() {
  //    smRodIds[1] = space.CreateChildMember(smTimelineId);
  //    Transform& rodTransform = space.AddComponent<Transform>(smRodIds[1]);
  //    Math::Quaternion rodRotation;
  //    rodRotation.FromTo({1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f});
  //    rodTransform.SetRotation(rodRotation);
  //    Model& rodModel = space.AddComponent<Model>(smRodIds[1]);
  //    rodModel.mModelId = Assets::nRod;

  //    smRodIds[2] = space.CreateChildMember(smTimelineId);
  //    Transform& underlineTransform =
  //      space.AddComponent<Transform>(smRodIds[2]);
  //    underlineTransform.SetScale({0.2f, 0.8f, 0.8f});
  //    Model& underlineModel = space.AddComponent<Model>(smRodIds[2]);
  //    underlineModel.mModelId = Assets::nRod;
  //  },
  //  [&space](float t) {
  //    Transform& rodTransform = *space.GetComponent<Transform>(smRodIds[1]);
  //    Vec3 rodScale =
  //      Interpolate<Vec3>({0.0f, 0.8f, 0.8f}, {0.5f, 0.8f, 0.8f}, t);
  //    Vec3 rodTranslation =
  //      Interpolate<Vec3>({-2.0f, 0.0f, 0.0f}, {-2.0f, 0.25f, 0.0f}, t);
  //    rodTransform.SetScale(rodScale);
  //    rodTransform.SetTranslation(rodTranslation);

  //    Transform& underlineTransform =
  //      *space.GetComponent<Transform>(smRodIds[2]);
  //    Vec3 underlineTranslation =
  //      Interpolate<Vec3>({-2.0f, 0.0f, 0.0f}, {-2.0f, 0.5f, 0.0f}, t);
  //    underlineTransform.SetTranslation(underlineTranslation);
  //  });
  //// Create the date.
  // firstTimelineEvent.AddSplit(
  //  [&space]() {
  //    smDates[0] = space.CreateChildMember(smTimelineId);
  //    Transform& dateTransform = space.AddComponent<Transform>(smDates[0]);
  //    dateTransform.SetTranslation({-2.0f, 0.6f, 0.0f});
  //    dateTransform.SetUniformScale(0.25f);
  //    Text& dateText = space.AddComponent<Text>(smDates[0]);
  //    dateText.mFontId = Assets::nFugazOne;
  //    dateText.mText = "63";
  //    dateText.mAlign = Text::Alignment::Center;
  //    dateText.mWidth = 0.0f;
  //  },
  //  [&space](float t) {
  //    Text& dateText = *space.GetComponent<Text>(smDates[0]);
  //    dateText.mFillAmount = t;
  //    std::cout << t << "\n";
  //  });
  // mEvents.Emplace(firstTimelineEvent);
}
Introduction nIntroduction;
} // namespace Video

*/

Sequence gSequence;
void CentralUpdate()
{
  if (Input::KeyPressed(Input::Key::Space)) {
    if (gSequence.mStatus == Sequence::Status::Play) {
      gSequence.Pause();
    } else {
      gSequence.Play();
    }
  }
  gSequence.Update(Temporal::DeltaTime());

  ImGui::Begin("Sequence");
  float time = gSequence.mTimePassed;
  ImGui::PushItemWidth(-1.0f);
  ImGui::SliderFloat("Time", &time, 0.0f, gSequence.mTotalTime);
  if (time != gSequence.mTimePassed) {
    gSequence.Pause();
    gSequence.Scrub(time);
  }
  ImGui::End();
}

void RegisterTypes()
{
  // Registrar::Register<Video::Rotator, Transform>();
  // Registrar::Register<Video::CameraOrbiter, Camera>();
  Registrar::Register<Line, Comp::Model>();
  Registrar::Register<Bracket, Comp::Transform>();
  Registrar::Register<Box, Comp::Model>();
  Registrar::Register<Arrow, Comp::Model>();
  Registrar::Register<Table, Comp::Transform>();
}

int main(int argc, char* argv[])
{
  Registrar::nRegisterCustomTypes = RegisterTypes;
  Result result = VarkorInit(argc, argv, "Varkor Videos", PROJECT_DIRECTORY);
  if (!result.Success()) {
    return 0;
  }

  TheFundamentalsOfGraphics(&gSequence);
  World::nCentralUpdate = CentralUpdate;
  VarkorRun();

  VarkorPurge();
}
