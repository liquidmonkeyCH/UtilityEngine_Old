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
#include <type_traits>

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace com
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace _impl 
{
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T>
struct service_refence
{
	using param_t = typename std::conditional<std::is_base_of<iface::Service, T>::value, T, wrap::Service<T>>::type;

	service_refence(void) :_data(nullptr) {}
	~service_refence(void) { destory(); }

	void create(void) { _data = new param_t; }
	void destory(void) { delete _data; _data = nullptr; }

	param_t* _data;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace _impl
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
protected:
	template<typename T>
	using refence = _impl::service_refence<T>;

	template<typename T>
	static refence<T>* _refence()
	{
		static refence<T> res;
		return &res;
	}
protected:
	template<typename T, typename... Args>
	struct _bind_function;

	template<typename T>
	struct _bind_function<T>
	{
		static void run(T* service) {}
	};

	template<class T, class F, typename... Args>
	struct _bind_function<T, F, Args...>
	{
		static auto run(T* service, F&& f, Args&& ... args)->decltype(std::bind(std::forward<F>(f), std::placeholders::_1, std::forward<Args>(args)...)((T*)(nullptr)))
		{
			return std::bind(std::forward<F>(f), std::placeholders::_1, std::forward<Args>(args)...)(service);
		}
	};

	template<typename T>
	struct guard
	{
		guard(refence<T>* res) :_res(res) {}
		~guard(void) { _res->destory(); }

		refence<T>* _res;
	};
public:
	template<typename T>
	static auto GetService(void)->typename refence<T>::param_t*
	{
		refence<T>* res = _refence<T>();
		if (res->_data)
			return res->_data;

		Clog::error("Service not found (%s)", refence<T>::param_t::ID());
		return nullptr;
	}

	template<typename T, typename... Args>
	static auto Attach(Args&& ... args)
	{
		using param_t = typename refence<T>::param_t;
		refence<T>* res = _refence<T>();
		if (res->_data)
			Clog::error_throw_no(error::duplicate_attach, "Service duplicate attach (%s)", param_t::ID());

		res->create();
		return _bind_function<param_t, Args...>::run(res->_data, std::forward<Args>(args)...);
	}

	template<typename T, typename... Args>
	static auto Detach(Args&& ... args)
	{
		using param_t = typename refence<T>::param_t;
		refence<T>* res = _refence<T>();
		if (!res->_data)
			Clog::error_throw_no(error::duplicate_detach, "Service duplicate detach (%s)", param_t::ID());

		guard<T> _guard(res);
		return _bind_function<param_t, Args...>::run(res->_data, std::forward<Args>(args)...);
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}// namespace com
////////////////////////////////////////////////////////////////////////////////////////////////////
}// namespace Utility
#endif // __COM_SERVICE_MANAGER_HPP__
