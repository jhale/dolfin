// Copyright (C) 2003-2012 Anders Logg
//
// This file is part of DOLFIN.
//
// DOLFIN is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// DOLFIN is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with DOLFIN. If not, see <http://www.gnu.org/licenses/>.
//
// Modified by Ola Skavhaug, 2007, 2009.
// Modified by Garth N. Wells, 2011.
//
// First added:  2003-03-13
// Last changed: 2013-11-15


#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unistd.h>

#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>

#ifdef __linux__
#include <sys/types.h>
#include <unistd.h>
#endif

#include <dolfin/common/constants.h>
#include <dolfin/common/defines.h>
#include <dolfin/common/MPI.h>
#include <dolfin/parameter/GlobalParameters.h>
#include "LogLevel.h"
#include "Logger.h"

using namespace dolfin;

typedef std::map<std::string, std::pair<std::size_t, double> >::iterator
map_iterator;
typedef std::map<std::string, std::pair<std::size_t, double> >::const_iterator
const_map_iterator;

// Function for monitoring memory usage, called by thread
#ifdef __linux__
void _monitor_memory_usage(dolfin::Logger* logger)
{
  assert(logger);

  // Open statm
  //std::fstream

  // Get process ID and page size
  const std::size_t pid = getpid();
  const size_t page_size = getpagesize();

  // Print some info
  std::stringstream s;
  s << "Initializing memory monitor for process " << pid << ".";
  logger->log(s.str());

  // Prepare statm file
  std::stringstream filename;
  filename << "/proc/" << pid << "/statm";
  std::ifstream statm;

  // Enter loop
  while (true)
  {
    // Sleep for a while
    boost::this_thread::sleep(boost::posix_time::seconds(1));

    // Read number of pages from statm
    statm.open(filename.str().c_str());
    if (!statm)
      logger->error("Unable to open statm file for process.");
    size_t num_pages;
    statm >> num_pages;
    statm.close();

    // Convert to MB and report memory usage
    const size_t num_mb = num_pages*page_size / (1024*1024);
    logger->_report_memory_usage(num_mb);
  }
}
#endif

