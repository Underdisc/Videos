#include <Error.h>

#include "Sequence.h"

float Ease(float t, EaseType easeType)
{
  switch (easeType) {
  case EaseType::Linear: break;
  case EaseType::QuadIn:
    t = (t - 1.0f);
    t = 1.0f - t * t;
    break;
  case EaseType::QuadOut: t = t * t; break;
  case EaseType::QuadOutIn:
    if (t < 0.5f) {
      t = 2 * t * t;
    } else {
      t = t - 1.0f;
      t = 1.0f - 2 * t * t;
    }
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

void Sequence::Event::Run(float t) const
{
  if (mFunction) {
    mFunction(t);
  }
}

Sequence::Sequence():
  mStatus(Status::Start), mTimeAfter(0.0f), mNextInactiveEvent(0)
{}

void Sequence::Add(const AddOptions& ao, std::function<void(float t)> function)
{
  Event newEvent;
  newEvent.mName = ao.mName;
  newEvent.mTimeUntil = 0.0f;
  newEvent.mDuration = ao.mDuration;
  newEvent.mEase = ao.mEase;
  newEvent.mFunction = function;
  mEvents.Push(newEvent);
}

void Sequence::Gap(float duration)
{
  Event newEvent;
  newEvent.mName = "Gap";
  newEvent.mTimeUntil = duration;
  newEvent.mDuration = 0.0f;
  mEvents.Push(newEvent);
}

void Sequence::Wait()
{
  LogAbortIf(mEvents.Size() == 0, "One existing event required to wait.");
  Gap(mEvents.Top().mDuration);
}

void Sequence::Continue()
{
  mStatus = Status::Perform;
}

void Sequence::Pause()
{
  mStatus = Status::Pause;
}

void Sequence::Update(float dt)
{
  if (mStatus != Status::Perform) {
    return;
  }

  mTimeAfter += dt;

  // Update active event times and complete any finished active events.
  for (int i = 0; i < mActiveEvents.Size(); ++i) {
    ActiveEvent& activeEvent = mActiveEvents[i];
    const Event& event = mEvents[activeEvent.mEventIndex];
    activeEvent.mTimeAfter += dt;
    if (activeEvent.mTimeAfter > event.mDuration) {
      event.Run(1.0f);
      mActiveEvents.Remove(i);
      --i;
    }
  }

  // Activate any events that haven't started.
  float timeAfterLastEvent = mTimeAfter;
  while (mNextInactiveEvent < mEvents.Size()) {
    const Event& event = mEvents[mNextInactiveEvent];
    if (mTimeAfter < event.mTimeUntil) {
      break;
    }
    mTimeAfter -= event.mTimeUntil;
    if (event.mDuration == 0.0f) {
      event.Run(1.0f);
    } else {
      ActiveEvent newActiveEvent;
      newActiveEvent.mEventIndex = mNextInactiveEvent;
      newActiveEvent.mTimeAfter = mTimeAfter;
      mActiveEvents.Push(newActiveEvent);
    }
    ++mNextInactiveEvent;
  }

  // Run all active events.
  for (int i = 0; i < mActiveEvents.Size(); ++i) {
    const ActiveEvent& activeEvent = mActiveEvents[i];
    const Event& event = mEvents[activeEvent.mEventIndex];
    float t = activeEvent.mTimeAfter / event.mDuration;
    t = Ease(t, event.mEase);
    event.Run(t);
  }

  if (mNextInactiveEvent == mEvents.Size() && mActiveEvents.Size() == 0) {
    mStatus = Status::End;
  }
}