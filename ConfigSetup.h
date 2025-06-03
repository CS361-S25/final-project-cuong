#ifndef CONFIGSETUP_H
#define CONFIGSETUP_H

#include "emp/config/ArgManager.hpp"
#include "emp/prefab/ConfigPanel.hpp"
#include "emp/web/UrlParams.hpp"

// Lab: https://anyaevostinar.github.io/classes/361-s25/emp_config_lab
// Guide: https://anyaevostinar.github.io/classes/361-s25/config_intro
EMP_BUILD_CONFIG(MyConfigType,
    VALUE(FILE_NAME, std::string, "_data.dat", "Root output file name"),
    VALUE(SEED, int, 5, "What value should the random seed be?"), 
    VALUE(START_NUM, int, 1, "How many organisms should the world be populated with at first?"),
    VALUE(UPDATE_NUM, int, 3000000, "How many updates should be done for the world state?"),
    VALUE(UPDATE_RECORD_FREQUENCY, int, 50, "How many updates should be done between each datafile record entry?"),
    VALUE(WORLD_LEN, int, 60, "How long is the world?"),
    VALUE(WORLD_WIDTH, int, 60, "How wide is the world?"),
    VALUE(CELL_SIZE, int, 10, "How large is each cell in the world?"),
    VALUE(MUTATION_RATE, float, 0.0075, "How likely wil each genome bit will be mutated?"),
    VALUE(MAX_BRIGHT,    float,   1,   "How bright (0-1) is the orgainsm with the most points?" ),
    VALUE(MIN_BRIGHT,    float, 0.8,   "How bright (0-1) is the orgainsm with the least points?" ),
)

extern MyConfigType worldConfig;

#endif