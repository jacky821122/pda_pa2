#include "structure.h"

#include <iostream>
#include <cstdlib>
#include <algorithm>
#include <iomanip>
#include <string>
#include <vector>
#include <stack>
#include <list>
#include <map>
#include <cmath>
#include <fstream>
#include <sstream>
#include <ctime>
#include <limits>

void bStarTreeCoor(block* root, int &planWidth, int &planHeight);
void Perturb(std::vector<block*> blocks, int blockNum, block **root);
void SwapNode(std::vector<block*> blocks, int id_a, int id_b);
void DeleteAndInsert(std::vector<block*> blocks, int id_a, int id_b, block **root);
void DeleteNode(std::vector<block*> blocks, int id_a, block **root);
void Insert(std::vector<block*> blocks, int id_a, int id_b);
double Penalty(float alpha, int planWidth, int planHeight, int outlineWidth, int outlineHeight);
block* TreeCopy(std::vector<block*> &copy, std::vector<net*> &nets_copy, std::vector<block*> origin, std::vector<net*> nets_origin);

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

	srand(time(0));
	clock_t begin_time = clock();
	
	int outlineWidth, outlineHeight, numBlock, numTerminal;
	std::ifstream finBlk(argv[2]), finNet(argv[3]);
	std::string line;
	std::vector<block*> vBlocks;
	std::map<std::string, block*> allBlock;
	std::map<std::string, std::pair<int, int> > allTerminal;
	
	/*
	-----------------------------------------------Parse .block file-----------------------------------------------
	 */
	if(!finBlk.is_open()) {
		cout << "Cannot open the file \"" << argv[2] << "\".\n";
		cout << "Usage: [executable file name] [α value] [input.block name] [input.net name] [output file name]\n";
		return 1;
	}

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
	block *root, *lastBlock;
	for(int i = 0; i < numBlock; i++){
		std::string tmpName;
		int tmpWid, tmpHei;
		token.clear();
		token.str(line);
		token >> tmpName;
		token >> tmpWid;
		token >> tmpHei;
		block* tmp = new block(tmpName, tmpWid, tmpHei);
		vBlocks.push_back(tmp);
		allBlock.insert(std::pair<std::string, block*>(tmpName, tmp));
		if(i == 0){
			tmp->leftChild = NULL;
			tmp->rightChild = NULL;
		} else if(i == numBlock-1) {
			root = tmp;
			root->leftChild = lastBlock;
			lastBlock->parent = tmp;
			root->rightChild = NULL;
			root->parent = NULL;
		} else {
			tmp->leftChild = lastBlock;
			lastBlock->parent = tmp;
			tmp->rightChild = NULL;
		}
		lastBlock = tmp;
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
	line.clear();
	token.clear();
	word.clear();
	ss.clear();
	
	/*
	-----------------------------------------------Parse .net file-----------------------------------------------
	 */
	int numNet;
	if(!finNet.is_open()){
		cout << "Cannot open the file \"" << argv[3] << "\".\nUsage: [executable file name] [α value] [input.block name] [input.net name] [output file name]\n";
		return 1;
	}

	getline(finNet, line);
	token.str(line);
	token >> word;
	token >> numNet;
	std::vector<net*> allNet;
	int planWidth = 0, planHeight = 0;
	bStarTreeCoor(root, planWidth, planHeight);

	for(int i = 0; i < numNet; i++){
		int numDegree;
		int minX = std::numeric_limits<int>::max();
		int minY = std::numeric_limits<int>::max();
		int maxX = 0;
		int maxY = 0;
		getline(finNet, line);
		token.clear();
		token.str(line);
		token >> word;
		token >> numDegree;
		net* tmp = new net();
		for(int j = 0; j < numDegree; j++){
			getline(finNet, line);
			token.clear();
			token.str(line);
			token >> word;

			std::map<std::string, block*>::iterator blockIter = allBlock.find(word);
			std::map<std::string, std::pair<int, int> >::iterator terminalIter = allTerminal.find(word);
			if(blockIter != allBlock.end()){
				tmp->blocksInNet.push_back(word);
			} else {
				minX = std::min(minX, terminalIter->second.first);
				minY = std::min(minY, terminalIter->second.second);
				maxX = std::max(maxX, terminalIter->second.first);
				maxY = std::max(maxY, terminalIter->second.second);
			}
		}
		allNet.push_back(tmp);
		tmp->bottomX = minX;
		tmp->bottomY = minY;
		tmp->topX = maxX;
		tmp->topY = maxY;
	}

	/*
	-----------------------------------------------SA Setup-----------------------------------------------
	 */
	int penaltyWeight = 10;
	int numPertubations = 10000;
	long long totalArea = 0;
	long long totalWireLength = 0;
	for(int i = 0; i < numPertubations; i++){
		Perturb(vBlocks, numBlock, &root);
		bStarTreeCoor(root, planWidth, planHeight);
		totalArea += (planWidth * planHeight);		
		int min_X, min_Y, max_X, max_Y;
		for(int j = 0; j < allNet.size(); j++){
			net* tmpNet = allNet[j];
			min_X = tmpNet->bottomX;
			min_Y = tmpNet->bottomY;
			max_X = tmpNet->topX;
			max_Y = tmpNet->topY;
			for(int k = 0; k < tmpNet->blocksInNet.size(); k++){
				block* tmpBlock = allBlock.find(tmpNet->blocksInNet[k])->second;
				int x = tmpBlock->xCenter;
				int y = tmpBlock->yCenter;
				min_X = std::min(min_X, x);
				min_Y = std::min(min_Y, y);
				max_X = std::max(max_X, x);
				max_Y = std::max(max_Y, y);
			}
			totalWireLength += (max_X - min_X + max_Y - min_Y);
		}
	}
	double avgArea = totalArea / static_cast<double>(numPertubations);
	double avgWireLength = totalWireLength / static_cast<double>(numPertubations);
	/////////////////////////////////Cost Function/////////////////////////////////
	double penalty = Penalty(alpha, planWidth, planHeight, outlineWidth, outlineHeight);
	int area = planWidth * planHeight;
	int min_X, min_Y, max_X, max_Y;
	int wireLength = 0;
	for(int j = 0; j < allNet.size(); j++){
		net* tmpNet = allNet[j];
		min_X = tmpNet->bottomX;
		min_Y = tmpNet->bottomY;
		max_X = tmpNet->topX;
		max_Y = tmpNet->topY;
		for(int k = 0; k < tmpNet->blocksInNet.size(); k++){
			block* tmpBlock = allBlock.find(tmpNet->blocksInNet[k])->second;
			int x = tmpBlock->xCenter;
			int y = tmpBlock->yCenter;
			min_X = std::min(min_X, x);
			min_Y = std::min(min_Y, y);
			max_X = std::max(max_X, x);
			max_Y = std::max(max_Y, y);
		}
		wireLength += (max_X - min_X + max_Y - min_Y);
	}
	double lastCost = alpha * area / avgArea + (1 - alpha) * wireLength / avgWireLength + penaltyWeight * penalty / avgArea;
	/////////////////////////////////Cost Function/////////////////////////////////


	double totalTopHillCost = 0.0;
	double avgTopHillCost = 0;
	for(int i = 0; i < numPertubations; i++){
		Perturb(vBlocks, numBlock, &root);
		bStarTreeCoor(root, planWidth, planHeight);
		/////////////////////////////////Cost Function/////////////////////////////////
		penalty = Penalty(alpha, planWidth, planHeight, outlineWidth, outlineHeight);
		area = planWidth * planHeight;
		wireLength = 0;
		for(int j = 0; j < allNet.size(); j++){
			net* tmpNet = allNet[j];
			min_X = tmpNet->bottomX;
			min_Y = tmpNet->bottomY;
			max_X = tmpNet->topX;
			max_Y = tmpNet->topY;
			for(int k = 0; k < tmpNet->blocksInNet.size(); k++){
				block* tmpBlock = allBlock.find(tmpNet->blocksInNet[k])->second;
				int x = tmpBlock->xCenter;
				int y = tmpBlock->yCenter;
				min_X = std::min(min_X, x);
				min_Y = std::min(min_Y, y);
				max_X = std::max(max_X, x);
				max_Y = std::max(max_Y, y);
			}
			wireLength += (max_X - min_X + max_Y - min_Y);
		}
		double cost = alpha * area / avgArea + (1 - alpha) * wireLength / avgWireLength + penaltyWeight * penalty / avgArea;
		/////////////////////////////////Cost Function/////////////////////////////////
		double costDelta = cost - lastCost;
		if(costDelta > 0){
			totalTopHillCost += cost;
		}
	}
	avgTopHillCost = totalTopHillCost / numPertubations;

	/*
	-----------------------------------------------SA Start-----------------------------------------------
	 */
	double init_p = 0.99;
	double init_temperature = avgTopHillCost / -1 * std::log(init_p);
	numPertubations = numBlock * numBlock * 3;
	
	// double r = 0.85; //SA
	// double frozonTemperature = init_temperature / 1000;
	int c = 100;
	int k = 7;
	double frozonTemperature = init_temperature / 10000; 
	int frozonWorse = numPertubations * 0.95;
	
	int newPlanWidth, newPlanHeight;
	std::vector<block*> newBlocks;
	std::vector<net*> newNet;
	block* newRoot = TreeCopy(newBlocks, newNet, vBlocks, allNet);
	bStarTreeCoor(newRoot, newPlanWidth, newPlanHeight);
	/////////////////////////////////Cost Function/////////////////////////////////
	penalty = Penalty(alpha, newPlanWidth, newPlanHeight, outlineWidth, outlineHeight);
	area = newPlanWidth * newPlanHeight;
	wireLength = 0;
	for(int j = 0; j < newNet.size(); j++){
		net* tmpNet = newNet[j];
		min_X = tmpNet->bottomX;
		min_Y = tmpNet->bottomY;
		max_X = tmpNet->topX;
		max_Y = tmpNet->topY;
		for(int k = 0; k < tmpNet->blocksInNet.size(); k++){
			block* tmpBlock = allBlock.find(tmpNet->blocksInNet[k])->second;
			int x = tmpBlock->xCenter;
			int y = tmpBlock->yCenter;
			min_X = std::min(min_X, x);
			min_Y = std::min(min_Y, y);
			max_X = std::max(max_X, x);
			max_Y = std::max(max_Y, y);
		}
		wireLength += (max_X - min_X + max_Y - min_Y);
	}
	double bestCost = alpha * area / avgArea + (1 - alpha) * wireLength / avgWireLength + penaltyWeight * penalty / avgArea;
	/////////////////////////////////Cost Function/////////////////////////////////

	while(planWidth > outlineWidth || planHeight > outlineHeight){
		int copiedPlanWidth, copiedPlanHeight;
		lastCost = bestCost;
		double temperature = init_temperature;
		float adaptiveAlpha = alpha;
		int num_iterations = 1;
		int num_worse = 0;

		while(num_worse < frozonWorse && temperature > frozonTemperature){
			double totalCostDiff = 0.0;
			int numFeasibleSol = 0;
			num_worse = 0;
			for(int i = 0; i < numPertubations; i++){
				std::map<std::string, block*> copiedAllBlock;
				std::vector<block*> copiedBlocks;
				std::vector<net*> copiedNet;
				block* copiedRoot = TreeCopy(copiedBlocks, copiedNet, newBlocks, newNet);
				for(int i = 0; i < copiedBlocks.size(); i++){
					copiedAllBlock.insert(std::pair<std::string, block*>(copiedBlocks[i]->blockName, copiedBlocks[i]));
				}
				Perturb(copiedBlocks, numBlock, &copiedRoot);
				bStarTreeCoor(copiedRoot, copiedPlanWidth, copiedPlanHeight);
				bool isFeasible = false;
				if(copiedPlanWidth <= outlineWidth && copiedPlanHeight <= outlineHeight){
					isFeasible = true;
					++numFeasibleSol;
				}
				/////////////////////////////////Cost Function/////////////////////////////////
				penalty = Penalty(alpha, copiedPlanWidth, copiedPlanHeight, outlineWidth, outlineHeight);
				area = copiedPlanWidth * copiedPlanHeight;
				wireLength = 0;
				for(int j = 0; j < copiedNet.size(); j++){
					net* tmpNet = copiedNet[j];
					min_X = tmpNet->bottomX;
					min_Y = tmpNet->bottomY;
					max_X = tmpNet->topX;
					max_Y = tmpNet->topY;
					for(int k = 0; k < tmpNet->blocksInNet.size(); k++){
						block* tmpBlock = copiedAllBlock.find(tmpNet->blocksInNet[k])->second;
						int x = tmpBlock->xCenter;
						int y = tmpBlock->yCenter;
						min_X = std::min(min_X, x);
						min_Y = std::min(min_Y, y);
						max_X = std::max(max_X, x);
						max_Y = std::max(max_Y, y);
					}
					wireLength += (max_X - min_X + max_Y - min_Y);
				}
				double copiedCost = alpha * area / avgArea + (1 - alpha) * wireLength / avgWireLength + penaltyWeight * penalty / avgArea;
				/////////////////////////////////Cost Function/////////////////////////////////
				double costDiff = copiedCost - lastCost;
				totalCostDiff += costDiff;
				if(costDiff < 0){
					for(int i = 0; i < newBlocks.size(); i++){
						delete newBlocks[i];
					}
					for(int i = 0; i < newNet.size(); i++){
						delete newNet[i];
					}
					newRoot = copiedRoot;
					newBlocks = copiedBlocks;
					newNet = copiedNet;
					lastCost = copiedCost;

					if(copiedCost < bestCost && isFeasible){
						for(int i = 0; i < vBlocks.size(); i++){
							delete vBlocks[i];
						}
						for(int i = 0; i < allNet.size(); i++){
							delete allNet[i];
						}
						vBlocks.clear();
						allNet.clear();
						root = TreeCopy(vBlocks, allNet, copiedBlocks, copiedNet);
						for(int i = 0; i < vBlocks.size(); i++){
							allBlock.insert(std::pair<std::string, block*>(vBlocks[i]->blockName, vBlocks[i]));
						}
						bestCost = copiedCost;
					}
				} else {
					++num_worse;
					double p = std::exp(-1 * costDiff / temperature);
					if(rand() / static_cast<double>(RAND_MAX) < p){
						for(int i = 0; i < newBlocks.size(); i++){
							delete newBlocks[i];
						}
						for(int i = 0; i < newNet.size(); i++){
							delete newNet[i];
						}
						newRoot = copiedRoot;
						newBlocks = copiedBlocks;
						newNet = copiedNet;
						lastCost = copiedCost;
					} else {
						for(int i = 0; i < copiedBlocks.size(); i++){
							delete copiedBlocks[i];
						}
						for(int i = 0; i < copiedNet.size(); i++){
							delete copiedNet[i];
						}
					}
				}
			}

			++num_iterations;
			double avgCostDiff = totalCostDiff / numPertubations;
			if(num_iterations >= 2 && num_iterations <= k){
				temperature = init_temperature * avgCostDiff / num_iterations / c;
			} else {
				temperature = init_temperature * avgCostDiff / num_iterations;
			}
			// temperature *= r;
		}
		bStarTreeCoor(root, planWidth, planHeight);
		penaltyWeight += 50;
	    double run_time = (clock() - begin_time) / static_cast<double>(CLOCKS_PER_SEC);
	    if(run_time > 240) break;
	}
	allBlock.clear();
	for(int i = 0; i < vBlocks.size(); i++){
		allBlock.insert(std::pair<std::string, block*>(vBlocks[i]->blockName, vBlocks[i]));
	}
	/*
	-----------------------------------------------SA Finished-----------------------------------------------
	 */
	double run_time = (clock() - begin_time) / static_cast<double>(CLOCKS_PER_SEC);
	cout << "Runtime: " << run_time << endl;
	/////////////////////////////////Cost Function/////////////////////////////////
	bStarTreeCoor(root, planWidth, planHeight);
	area = planWidth * planHeight;
	wireLength = 0;
	for(int j = 0; j < allNet.size(); j++){
		net* tmpNet = allNet[j];
		min_X = tmpNet->bottomX;
		min_Y = tmpNet->bottomY;
		max_X = tmpNet->topX;
		max_Y = tmpNet->topY;
		for(int k = 0; k < tmpNet->blocksInNet.size(); k++){
			block* tmpBlock = allBlock.find(tmpNet->blocksInNet[k])->second;
			int x = tmpBlock->xCenter;
			int y = tmpBlock->yCenter;
			// cout << tmpBlock->blockName << " " << x << " " << y << endl;
			min_X = std::min(min_X, x);
			min_Y = std::min(min_Y, y);
			max_X = std::max(max_X, x);
			max_Y = std::max(max_Y, y);
		}
		wireLength += (max_X - min_X + max_Y - min_Y);
	}
	double doubleAlpha = (double)alpha;
	double weightedArea = doubleAlpha * (double)area;
	double weightedHPWL = (1 - doubleAlpha) * (double)wireLength;
	double realCost = weightedArea + weightedHPWL;
	/////////////////////////////////Cost Function/////////////////////////////////
	/*
	-----------------------------------------------Output-----------------------------------------------
	 */
	std::ofstream fout(argv[4]);
	fout << realCost << endl << wireLength << endl << area << endl << planWidth << " " << planHeight << endl << run_time << endl;
	std::map<std::string, block*>::iterator iter = allBlock.begin();
	for(iter; iter != allBlock.end(); iter++){
		std::string name = iter->first;
		block* macro = iter->second;
		fout << name << " " << macro->xCoor << " " << macro->yCoor << " " << macro->xCoor + macro->width << " " << macro->yCoor + macro->height << endl;
	}

	return 0;
}

