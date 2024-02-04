#pragma once
#include <QObject>
#include <QThreadPool>
#include <QRunnable>
#include <Qstring>
#include <queue>
#include <memory>

#include "Common.h"


class Decryptor : public QObject
{
	Q_OBJECT

public:
	bool _finished = true;
	QThread _thread;

	//
	filesystem::path _outputPath;
	filesystem::path _filepath;
	bool _write163Key = true;


	explicit Decryptor(QObject* parent = nullptr)
		: QObject(parent)
	{
		this->moveToThread(&_thread);
		connect(this, &Decryptor::SigDecrypt, this, &Decryptor::Decrypt);
	}

	~Decryptor()
	{
		this->_thread.exit();
		this->_thread.wait();
	}

	void KGMADecryptor(const filesystem::path& originalFilePath) const;
	void NCMDecryptor(const filesystem::path& originalFilePath) const;

signals:
	void SigDecrypt(filesystem::path& filepath);
	void SigFinished(Decryptor* ptr);
	void SigErr(exception e, filesystem::path& filepath);

public slots:
	void Decrypt(filesystem::path& filepath)
	{
		try
		{
			this->_finished = false;
			this->_filepath = filepath;
			if (filepath.extension().string() == ".ncm")
			{
				this->NCMDecryptor(filepath);
			}
			else if (filepath.extension().string() == ".kgm" or filepath.extension().string() == ".kgma")
			{
				this->KGMADecryptor(filepath);
			}
			this->_finished = true;
			emit SigFinished(this);
		}
		catch (exception e)
		{
			emit SigErr(e, filepath);
		}
	};
};

class DecryptFactory : public QObject
{
	Q_OBJECT

private:
	queue<filesystem::path> _tasks;//全部音频文件
	vector<shared_ptr<Decryptor>> _decryptors;//解密子线程
	vector<QString> _exceptions;//错误统计
	int _threadCount = 8;
	unsigned char _conut = 0;

public:
	filesystem::path _outputPath = "";
	bool _write163Key = true;

	explicit DecryptFactory(int threadCount, QObject* parent = nullptr)
		: QObject(parent)
	{
		this->_threadCount = threadCount;
		for (int i = 0; i < _threadCount; i++)
		{
			auto c = make_shared<Decryptor>();
			connect(c.get(), &Decryptor::SigFinished, this, &DecryptFactory::Finished);
			this->_decryptors.push_back(c);
		}
	};

	void SetOutputPath(filesystem::path outputPath)
	{
		this->_outputPath = outputPath;
		for (auto& decryptor_ : this->_decryptors)
		{
			decryptor_->_outputPath = outputPath;
		}
	}

	void EnWite163Key(bool write163Key)
	{
		this->_write163Key = write163Key;
		for (auto& decryptor_ : this->_decryptors)
		{
			decryptor_->_write163Key = write163Key;
		}
	}

	void Add(vector<filesystem::path> files)
	{
		if (files.size() == 0) { return; }
		for (const auto& file : files)
		{
			this->_tasks.push(file);
		}
		for (auto& decryptor_ : this->_decryptors)
		{
			if (!decryptor_->_finished)
			{
				emit decryptor_->SigDecrypt(this->_tasks.front());
				this->_tasks.pop();
			}
		}
	}

	void AllFinished()
	{
		QString info;
		info += "任务处理完毕\n";
		for (QString &i : this->_exceptions)
		{
			info += i;
		}
		emit SigEnd(info);
	}

signals: 
	void SigEnd(QString info);
	void SigSignalFinished(QString info);

public slots:
	//单个任务完成
	void Finished(Decryptor* decryptor)
	{
		if (this->_tasks.empty())
		{
			//判断线程是否全部结束
			for (auto& decryptor_ : this->_decryptors)
			{
				if (!decryptor_->_finished) { return; }
			}
			AllFinished();
			return;
		}
		emit SigSignalFinished("已完成: "+QString::fromWCharArray(decryptor->_filepath.filename().wstring().c_str()));
		emit decryptor->SigDecrypt(this->_tasks.front());
		this->_tasks.pop();
	}

	void Err(exception e, filesystem::path& filepath)
	{
		QString info = "在处理 [" + QString::fromWCharArray(filepath.filename().wstring().c_str()) + "] 时发生了错误: " + QString::fromUtf8(e.what()) + "\n";
		this->_exceptions.push_back(info);
	}
};