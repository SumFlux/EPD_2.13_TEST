#include "Core/EventQueue.h"

EventQueue::EventQueue()
    : _head(0), _tail(0), _count(0) {
}

bool EventQueue::push(const Event& event) {
    if (isFull()) {
        Serial.println("[WARN] EventQueue is full, dropping event");
        return false;
    }

    _buffer[_tail] = event;
    _tail = (_tail + 1) % CAPACITY;
    _count++;

    return true;
}

bool EventQueue::pop(Event& event) {
    if (isEmpty()) {
        return false;
    }

    event = _buffer[_head];
    _head = (_head + 1) % CAPACITY;
    _count--;

    return true;
}

bool EventQueue::peek(Event& event) const {
    if (isEmpty()) {
        return false;
    }

    event = _buffer[_head];
    return true;
}

bool EventQueue::isEmpty() const {
    return _count == 0;
}

bool EventQueue::isFull() const {
    return _count >= CAPACITY;
}

size_t EventQueue::size() const {
    return _count;
}

void EventQueue::clear() {
    _head = 0;
    _tail = 0;
    _count = 0;
}
