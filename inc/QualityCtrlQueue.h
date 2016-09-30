/**
 *	@date		2016:9:21   15:07
 *	@name	 	QualityCtrlQueue.h
 *	@author		zhuqingquan	
 *	@brief		a queue for control the quality of play video and audio
 **/
#ifdef WINDOWS
#pragma once
#endif

#ifndef _QUALITY_CTRL_QUEUE_H_
#define _QUALITY_CTRL_QUEUE_H_

#include <list>
#include "CriticalSection.h"
#include "TimeCounter.h"

struct Item
{
	unsigned int id;
	unsigned int timestamp;

	unsigned int getTimestamp() { return timestamp; }
};

namespace Video
{
	//typedef void (*MediaDataCallback)(Item* data, void* userdata);

	template<typename VideoDataType, typename AudioDataType>
	struct MediaDataCallback
	{
		virtual int doVideoDataCallback(VideoDataType vData) = 0;
		virtual int doAudioDataCallback(AudioDataType aData) = 0;
	};

	/**
	 *	@name	QualityCtrlQueue
	 *	@brief	����Ƶ�����������ƶ���
	 *			����������⣺
	 *			1������ͬ������������������ʹ����ͬ����ʼʱ�����������Ƶ���űض���׼ȷ��ʱ���
	 *			2������cachesize��ָ����ʱ�������ݡ�ʵ�ʻ�������ݿ���С�������ֵ�������������������ֵ
	 *				���������ݷ�ӵʱ�ᶪ����������
	 *			3���������粨����ԭ��������Դ�����ж�֮�����»�������
	 *			4����΢�����粨�����ᵼ�����ݶ�ʧ
	 *			5������ʱ������ã�����������������Դ
	 *			====================================== ���¿�ѡ��ʵ�� ==================================
	 *			6��������ͨ�����ýӿڻ�֪���ò��������Ƿ�ᱻ������������ܱ����������ѡ�񲻲���������ݱ�����
	 **/
	template<typename VideoDataType, typename AudioDataType>
	class QualityCtrlQueue
	{
	public:
		QualityCtrlQueue();
		~QualityCtrlQueue();

		bool setCacheSize(unsigned int videoCacheMillsec, unsigned int audioCacheMillsec)
		{
			m_videoDelayTime = videoCacheMillsec;
			m_audioDelayTime = audioCacheMillsec;
			return true;
		}

		int getVideoCacheSize() const { return m_videoDelayTime; }
		int getAudioCacheSize() const { return m_audioDelayTime; }

		/**
		 *	@name			setDropDataThreshold
		 *	@brief			���ö������ݵ���ֵ�������������ʱ������VideoCacheSize+DropDataThresholdʱ�����ᴥ��������
		 *	@param[in]		unsigned int thresholdMillsec �������ݵ���ֵ������
		 *	@return			void 
		 **/
		void setDropDataThreshold(unsigned int thresholdMillsec) { m_dropThreshold = thresholdMillsec; }

		bool start();
		void stop();

		void setVideoDataCallback(MediaDataCallback<VideoDataType, AudioDataType>* videocallback)
		{
			m_videocb = videocallback;
		}
		void setAudioDataCallback(MediaDataCallback<VideoDataType, AudioDataType>* audiocallback)
		{
			m_audiocb = audiocallback;
		}

		bool insert_video(VideoDataType data);
		bool insert_audio(AudioDataType data);

		void doQuelityThread();

	private:
		VideoDataType getVideoSample();
		AudioDataType getAudioSample();

		void doVideoDataCallback(VideoDataType vData);
		void doAudioDataCallback(AudioDataType aData);

		unsigned int getCachedDataSize(const std::list<VideoDataType>& datalist);

		void resetTimeState();

	private:
		std::list<VideoDataType> m_VideoData;
		std::list<AudioDataType> m_AudioData;

		CCriticalLock m_videoSrcListLock;
		CCriticalLock m_AudioSrcListLock;
		CCriticalLock m_TsLock;

