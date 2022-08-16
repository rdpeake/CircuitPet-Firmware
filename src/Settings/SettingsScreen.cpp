#include "SettingsScreen.h"
//#include "../UserHWTest/UserHWTest.h"
#include <Input/Input.h>
#include <FS/CompressedFile.h>
#include <Settings.h>
#include <SPIFFS.h>
#include <Pins.hpp>
#include <CircuitPet.h>
#include <Loop/LoopManager.h>
#include <nvs_flash.h>

SettingsScreen::SettingsScreen* SettingsScreen::SettingsScreen::instance = nullptr;

SettingsScreen::SettingsScreen::SettingsScreen(Display& display) : screen(display), scrollLayout(new ScrollLayout(&screen)),
																   screenLayout(new LinearLayout(scrollLayout, VERTICAL)),
																   shutDownSlider(new DiscreteSlider(screenLayout, "Auto shutdown", { 0, 1, 5, 15, 30 })),
																   brightnessSlider(new SliderElement(screenLayout, "Brightness")),
																   soundSwitch(new BooleanElement(screenLayout, "Sound")),
																   rgbSlider(new SliderElement(screenLayout, "RGB brightness")),
																   hwTest(new TextElement(screenLayout, "Hardware test")),
																   factoryReset(new TextElement(screenLayout, "Factory reset")),
																   save(new TextElement(screenLayout, "Save")),
																   elements({ shutDownSlider, brightnessSlider, soundSwitch, rgbSlider, hwTest, factoryReset, save }){
	instance = this;
	buildUI();
	shutDownSlider->setIsSelected(true);
	shutDownSlider->setIndex(Settings.get().shutdownTime);
	brightnessSlider->setSliderValue(Settings.get().screenBrightness);
	soundSwitch->setBooleanSwitch(Settings.get().sound);
	rgbSlider->setSliderValue(Settings.get().RGBbrightness);
	screen.pack();
}

void SettingsScreen::SettingsScreen::onStart(){
	screen.unpack();

	Input::getInstance()->addListener(this);
	Input::getInstance()->setButtonHeldRepeatCallback(BTN_RIGHT, 150, [](uint){
		if(instance == nullptr) return;
		if(instance->selectedSetting == 1 && instance->editMode){
			instance->brightnessSlider->moveSliderValue(instance->scrollStep);

			Settings.get().screenBrightness = instance->brightnessSlider->getSliderValue();
			CircuitPet.setBrightness(instance->brightnessSlider->getSliderValue());

			instance->scrollCount++;
			if(instance->scrollCount >= 3){
				instance->scrollCount = 0;
				instance->scrollStep *= 2;
			}
		}else if(instance->selectedSetting == 3 && instance->editMode){
			instance->rgbSlider->moveSliderValue(instance->scrollStep);

			Settings.get().RGBbrightness = instance->rgbSlider->getSliderValue();
			RGB.setBrightness(instance->rgbSlider->getSliderValue());

			instance->scrollCount++;
			if(instance->scrollCount >= 3){
				instance->scrollCount = 0;
				instance->scrollStep *= 2;
			}
		}


		instance->draw();
	});

	Input::getInstance()->setButtonHeldRepeatCallback(BTN_LEFT, 150, [](uint){
		if(instance == nullptr) return;
		if(instance->selectedSetting == 1){
			instance->brightnessSlider->moveSliderValue(-instance->scrollStep);

			Settings.get().screenBrightness = instance->brightnessSlider->getSliderValue();
			CircuitPet.setBrightness(instance->brightnessSlider->getSliderValue());

			instance->scrollCount++;
			if(instance->scrollCount >= 3){
				instance->scrollCount = 0;
				instance->scrollStep *= 2;
			}
		}else if(instance->selectedSetting == 3){
			instance->rgbSlider->moveSliderValue(-instance->scrollStep);

			Settings.get().RGBbrightness = instance->rgbSlider->getSliderValue();
			RGB.setBrightness(instance->rgbSlider->getSliderValue());

			instance->scrollCount++;
			if(instance->scrollCount >= 3){
				instance->scrollCount = 0;
				instance->scrollStep *= 2;
			}
		}


		instance->draw();
	});
	backgroundBuffer = static_cast<Color*>(malloc(160 * 128 * 2));
	if(backgroundBuffer == nullptr){
		Serial.println("SettingsScreen background unpack error");
		return;
	}

	fs::File bgFile = SPIFFS.open("/Bg/settings.raw");
	bgFile.read(reinterpret_cast<uint8_t*>(backgroundBuffer), 160 * 128 * 2);
	bgFile.close();

	draw();
}

