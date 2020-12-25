/*
x64 Length Disassembler.
Copyright (C) 2020 Slek

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <ios>
#include <iostream>
#include <iterator>
#include <string>
#include <sstream>
#include <vector>

#include <stdio.h>

#include <Zydis/Zydis.h>

#include "ld64.h"
#include "ld64_test.h"

int main(int argc, char *argv[]) {
  
  if (argc != 2) {
    std::cout << "Usage: " << argv[0] << " <file>" << std::endl;
    return -1;
  }

  std::ifstream input(argv[1], std::ios::binary);
  
  std::vector<uint8_t> buffer(std::istream_iterator<uint8_t>(input), {});
  
  ZydisDecoder decoder;
  ZydisDecoderInit(&decoder, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_ADDRESS_WIDTH_64);
  
  Timer timer;
  uint64_t instructions = 0, supported = 0;
  double total_us = 0.;
  
  ZydisDecodedInstruction instruction;
  for (size_t offset = 0; offset < buffer.size(); ++offset) {
    if (ZYAN_SUCCESS(ZydisDecoderDecodeBuffer(&decoder, buffer.data() + offset, buffer.size() - offset, &instruction))) {
      timer.Start();
      int result = length_disasm(buffer.data() + offset);
      total_us += timer.ElapsedMicroSeconds();
      
      instructions++;
      
      if (result != -1) {
        supported++;
        if (result != instruction.length)
          std::cerr << opcodes(buffer.data() + offset, instruction.length) << std::endl;
      }
    }
  }
  
  printf("Instructions: %lu\n", instructions);
  printf("Supported: %.2f %%\n", (supported * 100.)/instructions);
  printf("Elapsed time: %.3f ms\n", total_us/1e3);
  
  return 0;
}
