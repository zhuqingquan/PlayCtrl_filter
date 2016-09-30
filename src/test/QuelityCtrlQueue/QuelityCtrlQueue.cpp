// QuelityCtrlQueue.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "QualityCtrlQueue.h"
#include <fstream>
#include <time.h> 

bool isRunning = false;

DWORD WINAPI genNormalData(LPVOID param)
{
	Video::QualityCtrlQueue<Item*, Item*>* dataQueue = reinterpret_cast<Video::QualityCtrlQueue<Item*, Item*>*>(param);
	if(dataQueue==NULL)
		return -1;
	int videoInterval[1] = {40};
	int videoIntervalCount = 1;
	int audioInterval[3] = {17, 17, 16};
	int audioIntervalCount = 3;

	unsigned int lastVideoTS = 0;
	unsigned int lastAudioTS = 0;

	int videoIndex = 0;
	int audioIndex = 0;
	RPC::TimeCounter timecount;
	LONGLONG firstVideoDataOut = 0;
	LONGLONG firstAudioDataOut = 0;
	while(isRunning)
	{
		LONGLONG now = timecount.now_in_millsec();
		
		if((now - firstAudioDataOut) > (lastVideoTS+videoInterval[videoIndex%videoIntervalCount]))
		{
			Item* vData = new Item();
			vData->id = videoIndex;
			vData->timestamp = lastVideoTS;
			lastVideoTS += videoInterval[videoIndex%videoIntervalCount];
			videoIndex++;
			if(firstVideoDataOut==0)
			{
				firstVideoDataOut = now;
			}
			dataQueue->insert_video(vData);
		}

		if((now - firstAudioDataOut) > (lastAudioTS+audioInterval[audioIndex%audioIntervalCount]))
		{
			Item* aData = new Item();
			aData->id = audioIndex;
			aData->timestamp = lastAudioTS;
			lastAudioTS += audioInterval[audioIndex%audioIntervalCount];
			audioIndex++;
			if(firstAudioDataOut==0)
			{
				firstAudioDataOut = now;
			}
			dataQueue->insert_audio(aData);
		}
		Sleep(5);
	}
	return 0;
}

DWORD WINAPI genData_unstable(LPVOID param)
{
	Video::QualityCtrlQueue<Item*, Item*>* dataQueue = reinterpret_cast<Video::QualityCtrlQueue<Item*, Item*>*>(param);
	if(dataQueue==NULL)
		return -1;
	int videoInterval[1] = {40};
	int videoIntervalCount = 1;
	int audioInterval[3] = {17, 17, 16};
	int audioIntervalCount = 3;

	unsigned int lastVideoTS = 0;
	unsigned int lastAudioTS = 0;

	bool isNeedSimulateUnstable = true;
	int videoIndex = 0;
	int audioIndex = 0;
	RPC::TimeCounter timecount;
	LONGLONG firstVideoDataOut = 0;
	LONGLONG firstAudioDataOut = 0;
	srand((unsigned)time(NULL)); 
	while(isRunning)
	{
		LONGLONG now = timecount.now_in_millsec();

		if((now - firstAudioDataOut) > (lastVideoTS+videoInterval[videoIndex%videoIntervalCount]))
		{
			Item* vData = new Item();
			vData->id = videoIndex;
			vData->timestamp = lastVideoTS;
			lastVideoTS += videoInterval[videoIndex%videoIntervalCount];
			videoIndex++;
			if(firstVideoDataOut==0)
			{
				firstVideoDataOut = now;
			}
			if(now-firstVideoDataOut > 1000*60*3)
			{
				isNeedSimulateUnstable = false;
			}
			dataQueue->insert_video(vData);
		}

		if((now - firstAudioDataOut) > (lastAudioTS+audioInterval[audioIndex%audioIntervalCount]))
		{
			Item* aData = new Item();
			aData->id = audioIndex;
			aData->timestamp = lastAudioTS;
			lastAudioTS += audioInterval[audioIndex%audioIntervalCount];
			audioIndex++;
			if(firstAudioDataOut==0)
			{
				firstAudioDataOut = now;
			}
			dataQueue->insert_audio(aData);
		}

		
		int t = rand() % 20000;//now % 100;
		if(t>=40 && t<50 && isNeedSimulateUnstable)
		{
			int sleeptime = rand() % (3000-500) + 500;
			char msg[256] = {0};
			sprintf(msg, "Simulate network unstable %d~~~~~~~~~~~~~~~~~~~~~\n", sleeptime);
			OutputDebugStringA(msg);
			Sleep(sleeptime);
		}
		else
		{
			Sleep(5);
		}
	}
	return 0;
}

