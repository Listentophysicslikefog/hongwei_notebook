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

Reader的Init函数会调用单列类ReceiverManager去调用Transport单列类CreateReceiver函数(入参第二个是函数类型的Receiver<M>类的这种函数类型MessageListener)创建Receiver然后存放到map里面，每种数据都有一个管理的ReceiverManager单列(可能是所有的共用一个单列)，创建Receiver时会传入一个函数，传入的在个函数会调用该种数据的DataDispatcher的Dispatch函数，传入的函数会保存在Receiver基类里面的msg_listener_函数，然后在OnNewMessage函数里面调用。

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


怎么判断使用那种通信模式(线程间、进程间)？？，应该是一个进程mainboard加载dag文件后可以知到当前进程对应topic是否有当前进程收，如果有可以使用线程间通信,但所还有一个问题，如果这个topic既有线程间通信又有进程间通信呢？？？？？？？



DataVisitor构造的时候会传入buffer，然后传入的buffer会添加到对应数据类型的DataDispatcher的管理buffer的map里面，数据来后会将数据放到该buffer里面。

ReceiverManager的GetReceiver函数里面调用的CreateReceiver函数第二个参数是注册的回调函数，在个回调函数里面DataDispatcher<MessageT>::Instance()->Dispatch会去调用对应的数据接收方的buffer的Fill函数(如果该buffer注册了需要融合多个数据的callback那么会调用该callback进行数据融合，数据是从DataVisitor构造的时候会传入buffer里面获取)
AllLatest是存放数据的类，第一个参数的数据类型的数据到来时就会进行融合一次



CreateReader可以传入收数据的回调函数也可以不传，不传时通过创建CreateReader时的，需要再详细看一下



Component里面的用户协程任务是什么时候触发调用的：？？？？？

DataVisitor里面data_notifier_->AddNotifier应该会将buffer_m0_第一个数据到来和notifier_（是一个callback函数）关联起来，然后数据到来时DataDispatcher会调用该Notify函数然后唤醒协程任务。

然后Component里面Initialize时会调用sched->CreateTask，这里会给DataVisitor里面notifier_回调函数赋值为当前的回调，这个回调应该就是触发注册协程的任务，如果Processor Run函数等待在Wait（context_的waite）或者wait_for就会被唤醒(是context_唤醒的)去获取协程任务执行
visitor->RegisterNotifyCallback([this, task_id]() {
      if (cyber_unlikely(stop_.load())) {
        return;
      }
      this->NotifyProcessor(task_id);
    });



    CreateRoutineFactory里面会去使用关联的DataVisitor去获取融合的数据，先更改协程的状态为等待io状态，如果没有获取到就yield






    获取协程任务Acquire能不能运行 lock_.test_and_set(std::memory_order_acquire);如果可以才返回该协程 ??这里需要再确认一下，怎么获取有效协程任务的。？？

    没有数据到来后执行协程任务的线程会一直运行消耗cpu吗? 缓存的消息会被使用完吗，不会导致部分数据处理晚吗(比如一瞬间来了很多数据，但是处理不及时，这样NotifyProcessor函数然后调用updated_.clear(std::memory_order_release)调用完成后，还有很多数据没有处理，那么后面没有NotifyProcessor后还没有处理的数据还会处理吗，会的)。？因为在协程执行晚任务后会更改为状态READY状态CRoutine::Yield(RoutineState::READY);这样就可以继续处理没有处理完的数据了。在CreateRoutineFactory里面的注册的回调函数里面。


    不会一直占有cpu， 当数据到来时会调用NotifyProcessor函数，该函数里面会调用事件到来需要触发的协程任务的SetUpdateFlag()函数updated_.clear(std::memory_order_release)，清理updated_将其值更改为false，这样获取有效协程的时候该协程序才可以被获取到(ClassicContext::NextRoutine())，获取到后如果没有数据到来触发就不会再获取到该协程了，因为(ClassicContext::NextRoutine()获取函数里面会调用cr->UpdateState()然后会调用updated_.test_and_set(std::memory_order_release），这样第二次就不会进入该逻辑将state_ 更改为 RoutineState::READY)，但是如果当前协程的状态就是READY那么还是会返回该协程然后运行该协程。

    在协程执行晚任务后会更改为状态READY状态CRoutine::Yield(RoutineState::READY);，这样下次还会执行该协程任务，直到所有的数据都处理完后协程CRoutine::Yield(); wait在DATA_WAIT，直到下次数据到来触发NotifyProcessor函数然后调用updated_.clear(std::memory_order_release)，清理updated_将其值更改为false，这样这个协程又可以被获取到然后运行了。







获取协程任务：
？？？？？？？
SchedulerChoreography的NotifyProcessor被触发后获取对应的协程，如果状态为DATA_WAIT或者IO_WAIT时，就会调用SetUpdateFlag清理updated_的状态将状态更改为false(这样是为了获取协程的时候状态为DATA_WAIT或者IO_WAIT时将状态更改为READY，这样获取有效协程时就 会  返回该协程，这个状态的更改在协程执行的时候更改的，就是CreateRoutineFactory里面调用函数CRoutine::GetCurrentRoutine()->set_state(RoutineState::DATA_WAIT)).


获取有效协程是Processor里面Run函数调用对应context的NextRoutine函数，这里会遍历对应context的所有协程 ，然后这里会调用UpdateState函数(函数里面updated_.test_and_set为true时，会判断state 如果state 为DATA_WAIT或者IO_WAIT会更改为READY然后返回协程)








怎么判断使用那一种通信模式的？？

