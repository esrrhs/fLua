#pragma once

#include "lua.hpp"

class DiffEnv {

};

extern "C" DiffEnv *new_diff_env();

extern "C" void free_diff_env(DiffEnv *env);