		HANDLE m_qualityThread;
		bool m_isQuelityThreadRunning;

		RPC::TimeCounter m_TimeCounter;
		LONGLONG m_firstPresentTime;
		LONGLONG m_startFrameTime;

		unsigned int m_videoDelayTime;
		unsigned int m_audioDelayTime;
		unsigned int m_dropThreshold;

		MediaDataCallback<VideoDataType, AudioDataType>* m_videocb;
		MediaDataCallback<AudioDataType, AudioDataType>* m_audiocb;

		long m_firstFrameType;
		long m_videoDropCount;
		long m_audioDropCount;
		long m_modifyDIS;
		long m_modifyDISIncress;

		unsigned int m_vLastOutputTS;		//����������Ƶ֡��ʱ���
		unsigned int m_aLastOutputTS;		//����������Ƶ֡��ʱ���
		unsigned int m_vLastInputTS;		//����������Ƶ֡��ʱ���
		unsigned int m_aLastInputTS;		//����������Ƶ֡��ʱ���
	};

	template<typename VideoDataType, typename AudioDataType>
	DWORD WINAPI qualityThreadWork(LPVOID param);

	template<typename VideoDataType, typename AudioDataType>
	DWORD WINAPI qualityThreadWork(LPVOID param)
	{
		QualityCtrlQueue<VideoDataType, AudioDataType>* pThis = (QualityCtrlQueue<VideoDataType, AudioDataType>*)param;
		if(pThis)
		{
			pThis->doQuelityThread();
		}
		return 0;
	}

	template<typename VideoDataType, typename AudioDataType>
	void QualityCtrlQueue<VideoDataType, AudioDataType>::doQuelityThread()
	{
		int looptimes = 1;
		unsigned int vLastInputTS = m_vLastInputTS;		//����������Ƶ֡��ʱ���
		unsigned int aLastInputTS = m_aLastInputTS;		//����������Ƶ֡��ʱ���
		while(m_isQuelityThreadRunning)
		{
			VideoDataType pVideo = getVideoSample();
			doVideoDataCallback(pVideo);
			AudioDataType pAudio = getAudioSample();
			doAudioDataCallback(pAudio);
			Sleep(10);

			if((looptimes%1000)==0)
			{
				CAutoLock vlock(m_videoSrcListLock);
				CAutoLock alock(m_AudioSrcListLock);
				unsigned int vCached = getCachedDataSize(m_VideoData);
				unsigned int aCached = getCachedDataSize(m_AudioData);
				char msg[512] = {0};
				sprintf(msg, "cached video %u audio %u  next v_ts %u a_ts %u  Droped v=%d a=%d md=%d mdInc=%d\n", 
					vCached, aCached,
					m_VideoData.size()>0 ? m_VideoData.front()->getTimestamp() : 0,
					m_AudioData.size()>0 ? m_AudioData.front()->getTimestamp() : 0,
					m_videoDropCount, m_audioDropCount, m_modifyDIS, m_modifyDISIncress);
				OutputDebugStringA(msg);
				if(vCached>m_videoDelayTime || aCached>m_audioDelayTime)
				{
					if(m_firstPresentTime!=-1)
					{
						m_firstPresentTime -= 10;
						InterlockedIncrement(&m_modifyDIS);
					}
				}
				if(vCached<m_videoDelayTime || aCached<m_audioDelayTime)
				{
					if(vLastInputTS!=m_vLastInputTS || aLastInputTS!=m_aLastInputTS)
					{
						m_firstPresentTime += 10;
						InterlockedIncrement(&m_modifyDISIncress);
					}				
				}
				vLastInputTS = m_vLastInputTS;
				aLastInputTS = m_aLastInputTS;
			}
			looptimes++;
		}
	}

	template<typename VideoDataType, typename AudioDataType>
	bool QualityCtrlQueue<VideoDataType, AudioDataType>::start()
	{
		m_isQuelityThreadRunning = true;
		m_qualityThread = CreateThread(NULL, 0, qualityThreadWork<VideoDataType, AudioDataType>, this, 0, 0);
		return 0;
	}

