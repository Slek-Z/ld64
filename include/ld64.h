/*
x64 Length Disassembler.
Copyright (C) 2020 Slek

x86 Length Disassembler.
Copyright (C) 2013 Byron Platt

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

#ifndef __LD64_H__
#define __LD64_H__

#include <cstdint>

/* length_disasm */
int length_disasm(const void* opcode0);

/* tables */
const static uint64_t modrm_t[4] = {
  0x0f0f0f0f0f0f0f0full, 0x00000a0800000000ull, 0x000000000000fffbull, 0xc0c00000ff0f00c3ull
};
const static uint64_t prefix_t[4] = {
  0x4040404000000000ull, 0x000000f00000ffffull, 0x0000000000000000ull, 0x000d000000000000ull
};
const static uint64_t supported_t[4] = {
  0x3f3f3f3f3f3fbf3full, 0xffffff08ffff0000ull, 0xfffffffffbfffffbull, 0xffe0fbffff8fbfcfull
};
const static uint64_t data0_t[4] = {
  0x0f0f0f0f0f0f0f0full, 0x0000f008ffff0000ull, 0x0000fcf0fbfffff0ull, 0xff20f000ff8f9a08ull
};
const static uint64_t data1_t[4] = {
  0x1010101010101010ull, 0xffff0c0000000000ull, 0x00ff010000000009ull, 0x000008ff00002143ull
};
const static uint64_t data66_t[4] = {
  0x2020202020202020ull, 0x0000030000000000ull, 0xff00020000000002ull, 0x0000000000000080ull
};
const static uint64_t supported2_t[4] = {
  0x00000000800028a0ull, 0x000000000000ffffull, 0xfcfffb3fffffffffull, 0x000000000000ff83ull
};
const static uint64_t modrm2_t[4] = {
  0x0000000080002000ull, 0x000000000000ffffull, 0xfcfff838ffff0000ull, 0x0000000000000083ull
};

/* table macros */
#define CHECK_TABLE(t, v) ((t[(v)>>6]>>((v)&0x3f))&1)

/* CHECK_PREFIX */
#define CHECK_PREFIX(v) CHECK_TABLE(prefix_t, v)
//#define CHECK_PREFIX(v) (((v)&0xe7)==0x26||((v)&0xf0)==0x40||((v)==0xfc)==0x64||(v)==0xf0||(v)==0xf2||(v)==0xf3)

/* CHECK_PREFIX_66 */
#define CHECK_PREFIX_66(v) ((v)==0x66)

/* CHECK_PREFIX_67 */
#define CHECK_PREFIX_67(v) ((v)==0x67)

/* CHECK_REX_W */
#define CHECK_REX_W(v) (((v)&0xf8)==0x48)

/* CHECK_0F */
#define CHECK_0F(v) ((v)==0x0f)

/* CHECK_SUPPORTED */
#define CHECK_SUPPORTED(v) CHECK_TABLE(supported_t, v)

/* CHECK_MODRM */
#define CHECK_MODRM(v) CHECK_TABLE(modrm_t, v)

/* CHECK_TEST */
#define CHECK_TEST(v) ((v)==0xf6||(v)==0xf7)

/* CHECK_DATA0 */
#define CHECK_DATA0(v) CHECK_TABLE(data0_t, v)

/* CHECK_DATA1 */
#define CHECK_DATA1(v) CHECK_TABLE(data1_t, v)

/* CHECK_DATA2 */
#define CHECK_DATA2(v) (((v)&0xf7)==0xc2||(v)==0xc8)

/* CHECK_DATA66 */
#define CHECK_DATA66(v) CHECK_TABLE(data66_t, v)
//#define CHECK_DATA66(v) (((v)&0xc7)==0x05||((v)&0xf8)==0xb8||((v)&0xfe)==0x68||(v)==0x81||(v)==0xa9||(v)==0xc7)

/* CHECK_DATA4 */
#define CHECK_DATA4(v) ((v)==0xe8||(v)==0xe9)

/* CHECK_DATA48 */
#define CHECK_DATA48(v) (((v)&0xf8)==0xb8)

/* CHECK_MEM67 */
#define CHECK_MEM67(v) (((v)&0xfc)==0xa0)

/* CHECK_SUPPORTED2 */
#define CHECK_SUPPORTED2(v) CHECK_TABLE(supported2_t, v)

/* CHECK_MODRM2 */
#define CHECK_MODRM2(v) CHECK_TABLE(modrm2_t, v)

/* CHECK_DATA12 */
#define CHECK_DATA12(v) ((v)==0xa4||(v)==0xac||(v)==0xba)

/* CHECK_DATA42 */
#define CHECK_DATA42(v) (((v)&0xf0)==0x80)

/* length_disasm */
int length_disasm(const void* opcode0) {

    const unsigned char* opcode = (const unsigned char *) opcode0;

    unsigned int flag = 0, rexw = 0;
    unsigned int ddef = 4, mdef = 8;
    unsigned int msize = 0, dsize = 0;

    unsigned char op, modrm, mod, rm;

    for (; op = *opcode++, CHECK_PREFIX(op); ) {
      if (CHECK_PREFIX_66(op)) ddef = 2;
      if (CHECK_PREFIX_67(op)) mdef = 4;
      if (CHECK_REX_W(op)) { rexw = 1; ddef = 4; }
      else rexw = 0;
    }

    if (!CHECK_SUPPORTED(op)) return -1;

    if (CHECK_0F(op)) {
      /* two byte opcode */
      op = *opcode++;
      if (!CHECK_SUPPORTED2(op)) return -1;
      if (CHECK_MODRM2(op)) flag++;
      if (CHECK_DATA12(op)) dsize++;
      if (CHECK_DATA42(op)) dsize += 4;
    } else {
      /* one byte opcode */
      if (CHECK_MODRM(op)) flag++;
      if (CHECK_TEST(op) && !(*opcode & 0x30)) dsize += (op & 1) ? ddef : 1;
      if (CHECK_DATA1(op)) dsize++;
      if (CHECK_DATA2(op)) dsize += 2;
      if (CHECK_DATA66(op)) dsize += ddef;
      if (CHECK_DATA4(op)) dsize += 4;
      if (CHECK_DATA48(op) && rexw) dsize = 8;
      if (CHECK_MEM67(op)) msize += mdef;
    }

    /* modrm */
    if (flag) {
      modrm = *opcode++;
      mod = modrm & 0xc0;
      rm  = modrm & 0x07;
      if (mod != 0xc0) {
        if (mod == 0x40) msize++;
        if (mod == 0x80) msize += 4;
        if (rm == 0x04) rm = *opcode++ & 0x07;
        if (rm == 0x05 && mod == 0x00) msize += 4;
      }
    }

    opcode += msize + dsize;

    return opcode - (const unsigned char *)opcode0;
}

#endif // __LD64_H__
