#ifndef __LOGGER_HPP__
#define __LOGGER_HPP__

#include <stdio.h>
#include <string>

#include <zlib.h>

#include <cassert>
#include <iostream>
#include <limits>

//#include <opencv/cv.h>
#include <opencv2/opencv.hpp>
//#include <opencv2/core/core.hpp>

namespace tango_interface {

enum LogLevel { kError, kWarning, kInfo, kDebug };

class Logger {
public:
  Logger();
  ~Logger();

  bool log(LogLevel level, const char *log_line, ...);
  void close();

private:
  bool file_exists(const std::string &filename) const;
  std::string level_string(LogLevel level) const;
  std::string current_date_time() const;
  std::string to_string(int) const;

  FILE *log_file_;
};
}

#endif
