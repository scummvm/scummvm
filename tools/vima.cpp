/* Residual - Virtual machine to run LucasArts' 3D adventure games
 * Copyright (C) 2003-2006 The ScummVM-Residual Team (www.scummvm.org)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 * $URL$
 * $Id$
 *
 */

#include <stdint.h>
#include <cstdio>
#include <cstring>

uint32_t get_be_uint32(char *p) {
  unsigned char *pos = reinterpret_cast<unsigned char *>(p);
  return (pos[0] << 24) | (pos[1] << 16) | (pos[2] << 8) | pos[3];
}

uint16_t imcTable1[] = {
  0x0007, 0x0008, 0x0009, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e,
  0x0010, 0x0011, 0x0013, 0x0015, 0x0017, 0x0019, 0x001c, 0x001f,
  0x0022, 0x0025, 0x0029, 0x002d, 0x0032, 0x0037, 0x003c, 0x0042,
  0x0049, 0x0050, 0x0058, 0x0061, 0x006b, 0x0076, 0x0082, 0x008f,
  0x009d, 0x00ad, 0x00be, 0x00d1, 0x00e6, 0x00fd, 0x0117, 0x0133,
  0x0151, 0x0173, 0x0198, 0x01c1, 0x01ee, 0x0220, 0x0256, 0x0292,
  0x02d4, 0x031c, 0x036c, 0x03c3, 0x0424, 0x048e, 0x0502, 0x0583,
  0x0610, 0x06ab, 0x0756, 0x0812, 0x08e0, 0x09c3, 0x0abd, 0x0bd0,
  0x0cff, 0x0e4c, 0x0fba, 0x114c, 0x1307, 0x14ee, 0x1706, 0x1954,
  0x1bdc, 0x1ea5, 0x21b6, 0x2515, 0x28ca, 0x2cdf, 0x315b, 0x364b,
  0x3bb9, 0x41b2, 0x4844, 0x4f7e, 0x5771, 0x602f, 0x69ce, 0x7462,
  0x7fff
};

uint8_t imcTable2[] = {
  0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
  0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
  0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
  0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
  0x04, 0x04, 0x04, 0x04, 0x04, 0x05, 0x05, 0x05, 0x05, 0x05,
  0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x06,
  0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
  0x06, 0x06, 0x06, 0x06, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
  0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07
};

uint8_t imcOtherTable1[] = {
  0xff, 0x04, 0xff, 0x04, 0x00, 0x00, 0x00, 0x00
};
uint8_t imcOtherTable2[] = {
  0xff, 0xff, 0x02, 0x06, 0xff, 0xff, 0x02, 0x06
};
uint8_t imcOtherTable3[] = {
  0xff, 0xff, 0xff, 0xff, 0x01, 0x02, 0x04, 0x06,
  0xff, 0xff, 0xff, 0xff, 0x01, 0x02, 0x04, 0x06
};
uint8_t imcOtherTable4[] = {
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0x01, 0x01, 0x01, 0x02, 0x02, 0x04, 0x05, 0x06,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0x01, 0x01, 0x01, 0x02, 0x02, 0x04, 0x05, 0x06
};
uint8_t imcOtherTable5[] = {
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02,
  0x02, 0x04, 0x04, 0x04, 0x05, 0x05, 0x06, 0x06,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02,
  0x02, 0x04, 0x04, 0x04, 0x05, 0x05, 0x06, 0x06
};
uint8_t imcOtherTable6[] = {
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
  0x02, 0x02, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
  0x05, 0x05, 0x05, 0x05, 0x06, 0x06, 0x06, 0x06,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
  0x02, 0x02, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
  0x05, 0x05, 0x05, 0x05, 0x06, 0x06, 0x06, 0x06
};
uint8_t *offsets[] = {
  NULL, NULL, imcOtherTable1, imcOtherTable2, imcOtherTable3,
  imcOtherTable4, imcOtherTable5, imcOtherTable6
};

uint16_t destTable[5786];
uint8_t sBytes[4];
uint16_t sWords[4];
int decLength, currTablePos, entrySize, decsToDo, tableEntry,
  sBytesPos, destPos_sWordsPos, destPos, outputWord,
  decsLeft, bytesToDec, var40, currTableVal, destOffs;
char *sourceBuffer;
unsigned char *sourcePos;
uint16_t *sWordsPos;

