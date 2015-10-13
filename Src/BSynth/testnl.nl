' This is a test script for various features of Notelist
' It is not an exhaustive test, but is good for regression
' testing to insure something didn't get broken.

! bang comment
' quote comment
tempo 4, 120;

var value1;
var rhy;

voice 0
begin
	instr "Tone";
	vol 100;
	channel 0;
! test single notes
	C4, 0.1, 100;
	D4, 0.1, 100;
	E4; F4; G4;
! test rest
	C4, 0.1, 100;
	R, 0.1, 100;
	C4, 0.2, 100;
	R, 0.2, 100;
! test group
	{C4, D4, E4, F4, G4}, 0.1, 100;
	C4, {0.1, 0.2, 0.3, 0.4}, 100;
! test all pitch names
	{ C, C#, Db, D, D#, Eb, F, F#, Gb, G, G#, Ab, A, A#, Bb, B }, %16, 100;
	{ c, c#, db, d, d#, eb, f, f#, gb, g, g#, ab, a, a#, bb, b }, %16, 100;
! test octaves
	{ C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10 }, %16, 100;
	{ C3, D, E, F, G, A, B, C4 }, %16, 100;
! test time
	time 10;
	{C4, D4, E4, F4, G4}, %8, 100;
	time curtime+0.5;
	{C4, D4, E4, F4, G4}, %8, 100;
! test expressions
	R, %4;
	C4+1, %4, 100;
	C4-1, %4, 100;
	C4*1.5, %4, 100;
	C4/2, %4, 100;
	
! test transpose
	transpose -12;
	{ C3, D, E, F, G, A, B, C4 }, %16, 100;
	transpose 12;
	{ C3, D, E, F, G, A, B, C4 }, %16, 100;
	transpose 0;
! test rhythms
	C4, {%1, %2, %3, %4, %6, %8, %12, %16, %24, %32 }, 100;
	C4, {W, H, Q, Ei, S, T}, 100;
	C4, {%4+%8, %4*2, %4.5 }, 100;
	C4, {0.5, 0.25, 0.25}, 100;
! test volumes
	C4, %8, {20, 30, 40, 50, 60, 70, 80, 90, 100};
	vol 20;
	C4, %8, 100;
	vol 20;
	C4, %8, 100;
	vol 30;
	C4, %8, 100;
	vol 40;
	C4, %8, 100;
	vol 50;
	C4, %8, 100;
	vol 60;
	C4, %8, 100;
	vol 70;
	C4, %8, 100;
	vol 80;
	C4, %8, 100;
	vol 90;
	C4, %8, 100;
	vol 100;
	C4, %8, 100;
! test loop
	loop 1 C4, %8, 100;
	R, %8;
	loop 4 C4, %8, 100;
	R, %8;
	loop 2
	begin
		loop 2
		begin
			C4+count, %8, 100;
		end
	end
! test channel
	channel 0;
	{C4, E4, G4, C4}, %8, 100;
	channel 1;
	{C4, E4, G4, C4}, %8, 100;
! test sync
	channel 0;
	mark "pos1";
	C3, %4+%8, 100;
	G3, %4+%8, 100;
	C4, %4+%8, 100;
	C3, %4, 100;
! test init/fn
	init 0 line 0, 12, 12;
	init 1 line 10, 100, 12;
	loop 13
		C4+fgen(0,1), %8, fgen(1,1);
	init 0 exp 0, 12, 12;
	loop 13
		C4+fgen(0,1), %8, 100;
	init 0 log 0, 12, 12;
	loop 13
		C4+fgen(0,1), %8, 100;
	init 0 rand -5, 7, 12;
	loop 13
		C4+fgen(0,1), %8, 100;
! test chords
	R, %4;
	[C3, G3, C4], %4, 33;
	R, %4;
	[C3, G3, C4], [%1, %2, %4], 33;
! test sustain/tie
	sus {C3, G3, C4, G4, C5}, {%1, %4, %8, %8, %8}, {50, 40, 30, 20};
	sus {C3, G3, C4, G4, C5}, {%1, 0, %4, 0, %4, 0}, {50, 40, 30, 20};
	R, %4;
	tie {C3, G3, C4, G4, C5}, {%1, %8, %16, %16, %8}, {50, 40, 30, 20};
! test variables and conditions
	R, %4;
	set value1 = C4;
	value1, %4, 100;
	set value1 = 50;
	value1, %4, 100;
	while (value1 < C5) do
	begin
		if (value1 >= F#4) then
			set rhy = %8;
		else
			set rhy = %4;
		value1, rhy, 100;
		set value1 = value1 + 1;
	end
	C5, %4, 100;
! test doubling
	double 7, 100;
	{C4, D4, E4, F4, G4}, %8, 100;
	double -12, 30;
	{C4, D4, E4, F4, G4}, %8, 100;
	double off;
	{C4, D4, E4, F4, G4}, %8, 100;
! test articulation
	R, %4;
	artic add 0.1;
	{C4, D4, E4, F4, G4}, %8, 100;
	R, %4;
	artic fixed 0.5;
	{C4, D4, E4, F4, G4}, %8, 100;
	R, 0.5;
	artic percent 80;
	{C4, D4, E4, F4, G4}, %8, 100;
	R, %4;
	artic percent param;
	{C4, D4, E4, F4, G4}, %8, 100, 200;
end

sequence "seq1"
begin
	{C5, G4, G5, C5}, {%8, %16, %16, %8}, 100;
end

voice 2
begin
	instrument "Tone";
	volume 100;
	channel 0;
	sync "pos1";
	play "seq1";
	play "seq1";
	play "seq1";
end
