#include "diff_lua.h"

extern "C" DiffEnv *new_diff_env() {
    return new DiffEnv();
}

extern "C" void free_diff_env(DiffEnv *env) {
    delete env;
}
