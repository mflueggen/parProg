Sample Playground for SIMD Operations
(C) 2019 OSM FG, HPI
Licence: MIT

This small project lets you fool around with SIMD operations on a video stream.
For simplicity reasons the stream is passed via stdin/stdout.

You can send those streams also via TCP if you local host does not support a specific SIMD ISA, just pipe the output using netcat.
Be aware that those uncompressed streams are HUGE!

./simdvid.py -s [input.mp4]                      ./simdvid.py -r
    |                                                |
    +->  | nc -TCP-> nc | ./filter | nc -TCP-> nc | -+

Frame format:
height     32bit, little-endian signed int
width      32bit, little-endian signed int
channels   32bit, little-endian signed int
framedata  (height * width * channels) single-precision floats
           no padding, fixed stride, range [0..1]

The datasource (simdvid.py) requires OpenCV (https://opencv.org/) python libraries and numpy installed. If called without video file, it will open up your webcam.

Provided examples:
	* Multiply values by scalar (overflow-prone)
	  [AltiVec, SSE2, plain loop]

Fancy Ideas for you (with out without adding channels):
    * flip channels
	* make grayscale
	* gaussian blur (requires horizontal fetching)
	* sobel (requires horizontal fetching)

Have fun!

