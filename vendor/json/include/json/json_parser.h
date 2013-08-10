// ----------------------------------------------------------------------------
#ifndef __json__json_parser_h__
#define __json__json_parser_h__
// ----------------------------------------------------------------------------
#include <json/json_value.h>
#include <json/json_array.h>
#include <json/json_object.h>
// ----------------------------------------------------------------------------
#include <json/details/json_parser_state.h>
#include <json/details/json_platform.h>
// ----------------------------------------------------------------------------
#include <memory>
#include <stack>
#include <array>
// ----------------------------------------------------------------------------
namespace json
{
  class parser
  {
    friend class parser_state_base;
  private:
#ifdef __JSON__HAS_CPP_USING_SUPPORT
    using state_ptr = std::unique_ptr<parser_state_base>;
#else
    typedef std::unique_ptr<parser_state_base> state_ptr;
#endif
  public:
    parser(value& value);
  public:
    size_t parse(const char* data, size_t data_len);
  public:
    bool complete() const noexcept;
  private:
    void push_state(parser_state_base* state);
  private:
    value& value_;
    std::stack<state_ptr, std::vector<state_ptr>> stack_;
  };
}
// ----------------------------------------------------------------------------
#endif // __json__json_parser_h__
