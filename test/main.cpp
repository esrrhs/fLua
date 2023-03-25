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
    auto env = new_diff_env();
    MyLogger logger;

    DiffVarInterface *input = 0;
    DiffVarInterface *diff = 0;
    if (calc_env_diff(env, &logger, input, diff) != 0) {
        printf("calc_env_diff failed\n");
        return -1;
    }

    free_diff_env(env);
    return 0;
}
