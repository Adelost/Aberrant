#include "Sound.h"

void TW_CALL tw_playSound(void *clientData)
{ 
	Sound *in = static_cast<Sound *>(clientData); // scene pointer is stored in clientData
	in->start(0);                            
}

void Sound::buildMenu(TwBar* menu)
{
	// Filter menu
	TwEnumVal seasonsEV[] = { {NONE, "None"}, {RADIO, "Radio"}, {MUFFLED, "Muffled"}};
	TwType seasonType;
	seasonType = TwDefineEnum("SeasonType", seasonsEV, 3);
	TwAddVarRW(menu, "Filter", seasonType, &season, "group='Sound'");
	
	// Volume/Freq menu
	TwAddVarRW(menu, "Music Volume",	TW_TYPE_FLOAT,	&music_volume,	"group='Sound'	step=0.01	min=0.00	max=1.00");
	TwAddVarRW(menu, "Music Frequency", TW_TYPE_FLOAT,	&music_freq,	"group='Sound'	step=0.01	min=0.01	max=2.00");
	
	TwAddButton(menu, "Test Sound", tw_playSound, this, "group=Sound");
	TwAddVarRW(menu, "Test Sound",		TW_TYPE_FLOAT,	&sound_freq,	"group='Sound'	step=0.01	min=0.00");
	TwAddVarRW(menu, "Sound Volume",	TW_TYPE_FLOAT,	&sound_volume,	"group='Sound'	step=0.01	min=0.00	max=1.00");
	TwAddVarRW(menu, "Sound Frequency",	TW_TYPE_FLOAT,	&sound_freq,	"group='Sound'	step=0.01	min=0.01	max=2.00");
	

	TwDefine("Settings/Sound opened=false");

	
}