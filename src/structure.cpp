#include "structure.h"

block::block(std::string str, int a, int b) : blockName(str), width(a), height(b){
	xCoor = 0;
	yCoor = 0;
	xCenter = 0;
	yCenter = 0;
	parent = NULL;
	leftChild = NULL;
	rightChild = NULL;
};

void block::updateCenter(){
	xCenter = (2 * xCoor + width) / 2;
	yCenter = (2 * yCoor + height) / 2;
}

net::net() {
	bottomX = 0;
	bottomY = 0;
	topX = 0;
	topY = 0;
};
