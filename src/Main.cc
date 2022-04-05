// Varkor commit hash: 

#include <functional>
#include <math.h>
#include <iostream>

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
#include <gfx/Model.h>
#include <math/Constants.h>
#include <math/Utility.h>
#include <util/Utility.h>
#include <world/World.h>

enum class EaseType
{
  Linear,
  QuadIn,
  Cubic,
  FlattenedCubic,
};

float Ease(float t, EaseType easeType)
{
  switch (easeType)
  {
  case EaseType::Linear:
    break;
  case EaseType::QuadIn:
    t = (t - 1.0f);
    t = 1.0f - t * t;
    break;
  case EaseType::Cubic:
    t = t * 2.0f - 1.0f;
    t = t * t * t * 0.5f + 0.5f;
    break;
  case EaseType::FlattenedCubic:
    t = 2.0f * t - 1.0f;
    t = t + t * t * t;
    t = t * 0.25f + 0.5f;
    break;
  }
  return t;
}

struct Event
{
private:
  size_t mLastSplit;

public:
  Event(): mLastSplit(0) {}

  // The idea.
  // This is the time until the the event starts since the beginning of the
  // previous event, or, if it's the first event, since the beginning of the
  // timeline.
  float mTimeUntil;
  float mDuration;
  EaseType mEase;

  typedef std::function<void()> BeginFunc;
  typedef std::function<void(float t)> PerformFunc;
  struct Split
  {
    BeginFunc mBegin;
    PerformFunc mPerform;

    Split(BeginFunc begin, PerformFunc perform):
      mBegin(begin), mPerform(perform)
    {}
  };
  Ds::Vector<Split> mSplits;

  void AddSplit(BeginFunc begin, PerformFunc perform)
  {
    mSplits.Emplace(begin, perform);
  }

  void Begin()
  {
    LogAbortIf(mSplits.Size() == 0, "Event does not contain any splits.");
    mSplits[mLastSplit].mBegin();
  }

  void Perform(float eventT)
  {
    eventT = Ease(eventT, mEase);
    float splitScaledT = eventT * mSplits.Size();
    size_t currentSplit = size_t(splitScaledT);

    if (currentSplit >= mSplits.Size()) {
      mSplits[mLastSplit].mPerform(1.0f);
      for (size_t i = mLastSplit + 1; i < mSplits.Size(); ++i) {
        mSplits[i].mBegin();
        mSplits[i].mPerform(1.0f);
      }
      return;
    }

    float splitT = splitScaledT - (float)currentSplit;
    if (mLastSplit < currentSplit) {
      for (size_t i = mLastSplit; i < currentSplit; ++i) {
        mSplits[i].mBegin();
        mSplits[i].mPerform(1.0f);
      }
      mSplits[currentSplit].mBegin();
      mLastSplit = currentSplit;
    }
    mSplits[mLastSplit].mPerform(splitT);
  }
};

struct Timeline
{
  enum class Status {
    Start,
    Perform,
    End,
    Pause
  };

  Timeline() : mStatus(Status::Start),
      mTimeSince(0.0f), mHotEventStart(0), mHotEventEnd(0) {}

  void Continue() {
    mStatus = Status::Perform;
  }

  void Pause() {
    mStatus = Status::Pause;
  }

  void Update()
  {
    if (mStatus != Status::Perform || mHotEventStart == mEvents.Size()) {
      return;
    }
    mTimeSince += Temporal::DeltaTime();

    // Start any new events.
    float untilSum = 0.0f;
    for (int i = mHotEventStart; i < mHotEventEnd; ++i) {
      Event& event = mEvents[i];
      untilSum += event.mTimeUntil;
    }
    for (int i = mHotEventEnd; i < mEvents.Size(); ++i) {
      Event &event = mEvents[i];
      untilSum += event.mTimeUntil;
      if (mTimeSince >= untilSum) {
        ++mHotEventEnd;
        event.Begin();
      } else {
        break;
      }
    }

    // Perform all of the currently active events and end any finished events.
    float timeSince = mTimeSince;
    for (int i = mHotEventStart; i < mHotEventEnd; ++i) {
      Event& event = mEvents[i];
      timeSince -= event.mTimeUntil;
      if (timeSince >= event.mDuration) {
        event.Perform(1.0f);
        ++mHotEventStart;
        mTimeSince -= event.mTimeUntil;
        if (mHotEventStart == mEvents.Size()) {
          mStatus = Status::End;
          return;
        }
      } else {
        event.Perform(timeSince / event.mDuration);
      }
    }
  }

