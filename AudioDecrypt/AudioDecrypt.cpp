#include "stdafx.h"
#include <future>
#include <QtConcurrent/QtConcurrent>
#include <exception>

#include "AudioDecrypt.h"

AudioDecrypt::AudioDecrypt(QWidget* parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	//
	QObject::connect(ui.pushButton_startProcess, &QPushButton::clicked, this, &AudioDecrypt::StartProcess);
	QObject::connect(ui.pushButton_selsctDir, &QPushButton::clicked, this, &AudioDecrypt::SelectDir);
	QObject::connect(ui.pushButton_choseSaveDir, &QPushButton::clicked, this, &AudioDecrypt::SelectSaveDir);
	//
	QObject::connect(this, &AudioDecrypt::SignalAddlog, this, &AudioDecrypt::Addlog);

}

AudioDecrypt::~AudioDecrypt()
{}

void AudioDecrypt::StartProcess()
{
	if (_model.rowCount() == 0)
	{
		Addlog("未搜索文件或未找到文件");
		return;
	}

	filesystem::path savedir;
	if (ui.checkBox_useSaveDir->checkState()) { savedir = ui.lineEdit_saveDir->text().toLocal8Bit().constData(); }
	else { savedir = filesystem::path(); }

	Addlog("线程开始");
	_Factory.SetOutputPath(savedir);
	_Factory.SetThreadAmount(ui.spinBox_threadCount->value());
	_Factory.EnDel(ui.checkBox_delete->checkState());
	_Factory.EnWite163Key(ui.checkBox_write163Key->checkState());

	_Factory.Add(_files);
	_files.clear();
	_model.clear();
	ui.listView_originalFiles->setModel(&_model);
}

void AudioDecrypt::SelectDir()
{
	//线程运行中
	_files = vector<filesystem::path>();
	_model.clear();
	auto folderPath = QString(QFileDialog::getExistingDirectory(this, tr("选择文件夹"), "/", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks));
	if (folderPath.isEmpty()) { return; }
	string Dir = folderPath.toStdString();
	ui.lineEdit_originalDir->setText(folderPath);

	_files = SerchFiles(ui.lineEdit_originalDir->text().toLocal8Bit().constData(), vector<string>{".ncm", ".kgm", ".kgma"});
	for (const auto& file : _files) {
		auto item = new QStandardItem(QString::fromWCharArray(file.filename().wstring().c_str()));
		_model.appendRow(item);
	}

	ui.listView_originalFiles->setModel(&_model);
	Addlog(QString("找到了 ").append(to_string(_files.size()).c_str()) + " 个文件");
}

void AudioDecrypt::SelectSaveDir()
{
	auto folderPath = QString(QFileDialog::getExistingDirectory(this, tr("选择文件夹"), "/", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks));
	if (folderPath.isEmpty()) { return; }
	string Dir = folderPath.toStdString();
	ui.lineEdit_saveDir->setText(folderPath);
}

void AudioDecrypt::Addlog(QString Info, QString End, bool Time)
{
	char buffer[20]{ '\0' };
	QString time_(" ");
	if (Time)
	{
		time_t now = time(nullptr);
		char buffer[80];
		strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
		time_.append("[").append(buffer).append("]");
	}

	auto text = ui.textEdit_log->toPlainText() + time_ + Info + End;
	ui.textEdit_log->setText(text);
	QTextCursor textCursor = ui.textEdit_log->textCursor();
	textCursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
	ui.textEdit_log->setTextCursor(textCursor);
}

void AudioDecrypt::Log(QString info)
{
	Addlog(info);
}
