/*
There a lot of questions that still need answering when it comes to the
animation system. One of the big questions is, "How do we handle destroying and
creating objects?"
*/

struct Animator
{
}

void AnimatorFiller()
{
  for (int i = 0; i < 8; ++i) {
    // Instant event.
    CreateEvent line = Animator.CreateEvent(0.0f);
    line.CreateObject();
    Transform& transform = createLine.Add<Line>();
    transform& Animator.Wait();

    // Animation event.
    AEvent expandLine = Animator.NewEvent(0.0f, 1.0f, Ease::Linear);
    expandLine.Animate(Event::Type::Scale, 0.0f, 1.0f);
    Event deleteLine = Animator.NewEvent(1.0f);
    deleteLine.DeleteObject(line);
  }
}