  Status mStatus;
  // The time since the start of the event before the current hot event range.
  float mTimeSince;
  int mHotEventStart;
  int mHotEventEnd;
  Ds::Vector<Event> mEvents;
};


template<typename T>
T Interpolate(const T& start, const T& end, float t)
{
  return (1.0f - t) * start + t * end;
}

using namespace Comp;

namespace Video
{

namespace Assets
{

AssetId nRod;
AssetId nWireframeCube;
AssetId nPyramid;
AssetId nFugazOne;

void Init()
{

  nRod = AssLib::Create<Gfx::Model>("Rod", "model/rod.obj");
  nWireframeCube =
      AssLib::Create<Gfx::Model>("WireframeCube", "model/wireframeCube.obj");
  nPyramid = AssLib::Create<Gfx::Model>("Pyramid", "model/pyramid.obj");
  nFugazOne = AssLib::AssetBin<Gfx::Font>::Require(
    "FugazOne", "font/fugazOne/font.ttf");
}

} // namespace Assets

struct VertexPosition : public Timeline
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
  //Event originSphereEvent;
  //originSphereEvent.mTimeUntil = 0.0f;
  //originSphereEvent.mDuration = 1.0f;
  //originSphereEvent.mBegin = [this, &space]() {
  //  mOriginSphereId = space.CreateMember();
  //  Model& model = space.AddComponent<Model>(mOriginSphereId);
  //  model.mModelId = AssLib::nSphereModelId;
  //  model.mShaderId = AssLib::nColorShaderId;
  //  AlphaColor &alphaColorComp =
  //      space.AddComponent<AlphaColor>(mOriginSphereId);
  //  alphaColorComp.mAlphaColor = {1.0f, 1.0f, 1.0f, 1.0f};
  //};
  //originSphereEvent.mPerform = [this, &space](float t) {
  //  t = Ease(t, EaseType::QuadIn);
  //  float size = Interpolate<float>(0.0f, 0.1f, t);
  //  Transform &transform = *space.GetComponent<Transform>(mOriginSphereId);
  //  transform.SetUniformScale(size);
  //};
  //mEvents.Push(Util::Move(originSphereEvent));

  //// Grow a rod on the x axis.
  //Event xAxisEvent;
  //xAxisEvent.mTimeUntil = 1.0f;
  //xAxisEvent.mDuration = 1.0f;
  //xAxisEvent.mBegin = [this, &space]() {
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
  //xAxisEvent.mPerform = [this, &space](float t) {
  //  t = Ease(t, EaseType::QuadIn);
  //  Vec3 rodScale =
  //      Interpolate<Vec3>({0.0f, 0.8f, 0.8f}, {1.0f, 0.8f, 0.8f}, t);
  //  Vec3 rodPos = Interpolate<Vec3>({0.0f, 0.0f, 0.0f}, {0.5f, 0.0f, 0.0f}, t);
  //  Vec3 spherePos =
  //      Interpolate<Vec3>({0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, t);

  //  Transform &rodTransform = *space.GetComponent<Transform>(mRodXId);
  //  rodTransform.SetScale(rodScale);
  //  rodTransform.SetTranslation(rodPos);
  //  Transform &sphereTransform = *space.GetComponent<Transform>(mSphereXId);
  //  sphereTransform.SetTranslation(spherePos);
  //};
  //mEvents.Push(Util::Move(xAxisEvent));

