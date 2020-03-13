#include <cassert>
#include <cstdint>
#include <iostream>

#include <libstud/json/parser.hxx>

using namespace std;
using namespace stud::json;

// Parse the data in the specified stream mode returning true if the
// data is valid JSON and false otherwise.
//
static bool
parse (const void* data, size_t size, bool stream_mode_enabled,
       const std::string& stream_mode_separators)
try
{
  parser p (data, size, "fuzz_buffer", stream_mode_enabled,
            stream_mode_separators);
  double num;
  bool b;
  for (auto e : p)
  {
    switch (e)
    {
    case event::begin_object:
    case event::end_object:
    case event::begin_array:
    case event::end_array:
    case event::null:
    case event::name:
    case event::string:
      break;
    case event::boolean:
      b = p.value<bool> ();
      break;
    case event::number:
      num = p.value<double> ();
      break;
    }
  }
  return true;
}
catch (...)
{
  return false;
}

extern "C" int
LLVMFuzzerTestOneInput (const uint8_t* data, size_t size)
{
  // If it's valid in the strict mode, don't waste time parsing it in
  // relaxed.
  //
  if (!parse (data, size, false, ""))
  {
    // Streaming on with at least one JSON whitespace char required
    // between JSON values. This should exercise most of the streaming
    // code.
    //
    parse (data, size, true, "ws");
  }
  return 0;
}
