#pragma once

template<typename T>
class TSingleton
{
public:
	TSingleton(TSingleton&&) = delete;
	TSingleton(const TSingleton&) = delete;
	TSingleton& operator=(TSingleton&&) = delete;
	TSingleton& operator=(const TSingleton) = delete;

	static T& Instance()
	{
		static T instance;
		return instance;
	}

protected:
	TSingleton() {}
};
