#pragma once
//QT
#include <QFileDialog>
#include <QtConcurrent/QtConcurrent>
#include <QStandardItemModel>
#include <QtWidgets/QMainWindow>
//
#include "ui_MusicDecryptor.h"
#include "DecryptThread.h"

using namespace std;

class MusicDecryptor : public QMainWindow
{
	Q_OBJECT

public:
	MusicDecryptor(QWidget* parent = nullptr);
	~MusicDecryptor();
	vector<filesystem::path> _files;
	bool is_connected = false;
	//ui量
	std::unique_ptr<QStandardItemModel> _listview_files_model;
	//内部量
	std::unique_ptr<DecryptThreadManager> _decrypt_thread_manager;

public slots:
	void addlog(QString Info, Qt::GlobalColor color = Qt::black, QString End = "\n");
	void startProcess();
	void selectDir();
	void selectSaveDir();
	void selectFiles();
	void selectKgDb();

signals:
	void signalAddlog(QString Info, QString End = "\n");

private:
	Ui::MusicDecryptorClass ui;
};