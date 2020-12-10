#include <restbed>

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <string>

#include "Utilities.h"

/**
 * @class CustomerLoger
 * @brief taken directly from the the [restbed
 * documentation](https://github.com/Corvusoft/restbed/blob/master/documentation/example/LOGGING.md)
 */
class CustomLogger : public restbed::Logger {
  using Level = restbed::Logger::Level;

 public:
  CustomLogger() {}
  /**
   * Halt/clean-up logger resources
   */
  void stop(void) { return; }

  /**
   * Initializes an instance of the CustomerLogger. No-op, but needed for the
   * restbed::Logger interface
   * @param Settings the settings for the session
   */
  void start(const std::shared_ptr<const restbed::Settings>&) { return; }

  /**
   * Prints a log message to the console of the format
   * <timestamp>: [<level>] <message>
   * @param level the restbed::Logger::Level of the message
   * @param format the format string provided to print
   * @param ... the arguments to print
   */
  void log(const Level level, const char* format, ...) {
    // Print the current server time in UTC
    std::string timestamp =
        TimeUtilities::ConvertTimeToString(TimeUtilities::CurrentTimeUTC());
    fprintf(stderr, "%s:", timestamp.c_str());

    // Print the log level
    fprintf(stderr, " [%s] ", LogLevel(level));

    va_list arguments;
    va_start(arguments, format);
    vfprintf(stderr, format, arguments);
    fprintf(stderr, "\n");
    va_end(arguments);
  }

  /**
   * Prints a log message, given a certain condition
   * @param expression the condition to meet to log the message
   * @param level the restbed::Logger::level of the message
   * @param format the format string provided to print
   * @param ... the argument list to print
   */
  void log_if(bool expression, const Level level, const char* format, ...) {
    if (expression) {
      va_list arguments;
      va_start(arguments, format);
      log(level, format, arguments);
      va_end(arguments);
    }
  }

 private:
  /**
   * @returns the c-string representation of a restbed::Logger::Level
   */
  const char* LogLevel(const Level& level) {
    switch (level) {
      case Level::DEBUG:
        return "DEBUG";
      case Level::FATAL:
        return "FATAL";
      case Level::ERROR:
        return "ERROR";
      case Level::WARNING:
        return "WARNING";
      case Level::SECURITY:
        return "SECURITY";
      default:
        return "INFO";
    }
  }
};
