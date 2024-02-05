#pragma once

#include <QtWidgets/QMainWindow>

#include "ui_AudioDecrypt.h"
#include "Decryptor/DecryptFactory.h"

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
	DecryptFactory _Factory = DecryptFactory(8);

	//
	bool _isConnected = false;

public slots:
	void Addlog(QString Info, QString End = "\n", bool Time = true);
	void Log(QString info);
signals:
	void SignalAddlog(QString Info, QString End = "\n", bool Time = true);

private:
	Ui::AudioDecryptClass ui;
};