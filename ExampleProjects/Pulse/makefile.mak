
light.nl: light.txt morsemuzak.exe
	morsemuzak.exe -ilight.txt -olight.nl

# for Windoze
morsemuzak.exe: morsemuzak.cpp
	cl morsemuzak.cpp
