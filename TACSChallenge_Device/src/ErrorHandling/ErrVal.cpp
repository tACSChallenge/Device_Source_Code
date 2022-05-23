/*
 * ErrVal.cpp
 *
 *  Created on: Dec 30, 2018
 *      Author: david
 */

#include "ErrVal.h"

Err error(error_t const& error) {
  return {error};
}

Err error(error_t&& error) {
  return { std::move(error) };
}

