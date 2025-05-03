#include "MusicDecryptor.h"

MusicDecryptor::MusicDecryptor(QWidget* parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	//初始化ui量
	this->_listview_files_model = make_unique<QStandardItemModel>();
	//初始化内部量
	this->_decrypt_thread_manager = make_unique<DecryptThreadManager>();
	//链接ui信号
	connect(this->ui.pushButton_selsctDir, SIGNAL(clicked()), this, SLOT(selectDir()));
	connect(this->ui.pushButton_choseSaveDir, SIGNAL(clicked()), this, SLOT(selectSaveDir()));
	connect(this->ui.pushButton_selectFiles, SIGNAL(clicked()), this, SLOT(selectFiles()));
	connect(this->ui.pushButton_startProcess, SIGNAL(clicked()), this, SLOT(startProcess()));
	//链接线程信号
	connect(this->_decrypt_thread_manager.get(), &DecryptThreadManager::sig_atask_finished, [=](QString info) {this->addlog(info,Qt::gray);});
	connect(this->_decrypt_thread_manager.get(), &DecryptThreadManager::sig_errorOccured, [=](QString info) {this->addlog(u8"错误:",Qt::red);this->addlog(info+u8"\n");});
	connect(this->_decrypt_thread_manager.get(), &DecryptThreadManager::sig_all_tasks_finished, [=]() {this->addlog(u8"--全部任务完成--",Qt::darkMagenta);});
}

MusicDecryptor::~MusicDecryptor()
{}

void MusicDecryptor::startProcess()
{
	if (not this->_decrypt_thread_manager->isFree())
	{
		addlog(u8"线程正在运行，请等待", Qt::blue);
		return;
	}
	if (this->_files.size() == 0)
	{
		addlog(u8"无可处理文件", Qt::blue);
		return;
	}
	// 确认保存目录
	filesystem::path savedir;
	if (this->ui.checkBox_useSaveDir->checkState()) 
	{ 
		savedir = this->ui.lineEdit_saveDir->text().toLocal8Bit().constData();
		if (savedir.empty() or not filesystem::exists(savedir))
		{
			savedir =  QCoreApplication::applicationDirPath().toStdWString();
		}
	}
	else { savedir = filesystem::path(); }

	// 添加任务
	this->_decrypt_thread_manager->setMaxThreads(this->ui.spinBox_threadCount->value());
	this->_decrypt_thread_manager->addTasks(this->_files, savedir, this->ui.checkBox_write163Key->checkState(),this->ui.checkBox_delete->checkState());
	this->addlog(u8"线程开始", Qt::darkMagenta);
	this->_files.clear();
}

void MusicDecryptor::selectDir()
{
	// 选择文件夹
	auto folderPath = QString(QFileDialog::getExistingDirectory(this, tr("选择文件夹"), "/", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks));
	if (folderPath.isEmpty()) { addlog(u8"未选择文件夹", Qt::blue); return; }
	fs::path dir = folderPath.toStdWString();
	this->ui.lineEdit_originalDir->setText(folderPath);

	// 重置
	this->_files.clear();
	this->_listview_files_model->clear();
	// 遍历搜索
	auto match = [](fs::path entry)
		{
			if (entry.extension() == u8".ncm" or
				entry.extension() == u8".kgm" or
				entry.extension() == u8".kgma" )
			{
				return true;
			}
			else { return false; }
		};
	for (const auto& entry : fs::recursive_directory_iterator(dir))
	{
		if (entry.is_regular_file() and match(entry.path()))
		{
			this->_files.push_back(entry.path());
		}
	}
	// 注入模型
	for (const auto& file : this->_files) 
	{
		auto item = new QStandardItem(QString(file.filename().wstring()));
		this->_listview_files_model->appendRow(item);
	}

	// 设置ui'
	this->ui.listView_originalFiles->setModel(this->_listview_files_model.get());
	this->addlog(QString(L"找到了 " + to_wstring(_files.size()) + L" 个文件"));
}

void MusicDecryptor::selectSaveDir()
{
	auto folderPath = QFileDialog::getExistingDirectory(this, u8"选择文件夹", "/", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if (folderPath.isEmpty()) { return; }
	string Dir = folderPath.toStdString();
	this->ui.lineEdit_saveDir->setText(folderPath);
}

void MusicDecryptor::selectFiles()
{
	QString filterText = u8"全部 (*.ncm *.kgm *.kgma);;网易云文件 (*.ncm);;酷狗文件 (*.kgm *.kgma)";
	QStringList selectedFiles = QFileDialog::getOpenFileNames(this, u8"选择文件", QCoreApplication::applicationDirPath(), filterText);
	if (selectedFiles.isEmpty()) { addlog(u8"未选择文件", Qt::blue); return; }
	// 重置
	this->_files.clear();
	this->_listview_files_model->clear();
	// 注入模型
	for (QString i : selectedFiles)
	{
		this->_files.push_back(i.toStdWString());
	}
	for (const auto& file : this->_files)
	{
		auto item = new QStandardItem(QString::fromStdWString(file.filename().wstring()));
		this->_listview_files_model->appendRow(item);
	}
	// 设置ui'
	this->ui.listView_originalFiles->setModel(this->_listview_files_model.get());
	this->addlog(QString(L"选择了 " + to_wstring(_files.size()) + L" 个文件"));
}

void MusicDecryptor::addlog(QString Info, Qt::GlobalColor color, QString End)
{
	// 自动滑动到文本框底部
	QTextCursor text_cursor = this->ui.textEdit_log->textCursor();
	text_cursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
	this->ui.textEdit_log->setTextCursor(text_cursor);
	// 输出日志
	QTextCharFormat format;
	format.setForeground(color);
	text_cursor.mergeCharFormat(format);
	text_cursor.insertText(Info + End);
	this->ui.textEdit_log->setTextCursor(text_cursor);
}