void initVima() {
  int destTableStartPos, incer;
  for (destTableStartPos = 0, incer = 0; destTableStartPos < 64;
       destTableStartPos++, incer++) {
    unsigned int destTablePos, imcTable1Pos;
    for (imcTable1Pos = 0, destTablePos = destTableStartPos;
	 imcTable1Pos < sizeof(imcTable1) / sizeof(imcTable1[0]);
	 imcTable1Pos++, destTablePos += 64) {
      int put = 0, count, tableValue;
      for (count = 32, tableValue = imcTable1[imcTable1Pos]; count != 0;
	   count >>= 1, tableValue >>= 1) {
	if ((incer & count) != 0)
	  put += tableValue;
      }
      destTable[destTablePos] = put;
    }
  }
}

void decompressVima_asm() {
  asm("\tpush %esp\n"
      "\tpush %ebp\n"
      "\tpush %ebx\n"
      "\tpush %esi\n"
      "\tpush %edi\n"
      "\n"
      "\tmov sourceBuffer,%ecx\n"
      "\tmov $1,%esi\n"
      "\tmov (%ecx),%al\n"
      "\tinc %ecx\n"
      "\ttest %al,%al\n"
      "\tmov %al,sBytes\n"
      "\tjge .readWords\n"
      "\tnot %al\n"
      "\tmov %al,sBytes\n"
      "\tmov $2,%esi\n"
      "\n"
      ".readWords:\n"
      "\tmov (%ecx),%ax\n"
      "\txor %edx,%edx\n"
      "\tmov %al,%dh\n"
      "\tadd $2,%ecx\n"
      "\tmov %ah,%dl\n"
      "\tcmp $1,%esi\n"
      "\tmov %dx,sWords\n"
      "\tjbe .oneWord\n"
      "\tmov (%ecx),%al\n"
      "\tinc %ecx\n"
      "\tmov %al,sBytes+1\n"
      "\txor %edx,%edx\n"
      "\tmov (%ecx),%ax\n"
      "\tadd $2,%ecx\n"
      "\tmov %al,%dh\n"
      "\tmov %ah,%dl\n"
      "\tmov %dx,sWords+2\n"
      "\n"
      ".oneWord:\n"
      "\tmov decLength,%eax\n"
      "\tmovl $1,currTablePos\n"
      "\tmov %esi,entrySize\n"
      "\tadd %esi,%esi\n"
      "\txor %edx,%edx\n"
      "\tdiv %esi\n"
      "\tmov %eax,decsToDo\n"
      "\tmov %ecx,sourcePos\n"
      "\n"
      "\txor %ebx,%ebx\n"
      "\n"
      "\tmov %eax,currTablePos\n"
      "\txor %edi,%edi\n"
      "\tcmp %edi,%eax\n"
      "\tjnz .label1\n"
      "\tmovb $0,sBytes+1\n"
      "\tmovb $0,sBytes\n"
      "\tmov %di,sWords+2\n"
      "\tmov %di,sWords\n"
      "\n"
      ".label1:\n"
      "\tmov sourcePos,%esi\n"
      "\tmov entrySize,%edx\n"
      "\txor %ecx,%ecx\n"
      "\tmov (%esi),%ch\n"
      "\tlea 1(%esi),%eax\n"
      "\txor %esi,%esi\n"
      "\tmov (%eax),%cl\n"
      "\tinc %eax\n"
      "\tcmp %edi,%edx\n"
      "\tmov %ecx,tableEntry\n"
      "\tmov %eax,sourcePos\n"
      "\tmov %esi,sBytesPos\n"
      "\tjbe .exitMainDec\n"
      "\tmov destOffs,%edi\n"
      "\tlea sWords,%eax\n"
      "\tsub %eax,%edi\n"
      "\tmov %eax,sWordsPos\n"
      "\tmov %edi,destPos_sWordsPos\n"
      "\n"
      ".nextByte:\n"
      "\tlea (%edi,%eax,1),%ecx\n"
      "\tmov %ecx,destPos\n"
      "\tmovsbl sBytes(%esi),%ecx\n"
      "\tmov %ecx,currTablePos\n"
      "\tmovswl (%eax),%ecx\n"
      "\tmov %ecx,outputWord\n"
      "\tmov decsToDo,%ecx\n"
      "\ttest %ecx,%ecx\n"
      "\tjz .done\n"
      "\tadd %edx,%edx\n"
      "\tmov %ecx,decsLeft\n"
      "\tmov %edx,bytesToDec\n"
      "\n"
      ".nextDec:\n"
      "\tmov currTablePos,%eax\n"
      "\tmov $1,%edx\n"
      "\tmov imcTable2(%eax),%cl\n"
      "\tmov %cl,currTableVal\n"
      "\tmov currTableVal,%esi\n"
      "\tand $0xff,%esi\n"
      "\tadd %esi,%ebx\n"
      "\tlea -1(%esi),%ecx\n"
      "\tmov %ebx,destOffs\n"
      "\tshl %cl,%edx\n"
      "\tmov $0x10,%cl\n"
      "\tsub %bl,%cl\n"
      "\tmov %dl,%al\n"
      "\tdec %al\n"
      "\tmov %al,var40\n"
      "\tmov tableEntry,%eax\n"
      "\tmov var40,%edi\n"
      "\tand $0xffff,%eax\n"
      "\tshr %cl,%eax\n"
      "\tand $0xff,%edi\n"
      "\tmov %edx,%ecx\n"
      "\tor %edi,%ecx\n"
      "\tand %ecx,%eax\n"
      "\tcmp $7,%ebx\n"
      "\tjle .label2\n"
      "\tmov sourcePos,%ebx\n"
      "\txor %ecx,%ecx\n"
      "\tmov tableEntry,%ch\n"
      "\tmovzbw (%ebx),%bx\n"
      "\tor %ebx,%ecx\n"
      "\tmov sourcePos,%ebx\n"
      "\tinc %ebx\n"
      "\tmov %ecx,tableEntry\n"
      "\tmov %ebx,sourcePos\n"
      "\tmov destOffs,%ebx\n"
      "\tsub $8,%ebx\n"
      "\tmov %ebx,destOffs\n"
      "\tjmp .label3\n"
      "\n"
      ".label2:\n"
      "\tmov tableEntry,%ecx\n"
      "\n"
      ".label3:\n"
      "\ttest %edx,%eax\n"
      "\tjz .clearEDX\n"
      "\txor %edx,%eax\n"
      "\tjmp .noClear\n"
      "\n"
      ".clearEDX:\n"
      "\txor %edx,%edx\n"
      "\n"
      ".noClear:\n"
      "\tcmp %edi,%eax\n"
      "\tjnz .label4\n"
      "\tmov %ecx,%edx\n"
      "\tmov %ebx,%ecx\n"
      "\tshl %cl,%edx\n"
      "\tmov sourcePos,%ecx\n"
      "\tmovzbw (%ecx),%di\n"
      "\tpush %ecx\n"
      "\tmovswl %dx,%ecx\n"
      "\txor %edx,%edx\n"
      "\tand $0xffffff00,%ecx\n"
      "\tmov %ecx,outputWord\n"
      "\tpop %ecx\n"
      "\tmov tableEntry,%dh\n"
      "\tor %edi,%edx\n"
      "\tinc %ecx\n"
      "\tmov %ecx,sourcePos\n"
      "\tmov $8,%cx\n"
      "\tsub %bx,%cx\n"
      "\tmov %edx,%edi\n"
      "\tshr %cl,%di\n"
      "\txor %ecx,%ecx\n"
      "\tmov destOffs,%ebx\n"
      "\tmov %dl,%ch\n"
      "\tmov %ecx,%edx\n"
      "\tmov sourcePos,%ecx\n"
      "\tand $0xff,%edi\n"
      "\tpush %ecx\n"
      "\tmov outputWord,%ecx\n"
      "\tor %edi,%ecx\n"
      "\tmov %ecx,outputWord\n"
      "\tpop %ecx\n"
      "\tmovzbw (%ecx),%di\n"
      "\n"
      "\tor %edi,%edx\n"
      "\tinc %ecx\n"
      "\tmov %edx,tableEntry\n"
      "\tmov %ecx,sourcePos\n"
      "\tjmp .writeDec\n"
      "\n"
      ".label4:\n"
      "\tmov $7,%ecx\n"
      "\tmov %eax,%edi\n"
      "\tsub %esi,%ecx\n"
      "\tshl %cl,%edi\n"
      "\tmov currTablePos,%ecx\n"
      "\tshl $6,%ecx\n"
      "\tor %ecx,%edi\n"
      "\txor %ecx,%ecx\n"
      "\ttest %eax,%eax\n"
      "\tmov destTable(,%edi,2),%cx\n"
      "\tmov %ecx,destOffs\n"
      "\tjz .label5\n"
      "\tmov currTablePos,%edi\n"
      "\txor %ecx,%ecx\n"
      "\tmov imcTable1(,%edi,2),%cx\n"
      "\tmov %ecx,%edi\n"
      "\tlea -1(%esi),%ecx\n"
      "\tshr %cl,%edi\n"
      "\tmov destOffs,%ecx\n"
      "\tadd %edi,%ecx\n"
      "\n"
      ".label5:\n"
      "\ttest %edx,%edx\n"
      "\tjz .label6\n"
      "\tneg %ecx\n"
      "\n"
      ".label6:\n"
      "\tmov outputWord,%edx\n"
      "\tadd %ecx,%edx\n"
      "\tcmp $-0x8000,%edx\n"
      "\tjge .label7\n"
      "\tmov $-0x8000,%edx\n"
      "\tmov %edx,outputWord\n"
      "\tjmp .writeDec\n"
      "\n"
      ".label7:\n"
      "\tcmp $0x7fff,%edx\n"
      "\tmov %edx,outputWord\n"
      "\tjle .writeDec\n"
      "\tmov $0x7fff,%edx\n"
      "\tmov %edx,outputWord\n"
      "\n"
      ".writeDec:\n"
      "\tmov destPos,%ecx\n"
      "\tmov bytesToDec,%edx\n"
      "\tpush %eax\n"
      "\tmov outputWord,%eax\n"
      "\tmov %ax,(%ecx)\n"
      "\tadd %edx,%ecx\n"
      "\tmov offsets(,%esi,4),%edx\n"
      "\tmov %ecx,destPos\n"
      "\tmov currTablePos,%ecx\n"
      "\tpop %eax\n"
      "\tmovsbl (%edx,%eax,1),%eax\n"
      "\tadd %eax,%ecx\n"
      "\tmov %ecx,currTablePos\n"
      "\tjns .label8\n"
      "\tmovl $0,currTablePos\n"
      "\tjmp .done\n"
      "\n"
      ".label8:\n"
      "\tmov currTablePos,%ecx\n"
      "\tmov $0x58,%eax\n"
      "\tcmp %eax,%ecx\n"
      "\tjle .done\n"
      "\tmov %eax,currTablePos\n"
      "\n"
      ".done:\n"
      "\tmov decsLeft,%eax\n"
      "\tdec %eax\n"
      "\tmov %eax,decsLeft\n"
      "\tjnz .nextDec\n"
      "\tmov entrySize,%edx\n"
      "\tmov sBytesPos,%esi\n"
      "\tmov sWordsPos,%eax\n"
      "\tmov destPos_sWordsPos,%edi\n"
      "\tmov currTablePos,%cl\n"
      "\tadd $2,%eax\n"
      "\tmov %cl,sBytes(%esi)\n"
      "\tpush %ebx\n"
      "\tmov outputWord,%ebx\n"
      "\tmov %bx,-2(%eax)\n"
      "\tpop %ebx\n"
      "\tinc %esi\n"
      "\tmov %eax,sWordsPos\n"
      "\tcmp %edx,%esi\n"
      "\tmov %esi,sBytesPos\n"
      "\tjb .nextByte\n"
      "\n"
      ".exitMainDec:\n"
      "\tpop %edi\n"
      "\tpop %esi\n"
      "\tpop %ebx\n"
      "\tpop %ebp\n"
      "\tpop %esp\n");
}

