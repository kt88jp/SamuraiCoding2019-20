#include <vector>
#include <utility>
#include <queue>
#include <assert.h>
#include <random>

#include "ToolsForAI.h"

using namespace std;

random_device rnd;
mt19937 mt(1);
uniform_int_distribution<> rand24(0, 24);
uniform_int_distribution<> rand8(0, 8);

//穴を考慮しない最短経路探索。高速でbestmoveとdistを算出できる
//侍の場合は近い2方向に他のエージェントがいた場合は動かない
//犬の場合は、近い3方向に他のエージェントがいた場合は、遠くなったとしてもとにかく動く（サインの誤発信を防ぐ）
DistInfo findSortestDistAndBestMoveByManhattan(pair<int, int> from, pair<int, int> to, GameInfo& info) {
	DistInfo res;

	static int dx[8] = { 0, 1, 1, 1, 0, -1, -1, -1 };
	static int dy[8] = { 1, 1, 0, -1, -1, -1, 0, 1 };

	bool isDog;
	if (info.id == 2 || info.id == 3) {
		isDog = true;
	}
	else {
		isDog = false;
	}

	if (!isDog) {

		res.dist = calcManhattanDist(from, to);

		int best = (int)1e9;

		for (int i = 0; i < 8; i += 2) {
			pair<int, int> next = make_pair(from.first + dx[i], from.second + dy[i]);

			if (next.first < 0 || next.first >= info.size || next.second < 0 || next.second >= info.size) {
				continue;
			}

			if (info.nowf.f[next.first][next.second].cellStatus == 0 ||
				info.nowf.f[next.first][next.second].cellStatus == 1 ||
				info.nowf.f[next.first][next.second].cellStatus == 2 ||
				info.nowf.f[next.first][next.second].cellStatus == 3) {
				continue;
			}

			if (calcManhattanDist(from, to) < calcManhattanDist(next, to)) {
				continue;
			}

			if (best > calcManhattanDist(next, to) + (info.nowf.f[next.first][next.second].cellStatus == 5 ? 1 : 0)) {
				best = calcManhattanDist(next, to);
				res.bestMove = convertMoveIntoOutput(from, next);
			}

		}
		res.targetPosition = to;
		return res;
	}
	else {
		res.dist = calcShortestDistForDog(from, to);

		int best = (int)1e9;

		for (int i = 0; i < 8; i++) {
			pair<int, int> next = make_pair(from.first + dx[i], from.second + dy[i]);

			if (next.first < 0 || next.first >= info.size || next.second < 0 || next.second >= info.size) {
				continue;
			}

			if (info.nowf.f[next.first][next.second].cellStatus == 0 ||
				info.nowf.f[next.first][next.second].cellStatus == 1 ||
				info.nowf.f[next.first][next.second].cellStatus == 2 ||
				info.nowf.f[next.first][next.second].cellStatus == 3 ||
				info.nowf.f[next.first][next.second].cellStatus == 5) {
				continue;
			}

			if (best > calcShortestDistForDog(next, to)) {
				best = calcShortestDistForDog(next, to);
				res.bestMove = convertMoveIntoOutput(from, next);
			}

		}

		res.targetPosition = to;
		return res;
	}
}

