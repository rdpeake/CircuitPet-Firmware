#include <Pins.hpp>
#include "Game3.h"
#include "../../GameEngine/Rendering/SpriteRC.h"
#include "../../GameEngine/Rendering/StaticRC.h"
#include "../../GameEngine/Collision/RectCC.h"
#include <Input/Input.h>
#include <time.h>

Game3::Game3() : Game("/Games/Game3", {
		{ "/Background.raw", {}, true },
		{ "/Bomb.raw",       {}, true },
		{ "/Dynamite.raw",   {}, true },
		{ "/Nut.raw",        {}, true },
		{ "/Screw.raw",      {}, true },
		{ "/DuckWalk.gif",   {}, true },
		{ "/DuckEat.gif",    {}, true }
}){}

void Game3::onLoad(){
	srand (time(NULL));

	bg = std::make_shared<GameObject>(
			std::make_unique<StaticRC>(getFile("/Background.raw"), PixelDim{ 160, 128 }),
			nullptr
	);
	addObject(bg);
	bg->getRenderComponent()->setLayer(-1);

	duck = new Duck(getFile("/DuckWalk.gif"), getFile("/DuckEat.gif"));
	addObject(duck->getGameObject());

	addTemplate("/Nut.raw", PixelDim{ 13, 13 }, 15);
	addTemplate("/Screw.raw", PixelDim{ 5, 15 }, 5);
	addTemplate("/Bomb.raw", PixelDim{ 14, 22 }, 0);
	addTemplate("/Dynamite.raw", PixelDim{ 5, 20 }, 0);

}

void Game3::onLoop(float deltaTime){
	duck->loop(deltaTime);
}

void Game3::onStart(){
	Input::getInstance()->addListener(this);
}

void Game3::onStop(){
	Input::getInstance()->removeListener(this);
}

void Game3::onRender(Sprite* canvas){
}

void Game3::buttonPressed(uint i){
	if(i == BTN_BACK){
		pop();
	}
}

void Game3::addTemplate(std::string file, PixelDim dim, int value){
	Template temp{ file, dim, value };
	if(value > 0){
		foods.push_back(temp);
	}else{
		bombs.push_back(temp);
	}
}

void Game3::spawnRandom(){
	int randNum = rand()%(101);
	if(randNum <= 70){
		spawnItem(foods[0]);
	}else{
		spawnItem(bombs[0]);
	}
}

void Game3::spawnItem(Game3::Template temp){
	int randPos = rand()%(161- temp.dim.x);
	auto go = std::make_shared<GameObject>(
			std::make_unique<StaticRC>(getFile(temp.path),temp.dim ),
			std::make_unique<RectCC>(temp.dim)
	);
	addObject(go);
	go->setPos({randPos,0});
	Item item{go,foods[0].value};
	collision.addPair(*duck->getGameObject(), *item.go,  [this, item](){ collisionHandler(item);});
}

void Game3::collisionHandler(Item item){
	removeObject(item.go);
	duck->startEating();
	if(item.value > 0){
		hugerMeter -= item.value;
	}else{
		lives--;
	}
	Serial.printf("lives: %d\thunger: %d\n", lives, hugerMeter);
}

