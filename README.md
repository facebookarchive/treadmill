# [Treadmill] (http://github.com/facebook/treadmill) #

Treadmill is a load testing framework for request based software services, i.e. Memcached.

## Dependencies ##

 - clang (http://clang.llvm.org/)
 - libevent (http://libevent.org/)
 - glog (https://code.google.com/p/google-glog/)
 - gflags (https://code.google.com/p/gflags/)
 - folly (https://github.com/facebook/folly)

## Configuration ##

###### Required packages ######

Install the software packages required by folly and treadmill.

```bash
sudo apt-get install git g++ automake autoconf autoconf-archive libtool libboost1.54-all-dev libgoogle-glog-dev libgflags-dev scons libevent-dev
```

###### Install folly ######

Download folly from github.

```bash
git clone https://github.com/facebook/folly
```

Download and uncompress double-conversion from googlecode.

```bash
wget https://double-conversion.googlecode.com/files/double-conversion-2.0.1.tar.gz
mkdir double-conversion
tar xvf double-conversion-2.0.1.tar.gz -C double-conversion/
```

Copy SConstruct.double-conversion file to double-conversion.

```bash
cp folly/folly/SConstruct.double-conversion double-conversion/
```

Build double-conversion.

```bash
cd double-conversion
scons -f SConstruct.double-conversion
make
cd ..
```

Remove the changes in this diff (https://github.com/facebook/folly/commit/56e0ec4fe2db38106311b09b88820a99860664f3).

Download gtest 1.6.0.

```bash
cd folly/folly/test
http://googletest.googlecode.com/files/gtest-1.6.0.zip
unzip gtest-1.6.0.zip
cd ../../../
```

Build folly.

```bash
cd folly/folly
autoreconf --install
LDFLAGS=-L<double-conversion>/ CPPFLAGS=-I<double-conversion>/src/ ./configure --with-boost-libdir=/usr/lib/x86_64-linux-gnu/
make
sudo make install
cd ..
```

Build treadmill.

```bash
cd treadmill
make
```
