// synthesizer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "olcNoiseMaker.h";



// Oscilator function types
#define OSC_SINE 0
#define OSC_SQUARE 1
#define OSC_TRIANGLE 2
#define OSC_SAW_ANA 3
#define OSC_SAW_OPT 4
#define OSC_NOISE 5




// Holds output frequency
atomic<double> frequencyOutput = 0.0;
// Base octave: A2
double octaveBaseFrequency = 110.0;
// Scaling factor for 12-tone equal temperament
double twelveToneScaling = pow(2.0, 1.0 / 12.0);
// Master Volume
double masterVolume = 0.4;

// Takes a frequency (Hz) and returns it in angular velocity
double w(double hertz)
{
	return hertz * 2 * PI;
}

// Takes a note in Hertz and what interval is wanted, returns that interval in Hertz.
// A positive interval returns a note in the higher octave, a negative in the lower.
double noteInterval(double note, int interval)
{
	return note * pow(twelveToneScaling, interval);
}

// Returns amplitude as a function of various types of oscilators
double osc(double hertz, double time, int type = OSC_SINE)
{
	switch (type)
	{
		// Sine Wave
		case OSC_SINE:
			return sin(w(hertz) * time);
		// Square Wave
		case OSC_SQUARE:
			return sin(w(hertz) * time) > 0.0 ? 1.0 : -1.0;
		// Triangle Wave
		case OSC_TRIANGLE:
			return asin(sin(w(hertz) * time)) * 2.0 / PI;
		// Saw Wave (Analog)
		case OSC_SAW_ANA:
		{
			double output = 0.0;

			for (double n = 1.0; n < 100.0; n++) {
				output += sin(n * w(hertz) * time) / n;
			}

			return output * (2.0 / PI);
		}
		// Saw Wave (Optimised)
		case OSC_SAW_OPT:
			return (2.0 / PI) * (hertz * PI * fmod(time, 1.0 / hertz) - (PI / 2.0));
		// Noise
		case OSC_NOISE:
			return 2.0 * ((double) rand() / (double) RAND_MAX) - 1.0;
		
		default: 0.0;
	}
}






// Envelope (Attack, Decay, Sustain, Release)
struct EnvelopeADSR
{

	double attackTime;
	double decayTime;
	double releaseTime;

	double attackAmplitude;
	double sustainAmplitude;

	// Time that the note starts playing
	double triggerOnTime;
	// Time that the note releases
	double triggerOffTime;

	bool noteOn;

	EnvelopeADSR()
	{
		attackTime = 0.10;
		decayTime = 0.01;
		attackAmplitude = 1.0;
		sustainAmplitude = 0.8;
		releaseTime = 0.20;
		triggerOffTime = 0.0;
		triggerOnTime = 0.0;
		noteOn = false;
	}

	wstring GetPhase(double time) {
		wstring * phase = new wstring(L"\rERR");

		double lifeTime = time - triggerOnTime;

		if (noteOn)
		{
			if (lifeTime <= attackTime)
				phase = new wstring(L"\rATTACK");
			if (lifeTime > attackTime && lifeTime <= (decayTime + attackTime))
				phase = new wstring(L"\rDECAY");
			if (lifeTime > (attackTime + decayTime))
				phase = new wstring(L"\rSUSTAIN");
		}
		else
			phase = new wstring(L"\rRELEASE");

		return *phase;

	}

	// Starts playing envelope
	void NoteOn(double timeOn) {
		triggerOnTime = timeOn;
		noteOn = true;
	}

	// Releases envelope
	void NoteOff(double timeOff) {
		triggerOffTime = timeOff;
		noteOn = false;
	}

	// Returns the amplitude of the envelope at a given time
	double GetAmplitude(double time) {
	
		// How long the note has been playing for
		double lifeTime = time - triggerOnTime;
		double amplitude = 0.0;

		// y = mx + c : Amplitude = gradient * time (normalised to phase time) + base amplitude of phase

		if (noteOn)
		{
			// Attack Phase (base amplitude is 0)
			if (lifeTime <= attackTime)
			{
				amplitude = 
					attackAmplitude *
					(lifeTime / attackTime);
			}
			// Decay Phase
			if (lifeTime > attackTime && lifeTime <= (attackTime + decayTime))
			{
				amplitude =
					(sustainAmplitude - attackAmplitude) *
					((lifeTime - attackTime) / decayTime) +
					attackAmplitude;
			}

			// Sustain Phase (constant amp)
			if (lifeTime > (attackTime + decayTime))
				amplitude = sustainAmplitude;

		}
		else
		{
			// Release Phase
			amplitude = 
				(0.0 - sustainAmplitude) *
				((time - triggerOffTime) / releaseTime) +
				sustainAmplitude;
		}


		// Epsilon Check: Prevents tiny or negative amplitude
		if (amplitude < 0.0001)
			amplitude = 0.0;

		return amplitude;

	}

};







EnvelopeADSR envelope;

// Make a sound for some given time
double MakeNoise(double time)
{
	double output = envelope.GetAmplitude(time) *
		(
			+ 1.0 * osc(frequencyOutput * 0.5, time, OSC_SINE)
			+ 1.0 * osc(frequencyOutput * 1.0, time, OSC_SAW_ANA)
		);
	// Master volume scaling
	return output * masterVolume;

}


int main()
{
    std::wcout << "lofi-synth" << std::endl;

	//	Get sound hardware
	vector<wstring> devices = olcNoiseMaker<short>::Enumerate();
	//	Display what was found
	for (auto d : devices) std::wcout << "Found sound device: " << d << std::endl;
	//	Create sound machine
	olcNoiseMaker<short> sound(devices[0], 44100, 1, 8, 512);

	sound.SetUserFunction(MakeNoise);

	// Keyboard indicator

	// Display a keyboard
	wcout << endl <<
		"	._________________________________________________________." << endl <<
		"	|   |   |   |   |   | |   |   |   |   | |   | |   |   |   |" << endl <<
		"	|   | S |   |   | F | | G |   |   | J | | K | | L |   |   |" << endl <<
		"	|   |___|   |   |___| |___|   |   |___| |___| |___|   |   |__" << endl <<
		"	|     |     |     |     |     |     |     |     |     |     |" << endl <<
		"	|  Z  |  X  |  C  |  V  |  B  |  N  |  M  |  ,  |  .  |  /  |" << endl <<
		"	|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|" << endl << endl;

	
	// Tracks which key was pressed
	int currentKey = -1;
	bool keyDown = false;
	
	while (1)
	{

		// Keyboard

		keyDown = false;
		for (int k = 0; k < 17; k++)
		{
			if (GetAsyncKeyState((unsigned char)("ZSXCFVGBNJMK\xbcL\xbe\xbf"[k])) & 0x8000) {

				if (currentKey != k)
				{
					envelope.NoteOn(sound.GetTime());
					frequencyOutput = noteInterval(octaveBaseFrequency, k);
					currentKey = k;
				}

				keyDown = true;
			}
		}

		if (!keyDown)
		{
			if (currentKey != -1)
			{
				envelope.NoteOff(sound.GetTime());
				currentKey = -1;
			}
		}

		wcout << envelope.GetPhase(sound.GetTime());

	}


	return 0;

}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu