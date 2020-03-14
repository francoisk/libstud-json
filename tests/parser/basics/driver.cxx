// Usage: argv[0]
// --fail-exc|--fail-bit|[<mode>]|--streaming-mode-separators <separators>
//
// --fail-exc  -- fail due to istream exception
// --fail-bit  -- fail due to istream badbit
//
// --streaming-mode-separators <separators> -- Enables streaming mode
//   with the specified required separators. If the separators string
//   begins with "any", the parser will be constructed with
//   streaming_mode::any_separator; if it begins with "all" the parser
//   will be constructed with streaming_mode::all_separators. If the
//   separators string begins with anything else (e.g., whitespace only)
//   the streaming mode will default to streaming_mode::any_separator.
//
// <mode>      -- numeric value parsing mode: i|u|f|d|l|

#include <cassert>
#include <cstdint>
#include <iostream>

#include <libstud/json/parser.hxx>

using namespace std;
namespace json = stud::json;

static string
number (const string& m, json::parser& p)
{
  if (m ==  "") return p.value ();
  if (m == "i") return to_string (p.value<int32_t> ());
  if (m == "u") return to_string (p.value<uint32_t> ());
  if (m == "f") return to_string (p.value<float> ());
  if (m == "d") return to_string (p.value<double> ());
  if (m == "l") return to_string (p.value<long double> ());

  assert (false);
  return "";
}

int main (int argc, const char* argv[])
{
  bool fail_exc (false);
  bool fail_bit (false);
  using json::streaming_mode;
  streaming_mode streaming_opts (streaming_mode::off);
  std::string streaming_mode_separators;
  string nm;
  if (argc > 1)
  {
    string o (argv[1]);

    if (o == "--fail-exc")
      fail_exc = true;
    else if (o == "--fail-bit")
      fail_bit = true;
    else if (o == "--streaming-mode-separators")
    {
      if (argc != 3)
      {
        cerr << "Missing streaming mode separators" << endl;
        return 1;
      }
      streaming_mode_separators = argv[2];
      streaming_opts = streaming_mode::any_separator;
      if (!streaming_mode_separators.empty ())
      {
        if (streaming_mode_separators.compare (0, 3, "any") == 0)
        {
          streaming_opts = streaming_mode::any_separator;
          streaming_mode_separators.erase (0, 3);
        }
        else if (streaming_mode_separators.compare (0, 3, "all") == 0)
        {
          streaming_opts = streaming_mode::all_separators;
          streaming_mode_separators.erase (0, 3);
        }
      }
    }
    else
      nm = move (o);
  }

  try
  {
    using namespace json;

    // It's not easy to cause the stream to fail when called by the parser.
    // So we will fail on EOF as the next best thing.
    //
    if (!fail_bit)
      cin.exceptions (istream::badbit  |
                      istream::failbit |
                      (fail_exc ? istream::eofbit : istream::goodbit));
    parser p (cin, "<stdin>", streaming_opts, streaming_mode_separators);
    size_t i (0); // Indentation.

    for (event e: p)
    {
      size_t j (i);
      string s;

      switch (e)
      {
      case event::begin_object: s = "{";     i += 2;                     break;
      case event::end_object:   s = "}"; j = i -= 2;                     break;
      case event::begin_array:  s = "[";     i += 2;                     break;
      case event::end_array:    s = "]"; j = i -= 2;                     break;
      case event::name:         s = p.name ();                           break;
      case event::string:       s = '"' + p.value () + '"';              break;
      case event::number:       s = number (nm, p);                      break;
      case event::boolean:      s = p.value<bool> () ? "true" : "false"; break;
      case event::null:         s = "NULL";                              break;
      }

      cout << string (j, ' ') << s << '\n';

      if (fail_bit)
        cin.setstate (istream::badbit);
    }

    return 0;
  }
  catch (const json::invalid_json& e)
  {
    cerr << e.name << ':' << e.line << ':' << e.column << ": error: "
         << e.what () << endl;
  }
  catch (const istream::failure&)
  {
    cerr << "error: unable to read from stdin" << endl;
  }

  return 1;
}
