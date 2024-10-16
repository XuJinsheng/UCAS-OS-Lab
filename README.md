TODO：请在此完成你自己项目的“说明书”。

Thread：生存周期
退出后不析构，status=dead，保证其他引用还有效
别人用Share_ptr引用，一般不会析构，引用时需要检查status
thread 被kill时，thread_table不清空，内核栈不释放，析构时才释放

Object：
自带计数器
用到的thread+1，thread close之后计数器-1，为0再析构
自身close也不析构，仅标记status
析构时删除table

实现要求：
init：查询key table，检查是否存在，不存在则创建并扔到table中否则从table中拿，thread-register



实现condition和barrier
