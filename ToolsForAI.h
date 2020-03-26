#pragma once
#include <utility>
#include "GameInfo.h"

using namespace std;

class DistInfo {
public:
	int bestMove = -1;
	int dist = -1;
	pair<int, int> targetPosition = make_pair(-1, -1);
};

class CellDataForBFS {
public:
	pair<int, int> now = make_pair(-1, -1);
	pair<int, int> from = make_pair(-1, -1);
	int dir = -1;
	int dist = -1;
	bool visited = false;
	bool isHole = false;
	bool delay = false;
};

DistInfo findSortestDistAndBestMoveByManhattan(pair<int, int> from, pair<int, int> to, GameInfo& info);
DistInfo findShortestDistAndBestMoveByBFS(pair<int, int> from, pair<int, int> to, GameInfo& info);
int convertMoveIntoOutput(pair<int, int> from, pair<int, int> to);
pair<int, int> convertMoveIntoDxDy(int move);
int calcManhattanDist(pair<int, int> from, pair<int, int> to);
int calcShortestDistForDog(pair<int, int> from, pair<int, int> to);
int convertMoveIntoOppositeMove(int move);
int randomAct(GameInfo &info);
int randomMove(GameInfo& info);
bool canAct(GameInfo& info, int move);
bool isInsideOfTheField(GameInfo& info, pair<int, int> position);