int eax, bit;

void decompressVima() {
  sourcePos = (unsigned char *) sourceBuffer;  // sourcePos in ecx
  entrySize = 1;  // entrySize in esi
  sBytes[0] = *sourcePos;
  sourcePos++;
  if (sBytes[0] & 0x80) {
    sBytes[0] = ~sBytes[0];
    entrySize = 2;
  }
  sWords[0] = (sourcePos[0] << 8) | sourcePos[1];
  sourcePos += 2;
  if (entrySize > 1) {
    sBytes[1] = *sourcePos;
    sourcePos++;
    sWords[1] = (sourcePos[0] << 8) | sourcePos[1];
    sourcePos += 2;
  }
  currTablePos = 1;
  decsToDo = decLength / (2 * entrySize);
  if (currTablePos == 0) {
    sBytes[1] = 0;
    sBytes[0] = 0;
    sWords[1] = 0;
    sWords[0] = 0;
  }
  tableEntry = (sourcePos[0] << 8) | sourcePos[1];
  sourcePos += 2;
  sBytesPos = 0;
  if (entrySize <= 0)
    goto exitMainDec;
  sWordsPos = sWords;
  destPos_sWordsPos = destOffs - (int) sWords;
  destOffs = 0;  // destOffs in ebx
 nextByte:
  // edi = destPos_sWordsPos, eax = sWordsPos, esi = sBytesPos
  destPos = destPos_sWordsPos + (int) sWordsPos;
  currTablePos = (signed char) sBytes[sBytesPos];
  outputWord = (signed short) *sWordsPos;
  if (decsToDo == 0)
    goto done;
  decsLeft = decsToDo;
  bytesToDec = entrySize * 2;
 nextDec:
  currTableVal = imcTable2[currTablePos];
  destOffs += currTableVal & 0xff;
  bit = 1 << (currTableVal - 1);
  var40 = bit - 1;
  eax = (tableEntry & 0xffff) >> (16 - destOffs);
  eax &= (bit | var40);
  if (destOffs > 7) {
    tableEntry = ((tableEntry & 0xff) << 8) | *sourcePos;
    sourcePos++;
    destOffs -= 8;
  }
  if (eax & bit)
    eax ^= bit;
  else
    bit = 0;
  if (eax == var40) {
    outputWord = ((signed short) (tableEntry << destOffs) & 0xffffff00);
    tableEntry = ((tableEntry & 0xff) << 8) | *sourcePos;
    sourcePos++;
    outputWord |= ((tableEntry >> (8 - destOffs)) & 0xff);
    tableEntry = ((tableEntry & 0xff) << 8) | *sourcePos;
    sourcePos++;
  }
  else {
    int index = (eax << (7 - currTableVal)) | (currTablePos << 6);
    int delta;
    delta = destTable[index];
    if (eax != 0)
      delta += (imcTable1[currTablePos] >> (currTableVal - 1));
    if (bit != 0)
      delta = -delta;
    outputWord += delta;
    if (outputWord < -0x8000)
      outputWord = -0x8000;
    else if (outputWord > 0x7fff)
      outputWord = 0x7fff;
  }
  *((uint16_t *) destPos) = outputWord;
  destPos += bytesToDec;
  currTablePos += (signed char) offsets[currTableVal][eax];
  if (currTablePos < 0)
    currTablePos = 0;
  else if (currTablePos > 88)
    currTablePos = 88;
 done:
  decsLeft--;
  if (decsLeft != 0)
    goto nextDec;
  sBytes[sBytesPos] = currTablePos;
  *sWordsPos = outputWord;
  sWordsPos++;
  sBytesPos++;
  if (sBytesPos < entrySize)
    goto nextByte;
 exitMainDec:
  return;
}

