#ifndef BLOCK_H
#define BLOCK_H

#include <string>

class block{
public:
	block(std::string, int, int);
	~block();
	std::string name;
	int width, height, xCoor, yCoor;
	block *leftChild, *rightChild;
};

#endif
