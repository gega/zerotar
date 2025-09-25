# zerotar

Streaming tar processor for embedded systems

## Features

`zerotar` is a minimal, streaming TAR parser for embedded systems with extremely low RAM. It is designed to parse a TAR archive incrementally without allocating large buffers.

- Single API call for streaming parsing
- Works with partial buffers
- Very low memory footprint
- Supports optional path flattening (`ZEROTAR_FLAGS_FLATTEN`)
- Handles USTAR-formatted tar headers
- Only core features supported:
  - filename
  - size
- All other fields are skipped (no permissions, owners, etc)

_Notes_

- The parser assumes extremely low memory environments, less than 200 bytes of RAM needed
- The parser expects USTAR headers. Malformed headers may be ignored or cause early termination.
- Always zero out struct zerotar_s before use to ensure proper operation.

## Usage

### Include

Include the header in your project:

```c
#include "zerotar.h"
```
In one source file, define the implementation:

```c
#define ZEROTAR_IMPLEMENTATION
#include "zerotar.h"
```

### Initialization

Before use, all fields in struct zerotar_s should be zeroed:

```c
struct zerotar_s zt = {0};
zt.flags = ZEROTAR_FLAGS_FLATTEN; // optional feature
zt.f_open  = my_open_callback;
zt.f_write = my_write_callback;
zt.f_close = my_close_callback;
zt.userdata = my_context;
```

_Note:_ Callbacks can be NULL in case we don't need some or all of them.

### Callbacks

```f_open(void *userdata, const char *name, int size)```

  Called when a new file starts. name is the file path (flattened if ZEROTAR_FLAGS_FLATTEN is set). size is the expected file size in bytes.

```f_write(void *userdata, uint8_t *buf, int len)```

  Called with chunks of file data. The parser may deliver data in arbitrary chunk sizes.

```f_close(void *userdata)```

  Called when a file finishes.

### API

```void zerotar(struct zerotar_s *zt, uint8_t *buf, int len);```

zt – pointer to the parser state

buf – buffer containing part of the TAR archive

len – number of bytes in the buffer

If len < 0, the parser is reset and internal state cleared.

Call zerotar() repeatedly as new data arrives.

