/**
 * @file com_service.hpp
 * 服务基类
 *
 * @author Hourui (liquidmonkey)
 **/

#ifndef __COM_SERVICE_HPP__
#define __COM_SERVICE_HPP__

#include "base_defines.hpp"

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
	virtual void LoadFromDatabase(void) = 0;
};
////////////////////////////////////////////////////////////////////////////////
#define DECLARE_SERVICE_ID(serviceId)	\
public:									\
	static SERVICE_ID ID(void)			\
	{									\
		return (#serviceId);			\
	}									\
private:								\
	serviceId(void);					\
	~serviceId(void);					\
	void LoadFromDatabase(void);		\
	template<class T> friend struct Utility::com::iface::ServiceManager::refence;
////////////////////////////////////////////////////////////////////////////////
}// namespace iface
////////////////////////////////////////////////////////////////////////////////
namespace wrap
{
////////////////////////////////////////////////////////////////////////////////
template<typename T>
class Service : public T
{
	Service(void) = default;
	~Service(void) = default;

	Service(const Service&) = delete;
	Service& operator=(const Service&) = delete;

	Service(const Service&&) = delete;
	Service& operator=(const Service&&) = delete;

	friend class ServiceManager;
public:
	using SERVICE_ID = const char*;

	static SERVICE_ID ID(void)
	{
		return type_name<T>();
	}
};
////////////////////////////////////////////////////////////////////////////////
}// namspace wrap
////////////////////////////////////////////////////////////////////////////////
}// namespace com 
////////////////////////////////////////////////////////////////////////////////
}// namespace Utility
#endif //__COM_SERVICE_HPP__
