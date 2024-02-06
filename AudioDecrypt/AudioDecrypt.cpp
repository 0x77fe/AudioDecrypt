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
	connect(this->ui.pushButton_startProcess, &QPushButton::clicked, this, &AudioDecrypt::StartProcess);
	connect(this->ui.pushButton_selsctDir, &QPushButton::clicked, this, &AudioDecrypt::SelectDir);
	connect(this->ui.pushButton_choseSaveDir, &QPushButton::clicked, this, &AudioDecrypt::SelectSaveDir);
	//
	connect(this, &AudioDecrypt::SignalAddlog, this, &AudioDecrypt::Addlog);
	connect(&this->_Factory, &DecryptFactory::sigSingleFinished, this, &AudioDecrypt::Finished);
	connect(&this->_Factory, &DecryptFactory::sigAllFinished, this, &AudioDecrypt::Finished);
}

AudioDecrypt::~AudioDecrypt()
{}

void AudioDecrypt::StartProcess()
{
	if (this->_model.rowCount() == 0)
	{
		Addlog("未搜索文件或未找到文件");
		return;
	}

	filesystem::path savedir;
	if (this->ui.checkBox_useSaveDir->checkState()) 
	{ 
		savedir = this->ui.lineEdit_saveDir->text().toLocal8Bit().constData();
	}
	else { savedir = filesystem::path(); }

	this->Addlog("线程开始");
	this->_Factory.SetOutputPath(savedir);
	this->_Factory.SetThreadAmount(ui.spinBox_threadCount->value());
	this->_Factory.EnDel(ui.checkBox_delete->checkState());
	this->_Factory.EnWite163Key(ui.checkBox_write163Key->checkState());

	this->_Factory.Add(_files);

	this->_files.clear();
	this->_model.clear();
	this->ui.listView_originalFiles->setModel(&_model);
}

void AudioDecrypt::SelectDir()
{
	//重置
	this->_files.clear();
	this->_model.clear();

	//选择文件夹
	auto folderPath = QString(QFileDialog::getExistingDirectory(this, tr("选择文件夹"), "/", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks));
	if (folderPath.isEmpty()) { return; }
	string Dir = folderPath.toStdString();
	this->ui.lineEdit_originalDir->setText(folderPath);

	//遍历搜索
	_files = SerchFiles
	(
		this->ui.lineEdit_originalDir->text().toLocal8Bit().constData(),
		vector<string>{".ncm", ".kgm", ".kgma"}
	);

	//注入模型
	for (const auto& file : this->_files) 
	{
		auto item = new QStandardItem(QString::fromStdWString(file.filename().wstring()));
		this->_model.appendRow(item);
	}

	//设置ui'
	this->ui.listView_originalFiles->setModel(&_model);
	this->Addlog(QString::fromUtf8("找到了 ")+QString::fromUtf8(to_string(_files.size()) + " 个文件"));
}

void AudioDecrypt::SelectSaveDir()
{
	auto folderPath = QFileDialog::getExistingDirectory(this, tr("选择文件夹"), "/", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if (folderPath.isEmpty()) { return; }
	string Dir = folderPath.toStdString();
	this->ui.lineEdit_saveDir->setText(folderPath);
}

void AudioDecrypt::Addlog(QString Info, QString End, bool Time)
{
	//获取系统时间文本
	char buffer[20]{ '\0' };
	QString time_(" ");
	if (Time)
	{
		time_t now = time(nullptr);
		char buffer[80];
		strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localtime(&now));
		time_+="["+QString::fromLocal8Bit(buffer)+"]";
	}

	auto text = this->ui.textEdit_log->toPlainText() + time_ + Info + End;

	//设置ui
	this->ui.textEdit_log->setText(text);

	//自动滑动到文本框底部
	QTextCursor textCursor = this->ui.textEdit_log->textCursor();
	textCursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
	this->ui.textEdit_log->setTextCursor(textCursor);
}

void AudioDecrypt::Finished(QString info)
{
	Addlog(info);
}
