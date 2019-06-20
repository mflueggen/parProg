#define get_h(h, y, x) heatmaps[h * width * height + y * width + x]

struct hotspot
{
    unsigned short x;
    unsigned short y;
    unsigned short start_round;
    unsigned short end_round;
};


__kernel void simulate(const unsigned short width, const unsigned short height,
                       const unsigned short rounds, __global float* heatmaps,
                       const unsigned short num_hotspots, __global const struct hotspot* hotspots)
{
    const unsigned short col = get_global_id(0);
    const unsigned short row = get_global_id(1);

    struct hotspot h;
    h.x = 65535u;

    for (unsigned short i = 0u; i < num_hotspots; ++i) {
        if (hotspots[i].y == row && hotspots[i].x == col) {
            h = hotspots[i];
        }
    }

    for (unsigned short round = 1u; round <= rounds; ++round) {
        const unsigned char current_heatmap_index = round % 2;
        const unsigned char old_heatmap_index = (current_heatmap_index + 1) % 2;

        double sum = 0.0;
        // for the 3x3 matrix around coord
        for (unsigned short y = row - 1; y <= row + 1; ++y) {
          for (unsigned short x = col - 1; x <= col + 1; ++x) {
            sum += get_h(old_heatmap_index, y, x);
          }
        }
        get_h(current_heatmap_index, row, col) = sum / 9.0;

        if (h.x < 65535 && round >= h.start_round && round < h.end_round)
          get_h(current_heatmap_index, h.y, h.x) = 1.0;

        barrier(CLK_GLOBAL_MEM_FENCE);
    }
}