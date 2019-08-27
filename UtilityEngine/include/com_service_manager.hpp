/**
* @file com_service_manager.hpp
* 服务管理
*
* @author hourui (liquidmonkey)
**/

#ifndef __COM_SERVICE_MANAGER_HPP__
#define __COM_SERVICE_MANAGER_HPP__

#include "logger.hpp"
#include "com_service.hpp"

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace com
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace iface
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class ServiceManager
{
public:
	enum class error
	{
		not_found = 0,
		duplicate_attach,
		duplicate_detach,
	};

	template<class T>
	struct refence
	{
		refence(void) :_data(nullptr) {}
		~refence(void) { clear(); }

		void clear(void) { delete _data; _data = nullptr; }
		void init(void) { T* t = new T; t->LoadFromDatabase(); _data = t; }

		T* _data;
	};
private:
	template<class T>
	static refence<T>* _refence(void)
	{
		static refence<T> res;
		return &res;
	}
public:
	template<class T>
	static T* GetService(void)
	{
		refence<T>* res = _refence<T>();
		if (res->_data)
			return res->_data;

		Clog::error("Service not found (%s)",T::ID());
		return nullptr;
	}

	template<class T>
	static void Attach(void)
	{
		refence<T>* res = _refence<T>();
		if (res->_data)
			Clog::error_throw_no(error::duplicate_attach, "Service duplicate attach (%s)", T::ID());

		res->init();
	}

	template<class T>
	static void Detach(void)
	{
		refence<T>* res = _refence<T>();
		if (!res->_data)
			Clog::error_throw_no(error::duplicate_detach, "Service duplicate detach (%s)", T::ID());

		res->clear();
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}// namespace iface
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace wrap
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class ServiceManager
{
public:
	enum class error
	{
		not_found = 0,
		duplicate_attach,
		duplicate_detach,
	};
private:
	template<typename T>
	struct refence
	{
		refence(void) :_data(nullptr) {}
		~refence(void) { clear(); }

		void clear(void) { delete _data; _data = nullptr; }

		Service<T>* _data;
	};

	template<typename T>
	static refence<T>* _refence()
	{
		static refence<T> res;
		return &res;
	}
private:
	template<typename T, typename... Args>
	struct _bind_function;

	template<typename T>
	struct _bind_function<T>
	{
		static void run(Service<T>* service){}
	};

	template<class T, class F, typename... Args>
	struct _bind_function<T, F, Args...>
	{
		static auto run(Service<T>* service, F&& f, Args&&... args)->decltype(std::bind(std::forward<F>(f), std::placeholders::_1, std::forward<Args>(args)...)((T*)(nullptr)))
		{
			return std::bind(std::forward<F>(f), std::placeholders::_1, std::forward<Args>(args)...)(service);
		}
	};

	template<typename T>
	struct guard
	{
		guard(refence<T>* res) :_res(res) {}
		~guard(void) { _res->clear(); }

		refence<T>* _res;
	};
public:
	template<typename T>
	static Service<T>* GetService(void)
	{
		refence<T>* res = _refence<T>();
		if (res->_data)
			return res->_data;
			
		Clog::error("Service not found (%s)", Service<T>::ID());
		return nullptr;
	}

	template<typename T, typename... Args>
	static auto Attach(Args&& ... args)
	{
		refence<T>* res = _refence<T>();
		if (res->_data)
			Clog::error_throw_no(error::duplicate_attach, "Service duplicate attach (%s)", Service<T>::ID());

		res->_data = new Service<T>;
		return _bind_function<T, Args...>::run(res->_data, std::forward<Args>(args)...);
	}

	template<typename T, typename... Args>
	static auto Detach(Args&& ... args)
	{
		refence<T>* res = _refence<T>();
		if (!res->_data)
			Clog::error_throw_no(error::duplicate_detach, "Service duplicate detach (%s)", Service<T>::ID());

		guard<T> _guard(res);
		return _bind_function<T, Args...>::run(res->_data, std::forward<Args>(args)...);
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}// namespace wrap
////////////////////////////////////////////////////////////////////////////////////////////////////
}// namespace com
////////////////////////////////////////////////////////////////////////////////////////////////////
}// namespace Utility
#endif // __COM_SERVICE_MANAGER_HPP__