  //// Grow a rod on the y axis.
  //Event yAxisEvent;
  //yAxisEvent.mTimeUntil = 1.0f;
  //yAxisEvent.mDuration = 1.0f;
  //yAxisEvent.mBegin = [this, &space]() {
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
  //yAxisEvent.mPerform = [this, &space](float t) {
  //  t = Ease(t, EaseType::QuadIn);
  //  Vec3 rodScale =
  //      Interpolate<Vec3>({0.0f, 0.8f, 0.8f}, {1.0f, 0.8f, 0.8f}, t);
  //  Vec3 rodPos = Interpolate<Vec3>({1.0f, 0.0f, 0.0f}, {1.0f, 0.5f, 0.0f}, t);
  //  Vec3 spherePos =
  //      Interpolate<Vec3>({1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 0.0f}, t);

  //  Transform &rodTransform = *space.GetComponent<Transform>(mRodYId);
  //  rodTransform.SetScale(rodScale);
  //  rodTransform.SetTranslation(rodPos);
  //  Transform &sphereTransform = *space.GetComponent<Transform>(mSphereYId);
  //  sphereTransform.SetTranslation(spherePos);
  //};
  //mEvents.Push(Util::Move(yAxisEvent));

  //// Grow a rod on the z axis.
  //Event zAxisEvent;
  //zAxisEvent.mTimeUntil = 1.0f;
  //zAxisEvent.mDuration = 1.0f;
  //zAxisEvent.mBegin = [this, &space]() {
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
  //zAxisEvent.mPerform = [this, &space](float t) {
  //  t = Ease(t, EaseType::QuadIn);
  //  Vec3 rodScale =
  //      Interpolate<Vec3>({0.0f, 0.8f, 0.8f}, {1.0f, 0.8f, 0.8f}, t);
  //  Vec3 rodPos = Interpolate<Vec3>({1.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 0.5f}, t);
  //  Vec3 spherePos =
  //      Interpolate<Vec3>({1.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, t);

  //  Transform &rodTransform = *space.GetComponent<Transform>(mRodZId);
  //  rodTransform.SetScale(rodScale);
  //  rodTransform.SetTranslation(rodPos);
  //  Transform &sphereTransform = *space.GetComponent<Transform>(mSphereZId);
  //  sphereTransform.SetTranslation(spherePos);
  //};
  //mEvents.Push(Util::Move(zAxisEvent));

  //Event textLabelEvent;
  //textLabelEvent.mTimeUntil = 1.0f;
  //textLabelEvent.mDuration = 1.0f;
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
    float totalTime = Temporal::TotalTime()/4.0f;
    Vec3 axisAddendA = {std::cosf(totalTime), 0.0f, std::sinf(totalTime)};
    Vec3 axisAddendB = {0.0f, std::cosf(totalTime), std::sinf(totalTime)};
    Vec3 momentAxis = axisAddendA + axisAddendB;
    mMoment.AngleAxis(Math::nPi/4.0f, momentAxis);

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
    //Vec3 newTranslation = {
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
  //Event rotatingCube;
  //rotatingCube.mTimeUntil = 0.0f;
  //rotatingCube.mDuration = 3.0f;
  //rotatingCube.mEase = EaseType::QuadIn;
  //rotatingCube.AddSplit(
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
  //mEvents.Emplace(rotatingCube);

  //Event firstTimelineEvent;
  //firstTimelineEvent.mTimeUntil = 4.0f;
  //firstTimelineEvent.mDuration = 3.0f;
  //firstTimelineEvent.mEase = EaseType::QuadIn;
  //// Create the timeline object and the first sphere.
  //firstTimelineEvent.AddSplit(
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
  //firstTimelineEvent.AddSplit(
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
  //firstTimelineEvent.AddSplit(
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
  //firstTimelineEvent.AddSplit(
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
//mEvents.Emplace(firstTimelineEvent);
}
Introduction nIntroduction;

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

struct BracketLabel
{
  World::MemberId mEdgeChildren[2];
  World::MemberId mConnectorChild;
  World::MemberId mTextLineChild;
  World::MemberId mTextChild;

  void VInit(const World::Object& owner)
  {
    //World::Object edgeChild0 = owner.CreateChild();
    //World::Object edgeChild1 = owner.CreateChild();
    //World::Object 
  }

  void SetExents(const Vec3& a, const Vec3& b);
};

