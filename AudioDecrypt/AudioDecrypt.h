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
	void SelectSaveDir();

	//
	QStandardItemModel _model;
	vector<filesystem::path> _files;
	QFutureWatcher<bool> _thWatcher;

	//
	bool isConnected = false;

public slots:
	void Addlog(QString Info, QString End = "\n", bool Time = true);
signals:
	void SignalAddlog(QString Info, QString End = "\n", bool Time = true);

private:
	Ui::AudioDecryptClass ui;
};