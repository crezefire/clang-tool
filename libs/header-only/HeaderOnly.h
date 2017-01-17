#pragma once

template <typename T> class Increment {
public:
  void PlusPlus(T &var) { ++var; }
};