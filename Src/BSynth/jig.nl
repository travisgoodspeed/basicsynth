' Test "jig"
' Daniel R. Mitchell
tempo 4, 144;

map "Fidl" 32:1.5, 43;

voice 0
begin      
	instr "Fidl";   
	channel 0;          
	artic percent, param;  
	transpose -12;   
	R, %1*3;  
	[C5, G5], %1*2, 40, 80, 1.0;
	[C6, G5], %1, 50, 80, 0.5;
	volume 140;  
	loop 2
	begin
		loop 4
		begin
			{C5, C5, C5, G5, C5, C5}, %12, {100, 80, 80, 100, 80, 100}, {60, 100, 100, 60, 100, 100}, 
			{0.04, 0.09, 0.09, 0.04, 0.09, 0.07}, { 2.0, 1.6 };
		end
		loop 2
		begin
			{C5, G5, A5, F5, E5, G5}, %12, {100, 80, 80, 100, 80, 100}, {100, 80, 80, 100, 80, 80}, 
			{0.09, 0.04, 0.04, 0.09, 0.09, 0.07}, { 2.2, 1.6 };
		end        
		{F5, E5, D5, G4}, {%8, %8, %6, %12}, 100, {60, 60, 80, 100};
		loop 2
		begin
			{C5, C5, C5, G5, C5, C5}, %12, {100, 80, 80, 100, 80, 100}, {80, 100, 80, 80, 100, 100}, {0.04, 0.09, 0.09, 0.04, 0.09, 0.07};
		end
		{C5, G5, G5, F5, E5, G5}, %12, {100, 80, 80, 100, 80, 100}, {80, 100, 80, 80, 100, 100}, {0.04, 0.09, 0.09, 0.04, 0.09, 0.07};
		{F5, E5, D5, G4}, {%8, %8, %6, %12}, 100, {60, 60, 80, 100}, {0.04, 0.09, 0.09, 0.04, 0.09, 0.07};
		{C5, G5, A5, F5, E5, G5}, %12, 100, {80, 100, 80, 80, 100, 100}, {0.04, 0.09, 0.09, 0.04, 0.09, 0.07}; 
		'transpose 0;
		vol 90;
		double 12, 100;
	end
	[C5, G5], %2, {80, 80}, 100, 0.1;
end          

voice 1
begin      
	instr "Fidl";   
	channel 3;
	volume 110;
	artic percent, param;  
	loop 2
	begin
		C1, %1, 80, 100, 0.5;
		C2, %1, 80, 100;
		C1, %1, 80, 100;
	end
	loop 6
	begin  
		C2, %1, 50, 100, 0.1;
		C1, %1, 50, 100;
	end       
	C1, %2, 50, 100;
end

voice 2
begin      
	instr "Drum";   
	channel 1;     
	artic fixed, 0.3;
	loop 12
	begin
		[C3, G3], %4, 80;
		G2, %4, 100;
	end
	loop 2
	begin
		loop 11
		begin                 
			[C3, G3], %4, 60;
			G2, %4, 80;
		end
		{G2, A2, B2}, %6, 100;  
	end       
	C3, %2, 80;
end

voice 3
begin      
	instr "Drum";  
	channel 2;     
	artic fixed, 0.4;   
	loop 2
	begin
		R, %1;   
		loop 4
		begin           
			R, %6;
			{C4, G4, C4}, %12, 90;
			R, %12;
		end
	end
	loop 2
	begin
		loop 11
		begin                 
			R, %6;
			C4, %12, 80;
			G4, %12, 100;
			C4, %12, 80;
			R, %12;
		end
		{G4, R, R, D4}, {%6, %6, %12, %12}, 100;  
	end       
	C4, %2, 80;
end