int main(int /* argc */, char *argv[]) {
  initVima();

  FILE *f = fopen(argv[1], "rb");
  if (f == NULL) {
    perror(argv[1]);
    return 1;
  }

  char magic[4];
  fread(magic, 4, 1, f);
  if (memcmp(magic, "MCMP", 4) != 0) {
    fprintf(stderr, "Not a valid file\n");
    return 1;
  }
  uint16_t numBlocks = getc(f) << 8;
  numBlocks |= getc(f);
  char *blocks = new char[9 * numBlocks];
  fread(blocks, 9, numBlocks, f);

  uint16_t numCodecs = getc(f) << 8;
  numCodecs |= getc(f);
  numCodecs /= 5;
  char *codecs = new char[5 * numCodecs];
  fread(codecs, 5, numCodecs, f);

  for (int i = 0; i < numBlocks; i++) {
    int codec = blocks[9 * i];
    int uncompSize = get_be_uint32(blocks + 9 * i + 1);
    int compSize = get_be_uint32(blocks + 9 * i + 5);

    sourceBuffer = new char[compSize];
    fread(sourceBuffer, 1, compSize, f);

    if (strcmp(codecs + 5 * codec, "NULL") == 0)
      fwrite(sourceBuffer, 1, uncompSize, stdout);
    else if (strcmp(codecs + 5 * codec, "VIMA") == 0) {
      decLength = uncompSize;
      char *buffer = new char[uncompSize];
      destOffs = (int) buffer;
      decompressVima();
      fwrite(buffer, 1, uncompSize, stdout);
      delete[] buffer;
    }
    else {
      fprintf(stderr, "Unrecognized codec %s\n", codecs + 5 * codec);
      return 1;
    }
    delete[] sourceBuffer;
  }
  return 0;
}
