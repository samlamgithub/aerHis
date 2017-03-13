#ifndef __TIMESTAMP_TYPE_HPP__
#define __TIMESTAMP_TYPE_HPP__

namespace tango_interface {

// Typedef for timestamp to support multiple architectures. Enables to switch quickly if needed.
typedef long long int timestamp_t; // changed from int64_t to long long int because of conflicts in printf-formats on Linux

} // namespace tango_interface

#endif // __TIMESTAMP_TYPE_HPP__
