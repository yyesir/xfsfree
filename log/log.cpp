/* log/log.cpp
 *
 * Copyright (C) 2007 Antonio Di Monaco
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include <iostream>
#include <iomanip>
#include <fstream>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <sys/timeb.h>
#include <unistd.h>

#include <thread>

#include "log/log.hpp"
#include "util/singleton.hpp"

using namespace Log;

class NullBuffer : public std::streambuf
{
public:
  int overflow(int c) { return c; }
};

std::string Logger::getPath()
{
    char szLogPath[ 512 ] = {0};
    char szLogName[128] = { 0x00 };

    memset( szLogPath, 0x0, sizeof(szLogPath));

    struct  tm  *ptm;
    struct  timeb stTimeb;
    static  char  szTime[128];

    memset( szTime, 0x0, sizeof(szTime) );
    ftime( &stTimeb );
    ptm = localtime( &stTimeb.time );
    sprintf(szLogName, "Manager_%04d-%02d-%02d.log", ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday);

    strcat(szLogPath, "/tmp/");
    strcat(szLogPath, szLogName);
    return std::string(szLogPath);
}

std::ostream &Logger::getFileStreamInstance()
{
  static std::ofstream f(getPath(), std::ios::out);

  return f;
}

std::ostream &Logger::getNullStreamInstance()
{
  static NullBuffer nullBuffer;
  static std::ostream nullStream(&nullBuffer);

  return nullStream;
}

bool Logger::isLogEnabled()
{
    return true;

//  CREATE_INSTANCE(LFS::Environment::Manager);
//  return INSTANCE(LFS::Environment::Manager).get(L"XFSPP_NO_LOG") != L"1";
}

std::ostream &Logger::streamInstance()
{
  static bool canLog = isLogEnabled();

  return (canLog)? getFileStreamInstance() : getNullStreamInstance();
}

Logger::Logger()
{

}

Logger::~Logger()
{
    struct  tm  *ptm;
    struct  timeb stTimeb;
    static  char  szTime[128];

    memset( szTime, 0x0, sizeof(szTime) );
    ftime( &stTimeb );
    ptm = localtime( &stTimeb.time );

    streamInstance() <<
      ptm->tm_year + 1900 << "-" <<
      std::setw(2) << std::setfill('0') << ptm->tm_mon + 1 << "-" <<
      std::setw(2) << std::setfill('0') << ptm->tm_mday << " " <<
      std::setw(2) << std::setfill('0') << ptm->tm_hour << ":" <<
      std::setw(2) << std::setfill('0') << ptm->tm_min << ":" <<
      std::setw(2) << std::setfill('0') << ptm->tm_sec << "." <<
      stTimeb.millitm << "\t" <<
      std::setw(6) << std::setfill('0') << getpid() << "\t" <<
      std::setw(6) << std::setfill('0') << std::this_thread::get_id() << "\t" <<
      str() << std::endl;
}

unsigned long GetTickCount()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ts.tv_sec*1000 + ts.tv_nsec / 1000000);
}

Method::Method(const std::string &fName, const std::string &params) :
  _tick(GetTickCount()),
  _fName(fName),
  _params(params)
{
  Logger() << "ENTER\t" << _fName << "\t" << _params;
}

Method::~Method()
{
  Logger() << "EXIT\t" << _fName << "\t" << _params << "\telapsed = " << (GetTickCount() - _tick);
}
