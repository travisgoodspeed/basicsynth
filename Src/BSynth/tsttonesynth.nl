' Test Tone Synth
' Daniel R. Mitchell
tempo 4, 60;

voice 0
begin
	instr "Test";
	channel 0;
	volume 100;
	artic percent 80;
	write "Test OSC wavetable";
	' OSC wavetable
	'map "Test" 16;
	map "Test" "oscwt";
	{C4, C4, C4, C4, C4}, %4, 100, 
	{ 0,  1,  2,  3, 4 }; 
	R, %4;
	'LFO frequency, wavetable, attack, level
	'map "Test" 25, 26, 27, 28;
	map "Test" "lfofrq", "lfowt", "lfoatk", "lfoamp";
	{C4, C4, C4, C4, C4, C4, C4}, %4, 100, 
	{ 2.0, 2.5, 3.0 3.5, 4.0 4.5, 5.0},
	{ 0, 0, 1, 1, 2, 2, 0},
	{ 0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6},
	{ 0.05, 0.10, 0.2, 0.3, 0.4, 0.5, 0.8};
	R, %4;
	' EG attack, peak, decay, sustain, release
	map "Test" 18, 19, 20, 21, 22;
	{C4, C4, C4, C4, C4, C4, C4}, %4, 100, 
	{ 0.01, 0.05, 0.10, 0.15, 0.20, 0.30, 0.40},
	{ 0.50, 0.60, 0.70, 0.80, 0.90, 1.00, 1.00},
	{ 0.01, 0.05, 0.10, 0.15, 0.20, 0.30, 0.40},
	{ 0.40, 0.50, 0.60, 0.70, 0.80, 0.90, 1.00},
	{ 0.05, 0.10, 0.15, 0.20, 0.30, 0.40, 0.50};
	R, %4;
	' EG {attack release}, peak
	map "Test" {18, 22}, 19;
	{C4, C4, C4, C4, C4, C4, C4}, %4, 100, 
	{ 0.01, 0.05, 0.10, 0.15, 0.20, 0.30, 0.40},
	{ 0.50, 0.60, 0.70, 0.80, 0.90, 1.00, 1.00};
	R, %2;

	instr "TestFM";
	' OSC wavetable
	map "TestFM" 16;
	{C4, C4, C4, C4, C4}, %4, 100, 
	{ 0,  1,  2,  3, 4 }; 
	R, %4;
	' Modulation mult, index
	map "TestFM" 36, 35;
	{C4, C4, C4, C4, C4, C4, C4, C4}, %4, 100, 
	{ 1,  2,  3,  4,  1,  1,  1, 1},
	{ 1,  1,  1,  1,  2,  4,  8, 10};
end
