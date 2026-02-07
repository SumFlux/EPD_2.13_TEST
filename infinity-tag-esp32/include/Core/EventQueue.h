#ifndef EVENT_QUEUE_H
#define EVENT_QUEUE_H

#include "Event.h"

/**
 * @brief 环形缓冲区事件队列
 *
 * 非阻塞的事件队列实现，用于事件的生产和消费
 * 容量：32个事件
 */
class EventQueue {
public:
    EventQueue();

    /**
     * @brief 将事件加入队列
     * @param event 要加入的事件
     * @return true 成功，false 队列已满
     */
    bool push(const Event& event);

    /**
     * @brief 从队列中取出事件
     * @param event 输出参数，存储取出的事件
     * @return true 成功，false 队列为空
     */
    bool pop(Event& event);

    /**
     * @brief 查看队列头部事件（不移除）
     * @param event 输出参数，存储队列头部的事件
     * @return true 成功，false 队列为空
     */
    bool peek(Event& event) const;

    /**
     * @brief 检查队列是否为空
     * @return true 队列为空，false 队列非空
     */
    bool isEmpty() const;

    /**
     * @brief 检查队列是否已满
     * @return true 队列已满，false 队列未满
     */
    bool isFull() const;

    /**
     * @brief 获取队列中的事件数量
     * @return 事件数量
     */
    size_t size() const;

    /**
     * @brief 清空队列
     */
    void clear();

private:
    static const size_t CAPACITY = 32;  // 队列容量
    Event _buffer[CAPACITY];            // 环形缓冲区
    size_t _head;                       // 队列头部索引
    size_t _tail;                       // 队列尾部索引
    size_t _count;                      // 当前事件数量
};

#endif // EVENT_QUEUE_H
