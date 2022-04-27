// Varkor Commit Hash: b2fb779

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
#include <math.h>
#include <math/Constants.h>
#include <math/Utility.h>
#include <util/Utility.h>
#include <world/World.h>

#include "TheFundamentalsOfGraphics.h"

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