void SettingsScreen::SettingsScreen::onStop(){
	Input::getInstance()->removeListener(this);
	LoopManager::removeListener(this);
	Input::getInstance()->removeButtonHeldRepeatCallback(BTN_RIGHT);
	Input::getInstance()->removeButtonHeldRepeatCallback(BTN_LEFT);
	RGB.setColor({ 0, 0, 0 });

	free(backgroundBuffer);
	screen.pack();
}

void SettingsScreen::SettingsScreen::draw(){
	screen.getSprite()->drawIcon(backgroundBuffer, 0, 0, 160, 128, 1);
//	screen.getSprite()->setTextColor(TFT_WHITE);
//	screen.getSprite()->setTextSize(1);
//	screen.getSprite()->setTextFont(1);
//	screen.getSprite()->setCursor(screenLayout->getTotalX() + 42, screenLayout->getTotalY() + 110);
//	screen.getSprite()->print("Version 1.0");

	scrollLayout->draw();
	screen.commit();
}

SettingsScreen::SettingsScreen::~SettingsScreen(){
	instance = nullptr;
}

void SettingsScreen::SettingsScreen::buildUI(){
	scrollLayout->setWHType(PARENT, PARENT);
	scrollLayout->setHeight(screen.getHeight());
	screenLayout->setWHType(PARENT, CHILDREN);
	screenLayout->setGutter(3);

	for(auto& el : elements){
		screenLayout->addChild(el);
	}

	scrollLayout->reflow();
	screenLayout->reflow();
	scrollLayout->addChild(screenLayout);
	screen.addChild(scrollLayout);
	screen.repos();
}

