#include "DuckScreen.h"
#include <Loop/LoopManager.h>
#include "../Stats/StatsManager.h"
#include <Chatter.h>

DuckScreen::DuckScreen(Sprite* base) : State(), base(base), bgSprite(base, StatMan.getLevel()),
									   osSprite(base, StatMan.getLevel()),
									   characterSprite(base, StatMan.getLevel(), StatMan.get().oilLevel, Anim::General),
									   statsSprite(base, StatMan.get().happiness, StatMan.get().oilLevel, 100), //TODO - set battery level instead of '100' placeholder
									   menu(base), hider(&menu){

	osSprite.setPos(osX, osY);
	statsSprite.setPos(statsX, statsY);
	characterSprite.setPos(characterX, characterY);
	menu.setOffsetY(menuY);

	menuItems = {
			{ "Oily", GameImage(base, "/MenuIcons/Icon1.raw"), {} },
			{ "Flappy", GameImage(base, "/MenuIcons/Icon2.raw"), {} },
			{ "Eaty", GameImage(base, "/MenuIcons/Icon3.raw"), {} },
			{ "Jump & Duck", GameImage(base, "/MenuIcons/Icon4.raw"), {} },
			{ "Disco danceoff", GameImage(base, "/MenuIcons/Icon5.raw"), {} },
			{ "Space duck", GameImage(base, "/MenuIcons/Icon6.raw"), {} },
	};

	menu.setItems(menuItems);
}


void DuckScreen::onStart(){
	Input::getInstance()->addListener(this);
	LoopManager::addListener(this); //Note - possible crash if start() is called before constructor finishes
}

void DuckScreen::onStop(){
	Input::getInstance()->removeListener(this);
	LoopManager::removeListener(this);
}

void DuckScreen::loop(uint micros){
	bgSprite.push();

	statsSprite.push();

	osSprite.push();

	characterSprite.loop(micros);
	characterSprite.push();

	menu.draw();

	base->push();
}

void DuckScreen::buttonPressed(uint i){
	hider.activity();

	switch(i){
		case BTN_LEFT:
			menu.prev();
			break;
		case BTN_RIGHT:
			menu.next();
			break;
		case BTN_A:
			menuItems[menu.getSelectedIndex()].primary();
			return;
		default:
			break;
	}
}

