// Samurai2019.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//

#include <iostream>
#include <string.h>
#include "GameInfo.h"
#include "AI.h"

using namespace std;

int main(int argc, char** argv) {

	GameInfo nowInfo;

	//for (int i = 1; i < argc; i++) {
	//	if (strcmp("-dump", argv[i]) == 0 && i + 1 < argc) {
	//		if (strcmp("samurai", argv[i + 1]) == 0) {
	//			nowInfo.dumpSamurai = true;
	//		}
	//		else if (strcmp("dog", argv[i + 1]) == 0) {
	//			nowInfo.dumpDog = true;
	//		}
	//		else if (strcmp("both", argv[i + 1]) == 0) {
	//			nowInfo.dumpSamurai = true;
	//			nowInfo.dumpDog = true;
	//		}
	//		i++;
	//	}
	//}

	while (!cin.eof()) {
		nowInfo.inputAndUpdate();
		int plan = nowInfo.id < 2 ? planSamurai(nowInfo) : planDog(nowInfo);
		nowInfo.PastPositionUpdate();
		cout << plan << endl;
	}
}

// プログラムの実行: Ctrl + F5 または [デバッグ] > [デバッグなしで開始] メニュー
// プログラムのデバッグ: F5 または [デバッグ] > [デバッグの開始] メニュー

// 作業を開始するためのヒント: 
//    1. ソリューション エクスプローラー ウィンドウを使用してファイルを追加/管理します 
//   2. チーム エクスプローラー ウィンドウを使用してソース管理に接続します
//   3. 出力ウィンドウを使用して、ビルド出力とその他のメッセージを表示します
//   4. エラー一覧ウィンドウを使用してエラーを表示します
//   5. [プロジェクト] > [新しい項目の追加] と移動して新しいコード ファイルを作成するか、[プロジェクト] > [既存の項目の追加] と移動して既存のコード ファイルをプロジェクトに追加します
//   6. 後ほどこのプロジェクトを再び開く場合、[ファイル] > [開く] > [プロジェクト] と移動して .sln ファイルを選択します
