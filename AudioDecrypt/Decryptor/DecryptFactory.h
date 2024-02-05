#pragma once
#include <qdebug>
#include <QObject>
#include <QThreadPool>
#include <QRunnable>
#include <Qstring>
#include <queue>
#include <memory>

#include "Common.h"

class DecryptFactory : public QObject
{
	Q_OBJECT

private:
	queue<filesystem::path> _tasks;//全部音频文件
	vector<QString> _exceptions;//错误统计
	bool _enDel = false;
	int _threadCount = 8;
	filesystem::path _outputPath = "";
	bool _write163Key = true;
	void NCMDecryptor(const filesystem::path& originalFilePath) const;
	void KGMADecryptor(const filesystem::path& originalFilePath) const;


public:

	explicit DecryptFactory(int threadCount, QObject* parent = nullptr)
		: QObject(parent)
	{
		this->_threadCount = threadCount;
	};

	void Decrypt(filesystem::path& filepath)
	{
		try
		{
			if (filepath.extension().string() == ".ncm")
			{
				this->NCMDecryptor(filepath);
			}
			else if (filepath.extension().string() == ".kgm" or filepath.extension().string() == ".kgma")
			{
				this->KGMADecryptor(filepath);
			}
		}
		catch (exception e)
		{
		};
	}

	void SetThreadAmount(int count)
	{
		_threadCount = count;
	}

	void SetOutputPath(filesystem::path outputPath)
	{
		this->_outputPath = outputPath;
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
	}
};