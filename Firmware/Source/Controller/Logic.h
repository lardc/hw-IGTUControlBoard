#ifndef CONTROLLER_LOGIC_H_
#define CONTROLLER_LOGIC_H_

// Definitions
//
#define INIT_48V_TIMER			10 // Р’СЂРµРјСЏ РґР»СЏ РІС‹СЃС‚Р°РІР»РµРЅРёСЏ 48v РЅР° РїР»Р°С‚Рµ, РІ РјСЃ
#define SW_CURRENT_CH_TIMER		1	// Р’СЂРµРјСЏ РґР»СЏ РїРµСЂРµРєР»СЋС‡РµРЅРёСЏ СЂРµР»Рµ С‚РѕРєР° , РІ РјСЃ
#define REGLTR_TIMER			10	// Р’СЂРµРјСЏ РґР»СЏ РІС‹С…РѕРґР° СЂРµРіСѓР»СЏС‚РѕСЂР° РЅР° СЂР°Р±РѕС‡РµРµ РЅР°РїСЂСЏР¶РµРЅРёРµ, РІ РјСЃ

// Includes
//
#include "stdinc.h"

//Functions
//
void LOGIC_HandleMeasurement();

#endif // CONTROLLER_LOGIC_H_
