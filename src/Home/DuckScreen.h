#ifndef CIRCUITPET_FIRMWARE_DUCKSCREEN_H
#define CIRCUITPET_FIRMWARE_DUCKSCREEN_H

#include "../State.h"
#include <Loop/LoopListener.h>
#include <Input/InputListener.h>
#include "Sprites/BgSprite.h"
#include "Sprites/OSSprite.h"
#include "Sprites/CharacterSprite.h"
#include "Sprites/StatsSprite.h"
#include "Menu/Menu.h"
#include "Menu/MenuHider.h"

class DuckScreen : public LoopListener, public State, private InputListener {
public:
	DuckScreen(Sprite* base);
	void loop(uint micros) override;

protected:
	void onStart() override;
	void onStop() override;

private:
	Sprite* base;

	BgSprite bgSprite;
	OSSprite osSprite;
	CharacterSprite characterSprite;
	StatsSprite statsSprite;
	Menu menu;
	MenuHider hider;

	std::vector<MenuItem> menuItems;

	constexpr static uint8_t osX = 125;
	constexpr static uint8_t osY = 1;

	constexpr static uint8_t characterX = 60;
	constexpr static uint8_t characterY = 60;

	constexpr static uint8_t statsX = 1;
	constexpr static uint8_t statsY = 0;

	constexpr static uint8_t menuY = 64;

	void buttonPressed(uint i) override;
};


#endif //CIRCUITPET_FIRMWARE_DUCKSCREEN_H