//穴を考慮した厳密な最短経路探索。最悪計算量はO(size*size)
DistInfo findShortestDistAndBestMoveByBFS(pair<int, int> from, pair<int, int> to, GameInfo& info) {
	DistInfo res;

	static int dx[8] = { 0, 1, 1, 1, 0, -1, -1, -1 };
	static int dy[8] = { 1, 1, 0, -1, -1, -1, 0, 1 };

	bool isDog;
	if (info.id == 2 || info.id == 3) {
		isDog = true;
	}
	else {
		isDog = false;
	}

	bool canReach = false;//そのマスに到達可能か（場合によっては犬は任意のマスに到達できない可能性がある。）

	//最短経路復元のために、各cellでどのような移動が行われていたのかなどについて記録しておくためのもの
	vector<vector<CellDataForBFS>> fielddata(info.size, vector<CellDataForBFS>(info.size));


	queue<CellDataForBFS> q;

	CellDataForBFS start;
	start.now = from;
	start.dist = 0;
	start.visited = true;

	q.push(start);

	bool fin = false;


	while (!q.empty()) {
		CellDataForBFS nowfdata = q.front();
		q.pop();

		fielddata[nowfdata.now.first][nowfdata.now.second] = nowfdata;

		if (nowfdata.delay) {//穴がある場所を通行する場合は埋める作業があるため、1ターン遅延させる。
			nowfdata.delay = false;
			q.push(nowfdata);
			continue;
		}

		if (nowfdata.now == to) {
			canReach = true;
			break;
		}

		for (int i = 0; i < 8; isDog ? i++ : i += 2) {
			CellDataForBFS nextfdata;
			nextfdata.now.first = nowfdata.now.first + dx[i];
			nextfdata.now.second = nowfdata.now.second + dy[i];

			if (nextfdata.now.first < 0 || nextfdata.now.first >= info.size || nextfdata.now.second < 0 || nextfdata.now.second >= info.size) {
				continue;
			}

			if (fielddata[nextfdata.now.first][nextfdata.now.second].visited) {
				continue;
			}

			nextfdata.from = nowfdata.now;

			nextfdata.dist = nowfdata.dist + 1;
			if (info.nowf.f[nextfdata.now.first][nextfdata.now.second].cellStatus == 5) {
				if (isDog) {
					continue;
				}
				nextfdata.dist++;//穴だったらコストは+2
				nextfdata.isHole = true;
				nextfdata.delay = true;
			}

			if (nextfdata.now != to || (info.id == 2 || info.id == 3)) {
				if (info.nowf.f[nextfdata.now.first][nextfdata.now.second].cellStatus == 0 ||
					info.nowf.f[nextfdata.now.first][nextfdata.now.second].cellStatus == 1 ||
					info.nowf.f[nextfdata.now.first][nextfdata.now.second].cellStatus == 2 ||
					info.nowf.f[nextfdata.now.first][nextfdata.now.second].cellStatus == 3) {
					continue;//進む先にエージェントがいたら通行不能なためcontinue
				}
			}
			else {
				int opponentDogId;
				if (info.id == 0) {
					opponentDogId = 3;
				}
				else {
					opponentDogId = 2;
				}
				if (info.nowf.f[nextfdata.now.first][nextfdata.now.second].cellStatus == 0 ||
					info.nowf.f[nextfdata.now.first][nextfdata.now.second].cellStatus == 1 ||
					info.nowf.f[nextfdata.now.first][nextfdata.now.second].cellStatus == opponentDogId) {
					continue;//進む先にエージェントがいたら通行不能なためcontinue
				}

			}


			nextfdata.visited = true;

			fielddata[nextfdata.now.first][nextfdata.now.second] = nextfdata;

			if (nextfdata.now == to) {
				canReach = true;
				fin = true;
				break;
			}


			//すべての条件をクリアして、nextのcellは新しく進入可能であることが分かった

			q.push(nextfdata);

		}

		if (fin) {
			break;
		}

	}

	//ここから最短経路の復元→bestMoveを決める

	if ((info.id == 2 || info.id == 3) && !canReach) {
		//犬は到達不可能なこともある。その場合はbestmove,distを-1として返す
		res.bestMove = -1;
		return res;
	}

	pair<int, int> now = fielddata[to.first][to.second].now;
	res.dist = fielddata[to.first][to.second].dist;
	res.targetPosition = now;

	pair<int, int> prev = make_pair(-1, -1);

	if (now == make_pair(-1, -1)) {
		res.bestMove = randomAct(info);
		return res;
	}

	while (now.first != start.now.first || now.second != start.now.second) {

		int x = now.first;
		int y = now.second;



		prev = fielddata[x][y].from;

		if (prev == start.now) {
			res.bestMove = convertMoveIntoOutput(prev, now);
			break;
		}

		now = prev;
	}


	return res;
}

int convertMoveIntoOutput(pair<int, int> from, pair<int, int> to) {
	if (from == to) {
		return -1;
	}

	pair<int, int> dxdy = make_pair(to.first - from.first, to.second - from.second);

	if (dxdy == make_pair(0, 1)) {
		return 0;
	}
	else if (dxdy == make_pair(-1, 1)) {
		return 1;
	}
	else if (dxdy == make_pair(-1, 0)) {
		return 2;
	}
	else if (dxdy == make_pair(-1, -1)) {
		return 3;
	}
	else if (dxdy == make_pair(0, -1)) {
		return 4;
	}
	else if (dxdy == make_pair(1, -1)) {
		return 5;
	}
	else if (dxdy == make_pair(1, 0)) {
		return 6;
	}
	else if (dxdy == make_pair(1, 1)) {
		return 7;
	}
	else {
		return -1;
	}


	//if (from.first - to.first == 0) {
	//	if (from.second - to.second == 1) {
	//		return 4;
	//	}
	//	else {
	//		return 0;
	//	}
	//}
	//else if (from.second - to.second == 0) {
	//	if (from.first - to.first == 1) {
	//		return 2;
	//	}
	//	else {
	//		return 6;
	//	}
	//}
	//else {
	//	if (from.first - to.first == -1) {
	//		if (from.second - to.second == 1) {
	//			return 3;
	//		}
	//		else {
	//			return 1;
	//		}
	//	}
	//	else {
	//		if (from.second - to.second == 1) {
	//			return 5;
	//		}
	//		else {
	//			return 7;
	//		}
	//	}
	//}
}

