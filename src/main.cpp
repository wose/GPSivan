#include <iostream>
#include <cstdio>
#include <unistd.h>
#include <thread>

#include "gps_com.h"

#define WAITING_TIME 5000000
#define RETRY_TIME 5
#define ONE_SECOND 1000000


gps_com gps;

void update_gps();

int main(void)
{
  
  std::thread gps_thread(update_gps);

  while(getchar() != 'q')
    {
      double lat, lon;
      if(gps.get_latlon(lat, lon))
        std::cout << "Lat: " << lat << " Lon: " << lon << std::endl;
    }
  
  gps.stop();
  gps_thread.join();

  return 0;
}

void update_gps()
{
  gps.update();
}
