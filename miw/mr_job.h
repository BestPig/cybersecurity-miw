/**
 * Copyright (c) 2015 SopraSteria
 * All rights reserved.
 * Author: Emmanuel Benazera <beniz@droidnik.fr>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of SopraSteria nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY SOPRASTERIA ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL SOPRASTERIA BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef MR_JOB_H
#define MR_JOB_H

#include <sys/mman.h>
#include <sched.h>
#include "application.hh"
#include "defsplitter.hh"
#include "bench.hh"
#ifdef JOS_USER
#include "wc-datafile.h"
#include <inc/sysprof.h>
#endif
#include "log_record.h"
#include "log_format.h"
#include <fstream>

enum { with_value_modifier = 0 };

using namespace miw;

class mr_job : public map_reduce
{
 public:
 mr_job(const char *f, int nsplit,
	const std::string &app_name,
	log_format *lf,
	const bool &store_content, const bool &compressed, const bool &quiet)
   : defs_(f, nsplit),_app_name(app_name),_lf(lf),_store_content(store_content),
    _compressed(compressed),_quiet(quiet)
  {}
 mr_job(char *d, const size_t &size, int nsplit,
	const std::string &app_name,
	log_format *lf,
	const bool &store_content, const bool &compressed, const bool &quiet)
   : defs_(d,size,nsplit),_app_name(app_name),_lf(lf),
    _store_content(store_content),_compressed(compressed),_quiet(quiet) {}
  virtual ~mr_job() {}
  
  void free_records(xarray<keyval_t> *wc_vals)
  {
    for (uint32_t i=0;i<wc_vals->size();i++)
      {
	log_record *lr = (log_record*)wc_vals->at(i)->val;
	delete lr;
      }
  }
  
  bool split(split_t *ma, int ncores) {
    return defs_.split(ma, ncores, "\n",0);
  }
  
  int key_compare(const void *s1, const void *s2) {
    return  strcasecmp((const char *) s1, (const char *) s2);
  }
  
  void run(const int &nprocs, const int &reduce_tasks,
	   const int &quiet, const std::string output_format, int &ndisp,
	   std::ofstream &fout)
  {
    std::cerr << "running mr_job\n";
    
    set_ncore(nprocs);
    set_reduce_task(reduce_tasks);
    sched_run();
    print_stats();
    
    /* get the number of results to display */
    //if (!quiet)
    print_top(&results_, ndisp);
    if (fout.is_open()) 
      {
	if (output_format == "json")
	  output_json(&results_,fout);
	else if (output_format == "csv")
	  output_csv(&results_,fout);
	else if (output_format.empty())
	  output_all(&results_,fout);
      }
    else if (output_format == "json")
      output_json(&results_,std::cout);
    free_records(&results_);
    free_results();
  }
  
  // output functions
  void print_top(xarray<keyval_t> *wc_vals, int &ndisp);
  void output_all(xarray<keyval_t> *wc_vals, std::ostream &fout);
  void output_json(xarray<keyval_t> *wc_vals, std::ostream &fout);
  void output_csv(xarray<keyval_t> *wc_vals, std::ostream &fout);
  
  // map reduce
  void map_function(split_t *ma);
  void reduce_function(void *key_in, void **vals_in, size_t vals_len);
  int combine_function(void *key_in, void **vals_in, size_t vals_len);
  
  void *modify_function(void *oldv, void *newv) {
    log_record *lr1 = (log_record*) oldv;
    log_record *lr2 = (log_record*) newv;
    //lr1->_sum += lr2->_sum;
    lr1->merge(lr2);
    delete lr2;
    return (void*)lr1;
   }
  
  void *key_copy(void *src, size_t s) {
    char *key = static_cast<char*>(src); // no need to strdup((char*)src);
    return key;
  }
  
  void key_free(void *k)
  {
    free(k);
  }
  
  int final_output_compare(const keyval_t *kv1, const keyval_t *kv2) {
#ifdef HADOOP
    return strcmp((char *) kv1->key_, (char *) kv2->key_);
#else
    /*if (alphanumeric)
      return strcmp((char *) kv1->key_, (char *) kv2->key_);*/
    size_t i1 = (size_t) kv1->val;
    size_t i2 = (size_t) kv2->val;
    if (i1 != i2)
      return i2 - i1;
    else
      return strcmp((char *) kv1->key_, (char *) kv2->key_);
#endif
  }
  
  bool has_value_modifier() const {
    return with_value_modifier;
  }
  
 private:
  defsplitter defs_;
  std::string _app_name;
  log_format *_lf;
  bool _store_content = false;
  bool _compressed = false;
  bool _quiet = false;
};

#endif