void bStarTreeCoor(block* root, int &planWidth, int &planHeight){
	block* current;
	std::stack<block*> unvisited;
	unvisited.push(root);
	root->xCoor = 0;
	std::list<std::pair<int, int> > contour;
	contour.push_back(std::pair<int, int>(0, 0));
	std::list<std::pair<int, int> >::iterator it = contour.begin();
	planWidth = 0;
	planHeight = 0;

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
		planWidth = std::max(planWidth, current->xCoor + current->width);

		while(it == contour.end() || it->first > current->xCoor) --it;
		std::list<std::pair<int, int> >::iterator tmpIt = it;
		int maxY = 0, currentRightX = current->xCoor + current->width;
		while(tmpIt != contour.end() && tmpIt->first < currentRightX){ // tmpIt->first = list x
			maxY = std::max((tmpIt++)->second, maxY);
		}

		tmpIt = it;

		if(tmpIt != contour.begin() && (--tmpIt)->second == maxY + current->height){
			tmpIt = contour.erase(++tmpIt);
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

		int tmpMax = 0;
		while(tmpIt != contour.end() && tmpIt->first <= currentRightX){
			it->second = tmpIt->second;
			tmpIt = contour.erase(tmpIt);
		}
		current->yCoor = maxY;
		current->updateCenter();	
		planHeight = std::max(planHeight, current->yCoor + current->height);
	}	
	return void();
}

