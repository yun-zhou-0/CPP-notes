#if 0
#include <new>
#define __THROW_BAD_ALLOC throw bad_alloc
#elif !defined(__THROW_BAD_ALLOC)

#include <iostream.h>
#define __THROW_BAD_ALLOC                                                      \
  cerr << "out of memory" << endl;                                             \
  exit(1)
#endif

// malloc-based allocator. 通常比稍後介紹的 default alloc 速度慢，
// 一般而言是 thread-safe，並且對於空間的運用比較高效（efficient）。
// 以下是第一級配置器。
// 注意，無「template型別參數」。至於「非型別參數」inst，完全沒派上用場。
template <int inst> class __malloc_alloc_template {

private:
  // 以下都是函式指標，所代表的函式將用來處理記憶體不足的情況。
  // oom : out of memory.
  static void *oom_malloc(size_t);
  static void *oom_realloc(void *, size_t);
  static void (*__malloc_alloc_oom_handler)();

public:
  static void *allocate(size_t n) {
    void *result = malloc(n);
    //  第一级配置器直接使用malloc，内存不足时使用oom_malloc
    if (0 == result)
      result = oom_malloc(n);
    return result;
  }

  static void deallocate(void *p, size_t /* n */) {
    free(p);
  } //  第一级配置器直接使用free

  static void *reallocate(void *p, size_t /* old_sz */, size_t new_sz) {
    void *result = realloc(p, new_sz);
    //  第一级配置器直接使用realloc，内存不足时使用oom_realloc
    if (0 == result)
      result = oom_realloc(p, new_sz);
    return result;
  }
  // 以下模擬 C++ 的 set_new_handler(). 換句話說，你可以透過它，指定你自己的
  // out-of-memory handler
  static void (*set_malloc_handler(void (*f)()))() {
    void (*old)() = __malloc_alloc_oom_handler;
    __malloc_alloc_oom_handler = f;
    return (old);
  }
};

// malloc_alloc out-of-memory handling

#ifndef __STL_STATIC_TEMPLATE_MEMBER_BUG
template <int inst>
void (*__malloc_alloc_template<inst>::__malloc_alloc_oom_handler)() = 0;
#endif

template <int inst> void *__malloc_alloc_template<inst>::oom_malloc(size_t n) {
  void (*my_malloc_handler)();
  void *result;

  for (;;) { // 不断尝试释放，配置的过程
    my_malloc_handler = __malloc_alloc_oom_handler;
    if (0 == my_malloc_handler) {
      __THROW_BAD_ALLOC; // 如果“内存不足处理例程”并没有实现，则会直接抛出异常或者中止程序
    }
    (*my_malloc_handler)(); // 调用处理进程，企图释放内存
    result = malloc(n);     // 再次尝试释放内存
    if (result)
      return (result);
  }
}

template <int inst>
void *__malloc_alloc_template<inst>::oom_realloc(void *p, size_t n) {
  void (*my_malloc_handler)();
  void *result;

  for (;;) {
    my_malloc_handler = __malloc_alloc_oom_handler;
    if (0 == my_malloc_handler) {
      __THROW_BAD_ALLOC;
    }
    (*my_malloc_handler)();
    result = realloc(p, n);
    if (result)
      return (result);
  }
}

typedef __malloc_alloc_template<0> malloc_alloc;