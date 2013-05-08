#include <iostream>
#include <unistd.h>

#include "gps_com.h"
#include "libgpsmm.h"

gps_com gps;

gps_com::gps_com() :
  _gpsd_con("localhost", DEFAULT_GPSD_PORT)
{
}

gps_com::~gps_com()
{
}

void gps_com::update()
{
  while(_run)
    {
      if(_gpsd_con.stream(WATCH_ENABLE|WATCH_JSON) == NULL)
        {
          std::cout << "No gpsd running?" << std::endl;
          usleep(_retry_time * _one_second);
          continue;
        }
      while(_run)
        {
          gps_data_t* new_data;
          if(!_gpsd_con.waiting(_waiting_time))
            {
              continue;
            }

          if((new_data = _gpsd_con.read()) == NULL)
            {
              continue;
            }
          else
            {
              std::lock_guard<std::mutex> lock(_gps_data_mutex);
              _last_data = *new_data;
            }
        }
    }
}

void gps_com::stop()
{
  _run = false;
}

bool gps_com::get_latlonvelalt(double &lat, double &lon, double &vel, double &alt)
{
  std::lock_guard<std::mutex> lock(_gps_data_mutex);
  if (_last_data.fix.mode >= MODE_2D)
    {
      lat = _last_data.fix.latitude;
      lon = _last_data.fix.longitude;
      vel = _last_data.fix.speed;
      alt = _last_data.fix.altitude;
      return true;
    }
  return false;
}

