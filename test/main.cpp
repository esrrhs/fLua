#include "diff_lua.h"
#include "stdio.h"

class MyLogger : public DiffLoggerInterface {
public:
    virtual LogLevel Level() {
        return DEBUG;
    }

    virtual void Log(LogLevel level, const char *file, int line, const char *func, const char *msg) {
        printf("%s:%d %s %s\n", file, line, func, msg);
    }
};

int main(int argc, char *argv[]) {
    MyLogger logger;

    auto src = DiffVarPoolAlloc();
    auto input = DiffVarPoolAlloc();
    auto diff = calc_env_diff(&logger, src, input);
    if (!diff) {
        printf("calc_env_diff failed\n");
        return -1;
    }

    auto diff_str = diff->DiffDump(0);
    printf("diff is %s", diff_str.c_str());

    DiffVarPoolReset(src);
    return 0;
}
