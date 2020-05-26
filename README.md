# mqtt_tools
cpp mqtt 测试工具及相关的库封装使用


主要实现功能:

1. 连接broker
2. 发布订阅主题
3. 能够发送文件(假设连接时有心跳包的话,建议关闭掉,防止客户端在传输过程中,因为心跳包的问题断开连接.)


```
设置构建目录为 :xxx\mqtt_tools\obj
如 : C:\code\fjs.com\mqtt_tools\obj

```

doc 下的目录 

[简介](doc%2F01_%E7%AE%80%E4%BB%8B.md)


```
使用时以c语言模式打开

#ifdef __cplusplus
extern "C" {
#endif
    #include <mqtt/MQTTAsync.h>
	#include "eclipse_paho_mqtt_c_win32/include/mqtt/Thread.h"
#ifdef __cplusplus
}
#endif

关于多线程模式,

thread_return_type WINAPI Run(void* n)
{
	xxxx....
    return 0;
}

 mqtt自带的封装线程
 xxxx myobja;
 Thread_start(&Run, (void*)myobja);

c++11 
 m_t = new std::thread(&CThreadRecv::run, myobja);
 m_t->detach();
 
 

```