void SettingsScreen::SettingsScreen::buttonPressed(uint id){
	switch(id){
		case BTN_LEFT:
			if(editMode){
				if(selectedSetting == 0){
					shutDownSlider->selectPrev();
				}else if(selectedSetting == 1){
					brightnessSlider->moveSliderValue(-1);
					Settings.get().screenBrightness = brightnessSlider->getSliderValue();
					CircuitPet.setBrightness(brightnessSlider->getSliderValue());
				}else if(selectedSetting == 3){
					rgbSlider->moveSliderValue(-1);
					Settings.get().RGBbrightness = rgbSlider->getSliderValue();
					RGB.setBrightness(rgbSlider->getSliderValue());
				}
			}else{
				elements[selectedSetting]->setIsSelected(false);
				selectedSetting--;
				if(selectedSetting < 0){
					selectedSetting = elements.size() - 1;
				}
				elements[selectedSetting]->setIsSelected(true);
				scrollLayout->scrollIntoView(selectedSetting, 0);
			}
			break;

		case BTN_RIGHT:
			if(editMode){
				if(selectedSetting == 0){
					shutDownSlider->selectNext();
				}else if(selectedSetting == 1){
					brightnessSlider->moveSliderValue(1);
					Settings.get().screenBrightness = brightnessSlider->getSliderValue();
					CircuitPet.setBrightness(brightnessSlider->getSliderValue());
				}else if(selectedSetting == 3){
					rgbSlider->moveSliderValue(1);
					Settings.get().RGBbrightness = rgbSlider->getSliderValue();
					RGB.setBrightness(rgbSlider->getSliderValue());
				}
			}else{
				elements[selectedSetting]->setIsSelected(false);
				selectedSetting++;
				if(selectedSetting > elements.size() - 1){
					selectedSetting = 0;
				}
				elements[selectedSetting]->setIsSelected(true);
				scrollLayout->scrollIntoView(selectedSetting, 0);
			}
			break;

		case BTN_A:
			if(selectedSetting == 0 || selectedSetting == 1 || selectedSetting == 3){
				editMode = !editMode;
				if(selectedSetting == 3){
					cycleRGB = !cycleRGB;
					if(cycleRGB){
						LoopManager::addListener(this);
					}else{
						LoopManager::removeListener(this);
						RGB.setColor(Pixel::Black);
					}
				}
			}
			elements[selectedSetting]->toggle();
			if(selectedSetting == 2){
				Settings.get().sound = instance->soundSwitch->getBooleanSwitch();
//				Playback.updateGain();
//				Playback.tone(500, 50);
			}else if(selectedSetting == 4){
//				Context* hwTest = new UserHWTest(*CircuitPet.getDisplay());
//				hwTest->push(this);
//				draw();
				break;
			}else if(selectedSetting == 5){
				nvs_flash_erase();
				CircuitPet.fadeOut();
				ESP.restart();
			}
			break;

		case BTN_B:
			if(editMode){
				editMode = false;
				elements[selectedSetting]->toggle();
				cycleRGB = false;
				LoopManager::removeListener(this);
				RGB.setColor(Pixel::Black);
			}else{
				Settings.get().shutdownTime = shutDownSlider->getIndex();
				Settings.get().sound = soundSwitch->getBooleanSwitch();
				Settings.get().RGBbrightness = rgbSlider->getSliderValue();
				Settings.store();
//			Playback.updateGain();
				popped = true;
				LoopManager::addListener(this);
				return;
			}
			break;
	}

	draw();
}


typedef struct {
	double r;       // a fraction between 0 and 1
	double g;       // a fraction between 0 and 1
	double b;       // a fraction between 0 and 1
} rgb;

typedef struct {
	double h;       // angle in degrees
	double s;       // a fraction between 0 and 1
	double v;       // a fraction between 0 and 1
} hsv;

rgb hsv2rgb(hsv in){
	double hh, p, q, t, ff;
	long i;
	rgb out;

	if(in.s <= 0.0){       // < is bogus, just shuts up warnings
		out.r = in.v;
		out.g = in.v;
		out.b = in.v;
		return out;
	}
	hh = in.h;
	if(hh >= 360.0) hh = 0.0;
	hh /= 60.0;
	i = (long)hh;
	ff = hh - i;
	p = in.v * (1.0 - in.s);
	q = in.v * (1.0 - (in.s * ff));
	t = in.v * (1.0 - (in.s * (1.0 - ff)));

	switch(i){
		case 0:
			out.r = in.v;
			out.g = t;
			out.b = p;
			break;
		case 1:
			out.r = q;
			out.g = in.v;
			out.b = p;
			break;
		case 2:
			out.r = p;
			out.g = in.v;
			out.b = t;
			break;

		case 3:
			out.r = p;
			out.g = q;
			out.b = in.v;
			break;
		case 4:
			out.r = t;
			out.g = p;
			out.b = in.v;
			break;
		case 5:
		default:
			out.r = in.v;
			out.g = p;
			out.b = q;
			break;
	}
	return out;
}

void SettingsScreen::SettingsScreen::loop(uint micros){
	if(popped){
		pop();
		return;
	}
	double hue = 360.0f * ((double)((millis() % 2000) / 2000.0));

	hsv h = { hue, 1, 1 };
	rgb c = hsv2rgb(h);

	RGB.setColor({ (uint8_t)(c.r * 255), (uint8_t)(c.g * 255), (uint8_t)(c.b * 255) });

}

void SettingsScreen::SettingsScreen::buttonReleased(uint id){
	if(id == BTN_LEFT || id == BTN_RIGHT){
		scrollStep = 1;
		scrollCount = 0;
	}

}
