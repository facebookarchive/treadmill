/*
* Copyright (c) 2013, Facebook, Inc.
* All rights reserved.
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*   * Redistributions of source code must retain the above copyright notice,
*     this list of conditions and the following disclaimer.
*   * Redistributions in binary form must reproduce the above copyright notice,
*     this list of conditions and the following disclaimer in the documentation
*     and/or other materials provided with the distribution.
*   * Neither the name Facebook nor the names of its contributors may be used to
*     endorse or promote products derived from this software without specific
*     prior written permission.
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "Request.h"
#include "Util.h"

#include <glog/logging.h>

namespace facebook {
namespace windtunnel {
namespace treadmill {

SetRequest::SetRequest(const string& key, int valueSize) :
  key_(key),
  valueSize_(valueSize) {}


void SetRequest::send(int fd, char* writeBuffer, char* valueBuffer) {
  // Write out key, flags exptime, and size.
  const string op = "set";
  const string key = "foo";
  int flag = 0;
  int exptime = 0;
  int size = 10;
  int res = sprintf(writeBuffer,
                     "%s %s %d %d %d\r\n",
                      op.c_str(),
                      key.c_str(),
                      flag,
                      exptime,
                      size);
  if (res < 0) {
    LOG(FATAL) << "Error with formatting key etc.";
  }
  writeBlock(fd, writeBuffer, res);

  // Write out value
  res = sprintf(writeBuffer,
                    "%.*s\r\n",
                    size,
                    valueBuffer);
  writeBlock(fd, writeBuffer, res);
}

} // treadmill
} // windtunnel
} // facebook
