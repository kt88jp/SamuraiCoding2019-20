#include "GameInfo.h"
#include "ToolsForAI.h"
#include "AI.h"

enum SamuraiStrategy {
	FOLLOWDOG,
	EXPLORE,
	MOVE_EXPLORE,
	AIM_S,
	WAIT_S
};

enum DogStrategy {
	CRAWL,
	//SENDSIGN,
	BARK,
	AIM_D,
	WAITFORSIGN,
	WAITONGOLD,
	OBSTRUCT,
};

SamuraiStrategy sStrategy = SamuraiStrategy::AIM_S;
DogStrategy dStrategy = DogStrategy::CRAWL;

bool finishCrawling = false;

bool dogPrepareForSign = false;

bool sentSign = false;
pair<int, int> sentSignPosition = make_pair(-1, -1);

bool MoveOver = false;//犬がラストの１マスから退いたかどうか



class Candidate {
public:
	pair<int, int> position = make_pair(-1, -1);
	int dist = -1;
	int bestMove = -1;
	int gAmount = 0;
};


int planSamurai(GameInfo &info) {

	if (info.dumpId == info.id) info.dump();

	static int dx[8] = { 0, 1, 1, 1, 0, -1, -1, -1 };
	static int dy[8] = { 1, 1, 0, -1, -1, -1, 0, 1 };

	pair<int, int> myPosition = info.positions[info.id];
	pair<int, int> dogPosition = info.positions[info.id + 2];
	pair<int, int> opponentDogPosition;
	if (info.id == 0) {
		opponentDogPosition = info.positions[3];
	}
	else {
		opponentDogPosition = info.positions[2];
	}

	//金の残量と見えている金の総量が一致したらfinishcrawling
	int knownTotalAmount = 0;
	for (auto i : info.knownGoldList) {
		knownTotalAmount += i.second;
	}
	if (knownTotalAmount == info.remainingGold) {
		finishCrawling = true;
	}


	//クロールが終了したかどうか判定
	if (info.notCheckedCellList.size() == 0) {
		finishCrawling = true;
	}

	//犬の周囲8方向はチェック済みとしておく
	if (!finishCrawling) {
		info.notCheckedCellList.erase(dogPosition);
		info.nowf.f[dogPosition.first][dogPosition.second].checked = true;


		for (int i = 0; i < 8; i++) {
			pair<int, int> next = make_pair(dogPosition.first + dx[i], dogPosition.second + dy[i]);

			if (next.first < 0 || next.first >= info.size || next.second < 0 || next.second >= info.size) {
				continue;
			}

			//info.nowf.f[next.first][next.second].checked = true;

			//info.notCheckedCellList.erase(next);
		}
	}

	//相手の犬の下をチェック済みに
	info.notCheckedCellList.erase(opponentDogPosition);
	info.nowf.f[opponentDogPosition.first][opponentDogPosition.second].checked;

	

	//サインの受け取り
	if (dogPrepareForSign) {
		//二回連続とまっていたらサインは受け取らない（何らかの理由でサインを出せなかった）
		//相手との競合があった場合もサインは受け取らない
		if (info.pastAction[info.id + 2] != -1 && info.positions[info.id + 2] != info.pastPositions[info.id + 2]) {
			pair<int, int> dxdy = make_pair(info.positions[info.id + 2].first - info.pastPositions[info.id + 2].first,
				info.positions[info.id + 2].second - info.pastPositions[info.id + 2].second);

			pair<int, int> goldDxdy = make_pair(dxdy.first * -1, dxdy.second * -1);

			pair<int, int> tmp = make_pair(info.pastPositions[info.id + 2].first + goldDxdy.first, info.pastPositions[info.id + 2].second + goldDxdy.second);

			if (isInsideOfTheField(info, tmp)) {
				info.toldGoldList.insert(tmp);
			}

			dogPrepareForSign = false;
		}
		else {
			dogPrepareForSign = false;
		}
	}
	else {
		//犬がサインの準備（止まる）をしているかを確認→次回サインを受け取るかどうかを決める
		if (info.pastPlan[info.id + 2] == -1 && info.positions[info.id + 2] == info.pastPositions[info.id + 2]) {
			dogPrepareForSign = true;
		}
	}

	bool CanDogMove = false;//犬が移動できる状態にあるか
	for (int i = 0; i < 8; i++) {
		pair<int, int> next;
		next.first = dogPosition.first + dx[i];
		next.second = dogPosition.second + dy[i];
		if (!isInsideOfTheField(info, next)) {
			continue;
		}
		if (info.nowf.f[next.first][next.second].cellStatus != 5) {
			CanDogMove = true;
			break;
		}
	}

	set<pair<int,int>> exploreCandidates;

	//作戦変更
	if (!CanDogMove) {
		//とりあえず犬を助けに行く
		sStrategy = SamuraiStrategy::FOLLOWDOG;
	}
	else if (info.knowngnum == 0 && info.toldGoldList.size() == 0) {
		for (int i = 0; i < 8; i += 2) {
			pair<int, int> next = make_pair(myPosition.first + dy[i], myPosition.second + dx[i]);
			if (!isInsideOfTheField(info, next)) {
				continue;
			}

			if (info.nowf.f[next.first][next.second].cellStatus != 4) {
				continue;
			}

			if (info.nowf.f[next.first][next.second].checked) {
				continue;
			}

			exploreCandidates.insert(next);
		}

		if (exploreCandidates.size() > 0) {
			sStrategy = SamuraiStrategy::EXPLORE;
		}
		else {
			sStrategy = SamuraiStrategy::MOVE_EXPLORE;
		}
	}
	else {
		bool blocked = true;
		for (auto& i : info.knownGoldList) {
			DistInfo tmp = findShortestDistAndBestMoveByBFS(myPosition, i.first, info);
			if (tmp.dist != -1) {
				blocked = false;
			}
		}

		//金がラスト一個だったらブロック扱いにはしない
		if (info.knowngnum == 1) {
			int amount = 0;
			for (auto i : info.knownGoldList) {
				amount = i.second;
			}
			if (amount == info.remainingGold) {
				blocked = false;
			}
		}

		for (auto& i : info.toldGoldList) {
			DistInfo tmp = findShortestDistAndBestMoveByBFS(myPosition, i, info);
			if (tmp.dist != -1) {
				blocked = false;
			}
		}



		if (blocked) {
			for (int i = 0; i < 8; i += 2) {
				pair<int, int> next = make_pair(myPosition.first + dy[i], myPosition.second + dx[i]);
				if (!isInsideOfTheField(info, next)) {
					continue;
				}

				if (info.nowf.f[next.first][next.second].cellStatus != 4) {
					continue;
				}

				if (info.nowf.f[next.first][next.second].checked) {
					continue;
				}

				exploreCandidates.insert(next);
			}

			if (exploreCandidates.size() > 0) {
				sStrategy = SamuraiStrategy::EXPLORE;
			}
			else {
				sStrategy = SamuraiStrategy::MOVE_EXPLORE;
			}
		}
		else {
			sStrategy = SamuraiStrategy::AIM_S;
		}
	}

	if (info.dumpId == info.id) dumpStrategy(info);

	//もし前回競合があった場合はランダムに動いて回避する
	if (info.step > 0 && info.pastAction[info.id] != info.pastPlan[info.id] && info.pastPositions[info.id] == info.positions[info.id]) {
		return randomMove(info);
	}

	//作戦に応じて行動を選択
	if (sStrategy == SamuraiStrategy::FOLLOWDOG) {
		DistInfo res;

		pair<int, int> distination;//犬の周囲八マスの中で一番近いところを目的地とする
		int dist = 1e9;
		for (int i = 0; i < 8; i++) {
			pair<int, int> tmp;
			tmp.first = dogPosition.first + dx[i];
			tmp.second = dogPosition.second + dy[i];
			if (!isInsideOfTheField(info, tmp)) {
				continue;
			}
			int tmpdist = findSortestDistAndBestMoveByManhattan(myPosition, tmp, info).dist;
			if (tmpdist < dist) {
				distination = tmp;
			}
		}
		
		res = findShortestDistAndBestMoveByBFS(myPosition, distination, info);

		pair<int, int> dxdy = convertMoveIntoDxDy(res.bestMove);

		if (info.nowf.f[myPosition.first + dxdy.first][myPosition.second + dxdy.second].cellStatus == 5) {
			res.bestMove += 16;
		}

		return res.bestMove;

	}
	else if (sStrategy == SamuraiStrategy::EXPLORE) {
		for (auto& i : exploreCandidates) {
			int res = convertMoveIntoOutput(myPosition, i);
			info.nowf.f[i.first][i.second].checked = true;
			info.notCheckedCellList.erase(i);
			res += 8;
			return res;
		}
	} else if (sStrategy == SamuraiStrategy::MOVE_EXPLORE) {
		Candidate candidate;
		candidate.dist = 1e9;
		for (auto& i : info.notCheckedCellList) {
			DistInfo tmp = findShortestDistAndBestMoveByBFS(myPosition, i, info);
			if (i == myPosition) continue;
			if (tmp.dist != -1 && tmp.dist < candidate.dist) {
				candidate.position = i;
				candidate.dist = tmp.dist;
				candidate.bestMove = tmp.bestMove;
			}
		}
		int res = candidate.bestMove;
		pair<int, int> dydx = convertMoveIntoDxDy(res);
		pair<int, int> to = make_pair(myPosition.first + dydx.first, myPosition.second + dydx.second);

		if (info.nowf.f[to.first][to.second].cellStatus == 5) {
			res += 16;
		}

		return res;
	}
	else {
		DistInfo res;

		pair<int, int> opponentPosition;
		if (info.id == 0) {
			opponentPosition = info.positions[1];
		}
		else {
			opponentPosition = info.positions[0];
		}
		pair<int, int> targetPosition;

		//class Candidate {
		//public:
		//	pair<int, int> position = make_pair(-1, -1);
		//	int dist = -1;
		//	int gAmount = 0;
		//};

		
		//狙うべき候補を探す
		//pair<pair<int,int>, int> candidate = make_pair(make_pair(-1,-1), -1);
		Candidate candidate;
		candidate.dist = 1e9;
		int diff = 1e9;
		if (info.knowngnum > 0) {

			for (auto &i : info.knownGoldList) {
				if (!(info.nowf.f[i.first.first][i.first.second].cellStatus == 4 || info.nowf.f[i.first.first][i.first.second].cellStatus == info.id + 2)) {
					continue;
				}
				int mydist = findShortestDistAndBestMoveByBFS(myPosition, i.first, info).dist;
				int opponentdist = findShortestDistAndBestMoveByBFS(opponentPosition, i.first, info).dist;
				if(opponentdist)
				//int tmpdiff = mydist - opponentdist;
				if (mydist <= opponentdist + 6 && mydist < candidate.dist) {
					candidate.position = i.first;
					candidate.gAmount = i.second;
					candidate.dist = mydist;
				}
			}

			if (info.knowngnum == 1 && (candidate.position == myPosition || candidate.position == make_pair(-1, -1))) {
				//狙える唯一の金が自分の下にあったらとりあえず退く
				return randomMove(info);
			}
		}

		//if (candidate.first == make_pair(-1, -1)) {
			for (auto& i : info.toldGoldList) {
				int mydist = findShortestDistAndBestMoveByBFS(myPosition, i, info).dist;
				if (candidate.dist > mydist && mydist != -1) {
					candidate.position = i;
					candidate.dist = mydist;
				}
			}

			if (candidate.position == myPosition || candidate.position == make_pair(-1, -1)) {
				//狙える唯一の金が自分の下にあったらとりあえず退く
				return randomMove(info);
			}
		//}

		if (candidate.position == make_pair(-1, -1)) {
			return randomMove(info);
		}

		//bestMoveを見つける
		//if (info.holesNum > (info.size * info.size) / 3) {
			res = findShortestDistAndBestMoveByBFS(myPosition, candidate.position, info);
		//}
		//else {
		//	res = findSortestDistAndBestMoveByManhattan(myPosition, candidate.first, info);
		//}

		pair<int, int> dxdy = convertMoveIntoDxDy(res.bestMove);

		if (info.nowf.f[myPosition.first + dxdy.first][myPosition.second + dxdy.second].cellStatus == 5) {
			res.bestMove += 16;
		}

		if (res.dist == 1) {
			res.bestMove += 8;
		}

		return res.bestMove;
	}


	return -1;
}

