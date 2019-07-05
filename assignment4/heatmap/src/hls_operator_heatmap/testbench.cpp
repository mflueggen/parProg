#ifndef __SYNTHESIS__

#include <iostream>

int main() {
  mtl_stream in;
  mtl_stream out;

  mtl_stream_data first = 0;
  mtl_stream_data second = 0;
  mtl_stream_data third = 0;

  second(255, 248) = 255;
  second(263, 256) = 255;

  mtl_stream_element element;
  element.keep = ~(mtl_stream_keep)0;

  element.data = first;
  in.write(element);
  element.data = second;
  in.write(element);
  element.last = true;
  element.data = third;
  in.write(element);

  hls_operator_heatmap(in, out);

  std::stringstream expected, actual;
  expected
    << "0000000000000000000000000000001221000000000000000000000000000000" << std::endl
    << "0000000000000000000000000000001221000000000000000000000000000000" << std::endl
    << "0000000000000000000000000000001221000000000000000000000000000000" << std::endl;

  do {
    element = out.read();

    for (int j=0; j < ROW_CELLS; j++)
    {
        unsigned const value = element.data(j * CELL_BITS + 7, j * CELL_BITS);
        if (value > 230)
        {
            actual << "X";
        }
        else
        {
            actual << 10 * value / 255;
        }
    }
    actual << std::endl;
  } while (!element.last);

  std::cout << actual.str();

  if (expected.str() != actual.str()) {
      std::cout << "Did not receive expected result" << std::endl;
      return 1;
  }

  std::cout << "Success." << std::endl;
  return 0;
}

#endif
