// synthesizer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "olcNoiseMaker.h";

// Holds output frequency
atomic<double> dFrequencyOutput = 0.0;
// Base octave: A2
double dOctaveBaseFrequency = 110.0;
// Scaling factor for 12-tone equal temperament
double d12thRootOf2 = pow(2.0, 1.0 / 12.0);
// Master Volume
double dMasterVolume = 0.4;

// Takes a frequency (Hz) and returns it in angular velocity
double w(double dHertz)
{
	return dHertz * 2 * PI;
}

// Takes a note in Hertz and what interval is wanted, returns that interval in Hertz.
// A positive interval returns a note in the higher octave, a negative in the lower.
double noteInterval(double note, int interval)
{
	if (interval < -12 || interval > 12)
		return -1;

	return note * pow(d12thRootOf2, interval);

}

// Returns amplitude as a function of various types of oscilators
double osc(double dHertz, double dTime, int type)
{
	switch (type)
	{
		// Sin Wave
		case 0:
			return sin(w(dHertz) * dTime);
		// Square Wave
		case 1:
			return sin(w(dHertz) * dTime) > 0.0 ? 1.0 : -1.0;
		// Triangle Wave
		case 2:
			return asin(sin(w(dHertz) * dTime)) * 2.0 / PI;
		default: 0.0;
	}
}

// Make a sound for some given time
double MakeNoise(double dTime)
{
	double dOutput = 1.0 * osc(dFrequencyOutput, dTime, 2);

	// Master volume scaling
	return dOutput * dMasterVolume;

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
		"|   |   |   |   |   | |   |   |   |   | |   | |   |   |   |" << endl <<
		"|   | S |   |   | F | | G |   |   | J | | K | | L |   |   |" << endl <<
		"|   |___|   |   |___| |___|   |   |___| |___| |___|   |   |__" << endl <<
		"|     |     |     |     |     |     |     |     |     |     |" << endl <<
		"|  Z  |  X  |  C  |  V  |  B  |  N  |  M  |  ,  |  .  |  /  |" << endl <<
		"|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|" << endl << endl;

	while (1)
	{

		// Keyboard

		bool keyDown = false;
		for (int k = 0; k < 17; k++)
		{
			if (GetAsyncKeyState((unsigned char)("ZSXCFVGBNJMK\xbcL\xbe\xbf"[k])) & 0x8000) {
				dFrequencyOutput = dOctaveBaseFrequency * pow(d12thRootOf2, k);
				keyDown = true;
			}
		}

		if (!keyDown)
			dFrequencyOutput = 0.0;

	}


	return 0;

}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu