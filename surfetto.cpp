#include "surfetto.h"

#include <fstream>
#include <cassert>
#include <iostream>

namespace surfetto {

bool g_tracing_enabled = false;
std::unique_ptr<char[]> g_trace_buffer = nullptr;
size_t g_trace_buffer_size = 0;
size_t g_trace_buffer_capacity = 0;

bool StartTracing(size_t buffer_capacity, bool force) {
  if (!force && g_tracing_enabled) return false;
  g_tracing_enabled = false;
  g_trace_buffer = std::make_unique<char[]>(buffer_capacity);
  g_trace_buffer_capacity = buffer_capacity;
  g_trace_buffer_size = 0;
  g_tracing_enabled = true;
  return true;
}

class TraceToFileWriter {
 public:
  TraceToFileWriter(const char* filename)
    : os(filename, std::ofstream::binary) { }

  void Write() {
    while (read_ptr < g_trace_buffer_size) {
      EventType type;
      ReadBuffer(&type);
      WriteInFile(type);
      if (type == EventType::BEGIN || type == EventType::INSTANT) {
        const char* event_name;
        ReadBuffer(&event_name);
        WriteInFile(event_name);
        // int16_t event_id = GetEventId(event_name, &is_new);
        // flags |= MaskType::HAS_EVENT_ID;
        // if (is_new) flags |=  MaskType::HAS_EVENT_NAME;
        // WriteInFile(event_id);
      }
      int64_t timestamp = 0;
      ReadBuffer(&timestamp);
      WriteInFile(timestamp);
    }
  }

 private:
  template<typename T>
  std::enable_if_t<std::is_fundamental<T>::value || std::is_enum<T>::value ||
                   std::is_pointer<T>::value>
  ReadBuffer(T* t) {
    std::memcpy(t, g_trace_buffer.get() + read_ptr, sizeof(T));
    read_ptr += sizeof(T);
  }

  template<typename T>
  std::enable_if_t<std::is_fundamental<T>::value || std::is_enum<T>::value>
  WriteInFile(T x) {
    os.write((const char*)&x, sizeof(T));
  }

  void WriteInFile(const char* str) {
    std::string_view sv(str);
    WriteInFile(uint16_t(sv.size()));
    os.write(str, sv.size());
  }

 private:
  size_t read_ptr = 0;
  std::ofstream os;
};

// Write trace to file after preprocessing.
void WriteTraceToFile(const char* filename) {
  TraceToFileWriter(filename).Write();
}

template<typename T>
std::enable_if_t<std::is_fundamental<T>::value || std::is_enum<T>::value>
static ReadFromFile(std::ifstream& file, T* x) {
  file.read((char*)x, sizeof(T));
}

static void ReadFromFile(std::ifstream& file, std::string* x) {
  int16_t size = 0;
  ReadFromFile(file, &size);
  x->resize(size);
  file.read(x->data(), size);
}

bool TraceFileReader::ReadNext(Event& event) {
  ReadFromFile(file, &event.type);
  if (!file) return false;
  if (event.type == EventType::BEGIN || event.type == EventType::INSTANT) {
    ReadFromFile(file, &event.name);
  }
  ReadFromFile(file, &event.timestamp);
  return true;
}

void Hierarchy::Make(const char* filename) {
  TraceFileReader reader(filename);
  Event event;
  int depth = 0;
  std::vector<std::pair<int64_t, size_t>> call_stack;
  while (reader.ReadNext(event)) {
    if (event.type == EventType::BEGIN) {
      call_stack.push_back({event.timestamp, slices.size()});
      slices.push_back({event.name, depth});
      depth++;
    } else if (event.type == EventType::END) {
      depth--;
      assert(call_stack.size() > 0);
      auto& tmp = call_stack.back();
      slices[tmp.second].duration = event.timestamp - tmp.first;
      call_stack.pop_back();
    } else { assert(false); }
  }
}


void Hierarchy::Print() const {
  for (auto& slice : slices) {
    std::cout << std::string(slice.depth, ' ') << slice.event_name << ": "
              << slice.duration << std::endl;
  }
}

// Event ReadEvent() {
//   uint8_t flags = ReadBytes(1);
//   event.type = EventType(flags & Masks::EVENT_TYPE);
//   if (flags & Masks::HAS_EVENT_ID) {
//     event.event_id = ReadBytes(2);
//   }
//   if (flags & Masks::HAS_EVENT_NAME) {
//     assert(event.event_id > 0);
//     uint16_t size = ReadBytes(2);
//     event_name_map[event.event_id] = std::string(ReadBytes(size), size);
//   }
// }

}  // namespace surfetto
