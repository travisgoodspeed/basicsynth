' This is a test script for various features of Notelist
! bang comment
' quote comment
version 1.0;
tempo 4, 120;
'middlec 0;
'system "echo hello";
'script "testscript";
'write "Write string " :: "pasted\n";
maxparam 3;
! attack in ms, release
map "Pulse"  10+8:0.001, 22:(1/1000);

var abc;
var var1, var2;
set abc = 1;

seq "seq1"
begin
	{C4, C5, G4}, %16;
end

voice 0
begin
	instr "Flut";
	vol 100;
	channel 2;
	C4, %4, 100;
	C, %8, 90;
	48, .125;
	mark "a";
	loop 3
	begin
		transpose 12*count;
		C4, %4, 100;
		D4, %4, 100;
		E4, %4, 100;
		F4, %4, 100;
		G4, %4, 100;
		A4, %4, 100;
		B4, %4, 100;
		if count > 1 then 
			set var1 = 50;
		else
			set var1 = 80;
		vol var1;
	end
	transpose 0;
	vol 100;
	init 0 line 0, 5, 5;
	loop (5) C4+fgen(0,1), %12;
	time curtime+0.5;
	C4, %8, 100;
	loop (4) curpit+1, curdur, curvol-10;
	C4, %16, 100;
	{D4, E4, F4, G4};
	C4, %16, 100;
	{curpit+2, curpit+4, curpit+5, curpit+7};
	R, %2;
	artic add 0.1;
	double 12,50;
	play "seq1";
	play "seq1";
	double off;
	mark "2";
	
	C5, %1, 100;
end

voice 1
begin
	instr "Pulse";
	vol 100;
	channel 3;
	sync "a";
	repeat 2
	begin
		C3, %1, 100, 1, 20;
		G2, %4*3, 100, 100, 200;
	end
	sync "2";
	C2, %1, 100, 500, 500;
end

voice 0 note C5, %4;
voice 1 note R, %4;

voice 0 channel 0;
voice 1 channel 1;
include "test2.nl";

var val1;
voice 0
begin
  set val1 = C4;
  loop (20)
  begin
    set val1 = val1 + rand(-12,12);
    write "Pitch: " :: val1;
    while val1 > C5 do
      set val1 = val1 - 12;
    while val1 < C3 do
      set val1 = val1 + 12;
    val1, %16, 100;
  end
  mark 3;
end

voice 1 sync 3;
