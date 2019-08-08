## Treadmill [![Build Status](https://travis-ci.org/facebook/treadmill.svg?branch=master)](https://travis-ci.org/facebook/treadmill)

Treadmill is an open-source modular load testing platform for characterizing
server-side applications (_e.g._, [Memcached](http://memcached.org/),
[Mcrouter](https://github.com/facebook/mcrouter), and
[FBThrift](https://github.com/facebook/fbthrift).

### Installation

The installation is done using a standard `autotools` flow. To install Treadmill
at `/path/to/install`, you can simply use the script as following.

```bash
$ ./scripts/install_ubuntu_14.04.sh /path/to/install
```

### Running

Once you have installed Treadmill, you can run the corresponding binary. For
example, you can test against a `Memcached` instance on `127.0.0.1:11211` by:

```bash
$ LD_LIBRARY_PATH="/path/to/install/install/lib:$LD_LIBRARY_PATH" \
    ./treadmill_memcached --hostname=127.0.0.1 --port=11211
```

You can also add the `/path/to/install/install/lib` permanently to
`$LD_LIBRARY_PATH`, so you do not have to type it in every time. More advanced
options can be found in `--help`.

### License

Treadmill is [BSD-licensed](LICENSE). We also provide an additional
[patent grant](PATENTS).

### Contributing

We would love to have your help in making Treadmill better. If you are
interested, please read our [guide to contributing](CONTRIBUTING.md).
