#ifndef STRUCTURE_H
#define STRUCTURE_H

#include <string>
#include <vector>

class block{
public:
	block(std::string str = "", int a = 0, int b = 0);
	std::string blockName;
	int width, height, xCoor, yCoor;
	int xCenter, yCenter;
	block *leftChild, *rightChild, *parent;
	void updateCenter();
};

class net{
public:
	net();
	// std::string netName;
	std::vector<std::string> blocksInNet;
	int bottomX, bottomY, topX, topY;
};

#endif
