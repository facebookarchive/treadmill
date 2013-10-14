namespace facebook {
namespace windtunnel {
namespace treadmill {

void writeBlock(int fd,
                char* buffer,
                int buffer_size);

void readBlock(int fd, char* buffer, int buffer_size);

}
}
}