void Perturb(std::vector<block*> blocks, int blockNum, block **root){
	int Op = rand() % 2;
	switch(Op){
		case 0 : {
			int a = rand() % blockNum;
			int b = rand() % blockNum;
			while(a == b){
				b = rand() % blockNum;
			}
			DeleteAndInsert(blocks, a, b, root);
			break;
		}
		case 1 : {
			int a = rand() % blockNum;
			int b = rand() % blockNum;
			while(a == b){
				b = rand() % blockNum;
			}
			SwapNode(blocks, a, b);
			break;
		}
		default:
			break;
	}
}

void SwapNode(std::vector<block*> blocks, int id_a, int id_b){
	block *blockA = blocks[id_a], *blockB = blocks[id_b];
	std::string tmpName;
	tmpName = blockA->blockName;
	blockA->blockName = blockB->blockName;
	blockB->blockName = tmpName;
	blockA->width = blockA->width ^ blockB->width;
	blockB->width = blockA->width ^ blockB->width;
	blockA->width = blockA->width ^ blockB->width;
	blockA->height = blockA->height ^ blockB->height;
	blockB->height = blockA->height ^ blockB->height;
	blockA->height = blockA->height ^ blockB->height;
}

void DeleteAndInsert(std::vector<block*> blocks, int id_a, int id_b, block **root){
	DeleteNode(blocks, id_a, root);
	Insert(blocks, id_a, id_b);
}

