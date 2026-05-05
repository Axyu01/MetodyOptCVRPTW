#ifndef LOGGER_H
#define LOGGER_H
#include <iostream>
#include <fstream>
#include "Solution.h"
#include <string>

using namespace std;
class Logger
{
    public:
        Logger(string file_name);
        virtual ~Logger();
        void Log(Solution* s);
        void Log(int value);

    protected:
        fstream _file;

    private:
};

#endif // LOGGER_H
