
namespace surfetto {

struct FixSizedBuffer {
  FixSizedBuffer(size_t capacity)
    : capacity(capacity), buffer(std::make_unique<char[]>(capacity)) {}
  
  bool Add() {
    return true;
  }
  template<typename T, typename... Ts>
  std::enable_if_t<std::is_fundamental_type<T>::value, bool>
  Add(T arg, Ts... args) {


    return Add(args...);
  }

  std::unique_ptr<char[]> buffer;
  size_t capacity = 0;
};

void AddInTracingBuffer() {
  g_surperfetto_enabled = false;
}

enum class EventType : char {BEGIN='B', END='E', INSTANT='I', NONE='N'};

template<size_t N1>
bool TraceEvent(EventType type, char (category&)[N1], char (event_name&)[N2]) {
  if LIKELY(!g_surperfetto_enabled) return false;
  if LIKELY(!CategoryEnabled(category_name)) return false;
  return AddInGlobalBuffer(type, &event_name, GetTimeNowNs());
}

struct TraceScope {
  TraceScope(): active_(TraceEvent(category, event_name, std::forward<Ts>(args)...)) {
    ;
    buffer << 
  }
  ~TraceScope() {
    if (active_) {
      AddInGlobalBuffer(EventType::END, GetTimeNowNs());
    }
  }
  bool 
  const char* end_event_name;
}


struct Event {
  int event_id = 0;
  Type type = UNKNOWN;
};

Event ReadEvent() {
  uint8_t flags = ReadBytes(1);
  event.type = EventType(flags & Masks::EVENT_TYPE);
  if (flags & Masks::HAS_EVENT_ID) {
    event.event_id = ReadBytes(2);
  }
  if (flags & Masks::HAS_EVENT_NAME) {
    assert(event.event_id > 0);
    uint16_t size = ReadBytes(2);
    event_name_map[event.event_id] = std::string(ReadBytes(size), size);
  }


}

// Write trace to file after preprocessing.
void WriteTraceToFile() {
  IByteStream bs(g_buffer);
  while (!bs.empty()) {
    EventType type;
    bs >> type;
    if (type == 'B' || type == 'I') {
      bs >> ptr;
    }
    bs >> timestamp;
    TraceEventProto event;
  }
  
}



}  // namespace surfetto

#define TRACE_EVENT(category, event_name, ...) ::surfetto::TraceEvent(category,  event_name, VA_ARGS);

#define TRACE_SCOPE(category, ...) ::surfetto::TraceScope trace_scope_tmp_var_surfetto{category, __FUNCTION__ , VA_ARGS};
