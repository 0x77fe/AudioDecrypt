#pragma once
#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif
#include <QtWidgets/QMainWindow>
#include "ui_AudioDecrypt.h"

using namespace std;

class AudioDecrypt : public QMainWindow
{
	Q_OBJECT

public:
	AudioDecrypt(QWidget* parent = nullptr);
	~AudioDecrypt();
	//
	void StartProcess();
	void SelectDir();
	void addlog(const string& info);
	void SelectSaveDir();

	//
	QStandardItemModel model;
	vector< filesystem::path> files;
	bool th = false;

private:
	Ui::AudioDecryptClass ui;
};