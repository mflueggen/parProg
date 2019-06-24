#ifndef __SYNTHESIS__

#include <fstream>

void print_hex(unsigned char * buffer, int length) {
  printf("0x");
  for(int i = 0; i < length; i++) {
    printf("%02x", buffer[i]);
  }
  printf("\n");
}

int main() {
  mtl_stream in;
  mtl_stream out;

  std::ifstream infile_dict, infile_hash;
  infile_dict.open("../../../../dict.txt", std::ifstream::in);
  infile_hash.open("../../../../hash.txt", std::ifstream::in);
  hash_t search;
  infile_hash.read((char *) search.getRawData(), 16);

  size_t readBytes;
  mtl_stream_element element;
  do {
    unsigned char buffer[64] = {0};
    std::string line;
	  std::getline(infile_dict, line);
    if(line.length() <= 55) {
        std::copy(line.begin(), line.end(), buffer);
    } else {
      std::cout << "The input dictionary file contains password which are longer than 55 characters" << std::endl;
    }

    // add 1 bit and pad the rest with 0s
    buffer[55] = 0x80;
    // store as 64 bit little endian integer the lenght of the original message in bits
    uint64_t line_len = line.length() * 8;
    memcpy(&buffer[56], &line_len, 8);
    readBytes = sizeof(buffer);
    memcpy(&element.data, buffer, readBytes);
    element.keep = 0xff;
    element.last = infile_dict.eof() ? 0x1 : 0x0;
    print_hex((unsigned char *) &element.data, 64);
    in.write(element);
  } while (!element.last);

  hls_operator_md5hash(in, out, search);

  FILE * outfile = fopen("../../../../found_pw.txt", "w");
  do {
    element = out.read();
    if (element.keep > 1) {
      uint64_t size;
      memcpy(&size, ((char *) &element.data) + 56, 8);
      size /= 8;
      fwrite(&(element.data), 1, size, outfile);
    }
  } while (!element.last);

  return 0;
}

#endif