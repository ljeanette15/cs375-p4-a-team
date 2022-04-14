#include "sliding_window.h"

int main(int argc, char* argv[])
{

    if (argc == 3)
    {
        send(argv[1]);
    }

    else if (argc == 2)
    {
        receive(argv[1]);
    }

    else
    {
        fprintf(stderr, "usage: sliding_window hostname");
        exit(1);
    }

    return 0;
}