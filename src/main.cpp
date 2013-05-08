#include <iostream>
#include <cstdio>
#include <unistd.h>
#include <thread>

#include "gps_com.h"
#include "glgui.h"
#include "encoder.h"


#define WAITING_TIME 5000000
#define RETRY_TIME 5
#define ONE_SECOND 1000000


//gps_com gps;
//glgui gui;

void update_gps();
void update_gui();

int main(void)
{
  //  if(!gui.init())
  //    return -1;

  std::thread gps_thread(update_gps);
  std::thread gui_thread(update_gui);

  enc.setup_encoder(17, 18);

  char c;

  do {
    c = getchar();
    switch(c)
      {
      case 'i':
        gui.zoom_in();
        break;
      case 'o':
        gui.zoom_out();
        break;
      }
  } while (c != 'q');

  gps.stop();
  gui.stop();

  gps_thread.join();
  gui_thread.join();

  return 0;
}

void update_gps()
{
  gps.update();
}

void update_gui()
{
  gui.update();
}
