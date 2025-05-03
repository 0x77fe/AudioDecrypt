#pragma once
#if _DEBUG
#include <qdebug.h>
#endif
#include <QObject>
#include <QMutex>
#include <QCoreApplication>
#include <QThread>

#include ".\Private\DMusicFactory.h"

class DecryptWorker : public QThread
{
	Q_OBJECT

public:
	DecryptWorker(QObject* parent = nullptr) : QThread(parent){};
public slots:
	void decrypt(fs::path filepath, fs::path outputDir, bool enableCloudkey = true)
	{
#if _DEBUG
		qDebug() << "Thread ID: " << this->thread()->currentThreadId() << "\n";
#endif
		_free = false;
		this->_current_file = filepath;
		DMusicIOConfig config;
		if (outputDir.empty())
		{
			outputDir = filepath.parent_path();
		}
		config.outputDir = outputDir;
		config.enWriteCloudkey = enableCloudkey;
		config.filepath = filepath;
		try
		{
			auto decryptor = DMusicFactory::create(config);
			decryptor->decrypt(config);
			_free = true;
			emit sig_decryptFinished(this);
		}
		catch (std::exception& e)
		{
#if _DEBUG
			qDebug() << e.what();
#endif

			emit sig_errorOccured(
				QString("Failed to decrypt file: ") +
				QString(config.filepath.u8string().c_str()) +
				QString("\nText:") +
				QString((const char8_t*)e.what()));
			_free = true;
		}
	};
	bool isFree() const
	{
		return _free;
	}
	fs::path getCurrentFilename() const
	{
		return this->_current_file.filename();
	};
	fs::path getCurrentFilepath() const
	{
		return this->_current_file;
	};

signals:
	void sig_errorOccured(QString);
	void sig_decryptFinished(DecryptWorker*);
	void sig_run_decrypt(fs::path filepath, fs::path outputDir, bool enableCloudkey = true);

private:
	bool _free = true;
	fs::path _current_file;
};

class DecryptThreadManager : public QObject
{
	Q_OBJECT

public:
	DecryptThreadManager(QObject* parent = nullptr) : QObject(parent)
	{
		this->_mutex = std::make_unique<QMutex>();
	};
	~DecryptThreadManager() 
	{
		for (auto& thread : this->_qthreads)
		{
			thread->quit();
			thread->wait();
		}
	};
	bool isFree() const
	{
		return _free;
	};
	void addTasks(const std::vector<fs::path>& files,const fs::path& outputDir, bool enableCloudkey = true, bool enableRemoveSourceFile = false)
	{
		if (not this->_free)
		{
			return;
		}
		this->_free = false;
		this->createDecryptThread();
		this->_files.insert(this->_files.end(), files.begin(), files.end());
		this->_output_dir = outputDir;
		this->_enable_cloudkey = enableCloudkey;
		this->_enable_remove_source_file = enableRemoveSourceFile;
		for (size_t i = 0; i < this->_max_threads and not this->_files.empty(); i++)
		{
			if (this->_decrypt_workers[i]->isFree())
			{
				emit _decrypt_workers[i]->sig_run_decrypt(this->_files.front(),outputDir, enableCloudkey);
				this->_files.erase(this->_files.begin());
			}
		}
	}

	void setMaxThreads(size_t max_threads)
	{
		if (max_threads > 16)
		{
			max_threads = 16;
		}
		this->_max_threads = max_threads;
	}

public slots:
	void aTaskFinished(DecryptWorker* worker)
	{
		auto lock = QMutexLocker(this->_mutex.get());
#if _DEBUG
		qDebug() << "Thread arrived critical section, ID: " << QThread::currentThreadId() << "\n";
#endif
		emit sig_atask_finished(QString(u8"解密完成: ") + QString(worker->getCurrentFilename().wstring()));
		if (this->_enable_remove_source_file)
		{
			fs::remove(worker->getCurrentFilepath());
		}
		if (this->_files.empty() and this->workersFree())
		{
			this->_free = true;
			emit sig_all_tasks_finished();
			return;
		}
		if (not this->_files.empty())
		{
			emit worker->sig_run_decrypt(this->_files.front(), this->_output_dir, this->_enable_cloudkey);
			this->_files.erase(this->_files.begin());
		}
	};
	void errorOccured(QString error)
	{
		emit sig_errorOccured(error);
	};
signals:
	void sig_atask_finished(QString);
	void sig_all_tasks_finished();
	void sig_errorOccured(QString);

private:
	bool _free = true;
	bool _enable_cloudkey = true;
	bool _enable_remove_source_file = false;
	fs::path _output_dir;
	size_t _max_threads = 8;
	std::vector<fs::path> _files;
	std::unique_ptr<QMutex> _mutex;
	std::vector<std::unique_ptr<DecryptWorker>> _decrypt_workers;
	std::vector<std::unique_ptr<QThread>> _qthreads;

	void createDecryptThread()
	{
		if (this->_max_threads > 16)
		{
			this->_max_threads = 16;
		}
		for (auto& thread : this->_qthreads)
		{
			if (thread->isRunning())
			{
				thread->quit();
				thread->wait();
			}
		}
		this->_decrypt_workers.resize(this->_max_threads);
		this->_qthreads.resize(this->_max_threads);
		for (size_t i = 0; i < this->_max_threads; i++)
		{
			this->_decrypt_workers[i] = std::make_unique<DecryptWorker>();
			this->_qthreads[i] = std::make_unique<QThread>(nullptr);
			this->_decrypt_workers[i]->moveToThread(this->_qthreads[i].get());
			this->_qthreads[i]->start();
			connect(this->_decrypt_workers[i].get(), &DecryptWorker::sig_run_decrypt, this->_decrypt_workers[i].get(), &DecryptWorker::decrypt);
			connect(this->_decrypt_workers[i].get(), &DecryptWorker::sig_decryptFinished, this, &DecryptThreadManager::aTaskFinished);
			connect(this->_decrypt_workers[i].get(), &DecryptWorker::sig_errorOccured, this, &DecryptThreadManager::errorOccured);
		}
	}

	bool workersFree() const
	{
		for (auto& worker : this->_decrypt_workers)
		{
			if (not worker->isFree())
			{
				return false;
			}
		}
		return true;
	};
};