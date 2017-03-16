include "common/fb303/if/fb303.thrift"

namespace cpp2 treadmill
namespace py treadmill

service TreadmillService extends fb303.FacebookService {
  bool pause();
  bool resume();
}
