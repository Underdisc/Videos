#include "EventSequence.h"

#include <iostream>

float Ease(float t, EaseType easeType)
{
  switch (easeType) {
  case EaseType::Linear: break;
  case EaseType::QuadIn:
    t = (t - 1.0f);
    t = 1.0f - t * t;
    break;
  case EaseType::QuadOut: t = t * t; break;
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

void EventSequence::Event::Run(float t) const
{
  mFunction(t);
}

EventSequence::EventSequence():
  mStatus(Status::Start), mTimeAfter(0.0f), mNextInactiveEvent(0)
{}

void EventSequence::AddEvent(
  const std::string& name,
  float timeUntil,
  std::function<void(float t)> function)
{
  Event newEvent;
  newEvent.mName = name;
  newEvent.mTimeUntil = timeUntil;
  newEvent.mDuration = 0.0f;
  newEvent.mEase = EaseType::Linear;
  newEvent.mFunction = function;
  mEvents.Push(newEvent);
}

void EventSequence::AddEvent(
  const std::string& name,
  float timeUntil,
  float duration,
  EaseType ease,
  std::function<void(float t)> function)
{
  Event newEvent;
  newEvent.mName = name;
  newEvent.mTimeUntil = timeUntil;
  newEvent.mDuration = duration;
  newEvent.mEase = ease;
  newEvent.mFunction = function;
  mEvents.Push(newEvent);
}

void EventSequence::Continue()
{
  mStatus = Status::Perform;
}

void EventSequence::Pause()
{
  mStatus = Status::Pause;
}

void EventSequence::Update(float dt)
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

  if (mNextInactiveEvent == mEvents.Size()) {
    mStatus = Status::End;
  }
}