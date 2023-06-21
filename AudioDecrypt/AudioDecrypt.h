#pragma once
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
	//void addlog(QString Info, QString End = "\n", bool Time = true);
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