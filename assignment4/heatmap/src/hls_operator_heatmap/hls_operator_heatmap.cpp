#include "hls_common/mtl_stream.h"

#define CELL_BITS 8
typedef ap_uint<CELL_BITS> cell_t;
#define ROW_CELLS (sizeof(mtl_stream_data)/sizeof(cell_t))

#define SLICE(word, offset) (word(CELL_BITS*(offset)+CELL_BITS-1, CELL_BITS*(offset)))


void sum_neighbors_row(const mtl_stream_element& in, mtl_stream_element& out) {
  SLICE(out.data, 0) = 0.333333333 * (SLICE(in.data, 0) + SLICE(in.data, 1));
  SLICE(out.data, 1) = 0.333333333 * (SLICE(in.data, 0) + SLICE(in.data, 1) + SLICE(in.data, 2));
  SLICE(out.data, 2) = 0.333333333 * (SLICE(in.data, 1) + SLICE(in.data, 2) + SLICE(in.data, 3));

  for (unsigned i = 3; i < 63; ++i) {
#pragma HLS unroll
    SLICE(out.data, i) = 0.333333333 * (SLICE(in.data, i - 1) + SLICE(in.data, i) + SLICE(in.data, i + 1));
  }
  SLICE(out.data, 63) = 0.333333333 * (SLICE(in.data, 62) + SLICE(in.data, 63));
}

void sum_neighbors_col(const mtl_stream_element* in, mtl_stream_element& out) {
  for (unsigned i = 0; i < 64; ++i) {
#pragma HLS unroll
    SLICE(out.data, i) = 0.333333333 * (SLICE(in[0].data, i) + SLICE(in[1].data, i) + SLICE(in[2].data, i));
  }
}

void hls_operator_heatmap(mtl_stream &in, mtl_stream &out) {
#pragma HLS INTERFACE axis port=in name=axis_input
#pragma HLS INTERFACE axis port=out name=axis_output
#pragma HLS INTERFACE s_axilite port=return bundle=control

  mtl_stream_element elements[3];
  mtl_stream_element element;
  elements[0].data = 0;
  unsigned row_index = 1;
  bool last = false;

  // deal with first row
  element = in.read();
  sum_neighbors_row(element, elements[1]);
  last = element.last;

  while (!last) {
#pragma HLS pipeline
    row_index = (row_index + 1) % 3;
    element = in.read();
    last = element.last;
    sum_neighbors_row(element, elements[row_index]);
    sum_neighbors_col(elements, element);
    element.last = false;
    out.write(element);
  }

  // deal with last row
  row_index = (row_index + 1) % 3;
  elements[row_index].data = 0;
  sum_neighbors_col(elements, element);
  element.last = true;
  out.write(element);
}

#include "testbench.cpp"

