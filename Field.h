#pragma once
#include <vector>
#include <utility>
#include <queue>
#include <iostream>
#include <iomanip>

using namespace std;

class CellInfo {
public:
	//�S�������̏��
	int cellStatus = 4;
	//��4:�󔒁A0����3:�G�[�W�F���gID�A5:��
	int goldAmount = 0;//���̗�

	//�������̏��
	bool isGoldHere = false;
	bool isGoldRevealed = false;
	bool checked = false;//���̏ꏊ���m�F�ς݂�
	

	//�������̏��
	int infoForDog = 0;
	//��0:�s���A1:�󔒂��m��A2:�������邱�Ƃ��m��

	bool isTold = false;//���ɃT�C���œ`�������ǂ���

	void init() {
		cellStatus = 4;
		isGoldHere = false;
		isGoldRevealed = false;
		goldAmount = 0;
		infoForDog = 0;
		isTold = false;
	}

	void initBeforeUpdate() {
		cellStatus = 4;
		isGoldRevealed = false;
	}

	void dump() {
		cerr << "cellStatus " << cellStatus << endl;
		cerr << "goldAmount " << goldAmount << endl;
		cerr << "isGoldHere " << static_cast<int>(isGoldHere) << endl;
		cerr << "isGoldRevealed " << static_cast<int>(isGoldRevealed) << endl;
		cerr << "infoForDog " << infoForDog << endl;
		cerr << "isTold" << static_cast<int>(isTold) << endl;
	}

};


class Field
{
public:
	vector<vector<CellInfo>> f;

	void init(int size) {
		f.resize(size);
		for (int i = 0; i < size; i++) {
			f[i].resize(size);
		}

		for (int i = 0; i < size; i++) {
			for (int j = 0; j < size; j++) {
				f[i][j].init();
			}
		}
	}

	void dumpCellStatus() {
		//cerr << "CellStatus" << endl;
		for (int i = 0; i < f.size(); i++) {
			for (int j = 0; j < f.size(); j++) {
				cerr << f[i][j].cellStatus << " ";
			}
			cerr << endl;
		}
	}

	void dumpGoldAmount() {
		//cerr << "GoldAmount" << endl;
		for (int i = 0; i < f.size(); i++) {
			for (int j = 0; j < f.size(); j++) {
				cerr << setw(3) << f[i][j].goldAmount << " ";
			}
			cerr << endl;
		}
	}

	void dumpIsGoldHere() {
		//cerr << "IsGoldHere" << endl;
		for (int i = 0; i < f.size(); i++) {
			for (int j = 0; j < f.size(); j++) {
				cerr << static_cast<int>(f[i][j].isGoldHere) << " ";
			}
			cerr << endl;
		}
	}

	void dumpInfoForDog() {
		//cerr << "InfoForDog" << endl;
		for (int i = 0; i < f.size(); i++) {
			for (int j = 0; j < f.size(); j++) {
				cerr << f[i][j].infoForDog << " ";
			}
			cerr << endl;
		}
	}

	void dumpIsTold() {
		//cerr << "IsTold" << endl;
		for (int i = 0; i < f.size(); i++) {
			for (int j = 0; j < f.size(); j++) {
				cerr << static_cast<int>(f[i][j].isTold) << " ";
			}
			cerr << endl;
		}
	}
	
};

