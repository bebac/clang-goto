// ----------------------------------------------------------------------------
#ifndef __json__object_h__
#define __json__object_h__
// ----------------------------------------------------------------------------
#include <json/json_value.h>
// ----------------------------------------------------------------------------
#include <json/details/json_platform.h>
// ----------------------------------------------------------------------------
#include <unordered_map>
// ----------------------------------------------------------------------------
namespace json
{
  class value;

  class object
  {
  public:
#ifdef __JSON__HAS_CPP_USING_SUPPORT
    using member_map = std::unordered_map<std::string, value>;
#else
    typedef std::unordered_map<std::string, value> member_map;
#endif
  public:
    object() : value_() {}
#ifdef __JSON__HAS_STD_INITIALIZER_LIST
    object(std::initializer_list<member_map::value_type> v) : value_(v) {}
#endif
  public:
    virtual ~object() {}
  public:
    virtual bool is_object() const { return true; }
  public:
    virtual void write(std::ostream& os) const;
  public:
    template <typename K> value operator[](K key) { return value_[std::forward<K>(key)]; }
  public:
    template <typename K, typename V> void member(K key, V v)
    {
      value_.emplace(std::forward<K>(key), std::forward<V>(v));
    }
  private:
    member_map value_;
  };

  inline void object::write(std::ostream& os) const
  {
    os << "{";
    for ( member_map::const_iterator it = begin(value_); it != end(value_); )
    {
      os << escape((*it).first) << ":" << (*it).second;

      if ( ++it != end(value_) ) {
        os << ",";
      }
    }
    os << "}";
  }
}

// ----------------------------------------------------------------------------
inline std::ostream& operator<<(std::ostream& os, const json::object& v)
{
  v.write(os);
  return os;
}

// ----------------------------------------------------------------------------
#endif // __json__object_h__
