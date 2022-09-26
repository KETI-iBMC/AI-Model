#include "WDT_stdx.hpp"
#include "watchdog.hpp"

int main(void) {
  cout << "KETI WDT Start .." << endl;
  auto wdt = KETI_Watchdog::SingleInstance();
  wdt->start();
  return 0;
}