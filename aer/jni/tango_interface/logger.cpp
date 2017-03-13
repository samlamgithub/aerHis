#include "logger.hpp"
#include "util.hpp"

#include <ctime>
#include <cstdarg>
#include <jni.h>
#include <tango_client_api.h>
#include <sys/stat.h>


namespace tango_interface {

Logger::Logger() {
  std::string filename("/sdcard/imperial_tango_" + current_date_time());
  // Ensure we can open multiple files within a second
  int version = 0;
  std::string version_suffix(".log");
  while (file_exists(filename + version_suffix)) {
    version_suffix = "_" + to_string(++version) + ".log";
  }
  // Finish opening the file
  filename += version_suffix;
  log_file_ = fopen(filename.c_str(),"w+");
  if (log_file_ == NULL) {
    LOGE("Logger: There was a problem opening the log file:%s",filename.c_str());
  }
}

Logger::~Logger() {
  close();
}

bool Logger::log(LogLevel level, const char* log_line, ...) {
  if (log_file_ != NULL) {
    fputs(level_string(level).c_str(),log_file_);
    fputs(current_date_time().c_str(),log_file_);
    fputs(":     ",log_file_);
    // Format the log line
    const int max_log_len = 2048;
    char buffer[max_log_len];
    va_list args;
    va_start(args,log_line);
    vsnprintf(buffer, max_log_len, log_line, args);
    va_end(args);
    fputs(buffer,log_file_);
    fputs("\n",log_file_);
    return true;
  }
  return false;
}

void Logger::close() {
  if (log_file_ != NULL) {
    fflush(log_file_);
    fclose(log_file_);
  }
}

bool Logger::file_exists(const std::string& filename) const {
  struct stat st;
  int result = stat(filename.c_str(), &st);
  return (result == 0);
}

std::string Logger::level_string(LogLevel level) const {
  switch (level) {
  case kError   : return std::string("ERROR  ");
  case kWarning : return std::string("WARNING");
  case kInfo    : return std::string("INFO   ");
  case kDebug   : return std::string("DEBUG  ");
  default       : return std::string("UNKNOWN");
  }
}

std::string Logger::current_date_time() const {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d_%H-%M-%S", &tstruct);
    return std::string(buf);
}

std::string Logger::to_string(int value) const {
  char buf[128];
  snprintf(buf,128,"%d",value);
  return std::string(buf);
}

}
