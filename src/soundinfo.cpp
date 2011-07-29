#include <string.h>
#include <sndfile.h>
#include <iostream>

namespace inoui
{

long int get_sound_file_duration(const std::string &file_name)
{
    SNDFILE *file;
    SF_INFO sfinfo;
    memset(&sfinfo, 0, sizeof(sfinfo));

    if ((file = sf_open(file_name.c_str(), SFM_READ, &sfinfo)) == NULL)
    {
        std::cerr << "Error : Not able to open input file " << file_name << std::endl;
        return 0;
    }
    sf_close(file);
    if (sfinfo.samplerate < 1)
        return 0;
    if (sfinfo.frames / sfinfo.samplerate > 0x7FFFFFFF)
        return 0;
    double seconds = (1.0 * sfinfo.frames) / sfinfo.samplerate;
    return static_cast<long int>(seconds * 1000);
}

}; // end of namespace

