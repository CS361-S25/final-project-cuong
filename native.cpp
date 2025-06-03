// Compile with `c++ -std=c++17 -Isignalgp-lite/include native.cpp`

#include <iostream>

#include "World.h"
#include "ConfigSetup.h"
MyConfigType worldConfig;

// This is the main function for the NATIVE version of this project.

int main(int argc, char *argv[]) {
  std::cout << "Test 0" <<std::endl;
  bool success = worldConfig.Read("MySettings.cfg");
  if(!success) worldConfig.Write("MySettings.cfg");
  emp::Random random(worldConfig.SEED());
  std::cout << "Test 1" <<std::endl;
  
  OrgWorld world(random);
  // Some SignalGP-Lite functionality uses its own emp::Random instance
  // so it's important to set that seed too when the main Random is created
  sgpl::tlrand.Get().ResetSeed(worldConfig.SEED());
  std::cout << "Test 2" <<std::endl;

  for (int i = 0; i < worldConfig.START_NUM(); i++) {
    Organism* new_org = new Organism(&world);
    world.Inject(*new_org);
  }
  std::cout << "Test 3" <<std::endl;
  // world.Resize(worldConfig.WORLD_LEN(), worldConfig.WORLD_WIDTH());

  world.SetupSolveFile("solveNative.data").SetTimingRepeat(worldConfig.UPDATE_RECORD_FREQUENCY());
  std::cout << "Test 4" <<std::endl;

  // for (int update = 0; update < worldConfig.UPDATE_NUM(); update++) {
  //   world.Update();
  // }
  while (1) {
    world.Update();
  }
  std::cout << "Test 5" <<std::endl;

}
