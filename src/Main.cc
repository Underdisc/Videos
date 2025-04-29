// Varkor Commit Hash: b2fb779

#include <Input.h>
#include <Result.h>
#include <Temporal.h>
#include <VarkorMain.h>
#include <comp/AlphaColor.h>
#include <comp/Camera.h>
#include <comp/Mesh.h>
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
#include <world/Registrar.h>
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
  RegisterComponent(Line);
  RegisterDependencies(Line, Mesh);
  RegisterComponent(Bracket);
  RegisterDependencies(Bracket, Transform);
  RegisterComponent(Box);
  RegisterDependencies(Box, Mesh);
  RegisterComponent(Arrow);
  RegisterDependencies(Arrow, Mesh);
  RegisterComponent(Table);
  RegisterDependencies(Table, Transform);
}

int main(int argc, char* argv[])
{
  Registrar::nRegisterCustomTypes = RegisterTypes;

  Options::Config config;
  config.mEditorLevel = Options::EditorLevel::Complete;
  config.mProjectDirectory = PROJECT_DIRECTORY;
  config.mWindowName = "Videos";
  Result result = VarkorInit(argc, argv, std::move(config));
  LogAbortIf(!result.Success(), result.mError.c_str());

  TheFundamentalsOfGraphics(&gSequence);
  World::nCentralUpdate = CentralUpdate;
  VarkorRun();

  VarkorPurge();
}
