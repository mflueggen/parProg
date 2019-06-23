#define get_h(h, y, x) heatmaps[h * width * height + (y) * width + (x)]
#define TILE_SIZE 32

struct hotspot
{
    unsigned short x;
    unsigned short y;
    unsigned short start_round;
    unsigned short end_round;
};

struct slim_hotspot
{
  unsigned short start_round;
  unsigned short end_round;
};


__kernel void simulate(const unsigned short width, const unsigned short padding_right, const unsigned short height,
                       const unsigned short padding_bottom, const unsigned short round, __global float* heatmaps,
                       const unsigned short num_hotspots, __global const struct slim_hotspot* hotspots)
{
  const unsigned short col = get_global_id(0);
  const unsigned short row = get_global_id(1);

  const unsigned char current_heatmap_index = round % 2;
  const unsigned char old_heatmap_index = (current_heatmap_index + 1) % 2;

  unsigned short width_diff = width - padding_right - col;
  width_diff = width_diff >> 15u;
  width_diff = !width_diff;

  unsigned short height_diff = height - padding_bottom - row;
  height_diff = height_diff >> 15u;
  height_diff = !height_diff;

  width_diff = width_diff & height_diff;

  float sum = get_h(old_heatmap_index, row, col - 1);
  sum += get_h(old_heatmap_index, row, col);
  sum += get_h(old_heatmap_index, row, col + 1);

  sum += get_h(old_heatmap_index, row-1, col - 1);
  sum += get_h(old_heatmap_index, row-1, col);
  sum += get_h(old_heatmap_index, row-1, col + 1);

  sum += get_h(old_heatmap_index, row+1, col - 1);
  sum += get_h(old_heatmap_index, row+1, col);
  sum += get_h(old_heatmap_index, row+1, col + 1);

  sum = sum * width_diff * 0.1111111111111111f;

  const struct slim_hotspot h = hotspots[row * width + col];
  unsigned short activate_hotspot = ((unsigned short)(h.start_round - round - 1u)) >> 15; // One, if h.start_round <= round
  activate_hotspot = activate_hotspot & (((unsigned short)(round - h.end_round)) >> 15) /*One, if round < h.end_round*/;
  get_h(current_heatmap_index, row, col) = sum + ((1.0f - sum) * activate_hotspot);
}
