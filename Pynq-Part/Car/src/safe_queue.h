/*
 * @Author: Sauron Wu
 * @GitHub: wutianze
 * @Email: 1369130123qq@gmail.com
 * @Date: 2019-10-21 16:51:39
 * @LastEditors: Sauron Wu
 * @LastEditTime: 2019-10-22 09:48:21
 * @Description: 
 */
#include <thread>
#include <condition_variable>
#include <mutex>
#include <memory>
#include <queue>
using namespace std;
template<typename T>
class safe_queue
{
public:
    mutex m_mut;
    condition_variable m_cond;
    queue<T> m_queue;  


    safe_queue(){}
    safe_queue(const safe_queue& rhs)
    {
        lock_guard<mutex> lk(rhs.m_mut);
        m_queue = rhs;
    }
    int size(){
        lock_guard<mutex> lk(m_mut);
        return m_queue.size();
    }
    void push(T data)
    {
        lock_guard<mutex> lk(m_mut);  
        m_queue.push(move(data)); 
        m_cond.notify_one(); 
    }
    void wait_and_pop(T &res)  
    {
        unique_lock<mutex> lk(m_mut);
        m_cond.wait(lk,[this]{return !m_queue.empty();});
        res = move(m_queue.front());
        m_queue.pop();
    }

    bool try_pop(T &res) 
    {
        lock_guard<mutex> lk(m_mut);
        if(m_queue.empty())
            return false;
        res = move(m_queue.front());
	m_queue.pop();
        return true;
    }

    shared_ptr<T> wait_and_pop()
    {
        unique_lock<mutex> lk(m_mut);
        m_cond.wait(lk,[this]{return !m_queue.empty();});
        shared_ptr<T> res(make_shared<T>(move(m_queue.front())));
        m_queue.pop();
        return res;
    }

    shared_ptr<T> try_pop()
    {
        lock_guard<mutex> lk(m_mut);
        if(m_queue.empty())
            return NULL;
        shared_ptr<T> res(make_shared<T>(move(m_queue.front())));
        m_queue.pop();
        return res;
    }
 };
