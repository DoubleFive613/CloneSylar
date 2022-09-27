//不可拷贝对象封装
#ifndef __SYLAR_NONCOPYABLE_H__
#define __SYLAR_NONCOPYABLE_H__

namespace sylar{
    //对象无法拷贝、赋值
    class Noncopyable
    {
    private:
        /* data */
    public:
        Noncopyable() = default;
        ~Noncopyable() = default;

        //拷贝构造函数禁用
        Noncopyable(const Noncopyable&) = delete;

        //赋值函数禁用
        Noncopyable& operator=(const Noncopyable&) = delete;
    };    
}

#endif