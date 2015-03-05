/* 
 * File:   broker.h
 * Author: Rohit Joshi <rohit.c.joshi@gmail.com>
 *
 * Created on February 26, 2015, 8:40 AM
 */

#ifndef BROKER_H
#define	BROKER_H

#include "thirdparty/readerwriterqueue.h"
#include "connection.h"
#include "connection_zmq.h"
#include "connection_file.h"
#include "utils.h"
#include "broker_config.h"
#include "producer.h"
#include "consumer.h"


namespace prakashq {
    
    //broker
    class broker {
    public:
       

       /**
        * constructor
        * @param config
        */
        broker(broker_config& config) : storage_(config), config_(config){

            LOG_IN("broker_config: %s", config.to_string().c_str());
            stop_ = false;
            LOG_OUT("");
        }

        /**
         * destructor
         */
        ~broker() {
            LOG_IN("");
            if(p_producer_) {
                delete p_producer_;
                p_producer_= NULL;
            }
            if(p_consumer_) {
                delete p_consumer_;
                p_consumer_= NULL;
            }
            LOG_OUT("");
        }

        bool init() {
            LOG_IN("");
            
            storage_.init(config_);        
            //initialize producer
            p_producer_ = new producer(&storage_, config_);
            if(!p_producer_->init()) {
                LOG_RET_FALSE("Failed to initialize producer");
            }
            //initialize consumer
            p_consumer_ = new consumer(&storage_,config_);
            if(!p_consumer_->init()) {
               LOG_RET_FALSE("Failed to initialize consumer"); 
            }
                    
            LOG_RET_TRUE("success");
        }

        /**
         * run
         * @return 
         */
        bool run() {
            LOG_IN("");
            p_producer_->run();
            p_consumer_->run();
            LOG_RET_TRUE("done");
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
            return storage_.get_total_msg_sent();
        }
         inline uint32_t get_total_msg_received() {
            return storage_.get_total_msg_received();
        }
         
         inline uint64_t get_queue_size() {
             return storage_.get_queue_size();
         }

       
        broker_config config_;
    private:
        
        bool stop_;
        broker_storage storage_;
        producer *p_producer_;
        consumer *p_consumer_;

    };
}

#endif	/* BROKER_H */

