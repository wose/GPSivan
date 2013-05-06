#include <iostream>
#include <unistd.h>

#include "gps_com.h"
#include "libgpsmm.h"

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
      if(_gpsd_con.stream(WATCH_ENABLE|WATCH_NMEA) == NULL)
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
              std::cout << "No data" << std::endl;
              continue;
            }

          if((new_data = _gpsd_con.read()) == NULL)
            {
              std::cout << "read error" << std::endl;
              continue;
            }
          else
            {
              std::cout << "got data" << std::endl;
              std::lock_guard<std::mutex> lock(_gps_data_mutex);
              if(new_data->fix.mode == MODE_2D or new_data->fix.mode == MODE_3D)
                std::cout << "latlon is set" << std::endl;
              _last_data = *new_data;
            }
        }
    }
}

void gps_com::stop()
{
  _run = false;
}

bool gps_com::get_latlon(double &lat, double &lon)
{
  std::lock_guard<std::mutex> lock(_gps_data_mutex);
  if (_last_data.fix.mode == MODE_2D or _last_data.fix.mode == MODE_3D)
    {
      lat= _last_data.fix.latitude;
      lon= _last_data.fix.longitude;
      return true;
    }
  else
    std::cout << _last_data.fix.mode << " " << _last_data.fix.latitude << std::endl;
  return false;
}