//-----------------------------------------------------------------------------
Logger::Logger() : _active(true), _log_level(INFO), indentation_level(0),
                   logstream(&std::cout), _maximum_memory_usage(-1)
{
  // Do nothing
}
//-----------------------------------------------------------------------------
Logger::~Logger()
{
  // Join memory monitor thread if it exists
  if (_thread_monitor_memory_usage)
    _thread_monitor_memory_usage->join();
}
//-----------------------------------------------------------------------------
void Logger::log(std::string msg, int log_level) const
{
  write(log_level, msg, -1);
}
//-----------------------------------------------------------------------------
void Logger::log_underline(std::string msg, int log_level) const
{
  if (msg.empty())
    log(msg, log_level);

  std::stringstream s;
  s << msg;
  s << "\n";
  for (int i = 0; i < indentation_level; i++)
    s << "  ";
  for (std::size_t i = 0; i < msg.size(); i++)
    s << "-";

  log(s.str(), log_level);
}
//-----------------------------------------------------------------------------
void Logger::warning(std::string msg) const
{
  std::string s = std::string("*** Warning: ") + msg;
  write(WARNING, s, -1);
}
//-----------------------------------------------------------------------------
void Logger::error(std::string msg) const
{
  std::string s = std::string("*** Error: ") + msg;
  throw std::runtime_error(s);
}
//-----------------------------------------------------------------------------
void Logger::dolfin_error(std::string location,
                          std::string task,
                          std::string reason,
                          int mpi_rank) const
{
  std::string _mpi_rank = boost::lexical_cast<std::string>(mpi_rank);
  if (mpi_rank < 0)
    _mpi_rank = "unknown";

  std::stringstream s;
  s << std::endl << std::endl
    << "*** "
    << "-------------------------------------------------------------------------"
    << std::endl
    << "*** DOLFIN encountered an error. If you are not able to resolve this issue"
    << std::endl
    << "*** using the information listed below, you can ask for help at"
    << std::endl
    << "***" << std::endl
    << "***     fenics@fenicsproject.org"
    << std::endl
    << "***" << std::endl
    << "*** Remember to include the error message listed below and, if possible,"
    << std::endl
    << "*** include a *minimal* running example to reproduce the error."
    << std::endl
    << "***" << std::endl
    << "*** "
    << "-------------------------------------------------------------------------"
    << std::endl
    << "*** " << "Error:   Unable to " << task << "." << std::endl
    << "*** " << "Reason:  " << reason << "." << std::endl
    << "*** " << "Where:   This error was encountered inside " << location << "."
    << std::endl
    << "*** " << "Process: " << _mpi_rank << std::endl
    << "*** " << std::endl
    << "*** " << "DOLFIN version: " << dolfin_version()  << std::endl
    << "*** " << "Git changeset:  " << git_commit_hash() << std::endl
    << "*** "
    << "-------------------------------------------------------------------------"
    << std::endl;

  throw std::runtime_error(s.str());
}
//-----------------------------------------------------------------------------
void Logger::deprecation(std::string feature,
                         std::string version_deprecated,
                         std::string version_remove,
                         std::string message) const
{
  std::stringstream s;
  s << "*** "
    << "-------------------------------------------------------------------------"
    << std::endl
    << "*** Warning: " << feature << " has been deprecated in DOLFIN version "
    << version_deprecated << "." << std::endl
    << "*** It will be removed from version " << version_remove << "."
    << std::endl
    << "*** " << message << std::endl
    << "*** "
    << "-------------------------------------------------------------------------"
    << std::endl;

  #ifdef DOLFIN_DEPRECATION_ERROR
  error(s.str());
  #else
  write(WARNING, s.str(), -1);
  #endif
}
//-----------------------------------------------------------------------------
void Logger::begin(std::string msg, int log_level)
{
  // Write a message
  log(msg, log_level);
  indentation_level++;
}
//-----------------------------------------------------------------------------
void Logger::end()
{
  indentation_level--;
}
//-----------------------------------------------------------------------------
void Logger::progress(std::string title, double p) const
{
  std::stringstream line;
  line << title << " [";

  const int N = DOLFIN_TERM_WIDTH - title.size() - 12 - 2*indentation_level;
  const int n = static_cast<int>(p*static_cast<double>(N));

  for (int i = 0; i < n; i++)
    line << '=';
  if (n < N)
    line << '>';
  for (int i = n+1; i < N; i++)
    line << ' ';

  line << std::setiosflags(std::ios::fixed);
  line << std::setprecision(1);
  line << "] " << 100.0*p << '%';

  write(PROGRESS, line.str(), -1);
}
//-----------------------------------------------------------------------------
void Logger::set_output_stream(std::ostream& ostream)
{
  logstream = &ostream;
}
//-----------------------------------------------------------------------------
void Logger::set_log_active(bool active)
{
  _active = active;
}
//-----------------------------------------------------------------------------
void Logger::set_log_level(int log_level)
{
  _log_level = log_level;
}
//-----------------------------------------------------------------------------
void Logger::register_timing(std::string task, double elapsed_time)
{
  // Remove small or negative numbers
  if (elapsed_time < DOLFIN_EPS)
    elapsed_time = 0.0;

  // Print a message
  std::stringstream line;
  line << "Elapsed time: " << elapsed_time << " (" << task << ")";
  log(line.str(), TRACE);

  // Store values for summary
  map_iterator it = _timings.find(task);
  if (it == _timings.end())
  {
    std::pair<std::size_t, double> timing(1, elapsed_time);
    _timings[task] = timing;
  }
  else
  {
    it->second.first += 1;
    it->second.second += elapsed_time;
  }
}
//-----------------------------------------------------------------------------
void Logger::list_timings(bool reset)
{
  // Check if timings are empty
  if (_timings.empty())
  {
    log("Timings: no timings to report.");
    return;
  }
  else
  {
    log("");
    log(timings(reset).str(true));
  }

  // Print maximum memory usage if available
  if (_maximum_memory_usage >= 0)
  {
    std::stringstream s;
    s << "\nMaximum memory usage: " << _maximum_memory_usage << " MB";
    log(s.str());
  }

}
//-----------------------------------------------------------------------------
Table Logger::timings(bool reset)
{
  // Generate timing table
  Table table("Summary of timings");
  for (const_map_iterator it = _timings.begin(); it != _timings.end(); ++it)
  {
    const std::string task    = it->first;
    const std::size_t num_timings    = it->second.first;
    const double total_time   = it->second.second;
    const double average_time = total_time / static_cast<double>(num_timings);

    table(task, "Average time") = average_time;
    table(task, "Total time")   = total_time;
    table(task, "Reps")         = num_timings;
  }

  // Clear timings
  if (reset)
    _timings.clear();

  return table;
}
//-----------------------------------------------------------------------------
double Logger::timing(std::string task, bool reset)
{
  // Find timing
  map_iterator it = _timings.find(task);
  if (it == _timings.end())
  {
    std::stringstream line;
    line << "No timings registered for task \"" << task << "\".";
    dolfin_error("Logger.cpp",
                 "extract timing for task",
                 line.str());
  }

  // Compute average
  const std::size_t num_timings  = it->second.first;
  const double total_time   = it->second.second;
  const double average_time = total_time / static_cast<double>(num_timings);

  // Clear timing
  _timings.erase(it);

  return average_time;
}
//-----------------------------------------------------------------------------
void Logger::monitor_memory_usage()
{
  #ifndef __linux__
  warning("Unable to initialize memory monitor; only available on GNU/Linux.");
  return;

  #else

  // Check that thread has not already been started
  if (_thread_monitor_memory_usage)
  {
    log("Memory monitor already initialize.");
    return;
  }

  // Create thread
  _thread_monitor_memory_usage.reset(new boost::thread(boost::bind(&_monitor_memory_usage, this)));

  #endif
}
//-----------------------------------------------------------------------------
void Logger::_report_memory_usage(size_t num_mb)
{
  std::stringstream s;
  s << "Memory usage: " << num_mb << " MB";
  log(s.str());
  _maximum_memory_usage = std::max(_maximum_memory_usage,
                                   static_cast<long int>(num_mb));
}
//-----------------------------------------------------------------------------
void Logger::__debug(std::string msg) const
{
  std::string s = std::string("DEBUG: ") + msg;
  write(DBG, s, -1);
}
//-----------------------------------------------------------------------------
void Logger::__dolfin_assert(std::string file, unsigned long line,
                             std::string function, std::string check) const
{
  std::stringstream location;
  location << file << " (line " << line << ")";
  std::stringstream task;
  task << "complete call to function " << function << "()";
  std::stringstream reason;
  reason << "Assertion " << check << " failed";
  dolfin_error(location.str(), task.str(), reason.str());
}
//-----------------------------------------------------------------------------
void Logger::write(int log_level, std::string msg, int rank) const
{
  // Check log level
  if (!_active || log_level < _log_level)
    return;

  // Check if we want output on root process only
  const bool std_out_all_processes = parameters["std_out_all_processes"];
  if (rank > 0 && !std_out_all_processes && log_level < WARNING)
    return;

  // Prefix with process number if running in parallel
  if (rank >= 0)
  {
    std::stringstream prefix;
    prefix << "Process " << rank << ": ";
    msg = prefix.str() + msg;
  }

  // Add indentation
  for (int i = 0; i < indentation_level; i++)
    msg = "  " + msg;

  // Write to stream
  *logstream << msg << std::endl;
}
//----------------------------------------------------------------------------