	template<typename VideoDataType, typename AudioDataType>
	void QualityCtrlQueue<VideoDataType, AudioDataType>::stop()
	{
		m_isQuelityThreadRunning = false;
		WaitForSingleObject(m_qualityThread, 5000);
		CloseHandle(m_qualityThread);
		m_qualityThread = NULL;
	}

	template<typename VideoDataType, typename AudioDataType>
	VideoDataType QualityCtrlQueue<VideoDataType, AudioDataType>::getVideoSample()
	{
		VideoDataType pSample = NULL;
		{
			CAutoLock lock(m_videoSrcListLock);
			while(getCachedDataSize(m_VideoData)>m_videoDelayTime+m_dropThreshold)
			{
				VideoDataType f1 = m_VideoData.front();
				m_VideoData.pop_front();
				if(1==m_firstFrameType)
				{
					VideoDataType f2 = m_VideoData.front();
					unsigned int interval = f2->getTimestamp() - f1->getTimestamp();
					CAutoLock tslock(m_TsLock);
					if(m_firstPresentTime!=-1)
					{
						m_firstPresentTime -= interval;
					}
				}
				delete f1;
				InterlockedIncrement(&m_videoDropCount);
			}
			while(m_VideoData.size()>0)
			{
				//����ж����Ч������һ�������
				pSample = m_VideoData.front();
				if(NULL==pSample)
				{
					m_VideoData.pop_front();
					continue;
				}
				else
				{
					break;
				}
			}
		}
		if(NULL==pSample)
			return NULL;

		LONGLONG ts = pSample->getTimestamp();//(LONGLONG)pSample->mediaTime.timeStart.tv_sec * 1000 + (LONGLONG)pSample->mediaTime.timeStart.tv_usec / 1000;
		if(ts<m_vLastOutputTS)
		{
			resetTimeState();
		}

		LONG interval = 0;
		LONGLONG presentInterval = 0;
		LONGLONG now = m_TimeCounter.now_in_millsec();

		{
			CAutoLock tslock(m_TsLock);
			InterlockedCompareExchange64(&m_firstPresentTime, now, -1);
			InterlockedCompareExchange64(&m_startFrameTime, ts, -1);
			interval = ts - m_startFrameTime;
			presentInterval = now - m_firstPresentTime;
		}

		if(presentInterval < interval+m_videoDelayTime)
		{
			return NULL;
		}

		CAutoLock lock(m_videoSrcListLock);
		m_VideoData.pop_front();
		return pSample;
	}

	template<typename VideoDataType, typename AudioDataType>
	AudioDataType QualityCtrlQueue<VideoDataType, AudioDataType>::getAudioSample()
	{
		AudioDataType pSample = NULL;
		{
			CAutoLock lock(m_AudioSrcListLock);
			while(getCachedDataSize(m_AudioData)>m_audioDelayTime+m_dropThreshold)
			{
				AudioDataType f1 = m_AudioData.front();
				m_AudioData.pop_front();
				if(2==m_firstFrameType)
				{
					AudioDataType f2 = m_AudioData.front();
					unsigned int interval = f2->getTimestamp() - f1->getTimestamp();
					CAutoLock tslock(m_TsLock);
					if(m_firstPresentTime!=-1)
					{
						m_firstPresentTime -= interval;
					}
				}

				delete f1;
				InterlockedIncrement(&m_audioDropCount);
			}
			while(m_AudioData.size()>0)
			{
				//����ж����Ч������һ�������
				pSample = m_AudioData.front();
				if(NULL==pSample)
				{
					m_AudioData.pop_front();
					continue;
				}
				else
				{
					break;
				}
			}
		}
		if(NULL==pSample)
			return NULL;

		LONGLONG ts = pSample->getTimestamp();//(LONGLONG)pSample->mediaTime.timeStart.tv_sec * 1000 + (LONGLONG)pSample->mediaTime.timeStart.tv_usec / 1000;
		if(ts<m_aLastOutputTS)
		{
			resetTimeState();
		}

		LONG interval = 0;
		LONGLONG presentInterval = 0;
		LONGLONG now = m_TimeCounter.now_in_millsec();

		{
			CAutoLock tslock(m_TsLock);
			InterlockedCompareExchange64(&m_firstPresentTime, now, -1);
			InterlockedCompareExchange64(&m_startFrameTime, ts, -1);
			interval = ts - m_startFrameTime;
			presentInterval = now - m_firstPresentTime;
		}

		if(presentInterval < interval+m_audioDelayTime)
		{
			return NULL;
		}

		CAutoLock lock(m_AudioSrcListLock);
		m_AudioData.pop_front();
		return pSample;
	}

