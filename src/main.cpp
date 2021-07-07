#include "utils/logging.h"
#include "utils/timing.h"

int main() {
    PROF_START(main)
    default_logger->info("Hello, World");
    PROF_END(main)
    return 0;
}
