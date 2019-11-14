/**
 * @file com_service.hpp
 * 服务基类
 *
 * @author Hourui (liquidmonkey)
 **/

#ifndef __COM_SERVICE_HPP__
#define __COM_SERVICE_HPP__

#include "com_typename.hpp"
#include "com_service_manager.hpp"

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////
namespace com
{
////////////////////////////////////////////////////////////////////////////////
namespace iface
{
class Service
{
public:
	using SERVICE_ID = const char*;
public:
	virtual SERVICE_ID GetId(void) = 0;
};
////////////////////////////////////////////////////////////////////////////////
#define DECLARE_SERVICE_ID(serviceId)	\
public:									\
	SERVICE_ID GetId(void)				\
	{									\
		return (#serviceId);			\
	}									\
	static SERVICE_ID ID(void)			\
	{									\
		return (#serviceId);			\
	}									\
private:								\
	serviceId(void);					\
	~serviceId(void);					\
	template<class> friend struct Utility::_impl::service_refence;
////////////////////////////////////////////////////////////////////////////////
}// namespace iface
////////////////////////////////////////////////////////////////////////////////
namespace wrap
{
////////////////////////////////////////////////////////////////////////////////
template<typename T>
class Service : public T
{
	Service(void){};
	~Service(void) {};

	Service(const Service&) = delete;
	Service& operator=(const Service&) = delete;

	Service(const Service&&) = delete;
	Service& operator=(const Service&&) = delete;
public:
	template<class> friend struct Utility::_impl::service_refence;
	using SERVICE_ID = const char*;

	static SERVICE_ID ID(void)
	{
		return com::type_name<Service<T>>();
	}
};
////////////////////////////////////////////////////////////////////////////////
}// namspace wrap
////////////////////////////////////////////////////////////////////////////////
}// namespace com 
////////////////////////////////////////////////////////////////////////////////
}// namespace Utility
#endif //__COM_SERVICE_HPP__
