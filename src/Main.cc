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
#include <imgui/imgui.h>
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
  using namespace Comp;
  Type<Line>::Register();
  Type<Line>::AddDependencies<Comp::Model>();
  Type<Bracket>::Register();
  Type<Bracket>::AddDependencies<Comp::Transform>();
  Type<Box>::Register();
  Type<Box>::AddDependencies<Comp::Model>();
  Type<Arrow>::Register();
  Type<Arrow>::AddDependencies<Comp::Model>();
  Type<Table>::Register();
  Type<Table>::AddDependencies<Comp::Transform>();
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
