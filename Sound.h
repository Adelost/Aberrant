#ifndef SOUND_H
#define SOUND_H

#define XAUDIO2_HELPER_FUNCTIONS
#include <windows.h>
#include <xaudio2.h>
#include "wave.h"
#include <vector>
#include "Util.h"
using namespace std;

//structure stored in the extra window bytes
struct SubmixVoices
{
	IXAudio2SubmixVoice* sound;
	IXAudio2SubmixVoice* music;
};

typedef enum {NONE, RADIO, MUFFLED} Seasons;

struct Filter
{
	XAUDIO2_FILTER_PARAMETERS none;
	XAUDIO2_FILTER_PARAMETERS radio;
	XAUDIO2_FILTER_PARAMETERS muffled;
};

class Sound
{
private:
	IXAudio2* engine;
	vector<IXAudio2SourceVoice*> sources;
	IXAudio2SourceVoice* musicSource;
	IXAudio2MasteringVoice* master;

	SubmixVoices submix;
	Filter filter;

	// Vars
	float music_volume;
	float sound_volume;
	float music_freq;
	float sound_freq;

	Seasons season;

public:
	Sound(){
		engine = NULL;
		master = NULL;

		music_volume = 0.15f;
		sound_volume = 0.2f;
		music_freq = 1.0f;
		sound_freq = 0.5f;

		season = NONE;
	};
	~Sound(){
		engine->Release();
		CoUninitialize();
	};

	void init(){
		//Init CO
		CoInitializeEx(NULL, COINIT_MULTITHREADED);

		//Create engine
		XAudio2Create(&engine);

		//Create the mastering voice
		engine->CreateMasteringVoice(&master);

		//Load sound-files
		loadMusic("dire_docks.wav", XAUDIO2_LOOP_INFINITE);
		loadSound("thunder.wav", 1);

		//Create submix voices
		submix.sound = create_submix(sources[0]);
		submix.music = create_submix(musicSource);
		submix.music->SetVolume(0.0f);

		//Link source voices to submix
		XAUDIO2_VOICE_SENDS *sendLink = get_sendLink(submix.sound);
		for(int i = 0; i < (int)sources.size(); i++)
			sources[i]->SetOutputVoices(sendLink);
		musicSource->SetOutputVoices(get_sendLink(submix.music));

		//Filter
		createFilters();

		//Modify volume
		float volumes [2] = { 1.0f, 0.0f}; 
		musicSource->SetChannelVolumes(1, volumes);
	};
	void buildMenu(TwBar* menu);

	void createFilters(){
		//None
		filter.none.Type = LowPassFilter;
		filter.none.Frequency = 1.0f;
		filter.none.OneOverQ = 1.0f;

		//Radio
		filter.radio.Type = BandPassFilter;
		filter.radio.Frequency = 0.3f;
		filter.radio.OneOverQ = 0.18f;

		//Muffled
		filter.muffled.Type = BandPassFilter;
		filter.muffled.Frequency = 0.035f;
		filter.muffled.OneOverQ = 0.83f;
	};

	void loadSound(string file, UINT LoopCount){
		//Load wave file
		string path = "data/sound/"+file;
		Wave* buffer = new Wave();
		if( !buffer->load(path.c_str(), LoopCount))
		{
			engine->Release();
			CoUninitialize();
			throw "Failed to load audio file "+file+"!";
		}

		//Create the source voice, based on loaded wave format
		IXAudio2SourceVoice* sourceVoice;
		engine->CreateSourceVoice( &sourceVoice, buffer->wf());
		sourceVoice->SubmitSourceBuffer(buffer->xaBuffer());
		sources.push_back(sourceVoice);
	};
	void loadMusic(string file, UINT LoopCount){
		//Load wave file
		string path = "data/sound/"+file;
		Wave* buffer = new Wave();
		if( !buffer->load(path.c_str(), LoopCount))
		{
			engine->Release();
			CoUninitialize();
			throw "Failed to load audio file "+file+"!";
		}

		//Create the source voice, based on loaded wave format
		engine->CreateSourceVoice( &musicSource, buffer->wf(), XAUDIO2_VOICE_USEFILTER);

		//Start music
		musicSource->Start(0, XAUDIO2_COMMIT_NOW);
		musicSource->SubmitSourceBuffer(buffer->xaBuffer());
	};

	void start(int index){
		sources[index]->SetFrequencyRatio(1.0f);
		sources[index]->Start(0, XAUDIO2_COMMIT_NOW);
	};
	void stop(int index){
		sources[index]->Stop();
		sources[index]->FlushSourceBuffers();
	};

	XAUDIO2_VOICE_SENDS* get_sendLink(IXAudio2SubmixVoice *submix){
		XAUDIO2_SEND_DESCRIPTOR *sendDesc = new XAUDIO2_SEND_DESCRIPTOR();
		sendDesc->Flags = 0;
		sendDesc->pOutputVoice = submix;
		XAUDIO2_VOICE_SENDS *voiceSends = new XAUDIO2_VOICE_SENDS();
		voiceSends->SendCount = 1;
		voiceSends->pSends = sendDesc;

		return voiceSends;
	};

	IXAudio2SubmixVoice* create_submix(IXAudio2SourceVoice *source){
		IXAudio2SubmixVoice* submix;
		submix = NULL;
		XAUDIO2_VOICE_DETAILS sd = {0};
		source->GetVoiceDetails(&sd);
		engine->CreateSubmixVoice(&submix, sd.InputChannels, sd.InputSampleRate);
		return submix;
	}