pair<int, int> convertMoveIntoDxDy(int move) {
	pair<int, int> res;
	switch (move) {
	case 0:
		res = make_pair(0, 1);
		break;
	case 1:
		res = make_pair(-1, 1);
		break;
	case 2:
		res = make_pair(-1, 0);
		break;
	case 3:
		res = make_pair(-1, -1);
		break;
	case 4:
		res = make_pair(0, -1);
		break;
	case 5:
		res = make_pair(1, -1);
		break;
	case 6:
		res = make_pair(1, 0);
		break;
	case 7:
		res = make_pair(1, 1);
		break;
	}

	return res;
}

int calcManhattanDist(pair<int, int> from, pair<int, int> to){
	return abs(from.first - to.first) + abs(from.second - to.second);
}

int calcShortestDistForDog(pair<int, int> from, pair<int, int> to) {
	int shorterEdge = min(abs(from.first - to.first), abs(from.second - to.second));
	int longerEdge = max(abs(from.first - to.first), abs(from.second - to.second));
	return shorterEdge + (longerEdge - shorterEdge);
}

int convertMoveIntoOppositeMove(int move) {
	switch (move) {
	case 0:
		return 4;
		break;
	case 1:
		return 5;
		break;
	case 2:
		return 6;
		break;
	case 3:
		return 7;
	case 4:
		return 0;
		break;
	case 5:
		return 1;
		break;
	case 6:
		return 2;
		break;
	case 7:
		return 3;
		break;
	}
}

int randomAct(GameInfo &info) {

	int cnt = 1000;
	
	if (info.id == 0 || info.id == 1) {
		while (cnt-- > 0) {
			int move = rand24(mt);
			move--;
			if (move == -1) {
				return move;
			}
			if (canAct(info, move)) {
				return move;
			}
		}
	}
	else {
		while (cnt-- > 0) {
			int move = rand8(mt);
			move--;
			if (move == -1) {
				return move;
			}
			if (canAct(info, move)) {
				return move;
			}
		}
	}

	return -1;
}

int randomMove(GameInfo &info) {

	int cnt = 1000;
	
	while (cnt-- > 0) {
		int move = rand8(mt);
		move--;
		if (move == -1) {
			return move;
		}
		if (canAct(info, move)) {
			return move;
		}
	}

	return -1;
}

bool canAct(GameInfo& info, int move) {
	int tmpmove = move;
	if (tmpmove >= 15) {
		tmpmove -= 16;
	}
	else if (move >= 8) {
		tmpmove -= 8;
	}
	pair<int, int> myPosition = info.positions[info.id];
	pair<int, int> dxdy = convertMoveIntoDxDy(tmpmove);
	pair<int, int> moveTo = make_pair(myPosition.first + dxdy.first, myPosition.second + dxdy.second);

	if (!isInsideOfTheField(info, moveTo)) {
		return false;
	}

	if (info.id == 0 || info.id == 1) {
		if (move % 2 != 0) {
			return false;
		}
		if (move < 15) {
			if (info.nowf.f[moveTo.first][moveTo.second].cellStatus == 4) {
				return true;
			}
			else {
				return false;
			}
		}
		else {
			if (info.nowf.f[moveTo.first][moveTo.second].cellStatus == 5) {
				return true;
			}
			else {
				return false;
			}
		}
	}
	else {
		if (move >= 8) {
			return false;
		}
		if (info.nowf.f[moveTo.first][moveTo.second].cellStatus == 4) {
			return true;
		}
		else {
			return false;
		}
	}
}

bool isInsideOfTheField(GameInfo &info, pair<int,int> position) {
	if (position.first < 0 || position.first >= info.size || position.second < 0 || position.second >= info.size) {
		return false;
	}
	else {
		return true;
	}
}
