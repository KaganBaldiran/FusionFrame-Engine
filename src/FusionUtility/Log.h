#pragma once
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <sstream>
#include "FusionDLLExport.h"

extern std::vector<std::string> logTraceBack;

#define FF_ERROR_CODE -0x100011
#define FF_SUCCESS_CODE 0x100012
#define FF_WARNING_CODE -0x100013
#define FF_INITIAL_CODE -0x130032

#define LOG(message) std::cout<< "LOG:: " << __FUNCTION__ <<" :: "<< message << std::endl
#define LOG_INF(message) std::cout<< "INFO:: " << __FUNCTION__<< " :: "<< message << std::endl
#define LOG_ERR(message) std::cerr<<"ERROR:: " << __FUNCTION__ <<" :: " << message << std::endl
#define LOG_WARN(message) std::cerr<<"WARNING:: " <<  __FUNCTION__<< " :: " << message << std::endl
#define LOG_TRACE(message) std::cout << "TRACE:: " << __FUNCTION__ << " :: " << message << std::endl
#define LOG_CRITICAL(message) std::cerr << "CRITICAL:: " << __FUNCTION__ << " :: " << message << std::endl
#define LOG_NOTICE(message) std::cout << "NOTICE:: " << __FUNCTION__ << " :: " << message << std::endl
#define LOG_VERBOSE(message) std::cout << "VERBOSE:: " << __FUNCTION__ << " :: " << message << std::endl
#define LOG_TIMESTAMP(message) std::cout << "[" << __TIME__ << "] " << __FUNCTION__ << " :: " << message << std::endl
#define LOG_PARAMETERS(...) std::cout << "PARAMETERS:: " << __FUNCTION__ << " :: " << #__VA_ARGS__ << " = " << __VA_ARGS__ << std::endl

#define LOG_ASSERT(condition, message) \
    if (!(condition)) { \
        std::cerr << "ASSERT FAILED:: " << __FUNCTION__ << " :: " << message << std::endl; \
    }


#define LOG_TRACEBACK(message) \
  {\
     std::ostringstream logstream; \
     logstream << "LOG:: " __FUNCTION__ " :: "<< message << std::endl; \
     std::string logMessage = logstream.str(); \
     std::cout << logMessage; \
     if(logTraceBack.size() <= 500) \
     { \
         logTraceBack.push_back(logMessage); \
     } \
     else \
     { \
        LOG_ERR("Exceeded the optimal log traceback count!") \
     } \
}

#define LOG_TO_FILE(filename, message) \
    { \
        std::ofstream logFile(filename, std::ios::app); \
        logFile << "LOG:: " << __FUNCTION__ << " :: " << message << std::endl; \
        logFile.close(); \
    }

#define LOG_TRACEBACK_PRINT_FILE(filename) \
    { \
       std::ofstream logFile(filename, std::ios::app); \
       for (auto &log : logTraceBack) \
       { \
           logFile << log;  \
       } \
       logFile.close(); \
}

class FUSIONFRAME_EXPORT FFexception : public std::exception
{
public:

    FFexception(const std::string& message) : message(message) {}
    const char* what() const noexcept override
    {
        return message.c_str();
    }
private:
    std::string message;
};