#ifndef EventSequence_h
#define EventSequence_h

#include <ds/Vector.h>
#include <functional>
#include <string>

template<typename T>
T Interpolate(const T& start, const T& end, float t)
{
  return (1.0f - t) * start + t * end;
}

enum class EaseType
{
  Linear,
  QuadIn,
  QuadOut,
  QuadOutIn,
  Cubic,
  FlattenedCubic,
};
float Ease(float t, EaseType easeType);

struct EventSequence
{
  enum class Status
  {
    Start,
    Perform,
    End,
    Pause
  };

  struct Options
  {
    std::string mName;
    float mDuration;
    EaseType mEase;
  };

  struct Event
  {
    std::string mName;
    // Duration since the previous event before this event begins.
    float mTimeUntil;
    float mDuration;
    EaseType mEase;
    std::function<void(float t)> mFunction;
    void Run(float t) const;
  };

  struct ActiveEvent
  {
    unsigned int mEventIndex;
    float mTimeAfter;
  };

  void Add(const Options& options, std::function<void(float t)> function);
  void Gap(float duration);
  void Wait();

  EventSequence();

  void Continue();
  void Pause();
  void Update(float dt);

  // Duration since the start of the last event activated event.
  float mTimeAfter;
  Status mStatus;
  unsigned int mNextInactiveEvent;
  Ds::Vector<Event> mEvents;
  Ds::Vector<ActiveEvent> mActiveEvents;
};

#endif