# scale
## Copyright
Public domain, 2025

github.com/SirJonthe

## About
`scale` is a simple C++11 function that scales a multi-dimensional source area over a multi-dimensional destination area.

## Design
The library contains a minimal amount of data structures and only exposes a single function used for its intended purpose, `scale`.

## Building
No special adjustments need to be made to build `scale` except enabling C++11 compatibility or above. Simply include the relevant headers in your code and make sure the headers and source files are available in your compiler search paths. Using `g++` as an example, building is no harder than:

```
g++ -std=c++11 code.cpp scale/scale.cpp
```

...where `code.cpp` is an example source file containing some user code as well as the entry point for the program.

## Examples
### Basic `scale` call
A basic call to `scale` takes a destination area defined by two integer points (a starting point, and an ending point), and a source area defined by two real points (a starting point, and an ending point). Real points use the built-in `fixed` data type, a real number using fixed-point precision, in order to avoid rounding errors while interpolating the coordinates.

```
using namespace cc0::scale;

Area<int32_t,1> dst_area = { { 0 }, { 10 } };
Area<fixed32_t,1> src_area = { { fixed32_t(0) }, { fixed32_t(10) } };
Area<int32_t,1> max_dst_bounds = { { 0 }, { 10 } }

void processor(Point<int32_t,1> dst, Point<fixed32_t,1> src);

scale(dst_area, src_area, processor, max_dst_bounds);
```
The `processor` and `max_dst_bounds` parameters will be discussed below.

As mentioned, areas are defined by two N-dimensional points, a starting point and an ending point. The starting point is inclusive, i.e. included in the processing, while the end-point is non-inclusive, i.e. not included in the processing. Areas can also have their starting and ending point axis flipped to allow for processing in reversed directions. More on this below.

### Processor functions/functors
When scaling one source area over a destination area a custom processor function/functor is called at every integer step in the destination area. Since the `scale` function does not do anything in and of itself, a processor is needed in order to actually perform meaningful work.

A processor function/functor takes two arguments, where the first is the current index of the destination area being iterated over, and the second is the current index of the source area being iterated over.

```
#include <iostream>

using namespace cc0::scale;

void processor(const Point<int32_t,1> &dst, const Point<fixed32_t,1> &src)
{
	std::cout << dst[0] << ", " << dst[1] << " -> " << int32_t(src[0]) << ", " << int32_t(src[1]) << std::endl;
}

Area<int32_t,1> dst_area = { { 0 }, { 10 } };
Area<fixed32_t,1> src_area = { { fixed32_t(0) }, { fixed32_t(10) } };
Area<int32_t,1> max_dst_bounds = { { 0 }, { 10 } }

scale(dst_area, src_area, processor, max_dst_bounds);
```
In the example above, the processor function only prints the destination and source indices, but does not perform meaningful work. A common task is writing the contents of one array into another, and will be shown in an example below.

### Destination mask
Destination masks ensures that no processing happens outside of the defined area. Destination areas that completely fall within the mask are wholly unaffected by it, while destination areas that completely fall outside of the mask result in no processing whatsoever. When destination areas partially fall outside of the mask the processing is appropriately offset, making processing naturally omit the parts of the destination area that fall outside of the mask.

Destination areas are generally used to define limits to memory regions and for multi-threaded processing where each thread can process separate memory regions without risking a write to a memory area which is also in the process of being written to by another thread.

```
Area<int32_t,1> dst_area = { { 0 }, { 40 } };
Area<fixed32_t,1> src_area = { { fixed32_t(0) }, { fixed32_t(40) } };
Area<int32_t,1> max_dst_bounds[4] = {
	{ { 0 }, { 10 } },
	{ { 10 }, { 20 } },
	{ { 20 }, { 30 } },
	{ { 30 }, { 40 } }
};

void processor(Point<int32_t,1> dst, Point<fixed32_t,1> src);

for (int32_t i = 0; i < 4; ++i) {
	scale(dst_area, src_area, processor, max_dst_bounds[i]); // NOTE: This can be used 
}
```