int planDog(GameInfo &info) {

	if(info.dumpId == info.id)info.dump();

	static int dx[8] = { 0, 1, 1, 1, 0, -1, -1, -1 };
	static int dy[8] = { 1, 1, 0, -1, -1, -1, 0, 1 };

	pair<int, int> myPosition = info.positions[info.id];
	pair<int, int> mySamuraiPosition = info.positions[info.id - 2];
	pair<int, int> opponentSamuraiPosition;
	if (info.id == 2) {
		opponentSamuraiPosition = info.positions[1];
	}
	else {
		opponentSamuraiPosition = info.positions[0];

	}
	pair<int, int> opponentDogPosition;
	if (info.id == 2) {
		opponentDogPosition = info.positions[3];
	}
	else {
		opponentDogPosition = info.positions[2];
	}

	//犬の周囲8方向はチェック済みとしておく
	if (!finishCrawling) {
		info.notCheckedCellList.erase(myPosition);
		if (info.nowf.f[myPosition.first][myPosition.second].isGoldHere) {
			info.nowf.f[myPosition.first][myPosition.second].infoForDog = 2;
		}
		else {
			info.nowf.f[myPosition.first][myPosition.second].infoForDog = 1;
		}
		for (int i = 0; i < 8; i++) {
			pair<int, int> next = make_pair(myPosition.first + dx[i], myPosition.second + dy[i]);

			if (next.first < 0 || next.first >= info.size || next.second < 0 || next.second >= info.size) {
				continue;
			}

			if (info.nowf.f[next.first][next.second].isGoldHere) {
				info.nowf.f[next.first][next.second].infoForDog = 2;
			}
			else {
				info.nowf.f[next.first][next.second].infoForDog = 1;
			}

			info.notCheckedCellList.erase(next);
		}
	}

	//相手の犬の上は確定
	info.notCheckedCellList.erase(opponentDogPosition);
	if (info.nowf.f[opponentDogPosition.first][opponentDogPosition.second].isGoldHere) {
		info.nowf.f[opponentDogPosition.first][opponentDogPosition.second].infoForDog = 2;
	}
	else {
		info.nowf.f[opponentDogPosition.first][opponentDogPosition.second].infoForDog = 1;
	}


	//前回サインをうまく送れたかどうか
	if (sentSign) {
		sentSign = false;
		info.signCandidates.clear();
		info.nowf.f[sentSignPosition.first][sentSignPosition.second].isTold = true;
		sentSignPosition = make_pair(-1, -1);
	}

	set<pair<int, int>> barkCandidates;


	//set<pair<pair<int,int>, pair<int,int>>> signCandidates;

	//金の残量と見えている金の総量が一致したらfinishcrawling
	int knownTotalAmount = 0;
	for (auto &i : info.knownGoldList) {
		knownTotalAmount += i.second;
	}
	if (knownTotalAmount == info.remainingGold) {
		finishCrawling = true;
		for (auto& i : info.notCheckedCellList) {
			info.nowf.f[i.first][i.second].infoForDog = 1;
		}
		info.notCheckedCellList.clear();
	}

	if (info.notCheckedCellList.size() == 0) {
		finishCrawling = true;
	}

	if (dStrategy != DogStrategy::OBSTRUCT && !dogPrepareForSign && (myPosition == info.pastPositions[info.id])) {
		//もし前回競合があった場合はランダムに動いて回避する
		if (info.step > 0 && info.pastAction[info.id] != info.pastPlan[info.id]) {
			return randomMove(info);
		}
	}


	//サイン送信部分
	if (dogPrepareForSign) {
		//もし、前回サインのために一時停止してたならサインを送る
		Candidate candidate;

		for (auto& i : info.signCandidates) {
			if (info.nowf.f[i.second.first][i.second.second].cellStatus != 4) {
				continue;
			}
			candidate.position = i.second;
			dogPrepareForSign = false;
			sentSign = true;
			info.nowf.f[i.first.first][i.first.second].isTold = true;
			break;
		}

		if (candidate.position == make_pair(-1, -1)) {
			dogPrepareForSign = false;
			sentSign = false;
			return -1;
		}

		candidate.bestMove = convertMoveIntoOutput(myPosition, candidate.position);

		dogPrepareForSign = false;

		//サインを送った場所を記録しておく（次回の行動で無事にサインが送れたかを確認するため）
		sentSignPosition = candidate.position;

		return candidate.bestMove;
	}

	if (info.sensedgnum > 0) {
		bool canBark = false;
		int mydist = 0;
		int opponentDist = 0;
		for (auto &i : info.newSensedGoldList) {
			mydist = findSortestDistAndBestMoveByManhattan(mySamuraiPosition, i.first, info).dist;
			opponentDist = findSortestDistAndBestMoveByManhattan(opponentSamuraiPosition, i.first, info).dist;
			if (mydist < opponentDist) {
				canBark = true;
				barkCandidates.insert(i.first);
			}
			else {
				pair<int, int> next = i.first;
				pair<int, int> dxdy = make_pair(next.first - myPosition.first, next.second - myPosition.second);
				pair<int, int> opposite = make_pair(myPosition.first + (dxdy.first * -1), myPosition.second + (dxdy.second * -1));

				if (!isInsideOfTheField(info, opposite)) {
					continue;
				}
				if (info.nowf.f[opposite.first][opposite.second].cellStatus != 4) {
					continue;
				}

				if (info.nowf.f[opposite.first][opposite.second].infoForDog == 2) {
					continue;
				}

				if (info.nowf.f[next.first][next.second].isTold) {
					continue;
				}


				pair<pair<int,int>, pair<int,int>> sCandidate;
				sCandidate.first = i.first;
				sCandidate.second = opposite;
				info.signCandidates.insert(sCandidate);
			}
		}
		if (canBark) {
			dStrategy = DogStrategy::BARK;
		}
		else {
			if (static_cast<int>(info.signCandidates.size()) == 0) {
				return randomMove(info);
			}
			if (!dogPrepareForSign) {
				dStrategy = DogStrategy::WAITFORSIGN;
			}
		}
		
	}
	else {
		if (finishCrawling) {
			if (info.nowf.f[myPosition.first][myPosition.second].isGoldHere) {
				info.sensedGoldList.erase(myPosition);
			}

			if (!MoveOver) {
				if (info.nowf.f[myPosition.first][myPosition.second].isGoldHere && info.knowngnum == 1) {
					dStrategy = DogStrategy::WAITONGOLD;
				}
				else {
					if (info.knowngnum > 0) {
						dStrategy = DogStrategy::OBSTRUCT;
					}
					else {
						dStrategy = DogStrategy::AIM_D;
					}
				}
			}
			else {
				dStrategy = DogStrategy::OBSTRUCT;
			}
		}
		else {
			dStrategy = DogStrategy::CRAWL;
		}
	}

	if (MoveOver) {
		dStrategy = DogStrategy::AIM_D;
	}




	if(info.dumpId == info.id) dumpStrategy(info);

	if (dStrategy == DogStrategy::CRAWL) {

		Candidate candidate;
		candidate.dist = static_cast<int>(1e9);

		for (auto& i : info.notCheckedCellList) {
			if (info.nowf.f[i.first][i.second].cellStatus == 4) {
				DistInfo tmp = findShortestDistAndBestMoveByBFS(myPosition, i, info);

				if (tmp.dist == 0) {
					continue;
				}

				pair<int, int> dxdy = convertMoveIntoDxDy(tmp.bestMove);

				if (myPosition.first + dxdy.first < 0 || myPosition.first + dxdy.first >= info.size ||
					myPosition.second + dxdy.second < 0 || myPosition.second + dxdy.second >= info.size) {
					continue;
				}

				if (!canAct(info, tmp.bestMove)) {
					continue;
				}

				if (tmp.dist < candidate.dist) {
					candidate.position = i;
					candidate.bestMove = tmp.bestMove;
					candidate.dist = tmp.dist;
				}
			}
		}


		return candidate.bestMove;
	}
	else if(dStrategy == DogStrategy::WAITFORSIGN){
		dogPrepareForSign = true;
		return -1;
	}
	//else if (dStrategy == DogStrategy::SENDSIGN) {
	//	Candidate candidate;

	//	for (auto &i : info.signCandidates) {
	//		candidate.position = i.second;
	//		dogPrepareForSign = false;
	//		sentSign = true;
	//		info.nowf.f[i.first.first][i.first.second].isTold = true;
	//		break;
	//	}

	//	candidate.bestMove = convertMoveIntoOutput(myPosition, candidate.position);
	//	
	//	dogPrepareForSign = false;

	//	//サインを送った場所を記録しておく（次回の行動で無事にサインが送れたかを確認するため）
	//	sentSignPosition = candidate.position;

	//	return candidate.bestMove;

	//}
	else if (dStrategy == DogStrategy::BARK) {
		dogPrepareForSign = false;
		for (auto& i : barkCandidates) {
			return convertMoveIntoOutput(myPosition, i);
		}
	}
	else if (dStrategy == DogStrategy::AIM_D) {
		Candidate candidate;
		dogPrepareForSign = false;
		for (auto& i : info.sensedGoldList) {
			if (i.second > candidate.gAmount) {
				candidate.position = i.first;
				candidate.gAmount = i.second;
			}
		}

		return findShortestDistAndBestMoveByBFS(myPosition, candidate.position, info).bestMove;

	}
	else if (dStrategy == DogStrategy::WAITONGOLD) {
		//自分の侍が近づいてくるまで待つ
		int opponentSId;
		if (info.id == 2) {
			opponentSId = 1;
		}
		else {
			opponentSId = 0;
		}

		if (info.score[info.id - 2] >= info.score[opponentSId]) {
			return -1;
		}

		if (calcManhattanDist(mySamuraiPosition, myPosition) > 3) {
			return -1;
		}

		//自分の侍が近づいてきたら、避ける
		Candidate candidate;
		for (int i = 0; i < 8; i++) {
			pair<int, int> next = make_pair(myPosition.first + dx[i], myPosition.second + dy[i]);

			if (next.first < 0 || next.second >= info.size || next.second < 0 || next.second >= info.size) {
				continue;
			}

			if (info.nowf.f[next.first][next.second].cellStatus != 4) {
				continue;
			}

			if (candidate.dist < calcManhattanDist(mySamuraiPosition, next)) {
				candidate.position = next;
				candidate.dist = calcManhattanDist(mySamuraiPosition, next);
			}
		}
		
		MoveOver = true;
		return findSortestDistAndBestMoveByManhattan(myPosition, candidate.position, info).bestMove;
	}
	else if (dStrategy == DogStrategy::OBSTRUCT) {
		//相手のbestMoveを予測
		//相手が狙っていると思われる金を推測する（相手から一番近い金へ移動すると仮定する）
		Candidate opponentCandidate;
		opponentCandidate.dist = 1e9;
		for (auto& i : info.knownGoldList) {
			DistInfo tmp = findShortestDistAndBestMoveByBFS(opponentSamuraiPosition, i.first, info);
			if (tmp.dist != -1 && tmp.dist < opponentCandidate.dist) {
				opponentCandidate.bestMove = tmp.bestMove;
				opponentCandidate.dist = tmp.dist;
			}
		}

		pair<int, int> dydx = convertMoveIntoDxDy(opponentCandidate.bestMove);
		pair<int, int> opponentNextPos = make_pair(opponentSamuraiPosition.first + dydx.first, opponentSamuraiPosition.second + dydx.second);

		DistInfo res = findShortestDistAndBestMoveByBFS(myPosition, opponentNextPos, info);

		return res.bestMove;

	}
	else {
		return -1;
	}
}

void dumpStrategy(GameInfo& info) {
#ifdef DEBUG
	cerr << info.id << " Strategy: ";
	switch (dStrategy) {
	case DogStrategy::CRAWL:
		cerr << "CRAWL" << endl;
		break;
	case DogStrategy::WAITFORSIGN:
		cerr << "WAITFORSIGN" << endl;
		break;
	//case DogStrategy::SENDSIGN:
	//	cerr << "SENDSIGN" << endl;
	//	break;
	case DogStrategy::BARK:
		cerr << "BARK" << endl;
		break;
	case DogStrategy::AIM_D:
		cerr << "AIM" << endl;
		break;
	case DogStrategy::WAITONGOLD:
		cerr << "WAITONGOLD" << endl;
		break;
	}

	cerr << "---Other Status---" << endl;

	if (dogPrepareForSign) {
		cerr << "DogPrepareForSign ";
	}
	if (sentSign) {
		cerr << "sentSign " << sentSignPosition.first << "-" << sentSignPosition.second << " ";
	}

	if (finishCrawling) {
		cerr << "FinishCrawling ";
	}
	cerr << endl;
	cerr << "*****************************************" << endl;
#endif
}