	template<typename VideoDataType, typename AudioDataType>
	void QualityCtrlQueue<VideoDataType, AudioDataType>::doVideoDataCallback( VideoDataType vData )
	{
		if(vData)
		{
			m_vLastOutputTS = vData->getTimestamp();
			if(m_videocb)
			{
				m_videocb->doVideoDataCallback(vData);
			}
		}
	}

	template<typename VideoDataType, typename AudioDataType>
	void QualityCtrlQueue<VideoDataType, AudioDataType>::doAudioDataCallback( AudioDataType aData )
	{
		if(aData)
		{
			m_aLastOutputTS = aData->getTimestamp();
			if(m_audiocb)
			{
				m_audiocb->doAudioDataCallback(aData);
			}
		}
	}

	template<typename VideoDataType, typename AudioDataType>
	unsigned int QualityCtrlQueue<VideoDataType, AudioDataType>::getCachedDataSize(const std::list<VideoDataType>& datalist)
	{
		if(datalist.size()<=1)
			return 0;
		return datalist.back()->getTimestamp() - datalist.front()->getTimestamp();
	}

	template<typename VideoDataType, typename AudioDataType>
	bool QualityCtrlQueue<VideoDataType, AudioDataType>::insert_video( VideoDataType data )
	{
		InterlockedCompareExchange(&m_firstFrameType, 1, 0);
		CAutoLock lock(m_videoSrcListLock);
		m_VideoData.push_back(data);
		m_vLastInputTS = data->getTimestamp();
		return true;
	}

	template<typename VideoDataType, typename AudioDataType>
	bool QualityCtrlQueue<VideoDataType, AudioDataType>::insert_audio( AudioDataType data )
	{
		InterlockedCompareExchange(&m_firstFrameType, 2, 0);
		CAutoLock lock(m_AudioSrcListLock);
		m_AudioData.push_back(data);
		m_aLastInputTS = data->getTimestamp();
		return true;
	}

	template<typename VideoDataType, typename AudioDataType>
	QualityCtrlQueue<VideoDataType, AudioDataType>::QualityCtrlQueue()
		: m_qualityThread(NULL), m_isQuelityThreadRunning(false)
		, m_firstPresentTime(-1), m_startFrameTime(-1)
		, m_videoDelayTime(0), m_audioDelayTime(0), m_dropThreshold(0)
		, m_videocb(NULL), m_audiocb(NULL)
		, m_firstFrameType(0)
		, m_videoDropCount(0), m_audioDropCount(0), m_modifyDIS(0), m_modifyDISIncress(0)
		, m_vLastOutputTS(0), m_aLastOutputTS(0), m_vLastInputTS(0), m_aLastInputTS(0)
	{
	}

	template<typename VideoDataType, typename AudioDataType>
	QualityCtrlQueue<VideoDataType, AudioDataType>::~QualityCtrlQueue()
	{
		stop();
	}

	template<typename VideoDataType, typename AudioDataType>
	void QualityCtrlQueue<VideoDataType, AudioDataType>::resetTimeState()
	{
		m_firstPresentTime = -1;
		m_startFrameTime = -1;
		m_vLastOutputTS = 0;
		m_aLastOutputTS = 0;
	}

}

#endif //_QUALITY_CTRL_QUEUE_H_