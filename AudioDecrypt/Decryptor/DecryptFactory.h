#pragma once
#include <qdebug>
#include <QObject>
#include <QThreadPool>
#include <QRunnable>
#include <Qstring>
#include <queue>
#include <memory>

#include "Common.h"
#include "NCMDecryptor.h"
#include "KGMADecryptor.h"

class DecryptThread : public QObject
{
	Q_OBJECT

private:
	void NCMDecryptor(const filesystem::path& originalFilePath);
	void KGMADecryptor(const filesystem::path& originalFilePath);

	//
	filesystem::path& _outputPath;
	bool& _enDel;
	bool& _write163Key;
	NCM ncm;
	KGMA kgma;

public:
	bool _finished = true;
	int _count;

	DecryptThread(filesystem::path& outputPath, bool& enDel, bool& write163Key, int count) :
		_outputPath(outputPath), _enDel(enDel), _write163Key(write163Key)
	{
		this->_count = count;
	};

	~DecryptThread()
	{
	};


public slots:

	void Decrypt(filesystem::path filepath)
	{
		this->_finished = false;
		qDebug() << "ID: " << QThread::currentThreadId() << " " << filepath.c_str();
		try
		{
			if (filepath.extension().string() == ".ncm")
			{
				ncm.NCMDecrypt(filepath, _write163Key, _outputPath);
			}
			else if (filepath.extension().string() == ".kgm" or filepath.extension().string() == ".kgma")
			{
				kgma.KGMADecrypt(filepath, _outputPath);
			}
			this->_finished = true;
			emit sigFinished(this->_count, filepath);
		}
		catch (exception e)
		{
			this->_finished = true;
			auto info = QString::fromUtf8(e.what());
			emit sigErr(this->_count, filepath, info);
		};
	}

	void Show()
	{
		qDebug() << "ID: " << QThread::currentThreadId() << "  count: " << _count;
	}

signals:
	void sigErr(int thCount, filesystem::path path, QString e);
	void sigFinished(int thCount, filesystem::path path);
	void sigDecrypt(filesystem::path filepath);
};



class DecryptFactory : public QObject
{
	Q_OBJECT

private:
	queue<filesystem::path> _tasks;//全部音频文件
	vector<filesystem::path> _files;
	int _threadCount = 8;
	vector<QString> _errs;
	QList<DecryptThread*> _decryptor;
	QList<QThread*> _threads;

	//
	filesystem::path _outputPath = "";
	bool _enDel = false;
	bool _write163Key = true;

	void CreateTh(int id)
	{
		auto th = new QThread(this);
		auto de = new DecryptThread(this->_outputPath, this->_enDel, this->_write163Key, id);
		de->moveToThread(th);
		connect(th, &QThread::started, de, &DecryptThread::Show);
		connect(de, &DecryptThread::sigDecrypt, de, &DecryptThread::Decrypt);
		connect(de, &DecryptThread::sigFinished, this, &DecryptFactory::ThreadFinished);
		connect(de, &DecryptThread::sigErr, this, &DecryptFactory::ThreadErr);
		th->start();
		this->_decryptor.push_back(de);
		this->_threads.push_back(th);
	}

	void DelTh()
	{
		auto de = this->_decryptor.last();
		auto th = this->_threads.last();
		th->quit();
		th->wait();
		this->_decryptor.removeLast();
		this->_threads.removeLast();
	}

public:

	explicit DecryptFactory(int threadCount, QObject* parent = nullptr)
		: QObject(parent)
	{
		this->_threadCount = threadCount;

		for (int i = 0; i < this->_threadCount; i++)
		{
			this->CreateTh(i);
		}
	};

	~DecryptFactory()
	{
		for (auto thread : this->_threads)
		{
			thread->quit();
			thread->wait();
			delete thread;
		}
		for (auto decryptor : this->_decryptor)
		{
			delete decryptor;
		}
	};

	void SetThreadAmount(int count)
	{
		_threadCount = count;
	}

	void SetOutputPath(filesystem::path outputPath)
	{
		this->_outputPath = outputPath.generic_wstring();
	}

	void EnWite163Key(bool write163Key)
	{
		this->_write163Key = write163Key;
	}

	void EnDel(bool del)
	{
		this->_enDel = del;
	}

	void Add(vector<filesystem::path> files)
	{
		this->_files = files;
		for (auto& file : this->_files)
		{
			this->_tasks.push(file);
		}
		for (auto de : this->_decryptor)
		{
			if (this->_tasks.empty()) { return; };
			if (!de->_finished) { continue; }
			de->sigDecrypt(this->_tasks.front());
			this->_tasks.pop();
		}
	}

public slots:

	void ThreadFinished(int thCount, filesystem::path path)
	{
		auto info = QString::fromUtf8("线程" + to_string(thCount) + "处理完毕: ") + QString::fromStdWString(path.filename().wstring());
		emit sigSingleFinished(info);

		//创建或删除线程
		if (this->_threads.size() < this->_threadCount)
		{
			for (int i = _threads.size(); i < _threadCount; i++)
			{
				this->CreateTh(i);
			}
		}
		if (this->_threads.size() > this->_threadCount)
		{
			for (int i = _threads.size(); i > _threadCount; i--)
			{
				this->DelTh();
			}
		}

		//为所有线程发放任务
		bool end = true;
		for (auto de : this->_decryptor)
		{
			if (!de->_finished) { continue; }
			if (this->_tasks.empty()) { continue; };
			end = false;
			de->sigDecrypt(this->_tasks.front());
			this->_tasks.pop();
		}

		//无更多任务
		for (auto de : this->_decryptor)
		{
			if (!de->_finished) { end = false; }
		}

		if (end)
		{
			auto info = QString::fromUtf8("所有线程处理完毕\n");
			if (!this->_errs.empty()) info += (QString::fromUtf8("处理时发生了一些意外:\n") + Qjoin(this->_errs, "\n"));
			emit sigAllFinished(info);
		};
	}

	void ThreadErr(int thCount, filesystem::path path, QString e)
	{
		auto info = 
			QString::fromUtf8("线程" + to_string(thCount) + "处理 \"") +
			QString::fromStdWString(path.filename().wstring()) +
			QString::fromUtf8("\"s时出现错误:\n  ") + e;
	}

signals:
	void sigSingleFinished(QString info);
	void sigAllFinished(QString info);
};