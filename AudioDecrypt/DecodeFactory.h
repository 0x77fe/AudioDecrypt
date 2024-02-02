#pragma once
#include "NCMDecryptor.h"
#include "KGMADecoder.h"

#include <QObject>
#include <QThreadPool>
#include <QRunnable>
#include <Qstring>

#include <queue>

using namespace std;

class MyThread : public QRunnable
{
public:
	void (*log)(QString Info, QString End, bool Time);
	queue<filesystem::path> _tasks;
	MyThread(vector<filesystem::path> files, void (*sig)(QString Info, QString End, bool Time))
	{
		log = sig;
		for (const auto& file : files)
		{
			_tasks.push(file);
		}
	}
	void run() override
	{

	}
};

class DecodeFactory : public QObject
{
	Q_OBJECT

private:
	queue<filesystem::path> _tasks;
	vector<QThread> threads;
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