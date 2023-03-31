#include "diff_lua.h"
#include "stdio.h"

int main(int argc, char *argv[]) {
    auto src = DiffVarPoolAlloc();
    auto input = DiffVarPoolAlloc();

    auto get_id = [](DiffVarInterface *v) {
        return v->DiffGetTableValue(DiffVarPoolAlloc()->DiffSetString("id", 2));
    };

    auto new_func = []() {
        return DiffVarPoolAlloc();
    };

    auto diff = CalDiff(src, input, get_id, new_func);
    if (!diff) {
        printf("calc_env_diff failed\n");
        return -1;
    }

    auto diff_str = diff->DiffDump(0);
    printf("diff is %s", diff_str.c_str());

    DiffVarPoolReset(src);
    return 0;
}
