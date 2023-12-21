#pragma once

#include <cstdint>

constexpr auto BIT(auto s) { return 1 << s; }


//-----------------------------------------------------------------------------

using BitMask32 = uint32_t;
using BitMask16 = uint16_t;

class BitSet32
{
	BitMask32   bset;

public:
	BitSet32()                             { bset = 0; }
	BitSet32(BitMask32 s)                  { bset = s; }

	operator    unsigned int() const       { return bset; }
	BitMask32   operator = (BitMask32 m)   { return bset = m; }
	BitMask32   operator |= (BitMask32 m)  { return bset |= m; }
	BitMask32   operator &= (BitMask32 m)  { return bset &= m; }
	BitMask32   operator ^= (BitMask32 m)  { return bset ^= m; }

	BitMask32   set()                      { return bset = ~0; }
	BitMask32   set(BitMask32 s)           { return bset |= s; }
	BitMask32   set(BitMask32 s, bool b)   { return bset = (bset&~s)|(b?s:0); }
	BitMask32   clear()                    { return bset = 0; }
	BitMask32   clear(BitMask32 s)         { return bset &= ~s; }
	uint32_t    test(BitMask32 s) const    { return (bset & s) != 0; }
	BitMask32   toggle(BitMask32 s)        { return bset ^= s; }

	BitMask32   mask() const               { return bset; }
};


//-----------------------------------------------------------------------------


class BitSet16
{
	BitMask16   bset;

public:
	BitSet16()                             { bset = 0; }
	BitSet16(BitMask16 s)                  { bset = s; }

	operator    unsigned int() const       { return bset; }
	BitMask16   operator = (BitMask16 m)   { return bset = m; }

	BitMask16   set()                      { return bset = ~0; }
	BitMask16   set(BitMask16 s)           { return bset |= s; }
	BitMask16   clear()                    { return bset = 0; }
	BitMask16   clear(BitMask16 s)         { return bset &= BitMask16(~s); }
	uint16_t    test(BitMask16 s) const    { return bset & s; }

	BitMask16   mask() const               { return bset; }
};
