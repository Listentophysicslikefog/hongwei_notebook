// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef MUDUO_BASE_MUTEX_H
#define MUDUO_BASE_MUTEX_H

#include "muduo/base/CurrentThread.h"
#include "muduo/base/noncopyable.h"
#include <assert.h>
#include <pthread.h>

// Thread safety annotations {
// https://clang.llvm.org/docs/ThreadSafetyAnalysis.html

// Enable thread safety attributes only with clang.
// The attributes can be safely erased when compiling with other compilers.
#if defined(__clang__) && (!defined(SWIG))
#define THREAD_ANNOTATION_ATTRIBUTE__(x)   __attribute__((x))
#else
#define THREAD_ANNOTATION_ATTRIBUTE__(x)   // no-op
#endif

#define CAPABILITY(x) \
  THREAD_ANNOTATION_ATTRIBUTE__(capability(x))

#define SCOPED_CAPABILITY \
  THREAD_ANNOTATION_ATTRIBUTE__(scoped_lockable)

#define GUARDED_BY(x) \
  THREAD_ANNOTATION_ATTRIBUTE__(guarded_by(x))

#define PT_GUARDED_BY(x) \
  THREAD_ANNOTATION_ATTRIBUTE__(pt_guarded_by(x))

#define ACQUIRED_BEFORE(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(acquired_before(__VA_ARGS__))

#define ACQUIRED_AFTER(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(acquired_after(__VA_ARGS__))

#define REQUIRES(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(requires_capability(__VA_ARGS__))

#define REQUIRES_SHARED(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(requires_shared_capability(__VA_ARGS__))

#define ACQUIRE(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(acquire_capability(__VA_ARGS__))

#define ACQUIRE_SHARED(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(acquire_shared_capability(__VA_ARGS__))

#define RELEASE(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(release_capability(__VA_ARGS__))

#define RELEASE_SHARED(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(release_shared_capability(__VA_ARGS__))

#define TRY_ACQUIRE(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(try_acquire_capability(__VA_ARGS__))

#define TRY_ACQUIRE_SHARED(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(try_acquire_shared_capability(__VA_ARGS__))

#define EXCLUDES(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(locks_excluded(__VA_ARGS__))

#define ASSERT_CAPABILITY(x) \
  THREAD_ANNOTATION_ATTRIBUTE__(assert_capability(x))

#define ASSERT_SHARED_CAPABILITY(x) \
  THREAD_ANNOTATION_ATTRIBUTE__(assert_shared_capability(x))

#define RETURN_CAPABILITY(x) \
  THREAD_ANNOTATION_ATTRIBUTE__(lock_returned(x))

#define NO_THREAD_SAFETY_ANALYSIS \
  THREAD_ANNOTATION_ATTRIBUTE__(no_thread_safety_analysis)

// End of thread safety annotations }

#ifdef CHECK_PTHREAD_RETURN_VALUE

#ifdef NDEBUG
__BEGIN_DECLS
extern void __assert_perror_fail (int errnum,
                                  const char *file,
                                  unsigned int line,
                                  const char *function)
    noexcept __attribute__ ((__noreturn__));
__END_DECLS
#endif

#define MCHECK(ret) ({ __typeof__ (ret) errnum = (ret);         \
                       if (__builtin_expect(errnum != 0, 0))    \
                         __assert_perror_fail (errnum, __FILE__, __LINE__, __func__);})

#else  // CHECK_PTHREAD_RETURN_VALUE

#define MCHECK(ret) ({ __typeof__ (ret) errnum = (ret);         \
                       assert(errnum == 0); (void) errnum;})

#endif // CHECK_PTHREAD_RETURN_VALUE

namespace muduo
{

// Use as data member of a class, eg.
//
// class Foo
// {
//  public:
//   int size() const;
//
//  private:
//   mutable MutexLock mutex_;
//   std::vector<int> data_ GUARDED_BY(mutex_);
// };
class CAPABILITY("mutex") MutexLock : noncopyable
{
 public:
  MutexLock()   //构造函数
    : holder_(0)   //将拥有该锁的线程id初始化为0，表示该锁没有被任何线程拥有
  {
    MCHECK(pthread_mutex_init(&mutex_, NULL));  //初始化该锁
  }

  ~MutexLock()
  {
    assert(holder_ == 0);  //析构函数中断言该锁没有被任何线程拥有才可以销毁该锁
    MCHECK(pthread_mutex_destroy(&mutex_));   //销毁该锁
  }

  // must be called when locked, i.e. for assertion
  bool isLockedByThisThread() const      //判断是否当前线程拥有该互斥锁
  {
    return holder_ == CurrentThread::tid();   //只需判断holder_是否等于当前线程的tid
  }

  void assertLocked() const ASSERT_CAPABILITY(this)    //断言当前线程拥有该锁
  {
    assert(isLockedByThisThread());
  }

  // internal usage

  void lock() ACQUIRE()   //加锁
  {
    MCHECK(pthread_mutex_lock(&mutex_));  //加锁
    assignHolder();   //将获该锁的线程id保存到 holder_
  }

  void unlock() RELEASE()  //解锁
  {
    unassignHolder(); //将holder_赋值为0
    MCHECK(pthread_mutex_unlock(&mutex_));  //解锁
  }

  pthread_mutex_t* getPthreadMutex() /* non-const */    //获取pthreadMutex对象
  {
    return &mutex_;
  }

 private:
  friend class Condition;

  class UnassignGuard : noncopyable
  {
   public:
    explicit UnassignGuard(MutexLock& owner)
      : owner_(owner)
    {
      owner_.unassignHolder();
    }

    ~UnassignGuard()
    {
      owner_.assignHolder();
    }

   private:
    MutexLock& owner_;
  };

  void unassignHolder()
  {
    holder_ = 0;
  }

  void assignHolder()
  {
    holder_ = CurrentThread::tid();
  }

  pthread_mutex_t mutex_;   //定义互斥锁，保存定义的互斥对象
  pid_t holder_;   //当前拥有这把锁的线程id
};

// Use as a stack variable, eg.
// int Foo::size() const
// {
//   MutexLockGuard lock(mutex_);
//   return data_.size();
// }
class SCOPED_CAPABILITY MutexLockGuard : noncopyable   //就是将mutex的加锁和解锁封装为类，避免自己调用时提前返回结果但是锁还没有释放
{                                               //封装为类时析构函数会自动释放
 public:
  explicit MutexLockGuard(MutexLock& mutex) ACQUIRE(mutex)  //引用MutexLock，对象的生存期不受MutexLockGuard管理，MutexLockGuard只是管理锁
    : mutex_(mutex)   //没有释放mutex对象
  {
    mutex_.lock();
  }

  ~MutexLockGuard() RELEASE()
  {
    mutex_.unlock();
  }

 private:

  MutexLock& mutex_;
};

}  // namespace muduo

// Prevent misuse like:
// MutexLockGuard(mutex_);   //错误的
// A tempory object doesn't hold the lock for long!，临时对象不可以长时间拥有该锁
#define MutexLockGuard(x) error "Missing guard object name"   //避免我们错误使用，直接将这种使用定义为后面字符串方便发现问题

#endif  // MUDUO_BASE_MUTEX_H
