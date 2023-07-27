#include "stdafx.h"
#include "AudioDecrypt.h"
#include "all.h"
#include <future>
#include <QtConcurrent/QtConcurrent>
#include <exception>



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
	if (th) { return; }

	bool skip = ui.checkBox_skip->checkState();
	bool use = ui.checkBox_useSaveDir->checkState();
	bool del = ui.checkBox_delete->checkState();
	auto files = &_files;

	filesystem::path savedir;
	if (use) { savedir = ui.lineEdit_saveDir->text().toLocal8Bit().constData(); }
	else { savedir = filesystem::path(); }

	th = !th;
	Addlog("线程开始");

	auto action = [this, skip, use, del, savedir, files]()
	{
		bool r = true;
		for (const auto& file : *files)
		{
			try {
				emit SignalAddlog("正在处理 " + QString::fromWCharArray(file.filename().wstring().c_str()));
				DecodeFactory(file, savedir, skip);
				emit SignalAddlog(" ... [完成] ", "\n", false);
				if (del) { remove(file); }
			}
			catch (exception e)
			{
				emit SignalAddlog("失败:" + QString::fromLocal8Bit(e.what()));
				r = false;
			};
		};
		return r;
	};

	auto end = [=]()
	{
		try
		{
			if (!_thWatcher.result())
			{
				Addlog("处理文件时发生了错误,请检查.");
			};
		}
		catch (exception e)
		{
			Addlog("线程发生致命错误,无法继续 " + QString::fromLocal8Bit(e.what()));
		};
		_files = vector<filesystem::path>();
		_model.clear();
		Addlog("全部处理完成 线程结束");
		th = !th;
	};

	auto fut = QtConcurrent::run(action);
	connect(&_thWatcher, &QFutureWatcher<bool>::finished, end);
	_thWatcher.setFuture(fut);

}

void AudioDecrypt::SelectDir()
{
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