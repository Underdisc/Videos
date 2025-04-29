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
  Flash,
};
float Ease(float t, EaseType easeType);

struct Sequence
{
  Sequence();
  enum class Status
  {
    Play,
    Pause
  };

  struct Event
  {
    std::string mName;
    float mStartTime;
    float mEndTime;
    EaseType mEase;
    std::function<void(float t)> mFunction;
    void Run(float t) const;
  };

  struct AddOptions
  {
    std::string mName;
    float mDuration;
    EaseType mEase;
  };
  void Add(const AddOptions& options, std::function<void(float t)> function);
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
  Status mStatus;
  unsigned int mNextInactiveEvent;
  Ds::Vector<Event> mEvents;
  Ds::Vector<unsigned int> mActiveEvents;
};

#endif