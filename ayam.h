
/*
Project : Ayam Bertelor Emas
Status  : Complete
Author  : Daniel Sirait <dns@cpan.org>
Copyright   : Copyright (c) 2013 - 2014, Daniel Sirait
License : Proprietary
Disclaimer  : I CAN UNDER NO CIRCUMSTANCES BE HELD RESPONSIBLE FOR
ANY CONSEQUENCES OF YOUR USE/MISUSE OF THIS DOCUMENT,
WHATEVER THAT MAY BE (GET BUSTED, WORLD WAR, ETC..).
*/

#ifndef _AYAM_H
#define _AYAM_H


#ifdef __cplusplus
extern "C" {
#endif
//
//typedef enum {
//	MARKET_OPEN_FLAT = 0,
//	MARKET_OPEN_BUY = 1,
//	MARKET_OPEN_SELL = 2,
//} MARKET_OPEN;
//
//
//typedef enum {
//	MARKET_CLOSE_NO = 3,
//	MARKET_CLOSE_BUY_OK = 4,
//	MARKET_CLOSE_SELL_OK = 5
//} MARKET_CLOSE;
//

#define MARKET_OPEN DWORD
#define MARKET_OPEN_FLAT 0
#define MARKET_OPEN_BUY 1
#define MARKET_OPEN_SELL 2

#define MARKET_CLOSE DWORD
#define MARKET_CLOSE_NO 3
#define MARKET_CLOSE_BUY_OK 4
#define MARKET_CLOSE_SELL_OK 5


typedef struct _ORDER {
	MARKET_OPEN type;
	bool state;
	float open_order;
} ORDER;

typedef enum {
	ANALYZE_OPEN = 1001,
	ANALYZE_CLOSE = 1002
} ANALYZE;

MARKET_OPEN open_market ();
MARKET_CLOSE close_market ();
void test_test_123 ();
void calc_sma ();
void calc_stddev ();
FLOAT pips2point(FLOAT);

__declspec(dllexport) void __cdecl ayam_init (DWORD);
__declspec(dllexport) DWORD __cdecl ayam_start (double, ANALYZE);
__declspec(dllexport) void __cdecl ayam_deinit ();
__declspec(dllexport) void ayam_mt4stoploss();

#ifdef __cplusplus
}
#endif


/* _AYAM_H */
#endif

