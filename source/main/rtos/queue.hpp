/**
 * @file Queue.h
 * @copyright (c) 2007-2015 Richard Damon
 * @author Richard Damon <richard.damon@gmail.com>
 * @parblock
 * MIT License:
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * It is requested (but not required by license) that any bugs found or
 * improvements made be shared, preferably to the author.
 * @endparblock
 *
 * @brief FreeRTOS Queue Wrapper
 *
 * This file contains a set of lightweight wrappers for queues using FreeRTOS
 *
 * @ingroup FreeRTOSCpp
 * @note Some functions have been removed since they are not used
 */
#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

/**
 * @brief Base Queue Wrapper
 *
 * This Base Class provides the Type Independent functionality for a Queue
 */
class queue_base {
  private:
  queue_base(queue_base const&) = delete;      ///< We are not copyable
  void operator=(queue_base const&) = delete; ///< We are not assignable
  protected:
  QueueHandle_t queue_handle;

  /**
   * @brief Constructor
   *
   * Effectively Abstract class so protected base.
   *
   * @param handle The queueHandle for the queue
   */
  queue_base(QueueHandle_t handle)
    : queue_handle(handle){};

  public:
  /**
   * @brief Destructor
   */
  virtual ~queue_base() {
    vQueueDelete(queue_handle);
  }

  /**
   * @brief Get number of items in the Queue
   *
   * @return The number of item in the Queue
   */
  unsigned waiting() const {
    return uxQueueMessagesWaiting(queue_handle);
  }

  /**
   * @brief Return number of spaces available in Queue
   *
   * @return the number of spaces available in the Queue
   */
  unsigned available() const {
    return uxQueueSpacesAvailable(queue_handle);
  }

  /**
   * @brief Reset the Queue
   *
   * Resets the Queue to an empty state.
   */
  void reset() {
    xQueueReset(queue_handle);
  }

  /**
   * @brief Check if Queue is Full
   *
   * @return True if Queue is Full
   */
  bool full() {
    return 0 == uxQueueSpacesAvailable(queue_handle);
  }

  /**
   * @brief Check if Queue is Empty
   *
   * @return True if Queue is Empty
   */
  bool empty() {
    return uxQueueMessagesWaiting(queue_handle) == 0;
  }
};

/**
 * @brief Typed Queue Wrapper
 *
 * This Base Class provides the Type Dependent functionality for a Queue
 *
 * @tparam T The type of object to be placed on the queue
 */
template<class T>
class queue_type_base : public queue_base {
  protected:
  queue_type_base(QueueHandle_t handle)
    : queue_base(handle) {
  }

  public:
  /**
   * @brief Push an item onto the Queue
   *
   * Puts an item onto the Queue so it will be the next item to remove.
   *
   * @param item The item to put on the Queue
   * @param time How long to wait for room if Queue is full
   *
   * @return True if successful
   */
  bool push(const T& item, TickType_t time = portMAX_DELAY) {
    return xQueueSendToFront(queue_handle, &item, time);
  }

  /**
   * @brief Add an item at end of the Queue
   *
   * Puts an item onto the Queue so it will be the last item to remove.
   *
   * @param item The item to put on the Queue
   * @param time How long to wait for room if Queue is full
   *
   * @return True if successful
   */
  bool add(const T& item, TickType_t time = portMAX_DELAY) {
    return xQueueSendToBack(queue_handle, &item, time);
  }

  /**
   * @brief Get an item from the Queue
   *
   * Gets the first item from the Queue
   *
   * @param item Variable to place the item
   * @param time How long to wait for an item to be available
   *
   * @return True if an item returned
   */
  bool pop(T& item, TickType_t time = portMAX_DELAY) {
    return xQueueReceive(queue_handle, &item, time);
  }

  /**
   * @brief Look at the first item in the Queue
   *
   * Gets the first item from the Queue leaving it there.
   *
   * @param item Variable to place the item
   * @param time How long to wait for an item to be available
   *
   * @return True if an item returned
   */
  bool peek(T& item, TickType_t time = 0) {
    return xQueuePeek(queue_handle, &item, time);
  }
};

/**
 * @brief Queue Wrapper
 *
 * Note, is a template on the type of object to place on the queue,
 * which makes the Queue more typesafe.
 *
 * @tparam T The type of object to be placed on the queue
 *         Note also, this type needs to be trivially copyable, and preferably a POD
 *         as the FreeRTOS queue code will copy it with memcpy().
 * @tparam queuelength The number of elements to reserve space for in the queue
 *         If 0 (which is the default value) then length will be provided to the constructor dynamically.
 */
template<class T, unsigned queue_length>
class queue : public queue_type_base<T> {
  public:
  /**
   * @brief Constructor.
   *
   * @param name The name to register the Queue with.
   */
  queue(const char* name = 0)
    : queue_type_base<T>(xQueueCreate(queue_length, sizeof(T))){};
};
