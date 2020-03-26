#pragma once
#include <iostream>
#include <set>
#include <map>
#include <vector>

#include "Field.h"

//#define DEBUG

using namespace std;

class GameInfo
{
public:

	int dumpId = 0;//ダンプするID番号（コンパイルごとに変更する必要あり）

	//今のフィールド
	Field nowf;

	//サインで伝えられた金のリスト（侍用）
	set<pair<int, int>> toldGoldList;

	set<pair<int, int>> notCheckedCellList;

	int id;
	int size;
	int step;
	int maxstep;
	int holesNum;
	set<pair<int, int>> holesList;
	int knowngnum;
	map<pair<int, int>, int> knownGoldList;
	int sensedgnum;
	map<pair<int, int>, int> sensedGoldList;

	map<pair<int, int>, int> newSensedGoldList;

	set<pair<pair<int, int>, pair<int, int>>> signCandidates;

	pair<int,int> positions[4];

	int pastPlan[4];
	int pastAction[4];

	int score[2];

	pair<int, int> pastPositions[4];

	int remainingGold;
	int remainingTime;


	void inputAndUpdate() {

		static int dx[8] = { 0, 1, 1, 1, 0, -1, -1, -1 };
		static int dy[8] = { 1, 1, 0, -1, -1, -1, 0, 1 };

		cin >> id >> size >> step >> maxstep;


		if (step == 0) {
			nowf.init(size);

			for (int i = 0; i < size; i++) {
				for (int j = 0; j < size; j++) {
					notCheckedCellList.insert(make_pair(i, j));
				}
			}

		}


		for (int i = 0; i < size; i++) {
			for (int j = 0; j < size; j++) {
				nowf.f[i][j].initBeforeUpdate();
			}
		}

		//以下、情報を受け取ってnowfに反映、リストを更新
		
		cin >> holesNum;
		set<pair<int, int>> newHolesList;
		for (int i = 0; i < holesNum; i++) {
			pair<int, int> tmp;
			cin >> tmp.first >> tmp.second;
			newHolesList.insert(tmp);
			nowf.f[tmp.first][tmp.second].cellStatus = 5;
			toldGoldList.erase(tmp);//穴になったということは発掘済みなので削除
			nowf.f[tmp.first][tmp.second].isTold = false;
			nowf.f[tmp.first][tmp.second].infoForDog = 1;//穴が開いたということは、何もないことが確定した
			nowf.f[tmp.first][tmp.second].isGoldHere = false;
			nowf.f[tmp.first][tmp.second].isGoldRevealed = false;
			nowf.f[tmp.first][tmp.second].checked = true;
		
			sensedGoldList.erase(tmp);//金があると考えられていた場所に穴が開いたら、もうそこに金はない

			notCheckedCellList.erase(tmp);
			toldGoldList.erase(tmp);
		}

		holesList = newHolesList;

		map<pair<int, int>, int> newKnownGoldList;
		
		cin >> knowngnum;
		for (int i = 0; i < knowngnum; i++) {
			pair<int, int> tmp1;
			int tmp2;
			cin >> tmp1.first >> tmp1.second;
			cin >> tmp2;
			newKnownGoldList[tmp1] = tmp2;
			nowf.f[tmp1.first][tmp1.second].isGoldHere = true;
			nowf.f[tmp1.first][tmp1.second].isGoldRevealed = true;
			nowf.f[tmp1.first][tmp1.second].goldAmount = tmp2;
			notCheckedCellList.erase(tmp1);
			toldGoldList.erase(tmp1);
			sensedGoldList.erase(tmp1);
		}
		knownGoldList = newKnownGoldList;

		

		cin >> sensedgnum;
		newSensedGoldList.clear();
		for (int i = 0; i < sensedgnum; i++) {
			pair<int, int> tmp1;
			int tmp2;
			cin >> tmp1.first >> tmp1.second;
			cin >> tmp2;
			sensedGoldList[tmp1] = tmp2;
			newSensedGoldList[tmp1] = tmp2;
			nowf.f[tmp1.first][tmp1.second].isGoldHere = true;
			nowf.f[tmp1.first][tmp1.second].infoForDog = 2;
			nowf.f[tmp1.first][tmp1.second].goldAmount = tmp2;

		}



		for (int i = 0; i < 4; i++) {
			pair<int, int> tmp;
			cin >> tmp.first >> tmp.second;
			positions[i] = tmp;
			if (i == 2 || i == 3) {
				if (!nowf.f[tmp.first][tmp.second].isGoldHere) {
					nowf.f[tmp.first][tmp.second].infoForDog = 1;//犬がいるにもかかわらず鳴いていないということは、そこには何もないことが確定した
					nowf.f[tmp.first][tmp.second].isGoldHere = false;
					nowf.f[tmp.first][tmp.second].isGoldRevealed = false;
				}
			}
			nowf.f[tmp.first][tmp.second].cellStatus = i;
		}

		for (int i = 0; i < 4; i++) {
			cin >> pastPlan[i];
		}

		for (int i = 0; i < 4; i++) {
			cin >> pastAction[i];
		}

		for (int i = 0; i < 2; i++) {
			cin >> score[i];
		}

		cin >> remainingGold >> remainingTime;


	}

	void PastPositionUpdate() {
		for (int i = 0; i < 4; i++) {
			pastPositions[i] = positions[i];
		}
	}

	void dump() {
#ifdef DEBUG
		cerr << "========" << endl;

		cerr << "STEP:" << step << endl;

		cerr << "ID:" << id << endl;

		cerr << "CellStatus" << endl;
		nowf.dumpCellStatus();

		cerr << "----" << endl;

		cerr << "GoldAmount" << endl;
		for (auto& i : knownGoldList) {
			cerr << i.first.first << "-" << i.first.second << "," << i.second << endl;
		}
		nowf.dumpGoldAmount();

		cerr << "----" << endl;

		cerr << "InfoForDog" << endl;
		nowf.dumpInfoForDog();

		cerr << "----" << endl;

		cerr << "IsGoldHere" << endl;
		nowf.dumpIsGoldHere();

		cerr << "----" << endl;

		cerr << "newSensedGoldList" << endl;
		for (auto& i : newSensedGoldList) {
			cerr << i.first.first << "-" << i.first.second << " " << i.second << endl;
		}

		cerr << "----" << endl;

		cerr << "SensedGoldList" << endl;
		for (auto& i : sensedGoldList) {
			cerr << i.first.first << "-" << i.first.second << "," << i.second << endl;
		}

		cerr << "----" << endl;

		cerr << "NotcheckedCellList " << notCheckedCellList.size() << endl;
		for (auto& i : notCheckedCellList) {
			cerr << i.first << "-" << i.second << endl;
		}

		cerr << "----" << endl;
		cerr << "ToldGoldList" << endl;
		for (auto& i : toldGoldList) {
			cerr << i.first << "-" << i.second << endl;
		}

		cerr << "PastPlan" << endl;
		for (int i = 0; i < 4; i++) {
			cerr << pastPlan[i] << " ";
		}

		cerr << endl;

		cerr << "PastAction" << endl;
		for (int i = 0; i < 4; i++) {
			cerr << pastAction[i] << endl;
		}

		cerr << "PastPositions" << endl;
		for (int i = 0; i < 4; i++) {
			cerr << pastPositions[i].first << "-" << pastPositions[i].second << endl;
		}

		cerr << "=======" << endl;
#endif
	}

};

