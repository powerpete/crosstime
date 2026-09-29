# Crosstime Kernel Module

## Description

This Linux kernel module provides a `/dev/crosstime` device that captures `CLOCK_REALTIME` and `CLOCK_MONOTONIC` timestamps in rapid succession directly from kernel space.

A single `read()` returns two `int64_t` nanosecond values:

| Index | Content |
|-------|---------|
| 0 | `CLOCK_REALTIME`  |
| 1 | `CLOCK_MONOTONIC` |

The kernel captures three timestamps internally (`CLOCK_MONOTONIC`, `CLOCK_REALTIME`, `CLOCK_MONOTONIC`) without any userspace context switches between them, computes the `CLOCK_MONOTONIC` midpoint, and returns the two resulting values. The offset between the two clocks can then be computed in userspace as:

```
offset = parts[0] - parts[1]
```

## Installation

### Prerequisites

Linux kernel headers must be installed. On Ubuntu/Debian:

```sh
sudo apt install linux-headers-$(uname -r)
```

### Building the Module

1. Clone the repository:
   ```sh
   git clone https://github.com/powerpete/crosstime.git
   cd crosstime
   ```

2. Build the module:
   ```sh
   make
   ```

3. Insert the module into the kernel:
   ```sh
   sudo insmod crosstime.ko
   ```

4. Verify the module is loaded:
   ```sh
   lsmod | grep crosstime
   ```

The device is created at `/dev/crosstime` with permissions `0444` (read-only for all users).

## Usage

The device returns 16 bytes of binary data (2 × `int64_t`, nanoseconds).

> **Note:** The device always returns exactly 16 bytes per read. The receive buffer must be at least 16 bytes. Read requests with a buffer smaller than 16 bytes are rejected with `EINVAL`.

A minimal C example:

```c
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>

int main(void) {
    int64_t parts[2];
    int fd = open("/dev/crosstime", O_RDONLY);
    if (fd < 0) { perror("open"); return 1; }

    if (read(fd, parts, sizeof(parts)) != sizeof(parts)) {
        perror("read"); close(fd); return 1;
    }
    close(fd);

    int64_t offset_ns = parts[0] - parts[1];

    printf("CLOCK_REALTIME  : %ld ns\n", parts[0]);
    printf("CLOCK_MONOTONIC : %ld ns\n", parts[1]);
    printf("Offset:           %ld ns\n", offset_ns);
    return 0;
}
```

## Code Overview

### Key Functions

- `crosstime_init`: Registers the character device, creates the device class and the `/dev/crosstime` device node with permissions `0444`.
- `crosstime_exit`: Removes the device node, destroys the device class and unregisters the character device.
- `dev_open`: Called when the device is opened.
- `dev_release`: Called when the device is released.
- `dev_read`: Captures three kernel timestamps (`CLOCK_MONOTONIC`, `CLOCK_REALTIME`, `CLOCK_MONOTONIC`), computes the `CLOCK_MONOTONIC` midpoint, and copies the two resulting values to userspace. Returns `-EINVAL` if the provided buffer is smaller than 16 bytes.

### Device File Operations

The `file_operations` structure defines the following callbacks:

- `.owner`: Set to `THIS_MODULE` so the kernel manages the module reference count automatically.
- `.open`: Points to `dev_open`.
- `.read`: Points to `dev_read`.
- `.release`: Points to `dev_release`.

## License

This project is licensed under the GPL License — see the [LICENSE](LICENSE) file for details.

## Acknowledgements

Thanks to the Linux kernel community and all contributors who have helped in building and improving the kernel, making projects like this possible.
