#ifndef _GPS_COM_H_
#define _GPS_COM_H_

#include <atomic>
#include <thread>
#include <libgpsmm.h>
#include <mutex>

class gps_com
{
 public:
  gps_com();
  ~gps_com();

 private:
  bool _run = true;
  gpsmm _gpsd_con;
  gps_data_t _last_data;
  std::mutex _gps_data_mutex;

  const static unsigned int _retry_time = 5;
  const static unsigned int _one_second = 1000000;
  const static unsigned int _waiting_time = 5000000;
  
 public:
  void update();
  bool get_latlon(double &lat, double &lon);
  void stop();
};

extern gps_com gps;

#endif
