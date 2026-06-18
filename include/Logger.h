#ifndef LOGGER_H
#define LOGGER_H
#include "Solution.h"
#include <fstream>
#include <iostream>
#include <string>

using namespace std;
class Logger {
public:
  Logger(string file_name);
  virtual ~Logger();
  void Log(Solution *s);
  void Log(double value);
  void Log(int gen, double best, double avg, double worst);

protected:
  fstream _file;

private:
};

#endif // LOGGER_H