DWORD WINAPI genData_simulateReconnect(LPVOID param)
{
	Video::QualityCtrlQueue<Item*, Item*>* dataQueue = reinterpret_cast<Video::QualityCtrlQueue<Item*, Item*>*>(param);
	if(dataQueue==NULL)
		return -1;
	int videoInterval[1] = {40};
	int videoIntervalCount = 1;
	int audioInterval[3] = {17, 17, 16};
	int audioIntervalCount = 3;

	unsigned int lastVideoTS = 0;
	unsigned int lastAudioTS = 0;

	bool isNeedSimulateReconnection = false;
	int videoIndex = 0;
	int audioIndex = 0;
	RPC::TimeCounter timecount;
	LONGLONG firstVideoDataOut = 0;
	LONGLONG firstAudioDataOut = 0;
	srand((unsigned)time(NULL)); 
	while(isRunning)
	{
		LONGLONG now = timecount.now_in_millsec();

		if((now - firstAudioDataOut) > (lastVideoTS+videoInterval[videoIndex%videoIntervalCount]))
		{
			Item* vData = new Item();
			vData->id = videoIndex;
			vData->timestamp = lastVideoTS;
			lastVideoTS += videoInterval[videoIndex%videoIntervalCount];
			videoIndex++;
			if(firstVideoDataOut==0)
			{
				firstVideoDataOut = now;
			}
			dataQueue->insert_video(vData);
		}

		if((now - firstAudioDataOut) > (lastAudioTS+audioInterval[audioIndex%audioIntervalCount]))
		{
			Item* aData = new Item();
			aData->id = audioIndex;
			aData->timestamp = lastAudioTS;
			lastAudioTS += audioInterval[audioIndex%audioIntervalCount];
			audioIndex++;
			if(firstAudioDataOut==0)
			{
				firstAudioDataOut = now;
			}
			dataQueue->insert_audio(aData);
		}

		if(now-firstVideoDataOut > 1000*30*1)
		{
			isNeedSimulateReconnection = true;
		}


		if(isNeedSimulateReconnection)
		{
			lastVideoTS = 0;
			lastAudioTS = 0;
			firstVideoDataOut = 0;
			firstAudioDataOut = 0;
			isNeedSimulateReconnection = false;

			int t = rand();
			if(t%2)
			{
				OutputDebugStringA("Simulate network reconnection Sleep 3s before.\n");
				Sleep(3000);
			}
			int sleeptime = rand() % (3000-500) + 1000;
			char msg[256] = {0};
			sprintf(msg, "Simulate network reconnection %d~~~~~~~~~~~~~~~~~~~~~\n", sleeptime);
			OutputDebugStringA(msg);
			Sleep(sleeptime);
		}
		else
		{
			Sleep(5);
		}
	}
	return 0;
}

DWORD WINAPI genData_cached5secdatabeforestart(LPVOID param)
{
	Video::QualityCtrlQueue<Item*, Item*>* dataQueue = reinterpret_cast<Video::QualityCtrlQueue<Item*, Item*>*>(param);
	if(dataQueue==NULL)
		return -1;
	int videoInterval[1] = {40};
	int videoIntervalCount = 1;
	int audioInterval[3] = {17, 17, 16};
	int audioIntervalCount = 3;

	unsigned int lastVideoTS = 0;
	unsigned int lastAudioTS = 0;

	int videoIndex = 0;
	int audioIndex = 0;
	RPC::TimeCounter timecount;
	LONGLONG firstVideoDataOut = 0;
	LONGLONG firstAudioDataOut = 0;
	while(isRunning && (lastVideoTS<5000 || lastAudioTS<5000))
	{
		//push 5 second data very fast
		
		if(lastAudioTS<5000)
		{
			Item* aData = new Item();
			aData->id = audioIndex;
			aData->timestamp = lastAudioTS;
			lastAudioTS += audioInterval[audioIndex%audioIntervalCount];
			audioIndex++;
			dataQueue->insert_audio(aData);
		}
		if(lastVideoTS<5000)
		{
			Item* vData = new Item();
			vData->id = videoIndex;
			vData->timestamp = lastVideoTS;
			lastVideoTS += videoInterval[videoIndex%videoIntervalCount];
			videoIndex++;
			dataQueue->insert_video(vData);
		}
	}
	while(isRunning)
	{
		LONGLONG now = timecount.now_in_millsec();

		if((now - firstAudioDataOut) > (lastVideoTS+videoInterval[videoIndex%videoIntervalCount]))
		{
			Item* vData = new Item();
			vData->id = videoIndex;
			vData->timestamp = lastVideoTS;
			lastVideoTS += videoInterval[videoIndex%videoIntervalCount];
			videoIndex++;
			if(firstVideoDataOut==0)
			{
				firstVideoDataOut = now-5000;
			}
			dataQueue->insert_video(vData);
		}

		if((now - firstAudioDataOut) > (lastAudioTS+audioInterval[audioIndex%audioIntervalCount]))
		{
			Item* aData = new Item();
			aData->id = audioIndex;
			aData->timestamp = lastAudioTS;
			lastAudioTS += audioInterval[audioIndex%audioIntervalCount];
			audioIndex++;
			if(firstAudioDataOut==0)
			{
				firstAudioDataOut = now-5000;
			}
			dataQueue->insert_audio(aData);
		}
		Sleep(5);
	}
	return 0;
}
void videodatacallback(Item* data, void* userdata);
void audiodatacallback(Item* data, void* userdata);

