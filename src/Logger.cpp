#include "Logger.h"
#include <string>
#include <stdlib.h>
using namespace std;

Logger::Logger(string file_name)
{
    _file.open(file_name,ios::out | ios::trunc);
    if(!_file.is_open())
        cout <<"Cannot open a " <<file_name <<" file!\n";
    else
        cout <<file_name <<" opened successfully!\n";
}

Logger::~Logger()
{
    _file.close();
}
void Logger::Log(Solution* s)
{
    _file << s->eval <<";";
    for(int g =0;g<s->size;g++)
    {
        _file << s->Genome[g]<<";";
    }
    _file <<"\n";
}
void Logger::Log(int value)
{
    _file << value <<";\n";
}
