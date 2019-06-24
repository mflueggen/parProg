#include "hls_common/mtl_stream.h"

#define WORD_BITS 512
typedef ap_uint<WORD_BITS> word_t;
#define HASH_BITS 128
typedef ap_uint<HASH_BITS> hash_t;

#define UNIT_IDX_BITS 5
typedef ap_uint<UNIT_IDX_BITS> unit_index_t;
#define UNIT_BITS (0x1<<UNIT_IDX_BITS)
typedef ap_uint<UNIT_BITS> unit_t;

#define WORD_IDX_BITS 4
typedef ap_uint<WORD_IDX_BITS> word_index_t;
#define EXTRACT_UNIT(word, index) (word.range(UNIT_BITS*index+UNIT_BITS-1, UNIT_BITS*index))

#define ITERATION_BITS 7
#define ITERATION_RANGE (0x1<<(ITERATION_BITS-1))
typedef ap_uint<ITERATION_BITS> iteration_ctr_t;

#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

// Implemented with pseudo code reference from wikipedia (https://en.wikipedia.org/wiki/MD5#Pseudocode)

const word_index_t G[ITERATION_RANGE] = {
  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,   //  i         for 0..15
  1,  6, 11,  0,  5, 10, 15,  4,  9, 14,  3,  8, 13,  2,  7, 12,   // (5*i+1)%16 for 16..31
  5,  8, 11, 14,  1,  4,  7, 10, 13,  0,  3,  6,  9, 12, 15,  2,   // (3*i+5)%16 for 32..47
  0,  7, 14,  5, 12,  3, 10,  1,  8, 15,  6, 13,  4, 11,  2,  9 }; // (7*i)%16   for 48..63
const unit_index_t S[ITERATION_RANGE] = {
  7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,
  5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,
  4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,
  6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21 };
const unit_t K[ITERATION_RANGE] = {
  0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
  0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
  0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
  0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
  0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
  0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
  0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
  0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
  0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
  0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
  0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
  0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
  0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
  0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
  0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
  0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391 };


void mprint_hex(unsigned char * buffer, int length)
{
  printf("0x");
  for(int i = 0; i < length; i++) {
    printf("%02x", buffer[i]);
  }
  printf("\n");
}


hash_t md5hash(word_t word) {
  printf("================= OPERATOR =============================\n");
  mprint_hex((unsigned char *)&word, 64);

  unit_t a = 0x67452301;
  unit_t b = 0xefcdab89;
  unit_t c = 0x98badcfe;
  unit_t d = 0x10325476;
  unit_t A = a;
  unit_t B = b;
  unit_t C = c;
  unit_t D = d;

  for (iteration_ctr_t i = 0; i < ITERATION_RANGE; ++i) {
    const unit_t w = EXTRACT_UNIT(word, G[i]); //32bit word of the 512bit block. M[g] in pseudo code
    const unit_t k = K[i];
    const unit_index_t s = S[i];
    // TODO implement the MD5 Hashing Algorithm
    unit_t F;
    if (0 <= i && i <= 15) {
      F = (B & C) | ((~B) & D);}
    else if (16 <= i && i <= 31){
      F = (D & B) | ((~D) & C);}
    else if (32 <= i && i <= 47){
      F = B ^ C ^ D;}
    else if (48 <= i && i <= 63){
      F = C ^ (B | (~D));}

    F = F + A + k + w;
    A = D;
    D = C;
    C = B;
    B = B + ROTATE_LEFT(F,s);
  }
  a = a + A;
  b = b + B;
  c = c + C;
  d = d + D;

  // useful for debugging: https://cryptii.com/pipes/md5-hash
  hash_t result = (d,c,b,a);
  printf("Hash: ");
  mprint_hex((unsigned char *)&result, 16);
  return result;
}

void hls_operator_md5hash(mtl_stream &in, mtl_stream &out, hash_t search) {
#pragma HLS INTERFACE axis port=in name=axis_input
#pragma HLS INTERFACE axis port=out name=axis_output
#pragma HLS INTERFACE s_axilite port=search bundle=control offset=0x100
#pragma HLS INTERFACE s_axilite port=return bundle=control
  mtl_stream_element input, output;
  hash_t candidate;

  snap_bool_t element_found = false;

  do {
    input = in.read();
    candidate = md5hash(input.data);

    if (candidate == search) {
      if (element_found) {
        out.write(output);
      }

      output = input;
      element_found = true;
    }
  } while(!input.last);

  // Terminate the stream
  output.last = true;
  if (element_found) {
    out.write(output);
  } else {
    output.data = 0;
    output.keep = 1;
    out.write(output);
  }
}

#include "testbench.cpp"
