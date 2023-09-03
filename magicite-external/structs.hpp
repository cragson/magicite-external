#pragma once
#include <cstdint>

class playerinfo
{
public:
	char pad_0000[40]; //0x0000
	int32_t m_iFood; //0x0028
	char pad_002C[4]; //0x002C
	int32_t m_iHealth; //0x0030
	int32_t m_iLevel; //0x0034
	float m_flCurrentXp; //0x0038
	float m_flNeededXpForLvlUp; //0x003C
	char pad_0040[44]; //0x0040
	int32_t m_iMoney; //0x006C
	char pad_0070[12]; //0x0070
	float m_flSpeed; //0x007C
	int32_t m_iMaximumHealth; //0x0080
	char pad_0084[140]; //0x0084
}; //Size: 0x0110
