#include "block.h"

#include <iostream>
#include <cstdlib>
#include <iomanip>
#include <string>
#include <vector>
#include <stack>
#include <list>
#include <map>
#include <fstream>
#include <sstream>

void bStarTreeCoor(block* root);

using std::cout;
using std::endl;

int main(int argc, char* argv[]){
	float alpha;
	if(argc <= 1){
		cout << "Usage: [executable file name] [α value] [input.block name] [input.net name] [output file name]\n";
		return 1;
	}
	alpha = atof(argv[1]);
	if(alpha > 1 || alpha <= 0){
		cout << "Usage: [executable file name] [α value] [input.block name] [input.net name] [output file name]\n";
		cout << "Error: Worng size of 'α'\n";
		return 1;
	}
	
	int outlineWidth, outlineHeight, numBlock, numTerminal;
	std::ifstream finBlk(argv[2]), finNet(argv[3]);
	std::string line;
	std::vector<block*> allBlock;
	std::map<std::string, std::pair<int, int> > allTerminal;
	
	if(!finBlk.is_open()) return 1;
	
	getline(finBlk, line);
	std::istringstream token(line);
	std::string word;
	std::stringstream ss;
	
	token >> word;
	token >> outlineWidth;
	token >> outlineHeight;

	getline(finBlk, line);
	token.clear();
	token.str(line);
	token >> word;
	token >> numBlock;
	getline(finBlk, line);
	token.clear();
	token.str(line);
	token >> word;
	token >> numTerminal;

	line.clear();
	while(line.size() == 0 || line[0] == '\r'){
		line.clear();
		getline(finBlk, line);
	}

	block *root;
	for(int i = 0; i < numBlock; i++){
		std::string tmpName;
		int tmpWid, tmpHei;
		token.clear();
		token.str(line);
		token >> tmpName;
		token >> tmpWid;
		token >> tmpHei;
		block* tmp = new block(tmpName, tmpWid, tmpHei);
		allBlock.push_back(tmp);
		if(i == 0){
			tmp->leftChild = NULL;
			tmp->rightChild = NULL;
		}
		else if(i == numBlock-1){
			root = tmp;
			root->leftChild = allBlock[i-1];
			root->rightChild = NULL;
		}
		else {
			tmp->leftChild = allBlock[i-1];
			tmp->rightChild = NULL;
		}
		getline(finBlk, line);
	}

	if(numTerminal != 0){
		line.clear();
		while(line.size() == 0 || line[0] == '\r'){
			line.clear();
			getline(finBlk, line);
		}

		for(int i = 0; i < numTerminal; i++){
			std::string tmpName;
			int _x, _y;
			token.clear();
			token.str(line);
			token >> tmpName;
			token >> word;
			token >> _x;
			token >> _y;
			allTerminal.insert(std::pair<std::string, std::pair<int, int> >(tmpName, std::pair<int, int>(_x, _y)));
			getline(finBlk, line);
		}
	}

	// for(std::map<std::string, std::pair<int, int> >::iterator iter = allTerminal.begin(); iter != allTerminal.end(); iter++){
		// cout << iter->first << " " << iter->second.first << " " << iter->second.second << endl;
	// }
	// cout << allTerminal.find("VDD")->second.first << " " << allTerminal.find("VDD")->second.second << endl;
	
	// for(std::vector<block*>::iterator it = allBlock.begin(); it != allBlock.end(); it++){
		// cout << (*it)->name << " " << (*it)->width << " " << (*it)->height << endl;
	// }
	
	block b1("b1", 90, 20), b2("b2", 20, 40), b3("b3", 40, 60), b4("b4", 60, 30), b5("b5", 110, 10), b6("b6", 30, 80);
	block b7("b7", 50, 20), b8("b8", 40, 40), b9("b9", 40, 40), b10("b10", 40, 40);
	allBlock[3]->rightChild = &b1;
	b1.leftChild = &b2;
	b1.rightChild = &b3;
	b2.leftChild = &b10;
	b3.leftChild = &b4;
	b4.leftChild = &b7;
	b7.leftChild = &b8;
	b8.leftChild = &b9;
	allBlock[1]->rightChild = &b5;
	b5.leftChild = &b6;
	bStarTreeCoor(root);
	
	// int count = 0;
	// block* ptr = root;
	// do{
	// 	count++;
	// 	cout << ptr->name << " " << ptr->xCoor << endl;
	// 	ptr = ptr->leftChild;
	// } while(ptr != NULL);

	// cout << endl << count << endl;

	// for(std::vector<block*>::iterator it = allBlock.begin(); it != allBlock.end(); it++){
	// 	delete (*it);
	// }
	
	// cout << outlineHeight << " " << outlineWidth << " " << numBlock << " " << numTerminal << endl;

	return 0;
}

void bStarTreeCoor(block* root){
	block* current;
	std::stack<block*> unvisited;
	unvisited.push(root);
	root->xCoor = 0;
	std::list<std::pair<int, int> > contour;
	contour.push_back(std::pair<int, int>(0, 0));
	std::list<std::pair<int, int> >::iterator it = contour.begin();
	
	while(!unvisited.empty()){
		current = (unvisited.top());
		unvisited.pop();
		if(current->rightChild != NULL){
			current->rightChild->xCoor = current->xCoor;
			unvisited.push(current->rightChild);
		}
		if(current->leftChild != NULL){
			current->leftChild->xCoor = current->xCoor + current->width;
			unvisited.push(current->leftChild);
		}

		while(it == contour.end() || it->first > current->xCoor) --it;
		std::list<std::pair<int, int> >::iterator tmpIt = it;
		int maxY = 0, currentRightX = current->xCoor + current->width;
		while(tmpIt != contour.end() && tmpIt->first < currentRightX){ // tmpIt->first = list x
			if(tmpIt->second > maxY) maxY = tmpIt->second;
			tmpIt++;
		}

		if(it != contour.begin() && (--it)->second == maxY + current->height){
			it = contour.erase(++it);
			it--;
		}
		else if(it->first != current->xCoor){
			contour.insert(++it, std::pair<int, int>(current->xCoor, maxY + current->height));
			it--;
		}
		else{
			it->second = maxY + current->height;
		}
		contour.insert(++it, std::pair<int, int>(currentRightX, maxY));

		tmpIt = it--;
		while(tmpIt != contour.end() && tmpIt->first <= currentRightX){
			it->second = tmpIt->second;
			contour.erase(tmpIt++);
		}
		current->yCoor = maxY;
			
			// cout << "List: ";
			// for(tmpIt = contour.begin(); tmpIt != contour.end(); tmpIt++){
				// cout << "(" << tmpIt->first << "," << tmpIt->second << ")";
			// }
			// cout << endl;
			// cout << "Iter: (" << it->first << "," << it->second << ") " << endl;
			// cout << "(" << it->first << "," << it->second << ") " << endl;
			// cout << endl;
			cout << current->name << " " << current->xCoor << " " << current->yCoor << endl;
	}
			// for(it = contour.begin(); it != contour.end(); it++){
				// cout << "(" << it->first << "," << it->second << ") ";
			// }
			// cout << endl;
	
	return void();
}
