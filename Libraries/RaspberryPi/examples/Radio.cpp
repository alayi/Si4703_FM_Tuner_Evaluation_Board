/*
Copyright (C) 2013 Christoph Thoma (thoma.christoph@gmx.at)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "../src/SparkFunSi4703.h"
#include <chrono>
#include <iostream>
#include <thread>

using std::cerr;
using std::cout;
using std::endl;

void rdsPrintFunc(Si4703_Breakout* radio) {
  const auto duration = std::chrono::seconds(15);
  const auto end_time = std::chrono::system_clock::now() + duration;
  cout << "RDS: " << std::flush;
  char rds_buffer[9];
  std::mutex mtx;
  std::unique_lock<std::mutex> lck(mtx);
  while (true) {
    if (std::chrono::system_clock::now() > end_time)
      return;
    if (radio->rdsCV().wait_for(lck, std::chrono::milliseconds(1000)) ==
        std::cv_status::timeout)
      continue;
    radio->getRDS(rds_buffer);
    cout << "\33[2K\r";
    cout << "RDS: " << rds_buffer << std::flush;
  }
}

int main(int argc, const char** argv) {
  if (argc != 2) {
    cerr << "usage:" << endl;
    cerr << "  Radio <freq>" << endl;
    cerr << "where:" << endl;
    cerr << "  freq: frequency in MHz (e.g. 103.7)" << endl;
    return 1;
  }
  float frequency = atof(argv[1]);
  int resetPin = 23;  // GPIO_23.
  int sdaPin = 0;     // GPIO_0 (SDA).

  Si4703_Breakout radio(resetPin, sdaPin);
  radio.powerOn();
  radio.setVolume(5);
  radio.setFrequency(frequency);
  cout << "Listening to station " << radio.getFrequency() << " MHz" << endl;

  std::thread th(&rdsPrintFunc, &radio);
  th.join();

  cout << endl;
  radio.printRegisters();

  return 0;
}
