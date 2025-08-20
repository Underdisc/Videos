#include <Error.h>

#include "Video.h"

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
  if (mLerp) {
    t = Ease(t, mEase);
    mLerp(t);
  }
}

void Sequence::Add(const AddOptions& ao) {
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
  newEvent.mBegin = ao.mBegin;
  newEvent.mLerp = ao.mLerp;
  newEvent.mEnd = ao.mEnd;
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

void Sequence::ScrubUp(float scrubTime) {
  Assert(scrubTime > mTimePassed);

  // Activate any events that haven't started.
  while (mNextInactiveEvent < mEvents.Size()) {
    const Event& event = mEvents[mNextInactiveEvent];
    if (scrubTime < event.mStartTime) {
      break;
    }
    mActiveEvents.Push(mNextInactiveEvent);
    ++mNextInactiveEvent;
  }

  // Process all activated events and removed finished ones.
  Ds::Vector<unsigned int> remainingActiveEvents;
  for (int i = 0; i < mActiveEvents.Size(); ++i) {
    const Event& event = mEvents[mActiveEvents[i]];
    if (mTimePassed < event.mStartTime) {
      if (event.mBegin) event.mBegin(Cross::In);
    }
    if (scrubTime >= event.mEndTime) {
      if (event.mLerp) event.mLerp(1.0f);
      if (event.mEnd) event.mEnd(Cross::Out);
    }
    else {
      float eventDuration = event.mEndTime - event.mStartTime;
      float passedDuration = scrubTime - event.mStartTime;
      float t = passedDuration / eventDuration;
      event.Run(t);
      remainingActiveEvents.Push(mActiveEvents[i]);
    }
  }
  mActiveEvents = std::move(remainingActiveEvents);

  if (AtEnd()) {
    mTimePassed = mTotalTime;
  }
  else {
    mTimePassed = scrubTime;
  }
}

void Sequence::ScrubDown(float scrubTime) {
  Assert(scrubTime < mTimePassed);

  // Collect events that must be handled.
  mActiveEvents.Clear();
  for (int i = mNextInactiveEvent - 1; i >= 0; --i) {
    const Event& event = mEvents[i];
    if (scrubTime > event.mEndTime) {
      continue;
    }
    if (scrubTime <= event.mStartTime) {
      mNextInactiveEvent = i;
    }
    mActiveEvents.Push(i);
  }

  // Handle the events and remove events which the scrub time is outside of.
  Ds::Vector<unsigned int> remainingActiveEvents;
  for (int i = mActiveEvents.Size() - 1; i >= 0; --i) {
    const Event& event = mEvents[mActiveEvents[i]];
    if (mTimePassed >= event.mEndTime) {
      if (event.mEnd) event.mEnd(Cross::In);
    }
    if (scrubTime <= event.mStartTime) {
      if (event.mLerp) event.mLerp(0.0f);
      if (event.mBegin) event.mBegin(Cross::Out);
    }
    else {
      float eventDuration = event.mEndTime - event.mStartTime;
      float passedDuration = scrubTime - event.mStartTime;
      float t = passedDuration / eventDuration;
      event.Run(t);
      remainingActiveEvents.Push(mActiveEvents[i]);
    }
  }
  mActiveEvents = std::move(remainingActiveEvents);

  if (scrubTime < 0.0f) {
    mTimePassed = 0.0f;
  }
  else {
    mTimePassed = scrubTime;
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
