#include <iostream>
#include "soundinfo.h"

int main(int argc, char *argv[])
{
    long int duration = get_sound_file_duration("test.wav");
    std::cout << "Duration is " << duration << " ms" << std::endl;
    return 0;
}


