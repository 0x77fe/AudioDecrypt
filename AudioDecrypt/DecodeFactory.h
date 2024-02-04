#pragma once
#include "Common.h"

#include <QObject>
#include <QThreadPool>
#include <QRunnable>
#include <Qstring>

#include <queue>

using namespace std;

class Decrypter : public QObject
{
public:

	queue<filesystem::path> _tasks;
	QThread _thread;
	QObject* _admin;

	//
	filesystem::path _outputPath = "";


	Decrypter(QObject* admin = nullptr)
	{
		_admin = admin;
		this->moveToThread(&_thread);
	}

	~Decrypter()
		{
		_thread.exit();
		_thread.wait();
		}

	void KGMADecrypter(const filesystem::path& originalFilePath);

	void de(const filesystem::path& filename, filesystem::path outputpath = *new filesystem::path(), bool skip = false)
	{
		if (filename.extension().string() == ".ncm")
		{
	}
		else if (filename.extension().string() == ".kgm" or filename.extension().string() == ".kgma")
	{
		}
	}
};

class DecodeFactory : public QObject
{
	Q_OBJECT

private:
	queue<filesystem::path> _tasks;
	vector<Decrypter> decrypters;
	int _threadCount;

public:
	DecodeFactory(int threadCount)
	{
		_threadCount = threadCount;
	};

	void Add(vector<filesystem::path> files)
	{
		if (files.size() == 0) { return; }
		for (const auto& file : files)
		{
			_tasks.push(file);
		}
	}

	void Run()
	{

	}

};

void DecodeFactory(const filesystem::path& filename, filesystem::path outputpath = *new filesystem::path(), bool skip = false)
{
	{
		if (filename.extension().string() == ".ncm")
		{
			ncm::Decrypt(filename, outputpath, skip);
		}
		else if (filename.extension().string() == ".kgm" or filename.extension().string() == ".kgma")
		{
			kgma::Decrypt(filename, outputpath, skip);
		}
	}
}