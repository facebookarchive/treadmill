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

#include <random>
#include <sys/time.h>

namespace facebook {
namespace windtunnel {
namespace treadmill {

using std::mt19937_64;
using std::string;
using std::uniform_real_distribution;

// Struct for Mersene Twister 19937 generator (64 bit)
struct RandomEngine {
  public:
    /**
     * Return a random number ranging in [0.0, 1.0] in double
     *
     * @return A random number ranging in [0.0, 1.0] in double
     */
    static double getDouble();

  private:
    // The Mersene Twister 19937 random engine (64 bit)
    static mt19937_64 random_engine_;
    // A uniform distribution for real numbers
    static uniform_real_distribution<double> uniform_distribution_;
};

/**
 * Read a line from the file descriptor
 *
 * @param fd The file descriptor
 * @param buffer The buffer to write
 * @param buffer_size The size of the read buffer
 * @return The total amount of bytes read
 */
int readLine(int fd, char* buffer, int buffer_size);

/**
 * Read from block given the file descriptor
 *
 * @param fd The file descriptor
 * @param buffer The buffer to write
 * @param buffer_size The size of the read buffer
 */
void readBlock(int fd, char* buffer, int buffer_size);

/**
 * Write to block given the file descriptor
 *
 * @param fd The file descriptor
 * @param buffer The buffer to write
 * @param buffer_size The size of the write buffer
 */
void writeBlock(int fd,
                const char* buffer,
                int buffer_size);

}  // namespace treadmill
}  // namespace windtunnel
}  // namespace facebook
