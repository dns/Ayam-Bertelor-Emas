
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


#pragma comment(linker,"\"/manifestdependency:type='win32' \
	name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
	processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#pragma comment(lib, "Comctl32.lib")


#undef UNICODE			// use 7-bit ASCII

#include <windows.h>
#include <commctrl.h>
#include <richedit.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <tchar.h>
#include <stdbool.h>

#include "darray.h"
#include "ayam.h"


#define DEBUG false		// true: debug msg on,   false: debug msg off


DWORD period;
ARRAY_FLOAT *prices;
ARRAY_FLOAT *sma;
ARRAY_FLOAT *stddev;

FLOAT sd_max, profit_loss;
CHAR buf[100];

ORDER order;


BOOL APIENTRY DllMain (HMODULE hModule, DWORD nReason, LPVOID Reserved) {
	switch (nReason) {
		case DLL_PROCESS_ATTACH:
			DisableThreadLibraryCalls(hModule);		//  For optimization (for 1 thread only)
			break;
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
	}
	return TRUE;
}


void calc_sma () {
	DWORD last_index, size, i;
	FLOAT tick, mean, tmp;
	
	size = arrayFloat_size(prices);
	last_index = size - 1;

	if (size > period) {
		tmp = 0.0f;
		for (i=last_index; i>(last_index-period); i--) {
			tmp += arrayFloat_get(prices, i);
		}
		mean = (float) tmp / period;
		arrayFloat_add(sma, mean);

	} else {
		arrayFloat_add(sma, -1.0f);
	}
    
}

/* calc_stddev() must be called after calc_sma() */
void calc_stddev () {
	DWORD last_index, size, i;
	float tick, mean, tmp, sd;

	size = arrayFloat_size(prices);
	last_index = size - 1;

	if (size > period) {
		tmp = 0.0f;
		mean = arrayFloat_get(sma, last_index);
		for (i=last_index; i>(last_index-period); i--) {
			tmp += pow( (arrayFloat_get(prices,i) - mean), 2);
		}
		sd = pow(tmp / period, (double) 5e-1);		/* sqrt(x), x^1/2, x^0.5f, x^5e-1 */
		arrayFloat_add(stddev, sd);

	} else {
		arrayFloat_add(stddev, -1.0f);
	}
}

void ayam_init (DWORD mt_period) {
	period = mt_period;			// 100 tick
	prices = arrayFloat_new();
	sma = arrayFloat_new();
	stddev = arrayFloat_new();

	order.state = false;
	order.type = MARKET_OPEN_FLAT;
	order.open_order = 0.0f;

	sd_max = 0.0f;
	profit_loss = 0.0f;

	// check for errors
	//if (prices != NULL && sma != NULL && stddev != NULL)
		//MessageBoxA(NULL, "Initialization Complete !", "DEBUG", MB_OK);

}


DWORD ayam_start (double tick, ANALYZE type) {
	DWORD size;

	size = arrayFloat_size(prices);
	arrayFloat_add(prices, tick);

	calc_sma();
	calc_stddev();

	if (sd_max < arrayFloat_last(stddev))
		sd_max = arrayFloat_last(stddev);

	if (type == ANALYZE_OPEN)
		return open_market();
	else if (type == ANALYZE_CLOSE)
		return close_market();
}


void ayam_deinit () {
	int pl_pips;
	
	pl_pips = profit_loss * 1e+4;
	sprintf(buf, "sd_max: %f\nprofit_loss: %f\nProfit/Loss (pips): %d\n", sd_max, profit_loss, pl_pips);

	MessageBoxA(NULL, buf, "ayam_deinit()", MB_OK);
	
	arrayFloat_destroy(prices);
	arrayFloat_destroy(stddev);
	arrayFloat_destroy(sma);
}



MARKET_OPEN open_market () {
	DWORD size;
	char signal[10];
	MARKET_OPEN result = MARKET_OPEN_FLAT;

	size = arrayFloat_size(prices);
	strcpy(signal, "");

	if (size > period) {
		if (arrayFloat_last(stddev) > 0.0002f) {
			if (arrayFloat_last(prices) > arrayFloat_last(sma)) {
				strcpy(signal, "BUY");
				result = MARKET_OPEN_BUY;
			} else if (arrayFloat_last(prices) < arrayFloat_last(sma)) {
				strcpy(signal, "SELL");
				result = MARKET_OPEN_SELL;
			}
		} else {
			strcpy(signal, "FLAT");
			result = MARKET_OPEN_FLAT;
		}
	}

	if ((result == MARKET_OPEN_SELL) ^ (result == MARKET_OPEN_BUY) && order.state == false) {
		if (DEBUG == true) {
			sprintf(buf, "Price: %f\nSMA: %f\nSTDDEV: %f\nsignal: %s",
				arrayFloat_last(prices), arrayFloat_last(sma), arrayFloat_last(stddev), signal);
			MessageBoxA(NULL, buf, "enter_market()", MB_OK);
		}

		order.state = true;
		order.type = result;
		order.open_order = arrayFloat_last(prices);

	}


	return result;
}

MARKET_CLOSE close_market () {
	MARKET_CLOSE result = MARKET_CLOSE_NO;
	DWORD size;
	
	size = arrayFloat_size(prices);

	if (size > period && order.state == true) {
		if (arrayFloat_last(stddev) < 0.0002f) {
			if (order.type == MARKET_OPEN_SELL) {
				profit_loss += arrayFloat_last(prices) - order.open_order;
				result = MARKET_CLOSE_SELL_OK;
				//MessageBoxA(NULL, "MARKET_CLOSE_SELL_OK", "close_market()", MB_OK);
			} else if (order.type == MARKET_OPEN_BUY) {
				profit_loss += arrayFloat_last(prices) - order.open_order;
				result = MARKET_CLOSE_BUY_OK;
				//MessageBoxA(NULL, "MARKET_CLOSE_BUY_OK", "close_market()", MB_OK);
			}


			order.state = false;
			order.type = MARKET_OPEN_FLAT;
			order.open_order = 0.0f;
		}
	}


	

	return result;
}


//MessageBoxA(NULL, "asd", "enter_market", MB_OK);