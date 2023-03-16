// 共通定義
#include "common.h"

// TWELTIE------------------------------>
/// @brief 整数を100で割った値を表示する
/// @param val 表示したい値
void print_div100(uint16_t val)
{
	Serial.print(val / 100, DEC);
	Serial.print(".");
	uint16_t val_frac = val % 100;
	if (val_frac < 10)
		Serial.print('0');
	Serial.print(val_frac, DEC);
}

/// @brief 整数を100で割った値を表示する
/// @param val 表示したい値
void print_div100(int16_t val)
{
	uint16_t val_abs = val < 0 ? -val : val;
	if (val < 0)
		Serial.print('-');
	print_div100(val_abs);
}