struct VertexTimeline: public Timeline
{
  static World::MemberId smCameraId;
  static World::MemberId smFocusLinesIds[8];
  static World::MemberId smVertexSphereId;
  void Init(World::Space& space);
};
World::MemberId VertexTimeline::smCameraId;
World::MemberId VertexTimeline::smFocusLinesIds[8];
World::MemberId VertexTimeline::smVertexSphereId;
VertexTimeline nVertexTimeline;

void VertexTimeline::Init(World::Space& space)
{
  smCameraId = space.CreateMember();
  Camera& camera = space.AddComponent<Camera>(smCameraId);
  camera.mProjectionType = Camera::ProjectionType::Orthographic;
  camera.mHeight = 10.0f;
  Transform& cameraTransform = *space.GetComponent<Transform>(smCameraId);
  cameraTransform.SetTranslation({0.0f, 0.0f, 10.0f});
  World::Object cameraObject(&space, smCameraId);
  camera.LocalLookAt({0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, cameraObject);
  space.mCameraId = smCameraId;

  Event vertexFocusLines;
  vertexFocusLines.mTimeUntil = 1.0f;
  vertexFocusLines.mDuration = 2.0f;
  vertexFocusLines.mEase = EaseType::FlattenedCubic;
  vertexFocusLines.AddSplit(
    [&space]() {
      for (int i = 0; i < 8; ++i)
      {
        Editor::nEditorMode = false;
        smFocusLinesIds[i] = space.CreateMember();
        space.AddComponent<Line>(smFocusLinesIds[i]);
      }
      smVertexSphereId = space.CreateMember();
      Comp::Model& model = space.AddComponent<Comp::Model>(smVertexSphereId);
      model.mModelId = AssLib::nSphereModelId;
      Comp::Transform& transform =
        *space.GetComponent<Comp::Transform>(smVertexSphereId);
      transform.SetUniformScale(0.0f);
    },
    [&space](float t) {
      float t2 = t * 2.0f;
      float endT = Math::Clamp(0.0f, 1.0f, t2);
      float startT = Math::Clamp(0.0f, 1.0f, t2 - 1.0f);
      float angleIncrement = Math::nPi / 4.0f;
      float angle = 0.0f;
      for (int i = 0; i < 8; ++i)
      {
        float cosAngle = std::cosf(angle);
        float sinAngle = std::sinf(angle);
        float endDistance = Interpolate(4.0f, 1.5f, endT);
        float startDistance = Interpolate(4.0f, 1.5f, startT);
        Vec3 start = {cosAngle * startDistance, sinAngle * startDistance, 0.0f};
        Vec3 end = {cosAngle * endDistance, sinAngle * endDistance, 0.0f};

        World::Object owner(&space, smFocusLinesIds[i]);
        Line& line = *owner.GetComponent<Line>();
        line.SetStart(start, owner);
        line.SetEnd(end, owner);

        angle += angleIncrement;

        Comp::Transform& transform =
          *space.GetComponent<Comp::Transform>(smVertexSphereId);
        transform.SetUniformScale(startT);
      }
    });
  
  mEvents.Emplace(vertexFocusLines);
}



void Init()
{
  Assets::Init();
  World::SpaceIt spaceIt = World::CreateTopSpace();
  nVertexTimeline.Init(*spaceIt);
}

void Run()
{
  if (Input::KeyPressed(Input::Key::Space))
  {
    nVertexTimeline.Continue();
  }
  nVertexTimeline.Update();
}

} // namespace Video

void RegisterTypes()
{
  Registrar::Register<Video::Rotator, Transform>();
  Registrar::Register<Video::CameraOrbiter, Camera>();

  Registrar::Register<Video::Line, Model>();
}

int main(int argc, char* argv[])
{
  Registrar::nRegisterCustomTypes = RegisterTypes;
  Result result = VarkorInit(argc, argv, "Varkor Videos", PROJECT_DIRECTORY);
  if (!result.Success()) {
    return 0;
  }

  Video::Init();
  World::nCentralUpdate = Video::Run;

  VarkorRun();
  VarkorPurge();
}

