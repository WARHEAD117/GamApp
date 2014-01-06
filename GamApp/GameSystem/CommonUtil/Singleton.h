#pragma once

template<class T>
class CSingleton
{
public:
	static T& Instance();

private:
	CSingleton(void){};
	~CSingleton(void){};

	static T* m_pInstance;
	static void DestorySingleton();
};

template<class T>
T* CSingleton<T>::m_pInstance = NULL;

template<class T>
T& CSingleton<T>::Instance()
{
	if (m_pInstance == NULL)
	{
		m_pInstance = new T();
		atexit(CSingleton<T>::DestorySingleton);
	}
	return *m_pInstance;
}

template<class T>
void CSingleton<T>::DestorySingleton()
{
	if (m_pInstance != NULL)
	{
		delete m_pInstance;
		m_pInstance = NULL;
	}
}
