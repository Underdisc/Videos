#include <Error.h>

#include "Sequence.h"

Sequence::Sequence():
  mTimePassed(0.0f), mTotalTime(0.0f), mNextInactiveEvent(0) {}

float Ease(float t, EaseType easeType) {
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
    }
    else {
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
  case EaseType::Flash:
    if (t > 0.0f) {
      t = 1.0f - t;
    }
  }
  return t;
}

void Sequence::Event::Run(float t) const {
  t = Ease(t, mEase);
  if (mFunction) {
    mFunction(t);
  }
}

void Sequence::Add(
  const AddOptions& ao, std::function<void(float t)> function) {
  Event newEvent;
  newEvent.mName = ao.mName;
  if (mEvents.Size() == 0) {
    newEvent.mStartTime = 0.0f;
  }
  else {
    newEvent.mStartTime = mEvents.Top().mStartTime;
  }
  newEvent.mEndTime = newEvent.mStartTime + ao.mDuration;
  newEvent.mEase = ao.mEase;
  newEvent.mFunction = function;
  mEvents.Push(newEvent);
  mTotalTime = newEvent.mEndTime;
}

void Sequence::Gap(float duration) {
  Event newEvent;
  newEvent.mName = "Gap";
  if (mEvents.Size() == 0) {
    newEvent.mStartTime = duration;
  }
  else {
    newEvent.mStartTime = mEvents.Top().mStartTime + duration;
  }
  newEvent.mEndTime = newEvent.mStartTime;
  mEvents.Push(newEvent);
  mTotalTime = newEvent.mEndTime;
}

void Sequence::Wait() {
  LogAbortIf(mEvents.Size() == 0, "One existing event required to wait.");
  Gap(mEvents.Top().mEndTime - mEvents.Top().mStartTime);
}

bool Sequence::AtEnd() {
  return mNextInactiveEvent == mEvents.Size() && mActiveEvents.Size() == 0;
}

void Sequence::Update(float dt) {
  if (AtEnd()) {
    return;
  }
  ScrubUp(mTimePassed + dt);
}

void Sequence::ScrubUp(float time) {
  // Activate any events that haven't started.
  while (mNextInactiveEvent < mEvents.Size()) {
    const Event& event = mEvents[mNextInactiveEvent];
    if (time < event.mStartTime) {
      break;
    }
    mActiveEvents.Push(mNextInactiveEvent);
    ++mNextInactiveEvent;
  }

  // Run all active events and removed finished ones.
  for (int i = 0; i < mActiveEvents.Size(); ++i) {
    const Event& event = mEvents[mActiveEvents[i]];
    if (time > event.mEndTime) {
      event.Run(1.0f);
      mActiveEvents.Remove(i);
      --i;
    }
    else {
      float eventDuration = event.mEndTime - event.mStartTime;
      float passedDuration = time - event.mStartTime;
      float t = passedDuration / eventDuration;
      event.Run(t);
    }
  }

  if (AtEnd()) {
    mTimePassed = mTotalTime;
  }
  else {
    mTimePassed = time;
  }
}

void Sequence::ScrubDown(float time) {
  // Put all events that haven't started at their starting position.
  mActiveEvents.Clear();
  for (int i = mNextInactiveEvent - 1; i >= 0; --i) {
    const Event& event = mEvents[i];
    event.Run(0.0f);
    if (time > event.mStartTime) {
      break;
    }
    mNextInactiveEvent = i;
  }

  // Find all of the active events and put them at their current positions.
  for (int i = 0; i < (int)mNextInactiveEvent; ++i) {
    const Event& event = mEvents[i];
    if (time >= event.mStartTime && time < event.mEndTime) {
      float eventDuration = event.mEndTime - event.mStartTime;
      float passedDuration = time - event.mStartTime;
      float t = passedDuration / eventDuration;
      event.Run(t);
      mActiveEvents.Push(i);
    }
  }
  if (time < 0.0f) {
    mTimePassed = 0.0f;
  }
  else {
    mTimePassed = time;
  }
}

void Sequence::Scrub(float time) {
  if (time > mTimePassed) {
    ScrubUp(time);
  }
  else {
    ScrubDown(time);
  }
}