	//void sterioPanning(D3DXVECTOR3 listner, D3DXVECTOR3 ){
	//	IXAudio2SubmixVoice* submix;
	//	submix = NULL;
	//	XAUDIO2_VOICE_DETAILS sd = {0};
	//	source->GetVoiceDetails(&sd);
	//	HR(engine->CreateSubmixVoice(&submix, sd.InputChannels, sd.InputSampleRate));
	//	return submix;
	//}

	void update(float dt)
	{
		// Set volume
		submix.music->SetVolume(music_volume);
		submix.sound->SetVolume(sound_volume);

		// Change frequency
		musicSource->SetFrequencyRatio(music_freq);
		sources[0]->SetFrequencyRatio(sound_freq);

		//Apply filter
		static Seasons currentState = season;
		if(currentState!=season)
		{
			if(season == NONE)
				musicSource->SetFilterParameters(&filter.none);
			if(season == RADIO)
				musicSource->SetFilterParameters(&filter.radio);
			if(season == MUFFLED)
				musicSource->SetFilterParameters(&filter.muffled);
			
			currentState = season;
		}

		////Calculate 3D-sound
		//D3DXVECTOR3 listner = D3DXVECTOR3( 0, 0, 0 );
		//D3DXVECTOR3 emiter = D3DXVECTOR3( 0, 0, 10);
		//     if( g_audioState.vListenerPos.x != g_audioState.listener.Position.x
		//         || g_audioState.vListenerPos.z != g_audioState.listener.Position.z )
		//     {
		//         D3DXVECTOR3 vDelta = g_audioState.vListenerPos - g_audioState.listener.Position;

		//         g_audioState.fListenerAngle = float( atan2( vDelta.x, vDelta.z ) );

		//         vDelta.y = 0.0f;
		//         D3DXVec3Normalize( &vDelta, &vDelta );

		//         g_audioState.listener.OrientFront.x = vDelta.x;
		//         g_audioState.listener.OrientFront.y = 0.f;
		//         g_audioState.listener.OrientFront.z = vDelta.z;
		//     }

		//     if (g_audioState.fUseListenerCone)
		//     {
		//         g_audioState.listener.pCone = (X3DAUDIO_CONE*)&Listener_DirectionalCone;
		//     }
		//     else
		//     {
		//         g_audioState.listener.pCone = NULL;
		//     }
		//     if (g_audioState.fUseInnerRadius)
		//     {
		//         g_audioState.emitter.InnerRadius = 2.0f;
		//         g_audioState.emitter.InnerRadiusAngle = X3DAUDIO_PI/4.0f;
		//     }
		//     else
		//     {
		//         g_audioState.emitter.InnerRadius = 0.0f;
		//         g_audioState.emitter.InnerRadiusAngle = 0.0f;
		//     }

		//     if( fElapsedTime > 0 )
		//     {
		//         D3DXVECTOR3 lVelocity = ( g_audioState.vListenerPos - g_audioState.listener.Position ) / fElapsedTime;
		//         g_audioState.listener.Position = g_audioState.vListenerPos;
		//         g_audioState.listener.Velocity = lVelocity;

		//         D3DXVECTOR3 eVelocity = ( g_audioState.vEmitterPos - g_audioState.emitter.Position ) / fElapsedTime;
		//         g_audioState.emitter.Position = g_audioState.vEmitterPos;
		//         g_audioState.emitter.Velocity = eVelocity;
		//     }

		//     DWORD dwCalcFlags = X3DAUDIO_CALCULATE_MATRIX | X3DAUDIO_CALCULATE_DOPPLER
		//         | X3DAUDIO_CALCULATE_LPF_DIRECT | X3DAUDIO_CALCULATE_LPF_REVERB
		//         | X3DAUDIO_CALCULATE_REVERB;
		//     if (g_audioState.fUseRedirectToLFE)
		//     {
		//         // On devices with an LFE channel, allow the mono source data
		//         // to be routed to the LFE destination channel.
		//         dwCalcFlags |= X3DAUDIO_CALCULATE_REDIRECT_TO_LFE;
		//     }

		//     X3DAudioCalculate( g_audioState.x3DInstance, &g_audioState.listener, &g_audioState.emitter, dwCalcFlags,
		//                        &g_audioState.dspSettings );

		//     IXAudio2SourceVoice* voice = g_audioState.pSourceVoice;
		//     if( voice )
		//     {
		//         // Apply X3DAudio generated DSP settings to XAudio2
		//         voice->SetFrequencyRatio( g_audioState.dspSettings.DopplerFactor );
		//         voice->SetOutputMatrix( g_audioState.pMasteringVoice, INPUTCHANNELS, g_audioState.nChannels,
		//                                 g_audioState.matrixCoefficients );

		//         voice->SetOutputMatrix(g_audioState.pSubmixVoice, 1, 1, &g_audioState.dspSettings.ReverbLevel);

		//         XAUDIO2_FILTER_PARAMETERS FilterParametersDirect = { LowPassFilter, 2.0f * sinf(X3DAUDIO_PI/6.0f * g_audioState.dspSettings.LPFDirectCoefficient), 1.0f }; // see XAudio2CutoffFrequencyToRadians() in XAudio2.h for more information on the formula used here
		//         voice->SetOutputFilterParameters(g_audioState.pMasteringVoice, &FilterParametersDirect);
		//         XAUDIO2_FILTER_PARAMETERS FilterParametersReverb = { LowPassFilter, 2.0f * sinf(X3DAUDIO_PI/6.0f * g_audioState.dspSettings.LPFReverbCoefficient), 1.0f }; // see XAudio2CutoffFrequencyToRadians() in XAudio2.h for more information on the formula used here
		//         voice->SetOutputFilterParameters(g_audioState.pSubmixVoice, &FilterParametersReverb);
		//     }
	};
};

#endif