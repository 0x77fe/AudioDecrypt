#include "stdafx.h"
#include "AudioDecrypt.h"
#include "all.h"
#include <future>
#include <QtConcurrent/QtConcurrent>

AudioDecrypt::AudioDecrypt(QWidget* parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	//
	QObject::connect(ui.pushButton_startProcess, &QPushButton::clicked, this, &AudioDecrypt::StartProcess);
	QObject::connect(ui.pushButton_selsctDir, &QPushButton::clicked, this, &AudioDecrypt::SelectDir);
	QObject::connect(ui.pushButton_choseSaveDir, &QPushButton::clicked, this, &AudioDecrypt::SelectSaveDir);
}

AudioDecrypt::~AudioDecrypt()
{}

void AudioDecrypt::StartProcess()
{
	if (model.rowCount() == 0)
	{
		addlog("未搜索文件或未找到文件");
		return;
	}
	if (th) { return; }
	bool skip = ui.checkBox_skip->checkState();
	bool use = ui.checkBox_useSaveDir->checkState();
	bool del = ui.checkBox_delete->checkState();
	filesystem::path savedir;
	if (use) { savedir = ui.lineEdit_saveDir->text().toLocal8Bit().constData(); }
	else { savedir = filesystem::path(); }
	th = !th;
	addlog("线程开始");
	auto action = [=]()
		{
			for (const filesystem::path& file : files)
			{
				try {
					DecodeFactory(file, savedir, skip);
					addlog("完成处理 " + file.filename().string());
					if (del) { remove(file); }
				}
				catch (exception e) { addlog("处理 " + file.string() + "时出错 \n" + e.what()); };
			};
			files = vector< filesystem::path>();
			model.clear();
			addlog("全部处理完成 线程结束");
			th = !th;
		};
	QtConcurrent::run(action);
}

void AudioDecrypt::SelectDir()
{
	auto folderPath = new QString(QFileDialog::getExistingDirectory(this, tr("选择文件夹"), "/", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks));
	if (folderPath->isEmpty()) { delete folderPath; return; }
	string Dir = folderPath->toStdString();
	ui.lineEdit_originalDir->setText(*folderPath);
	delete folderPath;

	files = SerchFiles(ui.lineEdit_originalDir->text().toLocal8Bit().constData(), ".ncm");
	for (auto var : SerchFiles(ui.lineEdit_originalDir->text().toLocal8Bit().constData(), ".kgm")) {
		files.push_back(var);
	};
	for (auto var : SerchFiles(ui.lineEdit_originalDir->text().toLocal8Bit().constData(), ".kgma")) {
		files.push_back(var);
	}

	for (const auto& file : files) {
		auto item = new QStandardItem(QString::fromUtf8(file.u8string().c_str()));
		model.appendRow(item);
	}

	ui.listView_originalFiles->setModel(&model);
	addlog("找到了 " + to_string(files.size()) + " 个文件");
}

void AudioDecrypt::SelectSaveDir()
{
	auto folderPath = new QString(QFileDialog::getExistingDirectory(this, tr("选择文件夹"), "/", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks));
	if (folderPath->isEmpty()) { delete folderPath; return; }
	string Dir = folderPath->toStdString();
	ui.lineEdit_saveDir->setText(*folderPath);
	delete folderPath;
}

void AudioDecrypt::addlog(const string& info)
{
	time_t now = time(nullptr);
	char buffer[80];
	strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&now));

	auto text = QString::fromUtf8("[" + string(buffer) + "]" + info);
	ui.textEdit_log->append(text);
}