/* 
 * File:   broker.h
 * Author: Rohit Joshi <rohit.c.joshi@gmail.com>
 *
 * Created on February 26, 2015, 8:40 AM
 */

#ifndef BROKER_H
#define    BROKER_H

#include "thirdparty/readerwriterqueue.h"
#include "connection.h"
#include "connection_zmq.h"
#include "connection_file.h"
#include "utils.h"
#include "broker_config.h"
#include "producer.h"
#include "consumer.h"


namespace lightq {

  //broker
  class broker {
  public:


      /**
       * constructor
       * @param config
       */
      broker(broker_config &config) : config_(config), storage_(config) {

          LOG_IN("broker_config: %s", config.to_string().c_str());
          stop_ = false;
          p_producer_ = NULL;
          p_consumer_ = NULL;
          LOG_OUT("");
      }

      /**
       * destructor
       */
      ~broker() {
          LOG_IN("");
          delete p_producer_;
          delete p_consumer_;
          LOG_OUT("");
      }

      bool init() {
          LOG_IN("");
          storage_.init(config_);
          LOG_RET_TRUE("success");
      }

      bool init_consumer(consumer_config &consumer_config) {
          LOG_IN("");
          //initialize consumer
          p_consumer_ = new consumer(&storage_, consumer_config);
          if (!p_consumer_->init()) {
              LOG_RET_FALSE("Failed to initialize consumer");
          } else {
              LOG_DEBUG("Consumer initialized successfully");
          }
          storage_.set_consumer_socket(p_consumer_->get_consumer_socket());
          if (p_consumer_->run()) {
              LOG_RET_TRUE("success");
          } else {
              LOG_RET_FALSE("Failed to run consumer");
          }
      }

      bool init_producer(producer_config &prod_config) {
          LOG_IN("");

          //initialize producer
          p_producer_ = new producer(&storage_, prod_config);
          if (!p_producer_->init()) {
              LOG_RET_FALSE("Failed to initialize producer");
          } else {
              LOG_DEBUG("Producer initialized successfully");
          }
          if (p_producer_->run()) {
              LOG_RET_TRUE("success");
          } else {
              LOG_RET_FALSE("Failed to run producer");
          }
      }


      /**
       * Stop the broker
       * @return
       */
      bool stop() {
          LOG_IN("");
          stop_ = true;
          LOG_RET_TRUE("stopped");

      }

      inline uint32_t get_total_msg_sent() {
          return storage_.get_total_dequeued_messages();
      }

      inline uint32_t get_total_msg_received() {
          return storage_.get_total_enqueued_messages();
      }

      inline uint64_t get_queue_size() {
          return storage_.get_queue_size();
      }

      producer *get_producer() {
          return p_producer_;
      }

      consumer *get_consumer() {
          return p_consumer_;
      }

      broker_config &get_config() {
          return config_;
      }

      broker_storage &get_storage() {
          return storage_;
      }

  private:
      broker_config config_;
      bool stop_;
      broker_storage storage_;
      producer *p_producer_;
      consumer *p_consumer_;

  };
}

#endif	/* BROKER_H */