void DeleteNode(std::vector<block*> blocks, int id_a, block **root){
	block *node = blocks[id_a];
	block *node_child = NULL;

	if(node->leftChild != NULL && node->rightChild != NULL){
		node_child = node->leftChild;
		block *currentNode = node_child;
		while(currentNode->leftChild != NULL && currentNode->rightChild != NULL){
			currentNode = currentNode->leftChild;
		}
		if(currentNode->rightChild != NULL){
			currentNode->leftChild = currentNode->rightChild;
			currentNode->rightChild = NULL;
		}

		while(currentNode->blockName != node->blockName){
			block *currentParent = currentNode->parent;
			block *currentCibling = currentParent->rightChild;
			currentNode->rightChild = currentCibling;
			currentCibling->parent = currentNode;
			currentNode = currentParent;
		}
	} else if (node->leftChild != NULL) {
		node_child = node->leftChild;
	} else if (node->rightChild != NULL) {
		node_child = node->rightChild;
	}

	if(node_child != NULL){
		node_child->parent = node->parent;
	}

	if(node->parent == NULL){
		*root = node_child;
	} else {
		if(node->parent->leftChild == node){
			node->parent->leftChild = node_child;
		} else {
			node->parent->rightChild = node_child;
		}
	}

	node->parent = NULL;
	node->leftChild = NULL;
	node->rightChild = NULL;
}

