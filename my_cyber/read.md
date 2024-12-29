AllLatestd 的构造函数入参4个 ChannelBuffer类型的 buffer 是ChannelBuffer new后传递进来，data_fusion_用来DataVisitor融合需要的几个数据，融合时机是buffer_m0_数据到来后开始融合，AllLatestd类型的data_fusion_构造的时候会注册callack这个callback是融合时机是buffer_m0_数据到来后开始融合，融合需要的所有数据

每种数据类型都有一个管理数据到来的单列类DataDispatcher

ChannelBuffer类构造函数创建的4个获取数据到来的buffer，会添加到统一管理对应一种数据的单列类DataDispatcher，当对应数据到来后该单列类会将数据存放到添加的buffer类里面，这里比较特殊的是data_fusion_融合的时候不是将数据放到buffer而是调用data_fusion_构造时注册的callback

DataVisitor的变量notifier_是子类的变量，他的callback是scheduler.cc里CreateTask注册的visitor->RegisterNotifyCallback([this, task_id]() {
      if (cyber_unlikely(stop_.load())) {
        return;
      }
      this->NotifyProcessor(task_id);
    });

    DataDispatcher里面Dispatch函数调用 DataNotifier的Notify函数


ReceiverManager里面创建Receiver调用GetReceiver然后调用DataDispatcher的Dispatch

Reader的Init函数会调用单列类ReceiverManager去调用Transport单列类创建Receiver然后存放到map里面，每种数据都有一个管理的ReceiverManager单列(可能是所有的共用一个单列)，创建Receiver时会传入一个函数，传入的在个函数会调用该种数据的DataDispatcher的Dispatch函数，传入的函数会保存在Receiver基类里面的msg_listener_函数，然后在OnNewMessage函数里面调用。

Receiver分为ShmReceiver、RtpsReceiver、IntraReceiver、HybridReceiver几种，他们都会继承一个Receiver类，ShmReceiver、RtpsReceiver、IntraReceiver里面的Enable函数会bind注测的OnNewMessage函数

这里以IntraDispatcher模式为列子其他的类似，IntraDispatcher单列调用AddListener函数，该函数的入参是一个函数，这里传入的是OnNewMessage函数，AddListener函数里面有一个ChannelChain类成员变量chain_(只有IntraDispatcher会使用，存放所有数据类型的然后再分开处理)的成员对象然后会调用AddListener函数根据不同的数据类型存放注册的回调函数。不同数据类型的回调函数都存放到这个map里面了oppo_handlers_

然后IntraDispatcher 类的AddListener函数会调用GetHandler(获取的handle是IntraDispatcher继承父类Dispatcher的成员变量(发送方的，线程通信模式，直接将接收数据的回调注册到发送方)存放的map里面msg_listeners_有对应的handle)，根据channel_id获取handle然后将listener_wrapper函数与该channel_id的handle关联起来，然后当发送方发送数据时就会调用注册的listener_wrapper函数。然后就会去调用ChannelChain类成员变量chain_的Run函数分别根据到来数据的类型调用之前用户注册到oppo_handlers_的不同的回调和处理数据。





Reader<MessageT>::Init()函数里面std::make_shared<data::DataVisitor<MessageT>>将创建获取用户数据的DataVisitor。后续注册的回调触发就是和DataVisitor有关，上面描述的就是在个流程。
croutine::CreateRoutineFactory<MessageT>(std::move(func), dv);创建任务协程将用户的回调函数任务与获取用户数据的DataVisitor关联起来，scheduler::Instance()单例的Scheduler::CreateTask会注册事件回调，将用户的任务协程注册进去。








加载模块:
在C++程序中，main 函数是程序的标准入口点。也就是说，当程序开始执行时，main 函数通常是第一个被调用的函数。然而，在某些情况下，可以在 main 函数执行之前运行一些代码。这通常通过以下几种方式实现：

    ‌全局对象的构造‌：
    在 C++ 中，全局对象和静态对象在 main 函数执行之前就被构造了。如果这些对象有构造函数，那么这些构造函数将在 main 函数之前执行。

    ‌初始化静态变量‌：
    类似于全局对象，静态变量的初始化也是在 main 函数之前完成的。

    编译器特定的扩展‌：
某些编译器提供了在 main 函数之前执行代码的特定扩展。例如，GCC 提供了 __attribute__((constructor)) 来标记在 main 之前需要执行的函数。



CYBER_REGISTER_COMPONENT的调用会先初始化指向的对象和类型，开始都是空,这些初始化是在main函数执行前进行的，后面main函数执行会对该变量赋值




‌Cyber RT中的GlobalData是一个重要的组件，它通常用于存储全局数据，供系统中的各个模块或组件共享和访问‌。

在Cyber RT的架构中，GlobalData可能包含了一些关键的系统级信息，比如注册的节点、通道、服务以及任务名称等‌
这些信息对于系统的正常运行和各个模块之间的通信至关重要。通过GlobalData，系统中的不同部分可以方便地获取到所需的全局数据，从而实现高效的数据共享和交互。

例如，在多线程环境下，各个线程可能需要访问共同的资源或数据。此时，GlobalData就提供了一个集中的存储位置，避免了数据分散在不同线程中导致的访问困难和数据不一致问题。然而，也需要注意到在多线程环境中对GlobalData的访问需要进行适当的同步和互斥控制，以避免数据竞争和死锁等问题的发生‌

总的来说，GlobalData在Cyber RT中扮演着重要的角色，它提供了全局数据的存储和访问机制，支持了系统中的数据共享和交互。对于理解和使用Cyber RT来说，了解GlobalData的作用和功能是非常有帮助的。

