default:   
	g++ ./main.cpp -o ./build/Endure.exe -O2 -Wall -Wno-missing-braces -I ./include/ -L ./lib/ -lraylib -lopengl32 -lgdi32 -lwinmm
	./build/Endure.exe