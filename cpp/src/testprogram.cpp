internal
u8 GlobalTestProgram[]
{
  /* 0x00 */ INSTRUCTION(0x00E0),
  /* 0x02 */ INSTRUCTION(0xA20C),
  /* 0x04 */ INSTRUCTION(0x6001),
  /* 0x06 */ INSTRUCTION(0x6102),
  /* 0x08 */ INSTRUCTION(0xD015),
  /* 0x0A */ INSTRUCTION(0xFFFF),

  // Sprite.
  /* 0x0C */ 0b1000'0000,
  /* 0x0D */ 0b0100'0000,
  /* 0x0E */ 0b0010'1000,
  /* 0x0F */ 0b0001'1000,
  /* 0x10 */ 0b0011'1000,

  // Padding.
  /* 0x11 */ 0b0000'0000,
};
