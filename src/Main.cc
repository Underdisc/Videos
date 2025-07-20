#include <Input.h>
#include <Result.h>
#include <Temporal.h>
#include <VarkorMain.h>
#include <comp/AlphaColor.h>
#include <comp/Mesh.h>
#include <comp/Text.h>
#include <debug/Draw.h>
#include <ds/Vector.h>
#include <editor/Editor.h>
#include <editor/LayerInterface.h>
#include <gfx/Font.h>
#include <imgui/imgui.h>
#include <math.h>
#include <math/Constants.h>
#include <math/Utility.h>
#include <util/Utility.h>
#include <world/Registrar.h>
#include <world/World.h>

#include "QuickHull.h"
#include "Sequence.h"

Video gVid;
void CentralUpdate() {
  gVid.mSeq.Update(Temporal::DeltaTime());
}

void EditorExtension() {
  ImGui::Begin("Sequence");
  float time = gVid.mSeq.mTimePassed;
  ImGui::PushItemWidth(-1.0f);
  ImGui::SliderFloat("Time", &time, 0.0f, gVid.mSeq.mTotalTime);
  if (time != gVid.mSeq.mTimePassed) {
    World::nPause = true;
    gVid.mSeq.Scrub(time);
  }
  ImGui::End();
}

int main(int argc, char* argv[]) {
  Options::Config config;
  config.mEditorLevel = Options::EditorLevel::Complete;
  config.mProjectDirectory = PROJECT_DIRECTORY;
  config.mWindowName = "Videos";
  Result result = VarkorInit(argc, argv, std::move(config));
  LogAbortIf(!result.Success(), result.mError.c_str());

  result = QuickHull(&gVid);
  LogAbortIf(!result.Success(), result.mError.c_str());
  World::nCentralUpdate = CentralUpdate;
  Editor::nExtension = EditorExtension;
  VarkorRun();
  VarkorPurge();
}
