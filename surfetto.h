#pragma once

#include <memory>
#include <type_traits>
#include <cstring>
#include <fstream>
#include <chrono>
#include <vector>

namespace surfetto {

extern bool g_tracing_enabled;
extern std::unique_ptr<char[]> g_trace_buffer;
extern size_t g_trace_buffer_size;
extern size_t g_trace_buffer_capacity;

template<typename...> struct Typechain {};

template<typename...> struct SizeofSum {};

template<>
struct SizeofSum<> : std::integral_constant<size_t, 0> {};

template<typename T, typename... Ts>
struct SizeofSum<T, Ts...> : std::integral_constant<size_t, sizeof(T)+ SizeofSum<Ts...>::value> {};

inline void AddInTracingBufferImpl() {}

template<typename T, typename... Ts>
std::enable_if_t<std::is_fundamental<T>::value || std::is_enum<T>::value ||
                 std::is_pointer<T>::value>
inline AddInTracingBufferImpl(T arg, Ts... args) {
  std::memcpy(g_trace_buffer.get() + g_trace_buffer_size, &arg, sizeof(T));
  g_trace_buffer_size += sizeof(T);
  return AddInTracingBufferImpl(std::forward<Ts>(args)...);
}

template<typename... Ts>
inline bool AddInTracingBuffer(Ts&&... args) {
  if (g_trace_buffer_size + SizeofSum<Ts...>::value > g_trace_buffer_capacity) {
    g_tracing_enabled = false;
    return false;
  }
  AddInTracingBufferImpl(std::forward<Ts>(args)...);
  return true;
}

enum class EventType : uint8_t {
  UNKNOWN=0b00,
  INSTANT=0b01,
  BEGIN=0b10,
  END=0b11,
};

enum class MaskType: uint8_t {
  RESERVED_FOR_EVENT_TYPE = 0b11,
  HAS_EVENT_ID = 0b100,
  HAS_EVENT_NAME = 0b1000,
};

// struct StringConstant {
//   template<size_t N1>
//   static StringConstant Make(char (event_name&)[N1]) {
//     return {event_name};
//   }
//   StringConstant()
//   const char* str;
// }

inline int64_t GetTimeNowNs() {
  auto now = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::nanoseconds>(
      now.time_since_epoch()).count();
}

template<size_t N1>
inline bool TraceEvent(EventType type, const char (&event_name)[N1]) {
  if (!g_tracing_enabled) [[likely]] return false;
  return AddInTracingBuffer(type, (const char*)event_name, GetTimeNowNs());
}

template<size_t N1>
inline bool TraceEventInstant(const char (&event_name)[N1]) {
  return TraceEvent(EventType::INSTANT, event_name);
}


struct TraceScope {
  template<size_t N1>
  TraceScope(const char (&funcname)[N1])
    : active_(TraceEvent(EventType::BEGIN, funcname)) { }

  template<size_t N1, size_t N2>
  TraceScope(const char (&funcname)[N1], const char (&event_name)[N2])
    : active_(TraceEvent(EventType::BEGIN, event_name)) { }

  ~TraceScope() {
    if (active_) {
      AddInTracingBuffer(EventType::END, GetTimeNowNs());
    }
  }
  bool active_ = false;
};

struct Event {
  EventType type = EventType::UNKNOWN;
  int event_id = 0;
  std::string name;
  int64_t timestamp = 0;
};

bool StartTracing(size_t buffer_capacity, bool force=false);

// Write trace to file after preprocessing.
void WriteTraceToFile(const char* filename);

class TraceFileReader {
 public:
  TraceFileReader(const char* filename): file(filename, std::ifstream::binary) {}
  bool ReadNext(Event& event);

 private:
  std::ifstream file;
};

struct Hierarchy {
  struct Slice {
    std::string event_name;
    int depth = 0;
    int64_t duration = -1;
  };
  void Make(const char* filename);
  void Print() const;
  std::vector<Slice> slices;
};

}  // namespace surfetto

#define TRACE_EVENT(event_name) ::surfetto::TraceEventInstant(event_name)

#define TRACE_SCOPE(...) ::surfetto::TraceScope trace_scope_tmp_var_surfetto(\
  {__FUNCTION__ , __VA_ARGS__})
