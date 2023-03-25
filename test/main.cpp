#include "diff_lua.h"

int main(int argc, char* argv[])
{
    auto env = new_diff_env();
    free_diff_env(env);
    return 0;
}
