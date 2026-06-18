#include "Logger.h"
#include <stdlib.h>
#include <string>
using namespace std;

Logger::Logger(string file_name) {
  _file.open(file_name, ios::out | ios::trunc);
  if (!_file.is_open())
    cout << "[logger] cannot open file: " << file_name << '\n';
}

Logger::~Logger() { _file.close(); }
void Logger::Log(Solution *s) {
  _file << s->eval << ";";
  for (int g = 0; g < s->size; g++) {
    _file << s->Genome[g] << ";";
  }
  _file << "\n";
}
void Logger::Log(double value) { _file << value << ";\n"; }
void Logger::Log(int gen, double best, double avg, double worst) {
  _file << gen << ";" << best << ";" << avg << ";" << worst << "\n";
}
