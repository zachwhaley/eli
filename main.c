#include "eli.h"

int main(int argc, const char *argv[])
{
    struct Eli *eli = eli_init(argc, argv);
    eli_run(eli);
    eli_term(eli);
    return 0;
}
