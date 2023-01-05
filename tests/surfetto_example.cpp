#include "surfetto.h"

#include <iostream>
#include <cassert>

inline void G() {
  TRACE_SCOPE();
}

inline void F() {
  TRACE_SCOPE();
  G();
  G();
  // G();
  // G();
  // G();
  // G();
  // for (int i = 0; i < 10; i++) {
  //   G();
  // }
}

inline void F2() {
  TRACE_SCOPE();
  F();
  F();
  F();
  F();
  F();
}

int main() {
  // std::array<int64_t, 20> t;
  // for (int i = 0; i < 20; i++) {
  //   t[i] = surfetto::GetTimeNowNs();
  // }
  // for (int i = 0; i < 19; i++) {
  //   std::cout << (t[i+1] - t[i]) << std::endl;
  // }

  surfetto::StartTracing(10'000'000L /* buffer capacity*/);
  F2();
  F2();
  F2();
  F2();
  F2();
  F2();
  surfetto::WriteTraceToFile("/tmp/abc");
  surfetto::Hierarchy h;
  h.Make("/tmp/abc");
  h.Print();
  // surfetto::TraceFileReader reader("/tmp/abc");
  // surfetto::Event event;
  // while (reader.ReadNext(event)) {
  //   std::cout << (event.type == surfetto::EventType::BEGIN ? "START": "END  ")
  //             << " ";
  //   std::cout << (event.type != surfetto::EventType::END ? event.name: "")
  //             << " : " << event.timestamp << std::endl;
  // }
}
