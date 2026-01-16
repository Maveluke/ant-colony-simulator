#pragma once
#include <iostream>

inline void ErrorLog(const std::string& msg) {
  //printf("[Error Log]: %s", &msg);
  std::cout << "[Error Log]: " << msg << std::endl;
}