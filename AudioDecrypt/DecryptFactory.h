#pragma once
#include "stdafx.h"
#include "Decryptor/Common.h"

#include <QObject>
#include <QThreadPool>
#include <QRunnable>
#include <Qstring>

#include <queue>

using namespace std;

class Decryptor : public QObject
{
	Q_OBJECT

public:
	bool _finished = true;
	QThread _thread;

	//
	filesystem::path _outputPath;
	filesystem::path _filepath;
	bool _write163Key;


	Decryptor(int admin,filesystem::path outputPath,bool write163Key)
	{
		//this->_admin = (DecodeFactory*)admin;
		_outputPath = outputPath;
		_write163Key = write163Key;
		this->moveToThread(&_thread);
	}

	~Decryptor()
	{
		this->_thread.exit();
		this->_thread.wait();
	}

	void KGMADecryptor(const filesystem::path& originalFilePath);
	void NCMDecryptor(const filesystem::path& originalFilePath);

	void De(filesystem::path& filepath)
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
		emit SigFinished(filepath);
	}

signals:
	void SigFinished(filesystem::path& FilePath);

};

class DecodeFactory : public QObject
{
	Q_OBJECT

private:
	queue<filesystem::path> _tasks;
	vector<Decryptor> _decryptors;
	int _threadCount = 8;
	unsigned char _conut = 0;

public:
	filesystem::path _outputPath = "";
	bool _write163Key = true;

	DecodeFactory(int threadCount)
	{
		this->_threadCount = threadCount;
		//this->_decryptors.resize(threadCount, Decryptor((int)this));

		//connect(this, &DecodeFactory::SigFinished, this, &DecodeFactory::Finished);
	};

	void SetOutputPath(filesystem::path outputPath)
	{
		this->_outputPath = outputPath;
	}

	void EnWite163Key(bool write163Key)
	{
		this->_write163Key = write163Key;
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
			if (!decryptor_._finished)
			{
				decryptor_.De(this->_tasks.front());
				this->_tasks.pop();
			}
		}
	}

public slots:
	void Finished(Decryptor* decryptor)
	{
		emit SigFinished(decryptor->_filepath);
		if (this->_tasks.empty())
		{
			for (auto& decryptor_ : this->_decryptors)
			{
				if (!decryptor_._finished) { return; }
			}
			//emit SigAllFinished;
			return;
		}
		decryptor->De(this->_tasks.front());
		this->_tasks.pop();
	}

signals:
	void SigAllFinished();
	void SigFinished(filesystem::path filepath);
	void SigErr(QString info);
};