### Copy memory from one array into another
The library contains a single built-in processor function for the common task of copying memory from one array into another called `write`.
```
using namespace cc0::scale;

float dst_array[10];
float src_array[10];

Area<int32_t,1> dst_area = { { 0 }, { 10 } };
Area<fixed32_t,1> src_area = { { fixed32_t(0) }, { fixed32_t(10) } };
Area<int32_t,1> max_dst_bounds = { { 0 }, { 10 } }

scale(dst_area, src_area, write(dst_array, src_array), max_dst_bounds);
```
The code above will just copy the source array into the destination array, but scaling enables memory copies to be used in more interesting ways:

```
using namespace cc0::scale;

float dst_array[20];
float src_array[10];

Area<int32_t,1> dst_area = { { 0 }, { 20 } };
Area<fixed32_t,1> src_area = { { fixed32_t(0) }, { fixed32_t(10) } };
Area<int32_t,1> max_dst_bounds = { { 0 }, { 20 } }

scale(dst_area, src_area, write(dst_array, src_array), max_dst_bounds);
```
The code above "stretches" a 10-element source array and stores it in a 20-element destination array where each source element will be written twice into adjacent destination memory locations. Stretching is arbitrary; There is no restriction on what source area to stretch over what destination area as long as it does not result in an invalid memory access.

Shrinking a source array:
```
using namespace cc0::scale;

float dst_array[5];
float src_array[10];

Area<int32_t,1> dst_area = { { 0 }, { 5 } };
Area<fixed32_t,1> src_area = { { fixed32_t(0) }, { fixed32_t(10) } };
Area<int32_t,1> max_dst_bounds = { { 0 }, { 5 } }

scale(dst_area, src_area, write(dst_array, src_array), max_dst_bounds);
```

Doing a partial write, i.e. copy/stretch only a part of the source array into a part of the destination array:
```
using namespace cc0::scale;

float dst_array[20];
float src_array[10];

Area<int32_t,1> dst_area = { { 5 }, { 10 } };
Area<fixed32_t,1> src_area = { { fixed32_t(2) }, { fixed32_t(7) } };
Area<int32_t,1> max_dst_bounds = { { 0 }, { 5 } }

scale(dst_area, src_area, write(dst_array, src_array), max_dst_bounds);
```

Reversing arrays:
```
using namespace cc0::scale;

float dst_array[10];
float src_array[10];

Area<int32_t,1> dst_area = { { 10 }, { 0 } };
Area<fixed32_t,1> src_area = { { fixed32_t(0) }, { fixed32_t(10) } };
Area<int32_t,1> max_dst_bounds = { { 0 }, { 10 } }

scale(dst_area, src_area, write(dst_array, src_array), max_dst_bounds);
```
Note that either the destination or source area can flip its axis. If only one of them is flipped then processing is reversed, but if both are flipped processing is the same as if neither were flipped.

Note that only simple nearest neighbor sampling is supported by `write`, but could be implemented by the user if need be.

### Multi-dimensional scaling
`scale` supports iterating over multiple-dimensions:
```
using namespace cc0::scale;

Area<int32_t,3> dst_area = { { 0,0,0 }, { 10,10,10 } };
Area<fixed32_t,3> src_area = { { fixed32_t(0),fixed32_t(0),fixed32_t(0) }, { fixed32_t(10),fixed32_t(10),fixed32_t(10) } };
Area<int32_t,3> max_dst_bounds = { { 0,0,0 }, { 10,10,10 } }

void processor(Point<int32_t,3> dst, Point<fixed32_t,3> src);

scale(dst_area, src_area, processor, max_dst_bounds);
```

Unfortunately, there is no version of `write` which takes more than one dimensions.

### `fixed32_t`
While the destination area iterates over integer coordinates, the source area does not need to be bound by this restriction. This means that the user has fine-grained control of the source area that is iterated over the destination area. Using memory copying as an example:
```
using namespace cc0::scale;

float dst_array[5];
float src_array[10];

Area<int32_t,1> dst_area = { { 0 }, { 5 } };
Area<fixed32_t,1> src_area = { { fixed32(0,75) }, { fixed32(3,229) } };
Area<int32_t,1> max_dst_bounds = { { 0 }, { 5 } }

scale(dst_area, src_area, write(dst_array, src_array), max_dst_bounds);
```
Note that `fixed32` is a helper function designed to make it easier to define fixed-point numbers where the first parameter is the integer part and the second parameter is the base-10 fractional part. In the example above, the source area is `[0.75, 3.229)` and interpolated across the destination area `[0, 5)`.