class OutputDataInfo : public Video::MediaDataCallback<Item*, Item*>
{
public:
	virtual int doVideoDataCallback(Item* vData)
	{
		videodatacallback(vData, this);
		return 0;
	}

	virtual int doAudioDataCallback(Item* aData)
	{
		audiodatacallback(aData, this);
		return 0;
	}
};

void videodatacallback(Item* data, void* userdata)
{
	if(data==NULL)
		return;
	static RPC::TimeCounter timecount;
	static LONGLONG lastVideoTs = 0;
	static std::ofstream vResultFile("VideoCallbackResult.txt");
	if(!vResultFile)
	{
		delete data;
		return;
	}
	if(lastVideoTs==0)
		lastVideoTs = timecount.now_in_millsec();
	else
	{
		LONGLONG now = timecount.now_in_millsec();
		LONGLONG dis = now - lastVideoTs;
		lastVideoTs = now;
		char result[128] = {0};
		sprintf(result, "%lld\t\t%u \r\n", dis, data->getTimestamp());
		//OutputDebugStringA(result);
		vResultFile << result;
	}
	delete data;
}

void audiodatacallback(Item* data, void* userdata)
{
	if(data==NULL)
		return;
	static RPC::TimeCounter timecount;
	static LONGLONG lastTs = 0;
	static std::ofstream AresultFile("AudioCallbackResult.txt");
	if(!AresultFile)
	{
		delete data;
		return;
	}
	if(lastTs==0)
		lastTs = timecount.now_in_millsec();
	else
	{
		LONGLONG now = timecount.now_in_millsec();
		LONGLONG dis = now - lastTs;
		lastTs = now;
		char result[128] = {0};
		sprintf(result, "%lld\t\t%u \r\n", dis, data->getTimestamp());
		//OutputDebugStringA(result);
		AresultFile << result;
	}
	delete data;
}

int _tmain(int argc, _TCHAR* argv[])
{
	OutputDataInfo dataResult;
	Video::QualityCtrlQueue<Item*, Item*>* dataQueue = new Video::QualityCtrlQueue<Item*, Item*>();
	dataQueue->setCacheSize(2000, 2000);
	dataQueue->setDropDataThreshold(200);
	dataQueue->setVideoDataCallback(&dataResult);
	dataQueue->setAudioDataCallback(&dataResult);
	dataQueue->start();
	system("pause");

	isRunning = true;
	//HANDLE genDataTh = CreateThread(NULL, 0, genNormalData, dataQueue, 0, NULL);
	//HANDLE genDataTh = CreateThread(NULL, 0, genData_cached5secdatabeforestart, dataQueue, 0, NULL);
	//HANDLE genDataTh = CreateThread(NULL, 0, genData_unstable, dataQueue, 0, NULL);
	HANDLE genDataTh = CreateThread(NULL, 0, genData_simulateReconnect, dataQueue, 0, NULL);
	system("pause");
	isRunning = false;
	WaitForSingleObject(genDataTh, 5000);

	dataQueue->stop();
	delete dataQueue;
	return 0;
}