void Insert(std::vector<block*> blocks, int id_a, int id_b){
	block *fromNode = blocks[id_a];
	block *toNode = blocks[id_b];
	bool HI = 0;

	fromNode->parent = toNode;
	if (rand() % 2 == 0){
		fromNode->leftChild = toNode->leftChild;
		if(fromNode->leftChild != NULL){
			fromNode->leftChild->parent = fromNode;
		}
		toNode->leftChild = fromNode;
	} else {
		fromNode->rightChild = toNode->rightChild;
		if(fromNode->rightChild != NULL){
			fromNode->rightChild->parent = fromNode;
		}
		toNode->rightChild = fromNode;
	}
}

double Penalty(float alpha, int planWidth, int planHeight, int outlineWidth, int outlineHeight){
	
	double penalty = 0.0;
	if(planWidth > outlineWidth || planHeight > outlineHeight){
		if(planWidth > outlineWidth && planHeight > outlineHeight){
			penalty += (planWidth * planHeight - outlineHeight * outlineWidth);
		} else if (planWidth > outlineWidth) {
			penalty += ((planWidth - outlineWidth) * planHeight);
		} else {
			penalty += ((planHeight - outlineHeight) * planWidth);
		}
		int widthDiff = planWidth - outlineWidth, heightDiff = planHeight - outlineHeight;
		penalty += (widthDiff * widthDiff + heightDiff * heightDiff);
	}

	return penalty;
}

