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
	void SelectSaveDir();

	//
	QStandardItemModel _model;
	vector<filesystem::path> _files;
	bool th = false;
	QFutureWatcher<bool> _thWatcher;

public slots:
	void Addlog(QString Info, QString End = "\n", bool Time = true);
signals:
	void SignalAddlog(QString Info, QString End = "\n", bool Time = true);

private:
	Ui::AudioDecryptClass ui;
};