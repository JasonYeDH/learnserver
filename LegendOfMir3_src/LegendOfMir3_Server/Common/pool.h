//------------------------------------------------------------------------
//�صĸ���
//------------------------------------------------------------------------

/*
	һЩ�����ǳ�Ƶ���ı������� ���磺MemoryStream, Bundle, TCPPacket�ȵ�
	�������ض�ͨ������˷�ֵ��Ч��Ԥ����ǰ������һЩ���󻺴����������õ���ʱ��ֱ�ӴӶ������
	��ȡһ��δ��ʹ�õĶ��󼴿ɡ�
*/
#ifndef _POOL_H_
#define _POOL_H_

#define OBJECT_POOL_INIT_SIZE	16
#define OBJECT_POOL_INIT_MAX_SIZE	OBJECT_POOL_INIT_SIZE * 16

#include "mutex.h"

template<class T>
class ObjectPool
{
public:
	typedef std::list<T*> OBJECTS;

	ObjectPool(std::string name):
		objects_(),
		max_(OBJECT_POOL_INIT_MAX_SIZE),
		isDestroyed_(false),
		mutex_(),
		name_(name),
		total_allocs_(0),
		obj_count_(0)
	{
	}

	ObjectPool(std::string name, unsigned int preAssignVal, size_t max):
		objects_(),
		max_((max == 0 ? 1 : max)),
		isDestroyed_(false),
		mutex_(),
		name_(name),
		total_allocs_(0),
		obj_count_(0)
	{
	}

	~ObjectPool()
	{
		destroy();
	}	
	
	void destroy()
	{
		mutex_.lockMutex();

		isDestroyed_ = true;

		typename OBJECTS::iterator iter = objects_.begin();
		for(; iter!=objects_.end(); ++iter)
		{
			if(!(*iter)->destructorPoolObject())
			{
				delete (*iter);
			}
		}
				
		objects_.clear();	
		obj_count_ = 0;
		mutex_.unlockMutex();
	}

	const OBJECTS& objects(void) const { return objects_; }

	void assignObjs(unsigned int preAssignVal = OBJECT_POOL_INIT_SIZE)
	{
		for(unsigned int i=0; i<preAssignVal; ++i){
			objects_.push_back(new T);
			++total_allocs_;
			++obj_count_;
		}
	}

	/** 
		ǿ�ƴ���һ��ָ�����͵Ķ��� ����������Ѿ������򷵻����еģ�����
		����һ���µģ� �����������Ǽ̳���T�ġ�
	*/
	template<typename T1>
	T* createObject(void)
	{
		mutex_.lockMutex();

		while(true)
		{
			if(obj_count_ > 0)
			{
				T* t = static_cast<T1*>(*objects_.begin());
				objects_.pop_front();
				--obj_count_;
				t->onEabledPoolObject();
				mutex_.unlockMutex();
				return t;
			}

			assignObjs();
		}

		mutex_.unlockMutex();

		return NULL;
	}

	/** 
		����һ������ ����������Ѿ������򷵻����еģ�����
		����һ���µġ�
	*/
	T* createObject(void)
	{
		mutex_.lockMutex();

		while(true)
		{
			if(obj_count_ > 0)
			{
				T* t = static_cast<T*>(*objects_.begin());
				objects_.pop_front();
				--obj_count_;
				t->onEabledPoolObject();
				mutex_.unlockMutex();
				return t;
			}

			assignObjs();
		}

		mutex_.unlockMutex();

		return NULL;
	}

	/**
		����һ������
	*/
	void reclaimObject(T* obj)
	{
		mutex_.lockMutex();
		reclaimObject_(obj);
		mutex_.unlockMutex();
	}

	/**
		����һ����������
	*/
	void reclaimObject(std::list<T*>& objs)
	{
		mutex_.lockMutex();

		typename std::list< T* >::iterator iter = objs.begin();
		for(; iter != objs.end(); ++iter)
		{
			reclaimObject_((*iter));
		}
		
		objs.clear();

		mutex_.unlockMutex();
	}

	/**
		����һ����������
	*/
	void reclaimObject(std::vector< T* >& objs)
	{
		mutex_.lockMutex();

		typename std::vector< T* >::iterator iter = objs.begin();
		for(; iter != objs.end(); ++iter)
		{
			reclaimObject_((*iter));
		}
		
		objs.clear();

		mutex_.unlockMutex();
	}

	/**
		����һ����������
	*/
	void reclaimObject(std::queue<T*>& objs)
	{
		mutex_.lockMutex();

		while(!objs.empty())
		{
			T* t = objs.front();
			objs.pop();
			reclaimObject_(t);
		}

		mutex_.unlockMutex();
	}

	size_t size(void) const{ return obj_count_; }
	
	std::string c_str()
	{
		char buf[1024];

		mutex_.lockMutex();

		sprintf(buf, "ObjectPool::c_str(): name=%s, objs=%d/%d, isDestroyed=%s.\n", 
			name_.c_str(), (int)obj_count_, (int)max_, (isDestroyed ? "true" : "false"));

		mutex_.unlockMutex();

		return buf;
	}

	size_t max() const{ return max_; }
	size_t totalAllocs() const{ return total_allocs_; }

	bool isDestroyed() const{ return isDestroyed_; }

protected:
	/**
		����һ������
	*/
	void reclaimObject_(T* obj)
	{
		if(obj != NULL)
		{
			// ������״̬
			obj->onReclaimObject();

			if(size() >= max_ || isDestroyed_)
			{
				delete obj;
				--total_allocs_;
			}
			else
			{
				objects_.push_back(obj);
				++obj_count_;
			}
		}
	}

protected:
	OBJECTS objects_;

	size_t max_;

	bool isDestroyed_;

	// һЩԭ�����������б�Ҫ��
	// ���磺dbmgr�����߳������log��cellapp�м���navmesh����̻߳ص����µ�log���
	Mutex mutex_;

	std::string name_;

	size_t total_allocs_;

	// Linux�����У�list.size()ʹ�õ���std::distance(begin(), end())��ʽ�����
	// ���������Ӱ�죬���������Լ���size��һ����¼
	size_t obj_count_;
};

/*
	�ض��� ����ʹ�óصĶ������ʵ�ֻ��չ��ܡ�
*/
class PoolObject
{
public:
	virtual ~PoolObject(){}
	virtual void onReclaimObject() = 0;
	virtual void onEabledPoolObject(){}

	virtual size_t getPoolObjectBytes(){ return 0; }

	/**
		�ض�������ǰ��֪ͨ
		ĳЩ��������ڴ���һЩ����
	*/
	virtual bool destructorPoolObject()
	{
		return false;
	}
};

#endif