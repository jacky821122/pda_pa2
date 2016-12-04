#include "block.h"

block::block(std::string str = "", int a = 0, int b = 0) : name(str), width(a), height(b){
	xCoor = 0;
	yCoor = 0;
	leftChild = NULL;
	rightChild = NULL;
};

block::~block(){};
