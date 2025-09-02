#ifndef Video_h
#define Video_h

#include <ds/Vector.h>
#include <functional>
#include <string>
#include <world/World.h>

template<typename T>
T Interpolate(const T& start, const T& end, float t) {
  return (1.0f - t) * start + t * end;
}

enum class EaseType {
  Linear,
  QuadIn,
  QuadOut,
  QuadOutIn,
  Cubic,
  FlattenedCubic,
  Flash,
};
float Ease(float t, EaseType easeType);

struct Sequence {
  Sequence();

  enum class Cross {
    In,
    Out,
  };

  struct DiscreteEvent {
    std::string mName;
    float mStartTime;
    float mEndTime;
    EaseType mEase;
    std::function<void(Cross dir)> mBegin;
    std::function<void(float t)> mLerp;
    std::function<void(Cross dir)> mEnd;
    void Run(float t) const;
  };

  struct ContinuousEvent {
    std::string mName;
    float mDuration;
    EaseType mEase;
    std::function<void(Cross dir)> mBegin;
    std::function<void(float t)> mLerp;
    std::function<void(Cross dir)> mEnd;
  };

  void AddDiscreteEvent(const DiscreteEvent& event);
  void AddContinuousEvent(const ContinuousEvent& event);
  void Gap(float duration);
  void Wait();

  void Play();
  void Pause();
  bool AtEnd();
  void Update(float dt);
  void Scrub(float time);
  void ScrubUp(float time);
  void ScrubDown(float time);

  // The number of seconds that have passed since the start of the sequence.
  float mTimePassed;
  // The total duration of the sequence.
  float mTotalTime;
  unsigned int mNextInactiveEvent;
  Ds::Vector<DiscreteEvent> mEvents;
  Ds::Vector<unsigned int> mActiveEvents;
};

struct Video {
  std::string mName;
  World::LayerIt mLayerIt;
  Sequence mSeq;
};

#endif
