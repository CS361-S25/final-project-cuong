// Compile with `c++ -std=c++17 -Isignalgp-lite/include native.cpp`

#include <iostream>

#include "World.h"
#include "ConfigSetup.h"
MyConfigType worldConfig;

// This is the main function for the NATIVE version of this project.

int main(int argc, char *argv[]) {
  bool success = worldConfig.Read("MySettings.cfg");
  if(!success) worldConfig.Write("MySettings.cfg");
  emp::Random random(worldConfig.SEED());
  
  OrgWorld world(random);
  // Some SignalGP-Lite functionality uses its own emp::Random instance
  // so it's important to set that seed too when the main Random is created
  sgpl::tlrand.Get().ResetSeed(worldConfig.SEED());

  for (int i = 0; i < worldConfig.START_NUM(); i++) {
    Organism* new_org = new Organism(&world);
    world.Inject(*new_org);
  }
  world.Resize(worldConfig.WORLD_LEN(), worldConfig.WORLD_WIDTH());

  world.SetupSolveFile("solveCountNative.data").SetTimingRepeat(worldConfig.UPDATE_RECORD_FREQUENCY());

  for (int update = 0; update < worldConfig.UPDATE_NUM(); update++) {
    world.Update();
  }

}