block* TreeCopy(std::vector<block*> &copy, std::vector<net*> &nets_copy, std::vector<block*> origin, std::vector<net*> nets_origin){
	block* root;
	std::string pName, lName, rName;
	std::map<std::string, block*> blocksIndex;
	for(int i = 0; i < origin.size(); i++){
		block* sample = origin[i];
		block* clone = new block(sample->blockName, sample->width, sample->height);
		clone -> xCoor       =  sample -> xCoor;
		clone -> yCoor       =  sample -> yCoor;
		clone -> xCenter     =  sample -> xCenter;
		clone -> yCenter     =  sample -> yCenter;
		if (clone->xCoor == 0 && clone->yCoor == 0) {
			root = clone;
		}
		copy.push_back(clone);
		blocksIndex.insert(std::pair<std::string, block*>(clone->blockName, clone));
	}
	for(int i = 0; i < origin.size(); i++){
		block* tmp = blocksIndex.find(origin[i]->blockName)->second;
		if(origin[i]->parent == NULL) tmp->parent = NULL;
		else tmp->parent = blocksIndex.find(origin[i]->parent->blockName)->second;
		if(origin[i]->leftChild == NULL) tmp->leftChild = NULL;
		else tmp->leftChild = blocksIndex.find(origin[i]->leftChild->blockName)->second;
		if(origin[i]->rightChild == NULL) tmp->rightChild = NULL;
		else tmp->rightChild = blocksIndex.find(origin[i]->rightChild->blockName)->second;
	}
	for(int i = 0; i < nets_origin.size(); i++){
		net* sample = nets_origin[i];
		net* clone = new net();
		clone -> bottomX = sample -> bottomX;
		clone -> bottomY = sample -> bottomY;
		clone -> topX    = sample -> topX;
		clone -> topY    = sample -> topY;
		for(int j = 0; j < sample->blocksInNet.size(); j++){
			std::string ghost = sample->blocksInNet[j];
			clone->blocksInNet.push_back(ghost);
		}
		nets_copy.push_back(clone);
	}
	return root;
}
