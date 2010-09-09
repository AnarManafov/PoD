/*
 *  local_types.h
 *  pod-ssh
 *
 *  Created by Anar Manafov on 26.08.10.
 *  Copyright 2010 Anar Manafov <Anar.Manafov@gmail.com>. All rights reserved.
 *
 */
#ifndef LOCAL_TYPES_H
#define LOCAL_TYPES_H
#include <boost/function.hpp>

typedef boost::function<void ( const std::string&, const std::string& )> log_func_t;

#endif

